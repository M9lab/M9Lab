uint32_t SDM = 0; //Sampling delay in ms
uint32_t SN = 0; // Sampling number
int SM = 50;   // maximum sampling number

void setup() {
Serial.begin (115200);
Serial.print("Start");
}
 
void loop() { 
  rulette();
}

void rulette(){
  if (++SN > SM)SN = 1;
  float SDMex = (log10(864)/SM)*SN; //the exponent part --86400 is the number of sec in 1 day
  SDM = pow(10,SDMex); // the sampling exponential delay
  Serial.print("Sample No: ");
  Serial.print(SN);
  Serial.print("\tDelay: ");
  Serial.println(SDM);
  delay(SDM);
}
