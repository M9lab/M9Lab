# 🚀 Quick Start - M9Lab TrainStation System

## 📦 Sistema Completo

- **Master:** ATOM Lite + SPK (trainstation_bt v1.5.2)
- **Display:** ESP32 + LCD ST7789 (minilcd v1.1)

---

## ⚡ Avvio Rapido (5 minuti)

### **Step 1: Carica il Codice** 📥

```
1. Carica trainstation_bt.ino sul Master (ATOM Lite)
2. Carica minilcd.ino sul Display
```

### **Step 2: Accendi i Dispositivi** 🔌

#### **a) Accendi il Display PRIMA:**
```
Display LCD mostra:
  ┌──────────────────────┐
  │ Bluetooth Slave...   │
  │ BT PRONTO            │
  │                      │
  │ In attesa Master     │
  │ Sul Master digita:   │
  │ enableslave          │
  └──────────────────────┘

Serial Monitor (115200):
  === M9LAB DISPLAY SLAVE ===
  Display OK
  Inizializzazione Bluetooth SLAVE...
  ✅ BT Slave inizializzato: M9Lab-Display-Slave
     Modalità: PASSIVA
     In attesa che il Master si connetta...
```

#### **b) Accendi il Master:**
```
Serial Monitor (115200):
  === ATOM LITE + SPK + BT ===
  RAM iniziale: 85000
  SD OK
  I2S OK
  MP3 OK
  
  --- Inizializzazione BT Slave ---
  ℹ️  BT Slave DISABILITATO per sicurezza
     Per attivarlo manualmente: digita 'enableslave'
  ---
  
  Riverloop (1) RAM:78000
  [Audio di sottofondo in esecuzione]
```

### **Step 3: Connetti Master al Display** 🔵

Nel Serial Monitor del **Master**, digita:
```
enableslave
```

Output atteso:
```
🔵 Attivo BT Slave...
RAM disponibile: 75744
Pulizia stato BT precedente...
Inizializzazione BT Slave...
✅ BT Slave attivo: M9Lab-TrainStation-Master
   In attesa connessione display...
```

Dopo 2-3 secondi, sul **Display** vedrai:
```
Serial Monitor:
  ✅ BT CONNESSO AL MASTER
     Display pronto per ricevere comandi

Display LCD:
  [Barra verde in alto]
  "BT CONNESSO AL MASTER"
  [torna alla schermata normale]
```

### **Step 4: Testa il Sistema** ✅

Nel Serial Monitor del **Master**, prova questi comandi:

#### **Test Meteo:**
```
meteo
```

**Risultato:**
- Master: Annuncio audio meteo
- Display: Mostra dati meteo con icone

#### **Test Treno:**
```
playtrain=141
```
(Frecciabianca, binario 4, partenza)

**Risultato:**
- Master: Annuncio audio treno
- Display: Mostra info treno

---

## 🎮 Comandi Disponibili sul Master

### **Audio:**
```
playtrain=XYZ    → Annuncio treno
                   X=tipo (1-7): 1=FB, 2=FR, 3=I, 4=IC, 5=RV, 6=ICN, 7=R
                   Y=binario (1-9)
                   Z=azione (1=partenza, 2=arrivo)

meteo            → Annuncio meteo + visualizzazione su display
alert1..alert10  → Vari annunci
```

### **Controllo:**
```
help             → Mostra tutti i comandi
vol+/vol-        → Regola volume
vol=50           → Imposta volume 50%
randomplay=1     → Attiva annunci casuali
settime=ntp      → Sincronizza orario con WiFi
```

### **Bluetooth:**
```
enableslave      → Attiva BT Slave (connessione display)
disableslave     → Disattiva BT Slave
togglebt         → Toggle BT cellulare (disabilita audio)
```

### **Debug:**
```
ram              → Mostra RAM disponibile
scanwifi         → Scansiona reti WiFi
gettime          → Mostra orario
```

---

## 🔘 Controllo con Bottone (Master)

Il Master ha un bottone che risponde a diversi pattern:

```
1 click  → ALERT1 (Non indicare i personaggi)
2 click  → ALERT9 (Si nascondono 5 personaggi)
3 click  → METEO (Annuncio meteo completo)
Long (3s)→ Toggle BT cellulare (on/off)
```

---

## 🎯 Indicatori Visivi Display

