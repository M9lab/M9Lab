# 🎵 Conversione Audio per TrenIno

Questa cartella contiene gli script per convertire i file MP3 al formato ottimale.

---

## 🚀 Guida Rapida (3 Passi)

### Passo 1: Installa FFmpeg

**Opzione A - Download Diretto** (5 minuti):
1. Vai su: https://www.gyan.dev/ffmpeg/builds/
2. Scarica: **ffmpeg-release-essentials.zip**
3. Estrai in `C:\ffmpeg`
4. Aggiungi `C:\ffmpeg\bin` al PATH di sistema

**Istruzioni dettagliate**: Vedi `INSTALL_FFMPEG.md`

### Passo 2: Copia File MP3

Copia tutti i tuoi file MP3 in questa cartella:
```
C:\M9lab\Code\arduino\TS_Master_Slaves\trainstation_bt\audio\
```

### Passo 3: Esegui Conversione

**Doppio click su**: `convert_audio.bat`

Oppure da PowerShell:
```powershell
.\convert_audio.ps1
```

---

## 📋 Cosa Fanno gli Script

1. ✅ **Backup**: Copia originali in `backup_originali\`
2. ✅ **Conversione**: 128 kbps CBR Mono 44.1kHz
3. ✅ **Sostituisce**: File convertiti con stesso nome
4. ✅ **Report**: Mostra successi/errori e risparmio spazio

---

## 🎯 Parametri Conversione

```
Bitrate:      128 kbps (buona qualità)
Mode:         CBR (Constant Bit Rate)
Canali:       Mono (perfetto per voce)
Sample Rate:  44.1 kHz (standard CD)
Encoder:      LAME MP3 (alta qualità)
```

**Perché questi parametri?**
- ✅ **CBR** = decoder ESP32 più efficiente
- ✅ **Mono** = file 50% più piccoli
- ✅ **128 kbps** = ottimo compromesso qualità/dimensione
- ✅ **44.1 kHz** = compatibile con tutto

---

## 📂 File in questa Cartella

| File | Descrizione |
|------|-------------|
| `convert_audio.bat` | Script Windows batch (più semplice) |
| `convert_audio.ps1` | Script PowerShell (con report dettagliato) |
| `INSTALL_FFMPEG.md` | Guida installazione FFmpeg |
| `README_AUDIO.md` | Questo file (guida rapida) |

---

## ⚠️ Note Importanti

### Prima della Conversione

- ✅ Fai backup separato dei file originali (non fidare solo dello script)
- ✅ Verifica che i file si riproducano correttamente sul PC
- ✅ Assicurati di avere spazio disco sufficiente

### Dopo la Conversione

- ✅ Testa qualche file convertito sul PC
- ✅ Controlla dimensioni file (dovrebbero essere simili)
- ✅ Copia sulla SD Card formattata FAT32
- ✅ Testa sul dispositivo Atom Lite

---

## 🎤 Alternativa: Audacity (GUI)

Se preferisci interfaccia grafica:

1. **Scarica Audacity**: https://www.audacityteam.org/
2. **Apri file MP3**
3. **Tracks → Mix → Mix Stereo down to Mono**
4. **File → Export → Export Audio**
   - Format: MP3
   - Bit Rate Mode: Constant
   - Quality: 128 kbps
5. **Salva**

Ripeti per ogni file (più lento ma con controllo totale)

---

## 📞 Problemi Comuni

### "FFmpeg non riconosciuto"

**Causa**: PATH non configurato correttamente  
**Soluzione**: Chiudi e riapri PowerShell dopo aver aggiunto al PATH

### "Access denied"

**Causa**: Permessi insufficienti  
**Soluzione**: Esegui PowerShell come Amministratore (tasto destro → "Esegui come amministratore")

### File convertiti gracchiano

**Causa**: File originali già corrotti  
**Soluzione**: Ri-scarica/ri-genera file audio da ReadSpeaker

---

## 💡 Tips

1. **Test Prima**: Converti 2-3 file e testali prima di convertire tutto
2. **Backup**: Conserva sempre backup_originali finché non hai testato tutto
3. **SD Card**: Dopo conversione, ri-formatta SD Card in FAT32 per ottimizzare

---

**Buona conversione! 🎵**
