#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();
Card *cards[3];
Gauge *gauges[3];
Graph *graphs[3];

void setup() {
    tft.init();
    tft.invertDisplay(true);
    tft.setRotation(1);
    tft.fillScreen(tft.color565(0x0a,0x0f,0x1a));

    // simple header
    tft.setTextSize(2);
    tft.setTextColor(tft.color565(0xe2,0xe8,0xf0));
    tft.drawString("PlantSense Pro", 10, 4);

    int cardW = 100;
    int cardH = 150;
    int spacing = 10;
    int startX = 10;
    int startY = 20;

    for (int i = 0; i < 3; i++) {
        int x = startX + i * (cardW + spacing);
        cards[i] = new Card(&tft, x, startY, cardW, cardH, "Plant");
        gauges[i] = new Gauge(&tft, x + cardW/2, startY + 60, 40);
        graphs[i] = new Graph(&tft, x + 10, startY + cardH - 60, cardW - 20, 50,
                              0, 100, "", LEGEND_RIGHT);
        cards[i]->draw();
    }
}

void loop() {
    for (int i = 0; i < 3; i++) {
        int m = random(0, 100);
        gauges[i]->setValue(m);
        graphs[i]->plotPoint(0, m);
        graphs[i]->nextX();
    }
    delay(4000);
}
