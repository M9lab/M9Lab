#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TimeLib.h>
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
#define COLORE_TESTO      ST77XX_YELLOW     // Giallo per info scorrevoli
#define COLORE_BINARIO    ST77XX_WHITE      // Bianco per testo principale
#define COLORE_ETICHETTE  0x39C7            // Grigio scuro per linee

// === NTP CONFIG ===
const char* ntpServer = "0.it.pool.ntp.org";
const char* wifiSSID = "StefxMobile";
const char* wifiPWD = "qwerty123456";
const long gmtOffset_sec = 3600;        // UTC+1 per Italia

// Variabili per lo scrolling
String infoScorrente = "";
int scrollX = 320;
int scrollSpeed = 2;
unsigned long lastScrollTime = 0;
int scrollDelay = 50;
bool displayMeteoAttivo = false;  // Flag per distinguere display meteo da tabellone

// Variabili meteo
float temperaturaCorrente = 21.0;
int weatherCodeCorrente = 0;
float windSpeed = 0.0;
int windDirection = 0;
String cittaMeteo = "TRIESTE";  // Fallback, verrà aggiornato da reverse geocoding

// Coordinate GPS (il nome città verrà recuperato automaticamente da queste coordinate)
float meteoLatitude = 45.65;  // Default: Trieste
float meteoLongitude = 13.77;

// === BLUETOOTH SLAVE ===
BluetoothSerial SerialBT;
bool btConnected = false;
unsigned long lastBTCheck = 0;
unsigned long btReconnectInterval = 5000;  // Riprova connessione ogni 5 secondi
char btInputBuffer[128];

// ========== PROTOTIPI FUNZIONI ==========
void mostraSplashScreen();
void disegnaDisplay(String destinazione, String binario, String trenoLinea1, String trenoLinea2, String orario, String tipoOrario, String infoScrolling);
void disegnaMeteo(String citta, String orario, String temperatura, int condizioniMeteo, float vento, int direzioneVento);
const unsigned char* getIconaMeteo(int weatherCode);
String getDescrizioneMeteo(int weatherCode);
String getWindDirection(int degrees);
bool syncTimeWithNTP();
bool getMeteo(float &temp, int &weatherCode, float &wind, int &windDir);
bool getCityNameFromCoords(float lat, float lon, String &cityName);
int getDaylightOffset();
void printTime(Stream &s);

// ========== FUNZIONI BLUETOOTH SLAVE ==========

bool initBluetooth() {
  Serial.println(F("🔵 Inizializzazione BT Slave..."));
  
  if (!SerialBT.begin("M9Lab-Display-Slave")) {
    Serial.println(F("❌ Errore inizializzazione BT"));
    return false;
  }
  
  Serial.println(F("✅ BT Slave attivo: M9Lab-Display-Slave"));
  Serial.println(F("In attesa di connessione dal master..."));
  return true;
}

void checkBluetoothConnection() {
  bool currentlyConnected = SerialBT.connected();
  
  if (currentlyConnected && !btConnected) {
    // Appena connesso
    Serial.println(F("✅ Master connesso via BT"));
    btConnected = true;
  } else if (!currentlyConnected && btConnected) {
    // Appena disconnesso
    Serial.println(F("⚠️  Master disconnesso"));
    btConnected = false;
  }
}

