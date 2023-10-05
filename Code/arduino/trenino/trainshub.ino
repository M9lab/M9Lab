
void scanAllTrains(){

  activeTrain = 0;
  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    //checkIntervalisExpired(i);
    if (! myTrains[i].hubobj->isConnected()) {

      colorSquare(allsquares[i],maincolour[num],1,4);	
      //myTrains[i].hubAddress = "";
      myTrains[i].hubState = -1;
      myTrains[i].trainState = 0;
      
      scanHub(i);
      
    } else{
      if (myTrains[i].hubState == 1) activeTrain++;
    }     
    
  }
}

void scanHub( int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
 
  // read battery and update and update addr
  if (myTrain->isConnected()) {
     myTrains[idTrain].hubAddress = myTrains->getHubAddress().toString().c_str();
     myTrain->activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallback);
  }else{    
    if (!myTrain->isConnecting()) myTrain->init(myTrains[idTrain].hubAddress.c_str(), 1);
  }
  
  if (myTrain->isConnecting()) {

    myTrain->connectHub();
    if (myTrain->isConnected()) {
      delay(200);
      // init
      myTrains[idTrain].hubAddress = myTrains->getHubAddress().toString().c_str();
      myTrains[idTrain].hubState = 0;
      myTrains[idTrain].trainState = 0;
      Serial.println("Now connected with hub -> "  + myTrains[idTrain].hubAddress);
           
      // activate Property Update
      myTrain->activateHubPropertyUpdate(HubPropertyReference::BUTTON, hubButtonCallback);
      delay(200);
      //myTrain->setLedColor(PURPLE);
      
    } else {
      Serial.println("Failed to connect with hub n " + idTrain);
    }
    
  }
}


void hubButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

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
      // auto start ???
      //myHub->setBasicMotorSpeed(portA, 15);

      switch (myTrains[idTrain].hubState) {

        case 0: //ready -> active
        {

          Serial.println("Hub " + myTrains[idTrain].hubAddress + " is ready");
          myTrains[idTrain].hubState = 1;            
          activeTrain++;
          byte portForDevice = myHub->getPortForDeviceType((byte)DeviceType::COLOR_DISTANCE_SENSOR);
          if (portForDevice != 255) {
            // activate hub button to receive updates
            //myHub->activatePortDevice(portB, colorDistanceSensorCallback);
            delay(200);
          }

          myHub->setLedColor(CYAN);

        }
        break;

        case 1: //active -> turnoff
        {
          myHub->setLedColor(RED);                    
          delay(100);
          myHub->shutDownHub();
          Serial.println(myTrains[idTrain].hubAddress + " is disconnected");            
          
          myTrains[idTrain].hubState = -1;            
          //stopTrain(idTrain);            

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
      // stop | invert | kill -> sensorAcceptedColors
      //if (color == sensorAcceptedColors[0]) stopTrain(idTrain); 
      //else if (color == sensorAcceptedColors[1]) stopAndDoTrain(idTrain, true); 
      //else if (color == sensorAcceptedColors[2]) killTrain(idTrain);     

  }
}

bool checkIfSensorColorIsAccepted(byte inputColor) {
  for (int i = 0; i < MY_COLOR_LEN; i++) {
    if (sensorAcceptedColors[i] == inputColor) return true;
  }
  return false;
}
