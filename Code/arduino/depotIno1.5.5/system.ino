
#define LEGENDA_CMD_WIDTH 20   // comando + padding; l'"=" e' sempre in colonna LEGENDA_CMD_WIDTH+1

static void legendaCmd(const __FlashStringHelper* cmd, int len) {
  Serial.print(cmd);
  for (int i = len; i < LEGENDA_CMD_WIDTH; i++) Serial.print(' ');
  Serial.print(F("= "));   // "=" in colonna fissa, senza spazio prima
}

void printLegenda() {

  Serial.print(F("\n*** M9Lab - DepotIno v."));
  Serial.print(ver);
  Serial.println(F(" ***"));
  Serial.println(F("================================================="));

  Serial.println(F("\n--- System ---"));
  legendaCmd(F("help"), 4);       Serial.println(F("show this message again"));
  legendaCmd(F("on"), 2);        Serial.println(F("set automatic system on"));
  legendaCmd(F("off"), 3);       Serial.println(F("set automatic system off"));
  legendaCmd(F("panic"), 5);     Serial.println(F("shutDown all hubs and reset"));
  legendaCmd(F("reset"), 5);     Serial.println(F("reset the system"));
  legendaCmd(F("sron"), 4);     Serial.println(F("enable remote control"));
  legendaCmd(F("sroff"), 5);    Serial.println(F("disable remote control"));
  legendaCmd(F("verboseon"), 9); Serial.println(F("show more status messages"));
  legendaCmd(F("verboseoff"), 10); Serial.println(F("show less status messages"));
  Serial.flush();

  Serial.println(F("\n--- Status / Log ---"));
  legendaCmd(F("status"), 6);   Serial.println(F("show system status"));
  legendaCmd(F("autospeedon"), 11);  Serial.println(F("speed by battery level on"));
  legendaCmd(F("autospeedoff"), 12); Serial.println(F("speed by battery level off"));
  Serial.flush();

  Serial.println(F("\n--- Switches ---"));
  legendaCmd(F("swa0 swb0 swc0"), 14); Serial.println(F("set switch straight"));
  legendaCmd(F("swa1 swb1 swc1"), 14); Serial.println(F("set switch turned"));
  legendaCmd(F("resetsw"), 7);  Serial.println(F("reset all switches"));
  legendaCmd(F("killsw"), 6);   Serial.println(F("kill the main hub"));
  legendaCmd(F("sws+"), 4);     Serial.println(F("switch motor speed +5"));
  legendaCmd(F("sws-"), 4);     Serial.println(F("switch motor speed -5"));
  legendaCmd(F("sws="), 4);     Serial.println(F("switch motor speed default (35)"));
  Serial.flush();

  Serial.println(F("\n--- Trains ---"));
  legendaCmd(F("str1 stg1 sty1"), 14); Serial.println(F("start RED, GREEN, YELLOW"));
  legendaCmd(F("str0 stg0 sty0"), 14); Serial.println(F("stop RED, GREEN, YELLOW"));
  legendaCmd(F("killr killg killy"), 17); Serial.println(F("kill RED, GREEN, YELLOW"));
  legendaCmd(F("killall"), 7);  Serial.println(F("kill all trains"));
  legendaCmd(F("cts+"), 4);     Serial.println(F("train speed +5"));
  legendaCmd(F("cts-"), 4);     Serial.println(F("train speed -5"));
  legendaCmd(F("cts="), 4);     Serial.print(F("train speed default ("));
  Serial.print(initialTrainSpeed);
  Serial.println(F(")"));
  Serial.flush();

  Serial.println(F("\n--- Lights ---"));
  legendaCmd(F("sbl1"), 4);     Serial.println(F("start blinking lights"));
  legendaCmd(F("sbl0"), 4);     Serial.println(F("stop blinking lights"));

  Serial.println(F("=================================================\n"));
  Serial.flush();   // assicura che tutta la legenda sia inviata (evita troncamento)
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
    killTrain(idTrain, true);  // quick: delay ridotto
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


// Larghezze colonne come in intestazione (hubColor|batteryLevel|hubState|trainState|speed|)
#define STATUS_COL_COLOR 8
#define STATUS_COL_BATT  12
#define STATUS_COL_HUB   8
#define STATUS_COL_TRN   10
#define STATUS_COL_SPD   5

// Stampa intero allineato a destra in campo di larghezza w
static void printR(int val, int w) {
  int n = val < 0 ? -val : val;
  int d = (val < 0) ? 1 : 0;
  do { d++; n /= 10; } while (n);
  for (int i = d; i < w; i++) Serial.print(' ');
  Serial.print(val);
}

void systemStatus() {

  Serial.println(F("hubColor|batteryLevel|hubState|trainState|speed|"));
  Serial.println(F("--------+------------+--------+----------+-----+"));

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    const char* color = myTrains[idTrain].hubColor;
    int clen = 0;
    while (color[clen]) clen++;
    Serial.print(color);
    for (int i = clen; i < STATUS_COL_COLOR; i++) Serial.print(' ');
    Serial.print(F("|"));
    printR(myTrains[idTrain].batteryLevel, STATUS_COL_BATT);
    Serial.print(F("|"));
    printR(myTrains[idTrain].hubState, STATUS_COL_HUB);
    Serial.print(F("|"));
    printR(myTrains[idTrain].trainState, STATUS_COL_TRN);
    Serial.print(F("|"));
    printR(myTrains[idTrain].speed, STATUS_COL_SPD);
    Serial.println(F("|"));
  }

  Serial.println(F("--------+------------+--------+----------+-----+"));

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
