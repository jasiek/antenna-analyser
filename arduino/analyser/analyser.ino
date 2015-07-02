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
  tft.fillScreen(BACKGROUND);
}

void loop() {
  float swr;
  for (float f = 120; f < 30000; f += 137) {
    swr = 999 / 100.0;
    displayFrequency(f, swr);
  }
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
  tft.print(String(swr));
}

