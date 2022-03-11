/**
 * A Legoino example to control a control plus hub
 * with a Motor connected on Port D
 * 
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */
 
 /* ver 1.2 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub mySwitchController;
byte pPortC = (byte)ControlPlusHubPort::C; //0 -> A) Black (C)
byte pPortD = (byte)ControlPlusHubPort::D; //1 -> B) Orange (D)
byte pPortA = (byte)ControlPlusHubPort::A; //2 -> C) White (A) // battery shed
int switchInterval = 250;
int switchVelocity = 35;
bool isVerbose = true;


typedef struct {
  byte* port;
  String switchColor;
  bool switchState;  
} Switches;


#define MY_SWITCH_LEN 3

//port  - color  -  status (0= straight 1= change) - vel_str - vel_change 
Switches mySwitchControlleres[MY_SWITCH_LEN] = {
  { &pPortC, "Black" , 0, }, //primo switch
  { &pPortD, "Orange" , 0,}, // secondo switch
  { &pPortA, "White" , 0, } // battery shed switch
};


void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
	  Serial.println(">" + command);
    if (command == "swa0") setSwitch(&mySwitchControlleres[0],0);
    else if(command == "swa1") setSwitch(&mySwitchControlleres[0],1);
    else if(command == "swb0") setSwitch(&mySwitchControlleres[1],0);
    else if(command == "swb1") setSwitch(&mySwitchControlleres[1],1);
    else if(command == "swc0") setSwitch(&mySwitchControlleres[2],0);
    else if(command == "swc1") setSwitch(&mySwitchControlleres[2],1);
	else if(command == "resetsw") resetSwitch();
	else{
		Serial.println(">command not found");
	}
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ready");
} 


// main loop
void loop() {
		

  if (!mySwitchController.isConnected() && !mySwitchController.isConnecting()) 
  {
    mySwitchController.init("90:84:2b:51:ba:b0"); 
  }

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (mySwitchController.isConnecting()) {
    mySwitchController.connectHub();
    if (mySwitchController.isConnected()) {
      Serial.println("Connected to HUB");
	  char hubName[] = "Switch";
	  mySwitchController.setHubName(hubName);
  	
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  // if connected, you can set the name of the hub, the led color and shut it down
  if (mySwitchController.isConnected()) {
	  	
	readFromSerial();        

  } else {
    Serial.println("ControlPlus hub is disconnected");
  }
  
} // End of loop


void setSwitch(Switches *cSwitch, bool position){	

  byte *myPort = (byte *)cSwitch->port;

  	// position 0=straight, 1= change
	
	 if(cSwitch->switchState == position){
		_println("already setted");
		return;
	 }
	 
	 int velocity = position ? switchVelocity : (switchVelocity * -1);  

  mySwitchController.setLedColor(YELLOW);    
	mySwitchController.setTachoMotorSpeed(*myPort, velocity);
	delay(switchInterval);
	mySwitchController.stopTachoMotor(*myPort);
  mySwitchController.setLedColor(BLUE);    
	
	cSwitch->switchState = position;
    
}



void _print(String text) {
  if (isVerbose) Serial.print(text);
}

void _println(String text) {
  if (isVerbose) Serial.println(text);
}

void resetSwitch() {
  for (int idSwitch = 0; idSwitch < MY_SWITCH_LEN; idSwitch++) {   
	  setSwitch(&mySwitchControlleres[idSwitch],0);  		
  }
}
