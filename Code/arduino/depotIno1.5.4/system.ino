
void printLegenda() {

  // print command lists
  Serial.println("*** M9Lab - DepotIno4 v." + ver + " ***");
  Serial.println("_________________________________________________");
  
  Serial.println("");
  Serial.println("-----System commands:-----");
  Serial.println("help = show this message again");
  Serial.println("on = set automatic system on");
  Serial.println("off = set automatic system off");
  Serial.println("panic = shutDown all hubs and reset the system");
  Serial.println("reset = reset the system");
  Serial.println("sron = enable remote control");
  Serial.println("sroff = disable remote control");
  Serial.println("verboseon = show more status messages");
  Serial.println("verboseoff = show less status messages");
  
  Serial.println("");
  Serial.println("-----Log commands:-----");
  Serial.println("status = show system status");
  Serial.println("autospeedon = set speed depends battery level on");
  Serial.println("autospeedoff = set speed depends battery level off");

  Serial.println("");
  Serial.println("-----Switches commands:-----");
  Serial.println("sw<X>0 = set switch X (a,b,c) straight");
  Serial.println("sw<X>1 = set switch X (a,b,c) turned");
  Serial.println("resetsw = reset all switches");  
  Serial.println("killsw = kill the main hub");
  Serial.println("sws+ = increase current switch motor speed to +5");
  Serial.println("sws- = decrease current switch motor speed to -5");
  Serial.println("sws= = reset current switch motor speed set to default (35)");

  Serial.println("");
  Serial.println("-----Trains commands:-----");
  Serial.println("str1|stg1|sty1 = start RED,GREEN or YELLOW Train");
  //Serial.println("stg1 = start GREEN Train");
  //Serial.println("sty1 = start YELLOW Train");

  Serial.println("str0|stg0|sty0 = stop RED,GREEN or YELLOW Train");
  //Serial.println("stg0 = stop GREEN Train");
  //Serial.println("sty0 = stop YELLOW Train");

  Serial.println("killr|killg|killy = kill RED,GREEN or YELLOW Train");
  //Serial.println("killg = kill GREEN Train");
  //Serial.println("killy = kill YELLOW Train");
  Serial.println("killall = kill all Trains");    

  Serial.println("cts+ = increase current train speed set +5");
  Serial.println("cts- = decrease current train speed set -5");
  Serial.print("cts= = reset current train speed set to default ");
  Serial.print("(");
  Serial.print(initialTrainSpeed);
  Serial.println(")");

  /*
  Serial.println("");
  Serial.println("-----Lights commands:-----");
  Serial.println("sbl1 = turn on blinking Lights");
  Serial.println("sbl0 = turn off blinking Lights");
  */

  Serial.println("_________________________________________________");
}

void verboseOn() {
  isVerbose = true;
}

void verboseOff() {
  isVerbose = false;
}

void autospeedOn() {
  autoSpeedEnabled = true;
}

void autospeedOff() {
  autoSpeedEnabled = false;
}

void systemOn() {
  if (!mySwitchController.isConnected()) {
    _println("Cannot find the Switch Controller");
  } else {
    _println("Automatic mode is active");
    isAutoEnabled = true;
  }
}

void systemOff() {
  _println("Automatic mode is disabled");
  isAutoEnabled = false;
}

void systemReset() {
  
  systemOff();
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    stopTrain(idTrain);
    myTrains[idTrain].speed = initialTrainSpeed;
    myTrains[idTrain].lastcolor = 0;
    myTrains[idTrain].colorPreviousMillis = 0;
    myTrains[idTrain].exitcount = 0;
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

  Serial.println("hubColor|batteryLevel|hubState|trainState|speed|out|");
  Serial.println("----------------------------------------------------");

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    
    _print_withspaces(myTrains[idTrain].hubColor,"hubColor");    
    _print_withspaces(String(myTrains[idTrain].batteryLevel),"batteryLevel");    
    _print_withspaces(String(myTrains[idTrain].hubState),"hubState");    
    _print_withspaces(String(myTrains[idTrain].trainState),"trainState");    
    _print_withspaces(String(myTrains[idTrain].speed),"speed");
    _print_withspaces(String(myTrains[idTrain].exitcount),"out");

    Serial.println("");
  }

  Serial.println("----------------------------------------------------");

  Serial.print("Switch Battery Level: ");
  Serial.println(switchBatteryLevel);

  Serial.print("Switch Motor Speed: ");
  Serial.println(switchVelocity);

  Serial.print("Automatic mode on: ");
  Serial.println(isAutoEnabled);

  Serial.print("Automatic speed on: ");
  Serial.println(autoSpeedEnabled);  

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

  if (!myTrain->isConnected()) return;

  if (millis() - myTrains[idTrain].colorPreviousMillis > colorInterval && myTrains[idTrain].colorPreviousMillis > 0) {
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].colorPreviousMillis = 0;
  }

}
