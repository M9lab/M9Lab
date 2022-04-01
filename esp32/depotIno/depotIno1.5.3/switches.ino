void setSwitch(Switches *cSwitch, bool position) {

  byte *myPort = (byte *)cSwitch->port;
  if (!mySwitchController.isConnected()) return;

  // position 0=straight, 1= turned
  if (cSwitch->switchState == position) {
    _println("already setted");
    return;
  }

  bool p = position;
  if (cSwitch->switchInvert == 1 ) p = !position;    
  
  int velocity = p ? switchVelocity : (switchVelocity * -1);    

  mySwitchController.setTachoMotorSpeed(*myPort, velocity);
  delay(switchInterval);
  mySwitchController.stopTachoMotor(*myPort);
  cSwitch->switchState = position;

}

void switchReset() {

  if (!mySwitchController.isConnected()) return;
  
  for (int idSwitch = 0; idSwitch < MY_SWITCH_LEN; idSwitch++) {
    setSwitch(&mySwitchControlleres[idSwitch], 0);
  }
  
}

void increaseSwitchSpeed(){
  switchVelocity = switchVelocity+ 5;
  _print("Switch motor speed now is "); 
  _println(String(switchVelocity));
}

void decreaseSwitchSpeed(){
  switchVelocity = switchVelocity - 5;
  _print("Switch motor speed now is "); 
  _println(String(switchVelocity));
}

void resetSwitchSpeed(){
  switchVelocity = 35;
  _print("Switch motor speed now is "); 
  _println(String(switchVelocity));
}
