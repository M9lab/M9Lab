# 🎵 Guida Ottimizzazione Qualità Audio

Questa guida ti aiuta a ottenere la **migliore qualità audio** possibile dal sistema TrenIno senza compromettere le performance.

---

## 📊 Ottimizzazioni Hardware/Software Applicate (v1.8.0)

### ✅ Buffer Audio Ottimizzati

Il codice ora usa buffer ottimizzati per qualità audio:

```cpp
// Setup normale (BT off)
SetBuffers(8, 1024);  // 8 buffer x 1024 bytes = 8KB totali
SetRate(44100);       // 44.1kHz sample rate (CD quality)

// Setup BT attivo
SetBuffers(4, 256);   // 4 buffer x 256 bytes = 1KB (risparmia RAM)
```

**Benefici**:
- ✅ **8KB buffer** = più tolleranza a ritardi SD Card
- ✅ **44.1kHz** = frequenza standard audio di qualità
- ✅ **8 buffer** = meno probabilità di underrun
- ✅ **Auto-adattamento** con BT attivo

---

### ✅ SD Card Velocità Massima

```cpp
SD.begin(-1, SPI, 80000000);  // 80MHz (fallback a 40MHz se non supportato)
```

**Velocità SPI**:
- **80MHz**: Velocità massima possibile
- **40MHz**: Fallback per SD Card più lente
- **Lettura più veloce** = meno lag audio

---

### ✅ Loop Audio Ottimizzato

```cpp
// 2 chiamate consecutive a mp3->loop() invece di 1
if(!mp3->loop()) break;
if(mp3->isRunning()) mp3->loop();  // Doppia alimentazione decoder
yield();
```

**Prima**: 1 chiamata + `delay(1)` = lento  
**Ora**: 2 chiamate + no delay = **2x più veloce**

---

## 🎼 Qualità File MP3 - Raccomandazioni

### ⚡ Per Audio Fluido e Veloce

| Parametro | Valore Consigliato | Perché |
|-----------|-------------------|--------|
| **Bitrate** | **128 kbps CBR** | Buon compromesso qualità/dimensione |
| **Sample Rate** | **44.1 kHz** | Standard CD, compatibile con tutti i decoder |
| **Canali** | **Mono** | 50% più leggero, perfetto per voce |
| **Encoder** | **LAME MP3** | Migliore qualità a parità di bitrate |
| **Mode** | **CBR** (Constant) | NO VBR, più facile da decodificare |

### 🎤 Per Voce (Annunci, Alert)

**Configurazione ottimale ReadSpeaker**:
```
Bitrate:      96-128 kbps
Sample rate:  44.1 kHz
Canali:       Mono
Formato:      MP3 CBR
Normalizzazione: -3dB (evita clipping)
```

**Vantaggi Mono**:
- ✅ File 50% più piccoli
- ✅ Lettura SD 2x più veloce
- ✅ Decodifica meno pesante
- ✅ Voce chiara e comprensibile
- ✅ Perfetto per annunci

### 🎶 Per Musica (Riverloop)

**Configurazione qualità superiore**:
```
Bitrate:      128-160 kbps
Sample rate:  44.1 kHz
Canali:       Stereo
Formato:      MP3 CBR
```

---

## 🛠️ Tool Consigliati per Conversione

### 1. **Audacity** (Gratuito, Open Source)

**Download**: https://www.audacityteam.org/

**Procedura**:
1. Apri file audio
2. **Tracks → Mix → Mix Stereo Down to Mono** (se voce)
3. **Effect → Normalize** → -3.0 dB
4. **File → Export → Export Audio**
5. Formato: **MP3**
6. Quality: **Standard, 128 kbps**
7. Mode: **Constant Bit Rate**
8. Salva

### 2. **FFmpeg** (Command Line)

**Conversione batch veloce**:

```bash
# Mono, 96 kbps, 44.1kHz
ffmpeg -i input.mp3 -ac 1 -b:a 96k -ar 44100 output.mp3

# Stereo, 128 kbps, 44.1kHz
ffmpeg -i input.mp3 -ac 2 -b:a 128k -ar 44100 output.mp3

# Batch: converti tutta la cartella
for file in *.mp3; do
  ffmpeg -i "$file" -ac 1 -b:a 96k -ar 44100 "optimized_$file"
done
```

### 3. **Online Audio Converter**

**Siti consigliati**:
- https://online-audio-converter.com/
- https://cloudconvert.com/mp3-converter

**Impostazioni**:
- Bitrate: 128 kbps
- Channels: Mono (per voce)
- Sample rate: 44100 Hz

---

## 📏 Dimensioni File Indicative

| Durata | Mono 96kbps | Stereo 128kbps | Stereo 192kbps |
|--------|-------------|----------------|----------------|
| **5 sec** | ~60 KB | ~80 KB | ~120 KB |
| **10 sec** | ~120 KB | ~160 KB | ~240 KB |
| **30 sec** | ~360 KB | ~480 KB | ~720 KB |
| **1 min** | ~720 KB | ~960 KB | ~1.4 MB |

