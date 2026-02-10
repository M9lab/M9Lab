void setSwitch(Switches *cSwitch, bool position) {

  if (!mySwitchController.isConnected()) return;

  // position 0=straight, 1= turned
  if (cSwitch->switchState == position) {
    if (isVerbose) Serial.println(F("already setted"));
    return;
  }

  bool p = position;
  if (cSwitch->switchInvert) p = !position;    
  
  int velocity = p ? switchVelocity : -switchVelocity;    

  mySwitchController.setTachoMotorSpeed(*(cSwitch->port), velocity);
  delay(switchInterval);
  mySwitchController.stopTachoMotor(*(cSwitch->port));
  cSwitch->switchState = position;

}

void switchReset() {

  if (!mySwitchController.isConnected()) return;
  
  for (int idSwitch = 0; idSwitch < MY_SWITCH_LEN; idSwitch++) {
    setSwitch(&mySwitchControlleres[idSwitch], 0);
  }
  
}

void increaseSwitchSpeed(){
  switchVelocity += 5;
  if (isVerbose) {
    Serial.print(F("Switch motor speed now is "));
    Serial.println(switchVelocity);
  }
}

void decreaseSwitchSpeed(){
  switchVelocity -= 5;
  if (isVerbose) {
    Serial.print(F("Switch motor speed now is "));
    Serial.println(switchVelocity);
  }
}

void resetSwitchSpeed(){
  switchVelocity = 35;
  if (isVerbose) {
    Serial.print(F("Switch motor speed now is "));
    Serial.println(switchVelocity);
  }
}