void processBluetoothCommand(char* cmd) {
  Serial.print(F("📥 Ricevuto comando: "));
  Serial.println(cmd);
  
  // Comando ALERT - non fare nulla
  if (strcmp(cmd, "ALERT") == 0) {
    Serial.println(F("➡️  Comando ALERT ignorato"));
    return;
  }
  
  // Comando METEO
  if (strcmp(cmd, "METEO") == 0) {
    Serial.println(F("➡️  Aggiorno display meteo..."));
    
    // Aggiorna ora corrente
    String orarioStr = String(hour()) + ":" + (minute() < 10 ? "0" : "") + String(minute());
    String tempStr = String((int)temperaturaCorrente) + "C";
    
    tft.fillScreen(COLORE_SFONDO);
    disegnaMeteo(cittaMeteo, orarioStr, tempStr, weatherCodeCorrente, windSpeed, windDirection);
    displayMeteoAttivo = true;  // Disabilita scroll
    Serial.println(F("✅ Display meteo aggiornato"));
    return;
  }
  
  // Comando TRAIN:dest:bin:linea1:linea2:orario:tipo
  if (strncmp(cmd, "TRAIN:", 6) == 0) {
    Serial.println(F("➡️  Aggiorno display treno..."));
    
    // Parsing parametri
    char* token = strtok(cmd + 6, ":");
    String destinazione = token ? String(token) : "DESTINAZIONE";
    
    token = strtok(NULL, ":");
    String binario = token ? String(token) : "1";
    
    token = strtok(NULL, ":");
    String trenoLinea1 = token ? String(token) : "TRENO";
    
    token = strtok(NULL, ":");
    String trenoLinea2 = token ? String(token) : "REG 8101";
    
    token = strtok(NULL, ":");
    String orario = token ? String(token) : "00:00";
    
    token = strtok(NULL, ":");
    String tipoOrario = token ? String(token) : "partenza";
    
    // Aggiorna display
    tft.fillScreen(COLORE_SFONDO);
    infoScorrente = "ANNUNCIO TRENO DA M9LAB TRAINSTATION";
    disegnaDisplay(destinazione, binario, trenoLinea1, trenoLinea2, orario, tipoOrario, infoScorrente);
    displayMeteoAttivo = false;  // Abilita scroll
    Serial.println(F("✅ Display treno aggiornato"));
    return;
  }
  
  Serial.println(F("⚠️  Comando non riconosciuto"));
}

// ========== FUNZIONI DI UTILITÀ ==========

// Calcola l'offset ora legale (DST) per l'Italia
int getDaylightOffset() {
  int m = month();
  int d = day();
  int dow = weekday();  // 1=domenica, 7=sabato
  
  // DST in Italia: ultima domenica di marzo alle 2:00 -> ultima domenica di ottobre alle 3:00
  if (m < 3 || m > 10) return 0;  // Gennaio, febbraio, novembre, dicembre: ora solare
  if (m > 3 && m < 10) return 3600;  // Aprile-settembre: ora legale
  
  // Marzo e ottobre: calcolo preciso
  int lastSunday = d - ((dow == 1) ? 0 : (dow - 1));
  while (lastSunday + 7 <= 31) lastSunday += 7;
  
  if (m == 3) return (d >= lastSunday) ? 3600 : 0;
  if (m == 10) return (d < lastSunday) ? 3600 : 0;
  
  return 0;
}

// Stampa l'ora formattata
void printTime(Stream &s) {
  s.printf("%02d:%02d:%02d", hour(), minute(), second());
}

// Converte i gradi del vento in direzione cardinale
String getWindDirection(int degrees) {
  // Normalizza tra 0-360
  degrees = degrees % 360;
  if (degrees < 0) degrees += 360;
  
  // 8 direzioni cardinali
  if (degrees >= 337 || degrees < 23) return "N";
  if (degrees >= 23 && degrees < 68) return "NE";
  if (degrees >= 68 && degrees < 113) return "E";
  if (degrees >= 113 && degrees < 158) return "SE";
  if (degrees >= 158 && degrees < 203) return "S";
  if (degrees >= 203 && degrees < 248) return "SW";
  if (degrees >= 248 && degrees < 293) return "W";
  if (degrees >= 293 && degrees < 337) return "NW";
  return "N";
}

// ========== FUNZIONI WiFi/NTP/METEO ==========

