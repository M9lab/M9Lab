/*
    Board: ESP32-WROOM-DA Module
    Library: 
      esp32 -> 3.3.3 (board)
      M5Atom -> 0.1.3
      Adafruit_GFX -> 1.12.4
*/


// === M9LAB DISPLAY SLAVE - v2.0 BT PASSIVO ===
// Modalità: SERVER PASSIVO (aspetta connessioni dal Master)
// Separatore comandi: | (pipe) per compatibilità con orari HH:MM
// Il Master si connette on-demand per inviare comandi

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <BluetoothSerial.h>
#include "icone_meteo.h"
#include "logo_m9lab.h"

#define TFT_CS    15
#define TFT_DC     2
#define TFT_RST    4
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_BL    32

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define TFT_WIDTH  172
#define TFT_HEIGHT 320

// Colori ottimizzati per display ferroviario
#define COLORE_SFONDO     ST77XX_BLACK
#define COLORE_TESTO      ST77XX_YELLOW
#define COLORE_BINARIO    ST77XX_WHITE
#define COLORE_ETICHETTE  0x39C7

// Variabili per lo scrolling
String infoScorrente = "ATTENZIONE E' VIETATO INDICARE I PERSONAGGI";
int scrollX = 320;
int scrollSpeed = 2;
unsigned long lastScrollTime = 0;
int scrollDelay = 50;
bool displayMeteoAttivo = false;
bool firstCommandReceived = false;  // Flag: primo comando ricevuto
bool infoLabelDrawn = false;         // Flag: label "informazioni" già disegnata

// === BLUETOOTH SLAVE (PASSIVO) ===
BluetoothSerial SerialBT;
bool btConnected = false;
unsigned long lastBTCheck = 0;
char btInputBuffer[128];

// Modalità: TRUE SLAVE (passivo) - aspetta che il Master si connetta on-demand
// Il Master apre connessione, invia comando, chiude → nessun conflitto audio!

// === VERBOSE MODE (debug dettagliato) ===
bool verboseMode = false;  // OFF di default per performance ottimali

// === DEBUG HELPER FUNCTIONS ===
// Funzioni per debug condizionale (solo se verboseMode=true)
void debugPrint(const char* str) {
  if (verboseMode) Serial.print(str);
}

void debugPrint(const __FlashStringHelper* str) {
  if (verboseMode) Serial.print(str);
}

void debugPrint(int val) {
  if (verboseMode) Serial.print(val);
}

void debugPrint(float val) {
  if (verboseMode) Serial.print(val);
}

void debugPrint(String str) {
  if (verboseMode) Serial.print(str);
}

void debugPrintln(const char* str) {
  if (verboseMode) Serial.println(str);
}

void debugPrintln(const __FlashStringHelper* str) {
  if (verboseMode) Serial.println(str);
}

void debugPrintln(int val) {
  if (verboseMode) Serial.println(val);
}

void debugPrintln(String str) {
  if (verboseMode) Serial.println(str);
}

void debugPrintln() {
  if (verboseMode) Serial.println();
}

// Prototipi
void mostraSplashScreen();
void disegnaDisplay(String, String, String, String, String, String, String);
void disegnaMeteo(String, String, String, int, float, int);
const unsigned char* getIconaMeteo(int);
String getDescrizioneMeteo(int);

// ========== FUNZIONI BLUETOOTH SLAVE PASSIVO ==========

bool initBluetooth() {
  Serial.println("📡 Inizializzazione Bluetooth SLAVE PASSIVO...");
  
  // Inizializzazione BT in modalità SLAVE PASSIVO
  // false = slave (aspetta connessioni invece di connettersi attivamente)
  if (!SerialBT.begin("M9Lab-Display-Slave", false)) {
    Serial.println("❌ ERRORE: Impossibile inizializzare BT!");
    return false;
  }
  
  // Abilita SSP per pairing automatico
  SerialBT.enableSSP();
  
  Serial.println("✅ BT Slave inizializzato: M9Lab-Display-Slave");
  Serial.println("   Modalità: SERVER PASSIVO");
  
  // Stampa MAC address per configurazione Master
  String macAddr = SerialBT.getBtAddressString();
  Serial.print("   MAC Display: ");
  Serial.println(macAddr);  
  Serial.println("   In attesa connessioni dal Master...");
  
  btConnected = false;
  
  return true;
}

