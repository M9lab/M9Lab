# depotIno1.5.4 – Librerie e versioni

**Ultima versione funzionante** con le **vecchie librerie**. Non modificare questo codice per workaround su librerie nuove; usare **depotIno1.5.5** per provare ESP32 v3 e Legoino aggiornato.

---

## Riepilogo (obbligatorio per 1.5.4)

| Componente | Versione |
|------------|----------|
| **Board ESP32** | **2.0.17** |
| **Legoino** | **1.0.0** (originale, non patchato) |
| **NimBLE-Arduino** | **1.4.1** (o compatibile con Legoino 1.0.0) |
| **FastLED** | **3.10.3** (o compatibile) |

---

## Perché queste versioni

- **ESP32 2.0.17**: core stabile con cui lo sketch è stato testato.
- **Legoino 1.0.0**: versione originale; nessun fix per ESP32 v3, nessun workaround nel codice.
- **NimBLE-Arduino**: dipendenza di Legoino su ESP32; usare la versione indicata nella documentazione Legoino 1.0.0 (tipicamente 1.4.x).

---

## Configurazione NimBLE

In `NimBLE-Arduino/src/nimconfig.h`:

- `#define CONFIG_BT_NIMBLE_MAX_CONNECTIONS` → valore **almeno 5** (3 treni + 1 switch + 1 remote).

---

## Installazione rapida

1. **Board**: Tools → Board → Boards Manager → **esp32** → installa **2.0.17**.
2. **Legoino**: installa **1.0.0** (Library Manager o ZIP originale).
3. **NimBLE-Arduino**: **1.4.1** (da GitHub o Library Manager).
4. **FastLED**: **3.10.3** (o compatibile).
5. Modifica **nimconfig.h** come sopra.

---

## depotIno1.5.5

Su **depotIno1.5.5** si provano:
- Board ESP32 v3.x
- Legoino 1.1.0 (o patchata per v3)
- Miglioramenti e fix al codice

Mantenere **depotIno1.5.4** invariato come baseline funzionante con 2.0.17 + Legoino 1.0.0.
