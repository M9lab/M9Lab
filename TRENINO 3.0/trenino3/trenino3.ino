// Questo sketch gestisce 3 treni (A,B,C) su due tracciati (1,2) con due scambi 9v (1,2)
// il tutto controllato da 5 sensore (2 per lo scambio a X) e 5 semafori (2 per lo scambio + 3 per stop alla stazione)
// IMPORTANTE: mai cambiare senso di marcia senza fermare il treno prima almeno per 250 ms
// 2019 Code by Stefx

// AGGIUNTO ->  Parte Stazione Multimendiale
// TODO -> verificare parte Stazione Multimediale su TA|x e TB|x
// nel caso commentare riga 358 lasciaso solo riga 363

/* ************CONFIG***************** */

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "RTClib.h"
#include <Wire.h>

String ver = "3.0.2";

// settings SM
RTC_DS1307 rtc;

DFRobotDFPlayerMini myDFPlayer;

// vars annuncio SM
int sm_action;
int sm_track;
int sm_train;
bool sm_finished = true;
bool sm_ready = false;
int sm_playList[15] = {1,2,3};
int sm_totalFile = 1;
int sm_idx = 0;
// end vars SM

// debug?
bool verbose = true;
//int appoggio;
bool scaduto = false;

int speedA1 = 230; //210 velocità treno 1 tracciato 1 (mai sotto 80 sennò non parte)
int speedA2 = 150; //150 velocità treno 2 tracciato 1 (mai sotto 80 sennò non parte)
int speedB = 130; //130 velocità treno tracciato 2 (mai sotto 80 sennò non parte)

/* ************PIN***************** */

// bluetooth 

//int rxPin = 11;
//int txPin = 2;

//SoftwareSerial bluetooth(rxPin, txPin);

//conflitto da risolvere ?? Lettore Mp3
int rxPin = 11;
int txPin = 12;
SoftwareSerial mySoftwareSerial(rxPin, txPin);

// tracciato 1 (lungo)
const byte T1_IN1=23; // train 1 motor pin 1 
const byte T1_IN2=25; // train 1 motor pin 2
const byte T1_ENA=9; // train 1 motor PMW

// tracciato 2 (corto)
const byte T2_IN1=27; // train 1 motor pin 1 
const byte T2_IN2=29; // train 1 motor pin 2
const byte T2_ENA=10; // train 1 motor PMW

// Scambio A
const byte S1_IN1=28; // train 1 motor pin 1 
const byte S1_IN2=26; // train 1 motor pin 2
const byte S1_ENA=7; // train 1 motor PMW

// Scambio B
const byte S2_IN1=24; // train 1 motor pin 1 
const byte S2_IN2=22; // train 1 motor pin 2
const byte S2_ENA=6; // train 1 motor PMW

// Sensori (5)
const byte ST1_incrocio = 35;
const byte ST2_incrocio = 39;
const byte ST1_binario1 = 31;
const byte ST1_binario2 = 33;
const byte ST2_binario1 = 37;

// Semafori (10) (LOW=verde/HIGH=rosso) 30 - 44
// Incrocio T1
const byte LT1_incrocioA = 30;
const byte LT1_incrocioB = 32;

// Incrocio T2
const byte LT2_incrocioA = 41;
const byte LT2_incrocioB = 43;

// Treno A
const byte LT1_binario1A = 46;
const byte LT1_binario1B = 48;

// Treno B
const byte LT1_binario2A = 42;
const byte LT1_binario2B = 44;

// Treno C
const byte LT2_binario1A = 36; 
const byte LT2_binario1B = 34; 

// luci
int ST_lucipin=45;

//possibili stati del tracciato (non toccare)
const byte TRACK_SECTION_CLEAR = 0;  //both light sensors off, power to both trains
const byte T1_TRAIN_DETECTED = 1;  //Red train detected, power to both trains
const byte T2_TRAIN_DETECTED = 2; //Green train detected, power to both trains
const byte T1_TRAIN_OFF = 3;  //Both trains detected (Green first), turn off Red train
const byte T2_TRAIN_OFF = 4;  //Both trains detected (Red first), turn off Green train
const byte TRACK_SECTION_OFF = 5;  //both light sensors off, turn off both trains
byte state = TRACK_SECTION_CLEAR;


