#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Pin mapping ESP32 → ST7789V
#define TFT_CS    15    // D15
#define TFT_DC     2    // D2
#define TFT_RST    4    // D4
#define TFT_MOSI  23    // D23
#define TFT_SCLK  18    // D18
#define TFT_BL    32    // D32 (backlight)

// Oggetto display
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Dimensioni display
#define TFT_WIDTH  172
#define TFT_HEIGHT 320

void setup() {
  // Reset hardware
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  // Inizializza SPI
  SPI.begin(TFT_SCLK, -1, TFT_MOSI);

  // Inizializza display
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(0);

  // Accendi backlight
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // Pulizia completa: fillScreen bianco
  tft.fillScreen(ST77XX_WHITE);
  delay(100);

  // Sovrascrivi tutta la RAM con scacchiera a blocchi
  int blockSize = 10; // dimensione quadrato scacchiera
  for (int y = 0; y < TFT_HEIGHT; y += blockSize) {
    for (int x = 0; x < TFT_WIDTH; x += blockSize) {
      if (((x/blockSize) + (y/blockSize)) % 2 == 0) {
        tft.fillRect(x, y, blockSize, blockSize, ST77XX_RED);
      } else {
        tft.fillRect(x, y, blockSize, blockSize, ST77XX_BLUE);
      }
    }
  }
}

void loop() {
  // Nessuna azione, scacchiera fissa
}
