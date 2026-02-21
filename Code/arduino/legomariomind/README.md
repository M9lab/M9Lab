# README – Lego Mario Mind

## 📌 Descrizione del progetto

**Lego Mario Mind** è un gioco **Mastermind interattivo** che utilizza **mattoncini LEGO colorati** come controller fisico.

Il sensore del **LEGO Mario Hub** legge i colori dei mattoncini o barcode e li invia all'**ESP32** via Bluetooth. L'ESP32 gestisce:
- 🎮 **WebSocket** per comunicazione real-time con il browser
- 💡 **LED WS2812** (25 LED) per feedback visivo dei colori
- 📡 **Hotspot WiFi** per accesso diretto senza rete esterna

Il gioco è ottimizzato per **mobile** (400x700px) con interfaccia responsive e barra superiore fissa.

---

## 🔧 Componenti Hardware

1. **ESP32** (PICO-D4 o compatibile con WiFi + BT)
2. **LEGO Mario Hub** (con sensore barcode)
3. **LED Strip WS2812** (25 LED) - connessi al pin GPIO 27
4. **Mattoncini LEGO colorati** o barcode LEGO Mario
5. Alimentazione 5V per ESP32 e LED

---

## 📋 Specifiche Tecniche ESP32

### Arduino IDE Configuration

```
Board: ESP32 Dev Module
Upload Speed: 115200
Flash Size: 4MB
Partition Scheme: Default 4MB with spiffs
```

### Librerie richieste

| Libreria | Versione | Note |
|----------|----------|------|
| **esp32** (board) | 2.0.17 | ⚠️ NON usare 3.x (incompatibile con ESPAsyncWebServer) |
| **FastLED** | 3.10.3 | Gestione LED WS2812 (GRB) |
| **ESPAsyncWebServer** | 3.1.0 | Server HTTP asincrono |
| **AsyncTCP** | 1.1.4 | Dipendenza di ESPAsyncWebServer |
| **Legoino** | 1.0.0 | Comunicazione BLE con LEGO Mario Hub (Lpf2Hub, sensore barcode/colore) |

Configurazione consigliata nello sketch: **Upload Speed 115200**.

### Installazione Librerie

**1. ESP32 Board (2.0.17)**
```
Arduino IDE → File → Preferences → Additional Boards Manager URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

Tools → Board → Boards Manager → Cerca "ESP32" → Installa versione 2.0.17
```

**2. Librerie Arduino (Library Manager)**
```
Sketch → Include Library → Manage Libraries → Cerca e installa:
- FastLED (3.10.3)
- Legoino (1.0.0)
```

**3. Librerie da GitHub (installazione manuale)**

**ESPAsyncWebServer:**
```
https://github.com/me-no-dev/ESPAsyncWebServer
Download ZIP → Sketch → Include Library → Add .ZIP Library
```

**AsyncTCP:**
```
https://github.com/me-no-dev/AsyncTCP
Download ZIP → Sketch → Include Library → Add .ZIP Library
```

---

## 📁 Struttura del Progetto

```
legomariomind/
│
├── legomariomind.ino          # Sketch principale (HTML/CSS/JS embedded)
├── README.md                  # Questo file
└── data/                      # File sorgente (NON usati da ESP32)
    ├── index.html             # File di riferimento/test
    ├── style.css              # CSS per modifiche future
    └── mastermind.js          # JavaScript per test locale
```

> ⚠️ **IMPORTANTE**: I file in `data/` sono **SOLO PER RIFERIMENTO**.  
> L'HTML/CSS/JS sono **embedded direttamente** nel file `.ino` per evitare SPIFFS/LittleFS.

---

## ⚙️ Configurazione WiFi

L'ESP32 crea un **Access Point** (Hotspot) con queste credenziali:

```cpp
SSID: legomariomind
Password: legomariomind
IP Address: 192.168.4.1
```

Per modificare, edita queste righe nel file `.ino`:
```cpp
const char* ssid = "legomariomind";
const char* password = "legomariomind";
```

---

## 🚀 Installazione e Upload

### 1. Preparazione Arduino IDE
- Installa tutte le librerie (vedi sopra)
- Seleziona: **Tools → Board → ESP32 Dev Module**
- Seleziona: **Tools → Upload Speed → 115200**
- Connetti ESP32 via USB

### 2. Upload del Codice
1. Apri `legomariomind.ino` nell'Arduino IDE
2. Clicca su **Upload** (→)
3. Se necessario, tieni premuto il pulsante **BOOT** sull'ESP32 durante l'upload
4. Attendi il completamento