//variabili per junction
unsigned long previousMillis = 0;
unsigned long previousMillis_uscita = 0;
int interval = 5000; //attesa attraversamento treno
int uscitastazione = 7000;

//  variabile per lettura fermi stazione
//int stazionevalue;

//sensori infrarosso  (fermi stazione)
boolean F1_isavaiable = true;
boolean F2_isavaiable = true;
boolean F3_isavaiable = true;

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

// stato del treno sul tracciato
//  0   = treno fermo
//  1   = treno partito
// -1   = treno retromarcia
//  2	= treno in ingresso

// lastcommand dei treni
// 0 fermati alla stazione
// 1 parti dalla stazione

// Mappatura tracciati
//N - codice - T_IN1 - T_IN2 - T_ENA - stato

Track myTrack[MY_TRACK_LEN] = {
  { 1, "T1",  T1_IN1, T1_IN2, T1_ENA, 0},
  { 2, "T2",  T2_IN1, T2_IN2, T2_ENA, 0}
};


/* ************SCAMBI***************** */

int interval_scambio = 500; //attesa delay degli scambi

// dichiaro struttura custom per gli scambi
typedef struct {
  byte id;
  String codice;
  byte T_IN1;
  byte T_IN2;
  byte T_ENA;
  byte stato;
} Switch;

// stato delo scambio
// 	0   =  diritto
//  1   =  scambia
// -1   =  unset

#define MY_SWITCH_LEN 2

Switch mySwitch[MY_SWITCH_LEN] = {
  { 1, "S1",  S1_IN1, S1_IN2, S1_ENA, 2},
  { 2, "S2",  S2_IN1, S2_IN2, S2_ENA, 2}
};

/* ************TRENI***************** */

// dichiaro struttura custom per treni
typedef struct {
  byte id;
  String codice;
  byte tracciato;
  byte scambio;
  byte semaforoA;
  byte semaforoB;
  byte stato;
  byte lastcommand;
  int speedT;
} Train;

// numero trenif
#define MY_TRAIN_LEN 3

// Mappatura treni
//N - codice  - tracciato	-	scambio	-	pin semaforo1 - pin semaforo2 -	stato(0 = fermo, 1 = avanti, -1 = indietro, 2= in ingresso) - int lastcommand;

Train myTrains[MY_TRAIN_LEN] = {
  { 1, "TA", 1, 1, LT1_binario1A, LT1_binario1B, 0, 0, speedA1},
  { 2, "TB", 1, 2, LT1_binario2A, LT1_binario2B, 0, 0, speedA2},
  { 3, "TC" , 2, 0, LT2_binario1A, LT2_binario1B, 0, 0, speedB}
};

/* ************COMANDI***************** */

// varibili per comandi e valori
String command;
String value;

// variabile per gli stati del tracciato/scambio
int status;

