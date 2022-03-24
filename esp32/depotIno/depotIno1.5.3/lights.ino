void startLights(Switches *cSwitch) {

  byte *myPort = (byte *)cSwitch->port;
  
  // always on
  //mySwitchController.setTachoMotorSpeed(*myPort, 50);
  
  // repeat 10 times (to test)
  for (int repeat = 0; repeat < 10; repeat++) {
	mySwitchController.setTachoMotorSpeedForTime(*myPort, 50,200);
  }		  
 
}

void stopLights(Switches *cSwitch) {
	
	byte *myPort = (byte *)cSwitch->port;
	mySwitchController.stopTachoMotor(*myPort);
	
	// lights_ison = false;
}

void startBlikLights() {
	lights_blink_ison=true;
}	

void stopBlikLights() {
	lights_blink_ison=false;
}	


void blinkLights(Switches *cSwitch){
	 byte *myPort = (byte *)cSwitch->port;
	 	 
	if (currentMillis_lights - previousMillis_lights >= interval_lights) {
    previousMillis = currentMillis;

    if (lights_ison) {
      mySwitchController.stopTachoMotor(*myPort);
    } else {
      mySwitchController.setTachoMotorSpeed(*myPort, 50);
    }

    
  }
}

