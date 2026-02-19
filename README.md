# GraphTFT

**GraphTFT** is an **Arduino/ESP32/ESP8266 library** for drawing **real-time charts** on TFT displays using [TFT\_eSPI](https://github.com/Bodmer/TFT_eSPI).

![Demo](dashboardDemo.jpeg)

## ✨ Features

* 📈 **Scrolling line graph** for real-time data
* 🟦 **Bar chart** with numeric values
* 🥧 **Pie chart** with legend
* 🎯 **Circular gauge** widget for percentage/level indicators
* 📊 Multiple data series with customizable colors and labels
* 📍 Flexible legend placement: top, bottom, left, or right
* 📐 Automatic axis scaling and labeling
* ⚡ Easy integration with sensors and real-time data

---

## 📋 Requirements

* [TFT\_eSPI](https://github.com/Bodmer/TFT_eSPI)
* Supported hardware: Arduino, ESP32, ESP8266

---

## 🚀 Installation

1. Clone or download this repository.
2. In **Arduino IDE**, add the library via:
   **Sketch > Include Library > Add .ZIP Library...**
3. Install [TFT\_eSPI](https://github.com/Bodmer/TFT_eSPI) using the **Arduino Library Manager**.

---

## 📚 Constructors

### 🔹 Line Graph

```cpp
Graph(
    TFT_eSPI *display,
    int x0, int y0,
    int totalW, int totalH,
    int graphYmin, int graphYmax,
    String graphTitle,
    LegendPosition legend = LEGEND_RIGHT,
    int nSeries = 1,
    String names[] = nullptr,
    uint16_t colors[] = nullptr,
    uint16_t bg = TFT_BLACK
)
```

### 🔹 Pie Chart

```cpp
PieChart(
    TFT_eSPI *display,
    int x0, int y0,
    int totalW, int totalH,
    String graphTitle,
    LegendPosition legend = LEGEND_RIGHT,
    int nSeries = 1,
    String names[] = nullptr,
    uint16_t colors[] = nullptr,
    uint16_t bg = TFT_BLACK
)
```

### 🔹 Bar Chart

```cpp
BarChart(
    TFT_eSPI *display,
    int x0, int y0,
    int totalW, int totalH,
    String graphTitle,
    LegendPosition legend = LEGEND_RIGHT,
    int nSeries = 1,
    String names[] = nullptr,
    uint16_t colors[] = nullptr,
    uint16_t bg = TFT_BLACK
)
```

### 🔹 Gauge

```cpp
Gauge(
    TFT_eSPI *display,
    int cx, int cy,
    int radius,
    uint16_t bg = TFT_BLACK,         // inner/background color
    uint16_t fg = TFT_GREEN,         // progress color
    int minVal = 0,                  // value mapped to 0%
    int maxVal = 100                 // value mapped to 100%
)
```

---

## 🛠 Examples

### 📈 Line Graph (Random Data)

```cpp
#include <TFT_eSPI.h>
#include <GraphTFT.h>

#define TFT_GREY 0x5AEB

TFT_eSPI tft = TFT_eSPI();

String names[3] = {"Sensor A", "Sensor B", "Sensor C"};
uint16_t colors[3] = {TFT_GREEN, TFT_RED, TFT_BLUE};

Graph g(&tft, 20, 20, 280, 200, 0, 100, "Test Graph", LEGEND_BOTTOM, 3, names, colors);

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_GREY);
    g.resetGraph();
}

void loop() {
    g.plotPoint(0, random(0, 100));
    g.plotPoint(1, random(0, 100));
    g.plotPoint(2, random(0, 100));
    g.nextX();
    delay(100);
}
```

---

### 🌡️ Line Graph with DHT22

```cpp
#include <TFT_eSPI.h>
#include <GraphTFT.h>
#include <DHT.h>

#define TFT_GREY 0x5AEB
#define DHTPIN 5
#define DHTTYPE DHT22

TFT_eSPI tft = TFT_eSPI();
DHT dht(DHTPIN, DHTTYPE);

String names[2] = {"Temperature", "Humidity"};
uint16_t colors[2] = {TFT_YELLOW, TFT_BLUE};

Graph g(&tft, 20, 20, 280, 200, 0, 100, "DHT22 Graph", LEGEND_BOTTOM, 2, names, colors);

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_GREY);
    g.resetGraph();
    dht.begin();
}

void loop() {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
        g.plotPoint(0, temp);
        g.plotPoint(1, hum);
        g.nextX();
    }
    delay(30000);
}
```

---

### 🥧 Pie Chart

```cpp
#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();

String labels[3] = {"A", "B", "C"};
uint16_t colors[3] = {TFT_RED, TFT_GREEN, TFT_BLUE};

PieChart pie(&tft, 20, 20, 280, 200, "Pie Example", LEGEND_RIGHT, 3, labels, colors);

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    float values[3] = {40, 30, 30};
    pie.setData(values);
    pie.draw();
}

void loop() {}
```

---

### 🟦 Bar Chart

```cpp
#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();

String labels[3] = {"X", "Y", "Z"};
uint16_t colors[3] = {TFT_CYAN, TFT_MAGENTA, TFT_ORANGE};

BarChart bar(&tft, 20, 20, 280, 200, "Bar Example", LEGEND_BOTTOM, 3, labels, colors);

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    float values[3] = {10.5, 25.2, 15.7};
    bar.setData(values);
    bar.draw();
}

void loop() {}
```

---

### 🧮 Gauge

```cpp
#include <TFT_eSPI.h>
#include <GraphTFT.h>

TFT_eSPI tft = TFT_eSPI();

Gauge g(&tft, 120, 120, 80, TFT_BLACK, TFT_GREEN);

void setup() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    g.setValue(0);
}

void loop() {
    static int v = 0;
    v += 3;
    if (v > 100) v = 0;
    g.setValue(v);
    delay(100);
}
```

---

### 🃏 Generic Card

`Card` is a lightweight container class for drawing rounded rectangles with
a title. It’s designed to be reused in dashboard layouts going beyond the
example – you can subclass it or simply draw a card and then add your own
widgets inside it.

```cpp
Card c(&tft, 10, 10, 120, 80, "Notes");
c.draw();
```

### 📦 Card layout example

The card example demonstrates how to combine basic widgets inside the
`Card` container.  In this sketch each card holds a gauge and a scrolling
graph; the loop manually updates those elements.

```cpp
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

```

Full sketch is available in `examples/PlantDashboardExample/PlantDashboardExample.ino`.


## 📑 Public Functions

### 📈 `Graph` (Scrolling Line Graph)

| Function                                                        | Description                                 |
| --------------------------------------------------------------- | ------------------------------------------- |
| `plotPoint(int series, int value)`                              | Plots a point in the selected series        |
| `nextX()`                                                       | Advances the X axis (auto-scroll when full) |
| `resetGraph()`                                                  | Clears and resets the graph                 |
| *(internal)* `drawBox(), drawAxes(), drawTitle(), drawLegend()` | Draw helper functions                       |

---

### 🥧 `PieChart`

| Function                                 | Description                |
| ---------------------------------------- | -------------------------- |
| `setData(float values[])`                | Sets values for each slice |
| `draw()`                                 | Draws the pie chart        |
| *(internal)* `drawTitle(), drawLegend()` | Draw helper functions      |

---

### 🟦 `BarChart`

| Function                                 | Description                             |
| ---------------------------------------- | --------------------------------------- |
| `setData(float values[])`                | Sets values for each bar                |
| `draw()`                                 | Draws the bar chart with numeric values |
| *(internal)* `drawTitle(), drawLegend()` | Draw helper functions                   |

---

## 📄 License

This project is licensed under the **MIT License**.

## 👨‍💻 Author

Made with ❤️ by **Pedro Lucas**