**Consiglio SD Card**: Min 4GB per 100+ file audio

---

## ⚙️ Ottimizzazioni Avanzate

### 1. Aumenta Buffer Se Hai RAM Extra

**Modifica** (riga ~1625):
```cpp
out->SetBuffers(12, 1024);  // Da 8x1024 a 12x1024 = 12KB buffer
```

**Pro**: Audio ancora più fluido  
**Contro**: Usa 4KB RAM extra

**Verifica RAM disponibile**:
```bash
ram  # Deve mostrare >100KB liberi
```

### 2. Pre-carica File Importanti

Per file critici usati spesso, potresti implementare pre-caching, ma richiede modifiche significative al codice.

### 3. SD Card Upgrade

**Consigliate**:
- **SanDisk Extreme**: UHS-I, fino a 100 MB/s
- **Samsung EVO Plus**: UHS-I, 100 MB/s
- **Kingston Canvas**: Class 10, affidabile

**Evitare**:
- SD Card generiche "no-name"
- Class 4 o inferiore
- SD Card vecchie/usurate

### 4. Formato SD Card Ottimale

```
Formato: FAT32
Allocation unit size: 32KB (invece di default 4KB)
```

**Su Windows**:
1. Tasto destro sulla SD → Format
2. File system: FAT32
3. Allocation unit size: **32768 bytes**
4. Format

**Beneficio**: Lettura file più veloce (cluster più grandi)

---

## 🎚️ Bilanciamento Qualità vs Performance

### Setup 1: Massima Qualità (RAM >120KB)

```cpp
// Nel setup e reinitAudio
out->SetBuffers(12, 1024);  // 12KB buffer
out->SetRate(44100);

// File MP3
Bitrate: 160 kbps stereo CBR
Sample: 44.1 kHz
```

**Pro**: Qualità audio eccellente  
**Contro**: Usa più RAM, potrebbe dare problemi con BT

---

### Setup 2: Bilanciato (Default, RAM ~100KB) ✅

```cpp
// Nel setup e reinitAudio (ATTUALE)
out->SetBuffers(8, 1024);  // 8KB buffer
out->SetRate(44100);

// File MP3
Bitrate: 128 kbps stereo o mono CBR
Sample: 44.1 kHz
```

**Pro**: Ottimo compromesso  
**Contro**: Nessuno, è la configurazione ideale

---

### Setup 3: Risparmio RAM (RAM <80KB)

```cpp
// Nel setup e reinitAudio
out->SetBuffers(4, 512);  // 2KB buffer
out->SetRate(44100);

// File MP3
Bitrate: 96 kbps mono CBR
Sample: 44.1 kHz
```

**Pro**: Minimo consumo RAM  
**Contro**: Possibili micro-glitch con SD lenta

---

## 🔍 Diagnosi Problemi Audio

### Audio a Scatti

**Possibili cause e soluzioni**:

1. **SD Card lenta**
   - Test: cambia SD Card
   - Soluzione: Upgrade a Class 10+

2. **File VBR** (Variable Bit Rate)
   - Test: apri MP3 in Audacity, controlla proprietà
   - Soluzione: Ri-codifica in CBR

3. **RAM insufficiente**
   - Test: comando `ram` → se <70KB è critico
   - Soluzione: Disabilita BT cellulare, riavvia

4. **Buffer troppo piccoli**
   - Test: aumenta buffer (vedi Setup 1 sopra)
   - Soluzione: `SetBuffers(12, 1024)`

5. **Verbose mode attivo**
   - Test: `verbose=0`
   - Soluzione: Disabilita per performance

### Audio Distorto

1. **Volume troppo alto**
   - Soluzione: `vol=50` o inferiore

2. **File MP3 con clipping**
   - Soluzione: Normalizza a -3dB in Audacity

3. **Bitrate troppo basso**
   - Soluzione: Usa min 96 kbps

### Audio con Pause/Gap

1. **Transizioni tra file**
   - Soluzione: Normalizza tutti i file allo stesso volume
   - Aggiungi 50ms silenzio a inizio/fine file

2. **Riverloop loop non seamless**
   - Soluzione: Crea file con fade in/out che si sovrappongono

---

## 📋 Checklist Qualità Ottimale

Prima di caricare file su SD Card:

- [ ] Tutti i file convertiti a **128 kbps CBR**
- [ ] Sample rate uniforme: **44.1 kHz**
- [ ] Voce annunci: **Mono** (risparmia spazio)
- [ ] Musica riverloop: **Stereo** (qualità)
- [ ] Normalizzati a **-3dB** (no clipping)
- [ ] Testati in Audacity (no errori/glitch)
- [ ] Nomi file corretti (0001.mp3, 0191.mp3, ecc.)
- [ ] SD Card **Class 10** o superiore
- [ ] SD Card formattata **FAT32, 32KB cluster**

---

## 🎯 Risultati Attesi

Con tutte le ottimizzazioni applicate:

