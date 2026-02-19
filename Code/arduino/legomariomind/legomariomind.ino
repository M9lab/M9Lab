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

// ======== MARIO LEGO ========
Lpf2Hub myMario;
HubType typeD;
bool isRemoteInitialized = false;
bool isRemoteInitFirst = false;
bool needsInit = true; // Flag per gestire init() con libreria patchata
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
.slot{ width:36px; height:36px; border-radius:50%; background:#333; border:2px solid #555; transition: transform 0.3s ease; }

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
.whiteBtn{ background:white; color:black; }
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
    <div class="colorBtn" data-color="blu" onclick="addColor('blu')" style="background:blue;"></div>
  </div>
  <div class="controlPanel">
    <div class="controlBtn whiteBtn" onclick="clearRow()" data-i18n="button.cancel">ANNULLA</div>
    <div class="controlBtn blackBtn" onclick="restartGame()" data-i18n="button.restart">RIAVVIA</div>
  </div>
</div>

<script>
// === TRADUZIONI ===
const translations = {
  it: {
    title: "Lego Mario Mind",
    attempt: "Tentativo {n} / 10",
    "menu.language": "Language",
    "button.cancel": "ANNULLA",
    "button.restart": "RIAVVIA",
    "message.win": "Hai vinto!",
    "message.lose": "Hai perso! Codice: {code}"
  },
  en: {
    title: "Lego Mario Mind",
    attempt: "Attempt {n} / 10",
    "menu.language": "Language",
    "button.cancel": "CANCEL",
    "button.restart": "RESTART",
    "message.win": "You won!",
    "message.lose": "You lost! Code: {code}"
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

function setLanguage(lang) {
  currentLang = lang;
  localStorage.setItem('language', lang);
  
  document.querySelectorAll('.menuOption').forEach(opt => {
    opt.classList.remove('active');
    opt.querySelector('.check').textContent = '';
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
      el.textContent = t(key, {n: currentAttempt + 1});
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
}

window.onload = () => {
  document.body.addEventListener('click', goFullScreen);
  
  // Inizializza lingua senza aprire menu
  document.querySelectorAll('.menuOption').forEach(opt => {
    opt.classList.remove('active');
    opt.querySelector('.check').textContent = '';
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

  if(feedback.filter(f=>f==="filled").length===4){ setMessage(t("message.win"),"win"); gameOver=true; }
  else{
    currentAttempt++; currentRow=[];
    if(currentAttempt>=10){ setMessage(t("message.lose", {code: secret.join(", ")}),"lose"); gameOver=true; }
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

  switch(color){
    case 21:  fullColor(CRGB::Red);    colorName="rosso"; Serial.println("-> ROSSO"); break;
    case 23:  fullColor(CRGB::Blue);   colorName="blu"; Serial.println("-> BLU"); break;
    case 24:  fullColor(CRGB::Yellow); colorName="giallo"; Serial.println("-> GIALLO"); break;
    case 37:  fullColor(CRGB::Green);  colorName="verde"; Serial.println("-> VERDE"); break;
    case 45:  fullColor(CRGB::Purple); colorName="viola"; Serial.println("-> VIOLA"); break;
    case 0:   fullColor(CRGB::Black);  colorName="nero"; Serial.println("-> NERO (ANNULLA)"); break;
    case 255: fullColor(CRGB::White);  colorName="bianco"; Serial.println("-> BIANCO (RIAVVIA)"); break;
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

  // NON inizializzo Mario Hub qui (come nello script funzionante)
  // L'init viene chiamato solo nel loop quando non è connesso
  Serial.println("Setup completato. Mario init verra' chiamato nel loop.");
  delay(200);
}

// ======== LOOP ========
void loop(){
  ws.cleanupClients();

  // Pattern adattato per libreria patchata con _isInitializing
  if(myMario.isConnecting()){
    typeD = myMario.getHubType();
    setCenterLED(CRGB::Yellow);
    isRemoteInitFirst = false; // Reset quando trova un device
    
    // Mario Hub (type 7)
    if((byte)typeD == 7){
      if(!myMario.connectHub()){
        Serial.println("Impossibile connettere Mario Hub");
      } else {
        Serial.println("Mario Hub connesso!");
        setCenterLED(CRGB::Green);
      }
    }
    
    // Remote (type 4)
    if((byte)typeD == 4){
      if(!myMario.connectHub()){
        Serial.println("Impossibile connettere Remote");
      } else {
        myMario.setLedColor(GREEN);
        Serial.println("Remote connesso!");
        setCenterLED(CRGB::Green);
      }
    }
  }

  if(myMario.isConnected() && !isRemoteInitialized){
    delay(200);
    
    if((byte)typeD == 7){
      byte portForDevice = myMario.getPortForDeviceType((byte)barcodeSensor);
      Serial.print("Cerco porta barcode sensor: "); Serial.println(portForDevice);
      if(portForDevice != 255){
        myMario.activatePortDevice(portForDevice, DeviceCallback);
        delay(200);
        isRemoteInitialized = true;
        isRemoteInitFirst = false;
        Serial.println("Sensore barcode ATTIVATO! Poggia Mario su un mattoncino colorato.");
      }
    }
    
    if((byte)typeD == 4){
      isRemoteInitialized = true;
      myMario.setLedColor(GREEN);
      isRemoteInitFirst = false;
      Serial.println("Remote inizializzato");
    }
  }

  // Gestione init() con libreria patchata (_isInitializing)
  if(!myMario.isConnected()){
    if(!isRemoteInitFirst){
      setCenterLED(CRGB::Red);
      needsInit = true; // Richiedi init al prossimo check
      isRemoteInitFirst = true;
      isRemoteInitialized = false;
      Serial.println("Mario disconnesso, richiedo init...");
    }
    
    // Chiama init() SOLO se needsInit=true (evita chiamate ripetute durante scan)
    if(needsInit){
      Serial.println("Chiamo myMario.init() per avviare scansione BLE...");
      myMario.init();
      needsInit = false; // Non richiamare finché non si disconnette di nuovo
      Serial.println("Scansione BLE avviata, attendo device...");
    }
  }
}
