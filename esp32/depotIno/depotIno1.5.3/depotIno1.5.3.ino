
// Questo sketch gestisce 3 treni (A,B,C) su un tracciato che va dal deposito alla galleria
// i treni partono una alla volta in maniera casuale oppure manualmente (remote opzionale)
// utilizza lego poweredup con la libreria Legoino by Cornelius Munz  (https://github.com/corneliusmunz/legoino) ver 1.0.1
// necessita di 3 hub city per i treni (motore + sensore colore) e un hub technic (3 motori) per gli scambi.
// DepotIno - 2022 Code by Stefx

/* note
1) aumentare max device (default=3) in NimBLE-Arduino/src/nimconfig.h #define CONFIG_BT_NIMBLE_MAX_CONNECTIONS X
2) installare cp210x su linux
	apt list linux-modules-extra-5.8.0-38-generic
	dpkg -L linux-modules-extra-5.8.0-38-generic | grep cp210x
	sudo modprobe cp210x
*/

// TODO:
// aggiungere telecomando per partenza manuale (to check)
// luci in porta hub lampeggianti quando parte treno (to check)
// aumento velocità (to check)



/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27
uint32_t SDM = 0; //Sampling delay in ms
uint32_t SN = 0; // Sampling number
int SM = 50;   // maximum sampling number


byte charStart = 0x30;
byte chargen[][5] = {
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

/* end led part */

#include "Lpf2Hub.h"

// create a hub instance for train
Lpf2Hub myTrainHub_TA;
Lpf2Hub myTrainHub_TB;
Lpf2Hub myTrainHub_TC;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;

int activeTrain = 0;
int colorInterval = 5000;
int beforeStartInterval = 5000;
int lastTrainStarted = -1;

// create a hub instance for switch
Lpf2Hub mySwitchController;
byte pPortD = (byte)ControlPlusHubPort::D; //0 -> A) White (D)
byte pPortC = (byte)ControlPlusHubPort::C; //1 -> B) Blue (C)
byte pPortB = (byte)ControlPlusHubPort::B; //2 -> C) Red (B) // battery shed
byte pPortA = (byte)ControlPlusHubPort::A; // -> Lights
int switchInterval = 350;
int switchVelocity = 35;
int switchBatteryLevel = 100;
String switchControllerAddress = "90:84:2b:51:ba:b0";

// global
bool isSystemReady = false;
bool isVerbose = true;
String ver = "1.5.2.5";

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
  char switchPosition[3];
  Color ledColor;
} Train;

// Switches structure
typedef struct {
  byte* port;
  String switchColor;
  bool switchState;
  bool switchInvert;
} Switches;

#define MY_TRAIN_LEN 3
#define MY_SWITCH_LEN 3
#define MY_COLOR_LEN 3

int initialTrainSpeed = 25;

// Color Maps --> stop | invert | kill
byte sensorAcceptedColors[MY_COLOR_LEN] = { (byte)Color::CYAN,  (byte)Color::YELLOW, (byte)Color::RED, };
// other color (byte)Color::BLUE,(byte)Color::WHITE

