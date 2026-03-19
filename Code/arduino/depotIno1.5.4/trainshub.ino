
void scanHub( int idTrain) {

  delay(75);  // dà respiro allo stack NimBLE tra un hub e l'altro (evita abort)
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;

  //battery update
  /*
  if (myTrain->isConnected()) {
     myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);
  }
  */

  if (!myTrain->isConnected() && !myTrain->isConnecting()) myTrain->init(myTrains[idTrain].hubAddress.c_str());

  if (myTrain->isConnecting()) {

    myTrain->connectHub();
    if (myTrain->isConnected()) {
      delay(200);
      Serial.println("Now connected with hub " + myTrains[idTrain].hubColor + " -> "  + myTrains[idTrain].hubAddress);

      // set the name
      
      char hubName[myTrains[idTrain].hubColor.length() + 1];
      myTrains[idTrain].hubColor.toCharArray(hubName, myTrains[idTrain].hubColor.length() + 1);
      //myTrain->setHubName(hubName);
      
      
      myTrains[idTrain].trainState = 0;
      myTrains[idTrain].hubState = 0;

      // activate Property Update
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);      
      delay(50);
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);
      
      myTrain->setLedColor(PURPLE);
      
    } else {
      //Serial.println("Failed to connect with hub " +  myTrains[idTrain].hubColor);
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
              delay(50);
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
            myTrains[idTrain].colorConsecutiveCount = 0;
            myTrains[idTrain].colorConsecutiveValue = -1;
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
    if (!checkIfSensorColorIsAccepted(color) || color == 0 || color == 255) return;

    // require COLOR_CONFIRM_COUNT consecutive same readings before acting
    int &count = myTrains[idTrain].colorConsecutiveCount;
    int &value = myTrains[idTrain].colorConsecutiveValue;
    if (color == value)
      count++;
    else {
      value = color;
      count = 1;
    }

    myHub->setLedColor((Color)color);

    if (count < COLOR_CONFIRM_COUNT) return;

    // confirmed: 2 consecutive same color
    count = 0;
    value = -1;
    myTrains[idTrain].lastcolor = color;

    Serial.print("Color ");
    Serial.print("Hub " + myTrains[idTrain].hubColor + ":");
    Serial.println(LegoinoCommon::ColorStringFromColor(color).c_str());

    // stop | invert | kill -> sensorAcceptedColors
    if (color == sensorAcceptedColors[0]) stopTrain(idTrain);
    else if (color == sensorAcceptedColors[1]) stopAndDoTrain(idTrain, true);
    else if (color == sensorAcceptedColors[2]) pendingKillTrain = idTrain;
  }
}

int getHubIdByAddress(String address) {
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    if (myTrains[i].hubAddress == address) return i;
  }
  return -1;
}
