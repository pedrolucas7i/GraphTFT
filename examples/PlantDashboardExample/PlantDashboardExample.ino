#include <TFT_eSPI.h>
#include <GraphTFT.h>

// simple card-based dashboard with multiple gauge widgets

TFT_eSPI tft = TFT_eSPI();

struct PlantCard {
    int x, y, w, h;
    Gauge *gauge;
    String name;
};

PlantCard cards[3];

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // define three cards side by side
    int cardW = 100;
    int cardH = 140;
    int spacing = 10;
    int startX = 10;
    int startY = 20;

    String names[3] = {"Monstera","Basil","Fern"};

    for (int i = 0; i < 3; i++) {
        cards[i].x = startX + i * (cardW + spacing);
        cards[i].y = startY;
        cards[i].w = cardW;
        cards[i].h = cardH;
        cards[i].name = names[i];

        // draw card background
        tft.fillRect(cards[i].x, cards[i].y, cardW, cardH, tft.color565(30,30,30));
        tft.drawRect(cards[i].x, cards[i].y, cardW, cardH, TFT_WHITE);

        // title
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.drawString(cards[i].name, cards[i].x + 5, cards[i].y + 5);

        // create gauge centered inside card
        int cx = cards[i].x + cardW/2;
        int cy = cards[i].y + 60;
        cards[i].gauge = new Gauge(&tft, cx, cy, 40, tft.color565(30,30,30), TFT_GREEN);
        cards[i].gauge->setValue(0);
    }
}

void loop() {
    // update each gauge with new random moisture values
    for (int i = 0; i < 3; i++) {
        int m = random(0, 101);
        cards[i].gauge->setValue(m);
    }
    delay(500);
}