// Trains Maps
// hubobj - hubColor  -  hubAddress - speed - lastcolor - colorPreviousMillis - hubState (-1 = off, 0=ready, 1=active) - trainstate (0 > tospeed) - batteryLevel - switchPosition - ledColor
Train myTrains[MY_TRAIN_LEN] = {
  { &myTrainHub_TB, "Red",     "90:84:2b:1c:be:cf", initialTrainSpeed , 0, 0, -1, 0, 100, "01", RED}
  , { &myTrainHub_TC, "Green",   "90:84:2b:16:9a:1f", initialTrainSpeed, 0, 0, -1, 0, 100, "00", GREEN}
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
//HubType typeD; //6 remote 7 mario
bool isRemoteInitialized = false;
//bool isRemoteInitFirst = false;
Lpf2Hub myRemote;
byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
String remoteAddress = "04:ee:03:b9:d8:19";
//DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;


/* lights */
unsigned long currentMillis_lights  = 0; 
unsigned long previousMillis_lights  = 0; 
const long interval_lights = 500;  
bool lights_blink_ison = false;
bool lights_ison = false;
int lights_count = 0;


void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.println(">" + command);
    
    // system
    if (command == "panic") panic();
    else if(command == "killsw") killSwitch();
    else if (command == "reset") systemReset();
    else if (command == "on") systemOn();
    else if (command == "off") systemOff();
    else if (command == "help") printLegenda();
    else if (command == "status") systemStatus();
    else if (command == "verboseon") verboseOn();
    else if (command == "verboseoff") verboseOff();

    // switches
    else if (command == "swa0") setSwitch(&mySwitchControlleres[0], 0);
    else if (command == "swa1") setSwitch(&mySwitchControlleres[0], 1);
    else if (command == "swb0") setSwitch(&mySwitchControlleres[1], 0);
    else if (command == "swb1") setSwitch(&mySwitchControlleres[1], 1);
    else if (command == "swc0") setSwitch(&mySwitchControlleres[2], 0);
    else if (command == "swc1") setSwitch(&mySwitchControlleres[2], 1);
	
    //lights
  	else if (command == "sbl1") startBlikLights(pPortA);
  	else if (command == "sbl0") stopBlikLights(pPortA);  	
  	
    // trains
  	else if (command == "str1") manualStartTrain(0);
  	else if (command == "stg1") manualStartTrain(1);
  	else if (command == "sty1") manualStartTrain(2);	

    else if (command == "str0") stopTrain(0);
    else if (command == "stg0") stopTrain(1);
    else if (command == "sty0") stopTrain(2);  

    else if (command == "killr") killTrain(0);
    else if (command == "killg") killTrain(1);
    else if (command == "killy") killTrain(2);  

    else if (command == "cts+") increaseCurrentTrainSpeed();  
    else if (command == "cts-") decreaseCurrentTrainSpeed();  
    else if (command == "cts=") resetCurrentTrainSpeed();  

    else if (command == "killall") for (int i = 0; i < MY_TRAIN_LEN; i++) { killTrain(i);}      
  
    // main hub
    else if (command == "resetsw") switchReset();
    else {
      Serial.println(">command not found");
    }
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
  while (Serial.available() == 0) {      
    //check for switch controller
    if (! mySwitchController.isConnected()){
      scanSwitchController();
    }else{

      // remote cotroller
      //if (! myRemote.isConnected()) scanRemoteController();

      // check for train
      activeTrain = 0;
      for (int i = 0; i < MY_TRAIN_LEN; i++) {
        checkIntervalisExpired(i);
        if (! myTrains[i].hubobj->isConnected()) {
          scanHub(i);
        } else{
          if (myTrains[i].hubState == 1) activeTrain++;
        }     
      }
    
      // do main code
      if (isSystemReady) doMainCode();    

      // light interval
      if (lights_blink_ison) blinkLights(pPortA);     
      
    }    
	
  }

}


void doMainCode() {

  if (checkIfAllTrainIsStopped()) {
	  
    int randIdTrain = random(0, MY_TRAIN_LEN - 1);
    if (activeTrain > 1 && lastTrainStarted == randIdTrain) return;
	  Lpf2Hub *myTrain = myTrains[randIdTrain].hubobj;
    if (!myTrain->isConnected()) return;
  
	  //rulette();  
    fullColor(colour[randIdTrain]);
    delay(1000);
    //doCountdown(randIdTrain);
    fullColor(colour[randIdTrain]);

    lastTrainStarted = randIdTrain;
  
     //startBlikLights(pPortA);
    delayBlinkLights(pPortA);
    startTrain(randIdTrain);      
    delay(beforeStartInterval);      
	  
  }

}
