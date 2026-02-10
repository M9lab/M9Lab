
void printLegenda() {

  // print command lists using F() macro to save RAM
  Serial.print(F("*** M9Lab - DepotIno4 v."));
  Serial.print(ver);
  Serial.println(F(" ***"));
  Serial.println(F("_________________________________________________"));
  
  Serial.println(F("\n-----System commands:-----"));
  Serial.println(F("help = show this message again"));
  Serial.println(F("on = set automatic system on"));
  Serial.println(F("off = set automatic system off"));
  Serial.println(F("panic = shutDown all hubs and reset the system"));
  Serial.println(F("reset = reset the system"));
  Serial.println(F("sron = enable remote control"));
  Serial.println(F("sroff = disable remote control"));
  Serial.println(F("verboseon = show more status messages"));
  Serial.println(F("verboseoff = show less status messages"));
  
  Serial.println(F("\n-----Log commands:-----"));
  Serial.println(F("status = show system status"));
  Serial.println(F("autospeedon = set speed depends battery level on"));
  Serial.println(F("autospeedoff = set speed depends battery level off"));

  Serial.println(F("\n-----Switches commands:-----"));
  Serial.println(F("sw<X>0 = set switch X (a,b,c) straight"));
  Serial.println(F("sw<X>1 = set switch X (a,b,c) turned"));
  Serial.println(F("resetsw = reset all switches"));  
  Serial.println(F("killsw = kill the main hub"));
  Serial.println(F("sws+ = increase current switch motor speed to +5"));
  Serial.println(F("sws- = decrease current switch motor speed to -5"));
  Serial.println(F("sws= = reset current switch motor speed set to default (35)"));

  Serial.println(F("\n-----Trains commands:-----"));
  Serial.println(F("str1|stg1|sty1 = start RED,GREEN or YELLOW Train"));
  Serial.println(F("str0|stg0|sty0 = stop RED,GREEN or YELLOW Train"));
  Serial.println(F("killr|killg|killy = kill RED,GREEN or YELLOW Train"));
  Serial.println(F("killall = kill all Trains"));

  Serial.println(F("cts+ = increase current train speed set +5"));
  Serial.println(F("cts- = decrease current train speed set -5"));
  Serial.print(F("cts= = reset current train speed set to default ("));
  Serial.print(initialTrainSpeed);
  Serial.println(F(")"));

  Serial.println(F("_________________________________________________"));
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
    if (isVerbose) Serial.println(F("Cannot find the Switch Controller"));
  } else {
    if (isVerbose) Serial.println(F("Automatic mode is active"));
    isAutoEnabled = true;
  }
}

void systemOff() {
  if (isVerbose) Serial.println(F("Automatic mode is disabled"));
  isAutoEnabled = false;
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
    Serial.print(F("Disconnected from hub "));
    Serial.print(myTrains[idTrain].hubColor);
    Serial.print(F(" -> "));
    Serial.println(myTrains[idTrain].hubAddress);
  }
  activeTrain = 0;

  //shutDown remote
  if(myRemote.isConnected()) myRemote.shutDownHub();
  isRemoteInitialized = false; 
  
  //shutDown main hub
  killSwitch();

}


void systemStatus() {

  Serial.println(F("hubColor|batteryLevel|hubState|trainState|speed|"));
  Serial.println(F("------------------------------------------------"));

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    
    Serial.print(myTrains[idTrain].hubColor);
    Serial.print(F("|"));
    Serial.print(myTrains[idTrain].batteryLevel);
    Serial.print(F("|"));
    Serial.print(myTrains[idTrain].hubState);
    Serial.print(F("|"));
    Serial.print(myTrains[idTrain].trainState);
    Serial.print(F("|"));
    Serial.print(myTrains[idTrain].speed);
    Serial.println(F("|"));
  }

  Serial.println(F("------------------------------------------------"));

  Serial.print(F("Switch Battery Level: "));
  Serial.println(switchBatteryLevel);

  Serial.print(F("Switch Motor Speed: "));
  Serial.println(switchVelocity);

  Serial.print(F("Automatic mode on: "));
  Serial.println(isAutoEnabled);

  Serial.print(F("Automatic speed on: "));
  Serial.println(autoSpeedEnabled);  

}

void _print(const char* text) {
  if (isVerbose) Serial.print(text);
}

void _println(const char* text) {
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

void checkIntervalisExpired(int idTrain) {

  if (myTrains[idTrain].colorPreviousMillis == 0) return;
  
  unsigned long currentMillis = millis();
  if (currentMillis - myTrains[idTrain].colorPreviousMillis > colorInterval) {
    Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
    myTrains[idTrain].trainState = myTrains[idTrain].speed;
    myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
    myTrains[idTrain].colorPreviousMillis = 0;
  }

}
