# 🔵 Guida BT Slave - M9Lab TrainStation

## 📋 Versione: 1.5.2-BT-SAFE-REBOOT

---

## ⚠️ IMPORTANTE: Cambio nel comportamento del BT Slave

Il BT Slave è ora **DISABILITATO DI DEFAULT** per evitare loop di reboot durante l'avvio.

---

## 🚀 Quick Start

### 1. **Prima accensione:**
```
Carica il codice → Apri Serial Monitor (115200 baud)
Dovresti vedere:
  --- Inizializzazione BT Slave ---
  ℹ️  BT Slave DISABILITATO per sicurezza
     Per attivarlo manualmente: digita 'enableslave'
```

### 2. **Attivare il BT Slave:**
```
Digita nel Serial Monitor: enableslave
Output atteso:
  🔵 Attivo BT Slave...
  RAM disponibile: 75000
  Pulizia stato BT precedente...
  Inizializzazione BT Slave...
  ✅ BT Slave attivo: M9Lab-TrainStation-Master
     In attesa connessione display...
```

### 3. **Accendi il Display:**
Il display si connetterà automaticamente al Master.

---

## 🔧 Configurazione Avanzata

### **Auto-start del BT Slave (SCONSIGLIATO):**

Se vuoi che il BT Slave si attivi automaticamente all'avvio (può causare reboot):

1. Nel file `trainstation_bt.ino`, cerca:
```cpp
#define BT_SLAVE_AUTO_START false
```

2. Cambia in:
```cpp
#define BT_SLAVE_AUTO_START true
```

3. Ricarica il codice

⚠️ **ATTENZIONE:** Se il sistema va in reboot loop, dovrai:
- Ricaricare il codice con `BT_SLAVE_AUTO_START false`
- Usare `enableslave` manualmente

---

## 📝 Comandi Disponibili

### **Gestione BT Slave:**
```
enableslave   → Attiva BT Slave manualmente
disableslave  → Disattiva BT Slave
```

### **Verifica Stato:**
```
help          → Mostra tutti i comandi + stato BT Slave
ram           → Mostra RAM disponibile
```

### **Controllo Treni:**
```
playtrain=XYZ → Annuncio treno
                X = tipo (1-7): 1=FB, 2=FR, 3=I, 4=IC, 5=RV, 6=ICN, 7=R
                Y = binario (1-9)
                Z = azione (1=partenza, 2=arrivo)
Esempio: playtrain=141 → Frecciabianca binario 4 in partenza
```

### **Meteo:**
```
meteo         → Annuncio meteo + invio dati al display
```

---

## 🔍 Troubleshooting

### **Problema: "RAM insufficiente per BT Slave"**
**Soluzione:**
```
1. Digita: ram
2. Se RAM < 45000 bytes:
   - Riavvia il sistema
   - Non attivare BT cellulare (non fare long press)
   - Riprova: enableslave
```

### **Problema: "Impossibile avviare BT Slave"**
**Soluzione:**
```
1. Riavvia il sistema (reset button)
2. Attendi 5 secondi dopo il boot
3. Riprova: enableslave
4. Se persiste, verifica alimentazione (USB 5V 2A)
```

### **Problema: "Display non si connette"**
**Verifica:**
```
1. Display acceso e operativo?
2. BT Slave attivo sul Master? (digita: help)
3. Sul display, Serial Monitor dovrebbe mostrare:
   "Connessione a Master: M9Lab-TrainStation-Master"
```

### **Problema: Sistema si riavvia continuamente**
**Soluzione:**
```
1. NON attivare BT Slave all'avvio
2. Il sistema si avvierà normalmente senza BT Slave
3. Puoi usare tutti i comandi via Serial Monitor
4. Verifica:
   - SD card inserita correttamente
   - File riverloop.mp3 presente
   - Alimentazione stabile
```

---

## 📊 Requisiti RAM

| Componente | RAM Richiesta |
|------------|---------------|
| Sistema base | ~30KB |
| Audio + SD | ~20KB |
| **BT Slave** | **~25KB** |
| BT Cellulare | ~40KB |
| **Totale minimo** | **75KB** |

**NOTA:** ATOM Lite ha ~95KB RAM disponibile dopo boot.

---

## ✅ Flusso Operativo Consigliato

```
1. Accendi Master (ATOM Lite)
   → Sistema si avvia senza BT Slave
   → Riverloop audio in esecuzione

2. Verifica sistema stabile (10-20 secondi)

3. Attiva BT Slave manualmente:
   enableslave

4. Accendi Display
   → Si connette automaticamente

5. Testa funzionalità:
   meteo          → Display mostra meteo
   playtrain=141  → Display mostra treno
```

---

## 🎯 Perché BT Slave è Disabilitato di Default?

1. **Prevenzione Reboot Loop:** 
   - Inizializzazione BT può causare assert failure in FreeRTOS
   - Disabilitandolo di default, il sistema è sempre stabile

2. **Flessibilità:**
   - Puoi usare il sistema anche senza display
   - Attivi BT Slave solo quando serve

3. **Debug Facilitato:**
   - Sistema si avvia sempre correttamente
   - Puoi verificare RAM e stato prima di attivare BT

4. **Compatibilità:**
   - Funziona anche con RAM limitata
   - Puoi scegliere tra BT Slave o BT Cellulare

---

## 🆘 Supporto

Se continui ad avere problemi:

1. **Verifica Hardware:**
   - ATOM Lite + SPK correttamente collegato
   - SD card funzionante (formattata FAT32)
   - Alimentazione 5V 2A stabile

2. **Verifica Software:**
   - Librerie ESP32 aggiornate
   - M5Atom library installata
   - File audio presenti su SD

3. **Log Serial:**
   - Apri Serial Monitor a 115200 baud
   - Copia tutto l'output dal boot
   - Analizza eventuali errori

---

## 📦 File Richiesti su SD Card

```
/riverloop.mp3         → Audio di sottofondo
/0001.mp3 - /0999.mp3  → File audio annunci
```

Verifica presenza con comando Serial dopo boot.

---

**Versione documento:** 1.0  
**Data:** 2026-01-12  
**Compatibile con:** trainstation_bt v1.5.2+
