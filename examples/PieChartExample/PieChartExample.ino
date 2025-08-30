#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();

String labels[4] = {"Apples", "Bananas", "Cherries", "Dates"};
uint16_t colors[4] = {TFT_RED, TFT_YELLOW, TFT_GREEN, TFT_BLUE};

// x, y, width, height totais (inclui título e legenda)
PieChart pie(&tft, 20, 20, 280, 200, "Fruit Share", LEGEND_RIGHT, 4, labels, colors);

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    float data[4] = {30, 45, 15, 10};
    pie.setData(data);
    pie.draw();
}

void loop() {

}
