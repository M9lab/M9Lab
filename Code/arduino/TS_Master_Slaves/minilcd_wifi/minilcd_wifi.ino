// === M9LAB DISPLAY SLAVE - v2.0 WiFi TCP ===
// Modalità: WiFi TCP CLIENT (si connette al Master via WiFi)
// Separatore comandi: | (pipe) per compatibilità con orari HH:MM
// Connessione TCP al Master IP

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <WiFi.h>
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
String infoScorrente = "WELCOME TO M9LAB TRAINSTATION";
int scrollX = 320;
int scrollSpeed = 2;
unsigned long lastScrollTime = 0;
int scrollDelay = 50;
bool displayMeteoAttivo = false;

// === WIFI TCP CLIENT ===
const char* wifiSSID = "StefxMobile";
const char* wifiPWD = "qwerty123456";
const char* masterIP = "10.248.101.102";  // IP del Master
const int masterPort = 8888;

WiFiClient tcpClient;
bool wifiConnected = false;
bool tcpConnected = false;
bool tcpConnecting = false;  // Flag per indicare connessione in corso
unsigned long lastTCPCheck = 0;
unsigned long lastBlinkTime = 0;
char tcpInputBuffer[128];

// Prototipi
void mostraSplashScreen();
void disegnaDisplay(String, String, String, String, String, String, String);
void disegnaMeteo(String, String, String, int, float, int);
const unsigned char* getIconaMeteo(int);
String getDescrizioneMeteo(int);

// ========== FUNZIONI WIFI TCP CLIENT ==========

