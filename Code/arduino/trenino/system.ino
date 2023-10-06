

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
	
    else {
      Serial.println("");
      Serial.println(">command not found");
    }
  }
}

void printLegenda() {

  // print command lists
  Serial.println("*** M9Lab - TrainIno v." + ver + " ***");
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
  }  
}

void panic() {
    
  systemReset();
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {

    myTrains[idTrain].trainState = 0;
    myTrains[idTrain].hubState = -1;    
    // shutDown Hub train
    killTrain(idTrain);
    Serial.println("Disconnected from hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
  }
  activeTrain = 0;

  //shutDown remote
  if(myRemote.isConnected()) myRemote.shutDownHub();
  isRemoteInitialized = false; 
  

}


void systemStatus() {

  Serial.println("hubColor|batteryLevel|hubState|trainState|speed|");
  Serial.println("------------------------------------------------");

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    
    _print_withspaces(myTrains[idTrain].hubColor,"hubColor");    
    _print_withspaces(String(myTrains[idTrain].batteryLevel),"batteryLevel");    
    _print_withspaces(String(myTrains[idTrain].hubState),"hubState");    
    _print_withspaces(String(myTrains[idTrain].trainState),"trainState");    
    _print_withspaces(String(myTrains[idTrain].speed),"speed");

    Serial.println("");
  }

  Serial.println("------------------------------------------------");

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


/* interval */

void saveInterval(unsigned long &previousMillis) {
  previousMillis = millis();
}


void checkIntervalisExpired(int idTrain ) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  if (millis() - myTrains[idTrain].colorPreviousMillis > colorInterval && myTrains[idTrain].colorPreviousMillis > 0) {
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].colorPreviousMillis = 0;
  }

}

bool checkIfSensorColorIsAccepted(byte inputColor) {
  for (int i = 0; i < MY_TILE_COLOR_LEN; i++) {
    if (sensorAcceptedColors[i] == inputColor) return true;
  }
  return false;
}
