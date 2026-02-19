#include "GraphTFT.h"
#include <math.h>
#include <algorithm>   // std::swap used by anti‑alias routines

// -----------------------------------------------------------------------------
//  helper routines for simple anti-aliased drawing
// -----------------------------------------------------------------------------

// you can disable pixel reads if your controller doesn't support them or if
// performance is a concern. define to 0 before including GraphTFT.h.
#ifndef AA_USE_READPIXEL
#define AA_USE_READPIXEL 1
#endif

// fractional part of x
static float fpart(float x) { return x - floor(x); }
// reverse fractional part
static float rfpart(float x) { return 1.0f - fpart(x); }

// optional tweak that boosts small alpha values so the smoothing is more
// visible on low‑resolution panels. comment out or adjust if the feathering
// becomes too heavy.
static float adjustAlpha(float a) {
    // sqrt gives more weight to non‑zero fractions without exceeding 1
    return a <= 0.0f ? 0.0f : (a >= 1.0f ? 1.0f : sqrtf(a));
}

// blend two 16‑bit 5/6/5 colours with a given opacity for the first colour
static uint16_t blendColor(uint16_t c1, uint16_t c2, float alpha) {
    alpha = adjustAlpha(alpha);
    // clamp
    if (alpha <= 0.0f) return c2;
    if (alpha >= 1.0f) return c1;
    uint8_t r1 = (c1 >> 11) & 0x1F;
    uint8_t g1 = (c1 >> 5)  & 0x3F;
    uint8_t b1 =  c1        & 0x1F;
    uint8_t r2 = (c2 >> 11) & 0x1F;
    uint8_t g2 = (c2 >> 5)  & 0x3F;
    uint8_t b2 =  c2        & 0x1F;
    uint8_t r = (uint8_t)(r1*alpha + r2*(1.0f-alpha));
    uint8_t g = (uint8_t)(g1*alpha + g2*(1.0f-alpha));
    uint8_t b = (uint8_t)(b1*alpha + b2*(1.0f-alpha));
    return (r << 11) | (g << 5) | b;
}

// draw a pixel with blending against a fixed background colour
static void blendPixel(TFT_eSPI *tft, int x, int y,
                       uint16_t colour, uint16_t bg, float alpha) {
    tft->drawPixel(x, y, blendColor(colour, bg, alpha));
}

// Xiaolin Wu's anti‑aliased line algorithm adapted for 16‑bit TFT
static void drawAALine(TFT_eSPI *tft, int x0, int y0, int x1, int y1,
                        uint16_t colour, uint16_t bg) {
    using std::swap; // bring std::swap into unqualified lookup for built-in types
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) { swap(x0, y0); swap(x1, y1); }
    if (x0 > x1) { swap(x0, x1); swap(y0, y1); }
    int dx = x1 - x0;
    int dy = y1 - y0;
    float gradient = dx == 0 ? 1.0f : (float)dy / (float)dx;

    // first endpoint
    float xend = roundf(x0);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = (int)xend;
    int ypxl1 = (int)floorf(yend);
    if (steep) {
        blendPixel(tft, ypxl1,   xpxl1, colour, bg, rfpart(yend) * xgap);
        blendPixel(tft, ypxl1+1, xpxl1, colour, bg, fpart(yend)  * xgap);
    } else {
        blendPixel(tft, xpxl1, ypxl1,   colour, bg, rfpart(yend) * xgap);
        blendPixel(tft, xpxl1, ypxl1+1, colour, bg, fpart(yend)  * xgap);
    }
    float intery = yend + gradient;

    // main loop
    for (int x = xpxl1 + 1; x <= x1 - 1; x++) {
        if (steep) {
            blendPixel(tft, (int)floorf(intery),   x, colour, bg, rfpart(intery));
            blendPixel(tft, (int)floorf(intery)+1, x, colour, bg, fpart(intery));
        } else {
            blendPixel(tft, x, (int)floorf(intery),   colour, bg, rfpart(intery));
            blendPixel(tft, x, (int)floorf(intery)+1, colour, bg, fpart(intery));
        }
        intery += gradient;
    }

    // last endpoint
    xend = roundf(x1);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = (int)xend;
    int ypxl2 = (int)floorf(yend);
    if (steep) {
        blendPixel(tft, ypxl2,   xpxl2, colour, bg, rfpart(yend) * xgap);
        blendPixel(tft, ypxl2+1, xpxl2, colour, bg, fpart(yend)  * xgap);
    } else {
        blendPixel(tft, xpxl2, ypxl2,   colour, bg, rfpart(yend) * xgap);
        blendPixel(tft, xpxl2, ypxl2+1, colour, bg, fpart(yend)  * xgap);
    }
}

