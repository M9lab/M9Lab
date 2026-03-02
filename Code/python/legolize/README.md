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