bool initWiFi() {
  Serial.println("🌐 Connessione WiFi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPWD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(" FALLITO!");
    return false;
  }
  
  Serial.println(" OK!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  wifiConnected = true;
  
  return true;
}

bool connectToMaster() {
  Serial.print("📡 Connessione a Master ");
  Serial.print(masterIP);
  Serial.print(":");
  Serial.println(masterPort);
  
  tcpConnecting = true;
  
  if (tcpClient.connect(masterIP, masterPort)) {
    Serial.println("✅ CONNESSO AL MASTER!");
    tcpConnected = true;
    tcpConnecting = false;
    return true;
  }
  
  Serial.println("❌ Connessione fallita");
  tcpConnected = false;
  tcpConnecting = false;
  return false;
}

void checkTCPConnection() {
  // Controlla ogni 2 secondi
  if (millis() - lastTCPCheck < 2000) {
    return;
  }
  lastTCPCheck = millis();
  
  bool currentlyConnected = tcpClient.connected();
  
  if (currentlyConnected && !tcpConnected) {
    // Connessione stabilita
    Serial.println("✅ TCP CONNESSO AL MASTER");
    tcpConnected = true;
    tcpConnecting = false;
    
  } else if (!currentlyConnected && tcpConnected) {
    // Connessione persa - RIPROVA AUTOMATICAMENTE
    Serial.println("⚠️  TCP DISCONNESSO DAL MASTER");
    Serial.println("   Tento riconnessione...");
    tcpConnected = false;
    tcpConnecting = true;
    
    // Riprova connessione
    tcpClient.stop();
    delay(1000);
    bool reconnected = tcpClient.connect(masterIP, masterPort);
    
    if (reconnected) {
      Serial.println("✅ Riconnesso!");
      tcpConnected = true;
      tcpConnecting = false;
    } else {
      Serial.println("⚠️  Riconnessione fallita, riproverò");
      tcpConnecting = false;
    }
  }
  
  // Se non connesso, riprova periodicamente
  if (!currentlyConnected && !tcpConnected) {
    static unsigned long lastRetry = 0;
    if (millis() - lastRetry > 5000) {  // Ogni 5 secondi
      lastRetry = millis();
      Serial.println("Riprovo connessione TCP...");
      tcpConnecting = true;
      tcpClient.stop();
      bool connected = tcpClient.connect(masterIP, masterPort);
      if (connected) {
        Serial.println("✅ Connesso!");
        tcpConnected = true;
        tcpConnecting = false;
      } else {
        tcpConnecting = false;
      }
    }
  }
}

void processTCPCommand(char* cmd) {
  Serial.print("📥 Comando ricevuto: ");
  Serial.println(cmd);
  
  // Comando ALERT - non fare nulla
  if (strcmp(cmd, "ALERT") == 0) {
    Serial.println("  → Comando ALERT ignorato");
    return;
  }
  
  // Comando METEO - formato: METEO|temperatura|weathercode|citta|orario
  // Separatore | per evitare conflitto con : nell'orario (HH:MM)
  if (strncmp(cmd, "METEO|", 6) == 0) {
    // Parse con separatore |
    char* token = strtok(cmd + 6, "|");
    
    // Temperatura arrotondata
    String temp = token ? String(token) + "C" : "21C";
    
    token = strtok(NULL, "|");
    int code = token ? atoi(token) : 0;
    
    token = strtok(NULL, "|");
    String citta = token ? String(token) : "TRIESTE";
    
    token = strtok(NULL, "|");
    String orario = token ? String(token) : "12:00";
    
    Serial.print("  → Meteo: ");
    Serial.print(citta);
    Serial.print(" ");
    Serial.print(temp);
    Serial.print(" code=");
    Serial.print(code);
    Serial.print(" orario=");
    Serial.println(orario);
    
    tft.fillScreen(COLORE_SFONDO);
    disegnaMeteo(citta, orario, temp, code, 0.0, 0);
    displayMeteoAttivo = true;
    return;
  }
  
  // Comando TRAIN - formato: TRAIN|dest|bin|linea1|linea2|orario|tipo
  // Separatore | per evitare conflitto con : nell'orario (HH:MM)
  if (strncmp(cmd, "TRAIN|", 6) == 0) {
    Serial.println("  → Comando TRAIN ricevuto");
    
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
    
    Serial.print("  → Treno: ");
    Serial.print(trenoLinea1);
    Serial.print(" ");
    Serial.print(trenoLinea2);
    Serial.print(" bin.");
    Serial.print(binario);
    Serial.print(" orario=");
    Serial.print(orario);
    Serial.print(" ");
    Serial.println(tipoOrario);
    
    tft.fillScreen(COLORE_SFONDO);
    infoScorrente = "ANNUNCIO TRENO DA M9LAB TRAINSTATION";
    disegnaDisplay(destinazione, binario, trenoLinea1, trenoLinea2, orario, tipoOrario, infoScorrente);
    displayMeteoAttivo = false;
    return;
  }
  
  // Comando non riconosciuto
  Serial.println("  ⚠️  Comando non riconosciuto");
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
  Serial.println("\n=== M9LAB DISPLAY WiFi TCP CLIENT ===");
  
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

  // Mostra splash screen
  mostraSplashScreen();
  
  // Inizializza WiFi
  tft.fillScreen(COLORE_SFONDO);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, 10);
  tft.print("WiFi TCP");
  tft.setCursor(10, 30);
  tft.print("Client...");
  
  if (initWiFi()) {
    tft.setTextColor(ST77XX_GREEN);
    tft.setCursor(10, 60);
    tft.print("WiFi OK");
    
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 90);
    tft.print("Connessione al");
    tft.setCursor(10, 105);
    tft.print("Master...");
    
    connectToMaster();
    
  } else {
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 60);
    tft.print("ERRORE WiFi!");
  }
  
  delay(3000);
  
  // TEST VISIVO: Schermo rosso per 2 secondi
  Serial.println("🧪 TEST: Schermo rosso...");
  tft.fillScreen(ST77XX_RED);
  delay(2000);
  
  // TEST VISIVO: Testo grande
  Serial.println("🧪 TEST: Testo grande...");
  tft.fillScreen(COLORE_SFONDO);
  tft.setTextSize(4);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 60);
  tft.print("TEST");
  delay(2000);
  
  // Mostra schermata iniziale
  Serial.println("🎨 Rendering schermata iniziale...");
  tft.fillScreen(COLORE_SFONDO);
  disegnaDisplay("TRIESTE", "1", "MEZZANINELAB", "REG 8101", "12:00", "partenza", infoScorrente);
  displayMeteoAttivo = false;
  Serial.println("✅ Setup completato! Display dovrebbe mostrare TRIESTE/MEZZANINELAB");
  Serial.println("   Aspetto 5 secondi prima di entrare nel loop...");
  delay(5000);  // Aspetta 5 secondi per vedere la schermata iniziale
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
  
  y += 20;

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

  // Pulisci area destra per "partenza/arrivo" prima di scrivere
  tft.fillRect(screenWidth - 120, y, 120, 16, COLORE_SFONDO);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, y);
  tft.print("treno");
  
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
  
  tft.setTextSize(8);
  tft.setTextColor(COLORE_TESTO);
  tft.setCursor(iconX + 90, y);
  tft.print(temperatura);
  
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
  // Check TCP
  checkTCPConnection();
  
  // PALLINO STATUS TCP SEMPRE VISIBILE (anche su schermata meteo)
  // Verde = connesso | Giallo lampeggiante = connessione in corso | Rosso = disconnesso
  static unsigned long lastDotUpdate = 0;
  static bool yellowBlink = false;
  
  if (millis() - lastDotUpdate > 500) {  // Aggiorna ogni 500ms
    lastDotUpdate = millis();
    yellowBlink = !yellowBlink;  // Toggle per lampeggio
    
    int dotX = TFT_HEIGHT - 8;
    int dotY = 5;
    uint16_t dotColor;
    
    if (tcpConnected) {
      dotColor = ST77XX_GREEN;    // Connesso (fisso)
    } else if (tcpConnecting) {
      // Giallo lampeggiante (alterna giallo/nero)
      dotColor = yellowBlink ? ST77XX_YELLOW : COLORE_SFONDO;
    } else {
      dotColor = ST77XX_RED;      // Disconnesso (fisso)
    }
    
    tft.fillCircle(dotX, dotY, 1, dotColor);  // Raggio 1px = diametro 2px
  }
  
  // Leggi comandi TCP
  if (tcpClient.available()) {
    String line = tcpClient.readStringUntil('\n');
    line.trim();
    
    Serial.print("📥 TCP RAW: ");
    Serial.println(line);
    
    if (line.length() > 0 && line.length() < 128) {
      line.toCharArray(tcpInputBuffer, sizeof(tcpInputBuffer));
      processTCPCommand(tcpInputBuffer);
    }
  }
  
  if (displayMeteoAttivo) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - lastScrollTime > scrollDelay) {
    lastScrollTime = currentTime;
    
    int screenWidth = TFT_HEIGHT;
    int screenHeight = TFT_WIDTH;
    int labelY = screenHeight - 28;
    int scrollY = screenHeight - 16;
    int areaHeight = 35;
    
    tft.fillRect(0, labelY, screenWidth, areaHeight, COLORE_SFONDO);
    
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(5, labelY);
    tft.print("informazioni");
    
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
}
