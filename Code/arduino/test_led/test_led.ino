/*

*/
/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27


int squareA[] = {0,1,5,6};
int squareB[] = {4,3,8,9};
int squareC[] = {20,15,16,21};
int squareD[] = {24,18,19,23};
int remote[] =  {2,7,12,17,22,10,11,13,14};

// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
uint32_t colour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow };

//rotate / flip
// https://macetech.github.io/FastLED-XY-Map-Generator/
const uint8_t XYTable[] = {
  4,   9,  14,  19,  24,
  3,   8,  13,  18,  23,
  2,   7,  12,  17,  22,
  1,   6,  11,  16,  21,
  0,   5,  10,  15,  20
};

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  delay(1000);
  Serial.begin(9600);
  //fullColor(CRGB::Blue);
  
  
  // init dispay
  colorSquare(squareA,CRGB::Blue,4);
  delay(1000);
  colorSquare(squareB,CRGB::Red,4);
  delay(1000);
  colorSquare(squareC,CRGB::Green,4);
  delay(1000);
  colorSquare(squareD,CRGB::Yellow,4);
  delay(1000);

  colorSquare(squareA,CRGB::Black,1);
  colorSquare(squareB,CRGB::Black,1);
  colorSquare(squareC,CRGB::Black,1);
  colorSquare(squareD,CRGB::Black,1);
  delay(3000);
  colorSquare(remote,CRGB::White,9);

  delay(3000);
  colorSquare(squareA,CRGB::Blue,1);
  
  

}

void loop() {

}

void fullColor(uint32_t color) {

  for (int num = 0; num < NUM_LEDS; num++) {
    leds[num] = color;
    //leds[XYTable[num]] = color;
    FastLED.show();
  }

}

void colorSquare(int square[] ,uint32_t color,int numx) {

  for (int num = 0; num < numx; num++) {  
    
    leds[square[num]] = color;    
    if (numx==9) delay(100);    
  }
  FastLED.show();

}