void setup() {

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  } else {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //Use softwareSerial to communicate with mp3.
  if (!myDFPlayer.begin(mySoftwareSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  // settaggi mp3
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms
  myDFPlayer.volume(30);  //Set volume value (0~30).
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  // motori (2)

  pinMode(T1_IN1, OUTPUT);
  pinMode(T1_IN2, OUTPUT);
  //pinMode(T1_ENA, OUTPUT);
  pinMode(T2_IN1, OUTPUT);
  pinMode(T2_IN2, OUTPUT);
  //pinMode(T2_ENA, OUTPUT);

  // velocita default
  analogWrite(T1_ENA, speedA1);
  analogWrite(T2_ENA, speedB);

  // scambi (2)
  pinMode(S1_IN1, OUTPUT);
  pinMode(S1_IN2, OUTPUT);
  //pinMode(S1_ENA, OUTPUT);
  analogWrite(S1_ENA, 255);

  pinMode(S2_IN1, OUTPUT);
  pinMode(S2_IN2, OUTPUT);
  //pinMode(S2_ENA, OUTPUT);
  analogWrite(S2_ENA, 255);

  // sensori (5)
  pinMode(ST1_incrocio, INPUT);
  pinMode(ST2_incrocio, INPUT);
  pinMode(ST1_binario1, INPUT);
  pinMode(ST1_binario2, INPUT);
  pinMode(ST2_binario1, INPUT);

  // semafori (10)
  pinMode(LT1_incrocioA, OUTPUT);
  pinMode(LT1_incrocioB, OUTPUT);
  pinMode(LT2_incrocioA, OUTPUT);
  pinMode(LT2_incrocioB, OUTPUT);
  pinMode(LT1_binario1A, OUTPUT);
  pinMode(LT1_binario1B, OUTPUT);
  pinMode(LT1_binario2A, OUTPUT);
  pinMode(LT1_binario2B, OUTPUT);
  pinMode(LT2_binario1A, OUTPUT);
  pinMode(LT2_binario1B, OUTPUT);

  //luci (1)
  pinMode(ST_lucipin, OUTPUT);

  printLegenda();
  sendOutput("\nInizio log:");

  systemReset();


}

void loop()
{

  // leggo ed eseguo su serial
  if (Serial.available())
  {

    // leggo ed eseguo da serial il comando in formato treno|binario|azione //ex: 111       
    command = Serial.readStringUntil('|');
    value = Serial.readString();

    //se un comando ingresso uscita treno allora mando audio
    // COMANDO NON VALIDO PER TRENO C || command=="tc" 
    // executeAudioPlayList(int sm_train, int sm_track , int sm_action) {    
    if(command=="ta"){      
      executeAudioPlayList(1,2,value.toInt());            
    }else if (command=="tb"){
      executeAudioPlayList(2,3,value.toInt());  
    }else{
      executeCommand(command, value);  
    }    
    
  }

  // routine di controllo per i sensori
  while (Serial.available() == 0) {

    audiocontrol();

    // sensori fermi stazione
    controllaSensore(1, digitalRead(ST1_binario1), F1_isavaiable);
    controllaSensore(2, digitalRead(ST1_binario2), F2_isavaiable);
    controllaSensore(3, digitalRead(ST2_binario1), F3_isavaiable);

    //controllo se scambiare scambio uscita  controllando il timeout
    if (millis() - previousMillis_uscita > uscitastazione) {
      if (scaduto == false) {
        scaduto = true;
        for (int i = 0; i < MY_TRAIN_LEN; i++) {
          if (myTrains[i].scambio > 0) scambia(myTrains[i].scambio, "diritto");
        }
      }

    }

    switch (state)
    {

      case TRACK_SECTION_CLEAR:  //both light sensors off, power to both trains
        {

          //Serial.println(state);
          // semafori
          settaSemaforo(LT1_incrocioA, LT1_incrocioB, "verde");
          settaSemaforo(LT2_incrocioA, LT2_incrocioB, "verde");

          // Faccio ripartire treno?
          if (myTrains[0].stato > 0) partiTreno(myTrains[0].tracciato, myTrains[0].speedT);
          if (myTrains[1].stato > 0) partiTreno(myTrains[1].tracciato, myTrains[1].speedT);
          if (myTrains[2].stato > 0) partiTreno(myTrains[2].tracciato, myTrains[2].speedT);

          if (digitalRead(ST1_incrocio) == HIGH) {
            previousMillis = millis();    //save the time
            state = T2_TRAIN_DETECTED;
            settaSemaforo(LT1_incrocioA, LT1_incrocioB, "rosso");
          }

          if (digitalRead(ST2_incrocio) == HIGH) {
            previousMillis = millis();    //save the time
            state = T1_TRAIN_DETECTED;
            settaSemaforo(LT2_incrocioA, LT2_incrocioB, "rosso");
          }

        }
        break;

      case T1_TRAIN_DETECTED:  //Red train detected, power to both trains
        {


          //NEWS: ---> Controlla se scambiare binario per ingresso treni tracciato grande
          if (myTrains[0].scambio > 0 && myTrains[0].stato == 2) scambia(myTrains[0].scambio, "scambia");
          if (myTrains[1].scambio > 0 && myTrains[1].stato == 2) scambia(myTrains[1].scambio, "scambia");

          //NEWS: ---> Controlla se scambiare binario post uscita treni tracciato grande
          // nel caso commentare la riga 307
          //if (myTrains[0].scambio>0 && myTrains[0].stato==1) scambia(myTrains[0].scambio,"diritto");
          //if (myTrains[1].scambio>0 && myTrains[1].stato==1) scambia(myTrains[1].scambio,"diritto");



          if (digitalRead(ST1_incrocio) == HIGH) {
            state = T2_TRAIN_OFF;
            settaSemaforo(LT2_incrocioA, LT2_incrocioB, "rosso");
            settaSemaforo(LT1_incrocioA, LT1_incrocioB, "verde");
          }

          else if (millis() - previousMillis > interval) {
            state = TRACK_SECTION_CLEAR;
          }
        }
        break;

      case T2_TRAIN_DETECTED: //Green train detected, power to both trains
        {

          //Serial.println(state);
          if (digitalRead(ST2_incrocio) == HIGH) {
            state = T1_TRAIN_OFF;
            settaSemaforo(LT1_incrocioA, LT1_incrocioB, "rosso");
            settaSemaforo(LT2_incrocioA, LT2_incrocioB, "verde");
          }

          else if (millis() - previousMillis > interval) {
            state = TRACK_SECTION_CLEAR;
          }
        }
        break;

      case T1_TRAIN_OFF:  //Both trains detected (Green first), turn off Red train
        {

          //Serial.println(state);
          if (millis() - previousMillis < interval) {

            fermaTreno(1);

            settaSemaforo(LT1_incrocioA, LT1_incrocioB, "rosso");
            settaSemaforo(LT2_incrocioA, LT2_incrocioB, "verde");
          }
          else {
            state = TRACK_SECTION_CLEAR;
          }
        }
        break;

      case T2_TRAIN_OFF:  //Both trains detected (Red first), turn off Green train
        {

          //Serial.println(state);
          if (millis() - previousMillis < interval) {
            fermaTreno(2);

            settaSemaforo(LT1_incrocioA, LT1_incrocioB, "verde");
            settaSemaforo(LT2_incrocioA, LT2_incrocioB, "rosso");
          }
          else {
            state = TRACK_SECTION_CLEAR;
          }
        }
        break;
    }

  }

}


void audiocontrol(){
  
  // debug su serial degli eventi del lettore mp3 (opzionale, da commentare)
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }

  if (sm_finished && sm_ready) {
    myDFPlayer.playMp3Folder(sm_playList[sm_idx++]);  //Play next mp3
    sm_finished = false;
  }

  // fine annuncio (ultimo mp3)
  if (sm_idx == sm_totalFile) {
    sm_idx = 0;
    sm_ready = false;

    Serial.println("fine annuncio");
    // TODO in base a azione faccio uscire treno o lo faccio entrare
    //Serial.println(sm_action); //112 (a) 123 (b)
    //Serial.println(sm_train);
    //Serial.println(sm_track);

    // setta stato treno entra
    if(sm_action==0){
      
      entraTreno(sm_train);     
      myTrains[sm_train-1].stato = 2; 
    }  
        
    // setta stato treno esci
    if(sm_action==1){      
      myTrains[sm_train-1].stato = 1;
      esciTreno(sm_train);      
    } 

     myTrains[sm_train-1].lastcommand = sm_action;
    
  }
}

// funzione per SM

void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      sm_finished = true;
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      sm_ready = true;
      sm_finished = true;
      break;
    case DFPlayerPlayFinished:
      //Serial.println(value);
      sm_finished = true;
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          //Serial.println(value);

          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;

    default:
      Serial.println(F("??"));
      break;
  }
}

