# 🔵 SETUP BLUETOOTH ON-DEMAND - v1.8.0

## 📋 STRATEGIA IMPLEMENTATA

**Problema risolto:** Bluetooth e riverloop.mp3 andavano in conflitto causando problemi di RAM e audio.

**Soluzione:** 
- **Master:** Apre BT solo per 2-3 secondi → invia comando → chiude BT → riavvia riverloop
- **Slave:** Rimane in modalità passiva, riceve comandi quando il Master si connette
- **Risultato:** ZERO conflitti, audio sempre funzionante, display riceve comandi!

---

## 🎯 STEP 1: CARICA IL CODICE SLAVE

### Hardware necessario:
- ESP32 WROOM-32
- Mini LCD ST7789 (172x320)

### Procedura:
1. Apri Arduino IDE
2. Carica `minilcd/minilcd.ino` sul Display
3. **IMPORTANTE:** Apri Serial Monitor (115200 baud)
4. Aspetta che il display si avvii
5. **Copia il MAC Address** che appare nel Serial Monitor

**Esempio output:**
```
📡 Inizializzazione Bluetooth SLAVE PASSIVO...
✅ BT Slave inizializzato: M9Lab-Display-Slave
   Modalità: SERVER PASSIVO
   MAC Display: A8:42:E3:CA:B9:3C

   ⚠️  IMPORTANTE: Configura questo MAC nel Master!
   Nel file trainstation_bt.ino, modifica la riga:
   uint8_t displayMAC[] = {0xA8, 0x42, 0xE3, 0xCA, 0xB9, 0x3C};
```

6. **ANNOTA IL MAC ADDRESS!** (es: `A8:42:E3:CA:B9:3C`)

---

## 🎯 STEP 2: CONFIGURA IL MASTER

### Hardware necessario:
- ATOM Lite + SPK Hat
- SD Card con audio files

### Procedura:
1. Apri `trainstation_bt/trainstation_bt.ino`
2. Trova questa riga (circa linea 127):

```cpp
uint8_t displayMAC[] = {0xA8, 0x42, 0xE3, 0xCA, 0xB9, 0x3C};  // MODIFICA QUESTO!
```

3. **Sostituisci con il MAC del tuo Display** copiato nello Step 1
   - Esempio: se il MAC è `A8:42:E3:CA:B9:3C`
   - Scrivi: `uint8_t displayMAC[] = {0xA8, 0x42, 0xE3, 0xCA, 0xB9, 0x3C};`

4. Carica il codice sull'ATOM Lite + SPK

---

## 🎯 STEP 3: TEST COMUNICAZIONE

### Test 1: Verifica Display
1. Alimenta il Display
2. Dovresti vedere:
   - Logo M9Lab
   - "BT SLAVE PASSIVO"
   - "BT PRONTO"
   - Pallino GIALLO in alto a destra (= pronto)

### Test 2: Verifica Master
1. Alimenta il Master (ATOM Lite + SPK)
2. Aspetta che riverloop.mp3 parta
3. Apri Serial Monitor (115200 baud)
4. Digita: `testdisplay`
5. Dovresti vedere nel Serial:
   ```
   📡 Invio BT al display...
      Pauso riverloop
      RAM: 45000 bytes
      Connetto a Display MAC: A8:42:E3:CA:B9:3C
   ✅ Connesso!
   📤 Inviato: TRAIN|TRENO TEST|-|M9LAB|TEST BT|12:30|test
   🔌 BT chiuso
      RAM recuperata: 52000
   Riverloop (1) RAM:52000
   ✅ Comando inviato al display
   ```

6. Sul **Display** dovresti vedere:
   - Scritta "TRENO TEST"
   - Pallino diventa VERDE per 2-3 secondi (Master connesso)
   - Torna GIALLO (pronto per prossimo comando)

### Test 3: Test Treno Random
1. Sul Master, digita: `rndtrn`
2. Dovresti sentire l'annuncio audio
3. Sul Display dovresti vedere i dati del treno

### Test 4: Test Meteo
1. Sul Master, digita: `meteo`
2. Riverloop si ferma, Master recupera meteo da WiFi
3. Display riceve dati meteo e li visualizza
4. Master riproduce annuncio audio
5. Riverloop riparte automaticamente

---

## 🎮 COMANDI DISPONIBILI

### Comandi Master (via Serial USB):

