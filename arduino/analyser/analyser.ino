#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(8, 10, 11, 12, 9, 13);

void setup() {
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.fillScreen(ILI9341_BLACK);
  tft.drawPixel(0, 0, ILI9341_GREEN);
}

void loop() {
  drawSWRPlot(135.7, 137.8);
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