void executeAudioPlayList(int sm_train_input, int sm_track_input , int sm_action_input) {

  sm_train = sm_train_input;
  sm_track = sm_track_input;
  sm_action = sm_action_input;

  DateTime now = rtc.now();
  int currentMinute = now.minute();
  int currentHour = now.hour();

  if(currentMinute==0) currentMinute=110;

  if (! rtc.isrunning()) {
    currentMinute = 22;
    currentHour = 17;
  }

  // reset
  sm_totalFile = 0;
  
  //for ( int i = 0; i < sizeof(sm_playList);  i++ )sm_playList[i] = (char)0;
  

  // bip
  addToPlayList(111);

  // il treno
  addToPlayList(121);

  // treno
  switch (sm_train) {
    //65001
    case 1:
      addToPlayList(201);
      addToPlayList(65);
      addToPlayList(110);
      //addToPlayList(100);
      addToPlayList(1);
      break;

    case 2:
      // railway express
      addToPlayList(202);
      addToPlayList(45);
      addToPlayList(61);
      break;

    case 3:
      // cargo express
      addToPlayList(203);
      addToPlayList(45);
      addToPlayList(59);
      break;

  }

  //di mezzanine lab
  addToPlayList(140);

  //delle ore
  addToPlayList(131);

  //orario
  addToPlayList(currentHour);

  // e
  addToPlayList(135);

  int minute = convertIntTo2DigitString(currentMinute);
  addToPlayList(minute);

  //e in arrivo al binario
  if (sm_action == 0) {
    addToPlayList(141);
  }

  //e in partenza dal binario
  if (sm_action == 1) {
    addToPlayList(146);
  }

  // n binario
  addToPlayList(sm_track);

  //avviso finale
  addToPlayList(165);

  // vietato indicare
  addToPlayList(191);

  // abilita playlist
  sm_ready = true;

}