```
help             → Lista comandi
testdisplay      → Test comunicazione con display
rndtrn           → Test treno random
meteo            → Annuncio meteo Trieste
playtrain=XYZ    → Annuncio treno (es: 411 = IC bin1 partenza)
alert1..10       → Vari alert
randomplay=1     → Attiva/disattiva eventi random
vol+/vol-/vol=50 → Controllo volume
settime=ntp      → Sincronizza orario con NTP
ram              → Mostra RAM disponibile
togglebt         → Attiva BT cellulare (per config remota)
```

### Pattern Bottone (ATOM Lite):
- **1 click**: Alert 1 (Non indicare i personaggi)
- **2 click**: Alert 9 (Si nascondono 5 personaggi)
- **3 click**: Annuncio meteo
- **Long press (3s)**: Toggle BT cellulare (per configurazione remota)

---

## 📊 INDICATORI STATUS

### Display (Pallino in alto a destra):
- 🟡 **GIALLO**: Pronto, in attesa comandi dal Master
- 🟢 **VERDE**: Master connesso, sta ricevendo comando

### Master (LED RGB ATOM):
- **Spento**: Normale, riverloop attivo
- 🔵 **BLU fisso**: BT cellulare attivo (per config remota, audio fermo)

---

## 🔧 RISOLUZIONE PROBLEMI

### Display non riceve comandi

**1. Verifica MAC Address:**
   - Sul Display Serial Monitor, controlla il MAC stampato
   - Sul Master, verifica che `displayMAC[]` corrisponda

**2. Verifica RAM Master:**
   - Digita `ram` sul Master
   - Se < 40000 bytes, il BT non si avvierà
   - Soluzione: riavvia Master

**3. Display mostra dati vecchi:**
   - Normale! I comandi vengono inviati solo quando necessario
   - Usa `testdisplay` per forzare aggiornamento

### Riverloop non riparte dopo comando

**1. Controlla Serial Monitor Master:**
   - Dopo invio comando dovresti vedere "Riverloop (1) RAM:XXXX"
   - Se non appare, digita `ram` per verificare memoria

**2. Audio glitch durante meteo:**
   - Normale durante fetch API WiFi
   - Riverloop ripartirà automaticamente dopo annuncio

### BT cellulare non si attiva

**1. Verifica RAM:**
   - Digita `ram`
   - Serve almeno 50000 bytes per BT cellulare
   - Se insufficiente, riavvia Master

**2. LED non diventa blu:**
   - Premi bottone per 3 secondi e rilascia
   - Aspetta 2-3 secondi per avvio BT

---

## 📈 VANTAGGI DELLA SOLUZIONE BT ON-DEMAND

✅ **ZERO conflitti** tra BT e riverloop  
✅ **Audio sempre funzionante** (riverloop sempre attivo)  
✅ **Display riceve comandi** affidabilmente  
✅ **RAM ottimizzata** (BT aperto solo 2-3 secondi)  
✅ **BT cellulare opzionale** (per config remota quando serve)  
✅ **Nessun cavo** UART necessario  

---

## 🚀 PROSSIMI PASSI

1. **Test prolungato**: Lascia il sistema acceso per qualche ora
2. **Random play**: Attiva con `randomplay=1` e `setinterval=120` (2 minuti)
3. **Personalizza annunci**: Modifica città meteo, nomi treni, ecc.

---

## 📝 NOTE TECNICHE

### Timing BT On-Demand:
- **Apertura BT**: ~500ms
- **Connessione MAC**: ~300ms
- **Invio comando**: ~150ms
- **Chiusura BT**: ~200ms
- **TOTALE**: ~1.2 secondi (riverloop fermo)

### RAM Usage:
- **Riverloop attivo**: ~45-55 KB free
- **BT display attivo**: ~40-45 KB free (1-2 secondi)
- **BT cellulare attivo**: ~30-35 KB free (audio fermo)

### Formato Comandi Display:
```
TRAIN|destinazione|binario|linea1|linea2|orario|tipo
METEO|temperatura|weathercode|citta|orario
```

Separatore: `|` (pipe) per evitare conflitti con `:` negli orari HH:MM

---

**Versione documento:** 1.0  
**Data:** 15 Gennaio 2026  
**Autore:** M9Lab  
**Codice versione:** Master v1.8.0-BT-ONDEMAND | Slave v2.0-BT-PASSIVO
