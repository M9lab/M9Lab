#include <SPI.h>
#include "RF24.h"
#include <legopowerfunctions.h>


RF24 myRadio (7, 8);
byte addresses[][6] = {"1Node","2Node","3Node","4Node"};

String nonce;

struct package
{
	int id=0;	
	int motorchcolor = RED;
	int speedx = PWM_BRK;
	/*char  text[100] = "Text to be transmitted";*/
};

typedef struct package Package;
Package data;

void setup(void){
	Serial.begin(9600);
	
	myRadio.begin();
	myRadio.setChannel(115); 
	myRadio.setPALevel(RF24_PA_MAX);
	myRadio.setDataRate( RF24_250KBPS ) ;  	
	
}

void loop(void){  

    if (Serial.available() > 0)
    {    
      
      nonce = Serial.readString();   
      Serial.println(nonce);   

      if ('1' == nonce.charAt(0)){                            
        sendRXToTrain(addresses[0],RED,PWM_FWD3);        
        delay (3000);
        sendRXToTrain(addresses[0],RED,PWM_BRK);    		
      }

      if ('2' == nonce.charAt(0)){                            
        sendRXToTrain(addresses[1],RED,PWM_FWD3);        
        delay (3000);
        sendRXToTrain(addresses[1],RED,PWM_BRK);    		
      }             
      
      if ('3' == nonce.charAt(0)){                            
        sendRXToTrain(addresses[2],RED,PWM_FWD3);        
        delay (3000);
        sendRXToTrain(addresses[2],RED,PWM_BRK);    		
      }     

      if ('4' == nonce.charAt(0)){                            
        sendRXToTrain(addresses[3],RED,PWM_FWD3);      
        delay (3000);
        sendRXToTrain(addresses[3],RED,PWM_BRK);      
      }        
      
    }
    
}


void sendRXToTrain(byte pipe,int motorchcolor, int speedx){
  
	myRadio.openWritingPipe(pipe);
	
	data.id = data.id + 1;
	data.motorchcolor = data.motorchcolor;
	data.speedx = data.speedx;  
	
    radio.write(data,sizeof(data));
  
}
 