### 3. Test Seriale
Apri il **Serial Monitor** (115200 baud):
```
Hotspot attivo!
IP ESP32: 192.168.4.1
```

---

## 🎮 Come Giocare

### 1. Connessione
1. Connetti il tuo smartphone/tablet alla rete WiFi: **legomariomind**
2. Apri il browser e vai a: `http://192.168.4.1`
3. Accendi il LEGO Mario Hub (Bluetooth)

### 2. Controlli

**Con Mario Hub (mattoncini colorati):**
- 🔴 **Rosso** (21), 🟡 **Giallo** (24), 🟢 **Verde** (37), 🟣 **Viola** (12), 🔵 **Blu** (23) = colori di gioco
- 🟠 **Arancio** (106) = **RIAVVIA** (nuova partita)
- ⚪ **Bianco** (19) = **ANNULLA** (cancella riga corrente)
- Il valore **26** (nero) viene **ignorato** (ritorna anche quando alzi Mario dal mattoncino)

**Con Browser (bottoni touch):**
- Clicca sui bottoni colorati per inserire la combinazione
- Pulsante **ANNULLA** (bianco) = cancella riga corrente
- Pulsante **RIAVVIA** (arancio) = nuovo gioco

### 3. LED Feedback
- **LED centrali** (tutti): Mostrano il colore selezionato
- **LED centrale** (12): Stato connessione Mario
  - 🔴 **Rosso**: Mario non connesso
  - 🟡 **Giallo**: Mario in connessione
  - 🟢 **Verde**: Mario connesso e pronto

---

## 🎨 Caratteristiche Interfaccia