// Sincronizza l'orario con NTP
bool syncTimeWithNTP() {
  Serial.println(F("🌐 Sincronizzazione NTP..."));
  
  // Reset completo WiFi
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(500);
  
  // Attiva WiFi
  WiFi.mode(WIFI_STA);
  delay(500);
  
  Serial.print(F("MAC: "));
  Serial.println(WiFi.macAddress());
  Serial.print(F("Connessione a: "));
  Serial.println(wifiSSID);
  WiFi.begin(wifiSSID, wifiPWD);
  
  Serial.print(F("Connessione"));
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
    
    if (attempts % 5 == 0) {
      Serial.print(F(" ["));
      Serial.print(WiFi.status());
      Serial.print(F("]"));
    }
    yield();
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F(" FALLITO!"));
    Serial.print(F("❌ WiFi status: "));
    Serial.println(WiFi.status());
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  Serial.println(F(" OK!"));
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP());
  
  // Configura e sincronizza NTP
  Serial.println(F("Sincronizzo con NTP..."));
  configTime(gmtOffset_sec, 0, ntpServer);
  
  // Attendi sincronizzazione
  int ntpAttempts = 0;
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo) && ntpAttempts < 20) {
    delay(500);
    Serial.print(".");
    ntpAttempts++;
    yield();
  }
  
  if (ntpAttempts >= 20) {
    Serial.println(F(" TIMEOUT!"));
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    return false;
  }
  
  Serial.println(F(" OK!"));
  
  // Aggiorna TimeLib
  setTime(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
          timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
  
  // Calcola e applica DST
  int dstOffset = getDaylightOffset();
  if (dstOffset == 3600) {
    adjustTime(3600);
  }
  
  Serial.print(F("✅ Orario aggiornato: "));
  printTime(Serial);
  Serial.printf(" %02d/%02d/%04d", day(), month(), year());
  Serial.print(F(" ("));
  Serial.print(dstOffset == 3600 ? F("Ora legale +1h") : F("Ora solare"));
  Serial.println(F(")"));
  
  return true;
}

// Recupera meteo da Open-Meteo usando coordinate GPS parametriche
bool getMeteo(float &temp, int &weatherCode, float &wind, int &windDir) {
  Serial.print(F("🌤️ Recupero meteo per "));
  Serial.print(cittaMeteo);
  Serial.print(F(" ("));
  Serial.print(meteoLatitude, 2);
  Serial.print(F(", "));
  Serial.print(meteoLongitude, 2);
  Serial.println(F(")..."));
  
  // Attiva WiFi se spento
  bool wifiWasOff = (WiFi.getMode() == WIFI_OFF);
  if (wifiWasOff) {
    WiFi.mode(WIFI_STA);
    delay(500);
  }
  
  // Connetti WiFi se non già connesso
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Connessione WiFi..."));
    WiFi.begin(wifiSSID, wifiPWD);
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
      yield();
    }
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println(F(" FALLITO!"));
      if (wifiWasOff) WiFi.mode(WIFI_OFF);
      return false;
    }
    Serial.println(F(" OK"));
  }
  
  // Chiamata API Open-Meteo con coordinate parametriche
  String url = "http://api.open-meteo.com/v1/forecast?latitude=" + String(meteoLatitude, 2) + 
               "&longitude=" + String(meteoLongitude, 2) + "&current_weather=true";
  Serial.print(F("URL: "));
  Serial.println(url);
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  
  Serial.println(F("Invio richiesta HTTP..."));
  int httpCode = http.GET();
  Serial.printf("Codice HTTP ricevuto: %d\n", httpCode);
  
  bool success = false;
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print(F("Payload ricevuto ("));
    Serial.print(payload.length());
    Serial.println(F(" bytes)"));
    Serial.println(F("--- INIZIO JSON ---"));
    Serial.println(payload);
    Serial.println(F("--- FINE JSON ---"));
    
    // Parse JSON
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      JsonObject weather = doc["current_weather"];
      
      if (!weather.isNull()) {
        temp = weather["temperature"];
        weatherCode = weather["weathercode"];
        wind = weather["windspeed"];          // Velocità vento in km/h
        windDir = weather["winddirection"];   // Direzione in gradi
        
        Serial.printf("✅ Meteo: %.1f°C, code=%d, vento=%.1f km/h da %d° (%s)\n", 
                      temp, weatherCode, wind, windDir, getWindDirection(windDir).c_str());
        success = true;
      } else {
        Serial.println(F("❌ Dati meteo non trovati nel JSON"));
        Serial.println(F("Struttura JSON ricevuta:"));
        serializeJsonPretty(doc, Serial);
      }
    } else {
      Serial.print(F("❌ Errore parsing JSON: "));
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("❌ HTTP error: %d\n", httpCode);
    if (httpCode == HTTPC_ERROR_CONNECTION_REFUSED) {
      Serial.println(F("   Connessione rifiutata"));
    } else if (httpCode == HTTPC_ERROR_CONNECTION_LOST) {
      Serial.println(F("   Connessione persa"));
    } else if (httpCode == -1) {
      Serial.println(F("   Timeout o errore di connessione"));
    }
  }
  
  http.end();
  
  // Rispegni WiFi se era spento prima
  if (wifiWasOff) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println(F("WiFi disattivato"));
  }
  
  return success;
}