void checkBluetoothConnection() {
  // Modalità PASSIVA: monitora solo lo stato, NON tenta connessioni attive
  bool currentlyConnected = SerialBT.hasClient();
  
  // Debug: mostra stato ogni volta che cambia
  static bool lastState = false;
  if (currentlyConnected != lastState) {
    debugPrint("BT hasClient: ");
    debugPrintln(currentlyConnected ? "TRUE" : "FALSE");
    lastState = currentlyConnected;
  }
  
  if (currentlyConnected && !btConnected) {
    // Master si è connesso
    Serial.println("✅ MASTER CONNESSO");  // Importante: sempre visibile
    debugPrintln("   Ricevo comandi...");
    btConnected = true;
    
  } else if (!currentlyConnected && btConnected) {
    // Master si è disconnesso (normale dopo invio comando on-demand)
    Serial.println("🔌 Master disconnesso");  // Importante: sempre visibile
    debugPrintln("   In attesa prossimo comando...");
    btConnected = false;
  }
  
  // PASSIVO: nessun tentativo di riconnessione attiva
  // Il Master si connetterà quando necessario
}

void processBluetoothCommand(char* cmd) {
  debugPrint("📥 Comando ricevuto: ");
  debugPrintln(cmd);
  
  // Comando ALERT - non fare nulla
  if (strcmp(cmd, "ALERT") == 0) {
    debugPrintln("  → Comando ALERT ignorato");
    return;
  }
  
  // Comando METEO - formato: METEO|temperatura|weathercode|citta|orario|vento
  // Separatore | per evitare conflitto con : nell'orario (HH:MM)
  if (strncmp(cmd, "METEO|", 6) == 0) {
    // Parse con separatore |
    char* token = strtok(cmd + 6, "|");
    
    // Temperatura come numero (senza simbolo, lo disegneremo dopo)
    String temp = token ? String(token) : "21";
    
    token = strtok(NULL, "|");
    int code = token ? atoi(token) : 0;
    
    token = strtok(NULL, "|");
    String citta = token ? String(token) : "TRIESTE";
    
    token = strtok(NULL, "|");
    String orario = token ? String(token) : "12:00";
    
    token = strtok(NULL, "|");
    float vento = token ? atof(token) : 0.0;
    
    debugPrint("  → Meteo: ");
    debugPrint(citta);
    debugPrint(" ");
    debugPrint(temp);
    debugPrint(" code=");
    debugPrint(code);
    debugPrint(" vento=");
    debugPrint(vento);
    debugPrint(" km/h orario=");
    debugPrintln(orario);
    
    tft.fillScreen(COLORE_SFONDO);
    disegnaMeteo(citta, orario, temp, code, vento, 0);
    displayMeteoAttivo = true;
    firstCommandReceived = true;  // Primo comando ricevuto
    infoLabelDrawn = false;       // Reset per quando tornerà schermata treno
    return;
  }
  
  // Comando TRAIN - formato: TRAIN|dest|bin|linea1|linea2|orario|tipo
  // Separatore | per evitare conflitto con : nell'orario (HH:MM)
  if (strncmp(cmd, "TRAIN|", 6) == 0) {
    debugPrintln("  → Comando TRAIN ricevuto");
    
    // Parse con separatore |
    char* token = strtok(cmd + 6, "|");
    String destinazione = token ? String(token) : "DESTINAZIONE";
    
    token = strtok(NULL, "|");
    String binario = token ? String(token) : "1";
    
    token = strtok(NULL, "|");
    String trenoLinea1 = token ? String(token) : "TRENO";
    
    token = strtok(NULL, "|");
    String trenoLinea2 = token ? String(token) : "REG 8101";
    
    token = strtok(NULL, "|");
    String orario = token ? String(token) : "00:00";
    
    token = strtok(NULL, "|");
    String tipoOrario = token ? String(token) : "partenza";
    
    // Rimuovi eventuali caratteri di fine riga (\n, \r) dal tipo
    tipoOrario.trim();
    
    debugPrint("  → Treno: ");
    debugPrint(trenoLinea1);
    debugPrint(" ");
    debugPrint(trenoLinea2);
    debugPrint(" bin.");
    debugPrint(binario);
    debugPrint(" orario=");
    debugPrint(orario);
    debugPrint(" tipo=[");
    debugPrint(tipoOrario);
    debugPrintln("]");  // Mostra tipo tra [] per vedere spazi nascosti
    
    tft.fillScreen(COLORE_SFONDO);
    infoScorrente = "ATTENZIONE E' VIETATO INDICARE I PERSONAGGI";
    disegnaDisplay(destinazione, binario, trenoLinea1, trenoLinea2, orario, tipoOrario, infoScorrente);
    displayMeteoAttivo = false;
    firstCommandReceived = true;  // Primo comando ricevuto
    infoLabelDrawn = false;       // Reset flag per ridisegnare label "informazioni"
    scrollX = 320;                // Reset posizione scroll
    return;
  }
  
  // Comando non riconosciuto
  debugPrintln("  ⚠️  Comando non riconosciuto");
}