### Layout Mobile-First (400x700px)
- **Barra superiore fissa** (#000): Titolo + contatore tentativi
- **Area gioco scrollabile**: 10 righe × 4 slot colore + feedback
- **Barra inferiore fissa** (#000): 5 bottoni colore + 2 controlli
- **Auto-scroll intelligente**: La riga corrente si centra automaticamente
- **Modal centrato**: Messaggi vittoria/sconfitta al centro schermo

### Responsive Design
- Ottimizzato per schermi 400×700px
- Nessuno scroll orizzontale
- Dimensioni slot: 36×36px
- Gap ottimizzati per visibilità completa

### PWA (Progressive Web App)
- **Icona personalizzata**: Cerchio rosso con "M" bianca
- **Aggiungi a Home**: Funziona come app nativa
- **Fullscreen mode**: Nasconde barre browser
- **Offline ready**: Tutto embedded, nessuna richiesta esterna

---

## 🔌 Pin e hardware

### Riepilogo pin (ESP32)

| Uso              | Pin / costante   | Note |
|------------------|------------------|------|
| **LED WS2812**   | **GPIO 27**      | Data In della striscia (definito come `DATA_PIN`) |
| Numero LED       | 25               | `NUM_LEDS` |
| LED centrale     | indice 12        | `CENTER_LED` – indicatore stato Mario (rosso/giallo/verde) |
| WiFi / WebServer | interno          | Nessun pin aggiuntivo |

### LED WS2812 (25 LED)
```
ESP32 GPIO 27 → Data In LED
5V → VCC LED
GND → GND LED
```

**Schema matrice LED (5×5):**
```
 0   1   2   3   4
 5   6   7   8   9
10  11 [12] 13  14    ← LED 12 = indicatore centrale (stato Mario)
15  16  17  18  19
20  21  22  23  24
```

### LEGO Mario Hub
- Connessione automatica via Bluetooth
- Nessun pairing manuale richiesto
- Il LED centrale diventa verde quando connesso

---

## 🛠️ Modifiche e Personalizzazione

### Modificare HTML/CSS/JS
1. Edita i file in `data/` per testing
2. Quando sei soddisfatto, copia le modifiche nel file `.ino`
3. Ri-carica il codice sull'ESP32

### Aggiungere Nuovi Colori
Nel file `.ino`, nella sezione Mario callback:
```cpp
case XX:  fullColor(CRGB::ColorName);  colorName="nome"; break;
```

### Cambiare Dimensioni LED
```cpp
#define NUM_LEDS 25  // Cambia secondo la tua strip
```

---

## 🐛 Troubleshooting

### Non vedo la rete WiFi "legomariomind"
- Premi il pulsante **RESET** sull'ESP32
- Controlla il Serial Monitor: deve apparire "Hotspot attivo!"
- Verifica di aver caricato il codice correttamente

### Mario non si connette (LED centrale rosso)
- Accendi il LEGO Mario Hub
- Avvicinalo all'ESP32 (< 2 metri)
- Riavvia l'ESP32 se necessario

### I LED non si accendono
- Verifica connessione GPIO 27
- Controlla alimentazione 5V dei LED
- Testa con `fullColor(CRGB::White);` nel setup

### Errore di compilazione con ESP32 3.x
- **Soluzione**: Downgrade a ESP32 board 2.0.17
- ESPAsyncWebServer NON è compatibile con 3.x

### WebSocket "CONNECTING state" error
- Già risolto nel codice con controllo `socket.readyState`
- Assicurati di avere l'ultima versione del file `.ino`

---

## 📱 Installazione come App Mobile

### iOS (Safari)
1. Apri `http://192.168.4.1`
2. Tocca il pulsante **Condividi** (quadrato con freccia)
3. Seleziona **"Aggiungi a Home"**
4. Icona "Mario Mind" apparirà sulla home

### Android (Chrome)
1. Apri `http://192.168.4.1`
2. Menu (⋮) → **"Aggiungi a schermata Home"**
3. L'app aprirà in modalità fullscreen

---

## 🎯 Regolamento (Mastermind)

### Obiettivo
Indovinare la **combinazione segreta** di **4 colori** (tra rosso, giallo, verde, viola, blu) nell’ordine esatto.

### Regole
1. **Codice segreto**: 4 colori, ciascuno scelto tra i 5 disponibili (ripetizioni ammesse).
2. **Tentativi**: massimo **10** tentativi.
3. **Inserimento**:  
   - Con **Mario**: appoggia Mario sui mattoncini colorati (o barcode) nell’ordine desiderato; ogni lettura aggiunge un colore alla riga corrente.  
   - Con il **browser**: tocca i 5 cerchi colorati per comporre la riga.
4. **Conferma**: quando hai inserito 4 colori, il gioco valuta automaticamente e mostra il feedback.
5. **Feedback** (pallini a destra di ogni riga):
   - **Pallino bianco pieno** (●): colore **corretto** e in **posizione corretta**.
   - **Pallino bianco vuoto** (○): colore **presente** nel codice ma in **posizione sbagliata**.
   - L’ordine dei pallini **non** indica quali posizioni sono corrette.
6. **Vittoria**: 4 pallini pieni = hai indovinato il codice.
7. **Sconfitta**: dopo 10 tentativi senza vittoria viene mostrato il codice segreto.

### Comandi durante il gioco
- **ANNULLA** (mattoncino bianco o pulsante bianco): cancella la riga corrente e ricomincia a inserire.
- **RIAVVIA** (mattoncino arancio o pulsante arancio): termina la partita e ne inizia una nuova (nuovo codice segreto).

---

## 📊 Mappa Colori Mario Hub

Valori decimali restituiti dal sensore barcode/colore del LEGO Mario. Usati nel codice per `marioColorToLed()` e per il gioco.

### Colori di gioco (combinazione)

| Colore | Codice sensore | Nome inviato al browser | LED / HTML |
|--------|----------------|-------------------------|------------|
| 🔴 Rosso  | 21 | `rosso`  | CRGB::Red      |
| 🟢 Verde  | 37 | `verde`  | CRGB::Green    |
| 🔵 Blu   | 23 | `blu`    | CRGB::Blue / #0066cc in HTML |
| 🟡 Giallo | 24 | `giallo` | CRGB::Yellow   |
| 🟣 Viola | 12 | `viola`  | CRGB::Purple   |

### Comandi speciali

| Colore   | Codice | Azione   | Note |
|----------|--------|----------|------|
| 🟠 Arancio | 106 | **RIAVVIA** | Nuova partita; pulsante RIAVVIA è arancio |
| ⚪ Bianco  | 19 | **ANNULLA** | Cancella la riga corrente |
| — (nero)   | 26 | *ignorato* | Non usato; il sensore ritorna 26 anche quando alzi Mario |

Lo sfondo degli slot vuoti nella griglia è **nero** (#000).

---

## 🔄 Aggiornamenti Futuri

- [ ] Modalità multiplayer
- [ ] Statistiche e punteggi
- [ ] Difficoltà variabile (3/4/5 colori)
- [ ] Suoni e effetti audio
- [ ] Animazioni LED avanzate

---

## 👨‍💻 Crediti

Progetto sviluppato per ESP32 con integrazione LEGO Mario Hub.

**Librerie utilizzate:**
- [FastLED](https://github.com/FastLED/FastLED)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [Legoino](https://github.com/corneliusmunz/legoino)

---

## 📄 Licenza

Progetto open source per uso educativo e personale.

---

**Buon divertimento con Lego Mario Mind! 🎮🔴**


Altri colori papabili
azzurro chiaro=66
verde stazione = 26
viola chiaro = 68