#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

Adafruit_ILI9341 tft = Adafruit_ILI9341(8, 10, 11, 12, 9, 13);

void setup() {
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
}

void loop() {
  drawSWRPlot(135.7, 137.8);
}

void drawSWRPlot(float begin, float end) {
  float xstep = (end - begin) / tft.height();
  float f = begin; // kHz
  float ystep = tft.width() / 64;

  for (int x = 0; x < tft.height(); x++) {
    float swr = measureSWR(f);
    int y = tft.height() - swr * ystep;
    tft.drawPixel(y, x, ILI9341_BLACK);
    
    f += xstep;
  }
}

float measureSWR(float f) {
  return 32; 
}