// ========== SPLASH SCREEN ==========

void mostraSplashScreen() {
  tft.fillScreen(COLORE_SFONDO);
  
  // Logo centrato (100x100)
  int logoX = (TFT_HEIGHT - LOGO_M9LAB_WIDTH) / 2;
  int logoY = 20;
  
  // Disegna logo pixel per pixel (formato RGB565)
  for (int y = 0; y < LOGO_M9LAB_HEIGHT; y++) {
    for (int x = 0; x < LOGO_M9LAB_WIDTH; x++) {
      int idx = y * LOGO_M9LAB_WIDTH + x;
      uint16_t color565 = pgm_read_word(&logo_m9lab[idx]);
      tft.drawPixel(logoX + x, logoY + y, color565);
    }
  }
  
  // Testo sotto il logo
  int textY = logoY + LOGO_M9LAB_HEIGHT + 20;
  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  
  String msg = "MEZZANINELAB";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  int textX = (TFT_HEIGHT - w) / 2;
  
  tft.setCursor(textX, textY);
  tft.print(msg);
  
  delay(2000);
}

void setup() {
  // Inizializza Serial per debug
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== M9LAB DISPLAY SLAVE ===");
  
  // Inizializza display
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(150);

  SPI.begin(TFT_SCLK, -1, TFT_MOSI);
  tft.init(TFT_WIDTH, TFT_HEIGHT);
  tft.setRotation(1);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  Serial.println("Display OK");

  // Mostra splash screen e mantienilo fino al primo comando
  mostraSplashScreen();
  
  // Inizializza Bluetooth (il logo resta visibile)
  if (!initBluetooth()) {
    // Solo in caso di errore, sovrascrivi con messaggio
    tft.fillScreen(COLORE_SFONDO);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.setCursor(10, 60);
    tft.print("ERRORE BT!");
  }
  
  // Logo splash rimane visibile fino al primo comando TRAIN o METEO
  // che chiamerà tft.fillScreen() per disegnare il contenuto
  displayMeteoAttivo = false;
}

