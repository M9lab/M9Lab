#include <legopowerfunctions.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

/* 
Modificare pipe e ch a seconda del treno utilizzato 
T1:0xE8E8F0F0E1LL, CH1 T2:0xE8E8F0F0E2LL T3:0xE8E8F0F0E3LL, CH3 T4:0xE8E8F0F0E4LL,CH4 
*/
 

const uint64_t pipe = 0xE8E8F0F0E3LL;
const int channel = CH3;
LEGOPowerFunctions lego(2);
int msg[2];
RF24 radio(9,10);

void setup(void){
 Serial.begin(9600);
  radio.begin();
// radio.setRetries(15,15);
 //radio.setPayloadSize(8);
 radio.startListening();
 radio.openReadingPipe(1,pipe);
}

void loop(void){
 if (radio.available()){
   bool done = false;    
   while (!done){
    done = radio.read(msg, sizeof(msg));      
    //Serial.println(msg[0]);
    //Serial.println(msg[1]);
    lego.SingleOutput(0, msg[1], msg[0], channel);         
  
     }
 } 
 else{
  //Serial.println("No radio available");
  }
}
