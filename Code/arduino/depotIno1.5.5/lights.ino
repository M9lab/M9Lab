void startLights(byte port) {    
  if (mySwitchController.isConnected()) mySwitchController.setTachoMotorSpeed(port, 20);    		  
}

void stopLights(byte port) {		
	if (mySwitchController.isConnected()) mySwitchController.stopTachoMotor(port);		
}

void startBlikLights(byte port) {
  _println("Start blinking lights");
	lights_blink_ison=true;
  lights_count = 0;
}	

void stopBlikLights(byte port) {
  lights_blink_ison=false;
  stopLights(port); 
  currentMillis_lights=0;
  previousMillis_lights=0;	
}	


void blinkLights(byte port){	

  currentMillis_lights = millis(); 
	if (currentMillis_lights - previousMillis_lights >= interval_lights) {
    
    previousMillis_lights = currentMillis_lights;
    lights_count++;

    if (lights_ison) {
      startLights(port);
      lights_ison=false;
    } else {
      stopLights(port);
      lights_ison=true;
      // stop after 8 blink
      if (lights_count>8) stopBlikLights(port);
    }
    
  }
}


// blink with normal delay
void delayBlinkLights(byte port){

  for (int i = 0; i < 4; i++ ) {
     startLights(port);
     delay(500);
     stopLights(port);
     delay(500);
  }

}
