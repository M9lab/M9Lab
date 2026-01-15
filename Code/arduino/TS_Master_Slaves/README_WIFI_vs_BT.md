# 🚂 TrainStation Display - WiFi vs Bluetooth

## 📁 Struttura Progetto

### **Master (trainstation_bt/)**
- `trainstation_bt.ino` - **v1.7.0-WIFI-TCP**
  - WiFi TCP Server porta 8888
  - BT Cellulare opzionale per configurazione
  - **Riverloop SEMPRE attivo** (WiFi risparmia RAM!)

### **Display - Due versioni:**

#### **1. minilcd/** - Versione Bluetooth Classic
- `minilcd.ino` - **v1.2-BT**
- Connessione: Bluetooth Classic
- Range: ~10 metri
- Setup: Pairing automatico via MAC
- RAM Master: ~40KB usati
- Riverloop Master: ❌ Disabilitato

#### **2. minilcd_wifi/** - Versione WiFi TCP ⭐ **CONSIGLIATA**
- `minilcd_wifi.ino` - **v2.0-WIFI-TCP**
- Connessione: WiFi TCP/IP
- Range: ~50+ metri
- Setup: Configurare IP Master
- RAM Master: ~20KB usati
- Riverloop Master: ✅ **Sempre attivo!**

---

## 🎯 Quale usare?

### **Usa WiFi TCP (`minilcd_wifi/`) se:**
- ✅ Vuoi il **riverloop** attivo sul Master
- ✅ Vuoi maggiore **stabilità**
- ✅ Hai una rete WiFi disponibile
- ✅ Vuoi **risparmiare RAM** sul Master (~20KB liberi in più!)
- ✅ **RACCOMANDATO** ⭐

### **Usa Bluetooth (`minilcd/`) se:**
- ✅ NON hai WiFi disponibile
- ✅ Vuoi connessione diretta senza router
- ✅ Non ti serve il riverloop

---

## ⚙️ Setup WiFi TCP

### **Passo 1: Master**
1. Apri `trainstation_bt/trainstation_bt.ino`
2. Carica su **Atom Lite + SPK**
3. Apri Serial Monitor
4. **Annota l'IP** che appare (es: `IP: 192.168.43.100`)

### **Passo 2: Display**
1. Apri `minilcd_wifi/minilcd_wifi.ino`
2. Vai alla **riga 26**:
```cpp
const char* masterIP = "192.168.43.1";  // ← MODIFICA QUESTO!
```
3. Sostituisci con l'IP del Master dal Passo 1
4. Carica su **Mini LCD**
5. Dovrebbe connettersi automaticamente! 🎉

---

## 🔍 Status Pallino Display

### **WiFi TCP Version:**
- 🟢 **Verde** = TCP connesso al Master
- 🟡 **Giallo lampeggiante** = WiFi OK, riconnessione TCP in corso
- 🔴 **Rosso** = WiFi disconnesso

### **Bluetooth Version:**
- 🟢 **Verde** = BT connesso al Master
- 🟡 **Giallo lampeggiante** = Tentativo connessione BT
- 🔴 **Rosso** = BT disconnesso

---

## 📊 Confronto Prestazioni

| Caratteristica | Bluetooth Classic | WiFi TCP |
|----------------|-------------------|----------|
| **RAM Master** | ~20KB liberi | ~40KB liberi |
| **Riverloop** | ❌ Disabilitato | ✅ Attivo |
| **Stabilità** | Media | Alta |
| **Range** | ~10m | ~50m+ |
| **Setup** | MAC pairing | IP config |
| **Latenza** | ~50ms | ~20ms |
| **Velocità** | 1-3 Mbps | 54+ Mbps |

---

## 🐛 Troubleshooting WiFi

### **Display non si connette:**
1. Verifica che Master e Display siano sulla **stessa rete WiFi**
2. Controlla l'**IP del Master** nel Serial Monitor
3. Verifica che l'IP nel `minilcd_wifi.ino` sia **corretto**
4. Controlla il **firewall** del router (porta 8888)

### **Connessione instabile:**
1. Avvicina il Display al router
2. Verifica la qualità del segnale WiFi
3. Usa un canale WiFi meno congestionato

---

## 📝 Note Tecniche

### **Perché WiFi consuma meno RAM?**
Lo stack Bluetooth Classic è molto pesante (~40KB). WiFi usa solo il client TCP che è molto più leggero (~20KB), lasciando più RAM per:
- Buffer audio I2S più grandi
- **Riverloop** sempre attivo
- Annunci più fluidi

### **Protocollo comandi:**
Entrambe le versioni usano lo stesso formato:
```
METEO|temp|weathercode|citta|orario
TRAIN|dest|bin|linea1|linea2|orario|tipo
```
Separatore: `|` (pipe) per compatibilità con orari `HH:MM`

---

## 🚀 Versioni Future

### **In arrivo:**
- 🔮 **ESP-NOW** version (solo ~10KB RAM!)
- 🔮 Display multipli simultanei
- 🔮 OTA updates via WiFi

---

**Creato da:** M9Lab  
**Versione:** 2.0 - Gennaio 2026  
**Hardware:** ESP32 Atom Lite + SPK / Mini LCD  
