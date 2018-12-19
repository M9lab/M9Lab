// Questo sketch gestisce l'audio per la stazione multimediale di Trenino 2.0
// leggo ed eseguo da serial il comando in formato treno|binario //ex: TA|1, TA|2, TB|1, TB|2 ....
// Utilizza il DFPlayer e la sua libreria (da caricare)


// Info: caricare i file in SD:/MP3/0001.mp3, SD:/MP3/0002.mp3 ecc ecc
// esempio file audio da creare (treno in arrivo):
// "TA|1" -> Il treno Freccia Bianca 60051 è in arrivo al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0001.mp3
// "TA|2" -> Il treno Freccia Bianca 60051 è in arrivo al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0002.mp3
// "TB|1" -> Il treno Railway Express 4561 è in arrivo al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0003.mp3
// "TB|2" -> Il treno Railway Express 4561 è in arrivo al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0004.mp3
// "TB|1" -> Il treno Railway Express 4559 è in arrivo al binario 1, allontanarsi dalla linea gialla -> SD:/MP3/0003.mp3
// "TB|2" -> Il treno Railway Express 4559 è in arrivo al binario 2, allontanarsi dalla linea gialla -> SD:/MP3/0004.mp3
//Il treno Freccia Bianca, 60 0 5 1 , delle ore 15 e 03  è in arrivo al binario 1,. Attenzione, allontanarsi dalla linea gialla. 
// Cargo Railway
// TODO -> audio treno in partenza, poi vediamo ;)

// NOte: il bluetooth per ora è commentato, nel caso basta abilitarlo e configurarlo.


// 2018 Code by Stefx 
// ver 1.0.1 

#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include "RTClib.h"

RTC_DS3231 rtc;

SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
//SoftwareSerial bluetooth(rxPin, txPin);

DFRobotDFPlayerMini myDFPlayer;


void setup(){
	
	Serial.begin(9600);
	//bluetooth.begin(9600);  
			
	//Use softwareSerial to communicate with mp3.		
	if (!myDFPlayer.begin(mySoftwareSerial)) {  
		Serial.println(F("Unable to begin:"));
		Serial.println(F("1.Please recheck the connection!"));
		Serial.println(F("2.Please insert the SD card!"));
		while(true);
	}
	Serial.println(F("DFPlayer Mini online."));	
	
	myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms  
	myDFPlayer.volume(10);  //Set volume value (0~30).		
	myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
	myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);		
	  
}

void loop(){   

	// leggo ed eseguo da serial il comando in formato treno|binario //ex: TA|1, TA|2 ....
	if (Serial.available())
	{						
		executeAudio(Serial.readString());	
	}
			
	/* 		
	if (bluetooth.available())
	{        
		executeAudio(bluetooth.readString());				
	} 
	*/
	
	// debug su serial degli eventi del lettore mp3 (opzionale, da commentare)
	if (myDFPlayer.available()) {
		//Print the detail message from DFPlayer to handle different errors and states.
		printDetail(myDFPlayer.readType(), myDFPlayer.read()); 
	}	
	
}


void printDetail(uint8_t type, int value){
	switch (type) {
	case TimeOut:
	  Serial.println(F("Time Out!"));
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
	  break;
	case DFPlayerUSBInserted:
	  Serial.println("USB Inserted!");
	  break;
	case DFPlayerUSBRemoved:
	  Serial.println("USB Removed!");
	  break;
	case DFPlayerPlayFinished:
	  Serial.print(F("Number:"));
	  Serial.print(value);
	  Serial.println(F(" Play Finished!"));
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
		  break;
		case Advertise:
		  Serial.println(F("In Advertise"));
		  break;
		default:
		  break;
	  }
	  break;
	default:
	  break;
}

void executeAudio(String command){
	
	
	DateTime now = rtc.now();
	int currentMinute = convertIntTo2DigitString(now.minute());
	int currentHour = now.hour(); 

	switch (command) {			
		case "TA|1":				         
			myDFPlayer.playMp3Folder(1); //play specific mp3 in SD:/MP3/0001.mp3; File Name(0~65535)	
			break;
		case "TA|2":				         
			myDFPlayer.playMp3Folder(2); //play specific mp3 in SD:/MP3/0002.mp3; File Name(0~65535)	
			break;				
		case "TB|1":				         
			myDFPlayer.playMp3Folder(3); //play specific mp3 in SD:/MP3/0003.mp3; File Name(0~65535)	
			break;								
		case "TB|2":				         
			myDFPlayer.playMp3Folder(4); //play specific mp3 in SD:/MP3/0004.mp3; File Name(0~65535)	
			break;															
	}
			
}

String convertIntTo2DigitString(int i)  {
  String s = String(i);
  if (i < 10)  {
    s = '0'+s;
  }
  return s;
}