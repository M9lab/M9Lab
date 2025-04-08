
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

  _print("Stop & ");
  (invert) ? _print("Invert") : _print("Go");
  _println(" " + myTrains[idTrain].hubColor);

  if (myTrains[idTrain].colorPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].colorPreviousMillis);
    myTrain->stopBasicMotor(portA);

    if (invert) myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  }

}

void startTrain(int idTrain) {

  systemStatus();

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()){
    _println("Train " + myTrains[idTrain].hubColor + " is disconnected");
    return;
  }

  lastTrainStarted =idTrain;
   
  // avoid immediate stop by discarge stop color 
  myTrains[idTrain].lastcolor = sensorAcceptedColors[0];    
    
  _println("Start Train " + myTrains[idTrain].hubColor + " Battery Level: "  + myTrains[idTrain].batteryLevel);    

  //  set the switches ready for the trains return
  for (int i = 0; i < strlen(myTrains[idTrain].switchPosition); i++ ) {
    bool c = (myTrains[idTrain].switchPosition[i]) == '1' ? true : false;
    setSwitch(&mySwitchControlleres[i], c);
  }
  

  // Battery Level Switch   
  if( myTrains[idTrain].batteryLevel<trainBatteryLevelLimit){
    setSwitch(&mySwitchControlleres[2], 1);
  }else{
    setSwitch(&mySwitchControlleres[2], 0);    
  }

  mySwitchController.setLedColor(myTrains[idTrain].ledColor);

  if (autoSpeedEnabled){
    //TEST: set speed depends by battery level
    float newspeed = (int) ((100*myTrains[idTrain].speed) / myTrains[idTrain].batteryLevel);
    Serial.println("speed:");
    Serial.println(myTrains[idTrain].speed);
    Serial.println("newspeed:");
    Serial.println(newspeed);
    myTrains[idTrain].speed = newspeed;  
  }
  
  delayBlinkLights(pPortA);
  
  if (myTrains[idTrain].speed < 0) myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  _println("Train: " + myTrains[idTrain].hubColor + " Started!!!");    

}

void stopTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  
  if (!myTrain->isConnected()){
    _println("Train " + myTrains[idTrain].hubColor + " is disconnected");
    return;
  }
  
  _println("Stop " + myTrains[idTrain].hubColor);
  
  myTrain->stopBasicMotor(portA);
  myTrains[idTrain].trainState = 0;

}

void killTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  _println("Kill " + myTrains[idTrain].hubColor);
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
  
  _println("Invert " + myTrains[idTrain].hubColor);
  myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
}


void increaseCurrentTrainSpeed(){
  
  if (lastTrainStarted==-1) return;

   addspeed = myTrains[lastTrainStarted].speed < 0 ? -5 : +5; 
   myTrains[lastTrainStarted].speed =  myTrains[lastTrainStarted].speed + addspeed;
   
   Lpf2Hub *myTrain = myTrains[lastTrainStarted].hubobj;
   myTrain->setBasicMotorSpeed(portA, myTrains[lastTrainStarted].speed);
   
   _println("Train: " + myTrains[lastTrainStarted].hubColor + " speed now is " + myTrains[lastTrainStarted].speed); 

}

void decreaseCurrentTrainSpeed(){
  if (lastTrainStarted==-1) return;

  addspeed = myTrains[lastTrainStarted].speed < 0 ? +5 : -5; 
  myTrains[lastTrainStarted].speed =  myTrains[lastTrainStarted].speed + addspeed;
  
  Lpf2Hub *myTrain = myTrains[lastTrainStarted].hubobj;
   myTrain->setBasicMotorSpeed(portA, myTrains[lastTrainStarted].speed);
  _println("Train: " + myTrains[lastTrainStarted].hubColor + " speed now is " + myTrains[lastTrainStarted].speed); 

}

void resetCurrentTrainSpeed(){
  if (lastTrainStarted==-1) return;

  myTrains[lastTrainStarted].speed =  initialTrainSpeed;
  _println("Train: " + myTrains[lastTrainStarted].hubColor + " speed now is " + myTrains[lastTrainStarted].speed + " (default)"); 

}
