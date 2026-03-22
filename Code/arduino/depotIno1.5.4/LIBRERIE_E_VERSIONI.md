# depotIno1.5.4 — Libraries & versions

Sketch architecture, workflow, and Serial commands: **[README.md](README.md)**.

**Known-good baseline** with the **older** library stack. Do not patch this sketch for newer-library workarounds; use **depotIno1.5.5** to try ESP32 v3 and updated Legoino.

---

## Summary (required for 1.5.4)

| Component | Version |
|-----------|---------|
| **ESP32 board** | **2.0.17** |
| **Legoino** | **1.0.0** (original, unpatched) |
| **NimBLE-Arduino** | **1.4.1** (or compatible with Legoino 1.0.0) |
| **FastLED** | **3.10.3** (or compatible) |

---

## Why these versions

- **ESP32 2.0.17:** stable core used for testing.
- **Legoino 1.0.0:** original; no ESP32 v3 fixes, no workarounds in this sketch.
- **NimBLE-Arduino:** Legoino dependency on ESP32; use version matching Legoino 1.0.0 docs (typically 1.4.x).

---

## NimBLE configuration

In `NimBLE-Arduino/src/nimconfig.h`: set `#define CONFIG_BT_NIMBLE_MAX_CONNECTIONS` to **at least 5** (3 trains + 1 switch + 1 remote).

---

## Quick install

1. **Board:** Boards Manager → **esp32** → install **2.0.17**.
2. **Legoino:** **1.0.0** (Library Manager or original ZIP).
3. **NimBLE-Arduino:** **1.4.1**.
4. **FastLED:** **3.10.3** (or compatible).
5. Edit **nimconfig.h** as above.

---

## depotIno1.5.5

On **depotIno1.5.5** you can try ESP32 v3.x, Legoino 1.1.0 (or patched for v3), and newer fixes. Keep **depotIno1.5.4** as the known-good baseline with 2.0.17 + Legoino 1.0.0.
