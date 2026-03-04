# 📦 Installazione FFmpeg su Windows

Guida rapida per installare FFmpeg e convertire i file audio.

---

## 🚀 Metodo 1: Download Diretto (Più Veloce)

### Passo 1: Scarica FFmpeg

1. Vai su: **https://www.gyan.dev/ffmpeg/builds/**

2. Scarica: **ffmpeg-release-essentials.zip**
   - Link diretto: https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-essentials.zip
   - Dimensione: ~80 MB

### Passo 2: Estrai

1. Estrai lo ZIP scaricato
2. Rinomina la cartella estratta in `ffmpeg`
3. Sposta la cartella in `C:\ffmpeg`

**Struttura finale**:
```
C:\ffmpeg\
  ├── bin\
  │   ├── ffmpeg.exe  ← Questo è importante
  │   ├── ffplay.exe
  │   └── ffprobe.exe
  ├── doc\
  └── presets\
```

### Passo 3: Aggiungi al PATH

1. Premi **Win + R**
2. Digita: `sysdm.cpl` e premi Enter
3. Vai su tab **"Avanzate"**
4. Clicca **"Variabili d'ambiente"**
5. Nella sezione **"Variabili di sistema"**, trova **Path**
6. Clicca **"Modifica"**
7. Clicca **"Nuovo"**
8. Aggiungi: `C:\ffmpeg\bin`
9. Clicca **OK** su tutte le finestre

### Passo 4: Verifica Installazione

1. **Chiudi e riapri** PowerShell/CMD
2. Digita:
   ```cmd
   ffmpeg -version
   ```
3. Dovresti vedere info su FFmpeg

---

## 🚀 Metodo 2: WinGet (Windows 11)

Se hai Windows 11:

```powershell
winget install Gyan.FFmpeg
```

Poi chiudi e riapri PowerShell.

---

## 🚀 Metodo 3: Chocolatey (Se hai Admin)

Se hai Chocolatey installato:

```powershell
# Apri PowerShell come Amministratore
choco install ffmpeg -y
```

---

## ✅ Dopo l'Installazione

### 1. Verifica FFmpeg

Apri PowerShell e digita:
```powershell
ffmpeg -version
```

Dovresti vedere qualcosa tipo:
```
ffmpeg version 2024.xx.xx...
```

### 2. Naviga alla Cartella Audio

```powershell
cd "C:\M9lab\Code\arduino\TS_Master_Slaves\trainstation_bt\audio"
```

### 3. Esegui Script Conversione

```powershell
.\convert_audio.bat
```

Lo script:
- ✅ Trova tutti i file MP3
- ✅ Crea backup originali
- ✅ Converte a 128 kbps CBR Mono
- ✅ Normalizza qualità
- ✅ Mantiene nomi file

---

## 🎯 Cosa Fa la Conversione

**Prima** (file misti):
```
0001.mp3 → 60 kbps VBR  ❌
0002.mp3 → 192 kbps Stereo  ❌
0003.mp3 → 128 kbps VBR  ❌
```

**Dopo** (uniformi):
```
0001.mp3 → 128 kbps CBR Mono  ✅
0002.mp3 → 128 kbps CBR Mono  ✅
0003.mp3 → 128 kbps CBR Mono  ✅
```

**Benefici**:
- ✅ Tutti i file **stessa qualità**
- ✅ **CBR** = decoder più felice
- ✅ **Mono** = 50% più leggeri
- ✅ **128 kbps** = buona qualità
- ✅ **44.1 kHz** = standard
- ✅ **Audio più fluido** su ESP32

---

## 🔧 Conversione Manuale (Singolo File)

Se preferisci convertire manualmente:

```bash
ffmpeg -i input.mp3 -ac 1 -b:a 128k -ar 44100 output.mp3
```

**Parametri**:
- `-ac 1` = Mono (1 canale)
- `-b:a 128k` = Bitrate 128 kbps
- `-ar 44100` = Sample rate 44.1 kHz

---

## 📝 Note Importanti

1. **Backup Automatico**
   - Lo script crea cartella `backup_originali`
   - Conserva i file originali per sicurezza

2. **Nomi File Preservati**
   - I nomi file rimangono identici
   - Compatibili con codice Arduino (0001.mp3, ecc.)

3. **Tempo Richiesto**
   - ~1-2 secondi per file
   - Per 100 file: ~3-5 minuti totali

4. **Spazio Disco**
   - File Mono = ~50% dimensione originale
   - Libera spazio su SD Card

---

## 🆘 Problemi?

### FFmpeg non funziona dopo installazione

**Soluzione**: Riavvia completamente il computer (non solo PowerShell)

### "Access Denied" durante conversione

**Soluzione**: Esegui PowerShell come Amministratore

### File corrotti dopo conversione

**Soluzione**: I backup sono in `backup_originali\`, puoi recuperarli

---

## 🔗 Link Utili

- **FFmpeg Download**: https://www.gyan.dev/ffmpeg/builds/
- **FFmpeg Docs**: https://ffmpeg.org/documentation.html
- **Audacity** (alternativa GUI): https://www.audacityteam.org/

---

**Ultima modifica**: 2026-02-13
