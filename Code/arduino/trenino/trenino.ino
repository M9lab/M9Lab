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
int* allsquares[4] = {squareA,squareB,squareC,squareD};
int remote[] =  {2,7,12,17,22,10,11,13,14};


byte chargen[][5] = {
{0x04, 0x0A, 0x11, 0x1F, 0x11},  // A
{0x1E, 0x11, 0x10, 0x11, 0x1E},  // C
{0x0E, 0x11, 0x11, 0x11, 0x0E},  // O
{0x10, 0x10, 0x10, 0x10, 0x1F},  // L
};



// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
int TOTNUM_COLORS = 4;
uint32_t maincolour[] = {CRGB::Red, CRGB::Yellow, CRGB::Teal, CRGB::Green };
uint32_t tilecolour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow, CRGB::Blue, CRGB::White };

//rotate / flip
// https://macetech.github.io/FastLED-XY-Map-Generator/
const uint8_t XYTable[] = {
     4,   3,   2,   1,   0,
     9,   8,   7,   6,   5,
    14,  13,  12,  11,  10,
    19,  18,  17,  16,  15,
    24,  23,  22,  21,  20
  };
CRGB leds[NUM_LEDS];

/* end led */

/* remote */
#include "Lpf2Hub.h"
Lpf2Hub myRemote;
byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
bool isRemoteInitialized = false;

/* end remote */

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  delay(1000);
  Serial.begin(9600);  
    
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
    if (! myRemote.isConnected()) scanRemoteController();
}

void init(){

   // led 
   initDisplay();

   //remote
   myRemote.init();

  
}


