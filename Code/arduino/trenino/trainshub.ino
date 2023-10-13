void scanAllTrains(){

  activeTrain = 0;
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    //checkIntervalisExpired(i);
    //checkInvertIntervalisExpired(i);
    //if (! myTrains[i].hubobj->isConnected()) {
    if (myTrains[i].hubState == -1) {

      //colorSquare(allsquares[i],maincolour[i],1,4);	
      //myTrains[i].hubAddress = "";
      //myTrains[i].hubState = -1;
      //myTrains[i].trainState = 0;
      
      scanHub(i);
      return;
      
    } else{
      if (myTrains[i].hubState == 1) activeTrain++;
    }     
    
  }
}


void scanHub( int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;  
 
  // read battery and update and update addr
  if (! myTrain->isConnected()) {           
    refreshLed(0);
    if (!myTrain->isConnecting()){
      myTrain->init();       
    }
  }
  
  if (myTrain->isConnecting()) {
    
    if (myTrain->getHubType() == HubType::POWERED_UP_REMOTE) return;
    myTrain->connectHub();    
    if (myTrain->isConnected()) {
      myTrain->setLedColor(PURPLE);                        
      String hb = myTrain->getHubAddress().toString().c_str();
      int idTrainC = getHubIdByAddress(hb);          
      if (idTrainC != -1)idTrain = idTrainC;        
      refreshLed(0);     
      myTrains[idTrain].hubAddress = hb;
      myTrains[idTrain].hubState = 0;
      myTrains[idTrain].trainState = 0;
      myTrains[idTrain].connectAttempt = myTrains[idTrain].connectAttempt + 1;
      delay(200);      
      // activate Property Update
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);        
      //myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);        
      Serial.println("Now connected with hub -> "  + myTrains[idTrain].hubAddress);            
                 
    }
    
  }
}


void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  //Serial.println("hubButtonCallback");
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1) return;
    
  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE){    
    myTrains[idTrain].batteryLevel = myHub->parseBatteryLevel(pData);      
    Serial.println("Train " + myTrains[idTrain].hubAddress + " Battery Level Updated: "  + myTrains[idTrain].batteryLevel);
  }  

  if (hubProperty == HubPropertyReference::BUTTON)
  {
    ButtonState buttonState = myHub->parseHubButton(pData);  
    if (buttonState == ButtonState::PRESSED)
    {
      
      switch (myTrains[idTrain].hubState) {

        case 0: //ready -> active
        {

          Serial.println("Hub " + myTrains[idTrain].hubAddress + " is ready");
          myTrains[idTrain].hubState = 1;            
          activeTrain++;
          byte portForDevice = myHub->getPortForDeviceType((byte)DeviceType::COLOR_DISTANCE_SENSOR);
          if (portForDevice != 255) {
            
            // activate color distance sensor
            //myHub->activatePortDevice(portB, colorDistanceSensorCallback);
            
          }
          refreshLed(0);
          myHub->setLedColor(myTrains[idTrain].ledColor);

          // autostart -> go train ?
          if(isRemoteInitialized) stopAndDoTrain(idTrain, false);

        }
        break;

        case 1: //active -> turnoff
        {                    
          myHub->shutDownHub();
          Serial.println(myTrains[idTrain].hubAddress + " is now disconnected");                      
          myTrains[idTrain].hubState = -1;            
          refreshLed(0);                  
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
	  double distance = myHub->parseDistance(pData);
	    
    if (myTrains[idTrain].lastcolor == color || !checkIfSensorColorIsAccepted(color) || color==0 || color==255) return;

      myTrains[idTrain].lastcolor = color;
    
      Serial.print("Color ");
      Serial.print("Hub " + myTrains[idTrain].hubColor + ":");
      Serial.println(LegoinoCommon::ColorStringFromColor(color).c_str()); 

	    Serial.print("Distance: ");
	    Serial.println(distance, DEC);	                      
      myHub->setLedColor((Color)color);
    
      // set hub LED color to detected color of sensor and set motor speed dependent on color
      // Color Tile map for trains --> 0- YELLOW stop | 1- GREEN stopandinvert | 2-RED kill | 3- BLUE stopandgo |  4- WHITE invert
      //if (color == sensorAcceptedColors[0]) stopTrain(idTrain); 
      //else if (color == sensorAcceptedColors[1]) stopAndDoTrain(idTrain, true); 
      //else if (color == sensorAcceptedColors[2]) killTrain(idTrain);     
      //else if (color == sensorAcceptedColors[3]) stopAndDoTrain(idTrain, false);     
      //else if (color == sensorAcceptedColors[4]) invertTrain(idTrain, false);     

  }
}

int getHubIdByAddress(String address) {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubAddress == address) return i;
  }
  return -1;
}