// Recupera nome città da coordinate GPS usando Nominatim (OpenStreetMap)
// OPZIONALE: chiamare solo se serve aggiornare il nome della città
bool getCityNameFromCoords(float lat, float lon, String &cityName) {
  Serial.print(F("🗺️  Reverse geocoding per ("));
  Serial.print(lat, 2);
  Serial.print(F(", "));
  Serial.print(lon, 2);
  Serial.println(F(")..."));
  
  // Verifica connessione WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("❌ WiFi non connesso"));
    return false;
  }
  
  // Chiamata API Nominatim (OpenStreetMap)
  // NOTA: Nominatim richiede un User-Agent valido
  String url = "http://nominatim.openstreetmap.org/reverse?format=json&lat=" + 
               String(lat, 6) + "&lon=" + String(lon, 6) + "&zoom=10&addressdetails=1";
  
  Serial.print(F("URL: "));
  Serial.println(url);
  
  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  http.addHeader("User-Agent", "M9LabArduinoESP32/1.0");  // User-Agent obbligatorio per Nominatim
  
  Serial.println(F("Invio richiesta HTTP..."));
  int httpCode = http.GET();
  Serial.printf("Codice HTTP ricevuto: %d\n", httpCode);
  
  bool success = false;
  
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.print(F("Payload ricevuto ("));
    Serial.print(payload.length());
    Serial.println(F(" bytes)"));
    
    // Parse JSON per estrarre il nome della città
    DynamicJsonDocument doc(2048);  // Nominatim risponde con JSON più grande
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      // Prova diversi campi per il nome città
      if (doc["address"]["city"]) {
        cityName = doc["address"]["city"].as<String>();
        success = true;
      } else if (doc["address"]["town"]) {
        cityName = doc["address"]["town"].as<String>();
        success = true;
      } else if (doc["address"]["village"]) {
        cityName = doc["address"]["village"].as<String>();
        success = true;
      } else if (doc["address"]["municipality"]) {
        cityName = doc["address"]["municipality"].as<String>();
        success = true;
      }
      
      if (success) {
        cityName.toUpperCase();  // Converti in maiuscolo
        Serial.print(F("✅ Città trovata: "));
        Serial.println(cityName);
      } else {
        Serial.println(F("❌ Nome città non trovato nel JSON"));
      }
    } else {
      Serial.print(F("❌ Errore parsing JSON: "));
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("❌ HTTP error: %d\n", httpCode);
  }
  
  http.end();
  return success;
}

