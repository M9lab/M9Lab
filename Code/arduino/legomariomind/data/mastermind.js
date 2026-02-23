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
      setMessage(t("message.lose", {code: secret.join(", ")}),"lose"); 
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
