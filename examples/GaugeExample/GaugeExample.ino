#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();

// draw a simple gauge in the middle of the screen
Gauge g(&tft, 120, 120, 80, TFT_BLACK, TFT_GREEN);

void setup() {
    tft.init();
    tft.setRotation(1); // landscape
    tft.fillScreen(TFT_BLACK);

    // start at zero
    g.setValue(0);
}

void loop() {
    static int v = 0;
    v += 2;
    if (v > 100) v = 0;
    g.setValue(v);
    delay(100);
}