| Metrica | Risultato |
|---------|-----------|
| **Lag/Stutter** | Quasi zero |
| **Transizioni file** | Fluide (<50ms gap) |
| **Qualità voce** | Chiara e comprensibile |
| **Qualità musica** | Buona (paragonabile a streaming) |
| **RAM disponibile** | 80-120KB (sufficiente per BT) |
| **Reattività comandi** | Immediata |

---

## 💡 Tips Professionali

### Tip 1: Normalizza Volume

Tutti i file devono avere volume simile per evitare sbalzi:

```bash
# FFmpeg: normalizza tutti i file
ffmpeg -i input.mp3 -filter:a loudnorm output.mp3
```

### Tip 2: Aggiungi Padding Silenzio

Per transizioni più naturali:

```bash
# Audacity: Generate → Silence
- Inizio file: 50ms silenzio
- Fine file: 50ms silenzio
```

### Tip 3: Test A/B

Testa diversi bitrate per trovare il minimo accettabile:
- 96 kbps = leggero ma buono
- 112 kbps = ottimo compromesso
- 128 kbps = qualità standard
- 160 kbps = alta qualità (consuma più)

### Tip 4: Verifica File Corrotti

```bash
# Windows: prova a riprodurre in VLC
# Se VLC mostra errori = file corrotto, ri-codifica
```

### Tip 5: Usa ReadSpeaker con Impostazioni Ottimali

**Voce consigliata**: Giorgio (maschile) o Alice (femminile)  
**Velocità**: Normale (100%)  
**Pitch**: Normale  
**Export**: MP3 128 kbps

---

## 🔧 Modifiche Codice per Qualità Massima

Se hai RAM extra (>120KB disponibili), modifica:

### File: `trainstation_bt.ino`

**Riga ~1625 e ~1260** - Aumenta buffer:
```cpp
out->SetBuffers(12, 1024);  // Da 8x1024 a 12x1024
```

**Riga ~1628** - Aumenta velocità SD:
```cpp
SD.begin(-1, SPI, 80000000);  // Già impostato a 80MHz
```

**Riga ~362** - Rimuovi commento buffer SD (se funziona):
```cpp
file->setBufferSize(4096);  // Togli // se la libreria lo supporta
```

---

## 📈 Monitoraggio Performance

### Comando RAM

Durante riproduzione, monitora RAM:
```bash
ram  # Mostra heap disponibile
```

**Valori normali**:
- **>100KB**: Ottimo, puoi aumentare buffer
- **80-100KB**: Buono, configurazione ideale
- **60-80KB**: Sufficiente ma critico
- **<60KB**: Problema, riduci buffer o disabilita BT

### Verbose Mode

Attiva per debug dettagliato:
```bash
verbose=1
```

Poi riproduci:
```bash
alert1
```

Analizza output per:
- Tempi di lettura SD
- RAM durante riproduzione
- Eventuali errori decoder

---

## 🎯 Risultato Finale Atteso

Con **tutte le ottimizzazioni** applicate (codice + file MP3 ottimizzati):

✅ **Audio playlist**: Fluido, transizioni rapide  
✅ **Riverloop**: Continuo senza scatti  
✅ **Voce annunci**: Chiara e intelligibile  
✅ **RAM disponibile**: 80-100KB (sufficiente)  
✅ **Reattività**: Comandi serial immediati  
✅ **Stabilità**: Zero crash o reset  

---

## 🚨 Se Audio Ancora Problematico

### Test Sistematico

1. **Test SD Card**:
   ```bash
   # Carica file 0191.mp3
   # Usa: alert1
   # Se lento → problema SD
   ```

2. **Test File MP3**:
   ```bash
   # Converti 0191.mp3 a 96 kbps mono
   # Ri-testa
   # Se fluido → i tuoi file sono troppo pesanti
   ```

3. **Test RAM**:
   ```bash
   ram
   # Se <70KB → problema RAM
   # Soluzione: disabilita randomplay, spegni BT
   ```

4. **Test Buffer**:
   ```bash
   # Aumenta buffer a 12x1024 (riga 1625)
   # Se migliora → era problema buffer
   ```

### Ultima Risorsa: Downgrade ESP32 Core

Se **tutto fallisce** e audio rimane problematico:

```
ESP32 Core: 2.0.17 (invece di 3.3.6)
ESP8266Audio: 1.9.7 (invece di 2.4.1)
```

Versioni più vecchie ma **ultra-stabili** e **testate su milioni di dispositivi**.

---

## 📞 Supporto Qualità Audio

Per problemi persistenti:

1. ✅ Verifica buffer: `SetBuffers(8, 1024)` attivo
2. ✅ Controlla SD: Class 10 o superiore
3. ✅ Testa file MP3: 128 kbps CBR
4. ✅ Monitora RAM: `ram` → >80KB
5. ✅ Verbose off: `verbose=0`
6. ✅ Core Debug Level: None

---

**Happy listening! 🎵✨**

*Ultima modifica: 2026-02-13*
