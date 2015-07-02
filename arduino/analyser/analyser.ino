#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#define BACKGROUND ILI9341_BLACK

Adafruit_ILI9341 tft = Adafruit_ILI9341(8, 10, 11, 12, 9, 13);

void setup() {
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
}

void loop() {
  float swr;
  for (float f = 120; f < 30000; f += 137) {
    swr = 127 / 100.0;
    displayFrequency(f, swr);
  }
}

String previousFrequency;
String previousSWR;

void displayFrequency(float f, float swr) {
  int x, y;
  
  if (previousFrequency.length() == 0) {
    tft.setCursor(20, 20);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_RED);
    tft.print("Frequency:");
  }

  tft.setCursor(150, 20);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.setTextColor(BACKGROUND);
  tft.print(previousFrequency);
  tft.setCursor(x, y);

  tft.setTextColor(ILI9341_GREEN);
  previousFrequency = String(f) + " MHz";
  tft.print(previousFrequency);

  if (previousSWR.length() == 0) {
    tft.setCursor(20, 40);
    tft.setTextColor(ILI9341_RED);
    tft.print("VSWR:     ");
  }

  tft.setCursor(150, 40);
  x = tft.getCursorX();
  y = tft.getCursorY();
  tft.setTextColor(BACKGROUND);
  tft.print(previousSWR);
  tft.setCursor(x, y);
  
  tft.setTextColor(ILI9341_GREEN);
  previousSWR = String(swr);
  tft.print(previousSWR);
}


void drawSWRPlot(float begin, float end) {
  float xstep = (end - begin) / tft.height();
  float f = begin; // kHz
  float ystep = tft.width() / 64;

  for (int x = tft.height(); x >= 0; --x) {
    float swr = measureSWR(f);
    int y = tft.width() - swr * ystep;
    tft.drawPixel(y, x, ILI9341_WHITE);
    
    f += xstep;
  }
}

float measureSWR(float f) {
  return (f - 135) * (f - 135); 
}


