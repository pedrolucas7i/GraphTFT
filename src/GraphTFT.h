#ifndef GRAPHTFT_H
#define GRAPHTFT_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum LegendPosition { LEGEND_TOP, LEGEND_BOTTOM, LEGEND_LEFT, LEGEND_RIGHT };

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

#endif
