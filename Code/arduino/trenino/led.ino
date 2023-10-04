/* led */
void fullColor(uint32_t color) {

  for (int num = 0; num < NUM_LEDS; num++) {
    leds[num] = color;
    //leds[XYTable[num]] = color;
    FastLED.show();
  }

}

void colorSquare(int square[] ,uint32_t color, int numfrom , int numto) {

  for (int num = numfrom; num < numto; num++) {  
    
    //leds[XYTable[num]] = color;
    leds[square[num]] = color;    
    if (numto==9) delay(100);    
  }
  FastLED.show();

}

void osCopyChar (int myCharIndex, uint32_t color){    
  for (int i=0; i<5; i++)
    for (int j=0; j<5; j++)
    {
      uint32_t typex = (bitRead(chargen[myCharIndex][i], j) == 0x00) ? CRGB::Black : color;
      int pos = (i*5)+j;
      //leds[pos] = typex;
      leds[XYTable[pos]] = typex;      
      FastLED.show(); 
    }
    
}

void initDisplay(){
   // scritta ACOL (facoltativa) 
   for (int num = 0; num < TOTNUM_COLORS; num++) {
	   osCopyChar(num,maincolour[num]);
	   delay(500);	   
   }
   
   fullColor(CRGB::Black);	   
   delay(1000);

   /* fine */
   
   for (int num = 0; num < TOTNUM_COLORS; num++) {	   
	   colorSquare(allsquares[num],maincolour[num],1,4);	   
	   delay(250);	   
   }   

}