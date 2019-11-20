// This sketch manages a Lego 9v train with the NiVo system
// IMPORTANT: never change direction without stopping the train before for at least 250 ms
// version 2.0.2 - 2019 Code by Stefx
// 

/*
command list:
ta|x => track A  input 0: stop train, 1: start speedA1 , 2: reverse train
va|x => train speed set to -> X = valori (100-240)
pa|? => panic button
/*
 
/* ************ GLOBAL CONFIG ***************** */
#include <SoftwareSerial.h>

// debug on serial?
bool serialVerbose = false;
// necessary to communicate via BT
bool bluetoothVerbose = true;

// default speed settings
int trainSpeed = 170; // from 100 to 240

/* ************PIN CONFIG ***************** */
// bluetooth 
int rxPin = 5;
int txPin = 4;

// track 1 pin config
const byte T1_IN1=6; // train 1 motor pin 1 
const byte T1_IN2=7; // train 1 motor pin 2
const byte T1_ENA=10; // train 1 motor PMW

/* ************ TRACKS CONFIG ***************** */
// dichiaro struttura custom per tracciato treni
typedef struct {
  byte id;  
  String code;   
  byte T_IN1;
  byte T_IN2;
  byte T_ENA;  
  int status;   
} Track;

// how many track ?
#define MY_TRACK_LEN 1

// Tracks Map
//N - code - T_IN1 - T_IN2 - T_ENA - status 

// We will only use one track but we will still create an array to optionally use it on several tracks diorama
Track myTrack[MY_TRACK_LEN] = {
  { 1, "T1",  T1_IN1,T1_IN2,T1_ENA,0}
};

/* ************ GLOBAL VARS ***************** */
// input variables for commands and values
String command;
String value;

/* LET'S START */
SoftwareSerial bluetooth(rxPin, txPin);

void setup() {

  Serial.begin(9600);
  bluetooth.begin(38400);  
  delay(1000);
  
  // setup motor
  pinMode(T1_IN1, OUTPUT);
  pinMode(T1_IN2, OUTPUT);  
  //pinMode(T1_ENA, OUTPUT);

  // set default speed
  analogWrite(T1_ENA, trainSpeed);  
       
  _stopTrain(0);
  
}

void loop()
{

  // read from serial
  if (Serial.available())
  {
    command = Serial.readStringUntil('|');
    value = Serial.readString();    
    executeCommand(command,value);  
  }
  
  //  read from bluetooth   
  if (bluetooth.available())
  {        
    command = bluetooth.readStringUntil('|');
    value = bluetooth.readString();    
    executeCommand(command,value);    
  }
        
}


void __executeCommand(){  
  // stop all trains
  _stopTrain(0);         
}

void executeCommand(String command, String value)
{
      
  // set speed
  if (command == "va"){          
    trainSpeed = value.toInt();          
    analogWrite(T1_ENA, trainSpeed);   
    _sendResponseOutput();                              
  } 

  // panic command  
  if (command == "pa"){                  
    __executeCommand();    
    _sendResponseOutput();          
  }   

  // system current status
  if (command == "ss"){                  
    _sendResponseOutput();    
  }  

	// tracks 
  if (command == "ta"){    
      
      switch (value.toInt()) {     
        case 0:     
          _stopTrain(0);        
          break;
          
        case 1:
          _startTrain(0,trainSpeed);          
          break;
          
        case 2:
          _reverseTrain(0,trainSpeed);            
          break;              
      }   

      _sendResponseOutput();
               
  }    
       
}


//  private functions  
void _sendResponseOutput(){   

  String commandx = "{\"T1\":" +  String(myTrack[0].status) + ",\"S1\":" +  String(trainSpeed) +  "}";     
  if (bluetoothVerbose) bluetooth.println(commandx);               
  if (serialVerbose) Serial.println(commandx);  
    
}

void _startTrain(int idtrack, int speedT){

  // stop train before change direction    
  if (myTrack[idtrack].status==2){
    _stopTrain(idtrack);
    delay(250);
  }  

  // set current speed  
  analogWrite(myTrack[idtrack].T_ENA, speedT);    
  
  // move the train forward
  digitalWrite(myTrack[idtrack].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack].T_IN2,HIGH); 
  
  // set new status 
  myTrack[idtrack].status=1;  

}

void _stopTrain(int idtrack){

  // stop the train
  digitalWrite(myTrack[idtrack].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack].T_IN2,LOW); 
  
  // set new status
  myTrack[idtrack].status=0;   
  
}

void _reverseTrain(int idtrack, int speedT){

  // stop train before change direction
  if (myTrack[idtrack-1].status==1){
    _stopTrain(idtrack);
    delay(250);
  }  
  
  // set current speed  
  analogWrite(myTrack[idtrack].T_ENA, speedT);    
  
  //reverse direction
  digitalWrite(myTrack[idtrack].T_IN1,HIGH); 
  digitalWrite(myTrack[idtrack].T_IN2,LOW); 
  
  // set new status  
  myTrack[idtrack].status=2;  
    
}
