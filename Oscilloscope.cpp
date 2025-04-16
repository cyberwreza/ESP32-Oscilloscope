// This script was made by Reza -- @c7berwreza on TikTok.

#include <Arduino.h>
#include <U8g2lib.h>

// === OLED  ===
// Use full buffer constructor (for smoother redraws)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0, /* reset=*/ U8X8_PIN_NONE, 
  /* clock=*/ 22, 
  /* data=*/ 21
);

// === CONFIGURATION ===
const int analogPin = 36;      // ADC pin on ESP32 (VP)
const int screenWidth = 128;
const int screenHeight = 64;
const int centerY = screenHeight / 2;
const int centerX = screenWidth / 2;
const int gridSize = 8;        // Grid spacing in pixels
const int adcResolution = 4095;
const int sampleDelay = 5;     // ms between samples

int waveform[screenWidth];     // Holds latest screen-width samples

// === INIT ===
void setup() {
  Serial.begin(115200);
  u8g2.begin();
  analogReadResolution(12); // 0–4095 for ESP32

  // Start waveform with center line
  for (int i = 0; i < screenWidth; i++) {
    waveform[i] = centerY;
  }
}

// === DRAW GRID & AXES ===
void drawGridAndAxis() {
  u8g2.setDrawColor(1); // white

  // Dotted vertical grid lines
  for (int x = 0; x < screenWidth; x += gridSize) {
    if (x == centerX) continue; // skip center axis for solid line
    for (int y = 0; y < screenHeight; y += 2) {
      u8g2.drawPixel(x, y);
    }
  }

  // Dotted horizontal grid lines
  for (int y = 0; y < screenHeight; y += gridSize) {
    if (y == centerY) continue; // skip center axis for solid line
    for (int x = 0; x < screenWidth; x += 2) {
      u8g2.drawPixel(x, y);
    }
  }

  // Solid center X and Y axes
  u8g2.drawHLine(0, centerY, screenWidth);   // Time axis
  u8g2.drawVLine(centerX, 0, screenHeight);  // Voltage axis

  // Axis labels
  u8g2.setFont(u8g2_font_5x8_tr);
  u8g2.drawStr(2, 10, "V");
  u8g2.drawStr(screenWidth - 18, centerY - 2, "Time");
}

// === DRAW WAVEFORM ===
void drawWaveform() {
  for (int i = 1; i < screenWidth; i++) {
    u8g2.drawLine(i - 1, waveform[i - 1], i, waveform[i]);
  }
}

// === DRAW SCALE LABELS ===
void drawScaleLabels() {
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.setCursor(2, screenHeight - 2);
  u8g2.print("1V/div");

  u8g2.setCursor(screenWidth - 40, screenHeight - 2);
  u8g2.print("5ms/div");
}

// === UPDATE WAVEFORM BUFFER ===
void updateWaveform() {
  // Shift left to make room for new sample
  for (int i = 0; i < screenWidth - 1; i++) {
    waveform[i] = waveform[i + 1];
  }

  // Read and smooth analog input
  int raw = analogRead(analogPin);
  static int previous = raw;
  raw = (raw + previous) / 2;
  previous = raw;

  // Scale to display range (inverted Y)
  int y = map(raw, 0, adcResolution, screenHeight - 1, 0);
  waveform[screenWidth - 1] = y;
}

// === MAIN LOOP ===
void loop() {
  updateWaveform();

  u8g2.firstPage();
  do {
    drawGridAndAxis();
    drawWaveform();
    drawScaleLabels();
  } while (u8g2.nextPage());

  delay(sampleDelay);
}
