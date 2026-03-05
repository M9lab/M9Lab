# Legolize

Stesso flusso e grafica di **fotomachine**: cattura foto → elaborazione → visualizzazione → invio email.

Invece del fotomontaggio con green screen, la foto viene inviata a **Replicate** per essere trasformata in stile LEGO (diorama).

- **Webcam e foto**: width 1024, aspect ratio 3:2 (nessun fotomontaggio, solo foto elaborata).
- **Flusso**: Scatta → elaborazione LEGO su Replicate → mostra risultato → inserisci email → invia.

## Configurazione

1. Copia `.env.example` in `.env` (o usa il `.env` nella cartella `python` con le stesse variabili).
2. Imposta in `.env`:
   - `replicateToken`: token API Replicate
   - `modelID`: ID del modello Replicate (es. `owner/model:version` o solo version hash)
3. Opzionale: SMTP per l’invio email (stesse variabili di fotomachine).

## Installazione

```bash
cd legolize
pip install -r requirements.txt
```

## Avvio

```bash
python legolize.py
```

Lo script cerca `.env` in `legolize/` o, se non esiste, in `python/`.

## Raspberry Pi (schermo 1360×768)

Su Linux, se esiste **`.env.raspberry`** nella cartella legolize viene caricato al posto di `.env` (come in fotomachine).

1. Copia `.env.raspberry` da esempio (o rinomina/copia da `.env`) e inserisci token Replicate e SMTP.
2. Il file è già ottimizzato per 1360×768: preview ridotte, font e padding più piccoli, QR code 200px.
3. **Raspberry 3**: in `.env.raspberry` è impostato `LEGOLIZE_LOW_RESOURCE=1`, che riduce uso di CPU/RAM:
   - Preview a ~7 FPS invece di 20 (`PREVIEW_INTERVAL_MS=150`)
   - Interpolazione OpenCV più veloce (INTER_NEAREST)
   - Font countdown caricato una sola volta
   - Sfondo a tiled logo disattivabile (`SKIP_LOGO_PATTERN=1`), meno tile se attivo
   - Buffer webcam = 1, rotellina caricamento meno frequente
4. Avvio: `python3 legolize.py`
5. **Errore "no display name"**: se lo script parte da systemd o SSH senza display, viene impostato `DISPLAY=:0` automaticamente. Se il Pi non ha monitor collegato, avvia con: `xvfb-run python3 legolize.py` (installa prima `sudo apt install xvfb`).
