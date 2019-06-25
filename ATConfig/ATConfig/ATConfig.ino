
    #include <SoftwareSerial.h>
    int rxPin = 4;
    int txPin = 5;
    SoftwareSerial bluetooth(rxPin, txPin);
    void setup()
    {
      Serial.begin(9600);
      bluetooth.begin(38400);
      Serial.println("Comunicazione inizializzata, Lista comandi per modulo HC-06:");
      Serial.println("AT              Se la comunicazione funziona il modulo risponde OK");
      Serial.println("AT+VERSION      Restituisce la versione del firmware");
      Serial.println("AT+BAUDx        Imposta il Baudrate, al posto di x mettere 1 per 1200 bps, 2=2400, 3=4800, 4=9600, 5=19200, 6=38400, 7=57600, 8=115200, 9=230400, A=460800, B=921600, C=1382400");
      Serial.println("AT+NAMEstring   Al posto di string mettere il nome che vuoi dare al modulo (massimo 20 caratteri)");
      Serial.println("AT+PINxxxx      Imposta il pincode del modulo bluetooth (es.1234)");
    }
    void loop()
    {
      if (bluetooth.available())
      {  
        Serial.write(bluetooth.read());
      }
      if (Serial.available())
      {
        bluetooth.write(Serial.read());
      }
    }
