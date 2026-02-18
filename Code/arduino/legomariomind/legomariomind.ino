/*
    Board: ESP32 DEV Module
    Library: 
      esp32 -> 2.0.17 (board)
      FastLED -> 3.10.3
      ESPAsyncWebServer -> 3.1.0
      Legoino -> 1.0.0
      AsyncTCP -> 1.1.4      
    UploadSpeed: 115200
*/

#include <WiFi.h>
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
#define CENTER_LED 12
CRGB leds[NUM_LEDS];
uint32_t lastColor = 0;
uint32_t centerLedColor = CRGB::Red;

// ======== MARIO LEGO ========
Lpf2Hub myMario;
HubType typeD;
bool isRemoteInitialized = false;
bool isRemoteInitFirst = false;
DeviceType barcodeSensor = DeviceType::MARIO_HUB_BARCODE_SENSOR;

// ======== WEBSERVER & WEBSOCKET ========
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ======== HTML, CSS, JS EMBEDDED ========
const char index_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Lego Mario Mind</title>
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black">
<meta name="apple-mobile-web-app-title" content="Mario Mind">
<link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Ccircle cx='50' cy='50' r='48' fill='%23e60012'/%3E%3Ctext x='50' y='70' font-size='60' font-weight='bold' fill='white' text-anchor='middle' font-family='Arial'%3EM%3C/text%3E%3C/svg%3E">
<link rel="apple-touch-icon" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Ccircle cx='50' cy='50' r='48' fill='%23e60012'/%3E%3Ctext x='50' y='70' font-size='60' font-weight='bold' fill='white' text-anchor='middle' font-family='Arial'%3EM%3C/text%3E%3C/svg%3E">
<style>
body{
  margin:0;
  font-family:Arial;
  background:#111;
  color:white;
  display:flex;
  flex-direction:column;
  height:100vh;
  overflow:hidden;
}

#topBar{
  position:fixed;
  top:0;
  left:0;
  right:0;
  background:#000;
  padding:8px 0;
  border-bottom:2px solid #444;
  z-index:100;
}

h1{
  margin:0;
  text-align:center;
  font-size:1.2em;
  line-height:1.2;
}

#attemptInfo{
  text-align:center;
  font-weight:bold;
  font-size:0.9em;
  margin:2px 0 0 0;
}

#gameArea{
  flex:1;
  overflow-y:auto;
  overflow-x:hidden;
  padding-top:75px;
  padding-bottom:145px;
}

#message{
	position: fixed;
	top: 50%;
	left: 50%;
	transform: translate(-50%, -50%);
	text-align: center;
	padding: 40px 60px;
	border-radius: 20px;
	font-weight: bold;
	font-size: 200%;
	z-index: 1000;
	box-shadow: 0 10px 40px rgba(0,0,0,0.8);
	display: none;
}

