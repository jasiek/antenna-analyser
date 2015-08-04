#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <AD9850.h>

#define BACKGROUND ILI9341_BLACK

// Analog pins are used for communication to make the circuit fit on a PCB easier.
Adafruit_ILI9341 tft = Adafruit_ILI9341(10, A3, A2);
AD9850 ad9850(7, 6, 5);

const String nameOfBand[] =  { "ALL",     "2200m",    "160m",     "80m",    "40m",    "30m",    "20m",    "17m",    "15m",      "12m",    "10m"   };
const double startOfBand[] = { 135E3,     135.7E3,    1.81E6,     3.5E6,    7E6,      10.1E6,   14E6,     18.068E6,  21E6,      24.89E6,  28E6    };
const double endOfBand[] =   { 29.7E6,    137.8E3,    2E6,        3.8E6,    7.2E6,    10.15E6,  14.35E6,  18.168E6,  21.450E6,  24.99E6,  29.7E6  };
const double steps[] =       { 1E6,       100,        3E3,        3E3,      3E3,      300,      3E3,      3E3,       3E3,       3E3,      6E3     };
const int numBands = 11;

// What kind of actions can be performed using the rotary encoder?
typedef enum {
  NONE,
  CLICK,
  LEFT,
  RIGHT
} Action;

void setup() {
  pinMode(2, INPUT_PULLUP); // Rotary encoder CLK
  pinMode(3, INPUT_PULLUP); // Rotary encoder CLICK
  pinMode(4, INPUT_PULLUP); // Rotary encoder DATA
  
  Serial.begin(57600);
  tft.begin();
  Serial.println("Width: " + String(tft.width()) + " Height: " + String(tft.height()));
  tft.setRotation(3);
}

void loop() {
  byte nextUIState = 9;

  while(1) {
    tft.fillScreen(BACKGROUND);
 
    switch(nextUIState) {
      case 9:
        nextUIState = mainMenu();
        break;
      case 0:
        displayAllBands();
        nextUIState = 9;
        break;
      case 1:
        displaySingleBand(chooseBandMenu());
        nextUIState = 9;
        break;
      case 2:
        displaySummary();
        nextUIState = 9;
        break;
    }
  }
}

// Display the main menu and handle view changes resulting from the operation of the
// rotary encoder.
// Returns to calling function when this menu is dismissed, communicating the menu state up the chain.
byte mainMenu() {
  byte mainMenuState = 0;
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_RED, BACKGROUND);
  tft.setCursor(10, 0);
  tft.print("Plot all bands");
  tft.setCursor(10, 10);
  tft.print("Plot single band");
  tft.setCursor(10, 20);
  tft.print("Display summary");

  while (1) {
    if (mainMenuState == 0) { tft.setTextColor(ILI9341_YELLOW, BACKGROUND); } else { tft.setTextColor(BACKGROUND, BACKGROUND); }
    tft.setCursor(0, 0);
    tft.print(">");
    if (mainMenuState == 1) { tft.setTextColor(ILI9341_YELLOW, BACKGROUND); } else { tft.setTextColor(BACKGROUND, BACKGROUND); }
    tft.setCursor(0, 10);
    tft.print(">");
    if (mainMenuState == 2) { tft.setTextColor(ILI9341_YELLOW, BACKGROUND); } else { tft.setTextColor(BACKGROUND, BACKGROUND); }
    tft.setCursor(0, 20);
    tft.print(">");
  
    switch(awaitAction()) {
      case CLICK:
        return mainMenuState;
      case LEFT:
        mainMenuState = (mainMenuState - 1) % 3;
        break;
      case RIGHT:
        mainMenuState = (mainMenuState + 1) % 3;
        break;
    }
  }
}

byte chooseBandMenu() {
  byte menuState = 0;
  byte i;
  tft.setTextColor(ILI9341_RED, BACKGROUND);
  for ( i = 0; i < numBands; i++) {
    tft.setCursor(10, i * 10);
    tft.print(nameOfBand[i]);
  }

  while (1) {
    for ( i = 0; i < numBands; i++) {
      tft.setCursor(0, i * 10);
      if (menuState == i) { tft.setTextColor(ILI9341_YELLOW, BACKGROUND); } else { tft.setTextColor(BACKGROUND, BACKGROUND); }
      tft.print(">");
    }

    switch (awaitAction()) {
      case CLICK:
        return menuState;
      case LEFT:
        menuState = (menuState - 1) % numBands;
        break;
      case RIGHT:
        menuState = (menuState + 1) % numBands;
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


void displayAllBands() {
  displayPlot(135E5, 29E6, "ALL");
  awaitAction();
}

void displaySingleBand(byte band) {
  displayPlot(startOfBand[band], endOfBand[band], nameOfBand[band]);
  awaitAction();
}

void displaySummary() {

  // for each band display the minimum swr and corresponding frequency
  
  int i;
  float swr, f;
  
  tft.setTextSize(1);

  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_RED, BACKGROUND);
  for (i = 1; i < numBands; i++) {
    tft.setCursor(0, i * 8);
    tft.print(nameOfBand[i]);
    tft.setCursor(6 * 6, i * 8);
    findMinimum(startOfBand[i], endOfBand[i], steps[i], &f, &swr);
    tft.print(f);
    tft.setCursor(18 * 6, i * 8);
    tft.print(swr);
  }

  awaitAction();
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

void findMinimum(float begin, float end, float step, float *freq, float *swr) {
  *freq = 0;
  *swr = 999;

  float tempswr, f;
  for (f = begin; f < end; f += step) {
    tempswr = measureSWR(f);
    if (tempswr < *swr) {
      *swr = tempswr;
      *freq = f;
    }
  }
}

float averageSWR(float begin, float end, float step) {
  float swrsum = 0;
  int count = 0;
  float f;
  for (f = begin; f < end; f += step) {
    swrsum += measureSWR(f);
    count++;
  }
  return swrsum / count;
}

float measureSWR(float f) {
  float fwd, rev;
  ad9850.setfreq(f);
  delay(10);
  fwd = analogRead(A0);
  rev = analogRead(A1);

  if (rev >= fwd) {
    return 999;
  } else {
    return (fwd + rev) / (fwd - rev);
  }
}
