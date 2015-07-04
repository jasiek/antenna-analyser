#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <AD9850.h>

#define BACKGROUND ILI9341_BLACK

Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 9, 8);
AD9850 ad9850(7, 6, 5);

volatile byte mode = 0;
volatile boolean blankScreen = true;

void setup() {
  // Set up PIN 3 for mode switching via microswitch.
  pinMode(3, INPUT_PULLUP);
  attachInterrupt(1, changeMode, FALLING);
  
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
}

float f = 123;
void loop() {
  float swr;

  if (blankScreen) {
    tft.fillScreen(BACKGROUND);
    blankScreen = false;
  }

  if (mode == 0) {
    displayFrequency(f += 21, swr);
  } else {
    displayPlot(130, 140, "2200m");
  }
}

// Change screen. No debouncing, this is good enough for now.
void changeMode() {
  mode = (mode + 1) % 2;
  blankScreen = true;
}

bool labelsVisible = false;
void displayFrequency(float f, float swr) {
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

void displayPlot(float begin, float end, String label) {
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED);
  tft.println(label);
  
  float xstep = (end - begin) / tft.width();
  float ystep = tft.height() / 10;

  for (int x = 0; x < tft.width(); x++) {
    float f = begin + x * xstep;

    int y = tft.height() - measureSWR(f) * ystep;
    tft.drawPixel(x, y, ILI9341_GREEN); 
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
