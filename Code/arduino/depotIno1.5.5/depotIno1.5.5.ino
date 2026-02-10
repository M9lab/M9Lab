
// This sketch manages 3 trains (A, B, C) on a track that goes from the depot to the tunnel
// trains depart one at a time randomly or manually (remotelly is optional)
// use lego poweredup with the Legoino library by Cornelius Munz (https://github.com/corneliusmunz/legoino) ver 1.1.0 (ex ver 1.0.1)
// needs 3 city hubs for trains (engine + color sensor) and a technic hub (3 engines) for exchanges and 1 kit lights.
// DepotIno - 2022 Code by Stefx

/* note
1) set max device (default=3) in C:\Users\stefano.ferrara\Documents\Arduino\libraries\NimBLE-Arduino\src\nimconfig.h #define CONFIG_BT_NIMBLE_MAX_CONNECTIONS X
2) install cp210x library on linux
	apt list linux-modules-extra-5.8.0-38-generic
	dpkg -L linux-modules-extra-5.8.0-38-generic | grep cp210x
	sudo modprobe cp210x
*/

//TEST: set speed depends by battery level

// version
const char* ver = "1.5.5.2";

/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27
uint32_t SDM = 0; //Sampling delay in ms
uint32_t SN = 0; // Sampling number
const int SM = 50;   // maximum sampling number


const byte charStart = 0x30;
const byte chargen[][5] PROGMEM = {
{0x0E, 0x13, 0x15, 0x19, 0x0E},  // 0
{0x00, 0x10, 0x1F, 0x12, 0x00},  // 1
{0x12, 0x15, 0x15, 0x15, 0x19},  // 2
{0x0A, 0x15, 0x15, 0x11, 0x11},  // 3
{0x1F, 0x04, 0x04, 0x04, 0x03},  // 4
{0x09, 0x15, 0x15, 0x15, 0x17},  // 5
{0x09, 0x15, 0x15, 0x15, 0x0E},  // 6
{0x03, 0x1D, 0x01, 0x01, 0x01},  // 7
{0x0A, 0x15, 0x15, 0x15, 0x0A},  // 8
{0x0E, 0x15, 0x15, 0x05, 0x02},  // 9
};

// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
const uint32_t colour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow };

//rotate / flip
// https://macetech.github.io/FastLED-XY-Map-Generator/
const uint8_t XYTable[] PROGMEM = {
     4,   9,  14,  19,  24,
     3,   8,  13,  18,  23,
     2,   7,  12,  17,  22,
     1,   6,  11,  16,  21,
     0,   5,  10,  15,  20
  };

CRGB leds[NUM_LEDS];

/* end led part */

#include "Lpf2Hub.h"

// create a hub instance for train
Lpf2Hub myTrainHub_TA;
Lpf2Hub myTrainHub_TB;
Lpf2Hub myTrainHub_TC;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;

int activeTrain = 0;
const unsigned long colorInterval = 35000; //how much wait before start after train is waiting for a action (go or invert)
const unsigned long beforeStartInterval = 5000; //how much wait before start the train
int lastTrainStarted = -1;
int lastTrainRandomStarted = -1;
unsigned int addspeed = 0;

// create a hub instance for switch
Lpf2Hub mySwitchController;
const byte pPortD = (byte)ControlPlusHubPort::D; //0 -> A) White (D)
const byte pPortC = (byte)ControlPlusHubPort::C; //1 -> B) Blue (C)
const byte pPortB = (byte)ControlPlusHubPort::B; //2 -> C) Red (B) // battery shed
const byte pPortA = (byte)ControlPlusHubPort::A; // -> Lights
const int switchInterval = 350;
int switchVelocity = 35;
int switchBatteryLevel = 100;
const char* switchControllerAddress = "90:84:2b:51:ba:b0";
const int trainBatteryLevelLimit = 10;

// global flags
bool isAutoEnabled = false;
bool isVerbose = true;
bool autoSpeedEnabled = false;

// Trains structure
typedef struct {
  Lpf2Hub* hubobj;
  const char* hubColor;
  const char* hubAddress;
  int speed;
  int lastcolor;
  unsigned long colorPreviousMillis;
  int hubState;
  int trainState;
  int batteryLevel;
  const char* switchPosition;
  Color ledColor;
} Train;

// Switches structure
typedef struct {
  const byte* port;
  const char* switchColor;
  bool switchState;
  bool switchInvert;
} Switches;

#define MY_TRAIN_LEN 3
#define MY_SWITCH_LEN 3
#define MY_COLOR_LEN 3

// default trains speed
const int initialTrainSpeed = 25;

// Color Maps for trains --> 1 stop | 2 invert | 3 kill
const byte sensorAcceptedColors[MY_COLOR_LEN] = { (byte)Color::YELLOW,  (byte)Color::GREEN, (byte)Color::RED};
// other color not used: (byte)Color::BLUE, (byte)Color::WHITE

// Trains Maps
// hubobj - hubColor  -  hubAddress - speed - lastcolor - colorPreviousMillis - hubState (-1 = off, 0=ready, 1=active) - trainstate (0 > tospeed) - batteryLevel - switchPosition - ledColor
Train myTrains[MY_TRAIN_LEN] = {
    { &myTrainHub_TB, "Red",     "90:84:2b:1c:be:cf", initialTrainSpeed , 0, 0, -1, 0, 100, "01", RED}
  , { &myTrainHub_TC, "Green",   "90:84:2b:16:9a:1f", initialTrainSpeed , 0, 0, -1, 0, 100, "00", GREEN}
  , { &myTrainHub_TA, "Yellow" , "90:84:2b:04:a8:c5", initialTrainSpeed , 0, 0, -1, 0, 100, "10", YELLOW}
};

