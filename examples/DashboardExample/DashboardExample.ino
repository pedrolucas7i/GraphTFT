#include <TFT_eSPI.h>
#include "GraphTFT.h"

TFT_eSPI tft = TFT_eSPI();

uint16_t COLORS[] = {
  tft.color565(66, 135, 245),   // Azul
  tft.color565(102, 187, 106),  // Verde
  tft.color565(255, 167, 38),   // Laranja
  tft.color565(171, 71, 188)    // Roxo
};

String lineNames[] = {"Temp °C","Hum %"};
String barNames[]  = {"ON","OFF","ALERT"};
String pieNames[]  = {"Solar","Grid","Battery"};

// since we added gauge support, we'll show moisture as a circular widget
Gauge moistureGauge(&tft, 120, 60, 50, tft.color565(30,30,30), COLORS[1]);

// --- Charts --- //
Graph lineChart(&tft, 0, 0, 245, 100, 0, 100, "Climate Monitor",
                  LEGEND_BOTTOM, 2, lineNames, COLORS, tft.color565(30,30,30));

BarChart barChart(&tft, 0, 100, 160, 140, "Device Status",
                  LEGEND_BOTTOM, 3, barNames, COLORS, tft.color565(30,30,30));

PieChart pieChart(&tft, 160, 100, 160, 140, "Energy Mix",
                  LEGEND_BOTTOM, 3, pieNames, COLORS, tft.color565(30,30,30));

// --- Mini Widget (canto superior direito do Line Chart) --- //
void drawMiniWidget(float temp, float hum) {
  int x0 = 250, y0 = 20, w = 65, h = 65;
  tft.fillRect(x0, y0, w, h, tft.color565(50,50,50));
  tft.drawRect(x0, y0, w, h, TFT_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, tft.color565(50,50,50));

  tft.drawString("T: " + String(temp,1) + "C", x0+5, y0+16);
  tft.drawString("H: " + String(hum,1) + "%", x0+5, y0+40);
}

void setup() {
  tft.init();
  tft.setRotation(1); // landscape
  tft.fillScreen(tft.color565(20,20,20));

  // Inicializar gráficos
  lineChart.resetGraph();

  float barVals[] = {10, 5, 2};
  barChart.setData(barVals);
  barChart.draw();

  float pieVals[] = {50, 30, 20};
  pieChart.setData(pieVals);
  pieChart.draw();

  // initialize gauge to zero moisture
  moistureGauge.setValue(0);
}

void loop() {
  static int counter = 0;
  float temp = 20 + random(-2,3) + sin(counter*0.2)*5;
  float hum  = 50 + random(-3,4) + cos(counter*0.15)*10;

  // Atualiza gráfico de linha
  lineChart.plotPoint(0, temp);
  lineChart.plotPoint(1, hum);
  lineChart.nextX();

  // Atualiza mini widget
  drawMiniWidget(temp, hum);

  // Atualiza barras (simulação)
  float barVals[] = {random(5,15), random(2,10), random(1,6)};
  barChart.setData(barVals);
  barChart.draw();

  // Atualiza pizza (simulação)
  float solar = random(30,60);
  float grid  = random(10,40);
  float batt  = 100 - solar - grid;
  float pieVals[] = {solar, grid, batt};
  pieChart.setData(pieVals);
  pieChart.draw();

  // Atualiza gauge (simulação de humidade/moisture)
  float moisture = random(0, 100);
  moistureGauge.setValue((int)moisture);

  counter++;
  delay(1200);
}