// Display tabellone ferroviario semplificato
void disegnaDisplay(String destinazione, String binario, String trenoLinea1, String trenoLinea2, String orario, String tipoOrario, String infoScrolling) {
  int screenWidth = TFT_HEIGHT;
  int y = 3;

  // Pulisci area destra (dove va "bin." e "partenza/arrivo")
  // per evitare sovrapposizioni con testi precedenti
  tft.fillRect(screenWidth - 80, 0, 80, 100, COLORE_SFONDO);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, y);
  tft.print("destinazione");
  
  tft.setCursor(screenWidth - 60, y);
  tft.print("bin.");
  
  y += 22;  // +2px padding-bottom per fare spazio alla label "informazioni" più grande

  // Pulisci area destinazione prima di scrivere
  tft.fillRect(0, y, screenWidth - 50, 24, COLORE_SFONDO);

  tft.setTextSize(3);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(5, y);
  tft.print(destinazione);

  tft.setTextSize(4);
  tft.setCursor(screenWidth - 40, y - 3);
  tft.print(binario);

  y += 30;
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 8;

  // PULISCI TUTTA LA RIGA prima di scrivere (risolve sovrapposizione partenza/arrivo)
  tft.fillRect(0, y, screenWidth, 20, COLORE_SFONDO);
  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, y);
  tft.print("treno");
  
  // Scrivi tipo orario a destra
  int tipoW = tipoOrario.length() * 12;
  tft.setCursor(screenWidth - tipoW - 5, y);
  tft.print(tipoOrario);
  
  y += 20;
  int trenoStartY = y;
  
  // Pulisci area sinistra per nome treno
  tft.fillRect(0, y, screenWidth - 100, 40, COLORE_SFONDO);
  
  tft.setTextSize(2);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(3, y);
  tft.print(trenoLinea1);
  
  y += 18;
  tft.setCursor(3, y);
  tft.print(trenoLinea2);

  tft.setTextSize(3);
  int orarioW = orario.length() * 18;
  tft.setCursor(screenWidth - orarioW - 5, trenoStartY + 4);
  tft.print(orario);

  y += 20;
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
}

// ========== FUNZIONI METEO ==========

// Converte il codice meteo nell'icona corrispondente
const unsigned char* getIconaMeteo(int weatherCode) {
  switch (weatherCode) {
    case 0: return icon_sole;
    case 1: case 2: case 3: return icon_nuvoloso;
    case 45: case 48: return icon_nebbia;
    case 51: case 53: case 55:
    case 61: case 63: case 65:
    case 80: case 81: case 82: return icon_pioggia;
    case 95: case 96: case 99: return icon_temporale;
    default: return icon_variabile;
  }
}

// Disegna il display meteo semplificato
void disegnaMeteo(String citta, String orario = "16:44", String temperatura = "21C", int condizioniMeteo = 0, float vento = 0.0, int direzioneVento = 0) {
  int screenWidth = TFT_HEIGHT;
  
  tft.fillScreen(COLORE_SFONDO);
  
  int y = 5;
  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, y);
  tft.print(citta);
  
  int orarioWidth = orario.length() * 12;
  tft.setCursor(screenWidth - orarioWidth - 10, y);
  tft.print(orario);
  
  y += 22;
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 12;
  
  const unsigned char* icona = getIconaMeteo(condizioniMeteo);
  
  int iconX = 30;
  for (int j = 0; j < 32; j++) {
    for (int i = 0; i < 32; i++) {
      int byteIndex = j * 4 + (i / 8);
      int bitIndex = 7 - (i % 8);
      bool pixelOn = (pgm_read_byte(&icona[byteIndex]) >> bitIndex) & 1;
      
      if (pixelOn) {
        tft.fillRect(iconX + i * 2, y + j * 2, 2, 2, COLORE_BINARIO);
      }
    }
  }
  
  // Disegna temperatura: numero grande + "C" piccola in alto
  tft.setTextSize(8);
  tft.setTextColor(COLORE_TESTO);
  int tempX = iconX + 90;
  tft.setCursor(tempX, y);
  tft.print(temperatura);  // Solo il numero
  
  // Calcola larghezza del numero per posizionare la "C"
  int numWidth = temperatura.length() * 48;  // 48 = larghezza approssimativa char size 8
  
  // Disegna "C" piccola in alto a destra del numero
  tft.setTextSize(3);  // "C" più piccola
  tft.setCursor(tempX + numWidth, y + 5);  // +5 per allinearla in alto
  tft.print("C");
  
  y += 66;
  
  String descrizione = getDescrizioneMeteo(condizioniMeteo);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  int descWidth = descrizione.length() * 12;
  int descX = (screenWidth - descWidth) / 2;
  tft.setCursor(descX, y - 2);
  tft.print(descrizione);
  
  y += 20;
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 8;
  
  tft.setTextSize(2);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(10, y);
  tft.print("Vento: ");
  tft.print((int)vento);
  tft.print(" km/h");
}

