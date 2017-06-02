#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <AD9850.h>

#define BACKGROUND ILI9341_BLACK
#define X_TICS 5
#define Y_TICS 10

#define TABLEN 50
#define T0 0
#define T1 T0 + TABLEN
#define T2 T1 + TABLEN
#define T3 T2 + TABLEN
#define T4 T3 + TABLEN

// Analog pins are used for communication to make the circuit fit on a PCB easier.
Adafruit_ILI9341 tft = Adafruit_ILI9341(10, A3, A2);
AD9850 ad9850;

const String nameOfBand[] =  { "2200m",    "160m",     "80m",    "40m",    "30m",    "20m",    "17m",    "15m",      "12m",    "10m"   };
const double startOfBand[] = { 135.7E3,    1.81E6,     3.5E6,    7E6,      10.1E6,   14E6,     18.068E6,  21E6,      24.89E6,  28E6    };
const double endOfBand[] =   { 137.8E3,    2E6,        3.8E6,    7.2E6,    10.15E6,  14.35E6,  18.168E6,  21.450E6,  24.99E6,  29.7E6  };
const double steps[] =       { 100,        3E3,        3E3,      3E3,      300,      3E3,      3E3,       3E3,       3E3,      6E3     };
const int numBands = 10;

// What kind of actions can be performed using the rotary encoder?
typedef enum {
  NONE,
  CLICK,
  LEFT,
  RIGHT
} Action;

float measureSWR(double f);
void drawXAxis(double begin, double end, byte nticks);
void drawYAxis(byte nticks);
void drawXAxisLabels(String bandLabel);
void sweepBand(double begin, double end, double step, float *min, float *avg, float *max);
void displayPlot(double begin, double end, String label);
void rollHandler();
void clickHandler();
byte awaitAction();
void displaySingleBand(byte band);
void displayAllBands();
void displaySummary();
void displayInfo();
byte mainMenu();

void setup() {
  pinMode(2, INPUT_PULLUP); // Rotary encoder CLK
  pinMode(3, INPUT_PULLUP); // Rotary encoder CLICK
  pinMode(4, INPUT_PULLUP); // Rotary encoder DATA

  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
  ad9850.begin(7, 6, 5, 0);
}

void loop() {
  byte nextUIState = 100;

  while(1) {
    tft.fillScreen(BACKGROUND);

    switch(nextUIState) {
      case 100:
        nextUIState = mainMenu();
        break;
      case numBands:
        displayAllBands();
        nextUIState = 100;
        break;
      case numBands + 1:
        displaySummary();
        nextUIState = 100;
        break;
      case numBands + 2:
        displayInfo();
        nextUIState = 100;
        break;
      default:
        displaySingleBand(nextUIState);
        nextUIState = 100;
        break;
    }
  }
}

// Display the main menu and handle view changes resulting from the operation of the
// rotary encoder.
// Returns to calling function when this menu is dismissed, communicating the menu state up the chain.
byte mainMenu() {
  const byte menuEntries = numBands + 3;
  byte menuState = 0;
  byte i;
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_RED, BACKGROUND);
  for (i = 0; i < numBands; i++) {
    tft.setCursor(10, i * 10);
    tft.print(nameOfBand[i] + " band");
  }

  tft.setCursor(10, 10 * (numBands));
  tft.print("Plot all bands");
  tft.setCursor(10, 10 * (numBands + 1));
  tft.print("Display summary");
  tft.setCursor(10, 10 * (numBands + 2));
  tft.print("Device information");

  while (1) {
    for (i = 0; i < menuEntries; i++) {
      tft.setCursor(0, i * 10);
      if (menuState == i) { tft.setTextColor(ILI9341_YELLOW, BACKGROUND); } else { tft.setTextColor(BACKGROUND, BACKGROUND); }
      tft.print(">");
    }

    switch(awaitAction()) {
      case CLICK:
        return menuState;
      case LEFT:
        menuState = (menuState - 1) % menuEntries;
        break;
      case RIGHT:
        menuState = (menuState + 1) % menuEntries;
        break;
    }
  }
}

// Await user interaction by setting interrupt handlers and looping on a variable modified from
// within an ISR.
volatile Action selectedAction;
byte awaitAction() {
  selectedAction = NONE;

  detachInterrupt(0);
  detachInterrupt(1);
  attachInterrupt(0, clickHandler, CHANGE);
  attachInterrupt(1, rollHandler, FALLING);
  interrupts();

  while (selectedAction == NONE);
  return selectedAction;
}

// Handles left- and right-turns on the rotary encoder
void rollHandler() {
  noInterrupts();

  selectedAction = (digitalRead(2) == digitalRead(4)) ? LEFT : RIGHT;
}

// Handles clicks on the rotary encoder
void clickHandler() {
  noInterrupts();

  selectedAction = CLICK;
}