int convertIntTo2DigitString(int i)  {
  if (i < 10)  {
    i = i + 100;
  }
  return i;
}

void addToPlayList(int mp3) {
  sm_playList[sm_totalFile] = mp3;
  sm_totalFile++;
}

void sendVietatoIndicareMsg() {

  sm_totalFile = 0;
  //for ( int i = 0; i < sizeof(sm_playList);  i++ )sm_playList[i] = (char)0;  
  
  addToPlayList(191);
  sm_ready = true;

}

// fine funzioni per SM


void printLegenda() {

  // print command
  sendOutput("*** M9Lab - Trenino2 v." + ver + " ***");
  sendOutput("http://m9lab.com - info@m9lab.com");
  sendOutput("_________________________________________________");
  sendOutput("Lista comandi:");

  sendOutput("t1|s = setta stato treno tracciato 1: {-1,0,1}");
  sendOutput("       -1 ->inverti");
  sendOutput("       0 ->ferma treno");
  sendOutput("       1 ->parti treno");

  sendOutput("t2|s = setta stato treno tracciato 2: {-1,0,1,2}");
  sendOutput("       -1 ->inverti");
  sendOutput("       0 ->ferma treno");
  sendOutput("       1 ->parti treno");

  sendOutput("s1|s = setta stato scambio 1: {'0','1'}");
  sendOutput("       0 ->diritto");
  sendOutput("       1 ->scambia");

  sendOutput("s2|s = setta stato scambio 2: {'0','1'}");
  sendOutput("       0 ->diritto");
  sendOutput("       1 ->scambia");

  sendOutput("v1|vel = setta andatura treno tracciato 1: {150,170,190,210,230}");
  sendOutput("v2|vel = setta andatura treno tracciato 2: {150,170,190,210,230}");

  sendOutput("ta|s = setta stato treno A: {'0','1'}");
  sendOutput("       0 ->entra");
  sendOutput("       1 ->esci");

  sendOutput("tb|s = setta stato treno B: {'0','1'}");
  sendOutput("       0 ->entra");
  sendOutput("       1 ->esci");

  sendOutput("tc|s = setta stato treno C: {'0','1'}");
  sendOutput("       0 ->entra");
  sendOutput("       1 ->esci");

  sendOutput("l1|s = setta luci stazione : {'0','1'}");
  sendOutput("       0 ->spente");
  sendOutput("       1 ->accese");

  sendOutput("ss| = stampa a video lo stato del sistema");
  sendOutput("sr| = reset del sistema");
  sendOutput("pa| = panic button");
  sendOutput("lc| = stampa a video lista comandi");
  sendOutput("vi| = vietato indicare i personaggi");

  sendOutput("_________________________________________________");
}

