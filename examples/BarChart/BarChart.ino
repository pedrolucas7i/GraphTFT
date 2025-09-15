#include <TFT_eSPI.h>
#include "GraphTFT.h"

TFT_eSPI tft = TFT_eSPI();  

// BarChart instance
String names[] = {"BTC", "ETH", "ADA", "XRP", "DOGE"};
uint16_t colors[] = {TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_YELLOW, TFT_RED};
BarChart barChart(&tft, 20, 20, 280, 200, "Crypto Market",
                  LEGEND_BOTTOM, 5, names, colors, TFT_BLACK);

float values[5] = {40, 30, 20, 15, 10}; // initial values

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  barChart.setData(values);
  barChart.draw();
}

void loop() {
  // Simulate small changes in bar values
  for (int i = 0; i < 5; i++) {
    int change = random(-5, 6);   // -5% to +5%
    values[i] += change;
    if (values[i] < 0) values[i] = 0; // no negative values
  }

  barChart.setData(values);
  barChart.draw();

  delay(2000); // update every 2s
}
