void stopAndDoTrain(int idTrain, bool invert) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;

  _print("Stop & ");
  (invert) ? _print("Invert") : _print("Go");
  _println(" " + myTrains[idTrain].hubColor);

  if (myTrains[idTrain].colorPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].colorPreviousMillis);
    myTrain->stopBasicMotor(portA);

    if (invert) myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  }

}

void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;

  _println("Invert " + myTrains[idTrain].hubColor);

  if (myTrains[idTrain].invertPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].invertPreviousMillis);
    myTrain->stopBasicMotor(portA);
    myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;   
    myTrains[idTrain].trainState = myTrains[idTrain].speed; 
  }

}

void startTrain(int idTrain) {

  systemStatus();

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()){
    _println("Train " + myTrains[idTrain].hubColor + " is disconnected");
    return;
  }
   
  // avoid immediate stop by discarge stop color 
  myTrains[idTrain].lastcolor = sensorAcceptedColors[0];    
    
  _println("Start " + myTrains[idTrain].hubColor);
  _println("Train " + myTrains[idTrain].hubColor + " Battery Level: "  + myTrains[idTrain].batteryLevel);  
    
  if (myTrains[idTrain].speed < 0) myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  _println("Train: " + myTrains[idTrain].hubColor + " Started!!!");    

}

void stopTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  
  if (!myTrain->isConnected()){
    _println("Train " + myTrains[idTrain].hubColor + " is disconnected");
    return;
  }
  
  _println("Stop " + myTrains[idTrain].hubColor);
  
  myTrain->stopBasicMotor(portA);
  myTrains[idTrain].trainState = 0;

}

void killTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  _println("Kill " + myTrains[idTrain].hubColor);
  myTrain->stopBasicMotor(portA);
  myTrain->shutDownHub();
  myTrains[idTrain].trainState = 0;     
  myTrains[idTrain].hubState = -1;
  
  activeTrain--;    

}

/* deprecated */
/*
void invertTrain(int idTrain) {

  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;
  
  _println("Invert " + myTrains[idTrain].hubColor);
  myTrains[idTrain].speed = -1 * myTrains[idTrain].speed;
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  
}
*/

void increaseCurrentTrainSpeed(){
  
  if (currentActiveTrainOnRemote == -1) return;

  int addspeed = myTrains[currentActiveTrainOnRemote].speed  + speedIncreaseStep;   
  if (addspeed > 100) addspeed = 100;
  myTrains[currentActiveTrainOnRemote].speed = addspeed;
   
  Lpf2Hub *myTrain = myTrains[currentActiveTrainOnRemote].hubobj;
  myTrain->setBasicMotorSpeed(portA, myTrains[currentActiveTrainOnRemote].speed);   
  _println("Train: " + myTrains[currentActiveTrainOnRemote].hubColor + " speed now is " + myTrains[currentActiveTrainOnRemote].speed); 

}

void decreaseCurrentTrainSpeed(){

  if (currentActiveTrainOnRemote == -1) return;

  int addspeed = myTrains[currentActiveTrainOnRemote].speed  - speedIncreaseStep;   
  if (addspeed < -100) addspeed = -100;
  myTrains[currentActiveTrainOnRemote].speed = addspeed;
  
  Lpf2Hub *myTrain = myTrains[currentActiveTrainOnRemote].hubobj;
   myTrain->setBasicMotorSpeed(portA, myTrains[currentActiveTrainOnRemote].speed);
  _println("Train: " + myTrains[currentActiveTrainOnRemote].hubColor + " speed now is " + myTrains[currentActiveTrainOnRemote].speed); 

}