void panicButton() {

  // arresto tutto
  sendOutput("No Panic!!!");



  myTrains[0].stato = 0;
  myTrains[1].stato = 0;
  myTrains[2].stato = 0;

  // tracciati spenti
  fermaTreno(1);
  fermaTreno(2);


  myTrains[0].lastcommand = 0;
  myTrains[1].lastcommand = 0;
  myTrains[2].lastcommand = 0;

  digitalWrite(mySwitch[0].T_IN1, LOW);
  digitalWrite(mySwitch[0].T_IN2, LOW);
  digitalWrite(mySwitch[1].T_IN1, LOW);
  digitalWrite(mySwitch[1].T_IN2, LOW);

}

void systemReset() {

  // setto valori default:
  sendOutput("System Reset!!!");

  // scambi: diritti

  // forzo stato scambi
  mySwitch[0].stato = 0;
  mySwitch[1].stato = 0;

  scambia(1, "diritto");
  scambia(2, "diritto");

  // tracciati spenti
  fermaTreno(1);
  fermaTreno(2);

  myTrains[0].lastcommand = 0;
  myTrains[1].lastcommand = 0;
  myTrains[2].lastcommand = 0;

  myTrains[0].stato = 0;
  myTrains[1].stato = 0;
  myTrains[2].stato = 0;

  // default semafori
  settaSemaforo(LT1_incrocioA, LT1_incrocioB, "verde");
  settaSemaforo(LT2_incrocioA, LT2_incrocioB, "verde");

  // altri semafori dipende dalla posizione partenza (da definire)
  settaSemaforo(LT1_binario1A, LT1_binario1B, "rosso");
  settaSemaforo(LT1_binario2A, LT1_binario2B, "rosso");
  settaSemaforo(LT2_binario1A, LT2_binario1B, "rosso");

  // luci stazione ON
  accendiLuci(ST_lucipin);

}

void executeCommand(String command, String value)
{

  value = value.substring(0, value.length());
  sendOutput("Input: command->" + command + ", valore->" + value + "");

  if (command == "vi") sendVietatoIndicareMsg();

  if (command == "ta") {

    status = value.toInt();

    switch (status) {
      case 0:

        myTrains[0].stato = 2;
        entraTreno(1);
        // setta lastcommand (fermati alla stazione)
        myTrains[0].lastcommand = 0;
        break;
      case 1:
        //esciTreno(1);
        // setta lastcommand (esci dalla stazione)
        myTrains[0].lastcommand = 1;
        break;
    }

  }

  if (command == "tb") {

    status = value.toInt();

    switch (status) {
      case 0:
        myTrains[1].stato = 2;
        entraTreno(2);
        // setta lastcommand (fermati alla stazione)
        myTrains[1].lastcommand = 0;
        break;
      case 1:
        esciTreno(2);
        myTrains[1].lastcommand = 1;
        break;
    }

  }

  if (command == "tc") {

    status = value.toInt();

    switch (status) {
      case 0:
        myTrains[2].stato = 2;
        entraTreno(3);
        // setta lastcommand (fermati alla stazione)
        myTrains[2].lastcommand = 0;
        break;
      case 1:
        esciTreno(3);
        myTrains[2].lastcommand = 1;
        break;
    }

  }

  if (command == "l1") {

    status = value.toInt();

    switch (status) {
      case 0:
        spegniLuci(ST_lucipin);
        break;
      case 1:
        accendiLuci(ST_lucipin);
        break;
    }

  }

  if (command == "va1") {

    speedA1 = value.toInt();
    myTrains[0].speedT = value.toInt();

    analogWrite(T1_ENA, speedA1);
    sendOutput("Setto Treno 1 tracciato 1 -> speed = " + value);

  }

  if (command == "va2") {

    speedA2 = value.toInt();
    myTrains[1].speedT = speedA2;
    analogWrite(T1_ENA, speedA2);
    sendOutput("Setto Treno 2 tracciato 1 -> speed = " + value);

  }

  if (command == "vb") {

    speedB = value.toInt();
    myTrains[2].speedT = speedB;
    analogWrite(T2_ENA, speedB);
    sendOutput("Setto Treno tracciato 2 -> speed = " + value);

  }

  if (command == "s1") {

    status = value.toInt();

    switch (status) {
      case 0:
        scambia(1, "diritto");
        break;
      case 1:
        scambia(1, "scambia");
        break;
    }

  }

  if (command == "s2") {

    status = value.toInt();

    switch (status) {
      case 0:
        scambia(2, "diritto");
        break;
      case 1:
        scambia(2, "scambia");
        break;
    }

  }

  if (command == "ss") {
    printSystemStatus();
  }

  if (command == "sr") {
    systemReset();
  }

  if (command == "pa") {
    panicButton();
  }

  if (command == "lc") {
    printLegenda();
  }

}

