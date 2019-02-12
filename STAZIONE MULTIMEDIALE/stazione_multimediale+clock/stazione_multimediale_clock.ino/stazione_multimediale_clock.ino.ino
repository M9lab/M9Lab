// Questo sketch gestisce l'audio per la stazione multimediale di Trenino 2.0
// leggo ed eseguo da serial il comando in formato treno|binario|azione //ex: 111, 123
// Utilizza il DFPlayer e la sua libreria (da caricare)

// Info: caricare i file in SD:/MP3/0001.mp3, SD:/MP3/0002.mp3 ecc ecc
// esempio file audio da creare (:
// "111" -> Il treno Freccia Bianca 60051 è in arrivo al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0001.mp3
// "122" -> Il treno Freccia Bianca 60051 è in partenza al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0002.mp3
// "211" -> Il treno Railway Express 4561 è in arrivo al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0003.mp3
// "221" -> Il treno Railway Express 4561 è in arrivo al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0004.mp3
// "312" -> Il treno Railway Express 4559 è in partenza al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0003.mp3
// "321" -> Il treno Railway Express 4559 è in arrivo al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0004.mp3

// TODO -> Playlist senza delay -> OK

// 2019 Code by Stefx 
// ver 1.0.6

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "RTClib.h"
#include <Wire.h>

// settings
RTC_DS1307 rtc;
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

// vars annuncio
String sm_action;
String sm_track;
String sm_train;
bool sm_finished = true;
bool sm_ready = false;
int sm_playList[20] = {};
int sm_totalFile = 1;
int sm_idx = 0;
// end vars


void setup(){

  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
       
  if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
  }else{
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
    while(true);
  }
  Serial.println(F("DFPlayer Mini online.")); 

  // settaggi mp3
  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms  
  myDFPlayer.volume(25);  //Set volume value (0~30).    
  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);    
    
}

void loop(){   

  // leggo ed eseguo da serial il comando in formato treno|binario|azione //ex: 111
  if (Serial.available()){    
    String text = Serial.readString();
    executeAudioPlayList(text.toInt());    
  }      
  
  // debug su serial degli eventi del lettore mp3 (opzionale, da commentare)
  if (myDFPlayer.available()) {    
     printDetail(myDFPlayer.readType(), myDFPlayer.read());      
  } 

  if (sm_finished && sm_ready){    
    myDFPlayer.playMp3Folder(sm_playList[sm_idx++]);  //Play next mp3         
    sm_finished = false;
  } 

  // fine annuncio (ultimo mp3)
  if (sm_idx == sm_totalFile){    
    sm_idx = 0;
    sm_ready = false;

    
    Serial.println("fine annuncio");       
    // TODO in base a azione faccio uscire treno o lo faccio entrare
    Serial.println(sm_action);
  }    
  
}

void printDetail(uint8_t type, int value){ 
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

void executeAudioPlayList(int command){
  
  //Serial.print(command);
  String exx = String(command); // print the integer
  
  sm_train = exx.charAt(0); // 1,2,3
  sm_track = exx.charAt(1);    // 1,2,3....
  sm_action = exx.charAt(2);    // 1-parte 2-arriva

   
  DateTime now = rtc.now();
  int currentMinute = now.minute();
  int currentHour = now.hour(); 
     
  if (! rtc.isrunning()) {
    currentMinute = 22;
    currentHour = 17;
  }
  
  // reset
  sm_totalFile=0;
  //memset(sm_playList, sm_totalFile, sizeof(sm_playList));
  for( int i = 0; i < sizeof(sm_playList);  ++i ) sm_playList[i] = (char)0;

  // bip
  addToPlayList(111);

  // il treno
  addToPlayList(121);    

  // treno
  switch (sm_train.toInt()) {      
    //65001
    case 1:                 
      addToPlayList(201);
      addToPlayList(65);
      addToPlayList(100);
      addToPlayList(100);
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
  if (sm_action=="1"){    
    addToPlayList(141);    
  }  

  //e in partenza dal binario
  if (sm_action=="2"){    
    addToPlayList(146);    
  }  
  
  // n binario
  addToPlayList(sm_track.toInt());  
  
  //avviso finale
  addToPlayList(165); 

  // vietato indicare
  addToPlayList(191);
    
  // abilita playlist
  sm_ready = true;
  
}

int convertIntTo2DigitString(int i)  {
  if (i < 10)  {
    i = i+100;
  }
  return i;
}

void addToPlayList(int mp3){  
  sm_playList[sm_totalFile] = mp3;
  sm_totalFile++;
  
}

