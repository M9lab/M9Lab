# depotIno1.5.4 — README

ESP32 sketch for a LEGO train layout: **three trains** (each City/Powered Up hub: motor on **A**, **color & distance sensor** on **A or B**) and one **Technic (Control+)** hub for **turnouts** and lights. BLE via **Legoino** (`Lpf2Hub`).

Board/library versions and `nimconfig.h`: see [`LIBRERIE_E_VERSIONI.md`](LIBRERIE_E_VERSIONI.md).

---

## What the sketch does

- Connects to the **switch controller** (main hub driving turnout motors).
- Connects up to **three** known train hubs by MAC.
- User **presses the button** on each train hub to put it in service (`Hub … is ready`).
- Serial **`on`** enables **automatic** operation: pseudo-random pick among stopped trains, countdown, start, track color reads for **stop / invert / kill** (only when auto is on).
- **LED matrix** (FastLED, data pin **GPIO 27**) for visual feedback (train color, roulette, etc.).

---

## Project files (Arduino tabs)

| File | Role |
|------|------|
| `depotIno1.5.4.ino` | Globals, `Train` / `Switches`, MACs, **Serial** parser, **FreeRTOS** `mainTask` on **core 0** (same as BLE). |
| `switchhub.ino` | Switch hub connection, delayed post-setup, battery callback. |
| `switches.ino` | Turnout motors (Technic ports). |
| `trainshub.ino` | `scanHub`, train hub button/battery, **color sensor** subscribe, `colorDistanceSensorCallback`. |
| `trains.ino` | Start/stop/kill, manual + `randomStartTrain`, speed, `stopAndDoTrain`. |
| `system.ino` | `printLegenda`, on/off/reset/panic, `systemStatus`, color helpers. |
| `led.ino` | LED matrix, countdown, roulette. |
| `lights.ino` | Lights on switch hub port **A**. |
| `remote.ino` | Optional Powered Up remote. |

---

## User workflow

1. Power on: boot, purple LEDs, Serial legend.
2. **Switch** connects → `Connected to Switch Controller`.
3. Trains connect over BLE → `Now connected with hub … -> MAC`.
4. On each train hub: **first button** → `Hub … is ready`, hub LED **cyan**, color sensor subscribed; **second button** → hub shutdown.
5. Send **`on`**: if switch OK, `isAutoEnabled = true`. After `AUTO_START_MIN_DELAY_MS` (15 s from boot), `randomStartTrain()` may run when all `trainState == 0`.

Recommended order is also in the header comment of `depotIno1.5.4.ino` (switch → trains → ready → `on`).

---

## Train hub state machine (`hubState`)

In `hubButtonCallback` (`trainshub.ino`): **`-1`** = out of service; **`0`** = connected, waiting for user (LED **purple**); **`1`** = **active**, color logic on. `activeTrain` counts hubs in state `1`.

---

## Motor and color sensor

Motor is always **`portA`**. Sensor port is **not** fixed: use `getPortForDeviceType(COLOR_DISTANCE_SENSOR)` and `activatePortDevice(thatPort, …)`. Hardcoding **`portB`** used to cause missing or cross-talk readings; the code now uses the reported port, with **retries** if still `255` (BLE device list not ready on first press).

---

## Track colors and actions

Defined in `depotIno1.5.4.ino`:

```cpp
// order: stop | invert | kill
sensorAcceptedColors[] = { YELLOW, GREEN, RED };
```

`COLOR_CONFIRM_COUNT` (default 2): require **N** consecutive equal samples before acting. In the color callback, `stopTrain` / invert / kill run only if **`isAutoEnabled`**; `Color Hub …` logs are still useful for debug when auto is off.

**Serial log:** `Color Hub Green: yellow` means train **Green** read **yellow** under the sensor (brick/track), not a wrong hub label.

---

## Main loop (`mainTask`)

On **core 0**: Serial → `readFromSerial()`; else scan switch, post-connect, optional remote, per train `scanHub` or `checkIntervalisExpired` (resume after `stopAndDoTrain` timeout), `pendingKillTrain`, `randomStartTrain`, `vTaskDelay(1)`. Arduino `loop()` is intentionally minimal (`delay(10)`); BLE logic runs in the task.

---

## Serial commands

Full list: send **`help`** (`printLegenda` in `system.ino`). Quick reference:

| Command | Effect |
|---------|--------|
| `on` / `off` | Auto layout on/off |
| `panic` | Shutdown + hard reset |
| `status` | Status dump |
| `str1` / `stg1` / `sty1` | Manual start Red (0) / Green (1) / Yellow (2) |
| `swa0` … `swc1` | Turnouts |

Index ↔ name mapping is in `myTrains[]` in `depotIno1.5.4.ino`.

---

## Configuration

Adapt MACs (`switchControllerAddress`, `myTrains[].hubAddress`, optional `remote`), `switchPosition[3]` per train, speeds in `myTrains`, intervals (`colorInterval`, `beforeStartInterval`, `KILL_COOLDOWN_MS`, …).

---

## Notes for future changes

Do not move BLE to another core without checking Legoino/NimBLE (assert risk). After ESP32/Legoino upgrades, compare **depotIno1.5.5** and `LIBRERIE_E_VERSIONI.md`. New train peripherals: always **`getPortForDeviceType` + `activatePortDevice(correctPort, …)`**, not a fixed port unless hardware requires it.

---

*Sketch author: Stefx / M9Lab.*
