void startLights(byte myPort) {

  // always on
  //mySwitchController.setTachoMotorSpeed(*myPort, 50);
  
  // repeat 10 times (to test)
  for (int repeat = 0; repeat < 10; repeat++) {
	mySwitchController.setTachoMotorSpeedForTime(myPort, 50,200);
  }		  
 
}

void stopLights(byte myPort) {
	
	//byte *myPort = (byte *)cSwitch->port;
	mySwitchController.stopTachoMotor(myPort);
	
	// lights_ison = false;
}

void startBlikLights() {
	lights_blink_ison=true;
}	

void stopBlikLights() {
	lights_blink_ison=false;
}	


void blinkLights(byte myPort){
	 //byte *myPort = (byte *)cSwitch->port;
	 	 
	if (currentMillis_lights - previousMillis_lights >= interval_lights) {
    previousMillis_lights = currentMillis_lights;

    if (lights_ison) {
      mySwitchController.stopTachoMotor(myPort);
    } else {
      mySwitchController.setTachoMotorSpeed(myPort, 50);
    }

    
  }
}
