// Questo sketch gestisce 2 treni 9v (A,B) su due tracciati (1,2) via bluetooth con Arduino Nano
// IMPORTANTE: mai cambiare senso di marcia senza fermare il treno prima almeno per 250 ms o più
// 2019 Code by Stefx 

/*
lista comandi:
ta|x => tracciato a -> X = 0) ferma treno, 1) parti treno , 2) inverti treno
tb|x => tracciato b -> X = 0) ferma treno, 1) parti treno , 2) inverti treno
va|x => velocità tracciato a -> X = valori (100-255)
vb|x => velocità tracciaato b -> X = valori (100-255)
pa|? => arresta tracciati
/*
/* ************CONFIG***************** */
#include <SoftwareSerial.h>

String ver = "1.2.1";

// debug?
bool verbose = true;
bool BTverbose = true;

// initial settings
int speedA1 = 170; //170 velocità treno 1 tracciato 1 (mai sotto 80 sennò non parte)
int speedA2 = 170; //170 velocità treno 2 tracciato 1 (mai sotto 80 sennò non parte)
float speedCalbrate = 1.2; // fattore aumento velocità per 2 treni contemporaneamente in circolo (x bug calo di potenza)

/* ************PIN***************** */

// bluetooth 
int rxPin = 4;
int txPin = 5;

// commentare riga sotto per usare porte 0,1 come BT
SoftwareSerial bluetooth(rxPin, txPin);

// tracciato 1 (lungo)
const byte T1_IN1=6; // train 1 motor pin 1 
const byte T1_IN2=7; // train 1 motor pin 2
const byte T1_ENA=10; // train 1 motor PMW

// tracciato 2 (corto)
const byte T2_IN1=8; // train 1 motor pin 1 
const byte T2_IN2=9; // train 1 motor pin 2
const byte T2_ENA=11; // train 1 motor PMW


/* ************TRACCIATI***************** */
// dichiaro struttura custom per tracciato treni
typedef struct {
  byte id;  
  String codice;   
  byte T_IN1;
  byte T_IN2;
  byte T_ENA;  
  int stato;   
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
  bluetooth.begin(38400);  
  delay(1000);  
  
  // motori
  pinMode(T1_IN1, OUTPUT);
  pinMode(T1_IN2, OUTPUT);  
  pinMode(T1_ENA, OUTPUT);
  pinMode(T2_IN1, OUTPUT);
  pinMode(T2_IN2, OUTPUT);
  pinMode(T2_ENA, OUTPUT);
	
  
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


void panicButton(){  
	// arresto tutto   
	fermaTreno(0);
	fermaTreno(1); 	
}

void executeCommand(String command, String value)
{
      
    /*velocità*/  
    if (command == "va"){          
      speedA1 = value.toInt();    
	  setTrainSpeed(0, speedT);                 
      sendResponseOutput();                              
    } 

    if (command == "vb"){      
      speedA2 = value.toInt();    
	  setTrainSpeed(1, speedT);          
      sendResponseOutput();                     
    } 
	    
    if (command == "pa"){                  
      panicButton();    
      sendResponseOutput();          
    }   

    if (command == "ss"){                  
      sendResponseOutput();    
    }  

	/*tracciati*/
  
    if (command == "ta"){    
  
      status = value.toInt();
  
        switch (status) {     
          case 0:     
            fermaTreno(0);        
            break;
            
          case 1:
            partiTreno(0,speedA1);          
            break;
            
          case 2:
            invertiTreno(0,speedA1);            
            break;              
        }   
  
        sendResponseOutput();
                 
    }   
  
    if (command == "tb"){    
    
      status = value.toInt();
  
      switch (status) {     
        case 0:     
          fermaTreno(1);        
          break;
          
        case 1:
          partiTreno(1,speedA2);          
          break;
          
        case 2:
          invertiTreno(1,speedA2);          
          break;              
      }     

      sendResponseOutput();
                 
    }     
       
}

void sendResponseOutput(){   

  String commandx = "{\"T1\":" +  String(myTrack[0].stato) + ",\"T2\":" +  String(myTrack[1].stato) + ",\"S1\":" +  String(speedA1) + ",\"S2\":" +  String(speedA2) + "}";     
  if (BTverbose) bluetooth.println(commandx);               
  if (verbose) Serial.println(commandx);  
    
}

// uso interno (private)
void partiTreno(int idtrack, int speedT){

  // arresto prima il treno se cambio direzione diretto
  
  if (myTrack[idtrack].stato==2){
    fermaTreno(idtrack);
    delay(2000);
  }  
  
  // setto stato  
  myTrack[idtrack].stato=1;  

  // setta velocità del treno  
  setTrainSpeed(idtrack, speedT);       
  
  // Muovo avanti treno
  digitalWrite(myTrack[idtrack].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack].T_IN2,HIGH); 
  
}

// uso interno (private)
void fermaTreno(int idtrack){

  // fermo il treno
  digitalWrite(myTrack[idtrack].T_IN1,LOW); 
  digitalWrite(myTrack[idtrack].T_IN2,LOW); 
  
  // setto stato
  myTrack[idtrack].stato=0;   
  
}

// uso interno (private)
void invertiTreno(int idtrack, int speedT){

  // arresto prima il treno se cambio direzione diretto  
  if (myTrack[idtrack].stato==1){
    fermaTreno(idtrack);
    delay(2000);
  }  
  
  // setto stato
  myTrack[idtrack].stato=2;  
  
  // setta velocità del treno  
  setTrainSpeed(idtrack, speedT);    

  // il treno andrà al contrario
  digitalWrite(myTrack[idtrack].T_IN1,HIGH); 
  digitalWrite(myTrack[idtrack].T_IN2,LOW); 
         
}


void setTrainSpeed(int idtrack, int speedT){
	
	// controlla se 2 treni sono in funzione
	if (myTrack[0].stato>0 && myTrack[1].stato>0){
		// risetta velocità
		if (idtrack==0) speedA1 = speedT;								
		if (idtrack==1) speedA2 = speedT;
		
		// ricalcolo nuove velocità aumentando potenza						
		analogWrite(myTrack[0].T_ENA, speedA1*speedCalbrate);   
		analogWrite(myTrack[1].T_ENA, speedA2*speedCalbrate);   		
		
	}else{	
	
		// altrimenti se un solo treno setta velocità
		analogWrite(myTrack[idtrack].T_ENA, speedT);   
	}	
}