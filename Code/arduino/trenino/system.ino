void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.println("");
    Serial.println(">" + command);
    Serial.println("");
    
    // system
    if (command == "panic") panic();
    else if (command == "reset") systemReset();
    else if (command == "help") printLegenda();
    else if (command == "status") systemStatus();
    else if (command == "verboseon") verboseOn();
    else if (command == "verboseoff") verboseOff();
    else if (command == "+") setCurrentTrainNext();
    else if (command == "-") setCurrentTrainPrev();
	
    else {
      Serial.println("");
      Serial.println(">command not found");
    }
  }
}

void printLegenda() {

  // print command lists
  Serial.println("*** M9Lab - TrenIno v." + ver + " ***");
  Serial.println("_________________________________________________");
  
  Serial.println("");
  Serial.println("-----System commands:-----");
  Serial.println("help = show this message again");
  Serial.println("panic = shutDown all trains and reset the system");
  Serial.println("reset = reset the system");
  
  Serial.println("");
  Serial.println("-----Log commands:-----");
  Serial.println("status = show system status");
  Serial.println("verboseon = show more status messages");
  Serial.println("verboseoff = show less status messages");

  Serial.println("");
  Serial.println("-----Remote commands (simulate):-----");
  Serial.println("+ = select next Train");
  Serial.println("- = select prev Train");  


  Serial.println("_________________________________________________");
}

void verboseOn() {
  isVerbose = true;
}

void verboseOff() {
  isVerbose = false;
}

void systemReset() {    
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    stopTrain(idTrain);
    myTrains[idTrain].speed = initialTrainSpeed;
    myTrains[idTrain].lastcolor = 0;
    myTrains[idTrain].colorPreviousMillis = 0;
    myTrains[idTrain].invertPreviousMillis = 0;
    myTrains[idTrain].trainState = 0;
    myTrains[idTrain].connectAttempt = 0;
  }  
}

void panic() {
    
  systemReset();
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {    

    killTrain(idTrain);
    myTrains[idTrain].hubState = -1;    
    // shutDown Hub train
    killTrain(idTrain);
    Serial.println("Disconnected from hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
  }
  activeTrain = 0;

  //shutDown remote
  if(myRemote.isConnected()){
    myRemote.shutDownHub();
    remoteIsNotConnected();  
  } 
    
}

void systemStatus() {

  Serial.println("hubColor|batteryLevel|hubState|trainState|speed|hubAddress|attempt");
  Serial.println("--------------------------------------------------------------------------");

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    
    _print_withspaces(myTrains[idTrain].hubColor,"hubColor");    
    _print_withspaces(String(myTrains[idTrain].batteryLevel),"batteryLevel");    
    _print_withspaces(String(myTrains[idTrain].hubState),"hubState");    
    _print_withspaces(String(myTrains[idTrain].trainState),"trainState");    
    _print_withspaces(String(myTrains[idTrain].speed),"speed");
    String addr = myTrains[idTrain].hubAddress != "" ? myTrains[idTrain].hubAddress : "-";
    //_print_withspaces(String(addr),"hubAddressSSSSS");
    _print_withspaces(String(myTrains[idTrain].connectAttempt),"attempt");

    Serial.println("");
  }

  Serial.println("--------------------------------------------------------------------------");

}

void _print_withspaces(String inputS, String lengthS){

  Serial.print(inputS);
  unsigned int nspaces = lengthS.length() - inputS.length();
  for(int i=0; i<nspaces; i++) Serial.print(" ");
  Serial.print("|");  

}

void _print(String text) {
  if (isVerbose) Serial.print(text);
}

void _println(String text) {
  if (isVerbose) Serial.println(text);
}


void saveInterval(unsigned long &previousMillis) {  
  previousMillis = millis();  
}

void checkRemoteIntervalisExpired(){    

  if (millis() - remoteactivityMillis > remoteInterval && remoteactivityMillis > 0) {
      restoreRemoteStatus();
  }

  if (millis() - remoteactivityMillis > 500 && remoteactivityMillis > 0) {
    refreshLed(2);
  }
}

void checkIntervalisExpired(int idTrain ) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  if (millis() - myTrains[idTrain].colorPreviousMillis > colorInterval && myTrains[idTrain].colorPreviousMillis > 0) {
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].colorPreviousMillis = 0;
  }

}

void checkInvertIntervalisExpired(int idTrain ) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  if (millis() - myTrains[idTrain].invertPreviousMillis > invertInterval && myTrains[idTrain].colorPreviousMillis > 0) {
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].invertPreviousMillis = 0;
  }

}

bool checkIfSensorColorIsAccepted(byte inputColor) {
  for (int i = 0; i < MY_TILE_COLOR_LEN; i++) {
    if (sensorAcceptedColors[i] == inputColor) return true;
  }
  return false;
}
