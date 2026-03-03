/*
    Board: M5Stack-ATOM
    Library: 
      esp32 -> 3.3.3 (board)
      M5Atom -> 0.1.3
      Adafruit_GFX -> 1.12.4
      Legoino -> 1.1.0
*/


# DepotIno v1.5.5.1

Sistema di controllo per 3 treni LEGO Powered UP su binario con deposito e tunnel.

## 🚨 Nota Importante: Compatibilità ESP32 v3.x

Questa versione include **fix critici** per la compatibilità con **ESP32 Arduino Core v3.x** (v3.3.4 testata).

### Problemi Risolti

| Errore Originale | Causa | Fix Applicato |
|------------------|-------|---------------|
| `internal compiler error: Segmentation fault` | GCC v12.2 più aggressivo | ✅ Refactoring `readFromSerial()` |
| `'std::string' does not name a type` | v3.x richiede `#include <string>` | ✅ Aggiunto in `LegoinoCommon.h` |
| `no declaration matches 'uint32_t...'` | Type checking più restrittivo | ✅ Allineati tipi in `LegoinoCommon.cpp` |

**Se hai ESP32 v2.x**: Il codice funziona comunque, ma i fix non sono necessari.  
**Se hai ESP32 v3.x**: I fix sono **obbligatori** per la compilazione.