void displayInfo() {
  tft.fillScreen(BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED, BACKGROUND);
  tft.println("HF Antenna Analyzer");
  tft.println();

  tft.setTextColor(ILI9341_YELLOW, BACKGROUND);
  tft.setTextSize(1);
  tft.println("Based on hardware design by Beric Dunn (K6BEZ)");
  tft.println("Software by Jan Szumiec (M0NIT)");
  tft.println();
  tft.println("Build: " + String(__DATE__) + " " + String(__TIME__));

  awaitAction();
}

// Display a plot for all bands, when done - press button or rotate to exit
void displayAllBands() {
  displayPlot(135E3, 29E6, "ALL");
  awaitAction();
}

// Display a plot for a single band, when done - press button or rotate to exit
void displaySingleBand(byte band) {
  displayPlot(startOfBand[band], endOfBand[band], nameOfBand[band]);
  awaitAction();
}

// Display a textual summary for all bands
// band name, minimum SWR, maximum SWR
// background color indicates suitability (red-amber-green)
void displaySummary() {
  tft.setCursor(T0, 0);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_WHITE, BACKGROUND);
  tft.print("BAND");
  tft.setCursor(T1, 0);
  tft.print("MIN");
  tft.setCursor(T2, 0);
  tft.print("AVG");
  tft.setCursor(T3, 0);
  tft.print("MAX");

  float min, avg, max;
  int bgcolor;

  for (int i = 0; i < numBands; i++) {
    sweepBand(startOfBand[i], endOfBand[i], steps[i], &min, &avg, &max);
    byte row = 10 + i * 8;

    bgcolor = ILI9341_RED;
    if (avg < 1.6) { bgcolor = ILI9341_YELLOW; }
    if (avg < 1.3) { bgcolor = ILI9341_GREEN; }
    tft.setTextColor(ILI9341_BLACK, bgcolor);

    tft.setCursor(T0, row);
    tft.print(nameOfBand[i]);
    tft.setCursor(T1, row);
    tft.print(min);
    tft.setCursor(T2, row);
    tft.print(avg);
    tft.setCursor(T3, row);
    tft.print(max);
  }

  awaitAction();
}

void sweepBand(double begin, double end, double step, float *min, float *avg, float *max) {
  *min = 99999;
  *max = 0;
  float sum = 0;
  float swr;
  int count = 0;
  double f;

  for (f = begin; f < end; f += step) {
    swr = measureSWR(f);
    if (swr > *max) { *max = swr; }
    if (swr < *min) { *min = swr; }
    sum += swr;
    count ++;
  }
  *avg = sum / count;
}

void displayPlot(double begin, double end, String label) {
  tft.fillScreen(BACKGROUND);

  double xstep = (end - begin) / tft.width();
  double ystep = tft.height() / Y_TICS;

  drawXAxis(begin, end, X_TICS);
  drawYAxis(Y_TICS);
  drawXAxisLabels(label);

  float maxSWR = 0;
  float minSWR = 1000;

  Serial.println("start= " + String(begin));
  Serial.println("end  = " + String(end));
  Serial.println("xstep= " + String(xstep));
  Serial.println("ystep= " + String(ystep));

  int prevx = 0;
  int prevy = 240;
  for (int x = 0; x < tft.width(); x += 5) {
    double f = begin + x * xstep;
    float swr = measureSWR(f);

    if (swr < minSWR) { minSWR = swr; }
    if (swr > maxSWR) { maxSWR = swr; }

    int y = tft.height() - swr * ystep;
    tft.drawLine(prevx, prevy, x, y, ILI9341_GREEN);
    prevx = x;
    prevy = y;
  }

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLUE, BACKGROUND);
  tft.setCursor(60, tft.height() - 10);
  tft.print("min=" + String(minSWR));
  tft.setCursor(120, tft.height() - 10);
  tft.print("max=" + String(maxSWR));
}

void drawXAxisLabels(String bandLabel) {
  tft.setTextSize(2);
  tft.setCursor(0, tft.height() - 15);
  tft.setTextColor(ILI9341_RED);
  tft.print(bandLabel);
}

void drawXAxis(double begin, double end, byte nticks) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLUE, BACKGROUND);
  byte n, xstep = tft.width() / nticks;
  double fstep = (end - begin) / nticks;

  for (n = 0; n < nticks; n++) {
    tft.drawLine(n * xstep, 0, n * xstep, 3, ILI9341_BLUE);
    tft.setCursor(n * xstep, 4);
    tft.print((begin + n * fstep) / 1E6);
  }
}

void drawYAxis(byte nticks) {
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLUE, BACKGROUND);
  byte n, ystep = tft.height() / nticks;
  for (n = nticks; n > 0; n--) {
    tft.drawLine(tft.width() - 1, n * ystep, tft.width() - 3, n * ystep, ILI9341_BLUE);
    tft.setCursor(tft.width() - 8, n * ystep - 3);
    tft.print(nticks - n);
  }
}

float measureSWR(double f) {
  float fwd, rev;
  ad9850.setfreq(f, 0);
  delay(10);
  fwd = analogRead(A0);
  rev = analogRead(A1);

  if (rev >= fwd) {
    return 999;
  } else {
    return (fwd + rev) / (fwd - rev);
  }
}
