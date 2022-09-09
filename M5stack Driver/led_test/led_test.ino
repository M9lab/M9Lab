#include <FastLED.h>
#define NUM_LEDS 25
#define DATA_PIN 27

uint32_t colour[] = {CRGB::Black, CRGB::Red, CRGB::Green, CRGB::Yellow};

#define COLOR_NUM 4

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);   
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  
  FastLED.setBrightness(20);  
  
}

void loop() {
  
  for(int num=0; num<COLOR_NUM; num++) {  
    fullColor(colour[num]);
    delay(1000);
  }   
  
}

void fullColor(uint32_t color){

 for(int num=0; num<NUM_LEDS; num++) {    
    leds[num] = color;
    FastLED.show(); 
    //delay(50);
  }

}