void sendOutput(String message) {

  if (verbose) {
    Serial.println(message);
  }

}

// uso interno private
void printSystemStatus() {

  command = "";

  for (int i = 0; i < MY_TRAIN_LEN; i++) {
    command += myTrains[i].codice	+ ":" + 	String(myTrains[i].stato) + ",";
  }

  for (int i = 0; i < MY_TRACK_LEN; i++) {
    command += myTrack[i].codice  + ":" +  String(myTrack[i].stato) + ",";
  }


  for (int i = 0; i < MY_SWITCH_LEN; i++) {
    command += mySwitch[i].codice  + ":" + String(mySwitch[i].stato) + ",";
  }

  command =  command.substring(0, command.length() - 1);
  sendOutput('{' + command + '}');


}

// uso interno (private)
void partiTreno(int idtrack, int speedT) {

  // setta velocità del treno

  analogWrite(myTrack[idtrack - 1].T_ENA, speedT);

  // the train will move forward
  digitalWrite(myTrack[idtrack - 1].T_IN1, LOW);
  digitalWrite(myTrack[idtrack - 1].T_IN2, HIGH);

  myTrack[idtrack - 1].stato = 1;

}

// uso interno (private)
void fermaTreno(int idtrack) {

  // the train will stop
  digitalWrite(myTrack[idtrack - 1].T_IN1, LOW);
  digitalWrite(myTrack[idtrack - 1].T_IN2, LOW);

  myTrack[idtrack - 1].stato = 0;

}

// uso interno (private)
void invertiTreno(int idtrack) {

  // the train will move in reverse
  digitalWrite(myTrack[idtrack - 1].T_IN1, HIGH);
  digitalWrite(myTrack[idtrack - 1].T_IN2, LOW);
  myTrack[idtrack - 1].stato = -1;

}


// uso interno (private)
void scambia(int idswitch, String direction) {

  if (direction == "scambia") {
    if  (mySwitch[idswitch - 1].stato != 1) {

      digitalWrite(mySwitch[idswitch - 1].T_IN1, HIGH);
      digitalWrite(mySwitch[idswitch - 1].T_IN2, LOW);
      delay(interval_scambio);
      digitalWrite(mySwitch[idswitch - 1].T_IN1, LOW);
      digitalWrite(mySwitch[idswitch - 1].T_IN2, LOW);

      mySwitch[idswitch - 1].stato = 1;

      sendOutput("Scambio " + mySwitch[idswitch - 1].codice + " settato come " + direction);
    } else {
      sendOutput("Scambio " + mySwitch[idswitch - 1].codice + " era gia' settato come " + direction);
    }
  }

  if (direction == "diritto" ) {
    if (mySwitch[idswitch - 1].stato != 0) {
      digitalWrite(mySwitch[idswitch - 1].T_IN1, LOW);
      digitalWrite(mySwitch[idswitch - 1].T_IN2, HIGH);
      delay(interval_scambio);
      digitalWrite(mySwitch[idswitch - 1].T_IN2, LOW);
      digitalWrite(mySwitch[idswitch - 1].T_IN1, LOW);

      mySwitch[idswitch - 1].stato = 0;

      sendOutput("Scambio " + mySwitch[idswitch - 1].codice + " settato come " + direction);
    } else {
      sendOutput("Scambio " + mySwitch[idswitch - 1].codice + " era gia' settato come " + direction);
    }
  }

}

