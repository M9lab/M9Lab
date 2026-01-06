# 📐 Configurazione Multi-Piattaforma Fotomachine

## 🖥️ File di configurazione

L'applicazione carica automaticamente il file `.env` corretto in base al sistema operativo:

### Windows / Mac
- File: `.env`
- Risoluzione ottimale: 1920×1080 o superiore
- Preview LIVE: 1280×720 (16:9)
- Preview RESULT: 800×600 (4:3)

### Raspberry Pi 3
- File: `.env.raspberry`
- Risoluzione schermo: 1360×768
- Preview LIVE: 960×540 (16:9)
- Preview RESULT: 640×480 (4:3)

## 🔧 Setup Raspberry Pi

1. **Copia il file di configurazione:**
   ```bash
   # Il file .env.raspberry è già presente e configurato
   # Se necessario, modificalo con:
   nano .env.raspberry
   ```

2. **Abilita la fotocamera:**
   ```bash
   sudo raspi-config
   # Vai a: Interface Options → Camera → Enable
   sudo reboot
   ```

3. **Installa le dipendenze:**
   ```bash
   sudo apt-get update
   sudo apt-get install python3-pip python3-opencv python3-pil python3-tk
   pip3 install -r requirements.txt
   ```

4. **Avvia l'applicazione:**
   ```bash
   python3 fotolego.py
   ```

## 📊 Dimensioni configurabili

### `.env` (Windows/Mac)
```ini
PREVIEW_LIVE_W=1280
PREVIEW_LIVE_H=720
PREVIEW_RESULT_W=800
PREVIEW_RESULT_H=600

FONT_SIZE_COUNTDOWN=200
FONT_SIZE_STATUS=24
FONT_SIZE_BUTTON_MAIN=26
FONT_SIZE_BUTTON_SECONDARY=20
FONT_SIZE_EMAIL_ENTRY=24
FONT_SIZE_EVENT_TEXT=60

PADDING_FRAME=20
PADDING_BUTTON=20
```

### `.env.raspberry` (Raspberry Pi)
```ini
PREVIEW_LIVE_W=960
PREVIEW_LIVE_H=540
PREVIEW_RESULT_W=640
PREVIEW_RESULT_H=480

FONT_SIZE_COUNTDOWN=120
FONT_SIZE_STATUS=18
FONT_SIZE_BUTTON_MAIN=20
FONT_SIZE_BUTTON_SECONDARY=16
FONT_SIZE_EMAIL_ENTRY=18
FONT_SIZE_EVENT_TEXT=48

PADDING_FRAME=10
PADDING_BUTTON=15
```

## 🎯 Personalizzazione

Puoi modificare questi valori per adattare l'interfaccia al tuo schermo:

- **PREVIEW_LIVE_W/H**: Dimensioni anteprima webcam live (16:9)
- **PREVIEW_RESULT_W/H**: Dimensioni anteprima foto composta (4:3)
- **FONT_SIZE_***: Dimensioni dei font per vari elementi UI
- **PADDING_***: Spaziature tra elementi

## ✅ Compatibilità garantita

Il codice rileva automaticamente:
- Sistema operativo (Windows/Linux/Mac)
- Backend webcam corretto (CAP_DSHOW/CAP_V4L2)
- File .env appropriato

Nessuna modifica al codice necessaria! 🚀

