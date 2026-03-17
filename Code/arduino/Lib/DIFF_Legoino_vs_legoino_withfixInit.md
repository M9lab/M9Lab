# Differenze tra `Legoino.zip` e `legoino_withfixInit.zip`

Confronto effettuato dopo estrazione dei due archivi in `Lib/Legoino_extracted` e `Lib/legoino_withfixInit_extracted`.

---

## Riepilogo

| File | Differenze |
|------|------------|
| **Lpf2Hub.cpp** | Solo formattazione (spazio nel costruttore). **Nessuna differenza nella logica di `init()` o fixInit.** |
| **LegoinoCommon.h** | `Legoino.zip` aggiunge `#include <string>` (fix per ESP32 v3). |
| **LegoinoCommon.cpp** | `Legoino.zip` usa tipi “estesi” e rimuove i default nei .cpp (fix type mismatch / default parameter per ESP32 v3). |

**Conclusione:** La versione **`Legoino.zip`** contiene i fix per ESP32 v3 (compilatore recente). La versione **`legoino_withfixInit.zip`** è il codice “originale” senza questi fix; il nome “withfixInit” non corrisponde a una differenza nella logica di `init()` (che è identica in entrambi).

---

## 1. Lpf2Hub.cpp

**Unica differenza:** formattazione del costruttore.

| Legoino.zip | legoino_withfixInit.zip |
|-------------|--------------------------|
| `Lpf2Hub::Lpf2Hub() {};` | `Lpf2Hub::Lpf2Hub(){};` |

- `init()`, `_isInitializing`, `scanEndedCallback()`, `isConnecting()` sono **identici** in entrambi i file.
- Non c’è quindi un “fix init” aggiuntivo in `legoino_withfixInit.zip`; la logica BLE/init è la stessa.

---

## 2. LegoinoCommon.h

| Legoino.zip | legoino_withfixInit.zip |
|-------------|--------------------------|
| Dopo `#include "Lpf2HubConst.h"` c’è **`#include <string>`** | Manca **`#include <string>`** |

- Con compilatore ESP32 v3, l’uso di `std::string` nella libreria richiede `#include <string>`.
- **`Legoino.zip`** = versione con questo fix (evita errori tipo `'std::string' does not name a type`).

---

## 3. LegoinoCommon.cpp

Differenze nelle firme e nei tipi di ritorno delle funzioni `Read*` (definizioni nel .cpp).

### Parametro `offset`

- **Legoino.zip:** parametro **senza** valore default nella definizione, es. `(uint8_t *data, int offset)`.
- **legoino_withfixInit.zip:** parametro **con** `= 0` nella definizione, es. `(uint8_t *data, int offset = 0)` (può dare “duplicate default argument” se anche in .h c’è il default).

### Tipi di ritorno / variabili

| Funzione | Legoino.zip (con fix) | legoino_withfixInit.zip (originale) |
|----------|------------------------|-------------------------------------|
| `ReadUInt8`  | `unsigned char`  | `uint8_t`  |
| `ReadInt8`   | `signed char`    | `int8_t`   |
| `ReadUInt16LE` | `unsigned short` | `uint16_t` |
| `ReadInt16LE`  | `signed short`  | `int16_t`  |
| `ReadUInt32LE` | `unsigned int`  | `uint32_t` |
| `ReadInt32LE`  | `signed int`    | `int32_t`  |

- Con ESP32 v3 il compilatore può dare errori di tipo (es. “no declaration matches”) se le dichiarazioni in .h e le definizioni in .cpp non sono allineate; la versione in **Legoino.zip** usa i tipi “estesi” che risolvono questi problemi.
- **Legoino.zip** = versione con fix type mismatch e default parameter per ESP32 v3.

---

## Quale usare

- **ESP32 core 2.0.x (es. 2.0.17)** e compilatore “vecchio”: puoi usare entrambe; per coerenza con il resto del repo spesso si usa **Legoino.zip** (già con fix string + tipi).
- **ESP32 core 3.x**: usare **Legoino.zip** (contiene `#include <string>` e le correzioni in `LegoinoCommon.cpp`).
- Il nome **legoino_withfixInit.zip** non indica una versione con un “fix init” aggiuntivo rispetto a **Legoino.zip**: la logica di `init()` è la stessa in entrambi.

---

*File generato dal confronto tra i sorgenti estratti da `Lib/Legoino.zip` e `Lib/legoino_withfixInit.zip`.*
