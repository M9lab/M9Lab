/*
    Board: ESP32 DEV Module
    Library: 
      esp32 -> 2.0.17 (board)
      FastLED -> 3.10.3
      ESPAsyncWebServer -> 3.1.0
      Legoino -> 1.0.0 (not patched)
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

// ======== BUTTON ATOM LITE ========
#define BUTTON_PIN 39  // Pulsante centrale Atom Lite

// ======== MARIO LEGO (stile Mariotest funzionante) ========
Lpf2Hub myMario;
HubType typeD;
bool isBarcodeSensorInitialized = false;
bool isVolumeSet = false;
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
  padding:8px 10px;
  border-bottom:2px solid #444;
  z-index:100;
  display:flex;
  align-items:center;
  justify-content:space-between;
}

#menuBtn{
  width:35px;
  height:35px;
  display:flex;
  flex-direction:column;
  justify-content:center;
  align-items:center;
  cursor:pointer;
  padding:5px;
  z-index:110;
  flex-shrink:0;
}

#menuBtn span{
  width:22px;
  height:3px;
  background:white;
  margin:3px 0;
  transition:0.3s;
  display:block;
}

#headerContent{
  flex:1;
  text-align:center;
}

h1{
  margin:0;
  font-size:1.2em;
  line-height:1.2;
}

#attemptInfo{
  font-weight:bold;
  font-size:0.9em;
  margin:2px 0 0 0;
}

#menuOverlay{
  position:fixed;
  top:0;
  left:0;
  right:0;
  bottom:0;
  background:rgba(0,0,0,0.7);
  z-index:200;
  display:none;
}

#menuPanel{
  position:fixed;
  top:0;
  left:-280px !important;
  width:calc(280px - 40px);
  height:100%;
  background:#1a1a1a;
  z-index:201;
  transition:left 0.3s ease;
  padding:20px;
  overflow-y:auto;
}

#menuPanel.open{
  left:0 !important;
}

#menuOverlay.open{
  display:block;
}

.menuSection{
  margin-bottom:25px;
}

.menuSection h3{
  color:#fff;
  margin:0 0 10px 0;
  font-size:1em;
  border-bottom:1px solid #444;
  padding-bottom:5px;
}

.menuOption{
  padding:12px 15px;
  background:#2a2a2a;
  margin:8px 0;
  border-radius:8px;
  cursor:pointer;
  color:#fff;
  display:flex;
  justify-content:space-between;
  align-items:center;
  min-height:45px;
}

.menuOption:hover{
  background:#3a3a3a;
}

.menuOption.active{
  background:#533483;
}

.menuOption svg{
  width:30px;
  height:20px;
}

.menuOption .check{
  font-size:1.2em;
  font-weight:bold;
}

#spacer{
  width:30px;
}

#creditsModal{
  position:fixed;
  top:0;
  left:0;
  right:0;
  bottom:0;
  background:rgba(0,0,0,0.85);
  z-index:300;
  display:none;
  justify-content:center;
  align-items:center;
}

#creditsModal.open{
  display:flex;
}

.creditsContent{
  background:#1a1a1a;
  padding:40px;
  border-radius:15px;
  max-width:400px;
  text-align:center;
  position:relative;
  border:2px solid #533483;
}

.creditsContent h2{
  margin:0 0 10px 0;
  color:#fff;
  font-size:1.8em;
}

.creditsContent p{
  margin:10px 0;
  color:#ccc;
  font-size:1.1em;
}

.socialLinks{
  margin-top:20px;
  display:flex;
  justify-content:center;
  gap:20px;
}

.socialLinks a{
  color:#fff;
  text-decoration:none;
  transition:all 0.2s;
  display:inline-block;
}

.socialLinks a svg{
  width:40px;
  height:40px;
  fill:currentColor;
}

.socialLinks a:hover{
  transform:scale(1.15);
  color:#533483;
}

.closeCredits{
  position:absolute;
  top:10px;
  right:15px;
  font-size:2em;
  color:#fff;
  cursor:pointer;
  line-height:1;
}

.closeCredits:hover{
  color:#533483;
}

#gameArea{
  flex:1;
  overflow-y:hidden;
  overflow-x:hidden;
  padding-top:65px;
  padding-bottom:125px;
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

:root{
  --slot-size: 30px;
  --gap-size: 5px;
  --col-width: 38px;
  --feedback-width: 68px;
}

.board{ display:flex; flex-direction:column; gap:var(--gap-size); padding:0; }
.row{ display:grid; grid-template-columns: repeat(4, var(--col-width)) var(--feedback-width); justify-content:center; align-items:center; column-gap:var(--gap-size); }
.row.current{ background:rgba(255,255,255,0.05); border-radius:8px; padding:2px 0; }
.slot{ width:var(--slot-size); height:var(--slot-size); border-radius:50%; background:#000; border:2px solid #555; transition: transform 0.3s ease; }

@keyframes colorAdded {
  0% { transform: scale(1); }
  50% { transform: scale(1.4); box-shadow: 0 0 20px rgba(255,255,255,0.8); }
  100% { transform: scale(1); }
}

.slot.added{ animation: colorAdded 0.5s ease; }

.feedback{ display:grid; grid-template-columns: repeat(4, 13px); gap:3px; justify-content:start; }
.fbDot{ width:10px; height:10px; border-radius:50%; border:1px solid white; background: rgba(255,255,255,0.1); opacity: 0.25;}
.fbDot.filled{ background: rgba(255,255,255,1); opacity: 1; }
.fbDot.empty{ background: rgba(255,255,255,0.5); opacity: 1; }

#bottomBar{ position:fixed; bottom:0; left:0; right:0; background:#000; padding:8px 0 10px 0; border-top:2px solid #444; z-index:100; }
.colorPanel{ display:flex; justify-content:center; gap:12px; margin-bottom:8px; }
.colorBtn{ width:45px; height:45px; border-radius:50%; border:2px solid white; cursor:pointer; transition: transform 0.2s ease; }
.colorBtn:active{ transform: scale(0.85); }
.colorBtn.clicked{ animation: colorClicked 0.4s ease; }
.colorBtn.disabled{ opacity:0.3; pointer-events:none; }

@keyframes colorClicked {
  0% { transform: scale(1); }
  50% { transform: scale(1.3); box-shadow: 0 0 25px rgba(255,255,255,0.9); }
  100% { transform: scale(1); }
}
.controlPanel{ display:flex; justify-content:center; gap:25px; }
.controlBtn{ width:100px; height:40px; border-radius:10px; display:flex; align-items:center; justify-content:center; font-weight:bold; font-size:13px; cursor:pointer; }
.whiteBtn{ background:#009894; color:white; }
.orangeBtn{ background:#D67923; color:white; }
.blackBtn{ background:black; color:white; border:2px solid white; }
</style>
</head>
<body>

<div id="topBar">
  <div id="menuBtn" onclick="toggleMenu()">
    <span></span>
    <span></span>
    <span></span>
  </div>
  <div id="headerContent">
    <h1 data-i18n="title">Lego Mario Mind</h1>
    <div id="attemptInfo" data-i18n="attempt">Tentativo 1 / 10</div>
  </div>
  <div id="spacer"></div>
</div>

<div id="menuOverlay" onclick="closeMenu()"></div>
<div id="menuPanel">
  <div class="menuSection">
    <h3>Language</h3>
    <div class="menuOption" onclick="setLanguage('it')" id="lang-it">
      <div style="display:flex; align-items:center; gap:10px;">
        <svg viewBox="0 0 3 2"><rect width="1" height="2" fill="#009246"/><rect x="1" width="1" height="2" fill="#fff"/><rect x="2" width="1" height="2" fill="#ce2b37"/></svg>
        <span>Italian</span>
      </div>
      <span class="check">✓</span>
    </div>
    <div class="menuOption" onclick="setLanguage('en')" id="lang-en">
      <div style="display:flex; align-items:center; gap:10px;">
        <svg viewBox="0 0 60 30"><clipPath id="t"><path d="M30,15 h30 v15 z v15 h-30 z h-30 v-15 z v-15 h30 z"/></clipPath><path d="M0,0 v30 h60 v-30 z" fill="#012169"/><path d="M0,0 L60,30 M60,0 L0,30" stroke="#fff" stroke-width="6"/><path d="M0,0 L60,30 M60,0 L0,30" clip-path="url(#t)" stroke="#C8102E" stroke-width="4"/><path d="M30,0 v30 M0,15 h60" stroke="#fff" stroke-width="10"/><path d="M30,0 v30 M0,15 h60" stroke="#C8102E" stroke-width="6"/></svg>
        <span>English</span>
      </div>
      <span class="check"></span>
    </div>
  </div>
  <div class="menuSection">
    <h3>Info</h3>
    <div class="menuOption" onclick="openCredits()">
      <span>Credits</span>
      <span>→</span>
    </div>
  </div>
</div>

<div id="creditsModal" onclick="closeCredits()">
  <div class="creditsContent" onclick="event.stopPropagation()">
    <span class="closeCredits" onclick="closeCredits()">×</span>
    <h2>MezzanineLab</h2>
    <p>Where Lego meets Arduino</p>
    <div class="socialLinks">
      <a href="https://www.facebook.com/m9lab/" target="_blank" title="Facebook">
        <svg viewBox="0 0 24 24"><path d="M24 12.073c0-6.627-5.373-12-12-12s-12 5.373-12 12c0 5.99 4.388 10.954 10.125 11.854v-8.385H7.078v-3.47h3.047V9.43c0-3.007 1.792-4.669 4.533-4.669 1.312 0 2.686.235 2.686.235v2.953H15.83c-1.491 0-1.956.925-1.956 1.874v2.25h3.328l-.532 3.47h-2.796v8.385C19.612 23.027 24 18.062 24 12.073z"/></svg>
      </a>
      <a href="https://www.instagram.com/mezzaninelab/" target="_blank" title="Instagram">
        <svg viewBox="0 0 24 24"><path d="M12 2.163c3.204 0 3.584.012 4.85.07 3.252.148 4.771 1.691 4.919 4.919.058 1.265.069 1.645.069 4.849 0 3.205-.012 3.584-.069 4.849-.149 3.225-1.664 4.771-4.919 4.919-1.266.058-1.644.07-4.85.07-3.204 0-3.584-.012-4.849-.07-3.26-.149-4.771-1.699-4.919-4.92-.058-1.265-.07-1.644-.07-4.849 0-3.204.013-3.583.07-4.849.149-3.227 1.664-4.771 4.919-4.919 1.266-.057 1.645-.069 4.849-.069zm0-2.163c-3.259 0-3.667.014-4.947.072-4.358.2-6.78 2.618-6.98 6.98-.059 1.281-.073 1.689-.073 4.948 0 3.259.014 3.668.072 4.948.2 4.358 2.618 6.78 6.98 6.98 1.281.058 1.689.072 4.948.072 3.259 0 3.668-.014 4.948-.072 4.354-.2 6.782-2.618 6.979-6.98.059-1.28.073-1.689.073-4.948 0-3.259-.014-3.667-.072-4.947-.196-4.354-2.617-6.78-6.979-6.98-1.281-.059-1.69-.073-4.949-.073zm0 5.838c-3.403 0-6.162 2.759-6.162 6.162s2.759 6.163 6.162 6.163 6.162-2.759 6.162-6.163c0-3.403-2.759-6.162-6.162-6.162zm0 10.162c-2.209 0-4-1.79-4-4 0-2.209 1.791-4 4-4s4 1.791 4 4c0 2.21-1.791 4-4 4zm6.406-11.845c-.796 0-1.441.645-1.441 1.44s.645 1.44 1.441 1.44c.795 0 1.439-.645 1.439-1.44s-.644-1.44-1.439-1.44z"/></svg>
      </a>
      <a href="https://youtube.com/@mezzaninelab" target="_blank" title="YouTube">
        <svg viewBox="0 0 24 24"><path d="M23.498 6.186a3.016 3.016 0 0 0-2.122-2.136C19.505 3.545 12 3.545 12 3.545s-7.505 0-9.377.505A3.017 3.017 0 0 0 .502 6.186C0 8.07 0 12 0 12s0 3.93.502 5.814a3.016 3.016 0 0 0 2.122 2.136c1.871.505 9.376.505 9.376.505s7.505 0 9.377-.505a3.015 3.015 0 0 0 2.122-2.136C24 15.93 24 12 24 12s0-3.93-.502-5.814zM9.545 15.568V8.432L15.818 12l-6.273 3.568z"/></svg>
      </a>
    </div>
    <p style="margin-top:25px; font-size:0.95em; color:#999;">Follow us for more creative projects!</p>
  </div>
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
    <div class="colorBtn" data-color="blu" onclick="addColor('blu')" style="background:#0066cc;"></div>
  </div>
  <div class="controlPanel">
    <div class="controlBtn whiteBtn" onclick="clearRow()" data-i18n="button.cancel">ANNULLA</div>
    <div class="controlBtn orangeBtn" onclick="restartGame()" data-i18n="button.restart">RIAVVIA</div>
  </div>
</div>

<script>
// === AUDIO SYSTEM ===
const audioCtx = new (window.AudioContext || window.webkitAudioContext)();

function playTone(frequency, duration, volume = 0.3) {
  const oscillator = audioCtx.createOscillator();
  const gainNode = audioCtx.createGain();
  
  oscillator.connect(gainNode);
  gainNode.connect(audioCtx.destination);
  
  oscillator.frequency.value = frequency;
  oscillator.type = 'sine';
  
  gainNode.gain.setValueAtTime(volume, audioCtx.currentTime);
  gainNode.gain.exponentialRampToValueAtTime(0.01, audioCtx.currentTime + duration);
  
  oscillator.start(audioCtx.currentTime);
  oscillator.stop(audioCtx.currentTime + duration);
}

function playColorSelect() {
  // Beep corto e acuto per selezione colore
  playTone(800, 0.08, 0.2);
}

function playRowResult() {
  // Due beep per risultato riga
  playTone(400, 0.1, 0.25);
  setTimeout(() => playTone(500, 0.1, 0.25), 120);
}

function playWinSound() {
  // Sequenza vittoriosa: do-mi-sol-do
  const notes = [523, 659, 784, 1047];
  notes.forEach((freq, i) => {
    setTimeout(() => playTone(freq, 0.3, 0.3), i * 150);
  });
}

function playLoseSound() {
  // Sequenza discendente triste
  const notes = [400, 350, 300, 250];
  notes.forEach((freq, i) => {
    setTimeout(() => playTone(freq, 0.4, 0.25), i * 200);
  });
}

// === TRADUZIONI ===
const translations = {
  it: {
    title: "Lego Mario Mind",
    attempt: "Tentativo {n} / 10",
    "menu.language": "Language",
    "button.cancel": "ANNULLA",
    "button.restart": "RIAVVIA",
    "message.win": "Hai vinto!",
    "message.lose": "Hai perso! Codice: {code}",
    "colors": {
      "rosso": "rosso",
      "giallo": "giallo", 
      "verde": "verde",
      "viola": "viola",
      "blu": "blu"
    }
  },
  en: {
    title: "Lego Mario Mind",
    attempt: "Attempt {n} / 10",
    "menu.language": "Language",
    "button.cancel": "CANCEL",
    "button.restart": "RESTART",
    "message.win": "You won!",
    "message.lose": "You lost! Code: {code}",
    "colors": {
      "rosso": "red",
      "giallo": "yellow",
      "verde": "green",
      "viola": "purple",
      "blu": "blue"
    }
  }
};

let currentLang = localStorage.getItem('language') || 'it';

function t(key, params = {}) {
  let text = translations[currentLang][key] || key;
  Object.keys(params).forEach(param => {
    text = text.replace(`{${param}}`, params[param]);
  });
  return text;
}

function translateColor(colorName) {
  return translations[currentLang].colors[colorName] || colorName;
}

function setLanguage(lang) {
  currentLang = lang;
  localStorage.setItem('language', lang);
  
  document.querySelectorAll('.menuOption').forEach(opt => {
    opt.classList.remove('active');
    const checkSpan = opt.querySelector('.check');
    if(checkSpan) checkSpan.textContent = '';
  });
  
  document.getElementById('lang-' + lang).classList.add('active');
  document.querySelector('#lang-' + lang + ' .check').textContent = '✓';
  
  updateUI();
  closeMenu();
}

function updateUI() {
  document.querySelectorAll('[data-i18n]').forEach(el => {
    const key = el.getAttribute('data-i18n');
    if(key === 'attempt') {
      // Usa currentAttempt se definito, altrimenti 0
      const attemptNum = typeof currentAttempt !== 'undefined' ? currentAttempt + 1 : 1;
      el.textContent = t(key, {n: attemptNum});
    } else {
      el.textContent = t(key);
    }
  });
}

function toggleMenu() {
  const overlay = document.getElementById('menuOverlay');
  const panel = document.getElementById('menuPanel');
  overlay.classList.toggle('open');
  panel.classList.toggle('open');
}

function closeMenu() {
  const overlay = document.getElementById('menuOverlay');
  const panel = document.getElementById('menuPanel');
  overlay.classList.remove('open');
  panel.classList.remove('open');
}

function openCredits() {
  document.getElementById('creditsModal').classList.add('open');
  closeMenu();
}

function closeCredits() {
  document.getElementById('creditsModal').classList.remove('open');
}

function goFullScreen() {
  let docEl = document.documentElement;
  if (docEl.requestFullscreen) {
    docEl.requestFullscreen();
  } else if (docEl.webkitRequestFullscreen) {
    docEl.webkitRequestFullscreen();
  } else if (docEl.msRequestFullscreen) {
    docEl.msRequestFullscreen();
  }
  
  // Ricalcola dimensioni dopo un breve delay (fullscreen impiega tempo)
  setTimeout(adjustBoardSize, 300);
}

function adjustBoardSize() {
  const topBarHeight = 65; // padding-top di gameArea
  const bottomBarHeight = 125; // padding-bottom di gameArea
  const tolerance = 20; // Tolleranza aggiuntiva
  
  // Controlla se siamo in fullscreen
  const isFullscreen = document.fullscreenElement || document.webkitFullscreenElement;
  const fullscreenExtra = isFullscreen ? 20 : 0; // Extra 20px se in fullscreen per "nessuna connessione"
  
  const safetyMargin = tolerance + fullscreenExtra;
  const availableHeight = window.innerHeight - topBarHeight - bottomBarHeight - safetyMargin;
  
  // Calcola dimensione ottimale pallini (10 righe + gap)
  // padding board ora è 0, quindi non serve sottrarlo
  const gap = 5;
  const maxSlotSize = 36; // dimensione massima per non esagerare su schermi grandi
  
  let slotSize = (availableHeight - gap * 9) / 10;
  slotSize = Math.min(slotSize, maxSlotSize); // non superare il max
  slotSize = Math.max(slotSize, 24); // dimensione minima
  
  // Applica dimensioni calcolate
  const root = document.documentElement;
  root.style.setProperty('--slot-size', slotSize + 'px');
  root.style.setProperty('--gap-size', gap + 'px');
  root.style.setProperty('--col-width', (slotSize + 4) + 'px'); // +4 per bordo
  root.style.setProperty('--feedback-width', (slotSize * 2) + 'px');
}

window.onload = () => {
  document.body.addEventListener('click', goFullScreen);
  
  // Adatta dimensioni board al dispositivo
  adjustBoardSize();
  window.addEventListener('resize', adjustBoardSize);
  
  // Ricalcola quando entra/esce da fullscreen
  document.addEventListener('fullscreenchange', adjustBoardSize);
  document.addEventListener('webkitfullscreenchange', adjustBoardSize);
  
  // Inizializza lingua senza aprire menu
  document.querySelectorAll('.menuOption').forEach(opt => {
    opt.classList.remove('active');
    const checkSpan = opt.querySelector('.check');
    if(checkSpan) checkSpan.textContent = '';
  });
  document.getElementById('lang-' + currentLang).classList.add('active');
  document.querySelector('#lang-' + currentLang + ' .check').textContent = '✓';
  
  // Assicura che il menu sia chiuso all'avvio
  const overlay = document.getElementById('menuOverlay');
  const panel = document.getElementById('menuPanel');
  overlay.classList.remove('open');
  panel.classList.remove('open');
  
  updateUI();
};

const colors = ["rosso","giallo","verde","viola","blu"];
const colorMap = {
  "rosso": "red",
  "giallo": "yellow",
  "verde": "green",
  "viola": "purple",
  "blu": "#0066cc"
};

let secret=[], currentRow=[], attempts=[], currentAttempt=0, gameOver=false;

let socket = new WebSocket('ws://' + location.host + '/ws');

socket.onmessage = function(event) {
  let color = event.data;
  console.log("WS Color received:", color);

  if (color === "nero" || color === "bianco") { clearRow(); return; }
  if (color === "arancio") { restartGame(); return; }

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
      if(i===currentAttempt){ 
        if(currentRow[j]){
          slot.style.background=colorMap[currentRow[j]];
          if(j === currentRow.length-1){
            slot.classList.add("added");
          }
        }
      }
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
  document.getElementById("attemptInfo").innerText=t('attempt', {n: currentAttempt+1});
  
  // Auto-scroll rimosso (non più necessario con tutte le righe visibili)
  // if(!gameOver && currentAttempt>0){
  //   const currentRowEl=document.getElementById("row"+currentAttempt);
  //   if(currentRowEl) currentRowEl.scrollIntoView({behavior:"smooth",block:"center"});
  // }
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
  
  // Suono selezione colore
  playColorSelect();
  
  // Anima il bottone cliccato
  const btn = document.querySelector(`.colorBtn[data-color="${color}"]`);
  if(btn){
    btn.classList.add("clicked");
    setTimeout(() => btn.classList.remove("clicked"), 400);
  }
  
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

  // Suono risultato riga
  playRowResult();

  if(feedback.filter(f=>f==="filled").length===4){ 
    // VITTORIA!
    setMessage(t("message.win"),"win"); 
    gameOver=true;
    setTimeout(playWinSound, 300); // Ritardo per sentire prima il feedback
  }
  else{
    currentAttempt++; currentRow=[];
    if(currentAttempt>=10){ 
      // SCONFITTA!
      const translatedSecret = secret.map(c => translateColor(c)).join(", ");
      setMessage(t("message.lose", {code: translatedSecret}),"lose"); 
      gameOver=true;
      setTimeout(playLoseSound, 300); // Ritardo per sentire prima il feedback
    }
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
    socket.send("led:arancio");
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
  
  Serial.print("Callback chiamato! Port: "); 
  Serial.print(portNumber); 
  Serial.print(" DeviceType: "); 
  Serial.println((byte)deviceType);

  if(deviceType == DeviceType::MARIO_HUB_BARCODE_SENSOR){
    MarioColor color = myMario->parseMarioColor(pData);
    Serial.print(">>> COLORE RILEVATO: "); 
    Serial.print((byte)color);
    Serial.print(" (0x");
    Serial.print((byte)color, HEX);
    Serial.println(")");
    marioColorToLed((byte)color);
  } else {
    Serial.println("DeviceType NON corrisponde a BARCODE_SENSOR");
  }
}

// ======== MAP COLORI ========
void marioColorToLed(byte color){
  String colorName = "";

  // Valori reali restituiti dal sensore Mario
  switch(color){
    case 21:  fullColor(CRGB::Red);    colorName="rosso"; Serial.println("-> ROSSO"); break;
    case 37:  fullColor(CRGB::Green);  colorName="verde"; Serial.println("-> VERDE"); break;
    case 23:  fullColor(CRGB::Blue);   colorName="blu"; Serial.println("-> BLU"); break;
    case 24:  fullColor(CRGB::Yellow); colorName="giallo"; Serial.println("-> GIALLO"); break;
    case 12:  fullColor(CRGB::Purple); colorName="viola"; Serial.println("-> VIOLA"); break;
    case 66:  fullColor(0x009894);     colorName="nero"; Serial.println("-> TURCHESE (ANNULLA)"); break;
    case 0:   return;  // ignorato (nero)
    case 19:  return;  // ignorato (bianco vecchio)
    case 26:  return;  // ignorato (ritorna quando alzi Mario)
    case 106: fullColor(CRGB::Orange); colorName="bianco"; Serial.println("-> ARANCIO (RIAVVIA)"); break;
    default:  
      Serial.print("-> COLORE SCONOSCIUTO: "); 
      Serial.println(color);
      return;
  }

  if(colorName != ""){
    notifyColorToBrowser(colorName);
    Serial.print("Inviato al browser: "); Serial.println(colorName);
  }
}

// ======== SETUP ========
void setup() {
  Serial.begin(115200);

  // Button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
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
        else if(color == "arancio") fullColor(CRGB::Orange);
        else if(color == "nero") fullColor(CRGB::Black);
      }
    }
  });
  server.addHandler(&ws);
  server.begin();

  // Avvia scansione BLE per Mario Hub (come in Mariotest.ino)
  myMario.init();
  Serial.println("Setup completato. Scansione BLE avviata per Mario Hub.");
  delay(200);
}

// ======== LOOP ========
void loop(){
  ws.cleanupClients();

  // Controllo bottone centrale Atom Lite per riavviare scansione Mario
  static bool lastButtonState = HIGH;
  bool buttonState = digitalRead(BUTTON_PIN);
  
  if(buttonState == LOW && lastButtonState == HIGH){
    // Bottone premuto
    delay(50); // Debounce
    Serial.println("\n=== BOTTONE PREMUTO ===");
    Serial.println("Riavvio scansione BLE per Mario Hub...");
    
    // Reset flag e riavvia init
    isBarcodeSensorInitialized = false;
    isVolumeSet = false;
    setCenterLED(CRGB::Blue); // LED blu per indicare scansione
    myMario.init();
    
    Serial.println("Scansione BLE riavviata. Premi il bottone BT su Mario Hub.");
  }
  lastButtonState = buttonState;

  // Connessione: come in Mariotest, connectHub quando isConnecting()
  if(myMario.isConnecting()){
    myMario.connectHub();
    if(myMario.isConnected()){
      typeD = myMario.getHubType();
      Serial.println("Connected to HUB");
      setCenterLED(CRGB::Green);
    } else {
      setCenterLED(CRGB::Yellow);
      Serial.println("Failed to connect to HUB");
    }
  }

  // Inizializza sensore barcode solo per Mario Hub (type 7)
  if(myMario.isConnected() && !isBarcodeSensorInitialized){
    delay(200);
    if((byte)typeD == 7){
      byte portForDevice = myMario.getPortForDeviceType((byte)barcodeSensor);
      Serial.print("check ports... barcode sensor: ");
      Serial.println(portForDevice, DEC);
      if(portForDevice != 255){
        myMario.activatePortDevice(portForDevice, DeviceCallback);
        delay(200);
        isBarcodeSensorInitialized = true;
        Serial.println("Sensore barcode ATTIVATO! Poggia Mario su un mattoncino colorato.");
      }
    } else {
      // Non è Mario Hub, considera comunque inizializzato per non bloccare il loop
      isBarcodeSensorInitialized = true;
    }
  }

  // Volume Mario DISABILITATO (0% per sentire solo audio browser)
  if(myMario.isConnected() && (byte)typeD == 7 && !isVolumeSet){
    Serial.println("set volume to 0% (muto)");
    myMario.setMarioVolume(0x00);
    isVolumeSet = true;
  }

  // Se si disconnette, resetta flag per permettere nuova connessione
  if(!myMario.isConnected()){
    setCenterLED(CRGB::Red);
    isBarcodeSensorInitialized = false;
    isVolumeSet = false;
  }
}
