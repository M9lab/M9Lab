void stopAndDoTrain(int idTrain, bool invert) {
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;

  _print("Stop & ");
  _println(invert ? "Invert" : "Go");
  _println(" " + myTrains[idTrain].hubColor);

  if (myTrains[idTrain].colorPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].colorPreviousMillis);
    myTrain->stopBasicMotor(portA);
    if (invert) myTrains[idTrain].speed *= -1;
  }
}

void invertTrain(int idTrain) {
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) return;

  _println("Invert " + myTrains[idTrain].hubColor);

  if (myTrains[idTrain].invertPreviousMillis == 0) {
    saveInterval(myTrains[idTrain].invertPreviousMillis);
    myTrain->stopBasicMotor(portA);
    myTrains[idTrain].speed *= -1;   
    myTrains[idTrain].trainState = myTrains[idTrain].speed; 
  }
}

void startTrain(int idTrain) {
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) {
    _println("Train " + myTrains[idTrain].hubColor + " is disconnected");
    return;
  }
   
  myTrains[idTrain].lastcolor = sensorAcceptedColors[0];    
  _println("Start " + myTrains[idTrain].hubColor);
  _println("Train " + myTrains[idTrain].hubColor + " Battery Level: " + myTrains[idTrain].batteryLevel);  
    
  myTrains[idTrain].speed = abs(myTrains[idTrain].speed);
  myTrains[idTrain].trainState = myTrains[idTrain].speed;
  myTrain->setBasicMotorSpeed(portA, myTrains[idTrain].speed);
  _println("Train: " + myTrains[idTrain].hubColor + " Started!!!"); 
}

void stopTrain(int idTrain) {
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  if (!myTrain->isConnected()) {
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

void adjustTrainSpeed(int idTrain, bool increase) {
  if (idTrain == -1) return;

  int newSpeed = myTrains[idTrain].speed + (increase ? speedIncreaseStep : -speedIncreaseStep);
  newSpeed = constrain(newSpeed, -100, 100);
  myTrains[idTrain].speed = newSpeed;
   
  Lpf2Hub *myTrain = myTrains[idTrain].hubobj;
  myTrain->setBasicMotorSpeed(portA, newSpeed);   
  _println("Train: " + myTrains[idTrain].hubColor + " speed now is " + newSpeed);
}

void increaseCurrentTrainSpeed() {
  adjustTrainSpeed(currentActiveTrainOnRemote, true);
}

void decreaseCurrentTrainSpeed() {
  adjustTrainSpeed(currentActiveTrainOnRemote, false);
}