// draw an anti‑aliased rectangle outline
static void drawRectAA(TFT_eSPI *tft, int x, int y, int w, int h,
                       uint16_t colour, uint16_t bg) {
    drawAALine(tft, x,     y,     x + w - 1, y,     colour, bg);
    drawAALine(tft, x + w - 1, y,     x + w - 1, y + h - 1, colour, bg);
    drawAALine(tft, x + w - 1, y + h - 1, x,     y + h - 1, colour, bg);
    drawAALine(tft, x,     y + h - 1, x,     y,     colour, bg);
}

// draw a circle outline with a simple feather (two concentric rings)
static void drawCircleAA(TFT_eSPI *tft, int cx, int cy, int r,
                         uint16_t colour, uint16_t bg) {
    tft->drawCircle(cx, cy, r, colour);
    uint16_t fade = blendColor(colour, bg, 0.5f);
    if (r + 1 > 0) tft->drawCircle(cx, cy, r + 1, fade);
    if (r - 1 > 0) tft->drawCircle(cx, cy, r - 1, fade);
}

// helper that fills a circle and smooths its border
static void fillCircleAA(TFT_eSPI *tft, int cx, int cy, int r,
                         uint16_t colour, uint16_t bg) {
    tft->fillCircle(cx, cy, r, colour);
    drawCircleAA(tft, cx, cy, r, colour, bg);
}


// =======================
//   LINE GRAPH (with scroll)
// =======================
Graph::Graph(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
             int ymin, int ymax, String graphTitle,
             LegendPosition legend, int nSeries, String names[], uint16_t colors[],
             uint16_t bg) {
    
    tft = display;
    x = x0; y = y0; w = totalW; h = totalH;
    yMin = ymin; yMax = ymax;
    bgColor = bg;
    posX = 0;
    seriesCount = nSeries;
    title = graphTitle;
    legendPos = legend;

    // Initialize series names and colors
    for (int i = 0; i < seriesCount; i++) {
        seriesNames[i] = (names) ? names[i] : "S" + String(i+1);
        seriesColors[i] = (colors) ? colors[i] : TFT_GREEN;
    }

    // Calculate legend size depending on position
    legendSize = 0;
    if (legendPos == LEGEND_TOP || legendPos == LEGEND_BOTTOM) {
        legendSize = 15;
    } else {
        for (int i = 0; i < seriesCount; i++) {
            int textW = tft->textWidth(seriesNames[i]) + 20;
            if (textW > legendSize) legendSize = textW;
        }
    }

    // Plot area
    plotX = x + axisMargin;
    plotY = y + titleSize;
    plotW = w - axisMargin;
    plotH = h - titleSize;

    // Adjust plot area for legend placement
    switch (legendPos) {
        case LEGEND_TOP:    plotY += legendSize; plotH -= legendSize; break;
        case LEGEND_BOTTOM: plotH -= legendSize; break;
        case LEGEND_LEFT:   plotX += legendSize; plotW -= legendSize; break;
        case LEGEND_RIGHT:  plotW -= legendSize; break;
    }

    // Initialize data buffer with baseline
    for (int i = 0; i < seriesCount; i++)
        for (int j = 0; j < plotW; j++)
            lastY[i][j] = plotY + plotH;

    // Initial draw
    drawBox();
    drawAxes();
    drawTitle();
    drawLegend();
}

void Graph::drawBox() {
    // background is solid; use normal fill
    tft->fillRect(plotX, plotY, plotW, plotH, bgColor);
    // anti-aliased border makes the box edges softer
    drawRectAA(tft, plotX, plotY, plotW, plotH, TFT_WHITE, bgColor);
}

