
void randomStartTrain() {

  if (checkIfAllTrainIsStopped()) {
	  
    int randIdTrain = random(0, MY_TRAIN_LEN);
    
    if (activeTrain > 1 && lastTrainRandomStarted == randIdTrain) return;
	  Lpf2Hub *myTrain = myTrains[randIdTrain].hubobj;
    if (!myTrain->isConnected()) return;
  
	  rulette();  
    fullColor(colour[randIdTrain]);
    delay(1000);
    doCountdown(randIdTrain);
    fullColor(colour[randIdTrain]);

    lastTrainRandomStarted = randIdTrain;
  
    startTrain(randIdTrain);      
    delay(beforeStartInterval);      
	  
  }

}

void manualStartTrain(int idtrain){
  
	if (checkIfAllTrainIsStopped()) {
				
		Lpf2Hub *myTrain = myTrains[idtrain].hubobj;
		if (!myTrain->isConnected()) return;
		
		fullColor(colour[idtrain]);
		delay(1000);		
		lastTrainRandomStarted = -1;
    
		startTrain(idtrain);      
	}	
	
}

void stopAndDoTrain(int idTrain, bool invert) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;

  if (isVerbose) {
    Serial.print("Stop & ");
    Serial.print(invert ? "Invert" : "Go");
    Serial.print(" ");
    Serial.println(myTrains[idTrain].hubColor);
  }

  if (myTrains[idTrain].colorPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].colorPreviousMillis);
    myTrain->stopBasicMotor(portA);

    if (invert) myTrains[idTrain].speed = -myTrains[idTrain].speed;
  }

}

void startTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()){
    if (isVerbose) {
      Serial.print("Train ");
      Serial.print(myTrains[idTrain].hubColor);
      Serial.println(" is disconnected");
    }
    return;
  }

  lastTrainStarted = idTrain;
   
  // avoid immediate stop by discarge stop color 
  myTrains[idTrain].lastcolor = sensorAcceptedColors[0];    
    
  if (isVerbose) {
    Serial.print("Start Train ");
    Serial.print(myTrains[idTrain].hubColor);
    Serial.print(" Battery Level: ");
    Serial.println(myTrains[idTrain].batteryLevel);
  }

  //  set the switches ready for the trains return
  const char* switchPos = myTrains[idTrain].switchPosition;
  for (int i = 0; switchPos[i] != '\0'; i++) {
    bool c = (switchPos[i] == '1');
    setSwitch(&mySwitchControlleres[i], c);
  }
  
  // Battery Level Switch   
  if(myTrains[idTrain].batteryLevel < trainBatteryLevelLimit){
    setSwitch(&mySwitchControlleres[2], 1);
  }else{
    setSwitch(&mySwitchControlleres[2], 0);    
  }

  mySwitchController.setLedColor(myTrains[idTrain].ledColor);

  if (autoSpeedEnabled){
    //TEST: set speed depends by battery level
    int newspeed = (100 * myTrains[idTrain].speed) / myTrains[idTrain].batteryLevel;
    if (isVerbose) {
      Serial.print("speed: ");
      Serial.print(myTrains[idTrain].speed);
      Serial.print(" newspeed: ");
      Serial.println(newspeed);
    }
    myTrains[idTrain].speed = newspeed;  
  }
  
  delayBlinkLights(pPortA);
  
  if (myTrains[idTrain].speed < 0) myTrains[idTrain].speed = -myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
  if (isVerbose) {
    Serial.print("Train: ");
    Serial.print(myTrains[idTrain].hubColor);
    Serial.println(" Started!!!");
  }
}

void stopTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  
  if (!myTrain->isConnected()){
    if (isVerbose) {
      Serial.print("Train ");
      Serial.print(myTrains[idTrain].hubColor);
      Serial.println(" is disconnected");
    }
    return;
  }
  
  if (isVerbose) {
    Serial.print("Stop ");
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrain->stopBasicMotor(portA);
  myTrains[idTrain].trainState = 0;

}

void killTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  if (isVerbose) {
    Serial.print("Kill ");
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrain->stopBasicMotor(portA);
  myTrain->shutDownHub();
  myTrains[idTrain].trainState = 0;     
  myTrains[idTrain].hubState = -1;
  
  activeTrain--;
  setSwitch(&mySwitchControlleres[2], 0);
  delay(2000);

}

void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  if (isVerbose) {
    Serial.print("Invert ");
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrains[idTrain].speed = -myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
}


void increaseCurrentTrainSpeed(){
  
  if (lastTrainStarted == -1) return;

  addspeed = myTrains[lastTrainStarted].speed < 0 ? -5 : 5; 
  myTrains[lastTrainStarted].speed += addspeed;
  
  Lpf2Hub *myTrain = myTrains[lastTrainStarted].hubobj;
  myTrain->setBasicMotorSpeed(portA, myTrains[lastTrainStarted].speed);
  
  if (isVerbose) {
    Serial.print("Train: ");
    Serial.print(myTrains[lastTrainStarted].hubColor);
    Serial.print(" speed now is ");
    Serial.println(myTrains[lastTrainStarted].speed);
  }
}

void decreaseCurrentTrainSpeed(){
  if (lastTrainStarted == -1) return;

  addspeed = myTrains[lastTrainStarted].speed < 0 ? 5 : -5; 
  myTrains[lastTrainStarted].speed += addspeed;
  
  Lpf2Hub *myTrain = myTrains[lastTrainStarted].hubobj;
  myTrain->setBasicMotorSpeed(portA, myTrains[lastTrainStarted].speed);
  
  if (isVerbose) {
    Serial.print("Train: ");
    Serial.print(myTrains[lastTrainStarted].hubColor);
    Serial.print(" speed now is ");
    Serial.println(myTrains[lastTrainStarted].speed);
  }
}

void resetCurrentTrainSpeed(){
  if (lastTrainStarted == -1) return;

  myTrains[lastTrainStarted].speed = initialTrainSpeed;
  
  if (isVerbose) {
    Serial.print("Train: ");
    Serial.print(myTrains[lastTrainStarted].hubColor);
    Serial.print(" speed now is ");
    Serial.print(myTrains[lastTrainStarted].speed);
    Serial.println(" (default)");
  }
}
