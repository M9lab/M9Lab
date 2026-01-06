# 🥧 Installazione Raspberry Pi 3

## 📦 Prerequisiti

### 1. Abilita la fotocamera
```bash
sudo raspi-config
# Vai a: Interface Options → Camera → Enable
sudo reboot
```

### 2. Installa dipendenze sistema
```bash
sudo apt-get update
sudo apt-get install -y python3-pip python3-opencv python3-pil python3-tk

# IMPORTANTE: Installa font emoji per visualizzare le icone
sudo apt-get install -y fonts-noto-color-emoji

# Riavvia per caricare i font
sudo reboot
```

### 3. Installa dipendenze Python
```bash
cd /home/legofoto/lego-photo-booth
pip3 install -r requirements.txt
```

## 🎨 Configurazione

Il file `.env.raspberry` è già ottimizzato per schermo 1360×768:

- Preview LIVE: 1031×580 (16:9)
- Preview RESULT: 640×480 (4:3)
- QR Code: 200×200 (ridotto per schermo piccolo)
- Font ridotti per Raspberry
- Emoji con font "Noto Color Emoji"

## 🚀 Avvio

### Manuale da terminale:
```bash
cd /home/legofoto/lego-photo-booth
python3 fotolego.py
```

### Desktop Entry
Crea file: `~/Desktop/lego-photo-booth.desktop`

```ini
[Desktop Entry]
Name=LEGO Photo Booth
Comment=Avvia il kiosk LEGO Photo Booth in fullscreen
Path=/home/legofoto/lego-photo-booth
Exec=python3 /home/legofoto/lego-photo-booth/fotolego.py
Icon=/home/legofoto/lego-photo-booth/icon.png
Terminal=false
Type=Application
StartupNotify=true
```

Rendi eseguibile:
```bash
chmod +x ~/Desktop/lego-photo-booth.desktop
```

### Avvio automatico all'accensione
```bash
mkdir -p ~/.config/autostart
cp ~/Desktop/lego-photo-booth.desktop ~/.config/autostart/
```

## ✅ Test

Dopo l'installazione dei font emoji, riavvia e lancia l'app. Dovresti vedere:

- ✅ Emoji nelle schermate
- 📱 Icone telefono
- 👍 Like Facebook
- 📸 Fotocamera
- 🚀 Razzo

Se non vedi gli emoji, verifica:
```bash
# Controlla font installati
fc-list | grep -i emoji

# Dovresti vedere: Noto Color Emoji
```

## 🔧 Troubleshooting

### Webcam non funziona
```bash
# Verifica device
ls -l /dev/video*

# Testa webcam
v4l2-ctl --list-devices

# Aggiungi permessi
sudo usermod -a -G video $USER
# Poi logout/login
```

### Emoji non visibili
```bash
# Reinstalla font
sudo apt-get install --reinstall fonts-noto-color-emoji
sudo fc-cache -fv
sudo reboot
```

### Fullscreen non funziona
Il codice usa `overrideredirect(True)` per rimuovere la barra del titolo.
Se hai problemi, verifica che non ci siano window manager che interferiscono.

## 📊 Dimensioni ottimizzate

| Elemento | Raspberry | Windows |
|----------|-----------|---------|
| Preview LIVE | 1031×580 | 1280×720 |
| Preview RESULT | 640×480 | 800×600 |
| QR Code | 200×200 | 350×350 |
| Font Status | 16px | 24px |
| Font Bottoni | 18px | 26px |
| Padding | 5-10px | 20px |

## 🎯 Performance

Raspberry Pi 3 può essere lento con OpenCV. Per migliorare:

1. **Overclock moderato** (opzionale):
   ```bash
   sudo raspi-config
   # Performance Options → Overclock
   ```

2. **Disabilita servizi non necessari**:
   ```bash
   sudo systemctl disable bluetooth
   sudo systemctl disable avahi-daemon
   ```

3. **Usa scheda SD veloce** (Class 10 o UHS-I)

Buon lavoro! 🚀

