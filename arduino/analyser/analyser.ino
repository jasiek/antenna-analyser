#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <AD9850.h>

#define BACKGROUND ILI9341_BLACK

Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 9, 8);
AD9850 ad9850(7, 6, 5);

const String nameOfBand[] =  { "ALL", "2200m", "160m", "80m", "40m", "30m", "20m",    "17m", "15m",   "12m", "10m"};
const double startOfBand[] = { 135E3, 135.7E3, 1.81E6, 3.5E6,  7E6, 10.1E6,  14E6, 18.068E6,  21E6, 24.89E6, 28E6};
const double endOfBand[] =   { 29.7E6, 137.8E3,    2E6, 3.8E6,7.2E6, 10.15E6, 14.35E6, 18.168E6, 21.450E6, 24.99E6, 29.7E6};
const double steps[] =       { 1E6,    100,    3E3,   3E3,  3E3,    50,   3E3,     3E3,     3E3,    3E3,   6E3};

volatile byte mode = 0;
volatile boolean blankScreen = true;
volatile byte band = 0; 
volatile double f = startOfBand[band];

void setup() {
  // Set up PIN 3 for mode switching via microswitch.
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  attachInterrupt(0, changeMode, FALLING);
  attachInterrupt(1, changeFrequency, RISING);
  
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
}

void loop() {
  float swr;

  if (blankScreen) {
    tft.fillScreen(BACKGROUND);
    blankScreen = false;
  }

  if (mode == 0) {
    displayFrequency(f, swr);
  } else {
    Serial.println("BAND: " + String(band) + ", " + nameOfBand[band]);
    displayPlot(startOfBand[band], endOfBand[band], nameOfBand[band]);
  }
}

// Change screen. No debouncing, this is good enough for now.
void changeMode() {
  band = (band + 1) % 11;
  
}

void changeFrequency() {
  switch(digitalRead(4)) {
    case LOW:
    f += 10E3;
    break;
    case HIGH:
    f -= 10E3;
  }

  if (f < 136E3) {
    f = 28E6;
    return;
  }

  if (f > 28E6) {
    f = 136E3;
    return;
  }
}


bool labelsVisible = false;
void displayFrequency(double f, double swr) {
  tft.setTextSize(2);

  if (!labelsVisible) {
    tft.setCursor(20, 20);
    tft.setTextColor(ILI9341_RED, BACKGROUND);
    tft.print("Frequency:");
  
    tft.setCursor(20, 40);
    tft.setTextColor(ILI9341_RED, BACKGROUND);
    tft.print("VSWR:");

    labelsVisible = true;
  }
  
  tft.setCursor(150, 20);
  tft.setTextColor(ILI9341_GREEN, BACKGROUND);
  tft.print(String(f) + " kHz  ");

  tft.setCursor(150, 40);
  tft.setTextColor(ILI9341_GREEN, BACKGROUND);
  tft.print(measureSWR(f));
}

void displayPlot(double begin, double end, String label) {
  tft.fillScreen(BACKGROUND);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED);
  tft.println(label);
  
  double xstep = (end - begin) / tft.width();
  double ystep = tft.height() / 10;

  float maxSWR = 0;
  float minSWR = 1000;

  Serial.println("start= " + String(begin));
  Serial.println("end  = " + String(end));
  Serial.println("xstep= " + String(xstep));
  Serial.println("ystep= " + String(ystep));

  int prevx = 0;
  int prevy = 240;
  for (int x = 0; x < tft.width(); x++) {
    double f = begin + x * xstep;
    float swr = measureSWR(f);

    if (swr < minSWR) { minSWR = swr; }
    if (swr > maxSWR) { maxSWR = swr; }
    
    int y = tft.height() - swr * ystep;
    tft.drawLine(prevx, prevy, x, y, ILI9341_GREEN);
    prevx = x;
    prevy = y;

    if (x % 30 == 0) {
      tft.setTextSize(1);
      tft.setCursor(0, 50);
      tft.setTextColor(ILI9341_BLUE, BACKGROUND);
      tft.println("min= " + String(minSWR));
      tft.println("max= " + String(maxSWR));
    }
  }
}

float measureSWR(float f) {
  float fwd, rev;
  ad9850.setfreq(f);
  delay(10);
  rev = analogRead(A0);
  fwd = analogRead(A1);

  if (rev >= fwd) {
    return 999;
  } else {
    return (fwd + rev) / (fwd - rev);
  }
}