// Mostra schermata di caricamento con logo
void mostraSplashScreen() {
  tft.fillScreen(COLORE_SFONDO);
  
  // Logo centrato (100x100)
  int logoX = (TFT_HEIGHT - LOGO_M9LAB_WIDTH) / 2;
  int logoY = 20;  // Più in alto per evitare taglio testo
  
  // Disegna logo pixel per pixel (formato RGB565)
  for (int y = 0; y < LOGO_M9LAB_HEIGHT; y++) {
    for (int x = 0; x < LOGO_M9LAB_WIDTH; x++) {
      int idx = y * LOGO_M9LAB_WIDTH + x;
      
      // Leggi colore RGB565 da PROGMEM (uint16_t)
      uint16_t color565 = pgm_read_word(&logo_m9lab[idx]);
      
      tft.drawPixel(logoX + x, logoY + y, color565);
    }
  }
  
  // Testo sotto il logo
  int textY = logoY + LOGO_M9LAB_HEIGHT + 20;
  
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  
  String msg = "Caricamento in corso...";
  int16_t x1, y1;
  uint16_t w, h;
  tft.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  int textX = (TFT_HEIGHT - w) / 2;
  
  tft.setCursor(textX, textY);
  tft.print(msg);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("\n\n=== MINI LCD DISPLAY ==="));
  
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

  // Mostra splash screen con logo M9LAB
  mostraSplashScreen();
  
  // ========== RECUPERO DATI ONLINE (METEO) ==========
  // All'avvio tenta sempre di connettersi e recuperare il meteo
  // Se fallisce, mostra un errore e passa al tabellone ferroviario
  Serial.println(F("\n🌐 Avvio connessione dati..."));
  
  // 1. Sincronizza orario con NTP
  bool timeOk = syncTimeWithNTP();
  
  // 2. Recupera nome città da coordinate GPS (reverse geocoding)
  if (WiFi.status() == WL_CONNECTED) {
    if (!getCityNameFromCoords(meteoLatitude, meteoLongitude, cittaMeteo)) {
      Serial.println(F("⚠️  Impossibile recuperare nome città, uso default"));
      // cittaMeteo rimane "TRIESTE" (valore default)
    }
  }
  
  // 3. Recupera meteo per la città identificata
  bool meteoOk = false;
  if (timeOk || WiFi.status() == WL_CONNECTED) {
    meteoOk = getMeteo(temperaturaCorrente, weatherCodeCorrente, windSpeed, windDirection);
  }
  
  // Disconnetti WiFi per risparmiare energia
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    Serial.println(F("✅ WiFi disattivato"));
  }
  
  // ========== MOSTRA IL DISPLAY ==========
  tft.fillScreen(COLORE_SFONDO);
  
  if (meteoOk && timeOk) {
    // ✅ SUCCESSO: Mostra display meteo con dati reali
    String orarioStr = String(hour()) + ":" + (minute() < 10 ? "0" : "") + String(minute());
    String tempStr = String((int)temperaturaCorrente) + "C";
    disegnaMeteo(cittaMeteo, orarioStr, tempStr, weatherCodeCorrente, windSpeed, windDirection);
    displayMeteoAttivo = true;  // Attiva display meteo
    Serial.println(F("✅ Display meteo attivo"));
    
  } else {
    // ❌ ERRORE: Mostra alert e poi passa al tabellone ferroviario
    Serial.println(F("❌ Connessione fallita, avvio display tabellone..."));
    
    // Mostra messaggio di errore per 3 secondi
    tft.fillScreen(COLORE_SFONDO);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor(10, 40);
    tft.print("ERRORE CONNESSIONE");
    
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_YELLOW);
    tft.setCursor(10, 70);
    if (!timeOk) {
      tft.println("WiFi non disponibile");
    }
    if (!meteoOk) {
      tft.println("Meteo non recuperato");
    }
    
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(10, 110);
    tft.print("Avvio tabellone...");
    
    delay(3000);  // Mostra errore per 3 secondi
    
    // Passa al display ferroviario (tabellone)
    tft.fillScreen(COLORE_SFONDO);
    String destinazione = "MF-TRIESTE";
    String binario = "1";
    String trenoLinea1 = "MEZZANINELAB";
    String trenoLinea2 = "REG 8101";
    String orarioStr = "16:44";  // Orario fisso in caso di errore
    String tipoOrario = "partenza";
    infoScorrente = "TANTI AUGURI DA BABBO NATALE";
    disegnaDisplay(destinazione, binario, trenoLinea1, trenoLinea2, orarioStr, tipoOrario, infoScorrente);
    displayMeteoAttivo = false;  // Attiva display tabellone (con scroll)
    Serial.println(F("✅ Display tabellone ferroviario attivo (modalità fallback)"));
  }
  
  // === INIZIALIZZA BLUETOOTH SLAVE ===
  Serial.println(F("\n--- Inizializzazione BT Slave ---"));
  if (initBluetooth()) {
    Serial.println(F("✅ BT Slave pronto"));
  } else {
    Serial.println(F("⚠️  BT Slave non disponibile (continuo senza)"));
  }
  Serial.println(F("---\n"));
  
  Serial.println(F("\n=== SETUP COMPLETATO ===\n"));
}

