#include <SPI.h>
#include "RF24.h"
#include <legopowerfunctions.h>

LEGOPowerFunctions lego(2);
RF24 myRadio (7, 8);

/* cambiare node e canale per ogni treno */
byte addresses[][6] = {"4Node"};
const int channel = CH4;
/* fine configurazione */


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
  myRadio.openReadingPipe(1, addresses[0]);
  myRadio.startListening();
}


void loop()  
{

  if ( myRadio.available()) 
  {
    while (myRadio.available())
    {
		myRadio.read( &data, sizeof(data) );
		lego.SingleOutput(0, data.motorchcolor, data.speedx, channel);    
    }
    Serial.print("\nPackage:");
    Serial.print(data.id);
    Serial.print("\n");	
    Serial.print(data.motorchcolor);
    Serial.print("\n");
    Serial.println(data.speedx);    
  }

}

