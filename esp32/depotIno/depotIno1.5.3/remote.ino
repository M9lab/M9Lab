void RemoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{

	Lpf2Hub *myRemote = (Lpf2Hub *)hub;  
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

void scanRemoteController()
{
  
	/* remote */      
     if (myRemote.isConnecting())
	  {
		if (myRemote.getHubType() == HubType::POWERED_UP_REMOTE)
		{
		  //This is the right device
		  if (!myRemote.connectHub())
		  {
			Serial.println("Unable to connect to hub");
		  }
		  else
		  {
			myRemote.setLedColor(GREEN);
			Serial.println("Remote connected.");
		  }
		}
	  }
	  
  
    if (myRemote.isConnected() && !isRemoteInitialized)
    {
        Serial.println("Remote is initialized");
		isRemoteInitialized = true;   		
		delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost           
		myRemote.activatePortDevice(portLeft, RemoteCallback);
		myRemote.activatePortDevice(portRight, RemoteCallback);    
		myRemote.setLedColor(GREEN);    
		isRemoteInitFirst = false;
        
    }
  
    if (! myRemote.isConnected() && ! isRemoteInitFirst){      
      myRemote.init();
      isRemoteInitialized = false; 
      isRemoteInitFirst = true;
    }
    
  
} // End of loop
