# README – Mastermind LEGO con Mario e ESP32

## 📌 Descrizione del progetto

Questo progetto permette di giocare a **Mastermind** usando **mattoncini LEGO colorati** come controller.
Il sensore del **LEGO Mario Hub** legge i colori dei mattoncini o dei barcode colorati e li invia all’ESP32, che a sua volta comunica i colori al **browser** tramite **WebSocket**.

Il browser aggiorna il gioco Mastermind in tempo reale, permettendo di giocare con i mattoncini come input fisico.

---

## 🔧 Componenti necessari

1. **ESP32** (qualsiasi modello con Wi-Fi)
2. **LEGO Mario Hub** (Mario o barcode colorato)
3. Mattoncini LEGO colorati o barre codificate LEGO Mario
4. Cavi e alimentazione per ESP32
5. Browser moderno (Chrome, Firefox, Edge)

**Opzionale:**

* LED WS2812 per visualizzare il colore fisico

---

## 📁 Struttura dei file

```
MastermindESP32/
│
├─ MastermindESP32.ino      # Sketch principale ESP32
└─ data/                    # File da caricare su SPIFFS
   ├─ index.html            # Pagina Mastermind
   ├─ style.css             # Stile della pagina
   ├─ mastermind.js         # Logica Mastermind + WebSocket
   └─ backg.webp            # Sfondo opzionale
```

> La cartella `data/` **non viene compilata direttamente**, serve solo per SPIFFS.

---

## ⚙️ Configurazione Wi-Fi

Apri `MastermindESP32.ino` e modifica queste righe:

```cpp
const char* ssid = "NOME_RETE";        // il tuo SSID Wi-Fi
const char* password = "PASSWORD_RETE"; // la password Wi-Fi
```

---

## 💾 Caricamento dei file su ESP32

1. Installa **ESP32FS Tool** in Arduino IDE:

   * Scarica da: [https://github.com/me-no-dev/arduino-esp32fs-plugin](https://github.com/me-no-dev/arduino-esp32fs-plugin)
   * Copia nella cartella `tools` di Arduino IDE
   * Riavvia Arduino IDE

2. Apri il tuo sketch `MastermindESP32.ino`.

3. Vai su **Strumenti → ESP32 Sketch Data Upload**.

   * Questo caricherà i file della cartella `data/` su SPIFFS.

---

## 🌐 Accesso alla pagina Mastermind

Carica il .ino sull’ESP32

Usa ESP32FS per caricare la cartella data/ su SPIFFS

Accendi l’ESP32 e attendi il messaggio seriale “Hotspot attivo!”

Collega il cellulare al Wi-Fi MastermindESP32

Apri http://192.168.4.1/ sul browser

Usa i mattoncini LEGO per giocare

---

## 🔹 Come caricare data/ su SPIFFS

Per far sì che l’ESP32 legga i tuoi file HTML/CSS/JS:

Scarica il plugin ESP32FS per Arduino IDE

Link: ESP32FS Tool GitHub

Segui le istruzioni per installarlo nella cartella tools di Arduino IDE

Crea la cartella data/ accanto al file .ino

Ad esempio:

MastermindESP32/
├── MastermindESP32.ino
└── data/
    ├── index.html
    ├── style.css
    ├── mastermind.js
    └── backg.webp


Apri Arduino IDE e seleziona la scheda ESP32 corretta.

Dal menù Strumenti > ESP32 Sketch Data Upload

Questo comando carica tutta la cartella data/ nella memoria SPIFFS dell’ESP32.

Quando finisce, vedrai un messaggio tipo: “Upload finished successfully”.

Ora l’ESP32 può servire i file tramite:

server.serveStatic("/", SPIFFS, "/index.html");
server.serveStatic("/style.css", SPIFFS, "/style.css");
server.serveStatic("/mastermind.js", SPIFFS, "/mastermind.js");
server.serveStatic("/backg.webp", SPIFFS, "/backg.webp");

https://github.com/me-no-dev/arduino-esp32fs-plugin?tab=readme-ov-file

## 🟩 Come giocare

1. Posiziona un mattoncino LEGO colorato (rosso, giallo, verde, blu, viola) sotto il sensore di Mario Hub.
2. Il colore rilevato appare **nel browser** e viene inserito automaticamente come tentativo nel gioco.
3. I colori speciali:

   * **Nero** = ANNULLA
   * **Bianco** = RIAVVIA

> Il colore **viola** potrebbe richiedere debug con `Serial.println()` per verificare il codice esatto.

---

## ⚡ Debug dei colori

Nel Serial Monitor puoi vedere i valori numerici dei colori rilevati da Mario Hub:

```cpp
Serial.print("Mario color detected: ");
Serial.println((byte)color);
```

* Questo ti permette di aggiornare `MastermindESP32.ino` se qualche colore non viene letto correttamente.

---


## ⚡️ Uso del browser in modalità a tutto schermo (Mobile)
Chrome / Edge / Firefox (Android)

1. Apri la pagina http://192.168.4.1// sul browser.
2. Tocca il menu (⋮) → seleziona Aggiungi a schermata Home.
3 .Questo creerà un’icona come una web app. Quando la apri da lì:
4 .La pagina viene mostrata full screen, senza barra degli indirizzi.


## 📝 Modifiche possibili

* Aggiungere nuovi colori o barcode personalizzati
* Cambiare sfondo `backg.webp` o stile della board
* Aggiungere LED WS2812 per visualizzare il colore fisico

---

## ✅ Conclusione

Con questo setup puoi trasformare **i mattoncini LEGO in controller fisici** per giocare a Mastermind!
La comunicazione in tempo reale tra ESP32 e browser tramite WebSocket rende l’esperienza interattiva
