#ifndef GRAPHTFT_H
#define GRAPHTFT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum LegendPosition { LEGEND_TOP, LEGEND_BOTTOM, LEGEND_LEFT, LEGEND_RIGHT };

// =======================
//   LINE GRAPH
// =======================
class Graph {
public:
    Graph(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
          int ymin, int ymax, String graphTitle,
          LegendPosition legend = LEGEND_RIGHT,
          int nSeries = 1, String names[] = nullptr, uint16_t colors[] = nullptr,
          uint16_t bg = TFT_BLACK);

    void plotPoint(int series, int value);
    void nextX();
    void resetGraph();

private:
    int x, y, w, h;
    int plotX, plotY, plotW, plotH;
    int yMin, yMax;
    uint16_t bgColor;
    TFT_eSPI *tft;
    int posX;
    int seriesCount;
    String seriesNames[5];
    uint16_t seriesColors[5];
    int lastY[5][500];
    String title;
    LegendPosition legendPos;

    int titleSize = 20;
    int axisMargin = 20;
    int legendSize = 0;

    void drawBox();
    void drawAxes(int yStep = 10);
    void drawTitle();
    void drawLegend();
};


// =======================
//   PIE CHART
// =======================
class PieChart {
public:
    PieChart(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
             String graphTitle,
             LegendPosition legend = LEGEND_RIGHT,
             int nSeries = 1, String names[] = nullptr, uint16_t colors[] = nullptr,
             uint16_t bg = TFT_BLACK);

    void setData(float values[]);
    void draw();

private:
    TFT_eSPI *tft;
    int x, y, w, h;       // full area
    int cx, cy, r;        // center and radius
    int slices;
    String sliceLabels[10];
    uint16_t sliceColors[10];
    float sliceValues[10];
    float total;
    uint16_t bgColor;
    String title;
    LegendPosition legendPos;

    // Layout
    int titleSize = 20;
    int legendSize = 0;

    void drawLegend();
    void drawTitle();
};


// =======================
//   BAR CHART
// =======================
class BarChart {
public:
    BarChart(TFT_eSPI *display, int x0, int y0, int totalW, int totalH,
             String graphTitle,
             LegendPosition legend = LEGEND_RIGHT,
             int nSeries = 1, String names[] = nullptr, uint16_t colors[] = nullptr,
             uint16_t bg = TFT_BLACK);

    void setData(float values[]);
    void draw();

private:
    TFT_eSPI *tft;
    int x, y, w, h;
    int plotX, plotY, plotW, plotH;
    int bars;
    String barLabels[10];
    uint16_t barColors[10];
    float barValues[10];
    float maxValue;
    uint16_t bgColor;
    String title;
    LegendPosition legendPos;

    // Layout
    int titleSize = 20;
    int axisMargin = 20;
    int legendSize = 0;

    void drawTitle();
    void drawLegend();
};


// =======================
//   GAUGE (circular ring)
// =======================
class Gauge {
public:
    /**
     * @param display  TFT_eSPI pointer
     * @param cx       center X coordinate
     * @param cy       center Y coordinate
     * @param radius   outer radius of the ring
     * @param bg       background color (also used to clear the inner circle)
     * @param fg       progress color
     * @param minVal   minimum measurable value (mapped to 0%)
     * @param maxVal   maximum measurable value (mapped to 100%)
     */
    Gauge(TFT_eSPI *display, int cx, int cy, int radius,
          uint16_t bg = TFT_BLACK, uint16_t fg = TFT_GREEN,
          int minVal = 0, int maxVal = 100);

    /**
     * Set a new value (will be clamped) and redraw gauge
     */
    void setValue(int value);

    /**
     * Change gauge colours
     */
    void setColors(uint16_t bg, uint16_t fg);

private:
    TFT_eSPI *tft;
    int cx, cy, radius;
    int thickness;
    uint16_t bgColor, fgColor;
    int minVal, maxVal;
    int currValue;

    void drawGauge();
};

#endif
