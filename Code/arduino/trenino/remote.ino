void remoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData){

	Lpf2Hub *myRemote = (Lpf2Hub *)hub;  
	if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON){
		ButtonState buttonState = myRemote->parseRemoteButton(pData);     
		//remoteColorToLed((byte)buttonState,(byte)portNumber);       
  }
  
}

void remoteColorToLed( byte buttonState, byte portNumber){

  if (!myRemote.isConnected()) return;
  
   Serial.println("buttonState=" + buttonState);
   Serial.println("portNumber=" + portNumber);
   
  /* 
   if (portNumber == (byte)portLeft && buttonState == ButtonState::UP){
      increaseCurrentTrainSpeed();
   }
   if (portNumber == (byte)portLeft && buttonState == ButtonState::DOWN){
      decreaseCurrentTrainSpeed();
   }
   if (portNumber == (byte)portLeft && buttonState == ButtonState::STOP){
      stopCurrentTrain();
   }
   if (portNumber == (byte)portRight && buttonState == ButtonState::UP){
      setCurrentTrainNext();
   }
   if (portNumber == (byte)portRight && buttonState == ButtonState::DOWN){
      setCurrentTrainPrev();
   }
   if (portNumber == (byte)portRight && buttonState == ButtonState::STOP){
      killRemote();
   }
  */
}

void scanRemoteController(){
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
        remoteIsConnected();
      }
    }
  }


  if (!myRemote.isConnected())
  {
		remoteIsNotConnected();
		myRemote.init(1);
  }

  if (myRemote.isConnected()  && !isRemoteInitialized)
  {
    Serial.println("System is initialized");
    isRemoteInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
    remoteIsConnected();
  }
}  

void remoteIsConnected(){
    myRemote.setLedColor(PURPLE);
    colorSquare(remote,CRGB::White,0,9);
    Serial.println("Remote ");
    Serial.println(myRemote.getHubAddress().toString().c_str());
    Serial.println(" is now connected.");    
}

void remoteIsNotConnected(){
  isRemoteInitialized = false;
  colorSquare(remote,CRGB::Black,0,9);
}

void killRemote(){
  myRemote.shutDownHub();
  remoteIsNotConnected();
}

void setCurrentTrainNext(){

    if (activeTrain < 2) return;

    int startIndex = currentActiveTrainOnRemote = -1 || currentActiveTrainOnRemote== (MY_TRAIN_LEN-1) ? 0 : currentActiveTrainOnRemote;

    for (int i = currentActiveTrainOnRemote; i < MY_TRAIN_LEN; i++) {
      if (myTrains[i].hubState == 1){
          currentActiveTrainOnRemote = i;
          colorSquare(myTrains[i].square,CRGB::Purple,0,1);
          return;
      }       
    }        
    
}

void setCurrentTrainPrev(){

    if (activeTrain < 2) return;

    int startIndex = currentActiveTrainOnRemote = -1 || currentActiveTrainOnRemote == 0 ? (MY_TRAIN_LEN-1) : currentActiveTrainOnRemote;

    for (int i = currentActiveTrainOnRemote; i > -1; i--) {
      if (myTrains[i].hubState == 1){
          currentActiveTrainOnRemote = i;
          colorSquare(myTrains[i].square,CRGB::Purple,0,1);
          return;
      }       
    }   

}