// ========== DISPLAY TABELLONE FERROVIARIO ==========
// Questa funzione può essere usata per altri scopi (es. modalità manuale, Bluetooth, etc.)
// Viene anche usata come fallback quando la connessione WiFi fallisce
void disegnaDisplay(String destinazione, String binario, String trenoLinea1, String trenoLinea2, String orario, String tipoOrario, String infoScrolling) {
  int screenWidth = TFT_HEIGHT;   // 320
  int screenHeight = TFT_WIDTH;   // 172

  int y = 3;

  // --- SEZIONE DESTINAZIONE ---
  // Etichetta "destinazione" (ciano) - INGRANDITA
  tft.setTextSize(2);  // Ingrandita da 1 a 2
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, y);
  tft.print("destinazione");
  
  // Etichetta "bin." - ALLINEATA A DESTRA (ciano)
  tft.setTextColor(ST77XX_CYAN);
  int binLabelWidth = 4 * 6 * 2;  // "bin." = 4 caratteri, size 2
  int binLabelX = screenWidth - binLabelWidth - 5;  // Allineato a destra con margine
  tft.setCursor(binLabelX, y);
  tft.print("bin.");
  
  y += 20;  // Aumentato padding tra label e testo (da 12 a 20)

  // DESTINAZIONE in grande (bianco) - ALLINEATA A SINISTRA
  tft.setTextSize(3);
  tft.setTextColor(COLORE_BINARIO);  // Bianco per maggior contrasto
  tft.setCursor(5, y);  // Allineata a sinistra
  tft.print(destinazione);

  // BINARIO in grande (bianco) - CENTRATO SOTTO "bin."
  tft.setTextSize(4);
  tft.setTextColor(COLORE_BINARIO);
  int binNumWidth = binario.length() * 6 * 4;
  int binLabelCenter = binLabelX + (binLabelWidth / 2);  // Centro della label "bin."
  int binNumX = binLabelCenter - (binNumWidth / 2);      // Centro il numero sotto la label
  tft.setCursor(binNumX, y - 3);
  tft.print(binario);

  y += 30;  // Più spazio prima della riga

  // Linea separatrice orizzontale
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 8;  // Più spazio dopo la riga (da 3 a 8)

  // --- SEZIONE TRENO ---
  // Etichetta "treno" (ciano) - INGRANDITA
  tft.setTextSize(2);  // Ingrandita da 1 a 2
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(5, y);
  tft.print("treno");
  
  // Etichetta "partenza" o "arrivo" (ciano) - PARAMETRICA E ALLINEATA A DESTRA
  int tipoOrarioWidth = tipoOrario.length() * 6 * 2;  // Calcolo dinamico, size 2
  tft.setCursor(screenWidth - tipoOrarioWidth - 5, y);
  tft.print(tipoOrario);
  
  y += 20;  // Aumentato padding tra label e testo (da 12 a 20)

  int trenoStartY = y;  // Salva la posizione iniziale delle righe treno
  
  // Nome treno LINEA 1 (bianco)
  tft.setTextSize(2);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(3, y);
  tft.print(trenoLinea1);
  
  y += 18;  // Aumentato spazio tra le due righe del treno
  
  // Nome treno LINEA 2 (bianco)
  tft.setTextSize(2);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(3, y);
  tft.print(trenoLinea2);

  // Orario (bianco) - PIÙ PICCOLO E ALLINEATO A DESTRA
  tft.setTextSize(3);  // Ridotto a 3
  tft.setTextColor(COLORE_BINARIO);
  int orarioWidth = orario.length() * 6 * 3;  // Corretto per size 3
  // Allineato a destra
  tft.setCursor(screenWidth - orarioWidth - 5, trenoStartY + 4);
  tft.print(orario);

  y += 20;  // Più spazio prima della riga

  // Linea separatrice
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 8;  // Più spazio dopo la riga
  
  // La sezione informazioni è ancorata in basso nel loop()
}