📖 Vedi sezione [Fix Applicati](#fix-applicati-per-compatibilità-esp32-board-package-v3x) per dettagli completi.

---

## Descrizione

Gestisce 3 treni (A, B, C) che partono dal deposito uno alla volta (manualmente o automaticamente).
- Utilizza 3 City Hub per i treni (motore + sensore colore)
- 1 Technic Hub per gli scambi (3 motori) e luci
- Supporto opzionale per telecomando remoto

Autore: Stefx - 2026

---

## Indice

- [Hardware Richiesto](#hardware-richiesto)
- [Librerie Richieste](#librerie-richieste)
- [Fix per ESP32 v3.x](#fix-applicati-per-compatibilità-esp32-board-package-v3x)
  - [Fix #1: `#include <string>`](#fix-1-aggiungere-include-string-in-legoinocommonh)
  - [Fix #2: Type mismatch](#fix-2-correggere-dichiarazioni-di-tipo-in-legoinocommoncpp)
  - [Fix #3: Segmentation fault](#fix-3-refactoring-readfromserial-progetto)
- [Configurazione NimBLE](#configurazione-nimble)
- [Comandi Seriali](#comandi-seriali)
- [Note di Installazione](#note-di-installazione)
- [Dettagli Tecnici ESP32](#dettagli-tecnici-esp32-v2x-vs-v3x)
- [Link Utili](#link-utili)

---

## Hardware Richiesto

- **ESP32** (qualsiasi board compatibile)
- **3x LEGO City Hub** (90:84:2b:...)
- **1x LEGO Technic Hub** (Control+)
- **1x Kit luci LEGO**
- **1x Striscia LED WS2812** (25 LED, pin 27)
- **Telecomando LEGO** (opzionale)

---

## Librerie Richieste

### 1. **Legoino** v1.1.0
- **Autore**: Cornelius Munz
- **Repository**: https://github.com/corneliusmunz/legoino
- **Scopo**: Controllo hub LEGO Powered UP / Boost / Control+

### 2. **NimBLE-Arduino** v1.4.1
- **Autore**: h2zero
- **Repository**: https://github.com/h2zero/NimBLE-Arduino
- **Scopo**: Comunicazione Bluetooth Low Energy
- **Note**: Configurare `CONFIG_BT_NIMBLE_MAX_CONNECTIONS` in `nimconfig.h`

### 3. **FastLED** v3.10.3
- **Autore**: Daniel Garcia
- **Repository**: https://github.com/FastLED/FastLED
- **Scopo**: Controllo striscia LED WS2812 (display 5x5)

---

## Fix Applicati per Compatibilità ESP32 (Board Package v3.x)

### 🔧 Versione ESP32 Utilizzata: **v3.3.4**

Le versioni recenti del compilatore ESP32 richiedono alcuni fix nella libreria **Legoino v1.1.0** (rilasciata nel 2020).

### ⚠️ IMPORTANTE: Causa Root dei Problemi

**TUTTI i bug sono causati da breaking changes introdotti in ESP32 Arduino Core v3.x** (basato su ESP-IDF 5.1):

| Problema | Causa ESP32 v3.x | Documentazione Ufficiale |
|----------|------------------|--------------------------|
| **Segmentation fault GIMPLE** | Compilatore GCC più aggressivo nell'ottimizzazione | [Arduino Forum](https://forum.arduino.cc/t/solved-gcc-segmentation-error-esp32/1129802) |
| **Missing `std::string`** | v3.x ha rimosso l'inclusione automatica di `<string>` | [Issue #5342](https://github.com/espressif/arduino-esp32/issues/5342) |
| **Type mismatch `uint32_t`** | Compilatore più restrittivo sui tipi di dato | [Legoino Issue #79](https://github.com/corneliusmunz/legoino/issues/79) |
| **Valori default duplicati** | Parser più rigoroso sulle dichiarazioni C++ | [Migration Guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html) |

**Riferimenti Ufficiali Espressif:**
- [Migration from 2.x to 3.0](https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html)
- [Announcing Arduino ESP32 Core v3.0.0](https://blog.espressif.com/announcing-the-arduino-esp32-core-version-3-0-0-3bf9f24e20d4)

### ⚠️ IMPORTANTE
Se reinstalli o aggiorni la libreria Legoino dal Library Manager, dovrai **riapplicare questi fix manualmente**.

### 💡 Alternative

**Opzione 1: Restare su ESP32 v3.x con i fix** ✅ (configurazione attuale)
- ✅ Hai le ultime funzionalità e supporto per nuovi chip (C6, H2, P4)
- ⚠️ Devi mantenere i fix se reinstalli le librerie

**Opzione 2: Downgrade a ESP32 v2.0.17**
- ✅ Compatibilità totale con Legoino/NimBLE senza modifiche
- ❌ Perdi supporto per nuovi chip e funzionalità
- 📦 Arduino IDE → Board Manager → ESP32 → Seleziona v2.0.17

**Opzione 3: Attendere aggiornamento Legoino**
- ❌ Progetto non in sviluppo attivo (ultimo commit: ottobre 2022)
- ❌ Update improbabile

---

### Fix #1: Aggiungere `#include <string>` in LegoinoCommon.h

**File**: `C:\Users\stefano.ferrara\Documents\Arduino\libraries\Legoino\src\LegoinoCommon.h`

**Problema**: Il compilatore ESP32 moderno richiede l'inclusione esplicita di `<string>` per usare `std::string`.

**Soluzione**: Aggiungere dopo le altre inclusioni (riga 16):

```cpp
#include "Arduino.h"
#include "Lpf2HubConst.h"
#include <string>  // <-- AGGIUNGERE QUESTA RIGA
```

---

### Fix #2: Correggere dichiarazioni di tipo in LegoinoCommon.cpp

**File**: `C:\Users\stefano.ferrara\Documents\Arduino\libraries\Legoino\src\LegoinoCommon.cpp`

**Problema**: 
1. Mismatch tra tipi dichiarati nell'header (`unsigned int`, `signed int`, ecc.) e implementazione (`uint32_t`, `int32_t`, ecc.)
2. Valori di default `= 0` presenti nell'implementazione (devono essere solo nell'header)

**Soluzione**: Modificare le seguenti funzioni (righe 77-111):

```cpp
// PRIMA (ERRATO):
uint8_t LegoinoCommon::ReadUInt8(uint8_t *data, int offset = 0)

// DOPO (CORRETTO):
unsigned char LegoinoCommon::ReadUInt8(uint8_t *data, int offset)
```

**Tutte le correzioni necessarie**:

| Riga | Prima (ERRATO) | Dopo (CORRETTO) |
|------|----------------|-----------------|
| 77 | `uint8_t LegoinoCommon::ReadUInt8(uint8_t *data, int offset = 0)` | `unsigned char LegoinoCommon::ReadUInt8(uint8_t *data, int offset)` |
| 79 | `uint8_t value = data[...]` | `unsigned char value = data[...]` |
| 83 | `int8_t LegoinoCommon::ReadInt8(uint8_t *data, int offset = 0)` | `signed char LegoinoCommon::ReadInt8(uint8_t *data, int offset)` |
| 85 | `int8_t value = (int8_t)data[...]` | `signed char value = (signed char)data[...]` |
| 89 | `uint16_t LegoinoCommon::ReadUInt16LE(uint8_t *data, int offset = 0)` | `unsigned short LegoinoCommon::ReadUInt16LE(uint8_t *data, int offset)` |
| 91 | `uint16_t value = ... (uint16_t)...` | `unsigned short value = ... (unsigned short)...` |
| 95 | `int16_t LegoinoCommon::ReadInt16LE(uint8_t *data, int offset = 0)` | `signed short LegoinoCommon::ReadInt16LE(uint8_t *data, int offset)` |
| 97 | `int16_t value = ... (int16_t)...` | `signed short value = ... (signed short)...` |
| 101 | `uint32_t LegoinoCommon::ReadUInt32LE(uint8_t *data, int offset = 0)` | `unsigned int LegoinoCommon::ReadUInt32LE(uint8_t *data, int offset)` |
| 103 | `uint32_t value = ... (uint32_t)...` | `unsigned int value = ... (unsigned int)...` |
| 107 | `int32_t LegoinoCommon::ReadInt32LE(uint8_t *data, int offset = 0)` | `signed int LegoinoCommon::ReadInt32LE(uint8_t *data, int offset)` |
| 109 | `int32_t value = ... (int16_t)... (uint32_t)...` | `signed int value = ... (int16_t)... (unsigned int)...` |

---

### Fix #3: Refactoring readFromSerial() (progetto)

**File**: `depotIno1.5.4.ino`

**Problema**: Internal Compiler Error (ICE) - Segmentation fault durante l'ottimizzazione GIMPLE.

```
during GIMPLE pass: copyprop
depotIno1.5.4.ino:165:6: internal compiler error: Segmentation fault
  165 | void readFromSerial() {
      |      ^~~~~~~~~~~~~~
```

**Causa Root (ESP32 v3.x)**: 
- GCC più recente (v12.2.0 in v3.x vs v8.4.0 in v2.x) è più aggressivo nell'ottimizzazione
- La lunga catena di 40+ confronti `if-else` su `String` causa overflow interno del compilatore
- Problema documentato: [Arduino Forum - GCC Segmentation Error](https://forum.arduino.cc/t/solved-gcc-segmentation-error-esp32/1129802)

**Soluzione**: Suddivisa la funzione in 4 helper functions più piccole:
- `handleSystemCommands()` - comandi di sistema
- `handleSwitchCommands()` - comandi scambi
- `handleTrainCommands()` - comandi treni
- `handleLightsCommands()` - comandi luci

Questo riduce la complessità di ogni funzione, evitando il crash del compilatore durante l'ottimizzazione.

---

## Configurazione NimBLE

### Compatibilità con ESP32 v3.x

**NimBLE-Arduino v1.4.1** funziona con ESP32 v3.x ma richiede attenzione:
- ⚠️ La libreria è stata progettata per ESP-IDF 4.x (usato in ESP32 v2.x)
- ⚠️ ESP32 v3.x usa ESP-IDF 5.1 con breaking changes
- ✅ Nella maggior parte dei casi funziona, ma ci sono issue note: [NimBLE Issue #641](https://github.com/h2zero/NimBLE-Arduino/issues/641)

**Se hai problemi con NimBLE:**
- Considera il downgrade a ESP32 v2.0.17
- Oppure usa le librerie BLE native di Arduino (meno efficienti)

### Configurazione Richiesta

Modificare il file `nimconfig.h` per aumentare il numero massimo di connessioni:

**File**: `C:\Users\stefano.ferrara\Documents\Arduino\libraries\NimBLE-Arduino\src\nimconfig.h`

```cpp
#define CONFIG_BT_NIMBLE_MAX_CONNECTIONS 6  // Default è 3, aumentare a 6
```

**Motivo**: Il progetto gestisce fino a 6 dispositivi BLE simultaneamente:
- 3 City Hub (treni)
- 1 Technic Hub (scambi)
- 1 Remote Hub (opzionale)
- 1 margine per riconnessioni

---

## Comandi Seriali

### Sistema
- `help` - Mostra tutti i comandi
- `on` / `off` - Attiva/disattiva modalità automatica
- `panic` - Arresto di emergenza
- `reset` - Reset sistema
- `status` - Mostra stato sistema
- `verboseon` / `verboseoff` - Abilita/disabilita messaggi dettagliati

### Treni
- `str1` / `stg1` / `sty1` - Avvia treno Red/Green/Yellow
- `str0` / `stg0` / `sty0` - Ferma treno Red/Green/Yellow
- `killr` / `killg` / `killy` - Kill treno Red/Green/Yellow
- `killall` - Kill tutti i treni
- `cts+` / `cts-` / `cts=` - Aumenta/diminuisci/reset velocità

### Scambi
- `swa0` / `swa1` - Scambio A dritto/deviato
- `swb0` / `swb1` - Scambio B dritto/deviato
- `swc0` / `swc1` - Scambio C (deposito batterie) dritto/deviato
- `sws+` / `sws-` / `sws=` - Aumenta/diminuisci/reset velocità motori scambi
- `resetsw` - Reset tutti gli scambi

### Telecomando
- `sron` / `sroff` - Abilita/disabilita controllo remoto

---

## Note di Installazione

### Installazione da Zero

1. **Installare Arduino IDE** (versione 2.x consigliata)

2. **Installare ESP32 Board Support**
   - Arduino IDE → Board Manager → "esp32" di Espressif Systems
   - Scegliere: v3.3.4 (con fix) oppure v2.0.17 (senza fix necessari)

3. **Installare le librerie** tramite Library Manager:
   - Legoino v1.1.0
   - NimBLE-Arduino v1.4.1
   - FastLED v3.10.3

4. **Se usi ESP32 v3.x**: Applicare i fix ai file della libreria Legoino:
   - ✅ Fix #1: `LegoinoCommon.h`
   - ✅ Fix #2: `LegoinoCommon.cpp`

5. **Configurare NimBLE** (`nimconfig.h`): `CONFIG_BT_NIMBLE_MAX_CONNECTIONS = 6`

6. **Aprire e compilare** `depotIno1.5.4.ino`

7. **Caricare** lo sketch sull'ESP32 (115200 baud)

### Checklist Pre-Compilazione

- [ ] ESP32 board package installato (v3.3.4 o v2.0.17)
- [ ] Legoino v1.1.0 installato
- [ ] NimBLE-Arduino v1.4.1 installato  
- [ ] FastLED v3.10.3 installato
- [ ] **Se ESP32 v3.x**: Fix applicati a `LegoinoCommon.h` e `LegoinoCommon.cpp`
- [ ] NimBLE configurato: `CONFIG_BT_NIMBLE_MAX_CONNECTIONS = 6`
- [ ] Board selezionata correttamente (es. "ESP32 Dev Module")
- [ ] Porta COM corretta selezionata

### Troubleshooting

**Errore di compilazione `std::string`?**
→ Applica Fix #1 (`#include <string>` in `LegoinoCommon.h`)

**Errore `no declaration matches 'uint32_t'`?**
→ Applica Fix #2 (correggi tipi in `LegoinoCommon.cpp`)

**Segmentation fault durante compilazione?**
→ Il progetto include già Fix #3 (refactoring `readFromSerial()`)

**NimBLE non connette più di 3 dispositivi?**
→ Aumenta `CONFIG_BT_NIMBLE_MAX_CONNECTIONS` in `nimconfig.h`

**Ancora problemi?**
→ Considera downgrade a ESP32 v2.0.17 (compatibilità totale senza fix)

---

## Backup e Manutenzione

### Backup dei Fix

Per comodità, i file modificati sono stati salvati:
- Backup libreria: `C:\Users\stefano.ferrara\Documents\Arduino\libraries\legoino_withfixInit.zip`

Se devi reinstallare la libreria, copia i file dal backup o riapplica i fix manualmente seguendo le istruzioni sopra.

### ⚠️ Attenzione agli Aggiornamenti

**Aggiornamento Legoino via Library Manager**:
- ❌ Sovrascriverà i fix applicati
- ✅ Riapplica manualmente Fix #1 e Fix #2
- 📋 Usa questo README come guida

**Aggiornamento ESP32 Board Package**:
- ✅ v3.x → v3.y: Fix rimangono validi
- ⚠️ v3.x → v4.x: Potrebbero servire nuovi fix (non ancora rilasciato)
- ✅ v3.x → v2.x (downgrade): Fix non più necessari

**Aggiornamento NimBLE-Arduino**:
- ✅ Versioni future potrebbero migliorare compatibilità ESP32 v3.x
- 📋 Verifica changelog prima di aggiornare

### Strategia Consigliata

**Per ambienti di produzione:**
1. Usa ESP32 v2.0.17 (stabile, senza fix necessari)
2. Non aggiornare librerie senza testare
3. Mantieni backup delle librerie funzionanti

**Per sviluppo/sperimentazione:**
1. Usa ESP32 v3.3.4+ (nuove funzionalità)
2. Mantieni questo README aggiornato con i fix
3. Testa ogni aggiornamento in ambiente isolato

---

## Storico Versioni

### v1.5.4.1 (Febbraio 2026)
**Compatibilità ESP32 v3.x**

**Modifiche al progetto:**
- ✅ Refactoring `readFromSerial()` → 4 funzioni helper (fix segmentation fault)
- ✅ Aggiornamento numero versione in sketch
- ✅ Documentazione completa con fix e troubleshooting

**Fix libreria Legoino richiesti:**
- ✅ Aggiunto `#include <string>` in `LegoinoCommon.h`
- ✅ Corretti tipi (`uint32_t` → `unsigned int`) in `LegoinoCommon.cpp`
- ✅ Rimossi valori default duplicati in `LegoinoCommon.cpp`

**Testing:**
- ✅ Testato con ESP32 Arduino Core v3.3.4
- ✅ Testato con ESP32 Arduino Core v2.0.17 (backward compatible)
- ✅ Compilazione successful su Windows 10
- ✅ Runtime test: 3 treni + 1 technic hub + remote

### v1.5.4 (2022)
**Versione originale**
- ✅ Funzionante con ESP32 v1.x / v2.x
- ✅ Legoino v1.1.0 + NimBLE v1.4.1 + FastLED
- ❌ Non compatibile con ESP32 v3.x senza fix

---

## Dettagli Tecnici: ESP32 v2.x vs v3.x

### Perché ESP32 v3.x ha causato questi problemi?

ESP32 Arduino Core v3.0 (rilasciato nel 2024) è basato su ESP-IDF 5.1, una **major release** con numerosi breaking changes rispetto a v2.x (basato su ESP-IDF 4.4).

#### Compilatore GCC
- **v2.x**: GCC 8.4.0 (2020)
- **v3.x**: GCC 12.2.0 (2022+)
- **Impatto**: Ottimizzazioni più aggressive, maggiore aderenza agli standard C++17/20

#### Breaking Changes Rilevanti
1. **BLE API**: Cambio da `std::string` a Arduino `String`
2. **Strict Type Checking**: Richiede match esatto tra dichiarazione e implementazione
3. **Header Includes**: Non più inclusioni automatiche implicite
4. **Compilation Flags**: Cambiate le flag di default `-MMD -c`

#### Timeline dei Problemi
- **2020**: Legoino v1.1.0 rilasciata per ESP32 v1.x/2.x
- **2021**: NimBLE-Arduino v1.4.1 rilasciata per ESP-IDF 4.x
- **2024**: ESP32 v3.0 rilasciata → breaking changes
- **2026**: ESP32 v3.3.4 attuale → problemi evidenziati

### Versioni Testate

| Componente | Versione Testata | Note |
|------------|------------------|------|
| ESP32 Arduino Core | v3.3.4 | ✅ Funziona con i fix applicati |
| ESP32 Arduino Core | v2.0.17 | ✅ Funziona senza modifiche |
| Legoino | v1.1.0 | ⚠️ Richiede fix per v3.x |
| NimBLE-Arduino | v1.4.1 | ⚠️ Funziona ma non ufficialmente supportato per v3.x |
| FastLED | v3.10.3 | ✅ Compatibile |

---

## Link Utili

### Librerie
- Legoino GitHub: https://github.com/corneliusmunz/legoino
- NimBLE-Arduino: https://github.com/h2zero/NimBLE-Arduino
- FastLED: https://github.com/FastLED/FastLED

### Documentazione ESP32
- ESP32 Migration Guide 2.x → 3.0: https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html
- ESP32 v3.0 Announcement: https://blog.espressif.com/announcing-the-arduino-esp32-core-version-3-0-0-3bf9f24e20d4
- ESP32 Release Notes: https://github.com/espressif/arduino-esp32/releases

### Issue Tracker
- Legoino uint32_t Issue: https://github.com/corneliusmunz/legoino/issues/79
- ESP32 std::string Issue: https://github.com/espressif/arduino-esp32/issues/5342
- NimBLE ESP32 v3 Compatibility: https://github.com/h2zero/NimBLE-Arduino/issues/641

---

*Ultima modifica: 10 Febbraio 2026*
*Versione firmware testata: v1.5.4.1*
*ESP32 Arduino Core: v3.3.4*
