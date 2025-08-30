#include "GraphTFT.h"
#include <math.h>

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

    for (int i = 0; i < seriesCount; i++) {
        seriesNames[i] = (names) ? names[i] : "S" + String(i+1);
        seriesColors[i] = (colors) ? colors[i] : TFT_GREEN;
    }

    legendSize = 0;
    if (legendPos == LEGEND_TOP || legendPos == LEGEND_BOTTOM) {
        legendSize = 15;
    } else {
        for (int i = 0; i < seriesCount; i++) {
            int textW = tft->textWidth(seriesNames[i]) + 20;
            if (textW > legendSize) legendSize = textW;
        }
    }

    plotX = x + axisMargin;
    plotY = y + titleSize;
    plotW = w - axisMargin;
    plotH = h - titleSize;

    switch (legendPos) {
        case LEGEND_TOP:    plotY += legendSize; plotH -= legendSize; break;
        case LEGEND_BOTTOM: plotH -= legendSize; break;
        case LEGEND_LEFT:   plotX += legendSize; plotW -= legendSize; break;
        case LEGEND_RIGHT:  plotW -= legendSize; break;
    }

    for (int i = 0; i < seriesCount; i++)
        for (int j = 0; j < plotW; j++)
            lastY[i][j] = plotY + plotH;

    drawBox();
    drawAxes();
    drawTitle();
    drawLegend();
}

void Graph::drawBox() {
    tft->fillRect(plotX, plotY, plotW, plotH, bgColor);
    tft->drawRect(plotX, plotY, plotW, plotH, TFT_WHITE);
}

void Graph::drawAxes(int yStep) {
    for (int v = yMin; v <= yMax; v += yStep) {
        int py = map(v, yMin, yMax, plotY + plotH, plotY);
        tft->drawLine(plotX - 3, py, plotX, py, TFT_WHITE);
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

    if (posX > 0) {
        int pxPrev = plotX + posX - 1;
        int pyPrev = lastY[series][posX - 1];
        tft->drawLine(pxPrev, pyPrev, px, py, seriesColors[series]);
    }
    lastY[series][posX] = py;
}

void Graph::nextX() {
    posX++;
    if (posX >= plotW) {
        posX = 0;
        resetGraph();
    }
}

void Graph::resetGraph() {
    drawBox();
    drawAxes();
    drawTitle();
    drawLegend();
    for (int i = 0; i < seriesCount; i++)
        for (int j = 0; j < plotW; j++)
            lastY[i][j] = plotY + plotH;
}


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

    for (int i = 0; i < slices; i++) {
        sliceLabels[i] = (names) ? names[i] : "S" + String(i+1);
        sliceColors[i] = (colors) ? colors[i] : tft->color565(50*i, 100, 200);
        sliceValues[i] = 0;
    }

    // calcula espaço para legenda
    legendSize = 0;
    if (legendPos == LEGEND_TOP || legendPos == LEGEND_BOTTOM) {
        legendSize = 15;
    } else {
        for (int i = 0; i < slices; i++) {
            int textW = tft->textWidth(sliceLabels[i]) + 20;
            if (textW > legendSize) legendSize = textW;
        }
    }

    // define centro e raio disponíveis
    int innerX = x, innerY = y, innerW = w, innerH = h;

    // espaço título
    innerY += titleSize;
    innerH -= titleSize;

    switch (legendPos) {
        case LEGEND_TOP:    innerY += legendSize; innerH -= legendSize; break;
        case LEGEND_BOTTOM: innerH -= legendSize; break;
        case LEGEND_LEFT:   innerX += legendSize; innerW -= legendSize; break;
        case LEGEND_RIGHT:  innerW -= legendSize; break;
    }

    cx = innerX + innerW/2;
    cy = innerY + innerH/2;
    r  = min(innerW, innerH) / 2 - 5; // margem interna
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

        for (float a = startAngle; a < endAngle; a += 1) {
            float x1 = cx + r * cos(radians(a));
            float y1 = cy + r * sin(radians(a));
            float x2 = cx + r * cos(radians(a+1));
            float y2 = cy + r * sin(radians(a+1));
            tft->fillTriangle(cx, cy, x1, y1, x2, y2, sliceColors[i]);
        }
        startAngle = endAngle;
    }

    drawTitle();
    drawLegend();
}

