/*

*/
/* remote */
#include "Lpf2Hub.h"
Lpf2Hub myRemote;
byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
bool isRemoteInitialized = false;

/* end remote */


/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27


int squareA[] = {0,1,5,6};
int squareB[] = {4,3,8,9};
int squareC[] = {20,15,16,21};
int squareD[] = {24,18,19,23};
int* allsquares[4] = {squareA,squareB,squareC,squareD};
int remote[] =  {2,7,12,17,22,10,11,13,14};


byte chargen[][5] = {
{0x1E, 0x05, 0x05, 0x05, 0x1E},  // A
{0x0E, 0x11, 0x11, 0x11, 0x11},  // C
{0x0E, 0x11, 0x11, 0x11, 0x0E},  // O
{0x1F, 0x10, 0x10, 0x10, 0x10},  // L
};



// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
int TOTNUM_COLORS = 4;
uint32_t maincolour[] = {CRGB::Red, CRGB::Yellow, CRGB::Teal, CRGB::Green };
uint32_t tilecolour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow, CRGB::Blue, CRGB::White };

//rotate / flip
// https://macetech.github.io/FastLED-XY-Map-Generator/
const uint8_t XYTable[] = {
     0,   5,  10,  15,  20,
     1,   6,  11,  16,  21,
     2,   7,  12,  17,  22,
     3,   8,  13,  18,  23,
     4,   9,  14,  19,  24
  };
CRGB leds[NUM_LEDS];

/* end led */



void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  delay(1000);
  Serial.begin(115200);  
    
  init();

  // test 	
  /*
  colorSquare(squareA,CRGB::Black,0,1);
  colorSquare(squareB,CRGB::Black,0,1);
  colorSquare(squareC,CRGB::Black,0,1);
  colorSquare(squareD,CRGB::Black,0,1);    
  delay(3000);
  colorSquare(squareA,CRGB::Red,0,1); 
  */ 
  
}

void loop() {

    // remote controller
     if (! isRemoteInitialized) scanRemoteController();
}

void init(){

   // led 
   initDisplay();

   //remote
   myRemote.init(1);
  
}


