# ULP-BLE Node: Part 4 of 6: Debugging, Performance and Reliability (Sections 19-21)

**Document ID:** ULPBLE-DOC-04 | **Revision:** 0.1 (draft) | **Date:** 2026-10-20
Legend tags are defined in Part 1. No measured results are claimed.

---

## 19. Debugging Guide and Common Faults

### 19.1 Toolbox

| Tool | Use |
|---|---|
| Nordic **PPK2** | Current profiling (source or ampere mode) |
| **nRF Connect** (mobile/desktop) | GATT exploration, adv inspection, connection parameter view |
| **nRF Sniffer for Bluetooth LE** | Over-the-air packet capture and timing |
| SEGGER J-Link + RTT / Ozone | Debug, trace logs (development builds only) |
| GDB + `arm-none-eabi` | Fault analysis from stacked registers |
| Logic analyzer | I²C, wake-pin timing |
| Oscilloscope | VDD sag during TX, wake latency |
| `nrfjprog` | Flash, erase, reset, read registers (e.g., RESETREAS) |

### 19.2 Debug build vs production build

| Item | Debug | Production |
|---|---|---|
| `NRF_LOG` / RTT / UART | On | **Off** (compile out) |
| LED indication | On | Off |
| WDT | Optional pause on halt | On |
| Assertions | Verbose | Minimal fault code |

**Always profile power with the production configuration**, with the debugger disconnected. A connected debugger can raise current significantly and can emulate rather than truly enter System OFF.

### 19.3 Common faults and fixes

| # | Symptom | Likely cause | Check / Fix |
|---|---|---|---|
| 1 | System OFF current far above target | Floating or pulled pins leaking; peripheral left enabled; debugger attached; external pull-up fighting a pin | Park pins as input-disconnected; confirm TWIM/SAADC/UARTE disabled; measure without debugger; inspect schematic for pull-up/pull-down contention |
| 2 | Device wakes immediately after System OFF (wake loop) | A SENSE pin already at its sense level (button held, INT latched not cleared) | Clear sensor interrupt latch before entry; check pin level; defer entry until released |
| 3 | Never wakes from button | SENSE not configured, wrong polarity, wrong pull, pin is not wake-capable in your configuration, or pin routing error | Verify `nrf_gpio_cfg_sense_input` call and polarity; scope the pin |
| 4 | Wake works on DK but not on custom PCB | Different pin, ESD network, or sensor INT polarity | Compare schematics; scope |
| 5 | State lost after System OFF | Expected: RAM not retained | Use GPREGRET, retained RAM section with correct power-set configuration, or flash |
| 6 | Retention CRC fails sporadically | Record straddles non-retained section; partial write on brown-out | Place record in a retained section; write-then-validate; keep record small |
| 7 | Radio does not start / `NRF_ERROR_` at SD enable | LFCLK source not available (no crystal fitted while config says crystal) | Match `NRF_SDH_CLOCK_LF_SRC` to hardware; RC calibration config if no crystal |
| 8 | Reset when enabling DC/DC | DC/DC enabled without inductor on board | Disable DC/DC in firmware or fit the required network |
| 9 | Random resets under TX | VDD sag from high-ESR coin cell | Add low-ESR bulk cap; reduce TX power; scope VDD during TX; check brown-out reset reason |
| 10 | Connection drops at range or in noisy RF | Supervision timeout too short relative to interval × (1+latency) | Re-check constraint (Part 2, Section 12.1); increase timeout |
| 11 | Sleep current higher with CI = short | Too many connection events; latency 0 | Raise connection interval / latency (central may override) |
| 12 | Phone ignores connection parameter request | Central policy limits | Log applied parameters; design for central's accepted range |
| 13 | I²C NACK / stuck bus | Sensor in reset, wrong address, SDA held low by sensor after aborted transfer | Bus-recovery (9 SCL pulses), re-init sensor, check pull-ups |
| 14 | Temperature reads high | Self-heating, proximity to regulator/battery, conversion mode left continuous | Use one-shot mode; relocate sensor; apply offset only after calibration |
| 15 | Battery voltage reads low during TX | Measuring during radio burst | Sample between events; average; discard post-TX samples |
| 16 | WDT resets in low-power idle | Health bit not set while sleeping (e.g., nothing posts `H_MAIN`) | Feed from a periodic `app_timer` that checks bits; ensure idle still produces periodic events |
| 17 | Hang when debugging | System OFF emulation / WDT running during halt | Configure WDT to pause on halt in debug; note emulated System OFF behaviour |
| 18 | HardFault after wake | Using peripheral/driver state assuming RAM survived | Re-initialize everything in `S_BOOT`; never assume RAM retention |
| 19 | Notifications missing | CCCD not enabled; queue/TX buffers full; MTU assumptions | Enable CCCD; check return codes (`NRF_ERROR_RESOURCES`); keep frame ≤ 20 B |
| 20 | Flash write failures or slow | FDS GC pending; write during radio activity | Handle FDS events; schedule writes in idle windows |

### 19.4 Systematic power-bug bisection
1. Build minimal firmware: no BLE, immediate System OFF → measure floor (board + sensors only).
2. Add BLE advertising only → compare to Online Power Profiler estimate.
3. Add sensors and wake logic incrementally; measure after each step.
4. Any step with an unexplained jump is the culprit; check peripheral enable state and GPIO configuration first.

### 19.5 Reading a fault record
After reboot, read the fault record (PC, LR, xPSR). Resolve PC and LR with `arm-none-eabi-addr2line -e build/app.elf <addr>`.

---

## 20. Performance Analysis

All values are **targets or analysis approaches**; measured values must be entered after testing.

### 20.1 Metrics

