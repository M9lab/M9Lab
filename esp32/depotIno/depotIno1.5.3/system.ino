
void printLegenda() {

  // print command lists
  Serial.println("*** M9Lab - DepotIno4 v." + ver + " ***");
  Serial.println("_________________________________________________");
  

  Serial.println("-----System commands:-----");
  Serial.println("help = show this message again");
  Serial.println("on = set automatic system to on");
  Serial.println("off = set automatic system to off");
  Serial.println("panic = shutDown all hubs and reset the system");
  Serial.println("reset = reset the system");
  

  Serial.println("-----Log commands:-----");
  Serial.println("status = show system status");
  Serial.println("verboseon = show more status messages");
  Serial.println("verboseoff = show less status messages");


  Serial.println("-----Switches commands:-----");
  Serial.println("swa0 = set switch A straight");
  Serial.println("swa1 = set switch A turned");
  Serial.println("swb0 = set switch B straight");
  Serial.println("swb1 = set switch B turned");
  Serial.println("swc0 = set switch C straight");
  Serial.println("swc1 = set switch C turned");  
  Serial.println("resetsw = reset all switches");  
  Serial.println("killsw = kill the main hub");


  Serial.println("-----Trains commands:-----");
  Serial.println("str1 = start RED Train");
  Serial.println("stg1 = start GREEN Train");
  Serial.println("sty1 = start YELLOW Train");

  Serial.println("str0 = stop RED Train");
  Serial.println("stg0 = stop GREEN Train");
  Serial.println("sty0 = stop YELLOW Train");

  Serial.println("killr = kill RED Train");
  Serial.println("killg = kill GREEN Train");
  Serial.println("killy = kill YELLOW Train");
  Serial.println("killall = kill all Trains");    

  Serial.println("cts+ = increase current train speed set +5");
  Serial.println("cts- = decrease current train speed set -5");
  Serial.println("cts+ = reset current train speed set to default" + initialTrainSpeed);

  
  Serial.println("-----Lights commands:-----");
  Serial.println("sbl1 = turn on blinking Lights");
  Serial.println("sbl0 = turn off blinking Lights");


  Serial.println("_________________________________________________");
}

void verboseOn() {
  isVerbose = true;
}

void verboseOff() {
  isVerbose = false;
}

void systemOn() {
  if (!mySwitchController.isConnected()) {
    _println("Cannot find the Switch Controller");
  } else {
    isSystemReady = true;
  }

}

void systemOff() {
  isSystemReady = false;
}

void systemReset() {
  
  systemOff();
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    stopTrain(idTrain);
    myTrains[idTrain].speed = initialTrainSpeed;
    myTrains[idTrain].lastcolor = 0;
    myTrains[idTrain].colorPreviousMillis = 0;
  }
  switchReset();
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
  
  //shutDown main hub
  killSwitch();

}


void systemStatus() {

  Serial.println("hubColor|batteryLevel|hubState|trainState|speed");
  Serial.println("_________________________________________________");

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    //Serial.println(myTrains[idTrain].hubColor + "|" + myTrains[idTrain].batteryLevel + "|" + myTrains[idTrain].hubState + "|" +  myTrains[idTrain].trainState + "|" + myTrains[idTrain].speed);
    
    _print_withspaces(myTrains[idTrain].hubColor,"hubColor");    
    _print_withspaces(String(myTrains[idTrain].batteryLevel),"batteryLevel");    
    _print_withspaces(String(myTrains[idTrain].hubState),"hubState");    
    _print_withspaces(String(myTrains[idTrain].trainState),"trainState");    
    _print_withspaces(String(myTrains[idTrain].speed),"speed");

    Serial.println("");
  }

  Serial.println("_________________________________________________");

  Serial.print("Switch Battery Level: ");
  Serial.println(switchBatteryLevel);

}

void _print_withspaces(String inputS, String lengthS){

  Serial.print(inputS);
  unsigned int nspaces = lengthS.length() - inputS.length();
  for(int i=0; i<nspaces-1; i++) Serial.print(" ");
  Serial.print("|");
  Serial.println();

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


bool checkIfAllTrainIsStopped() {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].trainState != 0) return false;
  }
  return true;
}


bool checkIfSensorColorIsAccepted(byte inputColor) {
  for (int i = 0; i < MY_COLOR_LEN; i++) {
    if (sensorAcceptedColors[i] == inputColor) return true;
  }
  return false;
}


void checkIntervalisExpired(int idTrain ) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  if (millis() - myTrains[idTrain].colorPreviousMillis > colorInterval && myTrains[idTrain].colorPreviousMillis > 0) {
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].colorPreviousMillis = 0;
  }

}
