# ⚙️ Configurazione Auto-Start BT Slave - v1.5.4

## 📋 Configurazione Attuale

**BT Slave: ✅ AUTO-START ATTIVO**

```cpp
#define BT_SLAVE_AUTO_START true
```

---

## 🚀 Comportamento all'Avvio

### **Sequenza Automatica:**

```
1. Sistema si avvia
   ↓
2. Inizializzazione hardware (SD, I2S, MP3)
   ↓
3. Sincronizzazione NTP + Meteo
   ↓
4. BT Slave si ATTIVA AUTOMATICAMENTE
   ↓
5. Riverloop DISABILITATO (RAM insufficiente)
   ↓
6. Sistema pronto per connessione Display
```

---

## 📊 Output Serial All'Avvio

```
=== ATOM LITE + SPK + BT ===
RAM iniziale: 85000
SD OK
I2S OK
MP3 OK
RAM disponibile: 78000

--- Sincronizzazione orario + Meteo ---
[... sincronizzazione NTP ...]
[... annuncio meteo ...]

--- Inizializzazione BT Slave ---
⚠️  Auto-start attivo
   Se il sistema si riavvia, cambia BT_SLAVE_AUTO_START a false
Fermo riverloop per inizializzazione BT...
🔵 Attivo BT Slave...
RAM disponibile: 75744
Pulizia stato BT precedente...
Inizializzazione BT Slave...
✅ BT Slave attivo: M9Lab-TrainStation-Master
   In attesa connessione display...
⚠️  RIVERLOOP DISABILITATO (RAM insufficiente)
   Audio di sottofondo non attivo durante BT Slave
   Annunci audio funzionano normalmente
✅ BT Slave pronto per connessione display
---

[sistema pronto]
```

---

## 🖥️ Connessione Display

### **Il Display si connette automaticamente:**

**Display Serial Monitor:**
```
=== M9LAB DISPLAY SLAVE ===
Display OK
Inizializzazione Bluetooth SLAVE...
✅ BT Slave inizializzato: M9Lab-Display-Slave
   Modalità: PASSIVA
   In attesa che il Master si connetta...

[dopo 2-3 secondi]

✅ BT CONNESSO AL MASTER
   Display pronto per ricevere comandi
```

**Master Serial Monitor:**
```
[nessun messaggio particolare, connessione silenziosa]
```

---

## ✅ Vantaggi Auto-Start

### **1. Plug & Play**
- Accendi Master → BT Slave attivo
- Accendi Display → Si connette automaticamente
- **Zero configurazione manuale**

### **2. Demo/Produzione**
- Sistema pronto immediatamente
- Nessun comando da digitare
- Esperienza utente fluida

### **3. Affidabilità**
- Nessuna dimenticanza (enableslave)
- Sempre pronto per il display
- Comportamento prevedibile

---

## ⚠️ Limitazioni

### **1. Riverloop Sempre Disabilitato**
- Audio di sottofondo NON disponibile
- Solo annunci audio su richiesta
- **Se serve riverloop:** disabilita auto-start

### **2. RAM Occupata Permanentemente**
- BT Slave usa ~25KB RAM sempre
- Meno RAM disponibile per operazioni
- **Non un problema** con ATOM Lite in questa config

### **3. Impossibile Disattivare senza Reboot**
- BT Slave attivo dalla prima accensione
- `disableslave` funziona, ma al prossimo reboot si riattiva
- **Per cambiare:** modifica codice e ricarica

---

## 🔄 Come Disabilitare Auto-Start

### **Se vuoi tornare alla modalità manuale:**

1. **Apri** `trainstation_bt.ino`

2. **Trova questa riga** (circa riga 140):
```cpp
#define BT_SLAVE_AUTO_START true
```

3. **Cambia in:**
```cpp
#define BT_SLAVE_AUTO_START false
```

4. **Ricarica** il codice sul Master

5. **Risultato:**
```
--- Inizializzazione BT Slave ---
ℹ️  BT Slave NON ATTIVATO (auto-start disabilitato)
   Per attivarlo manualmente: digita 'enableslave'
---

Riverloop (1) RAM:78000  ← Audio di sottofondo ATTIVO
```

