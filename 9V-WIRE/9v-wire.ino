// Questo sketch gestisce 2 treni 9v (A,B) su due tracciati (1,2) via bluetooth
// IMPORTANTE: mai cambiare senso di marcia senza fermare il treno prima almeno per 250 ms
// 2019 Code by Stefx 

/*
lista comandi:
ta|x => tracciato a -> X = 0) ferma treno, 1) parti treno , 2) inverti treno
tb|x => tracciato b -> X = 0) ferma treno, 1) parti treno , 2) inverti treno
va|x => velocità tracciato a -> X = valori (100-255)
vb|x => velocità tracciaato b -> X = valori (100-255)
pa|? => arresta tracciati
Funziona tutto
/*
/* ************CONFIG***************** */
#include <SoftwareSerial.h>

String ver = "1.1.2";

// debug?
bool verbose = true;
bool BTverbose = true;

// initial settings
int speedA1 = 255; //170 velocità treno 1 tracciato 1 (mai sotto 80 sennò non parte)
int speedA2 = 255; //150 velocità treno 2 tracciato 1 (mai sotto 80 sennò non parte)

/* ************PIN***************** */

// bluetooth 
int rxPin = 2;
int txPin = 4;

// commentare riga sotto per usare porte 0,1 come BT
SoftwareSerial bluetooth(rxPin, txPin);

// tracciato 1 (lungo)
const byte T1_IN1=5; // train 1 motor pin 1 
const byte T1_IN2=6; // train 1 motor pin 2
const byte T1_ENA=3; // train 1 motor PMW

// tracciato 2 (corto)
const byte T2_IN1=10; // train 1 motor pin 1 
const byte T2_IN2=11; // train 1 motor pin 2
const byte T2_ENA=9; // train 1 motor PMW


/* ************TRACCIATI***************** */
// dichiaro struttura custom per tracciato treni
typedef struct {
  byte id;  
  String codice;   
  byte T_IN1;
  byte T_IN2;
  byte T_ENA;  
  byte stato;   
} Track;

// numero tracciati presenti sul circuito
#define MY_TRACK_LEN 2 


// Mappatura tracciati
//N - codice - T_IN1 - T_IN2 - T_ENA - stato 

Track myTrack[MY_TRACK_LEN] = {
  { 1, "T1",  T1_IN1,T1_IN2,T1_ENA,0},  
  { 2, "T2",  T2_IN1,T2_IN2,T2_ENA,0}
};

/* ************COMANDI***************** */

// varibili per comandi e valori
String command;
String value;

// variabile per gli stati del tracciato/scambio
int status;


void setup() {

  Serial.begin(9600);
  bluetooth.begin(115200); 
  
  // motori (2)
  pinMode(T1_IN1, OUTPUT);
  pinMode(T1_IN2, OUTPUT);  
  //pinMode(T1_ENA, OUTPUT);
  pinMode(T2_IN1, OUTPUT);
  pinMode(T2_IN2, OUTPUT);
  //pinMode(T2_ENA, OUTPUT);

  // velocita default
  analogWrite(T1_ENA, speedA1);  
  analogWrite(T2_ENA, speedA2);  
   
  printHello(); 
  sendOutput("\nInizio log:");   
  fermaTreno(1);
  fermaTreno(2);
  
  
}

void loop()
{

  // leggo ed eseguo su serial
  if (Serial.available())
  {
    command = Serial.readStringUntil('|');
    value = Serial.readString();    
    executeCommand(command,value);  
  }
  
  // leggo ed eseguo su BT (commentare per usare BT su pin 0,1 di arduino)
  
  if (bluetooth.available())
  {        
    command = bluetooth.readStringUntil('|');
    value = bluetooth.readString();    
    executeCommand(command,value);    
    }
  
    
  
}


void printHello(){  

  sendOutput("*** M9Lab - 9V-Wire v." + ver + " ***");
  sendOutput("http://m9lab.com - info@m9lab.com");
  sendOutput("_________________________________________________");   
  
}

void panicButton(){
  
  // arresto tutto 
  sendOutput("No Panic!!!");   
  
  // tracciati spenti
  fermaTreno(1);
  fermaTreno(2);     
    
}

void executeCommand(String command, String value)
{

    value = value.substring(0,value.length());        
    sendOutput("Input: command->" + command + ", valore->" + value + "");    
  
    if (command == "ta"){    
  
    status = value.toInt();

    switch (status) {     
      case 0:     
        fermaTreno(1);
        break;
        
      case 1:
        partiTreno(1,speedA1);        
        break;
        
      case 2:
        invertiTreno(1,speedA1);        
        break;        
        
    }
                 
    }   
  
    if (command == "tb"){    
  
    status = value.toInt();

    switch (status) {     
      case 0:     
        fermaTreno(2);
        break;
        
      case 1:
        partiTreno(2,speedA2);        
        break;
        
      case 2:
        invertiTreno(2,speedA2);        
        break;              
    }       
                 
    } 

    
  
    if (command == "va"){    
      
      speedA1 = value.toInt();          
      analogWrite(T1_ENA, speedA1);        
      sendOutput("Setto Treno 1 tracciato 1 -> speed = " + value);        
       
    } 

  
    if (command == "vb"){    
  
      speedA2 = value.toInt();    
      analogWrite(T2_ENA, speedA2);  
      sendOutput("Setto Treno tracciato 2 -> speed = " + value);        
       
    } 
    

  if (command == "pa"){                  
    panicButton();
  }   

}

void sendOutput(String message){
  
  //commentare riga sotto per usare porte 0,1 come BT
  
  if (BTverbose){
    bluetooth.println(message);    
  }  

  if (verbose){
    Serial.println(message);
  } 
  
}

// uso interno (private)
void partiTreno(int idtrack, int speedT){

  // arresto prima il treno se cambio direzione diretto
  if (myTrack[idtrack-1].stato==2){
    fermaTreno(idtrack);
    delay(1000);
  }

  // setta velocità del treno  
  analogWrite(myTrack[idtrack-1].T_ENA, speedT);    
  
  // Muovo avanti treno
  digitalWrite(myTrack[idtrack-1].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack-1].T_IN2,HIGH); 
  
  // setto stato
  myTrack[idtrack-1].stato=1;

}

// uso interno (private)
void fermaTreno(int idtrack){

  // fermo il treno
  digitalWrite(myTrack[idtrack-1].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack-1].T_IN2,LOW); 
  
  // setto stato
  myTrack[idtrack-1].stato=0; 
  
}

// uso interno (private)
void invertiTreno(int idtrack, int speedT){

  // arresto prima il treno se cambio direzione diretto
  if (myTrack[idtrack-1].stato==1){
    fermaTreno(idtrack);
    delay(1000);
  }
  
  // setta velocità del treno  
  analogWrite(myTrack[idtrack-1].T_ENA, speedT);    

  // il treno andrà al contrario
  digitalWrite(myTrack[idtrack-1].T_IN1,HIGH); 
  digitalWrite(myTrack[idtrack-1].T_IN2,LOW); 
  // setto stato
  myTrack[idtrack-1].stato=2;
  
}