.message-neutral{ background:#ffffff; color:#111; }
.message-win{ background:#00c853; color:white; }
.message-lose{ background:#d50000; color:white; }

.board{ display:flex; flex-direction:column; gap:7px; padding:8px; }
.row{ display:grid; grid-template-columns: repeat(4, 45px) 75px; justify-content:center; align-items:center; column-gap:7px; }
.row.current{ background:rgba(255,255,255,0.05); border-radius:8px; padding:3px 0; }
.slot{ width:36px; height:36px; border-radius:50%; background:#333; border:2px solid #555; }

.feedback{ display:grid; grid-template-columns: repeat(4, 13px); gap:3px; justify-content:start; }
.fbDot{ width:10px; height:10px; border-radius:50%; border:1px solid white; background: rgba(255,255,255,0.1); opacity: 0.25;}
.fbDot.filled{ background: rgba(255,255,255,1); opacity: 1; }
.fbDot.empty{ background: rgba(255,255,255,0.5); opacity: 1; }

#bottomBar{ position:fixed; bottom:0; left:0; right:0; background:#000; padding:8px 0 10px 0; border-top:2px solid #444; z-index:100; }
.colorPanel{ display:flex; justify-content:center; gap:12px; margin-bottom:8px; }
.colorBtn{ width:45px; height:45px; border-radius:50%; border:2px solid white; cursor:pointer; }
.colorBtn.disabled{ opacity:0.3; pointer-events:none; }
.controlPanel{ display:flex; justify-content:center; gap:25px; }
.controlBtn{ width:100px; height:40px; border-radius:10px; display:flex; align-items:center; justify-content:center; font-weight:bold; font-size:13px; cursor:pointer; }
.whiteBtn{ background:white; color:black; }
.blackBtn{ background:black; color:white; border:2px solid white; }
</style>
</head>
<body>

<div id="topBar">
  <h1>Lego Mario Mind</h1>
  <div id="attemptInfo">Tentativo 1 / 10</div>
</div>

<div id="gameArea">
  <div id="message"></div>
  <div class="board" id="board"></div>
</div>

<div id="bottomBar">
  <div class="colorPanel">
    <div class="colorBtn" data-color="rosso" onclick="addColor('rosso')" style="background:red;"></div>
    <div class="colorBtn" data-color="giallo" onclick="addColor('giallo')" style="background:yellow;"></div>
    <div class="colorBtn" data-color="verde" onclick="addColor('verde')" style="background:green;"></div>
    <div class="colorBtn" data-color="viola" onclick="addColor('viola')" style="background:purple;"></div>
    <div class="colorBtn" data-color="blu" onclick="addColor('blu')" style="background:blue;"></div>
  </div>
  <div class="controlPanel">
    <div class="controlBtn whiteBtn" onclick="clearRow()">ANNULLA</div>
    <div class="controlBtn blackBtn" onclick="restartGame()">RIAVVIA</div>
  </div>
</div>

<script>
function goFullScreen() {
  let docEl = document.documentElement;
  if (docEl.requestFullscreen) {
    docEl.requestFullscreen();
  } else if (docEl.webkitRequestFullscreen) {
    docEl.webkitRequestFullscreen();
  } else if (docEl.msRequestFullscreen) {
    docEl.msRequestFullscreen();
  }
}

window.onload = () => {
  document.body.addEventListener('click', goFullScreen);
};

const colors = ["rosso","giallo","verde","viola","blu"];
const colorMap = {
  "rosso": "red",
  "giallo": "yellow",
  "verde": "green",
  "viola": "purple",
  "blu": "blue"
};

let secret=[], currentRow=[], attempts=[], currentAttempt=0, gameOver=false;

let socket = new WebSocket('ws://' + location.host + '/ws');

socket.onmessage = function(event) {
  let color = event.data;
  console.log("WS Color received:", color);

  if (color === "nero") { clearRow(); return; }
  if (color === "bianco") { restartGame(); return; }

  addColor(color);
};


function generateSecret(){
  secret=[];
  let available=[...colors];
  for(let i=0;i<4;i++){
    let index=Math.floor(Math.random()*available.length);
    secret.push(available[index]);
    available.splice(index,1);
  }
  console.log("Secret:",secret);
}

function drawBoard(){
  const board=document.getElementById("board");
  board.innerHTML="";
  for(let i=0;i<10;i++){
    const rowDiv=document.createElement("div"); 
    rowDiv.className="row";
    if(i===currentAttempt && !gameOver) rowDiv.classList.add("current");
    rowDiv.id="row"+i;
    
    let rowData=attempts[i]||[];
    for(let j=0;j<4;j++){
      const slot=document.createElement("div"); slot.className="slot";
      if(i===currentAttempt){ if(currentRow[j]) slot.style.background=colorMap[currentRow[j]]; }
      else{ if(rowData[j]) slot.style.background=colorMap[rowData[j]]; }
      rowDiv.appendChild(slot);
    }

    const feedbackDiv=document.createElement("div"); feedbackDiv.className="feedback";
    if(i<attempts.length){
      const fb=attempts[i].feedback;
      fb.forEach(type=>{
        const dot=document.createElement("div"); dot.className="fbDot";
        if(type==="filled") dot.classList.add("filled");
        if(type==="empty") dot.classList.add("empty");
        feedbackDiv.appendChild(dot);
      });
    } else { for(let k=0;k<4;k++){ const dot=document.createElement("div"); dot.className="fbDot"; feedbackDiv.appendChild(dot); } }

    rowDiv.appendChild(feedbackDiv); board.appendChild(rowDiv);
  }
  document.getElementById("attemptInfo").innerText="Tentativo "+(currentAttempt+1)+" / 10";
  
  if(!gameOver && currentAttempt>0){
    const currentRowEl=document.getElementById("row"+currentAttempt);
    if(currentRowEl) currentRowEl.scrollIntoView({behavior:"smooth",block:"center"});
  }
}

function updateColorButtons(){
  document.querySelectorAll(".colorBtn").forEach(btn=>{
    const c=btn.dataset.color;
    if(currentRow.includes(c)) btn.classList.add("disabled");
    else btn.classList.remove("disabled");
  });
}

function addColor(color){
  if(gameOver || currentRow.length>=4 || currentRow.includes(color)) return;
  if(socket.readyState === WebSocket.OPEN){
    socket.send("led:" + color);
  }
  currentRow.push(color); updateColorButtons(); drawBoard();
  if(currentRow.length===4) setTimeout(checkGuess,200);
}

function clearRow(){ 
  if(gameOver) return; 
  if(socket.readyState === WebSocket.OPEN){
    socket.send("led:nero");
  }
  currentRow=[]; updateColorButtons(); drawBoard(); setMessage(""); 
}

function checkGuess(){
  let feedback=[], tempSecret=[...secret], tempGuess=[...currentRow];
  for(let i=0;i<4;i++){ if(tempGuess[i]===tempSecret[i]){ feedback.push("filled"); tempSecret[i]=null; tempGuess[i]=null; } }
  for(let i=0;i<4;i++){ if(tempGuess[i]){ let idx=tempSecret.indexOf(tempGuess[i]); if(idx!==-1){ feedback.push("empty"); tempSecret[idx]=null; } } }

  attempts[currentAttempt]=[...currentRow]; attempts[currentAttempt].feedback=feedback;

  if(feedback.filter(f=>f==="filled").length===4){ setMessage("Hai vinto!","win"); gameOver=true; }
  else{
    currentAttempt++; currentRow=[];
    if(currentAttempt>=10){ setMessage("Hai perso! Codice: "+secret.join(", "),"lose"); gameOver=true; }
  }
  updateColorButtons(); drawBoard();
}

function setMessage(msg, type="neutral") {
  const el = document.getElementById("message");

  if(!msg) {
    el.innerText = "";
    el.style.display = "none";
    return;
  }

  el.style.display = "block";
  el.innerText = msg;

  el.classList.remove("message-neutral","message-win","message-lose");

  if(type==="win") el.classList.add("message-win");
  else if(type==="lose") el.classList.add("message-lose");
  else el.classList.add("message-neutral");
}


function restartGame(){ 
  if(socket.readyState === WebSocket.OPEN){
    socket.send("led:nero");
  }
  secret=[]; attempts=[]; currentRow=[]; currentAttempt=0; gameOver=false; setMessage(""); generateSecret(); updateColorButtons(); drawBoard(); 
}

generateSecret(); drawBoard();
</script>
</body>
</html>
)=====";

// Invia colore al browser
void notifyColorToBrowser(const String &colorName){
  ws.textAll(colorName);
}

// ======== FUNZIONI LED ========
void fullColor(uint32_t color){
  if(lastColor == color) return;
  FastLED.setBrightness(20);
  fill_solid(leds, NUM_LEDS, color);
  leds[CENTER_LED] = centerLedColor;
  FastLED.show();
  lastColor = color;
}

void setCenterLED(uint32_t color){
  centerLedColor = color;
  leds[CENTER_LED] = color;
  FastLED.show();
}

// Dichiarazione forward
void marioColorToLed(byte color);

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
  setCenterLED(CRGB::Red);

  // === Access Point ESP32 ===
  WiFi.softAP(ssid, password);
  Serial.println("Hotspot attivo!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.softAPIP());

  // Server HTTP - serve HTML embedded
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // WebSocket
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                AwsEventType type, void *arg, uint8_t *data, size_t len){
    if(type == WS_EVT_CONNECT){
      Serial.println("Browser connesso via WS");
    }
    else if(type == WS_EVT_DATA){
      String msg = "";
      for(size_t i=0; i<len; i++){
        msg += (char)data[i];
      }
      Serial.print("WS ricevuto: "); Serial.println(msg);
      
      if(msg.startsWith("led:")){
        String color = msg.substring(4);
        if(color == "rosso") fullColor(CRGB::Red);
        else if(color == "giallo") fullColor(CRGB::Yellow);
        else if(color == "verde") fullColor(CRGB::Green);
        else if(color == "viola") fullColor(CRGB::Purple);
        else if(color == "blu") fullColor(CRGB::Blue);
        else if(color == "nero") fullColor(CRGB::Black);
      }
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
    setCenterLED(CRGB::Yellow);
    typeD = myMario.getHubType();
    if((byte)typeD == 4 || (byte)typeD == 7){
      if(myMario.connectHub()){
        if((byte)typeD == 4) myMario.setLedColor(GREEN);
        setCenterLED(CRGB::Green);
      }
    }
  }

  if(myMario.isConnected() && !isRemoteInitialized){
    setCenterLED(CRGB::Green);
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
    setCenterLED(CRGB::Red);
    myMario.init();
    isRemoteInitialized = false;
    isRemoteInitFirst = true;
  }
}
