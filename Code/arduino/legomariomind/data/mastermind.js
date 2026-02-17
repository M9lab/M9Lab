// Chiedi il full screen all'utente
function goFullScreen() {
  let docEl = document.documentElement;
  if (docEl.requestFullscreen) {
    docEl.requestFullscreen();
  } else if (docEl.webkitRequestFullscreen) { /* Safari */
    docEl.webkitRequestFullscreen();
  } else if (docEl.msRequestFullscreen) { /* IE/Edge */
    docEl.msRequestFullscreen();
  }
}

// Esegui al caricamento della pagina
window.onload = () => {
  // In alcuni browser serve un click per attivare full screen
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

// WebSocket dal server ESP32
let socket = new WebSocket(`ws://${location.host}/ws`);

socket.onmessage = function(event) {
  let color = event.data;
  console.log("WS Color received:", color);

  // se è una funzione speciale
  if (color === "nero") { clearRow(); return; }
  if (color === "bianco") { restartGame(); return; }

  // altrimenti aggiunge il colore alla riga di gioco
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
    const rowDiv=document.createElement("div"); rowDiv.className="row";
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
  currentRow.push(color); updateColorButtons(); drawBoard();
  if(currentRow.length===4) setTimeout(checkGuess,200);
}

function clearRow(){ if(gameOver) return; currentRow=[]; updateColorButtons(); drawBoard(); setMessage(""); }

function checkGuess(){
  let feedback=[], tempSecret=[...secret], tempGuess=[...currentRow];
  for(let i=0;i<4;i++){ if(tempGuess[i]===tempSecret[i]){ feedback.push("filled"); tempSecret[i]=null; tempGuess[i]=null; } }
  for(let i=0;i<4;i++){ if(tempGuess[i]){ let idx=tempSecret.indexOf(tempGuess[i]); if(idx!==-1){ feedback.push("empty"); tempSecret[idx]=null; } } }

  attempts[currentAttempt]=[...currentRow]; attempts[currentAttempt].feedback=feedback;

  if(feedback.filter(f=>f==="filled").length===4){ setMessage("🎉 Hai vinto!","win"); gameOver=true; }
  else{
    currentAttempt++; currentRow=[];
    if(currentAttempt>=10){ setMessage("💥 Hai perso! Codice: "+secret.join(", "),"lose"); gameOver=true; }
  }
  updateColorButtons(); drawBoard();
}

function setMessage(msg, type="neutral") {
  const el = document.getElementById("message");

  if(!msg) {
    el.innerText = "";
    el.style.display = "none"; // nasconde completamente
    return;
  }

  el.style.display = "block"; // mostra se c'è messaggio
  el.innerText = msg;

  // reset classi
  el.classList.remove("message-neutral","message-win","message-lose");

  if(type==="win") el.classList.add("message-win");
  else if(type==="lose") el.classList.add("message-lose");
  else el.classList.add("message-neutral");
}


function restartGame(){ secret=[]; attempts=[]; currentRow=[]; currentAttempt=0; gameOver=false; setMessage(""); generateSecret(); updateColorButtons(); drawBoard(); }

generateSecret(); drawBoard();
