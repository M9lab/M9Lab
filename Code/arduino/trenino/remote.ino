void remoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData){

  

	Lpf2Hub *myRemote = (Lpf2Hub *)hub;  
	if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON){    
		ButtonState buttonState = myRemote->parseRemoteButton(pData);     
		remoteColorToLed((byte)buttonState,(byte)portNumber);       
  }
  
}

void remoteColorToLed( byte buttonState, byte portNumber){

  if (!myRemote.isConnected()) return;  
   
   if (portNumber == 0 && buttonState == 1){
      //increaseCurrentTrainSpeed();
   }
   if (portNumber == 0 && buttonState == 255){
      //decreaseCurrentTrainSpeed();
   }
   if (portNumber == 0 && buttonState == 127){
      //stopCurrentTrain();
   }
   if (portNumber == 1 && buttonState == 1){
      setCurrentTrainNext();
   }
   if (portNumber == 1 && buttonState == 255){
      setCurrentTrainPrev();
   }
   if (portNumber == 1 && buttonState == 127){
      killRemote();
   }
   
  
}

void scanRemoteController(){

  if (!myRemote.isConnected()){
    myRemote.init();
		remoteIsNotConnected();
		
  }

  if (myRemote.isConnecting()){
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

  if (myRemote.isConnected()  && !isRemoteInitialized){
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
    Serial.print("Remote ");
    Serial.print(myRemote.getHubAddress().toString().c_str());
    Serial.print(" is now connected.");    
}

void remoteIsNotConnected(){
    
    isRemoteInitialized = false;
    //colorSquare(remote,CRGB::Black,0,9);
}

void killRemote(){
  myRemote.shutDownHub();
  remoteIsNotConnected();
}

void setCurrentTrainNext(){

    if (activeTrain < 2) return;
    int startIndex = (currentActiveTrainOnRemote == -1 || currentActiveTrainOnRemote == (MY_TRAIN_LEN-1)) ? 0 : (currentActiveTrainOnRemote + 1);

    for (int i = startIndex; i < MY_TRAIN_LEN; i++) {
       
      if (myTrains[i].hubState == 1){
          currentActiveTrainOnRemote = i;
          refreshLed(2);   
          saveInterval(remoteactivityMillis);             
          return;        
      }

      if(i == (MY_TRAIN_LEN-1) && myTrains[MY_TRAIN_LEN-1].hubState != 1){
        currentActiveTrainOnRemote = 0;
        refreshLed(2);   
        saveInterval(remoteactivityMillis); 
      }       

    }        

}

void setCurrentTrainPrev(){

    if (activeTrain < 2) return;

    int startIndex = (currentActiveTrainOnRemote == -1 || currentActiveTrainOnRemote == 0) ? (MY_TRAIN_LEN-1) : (currentActiveTrainOnRemote - 1);
       
    for (int i = startIndex; i > -1; i--) {
      
      if (myTrains[i].hubState == 1){
          currentActiveTrainOnRemote = i;
          refreshLed(2);    
          saveInterval(remoteactivityMillis);                    
          return;      
      }   

      if(i == (0) && myTrains[0].hubState != 1){
        currentActiveTrainOnRemote = 3;
        refreshLed(2);   
        saveInterval(remoteactivityMillis); 
      }    
    }       
}

void refreshLed(int num){   
   // 0 = connected, 1= color, 2= selected, 3=battery
   
   for (int i = 0; i < MY_TRAIN_LEN; i++) {
    uint32_t color = CRGB::Black;
    if (num==0) color = myTrains[i].hubState == 1 ? maincolour[i] : CRGB::Black;
    if (num==2) color = currentActiveTrainOnRemote == i ? CRGB::White : maincolour[i];
    if (num==3) color = myTrains[i].batteryLevel > 10 ? maincolour[i] : CRGB::Purple;        
    colorSquare(myTrains[i].square,color,num,num+1);        
  }
}
