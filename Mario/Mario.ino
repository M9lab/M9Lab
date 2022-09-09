/**
 * 
 * A basic example which will connect to a Mario hub and request updates for 
 * the Pant, Color/Barcode and Gesture sensor. 
 *  
 * (c) Copyright 2020 - Cornelius Munz
 * Released under MIT License
 * 
 */

/* led part */
#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27

// ref: http://fastled.io/docs/3.1/struct_c_r_g_b.html
uint32_t colour[] = {CRGB::Red, CRGB::Green, CRGB::Yellow };

CRGB leds[NUM_LEDS];


#include "Lpf2Hub.h"

DeviceType pantSensor = DeviceType::MARIO_HUB_PANT_SENSOR;
DeviceType gestureSensor = DeviceType::MARIO_HUB_GESTURE_SENSOR;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;

bool isPantSensorInitialized = false;
bool isGestureSensorInitialized = false;
bool isBarcodeSensorInitialized = false;

// create a hub instance
Lpf2Hub myHub;

void MarioCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;

  if (deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR)
  {
    MarioColor color = myHub->parseMarioColor(pData);
    Serial.print("Mario Color: ");
    Serial.println((byte)color, HEX);
  }

}

void setup()
{
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  
  FastLED.setBrightness(20);  
  fullColor(CRGB::White);
  Serial.begin(115200);  
  myHub.init(); // initalize the MoveHub instance
}

// main loop
void loop()
{

  // connect flow. Search for BLE services and try to connect
  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to HUB");
    }
    else
    {
      Serial.println("Failed to connect to HUB");
    }
  }

  if (myHub.isConnected() && !isBarcodeSensorInitialized)
  {
    delay(200);
    Serial.print("check ports... if needed sensor is already connected: ");
    byte portForDevice = myHub.getPortForDeviceType((byte)barcodeSensor);
    Serial.println(portForDevice, DEC);
    if (portForDevice != 255)
    {
      Serial.println("activatePortDevice");
      myHub.activatePortDevice(portForDevice, MarioCallback);
      delay(200);
      isBarcodeSensorInitialized = true;
    };
  }  

} // End of loop


void fullColor(uint32_t color){

 for(int num=0; num<NUM_LEDS; num++) {    
    leds[num] = color;
    FastLED.show();     
  }

}
