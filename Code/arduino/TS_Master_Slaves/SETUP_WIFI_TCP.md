# 📡 Setup WiFi TCP - Guida Passo Passo

## 🎯 Obiettivo
Configurare comunicazione Master → Display tramite WiFi TCP per:
- ✅ **Risparmiare RAM** (~20KB in più liberi!)
- ✅ **Riverloop sempre attivo**
- ✅ **Connessione più stabile**
- ✅ **Range maggiore** (~50m+)

---

## 📦 Hardware Necessario
1. **Master:** ESP32 Atom Lite + SPK + SD card
2. **Display:** ESP32 Mini LCD
3. **Rete WiFi:** Router o hotspot mobile (es: "StefxMobile")

---

## 🔧 PASSO 1: Configurare il Master

### 1.1 Aprire il file Master
```
trainstation_bt/trainstation_bt.ino
```

### 1.2 Verificare configurazione WiFi (righe 51-52)
```cpp
const char* wifiSSID = "StefxMobile";  // Nome rete WiFi
const char* wifiPWD = "qwerty123456"; // Password WiFi
```
**⚠️ IMPORTANTE:** Se usi una rete WiFi diversa, modifica questi valori!

### 1.3 Caricare su Atom Lite
1. Collega l'Atom Lite via USB
2. Seleziona board: **"ESP32 Dev Module"** o **"M5Atom"**
3. Seleziona porta COM
4. Clicca **Upload** (Carica)

### 1.4 Annotare l'IP del Master
1. Apri **Serial Monitor** (115200 baud)
2. Attendi l'avvio
3. **Cerca questa sezione:**

```
🌐 Avvio WiFi TCP Server per display...
✅ WiFi TCP Server attivo
   IP: 192.168.43.100  ← QUESTO È L'IP DEL MASTER!
   Porta: 8888
   In attesa connessione display...
```

4. **📝 ANNOTA L'IP** (es: `192.168.43.100`)
   - Ti servirà nel passo 2!

---

## 🖥️ PASSO 2: Configurare il Display

### 2.1 Aprire il file Display WiFi
```
minilcd_wifi/minilcd_wifi.ino
```

### 2.2 Configurare WiFi (righe 32-34)
```cpp
const char* wifiSSID = "StefxMobile";  // Stessa rete del Master!
const char* wifiPWD = "qwerty123456"; // Stessa password!
const char* masterIP = "192.168.43.1"; // ← MODIFICA QUESTO!
```

**⚠️ IMPORTANTE:**
1. Usa **stessi SSID e password** del Master
2. Cambia `masterIP` con l'IP annotato nel Passo 1.4

**Esempio:**
Se il Master ha IP `192.168.43.100`, scrivi:
```cpp
const char* masterIP = "192.168.43.100";
```

### 2.3 Caricare su Mini LCD
1. Collega il Mini LCD via USB
2. Seleziona board: **"ESP32 Dev Module"**
3. Seleziona porta COM (diversa dal Master!)
4. Clicca **Upload** (Carica)

### 2.4 Verificare connessione
1. Apri **Serial Monitor** del Display (115200 baud)
2. Dovresti vedere:

```
🌐 Connessione WiFi...
 OK!
IP: 192.168.43.101
📡 Connessione a Master 192.168.43.100:8888
✅ CONNESSO AL MASTER!
✅ Sistema pronto!
```

3. Sul **Serial Monitor del Master** vedrai:
```
✅ Display connesso!
   IP client: 192.168.43.101
```

4. Sul **Display LCD** vedrai un **pallino verde** 🟢 in alto a destra

---

## 🎨 Significato Pallino Status

### Display LCD - Angolo alto destra
- 🟢 **Verde:** TCP connesso al Master
- 🟡 **Giallo lampeggiante:** WiFi OK, riconnessione TCP in corso...
- 🔴 **Rosso:** WiFi disconnesso

---

## ✅ Test Funzionamento

### Test 1: Annuncio Meteo
1. Sul Master, apri Serial Monitor
2. Digita: `meteo`
3. Premi Invio
4. **Risultato atteso:**
   - Master: `📤 Inviato a display: METEO|21|0|TRIESTE|12:30`
   - Display: Mostra schermata meteo con temperatura e icona

### Test 2: Annuncio Treno (Bottone)
1. Sul Master, premi il **bottone** 1 volta
2. **Risultato atteso:**
   - Master: `📤 Inviato a display: TRAIN|MF-TRIESTE|4|...`
   - Display: Mostra schermata treno con binario e orario

### Test 3: Random Play
1. Attendi ~60 secondi
2. Il Master dovrebbe annunciare automaticamente un treno o meteo
3. Il Display si aggiorna automaticamente

---

## 🐛 Troubleshooting

### ❌ Display: "ERRORE WiFi!"
**Problema:** Display non riesce a connettersi al WiFi

**Soluzioni:**
1. Verifica che SSID e password siano **identici** a quelli del Master
2. Verifica che la rete WiFi sia **attiva**
3. Avvicina il Display al router/hotspot
4. Riavvia il router/hotspot

---

### ❌ Display: Pallino rosso 🔴
**Problema:** WiFi connesso ma TCP non riesce a collegarsi al Master

**Soluzioni:**
1. **Verifica l'IP del Master:**
   - Apri Serial Monitor del Master
   - Cerca `IP: 192.168.43.xxx`
   - Controlla che corrisponda a `masterIP` nel codice del Display