### **Dot in alto a destra:**
- 🟢 **Verde** = Connesso al Master
- 🔴 **Rosso** = Non connesso

### **Barra superiore:**
- 🟩 **Verde** = Connessione stabilita (2 secondi)
- 🟥 **Rosso** = Connessione persa

---

## 🔍 Troubleshooting Rapido

### ❌ **Display non si connette**

**Causa:** BT Slave non attivato sul Master

**Soluzione:**
```
Sul Master: enableslave
```

---

### ❌ **Master dice "RAM insufficiente"**

**Causa:** RAM < 45KB (probabile BT cellulare attivo)

**Soluzione:**
```
1. Long press bottone (3s) per disattivare BT cellulare
2. Attendi 5 secondi
3. Riprova: enableslave
```

---

### ❌ **Master si riavvia continuamente**

**Causa:** Problema hardware o SD card

**Soluzione:**
```
1. Verifica SD card inserita e funzionante
2. Verifica file riverloop.mp3 presente
3. Verifica alimentazione 5V 2A stabile
4. NON attivare BT Slave se continua a riavviarsi
```

---

### ❌ **Audio non funziona**

**Causa:** BT cellulare attivo o file mancanti

**Soluzione:**
```
1. Verifica: help → BT Cellulare deve essere OFF
2. Se ON: long press bottone (3s)
3. Verifica file su SD card presenti
```

---

### ❌ **Display mostra dati vecchi**

**Causa:** Comandi non ricevuti

**Soluzione:**
```
1. Verifica dot verde in alto a destra display
2. Se rosso: enableslave sul Master
3. Riprova comando (es. meteo)
```

---

## 📊 Sequenza di Avvio Consigliata

```
┌─────────────────────────────────────────────┐
│ ORDINE CORRETTO DI ACCENSIONE               │
└─────────────────────────────────────────────┘

1. 📺 Accendi DISPLAY
   └─→ Aspetta BT pronto (10 sec)

2. 🎵 Accendi MASTER  
   └─→ Aspetta boot completo (10 sec)
   └─→ Verifica audio riverloop attivo

3. 🔵 Attiva BT Slave sul Master
   └─→ Digita: enableslave
   └─→ Aspetta "BT Slave attivo" (2 sec)

4. ✅ Verifica connessione
   └─→ Display mostra barra verde
   └─→ Dot verde in alto a destra

5. 🎉 Sistema pronto!
   └─→ Testa con: meteo
```

---

## 💡 Tips & Tricks

### **Tip 1: Ordine di accensione**
Accendi sempre il Display **PRIMA** del Master per evitare attese.

### **Tip 2: Verifica connessione**
Guarda il dot in alto a destra sul display:
- Verde = tutto OK
- Rosso = attiva BT Slave sul Master

### **Tip 3: RAM bassa?**
Se il Master ha poca RAM:
1. NON attivare BT cellulare
2. NON attivare BT Slave
3. Usa solo Serial Monitor per comandi

### **Tip 4: Reset rapido**
Se qualcosa non va:
1. Bottone reset su entrambi
2. Segui sequenza di avvio sopra

### **Tip 5: Modalità demo**
Per demo senza display:
- NON eseguire `enableslave`
- Usa solo comandi audio via Serial

---

## 📁 File Necessari su SD Card

```
SD Card (FAT32):
├── riverloop.mp3        → Audio di sottofondo
├── 0001.mp3 - 0999.mp3  → File audio annunci
└── [Struttura file audio M9Lab]
```

---

## 🆘 Link Utili

- **Documentazione completa BT Slave:** `BT_SLAVE_SETUP.md`
- **Schema comandi:** `help` sul Master
- **Test funzionalità:** `meteo` e `playtrain=141`

---

## ✅ Checklist Pre-Avvio

Prima di accendere, verifica:

- [ ] SD card inserita nel Master con file audio
- [ ] Alimentazione 5V 2A per Master
- [ ] Alimentazione per Display
- [ ] Serial Monitor aperto (115200 baud)
- [ ] Display acceso e operativo

Tutto OK? **Segui la sequenza di avvio sopra!** 🚀

---

**Versione:** 1.0  
**Data:** 2026-01-12  
**Compatibile con:**
- trainstation_bt v1.5.2+
- minilcd v1.1+
