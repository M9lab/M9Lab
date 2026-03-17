# Storico ultime modifiche pushate – depotIno1.5.4

Per capire cosa è stato pushato e potrebbe influire sul comportamento su Ubuntu (es. riavvii).

---

## Commit che hanno toccato `depotIno1.5.4`

| Commit     | Data       | Messaggio |
|-----------|------------|-----------|
| **3d17c89** | 10 Feb 2026 | fix per board esp32 v3 in librerie legoino |
| 75dbf42   | 23 Ott 2025 | Riorganizzazione cartelle e file (creazione cartella e file .ino) |

L’ultimo push che modifica **depotIno1.5.4** è **3d17c89**. Per questa cartella ci sono solo questi 2 commit.

---

## Altri 3 commit precedenti (depotIno, con date)

Commit che non toccano `depotIno1.5.4` ma sono nello stesso ambito o temporali vicini (utili per contesto e cronologia).

| Commit     | Data        | Messaggio |
|-----------|-------------|-----------|
| **29ac3b4** | 18 Dic 2025 | Add .gitignore and remove backups and sensitive files from repository *(rimossi dal repo 1.5.1, 1.5.2, 1.5.3; 1.5.4 non toccato)* |
| **5ffdce3** | 10 Feb 2026 | Depotino 1.5.5 - testare ottimizzazioni *(stesso giorno di 3d17c89; tocca solo 1.5.5)* |
| **fbbcb0d** | 13 Feb 2026 | depotIno - fix specs *(modifiche a README e .ino di depotIno1.5.5)* |

Per vedere le date in formato completo:
```bash
git log --format="%h %ad %s" --date=short -10 -- Code/arduino/depotIno1.5.4/ Code/arduino/depotIno1.5.5/
```

---

## Cosa cambia nel commit 3d17c89 (fix esp32 v3)

### 1. File modificati/aggiunti
- **Code/arduino/depotIno1.5.4/depotIno1.5.4.ino** – refactor comandi Serial
- **Code/arduino/depotIno1.5.4/README.md** – aggiunto (443 righe, documentazione + fix Legoino per ESP32 v3)
- **Code/arduino/Lib/Legoino_esp32_3fix.zip** – aggiunto (libreria Legoino con fix per ESP32 v3)

### 2. Modifiche in `depotIno1.5.4.ino`

Solo la gestione dei **comandi da Serial**: nessun cambiamento a loop principale, BLE, treni o switch.

- **Nuove funzioni helper** (prima di `readFromSerial()`):
  - `handleSystemCommands(cmd)` – panic, killsw, reset, on, off, help, status, autospeed, remote, verbose
  - `handleSwitchCommands(cmd)` – swa0/1, swb0/1, swc0/1, sws+/-, sws=, resetsw
  - `handleTrainCommands(cmd)` – str1/0, stg1/0, sty1/0, killr/g/y, cts+/-, killall
  - `handleLightsCommands(cmd)` – sbl1, sbl0

- **`readFromSerial()`**:
  - Aggiunto `command.trim()` sulla stringa letta da Serial.
  - La lunga sequenza di `if / else if` è sostituita da:
    - chiamate alle quattro funzioni sopra;
    - se nessuna gestisce il comando → `">command not found"`.

Nessuna modifica a:
- `setup()`, `loop()`, init BLE, Legoino, switch, treni, luci, logica di controllo.

---

## Possibili cause dei riavvii su Ubuntu

Le modifiche pushate sono solo refactor dei comandi Serial, quindi da sole non spiegano un riavvio continuo. Può dipendere da:

1. **Board / core diverse su Ubuntu**
   - Su Ubuntu: versione core ESP32 (2.0.x vs 3.x) e scheda selezionata (Dev Module, PICO-D4, ecc.).
   - Il README aggiunto in 3d17c89 indica **Legoino_esp32_3fix.zip** e fix in `LegoinoCommon.h` / `LegoinoCommon.cpp` per ESP32 v3. Su Ubuntu potresti avere:
     - libreria Legoino “standard” invece di `Legoino_esp32_3fix`, oppure
     - core 3.x senza i fix applicati → comportamenti strani o crash in init BLE.

2. **Libreria Legoino**
   - Su PC Ubuntu va usata la stessa variante usata in 3d17c89: **Legoino_esp32_3fix.zip** (e/o i fix descritti nel README di depotIno1.5.4).

3. **Watchdog / stack / init**
   - Riavvii subito dopo l’avvio possono venire da:
     - init BLE/Legoino diverso con core 3.x o libreria diversa,
     - stack overflow,
     - watchdog che scatta se il loop blocca o impiega troppo.

**Cosa verificare su Ubuntu**
- Board e core ESP32 (Tools → Board, versione “esp32”).
- Libreria: installata da **Lib/Legoino_esp32_3fix.zip** (e fix README se usi core 3.x).
- Serial Monitor all’avvio: messaggi prima del riavvio (panic, eccezione, “Guru Meditation”, ecc.).

---

## Comando per vedere le differenze sul repo

```bash
# Ultimi commit che toccano depotIno1.5.4
git log --oneline -10 -- Code/arduino/depotIno1.5.4/*

# Diff completo dell’ultimo commit su depotIno1.5.4
git show 3d17c89 -- Code/arduino/depotIno1.5.4/
```

---

*Generato per confronto con la copia su Ubuntu e debug riavvii.*
