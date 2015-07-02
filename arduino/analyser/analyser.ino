#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define BACKGROUND ILI9341_BLACK

Adafruit_ILI9341 tft = Adafruit_ILI9341(10, 9, 8);

void setup() {
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
  
}

void loop() {
  displayPlot(130, 140, "2200m");
  /* float swr;
  for (float f = 120; f < 30000; f += 1) {
    swr = 999 / 100.0;
    displayFrequency(f, swr);
  } */
}

bool labelsVisible = false;
void displayFrequency(float f, float swr) {
  tft.fillScreen(BACKGROUND);
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
  tft.print(String(swr));
}

void displayPlot(float begin, float end, String label) {
  tft.fillScreen(BACKGROUND);
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
  return (f - 137) * (f - 137);
}

