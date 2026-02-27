#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Audio.h"
#include "I2SMEMSSource.h"
#include "WakeWord.h"
#include <M5Core2.h>  // Libreria display AtomS3R / M5Stack

// ---------- CONFIG ----------
const char* ssid = "FSociety";
const char* password = "qwerty123456";
const char* replicateToken = "r8_3Wy4PCyMhULDu6EgD0SXRu0Oc7WMmtg38w3en";

// Modelli Replicate
const char* MODELLO_TESTO = "86zptcwbhxrm80cphb19cy8qh4"; // GPT-4.1-nano
const char* MODELLO_TTS   = "by67sg9dxdrm80cpjat9x3apxw"; // minimax/speech-02-turbo
const char* MODELLO_STT   = "vplzxotbmrsqbd7hsbehoaugaa";  // STT Replicate

// ---------- VAR GLOBALI -------------
Audio audio;
I2SMEMSSource mic;
WakeWord wakeword("hey atom");
bool wakeDetected = false;

// Buffer audio per STT
#define AUDIO_BUFFER_SIZE 32000
int16_t audioBuffer[AUDIO_BUFFER_SIZE];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Connettendo WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connesso");
  M5.Lcd.println("WiFi connesso");

  // Speaker
  audio.setPinout(7, 8, 6);
  audio.setVolume(10);

  // Microfono I2S
  mic.begin();

  pinMode(LED_BUILTIN, OUTPUT);
  M5.Lcd.println("Pronuncia 'Ehi Atom'");
  Serial.println("Pronuncia 'Ehi Atom'");
}

void loop() {
  int16_t sample = mic.read();

  // Wake-word detection
  if (!wakeDetected && wakeword.detect(sample)) {
    wakeDetected = true;
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Wake-word rilevata!");
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println("Wake-word rilevata!");
    speak("Dimmi pure.");
    bufferIndex = 0; // Inizia registrazione
  }

  // Registrazione audio se wake-word rilevata
  if (wakeDetected) {
    if (bufferIndex < AUDIO_BUFFER_SIZE) {
      audioBuffer[bufferIndex++] = sample;
    } else {
      // Buffer pieno → invia a STT
      String userText = sendAudioToSTT(audioBuffer, bufferIndex);
      Serial.println("Tu: " + userText);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setCursor(0,0);
      M5.Lcd.println("Tu: " + userText);

      // Richiesta GPT
      String reply = askReplicate(userText);
      Serial.println("Atom: " + reply);
      M5.Lcd.println("Atom: " + reply);

      // TTS
      speak(reply);

      wakeDetected = false;
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}

// ------------------ FUNZIONI ------------------

// Invio buffer audio a modello STT Replicate
String sendAudioToSTT(int16_t* buffer, int len) {
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("api.replicate.com", 443)) return "Errore connessione STT";

  String audioBase64 = base64EncodePCM16(buffer, len);

  String body = "{\"version\":\"" + String(MODELLO_STT) + "\",\"input\":{\"audio\":\"" + audioBase64 + "\"}}";

  client.println("POST /v1/predictions HTTP/1.1");
  client.println("Host: api.replicate.com");
  client.println("Authorization: Token " + String(replicateToken));
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); client.println(body.length());
  client.println();
  client.println(body);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String response = client.readString();
  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, response) != DeserializationError::Ok) return "Errore trascrizione";

  if (doc.containsKey("output") && doc["output"][0].is<const char*>()) {
    return String(doc["output"][0].as<const char*>());
  }
  return "Nessuna trascrizione";
}

// ---------------- FUNZIONI GPT / TTS ----------------

String askReplicate(String prompt) {
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("api.replicate.com", 443)) return "Errore connessione GPT";

  String body = "{\"version\":\"" + String(MODELLO_TESTO) + "\",\"input\":{\"text\":\"" + prompt + "\"}}";

  client.println("POST /v1/predictions HTTP/1.1");
  client.println("Host: api.replicate.com");
  client.println("Authorization: Token " + String(replicateToken));
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); client.println(body.length());
  client.println();
  client.println(body);

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  String response = client.readString();
  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, response) != DeserializationError::Ok) return "Non ho capito.";

  if (doc.containsKey("output") && doc["output"][0].is<const char*>()) {
    return String(doc["output"][0].as<const char*>());
  }
  return "Nessuna risposta utile";
}

void speak(String text) {
  String url = requestTTS(text);
  if (url.length() == 0) { Serial.println("Errore TTS"); return; }
  playAudioFromURL(url);
}

String requestTTS(String txt) {
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect("api.replicate.com", 443)) return "";

  String body = "{\"version\":\"" + String(MODELLO_TTS) + "\",\"input\":{\"text\":\"" + txt + "\"}}";

  client.println("POST /v1/predictions HTTP/1.1");
  client.println("Host: api.replicate.com");
  client.println("Authorization: Token " + String(replicateToken));
  client.println("Content-Type: application/json");
  client.print("Content-Length: "); client.println(body.length());
  client.println();
  client.println(body);

  while (client.connected()) { String line = client.readStringUntil('\n'); if (line == "\r") break; }

  String response = client.readString();
  DynamicJsonDocument doc(8192);
  if (deserializeJson(doc, response) != DeserializationError::Ok) return "";

  const char* audioUrl = doc["output"][0];
  if (audioUrl) return String(audioUrl);
  return "";
}

void playAudioFromURL(String url) {
  audio.connecttohost(url.c_str());
}

// ---------------- FUNZIONE BASE64 PCM16 ----------------
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
String base64EncodePCM16(int16_t* buffer, int len) {
  String encoded = "";
  uint8_t input[3];
  int i = 0;
  int totalBytes = len * 2;
  uint8_t* byteBuffer = (uint8_t*)buffer;

  while (i < totalBytes) {
    int bytesLeft = totalBytes - i;
    input[0] = byteBuffer[i++];
    input[1] = (bytesLeft > 1) ? byteBuffer[i++] : 0;
    input[2] = 0;

    encoded += b64_table[(input[0] & 0xFC) >> 2];
    encoded += b64_table[((input[0] & 0x03) << 4) | ((input[1] & 0xF0) >> 4)];
    encoded += (bytesLeft > 1) ? b64_table[((input[1] & 0x0F) << 2) | ((input[2] & 0xC0) >> 6)] : '=';
    encoded += '=';
  }

  return encoded;
}