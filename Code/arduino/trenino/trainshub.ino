void scanAllTrains() {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (!myTrains[i].hubobj->isConnected()) {
      scanHub(i, myTrains[i].hubAddress);        
      return;
    }     
  }
}

void scanHub(int idTrain, String hubA) {
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;  
 
  if (!myTrain->isConnected()) {           
    refreshLed(0);
    if (!myTrain->isConnecting()) {
      myTrain->init();            
    }
    return;
  }
  
  if (myTrain->isConnecting()) {
    if (myTrain->getHubType() == HubType::POWERED_UP_REMOTE) return;
    
    if (myTrain->connectHub()) {
      myTrain->setLedColor(PURPLE);                        
      String hb = myTrain->getHubAddress().toString().c_str();      
      delay(200);      
      
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);        
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);   

      myTrains[idTrain].hubAddress = hb;
      Serial.println("Now connected with hub -> " + hb);                                   
      refreshLed(0);           
      myTrains[idTrain].hubState = 0;
      myTrains[idTrain].trainState = 0;
      myTrains[idTrain].connectAttempt++;            
    }
  }
}

void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1) return;
    
  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE) {    
    myTrains[idTrain].batteryLevel = myHub->parseBatteryLevel(pData);      
    Serial.println("Train " + myTrains[idTrain].hubAddress + " Battery Level Updated: " + myTrains[idTrain].batteryLevel);
    return;
  }  

  if (hubProperty == HubPropertyReference::BUTTON && 
      myHub->parseHubButton(pData) == ButtonState::PRESSED) {
    
    switch (myTrains[idTrain].hubState) {
      case 0: // ready -> active
        activateHub(idTrain, myHub);
        break;

      case 1: // active -> turnoff
        deactivateHub(idTrain, myHub);
        break;
    }
  }
}

void activateHub(int idTrain, Lpf2Hub *myHub) {
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

  if(isRemoteInitialized) {
    stopAndDoTrain(idTrain, false);
  }
}

void deactivateHub(int idTrain, Lpf2Hub *myHub) {
  myHub->shutDownHub();
  Serial.println(myTrains[idTrain].hubAddress + " is now disconnected");                      
  myTrains[idTrain].hubState = -1;          
  activeTrain--;  
  myTrains[idTrain].hubAddress = "";
  refreshLed(0);                  
}

void colorDistanceSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {
  if (deviceType != DeviceType::COLOR_DISTANCE_SENSOR) return;

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  int idTrain = getHubIdByAddress(myHub->getHubAddress().toString().c_str());
  if (idTrain == -1 || myTrains[idTrain].hubState != 1) return;

  int color = myHub->parseColor(pData);
  double distance = myHub->parseDistance(pData);
	    
  if (myTrains[idTrain].lastcolor == color || 
      !checkIfSensorColorIsAccepted(color) || 
      color == 0 || 
      color == 255) return;

  myTrains[idTrain].lastcolor = color;
  
  Serial.println("Color Hub " + myTrains[idTrain].hubColor + ": " + 
                LegoinoCommon::ColorStringFromColor(color).c_str()); 
  Serial.println("Distance: " + String(distance, DEC));
                
  myHub->setLedColor((Color)color);
}

int getHubIdByAddress(String address) {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubAddress == address) return i;
  }
  return -1;
}

int getActiveTrain() {
  int ac = 0;
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubState == 1) ac++;
  }
  return ac;
}
