#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST    4
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_BL    32

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define TFT_WIDTH  172
#define TFT_HEIGHT 320

void setup() {
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI);
  tft.init(TFT_WIDTH, TFT_HEIGHT);

  // Ruota display di 90° CW
  tft.setRotation(1);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  tft.fillScreen(ST77XX_WHITE);

  String line1 = "Hello";
  String line2 = "World";

  int size1 = 2;
  int size2 = 3;

  // Dopo la rotazione: scambiamo logica width/height
  int screenWidth = TFT_HEIGHT;  // effettiva larghezza fisica
  int screenHeight = TFT_WIDTH;  // effettiva altezza fisica

  // Larghezza testo
  int w1 = line1.length() * 6 * size1;
  int w2 = line2.length() * 6 * size2;

  // Altezza testo
  int h1 = 8 * size1;
  int h2 = 8 * size2;
  int totalHeight = h1 + h2 + 5;

  int x1 = (screenWidth - w1) / 2;
  int x2 = (screenWidth - w2) / 2;
  int y1 = (screenHeight - totalHeight) / 2;
  int y2 = y1 + h1 + 5;

  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(x1, y1);
  tft.setTextSize(size1);
  tft.print(line1);

  tft.setCursor(x2, y2);
  tft.setTextSize(size2);
  tft.print(line2);
}

void loop() {}
