#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include <legopowerfunctions.h>
int msg[2];
RF24 radio(9,10);

const uint64_t pipe1 = 0xE8E8F0F0E1LL;
const uint64_t pipe2 = 0xE8E8F0F0E2LL;
const uint64_t pipe3 = 0xE8E8F0F0E3LL;
const uint64_t pipe4 = 0xE8E8F0F0E4LL;

String nonce;
char channel;
int speedx;

void setup(void){
 Serial.begin(9600);
 radio.begin();
 //radio.setRetries(15,15);
 //radio.setPayloadSize(8);
 //radio.openWritingPipe(pipe);
}

void loop(void){  

    //lego.SingleOutput(0, PWM_FWD3, trains[idtreno].motorchcolor, trains[idtreno].channel);
    // { 4, "10219", 4, CH4, RED , BLUE,0}
    // PWM_FWD3

 while (Serial.available() == 0){
   // Serial.println("aspetto");        
  } 

    if (Serial.available() > 0)
    {    
      
      nonce = Serial.readString();   
       
/*
      if ('t' == nonce.charAt(0)){   
        radio.openWritingPipe(pipe1);       
        Serial.println(nonce);        
        msg[0] = CH1 + PWM_FWD3 ;
        radio.write(msg, 1);
      }
*/

       Serial.println(nonce);   

      if ('1' == nonce.charAt(0)){                            
        sendRXToTrain(pipe1,RED,PWM_FWD3);        
      }

      if ('2' == nonce.charAt(0)){                            
        sendRXToTrain(pipe2,RED,PWM_FWD3);        
      }             
      
      if ('3' == nonce.charAt(0)){                            
        sendRXToTrain(pipe3,RED,PWM_FWD3);        
      }     

      if ('4' == nonce.charAt(0)){                            
        sendRXToTrain(pipe4,RED,PWM_FWD3);      
        delay (1000);
         sendRXToTrain(pipe4,RED,PWM_BRK);      
      }        
      
    }
    
}

//int idtreno
// lego.SingleOutput(0, PWM_BRK, trains[idtreno].motorchcolor, trains[idtreno].channel);   


void sendRXToTrain(uint64_t pipe,int motorchcolor, int speedx){

  radio.openWritingPipe(pipe); 
  msg[0] = motorchcolor;
  msg[1] = speedx ;
  radio.write(msg,sizeof(msg));
  
}
 
