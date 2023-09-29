
// This sketch manages max 8 trains (A, B, C) on a track reading color by sensor
// use lego poweredup with the Legoino library by Cornelius Munz (https://github.com/corneliusmunz/legoino) ver 1.1.0 
// MultiTrenIno - 2023 Code by Stefx

/* note
1) set max device to 8 (default=3) in NimBLE-Arduino/src/nimconfig.h #define CONFIG_BT_NIMBLE_MAX_CONNECTIONS X
2) install cp210x library on linux
	apt list linux-modules-extra-5.8.0-38-generic
	dpkg -L linux-modules-extra-5.8.0-38-generic | grep cp210x
	sudo modprobe cp210x
*/


// version
String ver = "1.0.1";

#include "Lpf2Hub.h"

// create a hub instance for train
Lpf2Hub myTrainHub_TA;
Lpf2Hub myTrainHub_TB;
Lpf2Hub myTrainHub_TC;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;

int activeTrain = 0;
int colorInterval = 35000; //how much wait before start after train is waiting for a action (go or invert)
int beforeStartInterval = 5000; //how much wait before start the train
int lastTrainStarted = -1;
int lastTrainRandomStarted = -1;
int unsigned addspeed =0;

// global flags
bool isAutoEnabled = false;
bool isVerbose = true;

// Trains structure
typedef struct {
  Lpf2Hub* hubobj;  
  String hubAddress;
  int speed;
  int lastcolor;
  unsigned long colorPreviousMillis;
  int hubState;
  int trainState;
  int batteryLevel;    
} Train;

#define MY_TRAIN_LEN 3
#define MY_COLOR_LEN 5

// default trains speed
int initialTrainSpeed = 25;

// Color Maps for trains --> 1- YELLOW stop | 2- GREEN invert | 3-RED kill | 4- BLUE stopandgo |  5- WHITE stopandinvert
byte sensorAcceptedColors[MY_COLOR_LEN] = { (byte)Color::YELLOW,  (byte)Color::GREEN, (byte)Color::RED, (byte)Color::BLUE, (byte)Color::WHITE};

// Trains Maps
// hubobj - hubAddress - speed - lastcolor - colorPreviousMillis - hubState (-1 = off, 0=ready, 1=active) - trainstate (0 > tospeed) - batterylevel
Train myTrains[MY_TRAIN_LEN] = {
    { &myTrainHub_TB, "", initialTrainSpeed , 0, 0, -1, 0, 100}
  , { &myTrainHub_TC, "", initialTrainSpeed , 0, 0, -1, 0, 100}
  , { &myTrainHub_TA, "", initialTrainSpeed , 0, 0, -1, 0, 100}
};



void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.println("");
    Serial.println(">" + command);
    Serial.println("");
    
    // system
    else if (command == "reset") systemReset();
    else if (command == "on") systemOn();
    else if (command == "off") systemOff();
    else if (command == "help") printLegenda();
    else if (command == "status") systemStatus();
    else if (command == "verboseon") verboseOn();
    else if (command == "verboseoff") verboseOff();
  	
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
  
    else {
      Serial.println("");
      Serial.println(">command not found");
    }
  }
}


void setup() {
  Serial.begin(115200);
  printLegenda();  
}

void loop() {

  //readFromSerial();
  while (Serial.available() == 0) {      

      // check for all trains
      activeTrain = 0;
      for (int i = 0; i < MY_TRAIN_LEN; i++) {
        //checkIntervalisExpired(i);
        if (! myTrains[i].hubobj->isConnected()) {

          myTrains[i].hubAddress = "";
          myTrains[idTrain].hubState = -1;
          myTrains[idTrain].trainState = 0;

         scanHub(i);
         
        } else{
          if (myTrains[i].hubState == 1) activeTrain++;
        }     
      }     
      
    }   
	
  }

}
