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
    leds[square[num]] = color;    
    //if (numto==9) delay(100);    
  }
  FastLED.show();

}

void osCopyChar (int myCharIndex, uint32_t color, byte arraytext[][5]){    
  for (int i=0; i<5; i++)
    for (int j=0; j<5; j++)
    {
      uint32_t typex = (bitRead(arraytext[myCharIndex][i], j) == 0x00) ? CRGB::Black : color;
      int pos = (i*5)+j;
      //leds[pos] = typex;
      leds[XYTable[pos]] = typex;      
      FastLED.show(); 
    }
    
}

void initDisplay(){

    // scritta trenino ??
    for (int num = 0; num < 7; num++) {
      int cnum = num < 4 ? num : num - 4;
      osCopyChar(num,maincolour[cnum],trenino);
      delay(300);	 
    }
    fullColor(CRGB::Black);	
    delay(1000);

    // by
    for (int num = 0; num < 1; num++) {      
      osCopyChar(num,CRGB::Red,by);
      delay(300);	 
    }
    fullColor(CRGB::Black);	
    delay(1000);

    // M9lab
    for (int num = 0; num < 5; num++) {      
      osCopyChar(num,CRGB::Teal,M9Lab);
      delay(300);	 
    }
    fullColor(CRGB::Black);	
    delay(1000);    



   // scritta ACOL (facoltativa)    
   /*
   for (int num = 0; num < TOTNUM_COLORS; num++) {
	   osCopyChar(num,maincolour[num],acol);

	   delay(300);	   
   }
   */
   
   fullColor(CRGB::Black);	   
   delay(1000);

   /* fine */
   for (int num = 0; num < TOTNUM_COLORS; num++) {	   
	   colorSquare(allsquares[num],maincolour[num],1,4);	   	      
   }   

}

void refreshLed(int num){   
   // 0 = connected, 1= color, 2= selected, 3=battery
   
   for (int i = 0; i < MY_TRAIN_LEN; i++) {
    uint32_t color = CRGB::Black;
    if (num==0){
      //if (myTrains[i].hubState == -1) color = CRGB::Black;
      if (myTrains[i].hubState == 0){
        color = myTrains[i].connectAttempt == 1 ? CRGB::White : CRGB::Purple;        
      }
      
      if (myTrains[i].hubState == 1) color = maincolour[i];      
    }
    if (num==2) color = currentActiveTrainOnRemote == i ? CRGB::White : maincolour[i];
    if (num==3) color = myTrains[i].batteryLevel > 10 ? maincolour[i] : CRGB::Purple;        
    colorSquare(myTrains[i].square,color,num,num+1);        
  }
}
