void startLights() {

  // always on
  mySwitchController.setTachoMotorSpeed(pPortA, 50);    		 
 
}

void stopLights() {
	
	//byte *myPort = (byte *)cSwitch->port;
	mySwitchController.stopTachoMotor(pPortA);
	
	// lights_ison = false;
}

void startBlikLights() {
	lights_blink_ison=true;
}	

void stopBlikLights() {
  lights_blink_ison=false;
  mySwitchController.stopTachoMotor(pPortA);
  currentMillis_lights=0;
  previousMillis_lights=0;	
}	


void blinkLights(){	

  currentMillis_lights = millis(); 

	if (currentMillis_lights - previousMillis_lights >= interval_lights) {
    
    previousMillis_lights = currentMillis_lights;

    if (lights_ison) {
      mySwitchController.stopTachoMotor(pPortA);
      lights_ison=false;
    } else {
      mySwitchController.setTachoMotorSpeed(pPortA, 50);
      lights_ison=true;
    }

    
  }
}