// Ottiene la descrizione testuale delle condizioni meteo
String getDescrizioneMeteo(int weatherCode) {
  switch (weatherCode) {
    case 0: return "SERENO";
    case 1: case 2: case 3: return "NUVOLOSO";
    case 45: case 48: return "NEBBIA";
    case 51: case 53: case 55: return "PIOGGERELLA";
    case 61: case 63: case 65:
    case 80: case 81: case 82: return "PIOGGIA";
    case 95: case 96: case 99: return "TEMPORALE";
    default: return "VARIABILE";
  }
}

void loop() {
  // Check BT più frequente (50ms) per catturare connessioni on-demand rapide
  if (millis() - lastBTCheck > 50) {
    checkBluetoothConnection();
    lastBTCheck = millis();
  }
  
  if (SerialBT.available()) {
    // Se ci sono dati, il Master è sicuramente connesso
    if (!btConnected) {
      btConnected = true;
      Serial.println("✅ MASTER CONNESSO (dati ricevuti)");
    }
    
    int idx = 0;
    unsigned long timeout = millis() + 100;
    
    while (SerialBT.available() && idx < 127 && millis() < timeout) {
      char c = SerialBT.read();
      if (c == '\n' || c == '\r') break;
      btInputBuffer[idx++] = c;
      delay(1);
    }
    btInputBuffer[idx] = '\0';
    
    while (SerialBT.available()) SerialBT.read();
    
    if (idx > 0) {
      processBluetoothCommand(btInputBuffer);
    }
  }
  
  // === COMANDI VIA SERIAL (per debug) ===
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    
    if (cmd.equalsIgnoreCase("verbose") || cmd.startsWith("verbose=")) {
      if (cmd.startsWith("verbose=")) {
        int val = cmd.substring(8).toInt();
        verboseMode = (val != 0);
      } else {
        verboseMode = !verboseMode;  // Toggle
      }
      Serial.print("🔊 Verbose mode: ");
      Serial.println(verboseMode ? "ON" : "OFF");
    }
    else if (cmd.equalsIgnoreCase("status")) {
      Serial.println("=== STATUS DISPLAY ===");
      Serial.print("BT Connected: ");
      Serial.println(btConnected ? "YES" : "NO");
      Serial.print("Verbose: ");
      Serial.println(verboseMode ? "ON" : "OFF");
      Serial.print("Display mode: ");
      Serial.println(displayMeteoAttivo ? "METEO" : "TRENO");
      Serial.println("======================");
    }
  }
  
  // Non disegnare scrolling se non è stato ricevuto il primo comando
  // (mantieni logo splash visibile)
  if (displayMeteoAttivo || !firstCommandReceived) {
    delay(10);  // Previene watchdog reset quando in attesa del primo comando
    yield();
    return;
  }
  
  unsigned long currentTime = millis();
  
  if (currentTime - lastScrollTime > scrollDelay) {
    lastScrollTime = currentTime;
    
    int screenWidth = TFT_HEIGHT;
    int screenHeight = TFT_WIDTH;
    int labelY = screenHeight - 36;     // Più in basso per label più grande
    int scrollY = screenHeight - 16;    // Scritta scorrevole 2px più in basso
    
    // Disegna label "informazioni" solo una volta (non ad ogni frame)
    if (!infoLabelDrawn) {
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_CYAN);
      tft.setCursor(5, labelY);
      tft.print("informazioni");
      infoLabelDrawn = true;
    }
    
    // Cancella e ridisegna SOLO l'area del testo scrollante (non la label sopra)
    int scrollAreaY = scrollY - 2;        // Inizia leggermente sopra il testo
    int scrollAreaHeight = 20;            // Altezza solo del testo scrollante (size 2 = ~16px + margine)
    tft.fillRect(0, scrollAreaY, screenWidth, scrollAreaHeight, COLORE_SFONDO);
    
    // Testo scorrevole
    tft.setTextSize(2);
    tft.setTextColor(COLORE_TESTO);
    tft.setCursor(scrollX, scrollY);
    tft.print(infoScorrente);
    
    scrollX -= scrollSpeed;
    
    int textWidth = infoScorrente.length() * 12;
    if (scrollX < -textWidth) {
      scrollX = screenWidth;
    }
  }
  
  // Previene watchdog reset
  yield();
}