void Graph::drawAxes(int yStep) {
    for (int v = yMin; v <= yMax; v += yStep) {
        int py = map(v, yMin, yMax, plotY + plotH, plotY);
        drawAALine(tft, plotX - 3, py, plotX, py, TFT_WHITE, bgColor);
        tft->setTextColor(TFT_WHITE, bgColor);
        tft->setTextSize(1);
        tft->drawCentreString(String(v), plotX - 15, py - 4, 1);
    }
}

void Graph::drawTitle() {
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, bgColor);
    tft->drawCentreString(title, x + w/2, y, 2);
}

void Graph::drawLegend() {
    int boxSize = 10;
    int padding = 4;

    switch (legendPos) {
        case LEGEND_TOP:
        case LEGEND_BOTTOM: {
            int lx = plotX;
            int ly = (legendPos == LEGEND_TOP) ? y + titleSize : plotY + plotH + 2;
            for (int i = 0; i < seriesCount; i++) {
                int textW = tft->textWidth(seriesNames[i]);
                tft->fillRect(lx, ly, boxSize, boxSize, seriesColors[i]);
                tft->setCursor(lx + boxSize + padding, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(seriesNames[i]);
                lx += boxSize + padding + textW + padding;
            }
            break;
        }
        case LEGEND_LEFT:
        case LEGEND_RIGHT: {
            for (int i = 0; i < seriesCount; i++) {
                int lx = (legendPos == LEGEND_LEFT) ? x + 2 : plotX + plotW + 2;
                int ly = plotY + i*(plotH / seriesCount);
                tft->fillRect(lx, ly, boxSize, boxSize, seriesColors[i]);
                tft->setCursor(lx + boxSize + 2, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(seriesNames[i]);
            }
            break;
        }
    }
}

void Graph::plotPoint(int series, int value) {
    if (series < 0 || series >= seriesCount) return;
    int py = map(value, yMin, yMax, plotY + plotH, plotY);
    int px = plotX + posX;

    // Draw line from previous point to current
    if (posX > 0) {
        int pxPrev = plotX + posX - 1;
        int pyPrev = lastY[series][posX - 1];
        drawAALine(tft, pxPrev, pyPrev, px, py, seriesColors[series], bgColor);
    }
    lastY[series][posX] = py;
}

void Graph::nextX() {
    posX++;
    if (posX >= plotW) {
        // 🔹 Scroll mode: shift all data one pixel to the left
        for (int i = 0; i < seriesCount; i++) {
            for (int j = 1; j < plotW; j++) {
                lastY[i][j-1] = lastY[i][j];
            }
        }

        // Clear plot area and redraw background (smoothing will happen in drawBox/axes)
        drawBox();
        drawAxes();
        drawTitle();
        drawLegend();

        // Redraw all series with shifted data
        for (int i = 0; i < seriesCount; i++) {
            for (int j = 1; j < plotW; j++) {
                if (lastY[i][j-1] != plotY + plotH && lastY[i][j] != plotY + plotH) {
                    drawAALine(tft,
                               plotX + j - 1, lastY[i][j-1],
                               plotX + j,     lastY[i][j],
                               seriesColors[i], bgColor);
                }
            }
        }

        posX = plotW - 1; // keep cursor at right edge
    }
}

void Graph::resetGraph() {
    // 🔹 Completely clears and resets the graph (manual reset)
    drawBox();
    drawAxes();
    drawTitle();
    drawLegend();
    posX = 0;
    for (int i = 0; i < seriesCount; i++)
        for (int j = 0; j < plotW; j++)
            lastY[i][j] = plotY + plotH;
}


// =======================
//   PIE CHART
// =======================
PieChart::PieChart(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
                   String graphTitle, LegendPosition legend,
                   int nSeries, String names[], uint16_t colors[],
                   uint16_t bg) {
    tft = display;
    x = x0; y = y0; w = totalW; h = totalH;
    bgColor = bg;
    title = graphTitle;
    legendPos = legend;
    slices = nSeries;
    total = 0;

    // Initialize slices
    for (int i = 0; i < slices; i++) {
        sliceLabels[i] = (names) ? names[i] : "S" + String(i+1);
        sliceColors[i] = (colors) ? colors[i] : tft->color565(50*i, 100, 200);
        sliceValues[i] = 0;
    }

    // Legend size calculation
    legendSize = 0;
    if (legendPos == LEGEND_TOP || legendPos == LEGEND_BOTTOM) {
        legendSize = 15;
    } else {
        for (int i = 0; i < slices; i++) {
            int textW = tft->textWidth(sliceLabels[i]) + 20;
            if (textW > legendSize) legendSize = textW;
        }
    }

    // Available space for chart
    int innerX = x, innerY = y, innerW = w, innerH = h;

    // Title space
    innerY += titleSize;
    innerH -= titleSize;

    // Adjust for legend
    switch (legendPos) {
        case LEGEND_TOP:    innerY += legendSize; innerH -= legendSize; break;
        case LEGEND_BOTTOM: innerH -= legendSize; break;
        case LEGEND_LEFT:   innerX += legendSize; innerW -= legendSize; break;
        case LEGEND_RIGHT:  innerW -= legendSize; break;
    }

    // Center and radius
    cx = innerX + innerW/2;
    cy = innerY + innerH/2;
    r  = min(innerW, innerH) / 2 - 5; // small margin inside
}

void PieChart::setData(float values[]) {
    total = 0;
    for (int i = 0; i < slices; i++) {
        sliceValues[i] = values[i];
        total += values[i];
    }
}

void PieChart::drawTitle() {
    if (title != "") {
        tft->setTextColor(TFT_WHITE, bgColor);
        tft->setTextSize(1);
        tft->drawCentreString(title, x + w/2, y, 2);
    }
}

void PieChart::drawLegend() {
    int boxSize = 10;
    int padding = 4;

    switch (legendPos) {
        case LEGEND_TOP:
        case LEGEND_BOTTOM: {
            int lx = x + 5;
            int ly = (legendPos == LEGEND_TOP) ? y + titleSize : y + h - legendSize;
            for (int i = 0; i < slices; i++) {
                int textW = tft->textWidth(sliceLabels[i]);
                tft->fillRect(lx, ly, boxSize, boxSize, sliceColors[i]);
                tft->setCursor(lx + boxSize + padding, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(sliceLabels[i]);
                lx += boxSize + padding + textW + padding;
            }
            break;
        }
        case LEGEND_LEFT:
        case LEGEND_RIGHT: {
            for (int i = 0; i < slices; i++) {
                int lx = (legendPos == LEGEND_LEFT) ? x + 2 : x + w - legendSize + 2;
                int ly = y + titleSize + i*(h / slices);
                tft->fillRect(lx, ly, boxSize, boxSize, sliceColors[i]);
                tft->setCursor(lx + boxSize + 2, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(sliceLabels[i]);
            }
            break;
        }
    }
}

void PieChart::draw() {
    tft->fillRect(x, y, w, h, bgColor);

    float startAngle = 0;
    for (int i = 0; i < slices; i++) {
        if (total <= 0) continue;
        float angle = (sliceValues[i] / total) * 360.0;
        float endAngle = startAngle + angle;

        // Draw pie slice by filling triangles from center
        for (float a = startAngle; a < endAngle; a += 1) {
            float x1 = cx + r * cos(radians(a));
            float y1 = cy + r * sin(radians(a));
            float x2 = cx + r * cos(radians(a+1));
            float y2 = cy + r * sin(radians(a+1));
            tft->fillTriangle(cx, cy, x1, y1, x2, y2, sliceColors[i]);
        }
        startAngle = endAngle;
    }

    // soften circumference
    drawCircleAA(tft, cx, cy, r, TFT_WHITE, bgColor);

    drawTitle();
    drawLegend();
}

// =======================
//   BAR CHART
// =======================
BarChart::BarChart(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
                   String graphTitle, LegendPosition legend,
                   int nSeries, String names[], uint16_t colors[],
                   uint16_t bg) {
    tft = display;
    x = x0; y = y0; w = totalW; h = totalH;
    bgColor = bg;
    title = graphTitle;
    legendPos = legend;
    bars = nSeries;

    // Initialize series names and colors
    for (int i = 0; i < bars; i++) {
        barLabels[i] = (names) ? names[i] : "S" + String(i+1);
        barColors[i] = (colors) ? colors[i] : TFT_BLUE;
        barValues[i] = 0;
    }

    // Legend size calculation
    legendSize = 0;
    if (legendPos == LEGEND_TOP || legendPos == LEGEND_BOTTOM) {
        legendSize = 15;
    } else {
        for (int i = 0; i < bars; i++) {
            int textW = tft->textWidth(barLabels[i]) + 20;
            if (textW > legendSize) legendSize = textW;
        }
    }

    // Plot area
    plotX = x + axisMargin;
    plotY = y + titleSize;
    plotW = w - axisMargin;
    plotH = h - titleSize;

    // Adjust for legend
    switch (legendPos) {
        case LEGEND_TOP:    plotY += legendSize; plotH -= legendSize; break;
        case LEGEND_BOTTOM: plotH -= legendSize; break;
        case LEGEND_LEFT:   plotX += legendSize; plotW -= legendSize; break;
        case LEGEND_RIGHT:  plotW -= legendSize; break;
    }
}

void BarChart::setData(float values[]) {
    maxValue = 0;
    for (int i = 0; i < bars; i++) {
        barValues[i] = values[i];
        if (barValues[i] > maxValue) maxValue = barValues[i];
    }
}

void BarChart::drawTitle() {
    if (title != "") {
        tft->setTextColor(TFT_WHITE, bgColor);
        tft->setTextSize(1);
        tft->drawCentreString(title, x + w/2, y, 2);
    }
}

void BarChart::drawLegend() {
    int boxSize = 10;
    int padding = 4;

    switch (legendPos) {
        case LEGEND_TOP:
        case LEGEND_BOTTOM: {
            int lx = plotX;
            int ly = (legendPos == LEGEND_TOP) ? y + titleSize : plotY + plotH + 2;
            for (int i = 0; i < bars; i++) {
                int textW = tft->textWidth(barLabels[i]);
                tft->fillRect(lx, ly, boxSize, boxSize, barColors[i]);
                tft->setCursor(lx + boxSize + padding, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(barLabels[i]);
                lx += boxSize + padding + textW + padding;
            }
            break;
        }
        case LEGEND_LEFT:
        case LEGEND_RIGHT: {
            for (int i = 0; i < bars; i++) {
                int lx = (legendPos == LEGEND_LEFT) ? x + 2 : plotX + plotW + 2;
                int ly = plotY + i*(plotH / bars);
                tft->fillRect(lx, ly, boxSize, boxSize, barColors[i]);
                tft->setCursor(lx + boxSize + 2, ly);
                tft->setTextColor(TFT_WHITE, bgColor);
                tft->setTextSize(1);
                tft->print(barLabels[i]);
            }
            break;
        }
    }
}

void BarChart::draw() {
    // Clear plot
    tft->fillRect(plotX, plotY, plotW, plotH, bgColor);
    drawRectAA(tft, plotX, plotY, plotW, plotH, TFT_WHITE, bgColor);

    if (maxValue <= 0) return;

    int barWidth = plotW / bars;
    float scaleY = (float)plotH / maxValue;

    for (int i = 0; i < bars; i++) {
        int barHeight = (int)(barValues[i] * scaleY);
        int bx = plotX + i * barWidth + 2;
        int by = plotY + plotH - barHeight;

        // Draw bar
        // bars are solid, no AA needed on volume – edges softened by drawing small rectangles with background
        tft->fillRect(bx, by, barWidth - 4, barHeight, barColors[i]);

        // ==========================
        // Draw value (with decimals)
        // ==========================
        String valStr = String(barValues[i], 1); // 1 decimal place
        int textY = by - 12;                     // posição padrão (acima da barra)

        if (textY < plotY + 2) {
            // 🔹 Se o texto sair do retângulo, desenha dentro da barra
            textY = by + 2;

            // Contraste automático
            uint16_t color = (barColors[i] == TFT_BLACK || 
                              barColors[i] == TFT_BLUE || 
                              barColors[i] == TFT_RED)
                             ? TFT_WHITE : TFT_BLACK;

            tft->setTextColor(color, barColors[i]);
        } else {
            tft->setTextColor(TFT_WHITE, bgColor);
        }

        tft->setTextSize(1);
        tft->drawCentreString(valStr, bx + (barWidth/2), textY, 1);
    }

    drawTitle();
    drawLegend();
}


// =======================
//   GAUGE IMPLEMENTATION
// =======================

Gauge::Gauge(TFT_eSPI *display, int cx_, int cy_, int radius_,
             uint16_t bg, uint16_t fg, int minVal_, int maxVal_) {
    tft = display;
    cx = cx_;
    cy = cy_;
    radius = radius_;
    bgColor = bg;
    fgColor = fg;
    minVal = minVal_;
    maxVal = maxVal_;
    currValue = minVal;
    thickness = 14; // default ring thickness
    drawGauge();
}

void Gauge::setColors(uint16_t bg, uint16_t fg) {
    bgColor = bg;
    fgColor = fg;
    drawGauge();
}

void Gauge::setValue(int value) {
    if (value < minVal) value = minVal;
    if (value > maxVal) value = maxVal;
    currValue = value;
    drawGauge();
}

static float _deg2rad(float d) { return d * 0.017453292519943295; }

void Gauge::drawGauge() {
    // clear area (outer circle + some margin) with smooth border
    fillCircleAA(tft, cx, cy, radius + 2, bgColor, bgColor);

    // compute angle span from -90 (top) clockwise
    float span = 0;
    if (maxVal > minVal) {
        span = ((float)(currValue - minVal) / (maxVal - minVal)) * 360.0;
    }
    float start = -90;
    float end = start + span;

    // draw filled pie for progress
    for (float a = start; a < end; a += 1.0) {
        float x1 = cx + radius * cos(_deg2rad(a));
        float y1 = cy + radius * sin(_deg2rad(a));
        float x2 = cx + radius * cos(_deg2rad(a + 1));
        float y2 = cy + radius * sin(_deg2rad(a + 1));
        tft->fillTriangle(cx, cy, x1, y1, x2, y2, fgColor);
    }

    // soften outer edge of progress ring
    drawCircleAA(tft, cx, cy, radius, fgColor, bgColor);

    // mask center to create ring effect
    int innerR = radius - thickness;
    if (innerR > 0) {
        fillCircleAA(tft, cx, cy, innerR, bgColor, fgColor);
        // smooth inner boundary as well
        drawCircleAA(tft, cx, cy, innerR, bgColor, fgColor);
    }

    // draw numeric value in center
    tft->setTextColor(fgColor, bgColor);
    tft->setTextSize(1);
    tft->drawCentreString(String(currValue), cx, cy - 8, 4);
}


// =======================
//   PLANT CARD IMPLEMENTATION
// =======================



// ---------------------
//  Card implementation
// ---------------------

Card::Card(TFT_eSPI *display,
           int x0, int y0, int cardW, int cardH,
           const String &title_,
           uint16_t bg, uint16_t border, uint16_t text) :
    tft(display), x(x0), y(y0), w(cardW), h(cardH),
    title(title_),
    bgColor(bg), borderColor(border), textColor(text)
{
}

void Card::draw() {
    // draw rounded rect and header
    tft->fillRoundRect(x, y, w, h, 10, bgColor);
    tft->drawRoundRect(x, y, w, h, 10, borderColor);

    if (title.length()) {
        int tx = x + 10;
        int ty = y + 10;
        tft->setTextSize(1);
        tft->setTextColor(textColor);
        tft->drawString(title, tx, ty + 2);
    }
}

void Card::setTitle(const String &t) { title = t; }
void Card::setColors(uint16_t bg, uint16_t border, uint16_t text) {
    bgColor = bg;
    borderColor = border;
    textColor = text;
}


