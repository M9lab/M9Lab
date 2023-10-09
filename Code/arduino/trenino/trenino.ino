/*
TrainIno

Matrix

---------
A(r)|B(y)
----+----
D(b)|C(g)
----^----
   usb


0,   1,   2,   3,   4,
5,   6,   7,   8,   9,
10,  11,  12,  13,  14,
15,  16,  17,  18,  19,
20,  21,  22,  23,  24

*/


// Global
String ver = "1.0.1";
bool isVerbose = true;

/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27

// 0 = connected, 1= color, 2= selected, 3=battery
int squareA[] = {0,1,6,5};
int squareB[] = {4,3,8,9};
int squareC[] = {24,23,18,19};
int squareD[] = {20,21,16,15};
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
uint32_t maincolour[] = {CRGB::Red, CRGB::Yellow, CRGB::Teal, CRGB::Green};

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


/* remote */
#include "Lpf2Hub.h"
Lpf2Hub myRemote;
byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
bool isRemoteInitialized = false;
unsigned long remoteactivityMillis = 0;
int remoteInterval = 20000;

/* end remote */

/* trains */

#define MY_TILE_COLOR_LEN 5
#define MY_TRAIN_LEN 4

Lpf2Hub myTrainHub_TA;
Lpf2Hub myTrainHub_TB;
Lpf2Hub myTrainHub_TC;
Lpf2Hub myTrainHub_TD;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;

int activeTrain = 0;
int currentActiveTrainOnRemote = -1;
int colorInterval = 35000; //how much wait before start after train is waiting for a action (go or invert)
int beforeStartInterval = 5000; //how much wait before start the train

// default trains speed
int initialTrainSpeed = 25;
int speedIncreaseStep = 10;

// Trains structure
typedef struct {
  Lpf2Hub* hubobj;
  String hubColor;
  String hubAddress;
  int speed;
  int lastcolor;
  unsigned long colorPreviousMillis;
  int hubState;
  int trainState;
  int batteryLevel;  
  Color ledColor;
  int* square;
} Train;


// Color Tile map for trains --> 1- YELLOW stop | 2- GREEN invert | 3-RED kill | 4- BLUE stopandgo |  5- WHITE stopandinvert
byte sensorAcceptedColors[MY_TILE_COLOR_LEN] = { (byte)Color::YELLOW, (byte)Color::GREEN, (byte)Color::RED, (byte) Color::BLUE, (byte)Color::WHITE };
// other color not used: (byte)Color::BLUE, (byte)Color::WHITE

// Trains Maps
// hubobj - hubColor  -  hubAddress - speed - lastcolor - colorPreviousMillis - hubState (-1 = off, 0=ready, 1=active) - trainstate (0 > tospeed) - batteryLevel -  ledColor - square
Train myTrains[MY_TRAIN_LEN] = {
    { &myTrainHub_TA, "Red",     "", initialTrainSpeed , 0, 0, -1, 0, 100,  RED , squareA}
  , { &myTrainHub_TB, "Yellow" , "", initialTrainSpeed , 0, 0, -1, 0, 100,  YELLOW , squareB}
  , { &myTrainHub_TC, "Blue" ,   "", initialTrainSpeed , 0, 0, -1, 0, 100,  BLUE ,squareC}
  , { &myTrainHub_TD, "Green",   "", initialTrainSpeed , 0, 0, -1, 0, 100,  GREEN ,squareD}
  
};

/* end trains */



void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
  delay(1000);
  Serial.begin(115200);  
    
  init();

  printLegenda(); 

  // test remote(tutti treni collegati)
  //testRemote();
  
}

void loop() {

  readFromSerial();
  while (Serial.available() == 0) {      
    // remote controller    
	  scanRemoteController();
    checkRemoteIntervalisExpired();
    scanAllTrains();

  }

}

void init(){

   // led 
   initDisplay();

   //remote
   //scanRemoteController();
  
}

// only for test
void testRemote(){

  delay(3000);
  activeTrain = 0;
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    
      myTrains[i].hubState = 1;
      colorSquare(myTrains[i].square,maincolour[i],0,1);    
      activeTrain++;
      
  }
  

  /*    
  colorSquare(squareB,CRGB::Black,0,1);
  colorSquare(squareC,CRGB::Black,0,1);
  colorSquare(squareD,CRGB::Black,0,1);    
  delay(3000);
  colorSquare(squareA,CRGB::Red,0,1); 
  */ 

}
