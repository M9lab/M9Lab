
void scanHub( int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

   
  if (!myTrain->isConnected() && !myTrain->isConnecting()){
     myTrain->init(myTrains[idTrain].hubAddress.c_str());
     delay(200);
  }else{
    
    if (myTrain->isConnecting()) {

      myTrain->connectHub();
      if (myTrain->isConnected()) {
        delay(200);
        Serial.println("Now connected with hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
  
        // set the name
        /*
        char hubName[myTrains[idTrain].hubColor.length() + 1];
        myTrains[idTrain].hubColor.toCharArray(hubName, myTrains[idTrain].hubColor.length() + 1);
        myTrain->setHubName(hubName);
        */
        
        myTrains[idTrain].trainState = 0;
        myTrains[idTrain].hubState = 0;
  
        // activate Property Update
        myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);      
        myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);
        
        myTrain->setLedColor(PURPLE);
        
      } else {
        Serial.println("Failed to connect with hub " +  myTrains[idTrain].hubColor);
      }
 
    }     
    
  }
}

void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1) return;  

  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE){    
    myTrains[idTrain].batteryLevel = myHub->parseBatteryLevel(pData);      
    //_println("Train " + myTrains[idTrain].hubColor + " Battery Level Updated: "  + myTrains[idTrain].batteryLevel);
  }  

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    ButtonState buttonState = myHub->parseHubButton(pData);
    if (buttonState == ButtonState::PRESSED)
    {
      // auto start
      //myHub->setBasicMotorSpeed(portA, 15);

      switch (myTrains[idTrain].hubState) {

        case 0: //ready -> active
          {

            Serial.println("Hub " + myTrains[idTrain].hubColor + " is ready");
            myTrains[idTrain].hubState = 1;            
            activeTrain++;
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
            //myHub->deactivatePortDevice(portB, colorDistanceSensorCallback);
            delay(100);
            myHub->shutDownHub();
            Serial.println("Disconnected from hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);
            
            myTrains[idTrain].hubState = -1;     
            activeTrain--;         
            stopTrain(idTrain);            

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
    
    if (myTrains[idTrain].lastcolor == color || !checkIfSensorColorIsAccepted(color) || color==0 || color==255) return;

      myTrains[idTrain].lastcolor = color;
    
      Serial.print("Color ");
      Serial.print("Hub " + myTrains[idTrain].hubColor + ":");
      //Serial.println(COLOR_STRING[color]);     
      Serial.println(LegoinoCommon::ColorStringFromColor(color).c_str()); 
    
      myHub->setLedColor((Color)color);
    
      // set hub LED color to detected color of sensor and set motor speed dependent on color
      // stop | invert | kill -> sensorAcceptedColors
      if (color == sensorAcceptedColors[0]) stopTrain(idTrain); 
      else if (color == sensorAcceptedColors[1]) stopAndDoTrain(idTrain, true); 
      else if (color == sensorAcceptedColors[2]) killTrain(idTrain); 
      
      //other functions not used
      /*
      startTrain(idTrain);
      stopAndDoTrain(idTrain, false);
      invertTrain(idTrain);
      */

  }
}

int getHubIdByAddress(String address) {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubAddress == address) return i;
  }
  return -1;
}