2. **L'IP del Master è cambiato?**
   - Se usi DHCP, l'IP può cambiare ad ogni riavvio
   - Soluzione: riavvia il Master, annota il nuovo IP, modifica e ricarica il Display

3. **Firewall?**
   - Alcuni router bloccano la porta 8888
   - Prova a disabilitare il firewall temporaneamente

4. **Master spento?**
   - Verifica che il Master sia acceso e il server attivo

---

### ❌ Display: Pallino giallo lampeggiante 🟡
**Problema:** WiFi OK ma TCP continua a disconnettersi

**Soluzioni:**
1. **Segnale WiFi debole:**
   - Avvicina Display e Master al router
   - Usa un router più potente
   - Evita ostacoli metallici

2. **Interferenze:**
   - Cambia canale WiFi del router (preferisci 1, 6, 11)
   - Disattiva Bluetooth sul router se presente

3. **Master sovraccarico:**
   - Apri Serial Monitor del Master
   - Digita: `ram`
   - Se RAM < 15KB, riavvia il Master

---

### ❌ Master: "WiFi TCP Server attivo" ma Display non si connette
**Problema:** Server avviato ma nessun client

**Soluzioni:**
1. **IP errato nel Display:**
   - Controlla riga 34 di `minilcd_wifi.ino`
   - Deve corrispondere all'IP stampato dal Master

2. **Rete WiFi diversa:**
   - Master e Display devono essere sulla **stessa rete**
   - Controlla SSID su entrambi

3. **Porta bloccata:**
   - Alcuni router bloccano la porta 8888
   - Prova a cambiare `DISPLAY_SERVER_PORT` in entrambi i file
   - Esempio: usa porta 12345

---

### 🔄 Master: IP cambia ad ogni riavvio (DHCP)

**Problema:** L'IP del Master è dinamico e cambia ogni volta

**Soluzione A: IP Statico (CONSIGLIATO)**

Modifica `initDisplayServer()` nel Master (dopo riga 159):

```cpp
// Invece di WiFi.begin(wifiSSID, wifiPWD);
IPAddress staticIP(192, 168, 43, 100);  // IP fisso
IPAddress gateway(192, 168, 43, 1);    // Gateway router
IPAddress subnet(255, 255, 255, 0);    // Subnet mask
IPAddress dns(8, 8, 8, 8);             // DNS Google

WiFi.config(staticIP, gateway, subnet, dns);
WiFi.begin(wifiSSID, wifiPWD);
```

Poi nel Display:
```cpp
const char* masterIP = "192.168.43.100";  // IP fisso del Master
```

**Soluzione B: Riservazione DHCP**
- Configura il router per assegnare sempre lo stesso IP al MAC del Master
- Vai nelle impostazioni del router → DHCP → Riserva IP

---

## 📊 Confronto WiFi TCP vs Bluetooth

| Caratteristica | Bluetooth | WiFi TCP |
|----------------|-----------|----------|
| RAM Master | ~20KB liberi | ~40KB liberi |
| Riverloop | ❌ Disabilitato | ✅ Attivo |
| Stabilità | ⚠️ Media | ✅ Alta |
| Range | ~10m | ~50m+ |
| Setup | MAC pairing | IP config |
| Velocità | 1-3 Mbps | 54+ Mbps |

---

## 🎓 Note Tecniche

### Perché WiFi usa meno RAM?
- **Bluetooth Classic:** Stack BT è ~40KB + buffers
- **WiFi TCP Client:** Solo client TCP ~20KB
- **Risparmio:** ~20KB liberi per audio e playlist!

### Formato Comandi TCP
Entrambi i sistemi usano lo stesso protocollo:

**Meteo:**
```
METEO|temperatura|weatherCode|città|orario
Esempio: METEO|21|0|TRIESTE|12:30
```

**Treno:**
```
TRAIN|destinazione|binario|linea1|linea2|orario|tipo
Esempio: TRAIN|MF-TRIESTE|4|MEZZANINELAB|IC 6563|12:45|partenza
```

Separatore: `|` (pipe) per compatibilità con orari `HH:MM`

---

## ✨ Vantaggi WiFi TCP

### 1. RAM Libera
- Riverloop **sempre attivo** in background
- Buffer audio più grandi → qualità migliore
- Nessun reboot per RAM insufficiente

### 2. Stabilità
- Protocollo TCP garantisce consegna pacchetti
- Riconnessione automatica se disconnesso
- Nessun problema di pairing Bluetooth

### 3. Scalabilità
- Possibile connettere **multipli display** allo stesso Master
- Possibile aggiungere controllo web browser
- Possibile OTA (aggiornamenti via WiFi)

### 4. Debugging
- Comandi visibili nel Serial Monitor
- Facile testing con tools (netcat, telnet)
- Log dettagliati su entrambi i dispositivi

---

## 🚀 Prossimi Sviluppi

### In arrivo:
- 🔮 **ESP-NOW** version (solo ~10KB RAM!)
- 🔮 Display multipli simultanei
- 🔮 Web interface di configurazione
- 🔮 OTA updates via WiFi
- 🔮 API REST per controllo esterno

---

## 📞 Supporto

Se hai problemi:
1. Controlla i log Serial Monitor di **entrambi** i dispositivi
2. Verifica la connessione WiFi con `scanwifi` sul Master
3. Controlla RAM con `ram` sul Master
4. Riavvia entrambi i dispositivi

---

**Versione:** 2.0 - WiFi TCP  
**Data:** Gennaio 2026  
**Autore:** M9Lab  
**Hardware:** ESP32 Atom Lite + SPK / Mini LCD  
**Licenza:** MIT  
