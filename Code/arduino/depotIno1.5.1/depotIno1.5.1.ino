
// Questo sketch gestisce 3 treni (A,B,C) su un tracciato che va dal deposito alla galleria
// i treni partono una alla volta in maniera casuale
// utilizza lego poweredup con la libreria Legoino by Cornelius Munz  (https://github.com/corneliusmunz/legoino) ver 1.0.1
// necessita di 3 hub city per i treni (motore + sensore colore) e un hub technic (3 motori) per gli scambi.
// TrenIno4 - 2021 Code by Stefx

/* note
1) aumentare max device (default=3) in NimBLE-Arduino/src/nimconfig.h #define CONFIG_BT_NIMBLE_MAX_CONNECTIONS X
2) installare cp210x su linux
	apt list linux-modules-extra-5.8.0-38-generic
	dpkg -L linux-modules-extra-5.8.0-38-generic | grep cp210x
	sudo modprobe cp210x
*/

// TODO:
// leggere livello batteria e scambiare di conseguenza il binario
// implementare roulette prima di far partire treno (to check)

/* laed part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27
uint32_t SDM = 0; //Sampling delay in ms
uint32_t SN = 0; // Sampling number
int SM = 50;   // maximum sampling number


byte charStart = 0x30;
byte chargen[][5] = {
{0x0E, 0x13, 0x15, 0x19, 0x0E},  // 0
{0x00, 0x10, 0x1F, 0x12, 0x00},  // 1
{0x12, 0x15, 0x15, 0x15, 0x19},  // 2
{0x0A, 0x15, 0x15, 0x11, 0x11},  // 3
{0x1F, 0x04, 0x04, 0x04, 0x03},  // 4
{0x09, 0x15, 0x15, 0x15, 0x17},  // 5
{0x09, 0x15, 0x15, 0x15, 0x0E},  // 6
{0x03, 0x1D, 0x01, 0x01, 0x01},  // 7
{0x0A, 0x15, 0x15, 0x15, 0x0A},  // 8
{0x0E, 0x15, 0x15, 0x05, 0x02},  // 9
};

// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
uint32_t colour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow };

//rotate / flip
// https://macetech.github.io/FastLED-XY-Map-Generator/
const uint8_t XYTable[] = {
     4,   9,  14,  19,  24,
     3,   8,  13,  18,  23,
     2,   7,  12,  17,  22,
     1,   6,  11,  16,  21,
     0,   5,  10,  15,  20
  };

CRGB leds[NUM_LEDS];

/* end led part */

#include "Lpf2Hub.h"

String ver = "1.5.1";

// create a hub instance for train
Lpf2Hub myTrainHub_TA;
Lpf2Hub myTrainHub_TB;
Lpf2Hub myTrainHub_TC;
byte portA = (byte)PoweredUpHubPort::A;
byte portB = (byte)PoweredUpHubPort::B;
//int trainSpeed = 25;

int activeTrain = 0;
int colorInterval = 5000;
int beforeStartInterval = 5000;
int lastTrainStarted = -1;

// create a hub instance for switch
Lpf2Hub mySwitchController;
byte pPortC = (byte)ControlPlusHubPort::D; //0 -> A) White (D)
byte pPortD = (byte)ControlPlusHubPort::C; //1 -> B) Blue (C)
byte pPortA = (byte)ControlPlusHubPort::B; //2 -> C) Red (B) // battery shed
int switchInterval = 300;
int switchVelocity = 35;
int switchBatteryLevel = 100;
String switchControllerAddress = "90:84:2b:51:ba:b0";

// global
bool isSystemReady = false;
bool isVerbose = true;


// Trains structure
typedef struct {
  Lpf2Hub* hubobj;
  String hubColor;
  String hubAddress;
  int speed;
  int lastcolor;
  unsigned long colorPreviousMillis;
  int hubState;
  int trainState;
  int batteryLevel;
  char switchPosition[3];
  Color ledColor;
} Train;

// Switches structure
typedef struct {
  byte* port;
  String switchColor;
  bool switchState;
  bool switchInvert;
} Switches;


#define MY_TRAIN_LEN 3
#define MY_SWITCH_LEN 3
#define MY_COLOR_LEN 3

// exclude black (trail) 
// GREEN instead CYAN -> to add