// uso interno (private)
void settaSemaforo(int pinsemaforoA, int pinsemaforoB, String colore) {

  if (colore == "rosso") {
    digitalWrite(pinsemaforoA, LOW);
    digitalWrite(pinsemaforoB, HIGH);
  } else {
    digitalWrite(pinsemaforoA, HIGH);
    digitalWrite(pinsemaforoB, LOW);
  }

}

// uso esterno (public)
void accendiLuci(int pinluce) {

  digitalWrite(pinluce, HIGH);
  sendOutput("Luci stazione accese");

}

// uso esterno (public)
void spegniLuci(int pinluce) {

  digitalWrite(pinluce, LOW);
  sendOutput("Luci stazione spente");

}

// uso interno (private)
void controllaSensore(int treno, int stazionevalue, boolean &F_isavaiable) {
  // usa lastcommand per verificare ultima azione assegnata al treno
  // nel caso lo fa entrare (o fermare)

  // leggo valore sensore
  if (stazionevalue == HIGH) {

    // se  sboccata lettura del sensore
    if (F_isavaiable == true) {

      // blocca successive letture
      F_isavaiable = false;

      sendOutput("Leggo sensore treno " + myTrains[treno - 1].codice);

      // se sta entrando treno
      if (myTrains[treno - 1].lastcommand == 0) {
        // ferma il treno
        fermaTreno(myTrains[treno - 1].tracciato);

        scambia(myTrains[treno - 1].scambio, "diritto");
        myTrains[treno - 1].stato = 0;

        sendOutput("Treno " + myTrains[treno - 1].codice + " entrato al binario");
      }

    }
  }
  else {
    // sblocco successive letture
    F_isavaiable = true;
  }

}

// uso esterno (public)
void esciTreno(int treno) {

  //appoggio=myTrains[treno-1].tracciato;

  // todo: controlla se altro treno su stesso tracciato è in giro, nel caso annulla comando
  //if (myTrack[appoggio-1].stato==1){
  if (myTrack[myTrains[treno - 1].tracciato - 1].stato == 1) {
    if (myTrains[treno - 1].stato == 1) {
      myTrains[treno - 1].lastcommand = 0;
      myTrains[treno - 1].stato = 0;
      sendOutput("Il treno " + myTrains[treno - 1].codice + " risulta partito, comando annullato.");
    } else {
      sendOutput("Sul tracciato risulta un treno in movimento, comando annullato.");
      myTrains[treno - 1].lastcommand = 0;
      myTrains[treno - 1].stato = 0;
    }
  } else {

    sendOutput("Faccio uscire treno " + myTrains[treno - 1].codice);

    // semaforo
    settaSemaforo(myTrains[treno - 1].semaforoA, myTrains[treno - 1].semaforoB, "verde");

    // scambia lo scambio per uscire (se scambio associato > 0 altrimenti non ha scambio)
    if (myTrains[treno - 1].scambio > 0) scambia(myTrains[treno - 1].scambio, "scambia");

    // muove treno
    partiTreno(myTrains[treno - 1].tracciato, myTrains[treno - 1].speedT);

    // aspetta passaggio locomotore su binario principale
    previousMillis_uscita = millis();    //save the time
    scaduto = false;

    //setta stato treno
    myTrains[treno - 1].stato = 1;

    sendOutput("Treno " + myTrains[treno - 1].codice + " uscito!");
  }

}


// uso esterno (public)
void entraTreno(int treno) {

  //controlla se treno già occupa il binario
  if (myTrains[treno - 1].stato == 0) {
    sendOutput("Il treno " + myTrains[treno - 1].codice + " risulta fermo al binario, comando annullato.");
  } else {

    sendOutput("Faccio entrare treno " + myTrains[treno - 1].codice);

    // semaforo
    settaSemaforo(myTrains[treno - 1].semaforoA, myTrains[treno - 1].semaforoB, "rosso");

    // scambia lo scambio per entrare (se scambio associato > 0 altrimenti non ha scambio)
    // comando trasferito nel sensore incrocio

    //if (myTrains[treno-1].scambio>0) scambia(myTrains[treno-1].scambio,"scambia");

    // il resto lo farà il sensore incaricato al passaggio del treno al binario
    // vedi funzione controllaSensore
  }

}
