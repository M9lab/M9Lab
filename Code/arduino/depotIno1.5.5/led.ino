
void doCountdown(int colorIndex){
  
  for(int num = 9; num >= 0; num--){      
    char digit = '0' + num;
    osCopyChar(digit, colorIndex);  
    delay(1000);  
  }  
   
}

void fullColor(uint32_t color){
  
  for(int num = 0; num < NUM_LEDS; num++) {    
    leds[num] = color;
  }
  FastLED.show();
  
}

void osCopyChar(char myChar, int colorIndex){
  
  myChar -= charStart;
  for (int i = 0; i < 5; i++) {
    byte charRow = pgm_read_byte(&chargen[myChar][i]);
    for (int j = 0; j < 5; j++) {
      uint32_t typex = (bitRead(charRow, j) == 0x00) ? CRGB::Black : colour[colorIndex];
      int pos = (i * 5) + j;
      byte xyPos = pgm_read_byte(&XYTable[pos]);
      leds[xyPos] = typex;
    }
  }
  FastLED.show();
    
}

void rulette(){
  
  if (isVerbose) Serial.println(F("rulette"));
  SN = 1;
  while (SN < SM) {
    float SDMex = (log10(864) / SM) * SN; //the exponent part --86400 is the number of sec in 1 day
    SDM = pow(10, SDMex); // the sampling exponential delay    
    fullColor(colour[SN % 3]);
    delay(SDM);     
    SN++;  
  }
     
}
