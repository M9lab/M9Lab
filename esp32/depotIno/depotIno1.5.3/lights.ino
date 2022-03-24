void startLights(Switches *cSwitch) {

  byte *myPort = (byte *)cSwitch->port;
  
  // always on
  //mySwitchController.setTachoMotorSpeed(*myPort, 50);
  
  // repeat 10 times
  for (int repeat = 0; repeat < 10; repeat++) {
	mySwitchController.setTachoMotorSpeedForTime(*myPort, 50,200);
  }		
  
 
}

void stopLights(Switches *cSwitch) {
	
	byte *myPort = (byte *)cSwitch->port;
	mySwitchController.stopTachoMotor(*myPort);
}

