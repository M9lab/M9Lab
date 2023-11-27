void remoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData){  

	Lpf2Hub *myRemote = (Lpf2Hub *)hub;  
	if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON){    
		ButtonState buttonState = myRemote->parseRemoteButton(pData);     
		remoteColorButtonController((byte)buttonState,(byte)portNumber);       
  }
  
}

void remoteColorButtonController( byte buttonState, byte portNumber){

  if (!myRemote.isConnected()) return;  
   
   if (portNumber == 0 && buttonState == 1){
      increaseCurrentTrainSpeed();
   }
   if (portNumber == 0 && buttonState == 255){
      decreaseCurrentTrainSpeed();
   }
   if (portNumber == 0 && buttonState == 127){
      stopTrain(currentActiveTrainOnRemote);
   }
   if (portNumber == 1 && buttonState == 1){
      setCurrentTrainNext();
   }
   if (portNumber == 1 && buttonState == 255){
      setCurrentTrainPrev();
   }
   if (portNumber == 1 && buttonState == 127){
        restoreRemoteStatus();
   }
     
}

void restoreRemoteStatus(){

  myRemote.setLedColor(PURPLE);      
  currentActiveTrainOnRemote = -1;
  refreshLed(2);
  
}

void scanRemoteController(){
  
  if(isRemoteInitialized) checkRemoteIntervalisExpired();

  if (!myRemote.isConnected()){
    myRemote.init();
		remoteIsNotConnected();		
  }

  if (myRemote.isConnecting()){
    if (myRemote.getHubType() == HubType::POWERED_UP_REMOTE)
    {
      //This is the right device
      if (!myRemote.connectHub()){
        Serial.println("Unable to connect to hub");
      }      
      
    }
  }

  if (myRemote.isConnected()  && !isRemoteInitialized){
    Serial.println("System is initialized");
    isRemoteInitialized = true;
    delay(200); //needed because otherwise the message is to fast after the connection procedure and the message will get lost
    // both activations are needed to get status updates
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
    myRemote.activateHubPropertyUpdate(HubPropertyReference::BUTTON, remoteButtonCallback); 
    remoteIsConnected();
  }

}  

void remoteButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  if (hubProperty == HubPropertyReference::BUTTON){
    ButtonState buttonState = myRemote.parseHubButton(pData);  
    if (buttonState == ButtonState::PRESSED) killRemote();                   
  }
  
}

void remoteIsConnected(){
    myRemote.setLedColor(PURPLE);
    colorSquare(remote,CRGB::White,0,1);
    Serial.print("Remote ");
    Serial.print(myRemote.getHubAddress().toString().c_str());
    Serial.print(" is now connected.");    
}

void remoteIsNotConnected(){    
    isRemoteInitialized = false;    
    currentActiveTrainOnRemote = -1;    
    colorSquare(remote,CRGB::Black,0,1);    
}

void killRemote(){
  remoteIsNotConnected();    
  myRemote.shutDownHub();
}

void setCurrentTrainNext(){

    if (getActiveTrain() < 1) return;    

    currentActiveTrainOnRemote++;    
    Serial.println(currentActiveTrainOnRemote);

    if (currentActiveTrainOnRemote == (MY_TRAIN_LEN)) currentActiveTrainOnRemote = 0;
     if (myTrains[currentActiveTrainOnRemote].hubState == 1){          
          myRemote.setLedColor(myTrains[currentActiveTrainOnRemote].ledColor);
          //refreshLed(2);    
          saveInterval(remoteactivityMillis);                    
          return;      
      }else{
        setCurrentTrainNext();
      }   

}

void setCurrentTrainPrev(){

    if (getActiveTrain() < 1) return;

    currentActiveTrainOnRemote--;    
    Serial.println(currentActiveTrainOnRemote);

    if (currentActiveTrainOnRemote == -1) currentActiveTrainOnRemote = MY_TRAIN_LEN-1;
     if (myTrains[currentActiveTrainOnRemote].hubState == 1){          
          myRemote.setLedColor(myTrains[currentActiveTrainOnRemote].ledColor);
          //refreshLed(2);    
          saveInterval(remoteactivityMillis);                    
          return;      
      }else{
        setCurrentTrainPrev();
      }   
 
}