---

## 📝 Comandi Disponibili

### **Con Auto-Start Attivo:**

```bash
# Display già connesso, comandi funzionano subito
meteo            → Annuncio + Display
playtrain=141    → Annuncio + Display
alert1..alert10  → Solo annuncio (Display ignora)

# Gestione BT Slave
disableslave     → Disattiva BT + Riattiva Riverloop
                   (al reboot tornerà attivo)

enableslave      → Non necessario (già attivo)
                   Ma funziona se l'hai disattivato

# Altri comandi
help             → Mostra stato sistema
ram              → Mostra RAM disponibile
```

---

## 🎯 Scenari d'Uso

### **SCENARIO 1: Sistema Demo/Produzione (CONSIGLIATO)**
```
Configurazione: AUTO-START = true

Avvio:
1. Accendi Master → BT Slave attivo
2. Accendi Display → Connessione automatica
3. Usa comandi → Tutto funziona

Vantaggi:
✅ Zero configurazione
✅ Esperienza fluida
✅ Pronto per demo/produzione

Svantaggi:
❌ Nessun audio di sottofondo
```

### **SCENARIO 2: Sistema con Riverloop (Sviluppo)**
```
Configurazione: AUTO-START = false

Avvio:
1. Accendi Master → Riverloop attivo
2. Audio di sottofondo → Atmosfera
3. Quando serve display → enableslave
4. Display si connette
5. Quando non serve → disableslave

Vantaggi:
✅ Audio di sottofondo disponibile
✅ Flessibilità

Svantaggi:
❌ Serve comando manuale
❌ Non plug & play
```

---

## 🔧 Troubleshooting

### **❓ "Il Display non si connette"**

**Verifica:**
1. Master acceso? Serial Monitor mostra "BT Slave attivo"?
2. Display acceso? Serial Monitor mostra "In attesa Master"?
3. Distanza < 10 metri?
4. Nessuna interferenza Bluetooth?

**Soluzione rapida:**
```
Sul Master: help
Cerca riga: BT Slave: ATTIVO
Se vedi OFF → qualcosa è andato storto al boot
```

---

### **❓ "Master si riavvia all'avvio"**

**Causa:** RAM insufficiente o conflitto

**Soluzione:**
1. Disabilita auto-start (vedi sopra)
2. Ricarica codice
3. Usa `enableslave` manualmente quando serve

---

### **❓ "Voglio riverloop + BT Slave insieme"**

**Non possibile su ATOM Lite** (RAM limitata: 95KB)

**Soluzioni:**
1. **Disabilita auto-start** → Usa BT Slave solo quando serve
2. **Upgrade hardware** → ESP32-WROVER (4MB PSRAM)
3. **Accetta trade-off** → Riverloop solo senza display

---

## 📊 Confronto Modalità

| Caratteristica | AUTO-START ON | AUTO-START OFF |
|---------------|---------------|----------------|
| Riverloop | ❌ Sempre OFF | ✅ Disponibile |
| BT Slave | ✅ Sempre ON | 🔧 Manuale |
| Display | ✅ Auto-connessione | 🔧 Dopo enableslave |
| Annunci | ✅ Funzionano | ✅ Funzionano |
| RAM Disponibile | ~70KB | ~75-80KB |
| Configurazione | 🚀 Zero | 📝 Comando manuale |
| Uso Consigliato | 🎭 Demo/Produzione | 🔬 Sviluppo |

---

## 🎉 Conclusione

**Configurazione Attuale: OTTIMALE per Demo/Produzione**

✅ Sistema plug & play  
✅ Display si connette automaticamente  
✅ Annunci funzionano perfettamente  
✅ Zero configurazione manuale  
✅ Comportamento prevedibile  

**Trade-off Accettabile:**
❌ Nessun audio di sottofondo (riverloop)  
✅ Ma sistema completamente funzionale  

**Se serve riverloop:** Disabilita auto-start e usa modalità manuale.

---

**Versione:** 1.5.4-AUTOSTART-BTSLAVE  
**Data:** 2026-01-12  
**Configurazione:** BT_SLAVE_AUTO_START = **true**
