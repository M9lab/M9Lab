void startLights(byte port) {  

  Serial.println("Lights ON");
  if (mySwitchController.isConnected()) mySwitchController.setTachoMotorSpeed(port, 50);    		  
}

void stopLights(byte port) {		

  Serial.println("Lights OFF");
	if (mySwitchController.isConnected()) mySwitchController.stopTachoMotor(port);		
}

void startBlikLights(byte port) {
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
      // stop after 10 blink
      if (lights_count>10) stopBlikLights(port);
    }

    
  }
}
