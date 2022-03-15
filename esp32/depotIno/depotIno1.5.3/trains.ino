
void manualStartTrain(int idtrain){

  
	if (checkIfAllTrainIsStopped()) {
				
		Lpf2Hub *myTrain = myTrains[idtrain].hubobj;
		if (!myTrain->isConnected()) return;
		
		fullColor(colour[idtrain]);
		delay(1000);		
		lastTrainStarted = -1;
    
		startTrain(idtrain);      
	}	
	
}




void stopAndDoTrain(int idTrain, bool invert) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

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

  //(evito stop immediato)
  myTrains[idTrain].lastcolor = (byte)Color::RED;  
    
  _println("Start " + myTrains[idTrain].hubColor);
  _println("Train " + myTrains[idTrain].hubColor + " Battery Level: "  + myTrains[idTrain].batteryLevel);  
  

  // TODO -> setta scambi per il ritorno
  for (int i = 0; i < strlen(myTrains[idTrain].switchPosition); i++ ) {
    bool c = (myTrains[idTrain].switchPosition[i]) == '1' ? true : false;
    setSwitch(&mySwitchControlleres[i], c);
  }
  

  //   Battery Level Switch   
  if( myTrains[idTrain].batteryLevel<10){
    setSwitch(&mySwitchControlleres[2], 1);
  }else{
    setSwitch(&mySwitchControlleres[2], 0);    
  }

  mySwitchController.setLedColor(myTrains[idTrain].ledColor);

  //TEST: set speed depend by battery level
  //float f = (100 - myTrains[idTrain].batteryLevel )/ 5;
  //myTrains[idTrain].speed = (int) (trainSpeed + f);
  //Serial.println(myTrains[idTrain].speed);

  if (myTrains[idTrain].speed < 0) myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  _println("Train: " + myTrains[idTrain].hubColor + " Started!!!");


}

void stopTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  _println("Stop " + myTrains[idTrain].hubColor);
  myTrain->stopBasicMotor(portA);
  myTrains[idTrain].trainState = 0;

}

void killTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  _println("Kill " + myTrains[idTrain].hubColor);
  myTrain->stopBasicMotor(portA);
  myTrain->shutDownHub();
  myTrains[idTrain].trainState = 0;     
  myTrains[idTrain].hubState = -1;
  
  //activeTrain--;
  setSwitch(&mySwitchControlleres[2], 0);
  delay(2000);

}

void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  _println("Invert " + myTrains[idTrain].hubColor);

  myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
}
