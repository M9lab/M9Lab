/*
TrenIno

TODO:
5) check distance/color

Matrix
---------
A(r)|B(y)
----+----
D(b)|C(g)
----^----
   usb

Pixel index
0,   1,   2,   3,   4,
5,   6,   7,   8,   9,
10,  11,  12,  13,  14,
15,  16,  17,  18,  19,
20,  21,  22,  23,  24

*/


// Global
String ver = "1.0.1";
bool isVerbose = true;

/* M5 Atom*/
#include "M5Atom.h"

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
int remote[] =  {12,2,7,17,22,10,11,13,14};

byte acol[][5] = {
  {0x1E, 0x05, 0x05, 0x05, 0x1E},  // A
  {0x0E, 0x11, 0x11, 0x11, 0x11},  // C
  {0x0E, 0x11, 0x11, 0x11, 0x0E},  // O
  {0x1F, 0x10, 0x10, 0x10, 0x10},  // L
};

byte trenino[][5] = {
  {0x01, 0x01, 0x1F, 0x01, 0x01},  // T
  {0x1F, 0x02, 0x01, 0x01, 0x02},  // r
  {0x0E, 0x15, 0x15, 0x15, 0x16},  // e
  {0x1F, 0x01, 0x01, 0x01, 0x1E},  // n
  {0x00, 0x00, 0x1F, 0x00, 0x00},  // I
  {0x1F, 0x01, 0x01, 0x01, 0x1E},  // n
  {0x0E, 0x11, 0x11, 0x11, 0x0E},  // o 
};

byte by[][5] = {
  {0x1F, 0x12, 0x12, 0x12, 0x0C},  // b
  {0x13, 0x14, 0x14, 0x14, 0x0F},  // y
};

byte M9Lab[][5] = {
  {0x1F, 0x02, 0x04, 0x02, 0x1F},  // M
  {0x02, 0x05, 0x15, 0x15, 0x0E},  // 9
  {0x1F, 0x10, 0x10, 0x10, 0x10},  // L
  {0x09, 0x15, 0x15, 0x15, 0x1E},  // a
  {0x1F, 0x12, 0x12, 0x12, 0x0C},  // b  
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
int colorInterval = 30000; //how much wait before start after train is waiting for a action (go or invert)
int invertInterval = 2000; //how much wait before start after train is waiting for direct invert

// default trains speed
int initialTrainSpeed = 20;
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
  unsigned long invertPreviousMillis;
  int connectAttempt;
} Train;


// Color Tile map for trains --> 1- YELLOW stop | 2- GREEN invert | 3-RED kill | 4- BLUE stopandgo |  5- WHITE stopandinvert
byte sensorAcceptedColors[MY_TILE_COLOR_LEN] = { (byte)Color::YELLOW, (byte)Color::GREEN, (byte)Color::RED, (byte) Color::BLUE, (byte)Color::WHITE };


// Trains Maps
// hubobj - hubColor  -  hubAddress - speed - lastcolor - colorPreviousMillis - hubState (-1 = off, 0=ready, 1=active) - trainstate (0 > tospeed) - batteryLevel -  ledColor - square - invertPreviousMillis
Train myTrains[MY_TRAIN_LEN] = {
    { &myTrainHub_TA, "Red",     "", initialTrainSpeed , 0, 0, -1, 0, 100,  RED,    squareA, 0, 0}
  , { &myTrainHub_TB, "Yellow" , "", initialTrainSpeed , 0, 0, -1, 0, 100,  YELLOW, squareB, 0, 0}
  , { &myTrainHub_TC, "Blue" ,   "", initialTrainSpeed , 0, 0, -1, 0, 100,  BLUE,   squareC, 0, 0}
  , { &myTrainHub_TD, "Green",   "", initialTrainSpeed , 0, 0, -1, 0, 100,  GREEN,  squareD, 0, 0}
  
};

//bool resetAddress = false;

/* end trains */


void setup() {
  
  M5.begin();
  delay(10);
  

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);
 
  Serial.begin(115200);  
    
  // led 
  initDisplay();
  delay(1000);
  printLegenda(); 
  
}

void loop() {
  
  readFromSerial();
  while (Serial.available() == 0) {      

    // remote controller    
	  scanRemoteController();    

    // trains
    scanAllTrains();

    if (M5.Btn.pressedFor(2000)) {    //If the button is pressed for more than 2 seconds
      Serial.println("pressedFor"); 
      fullColor(CRGB::Red);
      panic();
      delay(3000);
      initDisplay();
    } 

    // delay(50);
    M5.update();

  }
   
}