// Color Maps
byte sensorAcceptedColors[MY_COLOR_LEN] = { (byte)Color::CYAN, (byte)Color::RED};  //  (byte)Color::BLUE,(byte)Color::YELLOW,(byte)Color::WHITE

// Trains Maps
//code  - hubobj - hubColor  -  hubAddress - speed - lastcolor - hubState (-1 = off, 0=ready, 1=active) - trainstate - batteryLevel - switchPosition
Train myTrains[MY_TRAIN_LEN] = {
  { &myTrainHub_TB, "Red",     "90:84:2b:1c:be:cf", 30 , 0, 0, -1, 0, 100, "01", RED}
  , { &myTrainHub_TC, "Green",   "90:84:2b:16:9a:1f", 30, 0, 0, -1, 0, 100, "00", GREEN}
  , { &myTrainHub_TA, "Yellow" , "90:84:2b:04:a8:c5", 45 , 0, 0, -1, 0, 100, "10", YELLOW}
};


// Switch Maps
//port  - color  -  status (0= straight 1= change) - invert
Switches mySwitchControlleres[MY_SWITCH_LEN] = {
  { &pPortC, "White" , 0, 0 }, //first switch
  { &pPortD, "Blu" , 0, 1}, // second switch
  { &pPortA, "Red" , 0, 0 } // battery shed switch
};


void printLegenda() {

  // print command
  Serial.println("*** M9Lab - TrenIno4 v." + ver + " ***");
  Serial.println("_________________________________________________");
  Serial.println("List of commands used by system:");

  Serial.println("on = set system on");
  Serial.println("off = set system off");
  Serial.println("panic = shutDown all hubs and reset the system");
  Serial.println("reset = reset the system");
  Serial.println("status = show system status");
  Serial.println("help = show this message again");
  Serial.println("verboseon = show more status messages");
  Serial.println("verboseoff = show less status messages");

  Serial.println("swa0 = set switch A straight");
  Serial.println("swa1 = set switch A turned");
  Serial.println("swb0 = set switch B straight");
  Serial.println("swb1 = set switch B turned");
  Serial.println("swc0 = set switch C straight");
  Serial.println("swc1 = set switch C turned");
  Serial.println("resetsw = reset all switches");

  Serial.println("_________________________________________________");
}

void readFromSerial() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    Serial.println(">" + command);
    if (command == "panic") panic();
    else if (command == "reset") systemReset();
    else if (command == "on") systemOn();
    else if (command == "off") systemOff();
    else if (command == "help") printLegenda();
    else if (command == "status") systemStatus();
    else if (command == "verboseon") verboseOn();
    else if (command == "verboseoff") verboseOff();

    else if (command == "swa0") setSwitch(&mySwitchControlleres[0], 0);
    else if (command == "swa1") setSwitch(&mySwitchControlleres[0], 1);
    else if (command == "swb0") setSwitch(&mySwitchControlleres[1], 0);
    else if (command == "swb1") setSwitch(&mySwitchControlleres[1], 1);
    else if (command == "swc0") setSwitch(&mySwitchControlleres[2], 0);
    else if (command == "swc1") setSwitch(&mySwitchControlleres[2], 1);
    else if (command == "resetsw") switchReset();
    else {
      Serial.println(">command not found");
    }
  }
}


void setSwitch(Switches *cSwitch, bool position) {

  byte *myPort = (byte *)cSwitch->port;

  // position 0=straight, 1= turned

  if (cSwitch->switchState == position) {
    _println("already setted");
    return;
  }

  bool p = position;
  if (cSwitch->switchInvert == 1 ) p = !position;    
  
  int velocity = p ? switchVelocity : (switchVelocity * -1);    

  mySwitchController.setTachoMotorSpeed(*myPort, velocity);
  delay(switchInterval);
  mySwitchController.stopTachoMotor(*myPort);
  cSwitch->switchState = position;

}

void switchReset() {
  for (int idSwitch = 0; idSwitch < MY_SWITCH_LEN; idSwitch++) {
    setSwitch(&mySwitchControlleres[idSwitch], 0);
  }
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
  //TODO
  systemOff();
  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    stopTrain(idTrain);
    myTrains[idTrain].speed = 0;
    myTrains[idTrain].lastcolor = 0;
    myTrains[idTrain].colorPreviousMillis = 0;
  }
  switchReset();
}

