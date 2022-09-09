// AcolLed 1.0.2 -> Controlla i led atom matrix via Mario/Remote per illuminare una bacheca Lego
// utilizza lego poweredup con la libreria Legoino by Cornelius Munz  (https://github.com/corneliusmunz/legoino) ver 1.1.0
// AcolLed - 2021 Code by Stefx

/* note per 
  installare cp210x su linux
  apt list linux-modules-extra-5.8.0-38-generic
  dpkg -L linux-modules-extra-5.8.0-38-generic | grep cp210x
  sudo modprobe cp210x
*/

/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27
CRGB leds[NUM_LEDS];


#include "Lpf2Hub.h"
HubType typeD; //4 remote 7 mario
bool isRemoteInitialized = false;
bool isRemoteInitFirst = false;
uint32_t lastColor;

// create a hub instance
Lpf2Hub myRemote;

byte portLeft = (byte)PoweredUpRemoteHubPort::LEFT;
byte portRight = (byte)PoweredUpRemoteHubPort::RIGHT;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;

void DeviceCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  //Lpf2Hub *myMario = (Lpf2Hub *)hub;
  Lpf2Hub *myRemote = (Lpf2Hub *)hub;

  //Serial.println("DeviceCallback!!!");
  //Serial.println((byte)deviceType);

  if (deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR)
  {
    MarioColor color = myRemote->parseMarioColor(pData);
    //Serial.print("Mario Color: ");
    //Serial.println((byte)color);
    marioColorToLed((byte)color);    
  }
  
   if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON){
      ButtonState buttonState = myRemote->parseRemoteButton(pData);     
      remoteColorToLed((byte)buttonState,(byte)portNumber);
   }

}

void remoteColorToLed( byte buttonState, byte portNumber){
   
  if (buttonState==1 && portNumber == 0)  fullColor(CRGB::Blue);
  if (buttonState==255 && portNumber == 0)  fullColor(CRGB::Green);
  if (buttonState==1 && portNumber == 1)  fullColor(CRGB::Red);
  if (buttonState==255 && portNumber == 1)  fullColor(CRGB::Yellow);
  if (buttonState==127 && portNumber == 0)  fullColor(CRGB::White);
  if (buttonState==127 && portNumber == 1) {    
    myRemote.shutDownHub();
    isRemoteInitFirst = false;
  }
    
}

void marioColorToLed( byte color){
     
 switch (color) {

        case 23: 
          fullColor(CRGB::Blue);         
        break;

        case 37:           
          fullColor(CRGB::Green);                   
        break;

        case 21: 
          fullColor(CRGB::Red);                   
        break;

        case 24:           
          fullColor(CRGB::Yellow);                   
        break;           
  }        
  
}

void setup()
{
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);    
  fullColor(CRGB::White);
  //Serial.begin(115200);  

  // force shutdown
  /*
  if (myMario.isConnected())  myMario.shutDownHub();
  if (myRemote.isConnected())  myRemote.shutDownHub();
  */
  
  
  
  delay(200);
  //myRemote.init(); 
}

// main loop
void loop()
{

  /* remote */      
    if (myRemote.isConnecting())
    {

      typeD = myRemote.getHubType();
      //Serial.println("getHubType");
      //Serial.print((byte)typeD);

      // 4 remote
      // 7 mario

      // remote
      //if (myRemote.getHubType() == HubType::POWERED_UP_REMOTE)
      if ((byte)typeD == 4)
      {
       
        //This is the right device
        if (!myRemote.connectHub())
        {
          //Serial.println("Unable to connect to hub");
        }
        else
        {
          myRemote.setLedColor(GREEN);
          //Serial.println("Remote connected.");
        }
      }

      // mario
      if ((byte)typeD == 7){         
        //This is the right device
        if (!myRemote.connectHub())
        {
          //Serial.println("Unable to connect to hub");
        }
        else
        {          
          //Serial.println("Mario connected.");
        }
      }
    }
  
    if (myRemote.isConnected() && !isRemoteInitialized)
    {
            
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost      
      
      if ((byte)typeD == 7){
        byte portForDevice = myRemote.getPortForDeviceType((byte)barcodeSensor);
        //Serial.println(portForDevice);
        if (portForDevice != 255)
        {    
          myRemote.activatePortDevice(portForDevice, DeviceCallback);
          delay(200);
          isRemoteInitialized = true;          
          isRemoteInitFirst = false;
          //Serial.println("Mario is initialized");
        };
                      
      }  

      if ((byte)typeD==4){
        isRemoteInitialized = true;        
        //Serial.println("Remote is initialized");
        myRemote.activatePortDevice(portLeft, DeviceCallback);
        myRemote.activatePortDevice(portRight, DeviceCallback);    
        myRemote.setLedColor(GREEN);    
        isRemoteInitFirst = false;
      }  
    }
  
    if (! myRemote.isConnected() && ! isRemoteInitFirst){
      //if (!isMarioSensorInitialized) fullColor(CRGB::White); 
      myRemote.init();
      isRemoteInitialized = false; 
      isRemoteInitFirst = true;
    }
    
  
} // End of loop

void fullColor(uint32_t color){

  if (lastColor==color) return;
  
  FastLED.setBrightness(20);  
  fill_solid(leds, NUM_LEDS, color);  
  for (int i=0; i<20; i++){
    FastLED.setBrightness(i);   
    delay(75);
    FastLED.show();
  }   
  lastColor=color;

}
