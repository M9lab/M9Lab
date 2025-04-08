
void scanSwitchController() {

  if (!mySwitchController.isConnecting()){
    mySwitchController.init(switchControllerAddress.c_str(), 1);
    //mySwitchController.init();
    delay(1000);
  }else{  
    mySwitchController.connectHub();
    if (mySwitchController.isConnected()) {
      Serial.println("Connected to Switch Controller");
      char hubName[] = "Switch";
      mySwitchController.setHubName(hubName);
      mySwitchController.activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallbackSwitch);
      fullColor(CRGB::Cyan);

    } else {
      Serial.println("Failed to connect to Switch Controller");
    }
  }

}

void killSwitch(){
  mySwitchController.shutDownHub();
  fullColor(CRGB::Purple);
}

void hubButtonCallbackSwitch(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE) switchBatteryLevel = myHub->parseBatteryLevel(pData);
 
}