void panic() {
  //TODO
  systemReset();

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    // shutDown all Hub
    Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
    myTrain->shutDownHub();
    Serial.println("Disconnected from hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
  }
  activeTrain = 0;

  mySwitchController.shutDownHub();

}


// uso interno private
void systemStatus() {

  Serial.println("hubColor,batteryLevel,hubState,trainState,speed");
  Serial.println("_________________________________________________");

  for (int idTrain = 0; idTrain < MY_TRAIN_LEN; idTrain++) {
    Serial.println(myTrains[idTrain].hubColor + "," + myTrains[idTrain].batteryLevel + "," + myTrains[idTrain].hubState + "," +  myTrains[idTrain].trainState + "," + myTrains[idTrain].speed);
  }

  Serial.println("_________________________________________________");

  Serial.println("Switch Battery Level: ");
  Serial.println(switchBatteryLevel);

}

void hubButtonCallbackSwitch(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE) switchBatteryLevel = myHub->parseBatteryLevel(pData);
 
}

void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1) return;

  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE) myTrains[idTrain].batteryLevel = myHub->parseBatteryLevel(pData);      

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    ButtonState buttonState = myHub->parseHubButton(pData);
    if (buttonState == ButtonState::PRESSED)
    {
      //myHub->setBasicMotorSpeed(portA, 15);

      switch (myTrains[idTrain].hubState) {

        case 0: //ready -> active
          {

            Serial.println("Hub " + myTrains[idTrain].hubColor + " is ready");
            myTrains[idTrain].hubState = 1;            

            byte portForDevice = myHub->getPortForDeviceType((byte)DeviceType::COLOR_DISTANCE_SENSOR);
            if (portForDevice != 255) {
              // activate hub button to receive updates
              myHub->activatePortDevice(portB, colorDistanceSensorCallback);
              delay(200);
            }

            myHub->setLedColor(CYAN);

          }
          break;

        case 1: //active -> turnoff
          {
            myHub->setLedColor(RED);
            // disconnect color sensor
            //myHub->deactivatePortDevice(_portB, 37);
            delay(100);
            myHub->shutDownHub();
            Serial.println("Disconnected from hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
            
            myTrains[idTrain].hubState = -1;

          }
          break;

      }

    }
  }
}

// callback function to handle updates of sensor values
void colorDistanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1) return;
  if (myTrains[idTrain].hubState != 1) return;

  if (deviceType == DeviceType::COLOR_DISTANCE_SENSOR) {
    int color = myHub->parseColor(pData);

    if (myTrains[idTrain].lastcolor == color || !checkIfSensorColorIsAccepted(color) || color==0) return;
    myTrains[idTrain].lastcolor = color;

    
      Serial.print("Color ");
      Serial.print("Hub " + myTrains[idTrain].hubColor + ":");
      Serial.println(COLOR_STRING[color]);
      Serial.print("Color dec: ");
      Serial.println(color,DEC);
    

    myHub->setLedColor((Color)color);

    // set hub LED color to detected color of sensor and set motor speed dependent on color
    if (color == (byte)Color::RED) stopTrain(idTrain);
    else if (color == (byte)Color::WHITE) startTrain(idTrain);
    else if (color == (byte)Color::CYAN) stopAndDoTrain(idTrain, true); //GREEN
    else if (color == (byte)Color::YELLOW) stopAndDoTrain(idTrain, false);
    else if (color == (byte)Color::BLUE) invertTrain(idTrain);

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

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  _println("Start " + myTrains[idTrain].hubColor);
  _println("Train " + myTrains[idTrain].hubColor + " Battery Level: "  + myTrains[idTrain].batteryLevel);

  // TODO -> setta scambi per il ritorno e/o Battery Level (to check)
  for (int i = 0; i < strlen(myTrains[idTrain].switchPosition); i++ ) {
    bool c = (myTrains[idTrain].switchPosition[i]) == '1' ? true : false;
    setSwitch(&mySwitchControlleres[i], c);
  }

  mySwitchController.setLedColor(myTrains[idTrain].ledColor);

  //TEST: set speed depend by battery level
  //float f = (100 - myTrains[idTrain].batteryLevel )/ 5;
  //myTrains[idTrain].speed = (int) (trainSpeed + f);
  Serial.println(myTrains[idTrain].speed);

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

void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  _println("Invert " + myTrains[idTrain].hubColor);

  myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
}


void setup() {

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  
  FastLED.setBrightness(20);  
  delay(3000);
  Serial.begin(115200);
  printLegenda();

}

