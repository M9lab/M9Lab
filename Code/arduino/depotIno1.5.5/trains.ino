
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
    Serial.print(F("Stop & "));
    Serial.print(invert ? F("Invert") : F("Go"));
    Serial.print(F(" "));
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
      Serial.print(F("Train "));
      Serial.print(myTrains[idTrain].hubColor);
      Serial.println(F(" is disconnected"));
    }
    return;
  }

  lastTrainStarted = idTrain;
   
  // avoid immediate stop by discarge stop color 
  myTrains[idTrain].lastcolor = sensorAcceptedColors[0];    
    
  if (isVerbose) {
    Serial.print(F("Start Train "));
    Serial.print(myTrains[idTrain].hubColor);
    Serial.print(F(" Battery Level: "));
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

  if (autoSpeedEnabled && myTrains[idTrain].batteryLevel > 0) {
    // Speed scaled by battery level (evita divisione per zero)
    int newspeed = (100 * myTrains[idTrain].speed) / myTrains[idTrain].batteryLevel;
    if (isVerbose) {
      Serial.print(F("speed: "));
      Serial.print(myTrains[idTrain].speed);
      Serial.print(F(" newspeed: "));
      Serial.println(newspeed);
    }
    myTrains[idTrain].speed = newspeed;
  }
  
  delayBlinkLights(pPortA);
  
  if (myTrains[idTrain].speed < 0) myTrains[idTrain].speed = -myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
  if (isVerbose) {
    Serial.print(F("Train: "));
    Serial.print(myTrains[idTrain].hubColor);
    Serial.println(F(" Started!!!"));
  }
}

void stopTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  
  if (!myTrain->isConnected()){
    if (isVerbose) {
      Serial.print(F("Train "));
      Serial.print(myTrains[idTrain].hubColor);
      Serial.println(F(" is disconnected"));
    }
    return;
  }
  
  if (isVerbose) {
    Serial.print(F("Stop "));
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrain->stopBasicMotor(portA);
  myTrains[idTrain].trainState = 0;

}

void killTrain(int idTrain, bool quick) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  if (isVerbose) {
    Serial.print(F("Kill "));
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrain->stopBasicMotor(portA);
  myTrain->shutDownHub();
  myTrains[idTrain].trainState = 0;
  myTrains[idTrain].hubState = -1;

  activeTrain--;
  setSwitch(&mySwitchControlleres[2], 0);
  delay(quick ? 300 : 2000);  // breve se chiamato da panic
}

void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  if (isVerbose) {
    Serial.print(F("Invert "));
    Serial.println(myTrains[idTrain].hubColor);
  }
  
  myTrains[idTrain].speed = -myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
}


// Delta positivo = aumenta, negativo = diminuisce (rispetta il verso del treno). Clamp ±TRAIN_SPEED_MAX.
void changeCurrentTrainSpeed(int delta) {
  if (lastTrainStarted == -1) return;
  Lpf2Hub *myTrain = myTrains[lastTrainStarted].hubobj;
  int s = myTrains[lastTrainStarted].speed;
  addspeed = (s < 0) ? -delta : delta;
  s = s + addspeed;
  if (s > TRAIN_SPEED_MAX) s = TRAIN_SPEED_MAX;
  if (s < -TRAIN_SPEED_MAX) s = -TRAIN_SPEED_MAX;
  myTrains[lastTrainStarted].speed = s;
  myTrain->setBasicMotorSpeed(portA, s);
  if (isVerbose) {
    Serial.print(F("Train: "));
    Serial.print(myTrains[lastTrainStarted].hubColor);
    Serial.print(F(" speed now is "));
    Serial.println(s);
  }
}

void resetCurrentTrainSpeed(){
  if (lastTrainStarted == -1) return;

  myTrains[lastTrainStarted].speed = initialTrainSpeed;
  
  if (isVerbose) {
    Serial.print(F("Train: "));
    Serial.print(myTrains[lastTrainStarted].hubColor);
    Serial.print(F(" speed now is "));
    Serial.print(myTrains[lastTrainStarted].speed);
    Serial.println(F(" (default)"));
  }
}
