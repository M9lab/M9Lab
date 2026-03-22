void scanSwitchController() {

  if (!mySwitchController.isConnecting()){
    mySwitchController.init(switchControllerAddress.c_str(), 1);
    delay(1000);
  } else {
    mySwitchController.connectHub();
    if (mySwitchController.isConnected()) {
      Serial.println("Connected to Switch Controller");
      fullColor(CRGB::Cyan);
      // defer BLE setup to avoid assert (interrupt_hlevel_disable from wrong core)
      pendingSwitchSetup = true;
      switchConnectDoneMillis = millis();
    } else {
      Serial.println("Failed to connect to Switch Controller");
    }
  }
}

void doSwitchPostConnectSetup() {
  if (!mySwitchController.isConnected()) return;
  char hubName[] = "Switch";
  mySwitchController.setHubName(hubName);
  mySwitchController.activateHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, hubButtonCallbackSwitch);
}

void runSwitchPostConnectIfNeeded() {
  if (!pendingSwitchSetup) return;
  if (millis() - switchConnectDoneMillis < SWITCH_POST_CONNECT_DELAY_MS) return;
  pendingSwitchSetup = false;
  doSwitchPostConnectSetup();
}

void killSwitch(){
  mySwitchController.shutDownHub();
  fullColor(CRGB::Purple);
}

void hubButtonCallbackSwitch(void *hub, HubPropertyReference hubProperty, uint8_t *pData) {

  Lpf2Hub *myHub = (Lpf2Hub *)hub;
  if (hubProperty == HubPropertyReference::BATTERY_VOLTAGE) switchBatteryLevel = myHub->parseBatteryLevel(pData);
 
}