void loop() {

  readFromSerial();
  while (Serial.available() == 0) {

    //check for switch controller
    if (! mySwitchController.isConnected()) scanSwitchController();

    // check for train
	  activeTrain = 0;
    for (int i = 0; i < MY_TRAIN_LEN; i++) {
      checkIntervalisExpired(i);
      if (! myTrains[i].hubobj->isConnected()) {
  		  scanHub(i);
  	  } else{
  		  if (myTrains[i].hubState == 1) activeTrain++;
  	  }		  
    }
    
    if (isSystemReady) doMainCode();
  }

}

void doMainCode() {

  if (checkIfAllTrainIsStopped()) {
	  
    int randIdTrain = random(1, MY_TRAIN_LEN + 1) - 1;
    if (activeTrain > 1 && lastTrainStarted == randIdTrain) return;
	  Lpf2Hub *myTrain = myTrains[randIdTrain].hubobj;
    if (!myTrain->isConnected()) return;
  
	  rulette();  
    fullColor(colour[randIdTrain]);
    delay(1000);
    doCountdown(randIdTrain);
    fullColor(colour[randIdTrain]);
  
    lastTrainStarted = randIdTrain;
    startTrain(randIdTrain);  
    delay(beforeStartInterval);  
	  
  }

}

void scanSwitchController() {

  if (!mySwitchController.isConnected() && !mySwitchController.isConnecting()) mySwitchController.init(switchControllerAddress.c_str(), 1);

  if (mySwitchController.isConnecting()) {
    mySwitchController.connectHub();
    if (mySwitchController.isConnected()) {
      Serial.println("Connected to Switch Controller");
      char hubName[] = "Switch";
      mySwitchController.setHubName(hubName);
      mySwitchController.activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallbackSwitch);

    } else {
      Serial.println("Failed to connect to Switch Controller");
    }
  }

}

void scanHub( int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  if (!myTrain->isConnected() && !myTrain->isConnecting()) myTrain->init(myTrains[idTrain].hubAddress.c_str(), 1);

  if (myTrain->isConnecting()) {

    myTrain->connectHub();
    if (myTrain->isConnected()) {
      delay(500);
      Serial.println("Now connected with hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);

      // set the name
      char hubName[myTrains[idTrain].hubColor.length() + 1];
      myTrains[idTrain].hubColor.toCharArray(hubName, myTrains[idTrain].hubColor.length() + 1);
      myTrain->setHubName(hubName);

      // activate Property Update
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);
      delay(200);
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);
      delay(200);
      myTrain->setLedColor(PURPLE);
      myTrains[idTrain].hubState = 0;
      activeTrain++;

    } else {
      Serial.println("Failed to connect with hub " +  myTrains[idTrain].hubColor);
    }

  }
}

int getHubIdByAddress(String address) {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubAddress == address) return i;
  }
  return -1;
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


/* led part */

void doCountdown(int colorIndex){
 for(int num=9; num>-1; num--){
      
      char b[1];
      String str;
      str=String(num);
      str.toCharArray(b,1);
      osCopyChar(str[0],colorIndex);  
      delay(1000);  
   }  
}

void fullColor(uint32_t color){

 for(int num=0; num<NUM_LEDS; num++) {    
    leds[num] = color;
    FastLED.show();     
  }

}

void osCopyChar (char myChar, int colorIndex)
{
  myChar -= charStart;

  for (int i=0; i<5; i++)
    for (int j=0; j<5; j++)
    {
      uint32_t typex = (bitRead(chargen[myChar][i], j) == 0x00) ? CRGB::Black : colour[colorIndex];
      int pos = (i*5)+j;
      leds[XYTable[pos]] = typex;
      FastLED.show(); 
    }
}

void rulette(){
  Serial.println("rulette");  
  SN = 1;
  while (SN<SM) {
    float SDMex = (log10(864)/SM)*SN; //the exponent part --86400 is the number of sec in 1 day
    SDM = pow(10,SDMex); // the sampling exponential delay
    Serial.print("Sample No: ");
    Serial.print(SN);
    Serial.print("Color No: ");
    Serial.print(SN % 3);
    Serial.print("\tDelay: ");
    Serial.println(SDM);
    fullColor(colour[SN % 3]);
    delay(SDM);     
    SN++;  
  }
     
}