// Switch Maps
//port  - color  -  status (0= straight 1= change) - invert
Switches mySwitchControlleres[MY_SWITCH_LEN] = {
  { &pPortD, "White" , 0, 0 }, //first switch
  { &pPortC, "Blu" , 0, 1}, // second switch
  { &pPortB, "Red" , 0, 0 } // battery shed switch
};


/* remote part */
bool isRemoteInitialized = false;
Lpf2Hub myRemote;
const byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
const byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
const char* remoteAddress = "04:ee:03:b9:d8:19";
bool isRemoteActive = false;

/* lights */
unsigned long currentMillis_lights  = 0; 
unsigned long previousMillis_lights  = 0; 
const unsigned long interval_lights = 500;  
bool lights_blink_ison = false;
bool lights_ison = false;
int lights_count = 0;


// Helper function for system commands
bool handleSystemCommands(const String &cmd) {
  if (cmd == "panic") { panic(); return true; }
  if (cmd == "killsw") { killSwitch(); return true; }
  if (cmd == "reset") { systemReset(); return true; }
  if (cmd == "on") { systemOn(); return true; }
  if (cmd == "off") { systemOff(); return true; }
  if (cmd == "help") { printLegenda(); return true; }
  if (cmd == "status") { systemStatus(); return true; }
  if (cmd == "autospeedon") { autospeedOn(); return true; }
  if (cmd == "autospeedoff") { autospeedOff(); return true; }
  if (cmd == "sron") { setRemoteOn(); return true; }
  if (cmd == "sroff") { setRemoteOff(); return true; }
  if (cmd == "verboseon") { verboseOn(); return true; }
  if (cmd == "verboseoff") { verboseOff(); return true; }
  return false;
}

// Helper function for switch commands
bool handleSwitchCommands(const String &cmd) {
  if (cmd == "swa0") { setSwitch(&mySwitchControlleres[0], 0); return true; }
  if (cmd == "swa1") { setSwitch(&mySwitchControlleres[0], 1); return true; }
  if (cmd == "swb0") { setSwitch(&mySwitchControlleres[1], 0); return true; }
  if (cmd == "swb1") { setSwitch(&mySwitchControlleres[1], 1); return true; }
  if (cmd == "swc0") { setSwitch(&mySwitchControlleres[2], 0); return true; }
  if (cmd == "swc1") { setSwitch(&mySwitchControlleres[2], 1); return true; }
  if (cmd == "sws+") { increaseSwitchSpeed(); return true; }
  if (cmd == "sws-") { decreaseSwitchSpeed(); return true; }
  if (cmd == "sws=") { resetSwitchSpeed(); return true; }
  if (cmd == "resetsw") { switchReset(); return true; }
  return false;
}

// Helper function for train commands
bool handleTrainCommands(const String &cmd) {
  if (cmd == "str1") { manualStartTrain(0); return true; }
  if (cmd == "stg1") { manualStartTrain(1); return true; }
  if (cmd == "sty1") { manualStartTrain(2); return true; }
  if (cmd == "str0") { stopTrain(0); return true; }
  if (cmd == "stg0") { stopTrain(1); return true; }
  if (cmd == "sty0") { stopTrain(2); return true; }
  if (cmd == "killr") { killTrain(0); return true; }
  if (cmd == "killg") { killTrain(1); return true; }
  if (cmd == "killy") { killTrain(2); return true; }
  if (cmd == "cts+") { increaseCurrentTrainSpeed(); return true; }
  if (cmd == "cts-") { decreaseCurrentTrainSpeed(); return true; }
  if (cmd == "cts=") { resetCurrentTrainSpeed(); return true; }
  if (cmd == "killall") { 
    for (int i = 0; i < MY_TRAIN_LEN; i++) { killTrain(i); }
    return true;
  }
  return false;
}

// Helper function for lights commands
bool handleLightsCommands(const String &cmd) {
  if (cmd == "sbl1") { startBlikLights(pPortA); return true; }
  if (cmd == "sbl0") { stopBlikLights(pPortA); return true; }
  return false;
}

void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    Serial.print("\n>");
    Serial.print(command);
    Serial.print("\n\n");
    
    // Try each command category
    if (handleSystemCommands(command)) return;
    if (handleSwitchCommands(command)) return;
    if (handleTrainCommands(command)) return;
    if (handleLightsCommands(command)) return;
    
    // Command not found
    Serial.println("\n>command not found");
  }
}

void setup() {

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  
  FastLED.setBrightness(20);  
  delay(3000);
  Serial.begin(115200);
  printLegenda();  
  fullColor(CRGB::Purple);

}

void loop() {

  readFromSerial();
  
  //check for switch controller
  if (! mySwitchController.isConnected()){
    scanSwitchController();
  }else{

    // remote controller
    if (isRemoteActive && ! myRemote.isConnected()) scanRemoteController();

    // check for all trains     
    for (int i = 0; i < MY_TRAIN_LEN; i++) {        
      if (! myTrains[i].hubobj->isConnected()) {
       scanHub(i);
      } else{
        checkIntervalisExpired(i);
      }     
    }
  
    // do the automatic train start is activated
    if (isAutoEnabled) randomStartTrain();    

    // check bliking light interval
    //if (lights_blink_ison) blinkLights(pPortA);     
    
  }

}