// ========== FUNZIONI METEO ==========

// Converte il codice meteo nell'icona corrispondente
const unsigned char* getIconaMeteo(int weatherCode) {
  switch (weatherCode) {
    case 0: return icon_sole; // 311 - cielo sereno
    case 1: case 2: case 3: return icon_nuvoloso; // 312 - parzialmente nuvoloso
    case 45: case 48: return icon_nebbia; // 314 - nebbia
    case 51: case 53: case 55: // drizzle
    case 61: case 63: case 65: // pioggia
    case 80: case 81: case 82: return icon_pioggia; // 313 - pioggia
    case 95: case 96: case 99: return icon_temporale; // 315 - temporale
    default: return icon_variabile; // 316 - condizioni variabili
  }
}

// Disegna il display meteo
void disegnaMeteo(String citta, String orario = "16:44", String temperatura = "21C", int condizioniMeteo = 0, float vento = 0.0, int direzioneVento = 0) {
  int screenWidth = TFT_HEIGHT;   // 320
  int screenHeight = TFT_WIDTH;   // 172
  
  // Cancella schermo
  tft.fillScreen(COLORE_SFONDO);
  
  int y = 5;
  
  // --- CITTÀ E ORARIO ---
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, y);
  tft.print(citta);
  
  // Orario allineato a destra
  int orarioWidth = orario.length() * 6 * 2;
  tft.setCursor(screenWidth - orarioWidth - 10, y);
  tft.print(orario);
  
  y += 22;
  
  // Linea separatrice
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 12;
  
  // --- ICONA METEO E TEMPERATURA AFFIANCATI ---
  const unsigned char* icona = getIconaMeteo(condizioniMeteo);
  
  // Disegna icona GRANDE (64x64) a sinistra - usando scale 2x
  int iconX = 30;
  for (int j = 0; j < 32; j++) {
    for (int i = 0; i < 32; i++) {
      int byteIndex = j * 4 + (i / 8);
      int bitIndex = 7 - (i % 8);
      bool pixelOn = (pgm_read_byte(&icona[byteIndex]) >> bitIndex) & 1;
      
      if (pixelOn) {
        // Disegna pixel 2x2 per scala 2x
        tft.fillRect(iconX + i * 2, y + j * 2, 2, 2, COLORE_BINARIO);
      }
    }
  }
  
  // TEMPERATURA a destra dell'icona (grande) - alta come icona (64px)
  tft.setTextSize(8);  // textSize(8) = 64px di altezza (come icona)
  tft.setTextColor(COLORE_TESTO);
  tft.setCursor(iconX + 90, y);  // Allineata in alto con icona
  tft.print(temperatura);
  
  y += 66;  // Avanza dopo icona (64px) + piccolo margine
  
  // --- DESCRIZIONE CONDIZIONI (centrata) ---
  String descrizione = getDescrizioneMeteo(condizioniMeteo);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_CYAN);
  int descWidth = descrizione.length() * 6 * 2;
  int descX = (screenWidth - descWidth) / 2;
  tft.setCursor(descX, y - 2);  // Spostata su di 2px
  tft.print(descrizione);
  
  y += 20;
  
  // Linea separatrice
  tft.drawFastHLine(0, y, screenWidth, COLORE_ETICHETTE);
  y += 8;
  
  // --- SEZIONE VENTO ---
  tft.setTextSize(2);  // Ingrandita
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(10, y);
  tft.print("vento");
  
  y += 18;  // Più spazio per label più grande
  
  // Informazioni vento: direzione + velocità + unità (tutto omogeneo)
  String windDir = getWindDirection(direzioneVento);
  String windInfo = windDir + " " + String((int)vento) + " km/h";
  
  tft.setTextSize(2);
  tft.setTextColor(COLORE_BINARIO);
  tft.setCursor(10, y);
  tft.print(windInfo);  // Es: "NE 15 km/h" tutto con stessa dimensione
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
  // === GESTIONE BLUETOOTH ===
  // Controlla stato connessione periodicamente
  if (millis() - lastBTCheck > 1000) {
    checkBluetoothConnection();
    lastBTCheck = millis();
  }
  
  // Leggi comandi da BT se disponibili
  if (SerialBT.available()) {
    int idx = 0;
    unsigned long timeout = millis() + 100;
    
    while (SerialBT.available() && idx < 127 && millis() < timeout) {
      char c = SerialBT.read();
      if (c == '\n' || c == '\r') break;
      btInputBuffer[idx++] = c;
      delay(1);
    }
    btInputBuffer[idx] = '\0';
    
    // Svuota buffer residuo
    while (SerialBT.available()) SerialBT.read();
    
    if (idx > 0) {
      processBluetoothCommand(btInputBuffer);
    }
  }
  
  // === GESTIONE DISPLAY ===
  // Lo scroll viene mostrato SOLO per il tabellone ferroviario, NON per il meteo
  if (displayMeteoAttivo) {
    return;  // Non mostrare scroll se è attivo il display meteo
  }
  
  unsigned long currentTime = millis();
  
  // Aggiorna lo scroll ogni scrollDelay millisecondi
  if (currentTime - lastScrollTime > scrollDelay) {
    lastScrollTime = currentTime;
    
    // Area ANCORATA IN BASSO al display
    int screenWidth = TFT_HEIGHT;   // 320
    int screenHeight = TFT_WIDTH;   // 172
    int labelY = screenHeight - 28;  // Posizione label "informazioni"
    int scrollY = screenHeight - 16; // Posizione testo scorrevole
    int areaHeight = 35;             // Altezza totale area da cancellare
    
    // Cancella TUTTA l'area in basso per eliminare strisce
    tft.fillRect(0, labelY, screenWidth, areaHeight, COLORE_SFONDO);
    
    // Disegna label "informazioni" (ciano) - PIÙ PICCOLA
    tft.setTextSize(1);  // Ridotta a 1
    tft.setTextColor(ST77XX_CYAN);
    tft.setCursor(5, labelY);
    tft.print("informazioni");
    
    // Disegna il testo scorrevole (giallo) - size 2
    tft.setTextSize(2);
    tft.setTextColor(COLORE_TESTO);
    tft.setCursor(scrollX, scrollY);
    tft.print(infoScorrente);
    
    // Aggiorna la posizione
    scrollX -= scrollSpeed;
    
    // Reset quando il testo esce completamente a sinistra
    int textWidth = infoScorrente.length() * 6 * 2;  // Dimensione 2
    if (scrollX < -textWidth) {
      scrollX = screenWidth;  // Ricomincia da destra
    }
  }
}
