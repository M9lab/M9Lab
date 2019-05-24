// Questo sketch gestisce un treno 9v su tracciato singolo (Xmas9V)
// IMPORTANTE: mai cambiare senso di marcia senza fermare il treno prima almeno per 250 ms
// 2017 Code by Stefx 
// ver 1.1.8 

String ver = "1.1.8";

#include <SoftwareSerial.h>

// debug?
bool verbose = true;
bool BTverbose = true;
String lastmessage;

// bluetooth 
int rxPin = 2;
int txPin = 4;
SoftwareSerial bluetooth(rxPin, txPin);

const int T1_IN1=5; // train 1 motor pin 1 
const int T1_IN2=6; // train 1 motor pin 2
const int T1_ENA=11; // train 1 motor PMW

int speed1 = 240; // velocità treno 1 (mai sotto 80 sennò non parte)

// dichiaro struttura custom per treni
typedef struct {
   int id;  
   String codice;   
   int T_IN1;
   int T_IN2;
   int T_ena;  
   int stato; 
} Train;

// numero treni presenti sul circuito
#define MY_TRAIN_LEN 1 

// stato del treno
//  0   = treno fermo
//  1   = treno partito
// -1   = treno retromarcia 

// Mappatura treni
//N - codice - T_IN1 - T_IN2 - T_ena - stato

Train trains[MY_TRAIN_LEN] = {
  { 1, "traincode",  T1_IN1,T1_IN2,T1_ENA,0}  
};


String command;
String value;
// possibili comandi:
// set0|sec = setta timer stop in sec
// set1|sec = setta timer start in sec
// status|s = setta stato tracciato {1,2,3}



// solo per test
const int ledPin =  13;               // the number of the LED pin

int ledState = LOW;                   // ledState used to set the LED
long previousMillis = 0;              // will store last time LED was updated

long interval;
long interval_start = 30000;           // interval at which to blink (milliseconds)
long interval_stop = 30000;


int status;
// possibili stati del tracciato
// status = 0 fermo nel loop
// status = 1 parti nel loop
// status = 2 parti perpetuo
// status = 3 stop perpetuo


void setup() {

  Serial.begin(9600);
  bluetooth.begin(9600);  
  
  // print command
/*   sendOutput("\n");
  sendOutput("*** M9Lab - Xmas9V v." + ver + " ***");
  sendOutput("http://m9lab.com - info@m9lab.com");
  sendOutput("_________________________________________________");
  sendOutput("Lista comandi:");
  sendOutput("set0|sec = setta timer stop nel loop in sec");
  sendOutput("set1|sec = setta timer start nel loop in sec");
  sendOutput("status|s = setta stato tracciato: {0,1,2,3}");  
  sendOutput("0->mode loop on: stop");
  sendOutput("1->mode loop on: start");  
  sendOutput("2->parti treno");
  sendOutput("3->ferma treno");  
  sendOutput("speed|vel = setta andatura treno: {90,110,130,150,170}");
  sendOutput("_________________________________________________");
  sendOutput("Inizio log:"); */
  
  sendOutput("\n*** M9Lab - Xmas9V v." + ver + " ***\nhttp://m9lab.com - info@m9lab.com\n_________________________________________________\nLista comandi:\nset0|sec = setta timer stop nel loop in sec\nset1|sec = setta timer start nel loop in sec\nstatus|s = setta stato tracciato: {0,1,2,3}\n0->mode loop on: stop\n1->mode loop on: start\n2->parti treno\n3->ferma treno\nspeed|vel = setta andatura treno: {150,170,190,210,230}\n_________________________________________________\nInizio log:");
  
  
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);  
  

  // attesa iniziale
  delay(3000);  
  analogWrite(T1_ENA, speed1);  

  // default parte il treno
  interval = interval_start;
  status = 1; 
  
  
}

void loop()
{

  if (bluetooth.available())
  {        
      command = bluetooth.readStringUntil('|');
      value = bluetooth.readString();    
      executeCommand(command,value);
        
  }
  
  if (Serial.available())
  {
  
    command = Serial.readStringUntil('|');
    value = Serial.readString();    
    executeCommand(command,value);
        
  }


  // start timer
  unsigned long currentMillis = millis();

  // se stato compreso nel loop (0/1)
  if (status<2){
 
    if(currentMillis - previousMillis > interval) {
   
    previousMillis = currentMillis;   

    // start code
    if (status == 0){
      
      sendOutput("Stop per " + String(interval_stop/1000) + " secondi");              
      start_exec();
      fermaTreno(1);
      interval = interval_stop;
      status = 1;                 
      
    }else{

      sendOutput("Start per " + String(interval_start/1000) + " secondi");        
      // stop code      
      stop_exec();
      partiTreno(1);
      interval = interval_start;
      status = 0;            
      
    }

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
        
    }

  }else{

      if (status == 2){
        sendOutput("Start perpetuo");  
        partiTreno(1);
      }

      if (status == 3){
        sendOutput("Stop perpetuo");  
        fermaTreno(1);
        
      }
      
    
  }
}

void executeCommand(String command, String value)
{

    value = value.substring(0,value.length());   
     
    sendOutput("Input ricevuti:\n command->" + command + ", valore->" + value);    

    if (command == "set0"){    
  
      interval_stop = value.toInt() * 1000;
      sendOutput("Setto intervallo stop a " + value + " secondi");      
      
    }
  
    if (command == "set1"){   
    
      interval_start = value.toInt() * 1000;
      sendOutput("Setto intervallo start a " + value + " secondi");
       
    }

    if (command == "status"){    
  
      status = value.toInt();
      sendOutput("Setto status a " + value);        
       
    }
	
    if (command == "speed"){    
  
      speed1 = value.toInt();
	  analogWrite(T1_ENA, speed1);  
      sendOutput("Setto speed a " + value);        
       
    }	

}

void sendOutput(String message){

  if (lastmessage != message){
    
    if (BTverbose){
      bluetooth.println(message);    
    }
  
    if (verbose){
      Serial.println(message);
    }
  
    lastmessage = message;
  }
}

void start_exec(){
  ledState = LOW;
}

void stop_exec(){
  ledState = HIGH;    
}

void partiTreno(int idtreno){
  
  // the train will move forward
  digitalWrite(trains[idtreno-1].T_IN1,LOW); 
  digitalWrite(trains[idtreno-1].T_IN2,HIGH); 
  
  trains[idtreno-1].stato=1;

}

void fermaTreno(int idtreno){

  // the train will stop
  digitalWrite(trains[idtreno-1].T_IN1,LOW); 
  digitalWrite(trains[idtreno-1].T_IN2,LOW); 
  
  trains[idtreno-1].stato=0;
  
}

void invertiTreno(int idtreno){

  // the train will move in reverse
  digitalWrite(trains[idtreno-1].T_IN1,HIGH); 
  digitalWrite(trains[idtreno-1].T_IN2,LOW); 
  
  trains[idtreno-1].stato=-1;
  
}