| Metric | Definition | Target **[TARGET]** | Measured |
|---|---|---|---|
| I_sysoff | Board-level current in System OFF, wake armed | ≤ 2 µA | **[VERIFY]** |
| I_adv_slow | Average, 1000 ms advertising, 0 dBm | ≤ 15 µA | **[VERIFY]** |
| I_conn_idle | Average, CI 500 ms, latency 4 | ≤ 15 µA | **[VERIFY]** |
| Q_telem | Charge per sample + notify event | Record | **[VERIFY]** |
| T_wake_boot | Wake edge → first advertising packet | < 1 s | **[VERIFY]** |
| T_discover | Time for phone to see device (fast adv) | < 3 s typical | **[VERIFY]** |
| Flash use | Application size | ≤ 100 kB | **[VERIFY]** |
| RAM use | Static + SoftDevice | ≤ 25 kB (52833-safe) | **[VERIFY]** |
| Battery estimate | See Part 3, Section 16 | Derived from measured I | **[VERIFY]** |

### 20.2 Wake-latency decomposition (qualitative)
Wake latency = reset/boot + clock startup (LFXO and HFXO start times **[DATASHEET-CHECK]**) + SoftDevice enable + first advertising event. The largest contributors are typically the crystal startup and SoftDevice initialization; measure each segment with GPIO toggles captured on a logic analyzer.

### 20.3 Trade-off analysis

| Choice | Energy | Responsiveness | Robustness |
|---|---|---|---|
| Shorter advertising interval | ↑ | ↑ | = |
| Longer connection interval + latency | ↓ | ↓ | ↓ (needs longer timeout) |
| Early System OFF | ↓↓ | ↓ (wake = reboot) | = |
| Keep RAM retained | ↑ (per block) | ↑ (no restore) | ↑ |
| Lower TX power | ↓ | = | ↓ (range) |

### 20.4 Profiling procedure
Capture three states with identical firmware commit: (a) adv fast, (b) adv slow, (c) connected idle, then (d) System OFF; add (e) one telemetry event with annotated GPIO markers. Report mean, min and max current and charge per event, with test conditions.

---

## 21. FMEA / Reliability Analysis

Severity (S), Occurrence (O), Detection (D) are rated 1-10 and are **[ASSUMED] engineering estimates for review**, not field data. RPN = S × O × D.

| # | Function | Failure mode | Effect | Cause | S | O | D | RPN | Mitigation / Detection |
|---|---|---|---|---|---|---|---|---|---|
| 1 | System OFF entry | Entry fails; stays in System ON | Battery drains quickly | Pending event/pin asserted, error return | 7 | 4 | 4 | 112 | Check return codes; retry policy; test TC-16/19; telemetry flag |
| 2 | Wake source | Device never wakes | Device appears dead | Mis-configured SENSE, hardware fault | 8 | 3 | 5 | 120 | Production test of each wake path; redundant button wake |
| 3 | Wake source | Spurious wake loop | Battery depletion | Latched/floating INT line | 7 | 4 | 4 | 112 | Clear latch; defer entry; soak test TC-26 |
| 4 | Retention | Corrupt state restored | Wrong counters/config | Brown-out during write | 5 | 3 | 3 | 45 | CRC-16 + magic + version; defaults on failure |
| 5 | Retention | Excess retained RAM | Higher OFF current | Retaining entire RAM | 4 | 3 | 3 | 36 | Retain single section only; measure |
| 6 | Power supply | Brown-out reset under TX | Connection drops/resets | High-ESR cell, low cap | 7 | 5 | 4 | 140 | Bulk cap, TX power policy, reset-reason log |
| 7 | Battery measurement | False low-battery | Premature shutdown | Sampling during TX burst | 5 | 5 | 4 | 100 | Sample between events; average; hysteresis |
| 8 | Sensor I²C | Bus stuck | No telemetry | Interrupted transfer | 5 | 4 | 3 | 60 | Bus recovery, sensor-fault flag |
| 9 | Sensor | Wrong reading | Bad data | Self-heating/placement | 4 | 4 | 5 | 80 | Layout, one-shot mode, calibration |
| 10 | Firmware | Hang | No response | Logic deadlock | 8 | 3 | 3 | 72 | WDT with health bits |
| 11 | BLE link | Unexpected disconnects | Data gaps | Timeout/interval mismatch, RF noise | 4 | 5 | 3 | 60 | Parameter constraint check, auto re-advertise |
| 12 | Control channel | Unauthorized command | Device forced to OFF/reset | Open characteristic | 6 | 4 | 5 | 120 | Encrypted/bonded link requirement for Control writes |
| 13 | Flash | Wear-out | Config loss | Excess writes | 5 | 2 | 4 | 40 | Write only on change; FDS wear levelling |
| 14 | Hardware | DC/DC enabled without inductor | Device non-functional | BOM/firmware mismatch | 8 | 2 | 3 | 48 | Board variant macro; bring-up checklist |
| 15 | Enclosure/PCB | Leakage current from contamination | Higher OFF current | Flux residue/humidity | 4 | 4 | 6 | 96 | Cleaning process, coating, OFF-current test in production |

**Top risks by RPN:** #6 brown-out under TX (140), #2 no wake (120), #12 unauthorized control (120), #1/#3 (112). Re-rate after test results.

### 21.1 Reliability considerations
- Environmental: coin-cell capacity and internal resistance vary strongly with temperature; test at the intended range.
- Lifetime: dominated by self-discharge and leakage once average current is in the low µA; model both.
- Software lifecycle: version the retained record and protocol; keep backward-compatible parsing.
- Production test: mandatory OFF-current check and wake-path check per unit.

*End of Part 4.*
