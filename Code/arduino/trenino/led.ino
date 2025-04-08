/* led */
void fullColor(uint32_t color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void colorSquare(int square[], uint32_t color, int numfrom, int numto) {
  for (int num = numfrom; num < numto; num++) {          
    leds[square[num]] = color;          
  }
  FastLED.show();
}

void osCopyChar(int myCharIndex, uint32_t color, byte arraytext[][5], uint32_t bgcolor) {   
  for (int i=0; i<5; i++) {
    for (int j=0; j<5; j++) {
      leds[XYTable[(i*5)+j]] = (bitRead(arraytext[myCharIndex][i], j) == 0x00) ? bgcolor : color;
    }
  }
  FastLED.show();
}

void initDisplay(){

  // M9lab
    for (int num = 0; num < 2; num++) {      
      osCopyChar(num,CRGB::Yellow,M9Lab,CRGB::Teal);
      delay(1000);   
    }

    fullColor(CRGB::Black); 
    delay(1000);

    for (int num = 0; num < 4; num++) {
      fullColor(maincolour[num]); 
      delay(150);
    }

    fullColor(CRGB::Black); 
    delay(1000);


    // scritta trenino ??
    for (int num = 0; num < 7; num++) {
      int cnum = num < 4 ? num : num - 4;
      osCopyChar(num,maincolour[cnum],trenino,CRGB::Black);
      delay(300);   
    }
    fullColor(CRGB::Black); 
    delay(1000);

    // by    
    /*
    for (int num = 0; num < 2; num++) {      
      osCopyChar(num,CRGB::Red,by,CRGB::Black);
      delay(1000);   
    }
    fullColor(CRGB::Black); 
    delay(1000);
    */
    
    
    fullColor(CRGB::Black); 
    delay(500);  
    
    
  /*
   // scritta ACOL (facoltativa)       
   for (int num = 0; num < TOTNUM_COLORS; num++) {
	   osCopyChar(num,CRGB::White,acol,maincolour[num]);

	   delay(300);	   
   }      
   fullColor(CRGB::Black);	   
   delay(1000);
   */

   /* fine */
   for (int num = 0; num < TOTNUM_COLORS; num++) {	   
	   colorSquare(allsquares[num],maincolour[num],1,4);	  
     delay(300); 	      
   }   

}

void refreshLed(int num){   
   // 0 = connected, 1= color, 2= selected, 3=battery
   
   for (int i = 0; i < MY_TRAIN_LEN; i++) {
    uint32_t color = CRGB::Black;
    if (num==0){
      
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
