#include <WiFi.h>
#include <SPIFFS.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Lpf2Hub.h"

// ======== HOTSPOT ESP32 ========
const char* ssid = "legomariomind";   // Nome rete Wi-Fi dell'ESP32
const char* password = "legomariomind";      // Password Wi-Fi (min 8 caratteri)

// ======== LED WS2812 ========
#define NUM_LEDS 25
#define DATA_PIN 27
CRGB leds[NUM_LEDS];
uint32_t lastColor = 0;

// ======== MARIO LEGO ========
Lpf2Hub myMario;
HubType typeD;
bool isRemoteInitialized = false;
bool isRemoteInitFirst = false;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;

// ======== WEBSERVER & WEBSOCKET ========
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Invia colore al browser
void notifyColorToBrowser(const String &colorName){
  ws.textAll(colorName);
}

// ======== FUNZIONI LED ========
void fullColor(uint32_t color){
  if(lastColor == color) return;
  FastLED.setBrightness(20);
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
  lastColor = color;
}

// ======== MARIO HUB CALLBACK ========
void DeviceCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData){
  Lpf2Hub *myMario = (Lpf2Hub*)hub;

  if(deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR){
    byte color = (byte) myMario->parseMarioColor(pData); 
    Serial.print("Mario color detected: "); Serial.println(color);
    marioColorToLed(color);
  }
}

// ======== MAP COLORI ========
void marioColorToLed(byte color){
  String colorName = "";

  switch(color){
    case 21:  fullColor(CRGB::Red);    colorName="rosso"; break;
    case 23:  fullColor(CRGB::Blue);   colorName="blu"; break;
    case 24:  fullColor(CRGB::Yellow); colorName="giallo"; break;
    case 37:  fullColor(CRGB::Green);  colorName="verde"; break;
    case 45:  fullColor(CRGB::Purple); colorName="viola"; break; // debug con Serial
    case 0:   fullColor(CRGB::Black);  colorName="nero"; break;   // ANNULLA
    case 255: fullColor(CRGB::White);  colorName="bianco"; break; // RIAVVIA
  }

  notifyColorToBrowser(colorName);
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);

  // LED
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  fullColor(CRGB::White);

  // SPIFFS
  if(!SPIFFS.begin(true)){ 
    Serial.println("SPIFFS mount failed"); 
    return; 
  }

  // === Access Point ESP32 ===
  WiFi.softAP(ssid, password);
  Serial.println("Hotspot attivo!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.softAPIP()); // Normalmente 192.168.4.1

  // Server HTTP
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/mastermind.js", SPIFFS, "/mastermind.js");
  server.serveStatic("/backg.webp", SPIFFS, "/backg.webp");

  // WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                AwsEventType type, void *arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
      Serial.println("Browser connesso via WS");
    }
  });
  server.addHandler(&ws);
  server.begin();

  // Inizializza Mario Hub
  delay(200);
  myMario.init();
}

// ======== LOOP ========
void loop(){
  ws.cleanupClients();

  // gestione Mario Hub
  if(myMario.isConnecting()){
    typeD = myMario.getHubType();
    if((byte)typeD == 4 || (byte)typeD == 7){
      if(myMario.connectHub()){
        if((byte)typeD == 4) myMario.setLedColor(GREEN);
      }
    }
  }

  if(myMario.isConnected() && !isRemoteInitialized){
    delay(200);
    if((byte)typeD == 7){
      byte portForDevice = myMario.getPortForDeviceType((byte)barcodeSensor);
      if(portForDevice != 255){
        myMario.activatePortDevice(portForDevice, DeviceCallback);
        delay(200);
        isRemoteInitialized = true;
        isRemoteInitFirst = false;
      }
    }
    if((byte)typeD == 4){
      isRemoteInitialized = true;
      myMario.setLedColor(GREEN);
      isRemoteInitFirst = false;
    }
  }

  if(!myMario.isConnected() && !isRemoteInitFirst){
    myMario.init();
    isRemoteInitialized = false;
    isRemoteInitFirst = true;
  }
}
