void remoteCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData) {  
  if (deviceType == DeviceType::REMOTE_CONTROL_BUTTON) {    
    ButtonState buttonState = ((Lpf2Hub *)hub)->parseRemoteButton(pData);     
    remoteColorButtonController((byte)buttonState, (byte)portNumber);       
  }
}

void remoteColorButtonController(byte buttonState, byte portNumber) {
  if (!myRemote.isConnected()) return;  
   
  switch(portNumber) {
    case 0:
      switch(buttonState) {
        case 1: increaseCurrentTrainSpeed(); break;
        case 255: decreaseCurrentTrainSpeed(); break;
        case 127: stopTrain(currentActiveTrainOnRemote); break;
      }
      break;
    case 1:
      switch(buttonState) {
        case 1: setCurrentTrainNext(); break;
        case 255: setCurrentTrainPrev(); break;
        case 127: restoreRemoteStatus(); break;
      }
      break;
  }
}

void restoreRemoteStatus() {
  myRemote.setLedColor(PURPLE);      
  currentActiveTrainOnRemote = -1;
  refreshLed(2);
}

void scanRemoteController() {
  if(isRemoteInitialized) {
    checkRemoteIntervalisExpired();
    return;
  }

  if (!myRemote.isConnected()) {
    myRemote.init();
    remoteIsNotConnected();
    return;
  }

  if (myRemote.isConnecting() && myRemote.getHubType() == HubType::POWERED_UP_REMOTE) {
    if (!myRemote.connectHub()) {
      Serial.println("Unable to connect to hub");
      return;
    }
  }

  if (myRemote.isConnected()) {
    Serial.println("System is initialized");
    isRemoteInitialized = true;
    delay(200);
    myRemote.activatePortDevice(portLeft, remoteCallback);
    myRemote.activatePortDevice(portRight, remoteCallback);
    myRemote.activateHubPropertyUpdate(HubPropertyReference::BUTTON, remoteButtonCallback); 
    remoteIsConnected();
  }
}  

void remoteButtonCallback(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {
  if (hubProperty == HubPropertyReference::BUTTON && 
      ((Lpf2Hub *)hub)->parseHubButton(pData) == ButtonState::PRESSED) {
    killRemote();
  }
}

void remoteIsConnected() {
  myRemote.setLedColor(PURPLE);
  colorSquare(remote, CRGB::White, 0, 1);
  Serial.println("Remote " + String(myRemote.getHubAddress().toString().c_str()) + " is now connected.");
}

void remoteIsNotConnected() {    
  isRemoteInitialized = false;    
  currentActiveTrainOnRemote = -1;    
  colorSquare(remote, CRGB::Black, 0, 1);    
}

void killRemote() {
  remoteIsNotConnected();    
  myRemote.shutDownHub();
}

void setCurrentTrainNext() {
  if (getActiveTrain() < 1) return;    

  currentActiveTrainOnRemote = (currentActiveTrainOnRemote + 1) % MY_TRAIN_LEN;
  
  if (myTrains[currentActiveTrainOnRemote].hubState == 1) {          
    myRemote.setLedColor(myTrains[currentActiveTrainOnRemote].ledColor);
    saveInterval(remoteactivityMillis);                    
  } else {
    setCurrentTrainNext();
  }   
}

void setCurrentTrainPrev() {
  if (getActiveTrain() < 1) return;

  currentActiveTrainOnRemote = (currentActiveTrainOnRemote - 1 + MY_TRAIN_LEN) % MY_TRAIN_LEN;
  
  if (myTrains[currentActiveTrainOnRemote].hubState == 1) {          
    myRemote.setLedColor(myTrains[currentActiveTrainOnRemote].ledColor);
    saveInterval(remoteactivityMillis);                    
  } else {
    setCurrentTrainPrev();
  }   
}
