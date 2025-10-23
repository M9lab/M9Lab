
void doCountdown(int colorIndex){
  
 for(int num=9; num>-1; num--){      
      char b[1];
      String str;
      str=String(num);
      str.toCharArray(b,1);
      osCopyChar(str[0],colorIndex);  
      delay(1000);  
   }  
   
}

void fullColor(uint32_t color){
  
 for(int num=0; num<NUM_LEDS; num++) {    
    leds[num] = color;
    FastLED.show();     
  }
  
}

void osCopyChar (char myChar, int colorIndex){
  
  myChar -= charStart;
  for (int i=0; i<5; i++)
    for (int j=0; j<5; j++)
    {
      uint32_t typex = (bitRead(chargen[myChar][i], j) == 0x00) ? CRGB::Black : colour[colorIndex];
      int pos = (i*5)+j;
      leds[XYTable[pos]] = typex;
      FastLED.show(); 
    }
    
}

void rulette(){
  
  _println("rulette");  
  SN = 1;
  while (SN<SM) {
    float SDMex = (log10(864)/SM)*SN; //the exponent part --86400 is the number of sec in 1 day
    SDM = pow(10,SDMex); // the sampling exponential delay    
    fullColor(colour[SN % 3]);
    delay(SDM);     
    SN++;  
  }
     
}
