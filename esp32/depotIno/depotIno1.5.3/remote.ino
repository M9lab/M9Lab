void RemoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  //Lpf2Hub *myMario = (Lpf2Hub *)hub;
  Lpf2Hub *myRemote = (Lpf2Hub *)hub;

  /*
  if (deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR)
  {
    MarioColor color = myRemote->parseMarioColor(pData);
    marioColorToLed((byte)color);    
  }
  */
  
   if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON){
      ButtonState buttonState = myRemote->parseRemoteButton(pData);     
      remoteColorToLed((byte)buttonState,(byte)portNumber);
   }

}

void remoteColorToLed( byte buttonState, byte portNumber){
   
  if (buttonState==1 && portNumber == 0)  panic(); //blue
  if (buttonState==255 && portNumber == 0)  manualStartTrain(1); //green
  if (buttonState==1 && portNumber == 1)  manualStartTrain(0); //red 
  if (buttonState==255 && portNumber == 1) manualStartTrain(2); //yellow
  if (buttonState==127 && portNumber == 0)  fullColor(CRGB::White);
  if (buttonState==127 && portNumber == 1) {    
    myRemote.shutDownHub();
    isRemoteInitFirst = false;
    Serial.println("Remote disconnected.");
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




void scanRemoteController()
{

  
  /* remote */      
    if (myRemote.isConnecting())
    {

      typeD = myRemote.getHubType();      
      if ((byte)typeD == 4)
      {
       
        //This is the right device
        if (!myRemote.connectHub())
        {
          Serial.println("Unable to connect to Remote");
        }
        else
        {
          myRemote.setLedColor(GREEN);
          Serial.println("Remote connected.");
        }
      }

      // mario
      if ((byte)typeD == 7){         
        //This is the right device
        if (!myRemote.connectHub())
        {
          Serial.println("Unable to connect to Mario");
        }
        else
        {          
          Serial.println("Mario connected.");
        }
      }
    }
  
    if (myRemote.isConnected() && !isRemoteInitialized)
    {
            
      delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost      
      /*      
      if ((byte)typeD == 7){
        byte portForDevice = myRemote.getPortForDeviceType((byte)barcodeSensor);        
        if (portForDevice != 255)
        {    
          myRemote.activatePortDevice(portForDevice, RemoteCallback);
          delay(200);
          isRemoteInitialized = true;          
          isRemoteInitFirst = false;
          Serial.println("Mario is initialized");
        };
                      
      } 
      */ 

      if ((byte)typeD==4){
        isRemoteInitialized = true;        
        Serial.println("Remote is initialized");
        myRemote.activatePortDevice(portLeft, RemoteCallback);
        myRemote.activatePortDevice(portRight, RemoteCallback);    
        myRemote.setLedColor(GREEN);    
        isRemoteInitFirst = false;
      }  
    }
  
    if (! myRemote.isConnected() && ! isRemoteInitFirst){      
      myRemote.init();
      isRemoteInitialized = false; 
      isRemoteInitFirst = true;
    }
    
  
} // End of loop
