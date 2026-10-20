# ULP-BLE Node: Part 3 of 6: Power/Memory, PCB and Test Plan (Sections 16-18)

**Document ID:** ULPBLE-DOC-03 | **Revision:** 0.1 (draft) | **Date:** 2026-10-20
Legend tags are defined in Part 1. **All numbers here are targets, assumptions or datasheet-check values, not measurements.**

---

## 16. Power and Memory Analysis

### 16.1 Power model

Average current of a duty-cycled BLE device:

```
I_avg = I_sleep + (Q_adv / T_adv) + (Q_conn / T_ci) + (Q_sample / T_telem) + (Q_wake_boot × N_wake / T_day)
```

where `Q_x` is the charge (µC) per event, `T` the event period (s). Battery life:

```
Life_h = (C_nominal_mAh × derating) / I_avg_mA
```

### 16.2 Illustrative budget (ALL INPUTS **[ASSUMED]**, replace with PPK2 measurements **[VERIFY]**)

| Item | Assumed value | Basis |
|---|---|---|
| System ON sleep (RTC running, retained RAM, sensors idle) | 3 µA | Placeholder; datasheet gives chip-only figure **[DATASHEET-CHECK]** |
| Advertising event charge (0 dBm, 3 channels, DC/DC on) | 12 µC | Placeholder; use Nordic Online Power Profiler for BLE to estimate, then measure |
| Connection event charge (empty event) | 8 µC | Placeholder |
| Telemetry sample + notify charge (TMP117 one-shot + SAADC + notify) | 60 µC | Placeholder |
| Board-level System OFF (accel wake-on-motion armed) | 2 µA | Target; chip ≈ sub-µA **[DATASHEET-CHECK]** plus sensor |
| Usable battery capacity (CR2477) | 600 mAh | Derated from nominal ~1000 mAh **[ASSUMED]**; verify per supplier curve at your pulse load |

Worked examples (arithmetic only; inputs are assumptions):

| Scenario | Calculation | I_avg | Notes |
|---|---|---|---|
| Slow advertising, 1000 ms | 3 + 12/1.0 | **15 µA** | Dominated by advertising |
| Fast advertising, 100 ms | 3 + 12/0.1 | **123 µA** | Why fast adv is time-limited |
| Connected, CI = 500 ms, latency 4 (event every ≈2.5 s worst-case idle) | 3 + 8/2.5 | **≈ 6.2 µA** | Latency lets the peripheral skip events; real value depends on central |
| Connected, telemetry every 10 s | 6.2 + 60/10 | **≈ 12.2 µA** | |
| System OFF (armed) | n/a | **2 µA** | Wake-up costs boot + advertising energy |

Duty-cycle example (assumed): 2 h/day connected-or-advertising at ≈15 µA, 22 h/day System OFF at 2 µA:
`2×15 + 22×2 = 74 µAh/day` → `600 mAh / 0.074 mAh/day ≈ 8100 days`. This is **not credible as a shelf life**: battery self-discharge, sensor leakage, and temperature effects dominate beyond a few years. The takeaway is the *ratio* between modes, not the absolute number. **[VERIFY]** via long-duration measurement.

### 16.3 Where current is saved (design levers)

| Lever | Saves | Cost |
|---|---|---|
| Advertising interval 100 ms → 1000 ms | ~10× adv average | Slower discovery |
| Slave latency 0 → 4 | Fewer connection events | Latency to host-initiated data |
| System OFF instead of idle advertising | µA-scale vs. tens of µA | Wake = reset, needs retention and external wake sources |
| DC/DC enabled | Lower radio-active current | Needs inductor |
| TX power 0 dBm → lower | Lower TX charge | Range |
| TWIM/SAADC off between samples | Avoid µA-to-mA leaks | Re-init cost per sample |
| UART/RTT logging off | Avoids HFCLK request / mA-level draw | No logs in field |
| Unused pins disconnected | Removes leakage paths | Discipline in HAL |

### 16.4 Measurement method (PPK2)
1. Remove debugger power/back-feed; power the DUT from PPK2 source mode (3.0 V) **or** ampere-meter mode with the actual cell **[VERIFY]**.
2. Capture ≥ 60 s for advertising/connected averages; ≥ 10 min for System ON idle; ≥ 1 h for System OFF with a µA-capable setup (PPK2 low range resolution limits apply **[DATASHEET-CHECK]**).
3. Record: firmware commit hash, battery voltage, temperature, adv/connection parameters, TX power, build flags (logging off).
4. Export CSV; fill the results table in the Final Report (Part 5) with **measured** values only.

### 16.5 Memory analysis

**Flash map (nRF52840, indicative layout; confirm against SoftDevice release notes and linker map [VERIFY])**

| Region | Size (indicative) | Notes |
|---|---|---|
| MBR + SoftDevice S140 | ≈ 150-160 kB **[DATASHEET-CHECK]** | Start address of the application follows the SoftDevice size |
| Application | **≤ 100 kB [TARGET]** | Read actual size from `.map` |
| FDS pages | 3 × 4 kB = 12 kB **[ASSUMED]** | Config + counters |
| Bootloader (optional) | ~24-40 kB | If DFU is added |
| Free | Remainder | nRF52840 has 1 MB; nRF52833 has 512 kB: both fit |

**RAM map (nRF52840 256 kB; nRF52833 128 kB)**

| Region | Estimate **[ASSUMED]** | Notes |
|---|---|---|
| SoftDevice RAM | ≈ 6-10 kB for 1 connection (depends on config) | Report by `sdk_config` / `nrf_sdh_ble_app_ram_start_get()` log |
| Application static | ≤ 8 kB | |
| Stacks: main + ISR | 2 kB | `__STACK_SIZE` **[ASSUMED]** |
| Heap | 0 (static allocation) | |
| Retained `.noinit` record | ≤ 64 B | CRC-protected |
| FreeRTOS variant (optional) | + ≈ 4-6 kB | Task stacks and kernel objects |

**Retention considerations**
- System OFF default: RAM **not** retained (lowest current). State is preserved in GPREGRET (2 × 8 bit), in an optional retained RAM section, or in flash.
- Retaining RAM sections adds current per retained block **[DATASHEET-CHECK]**. Retain only the section holding the `.noinit` record (place it with a linker section attribute), not the whole RAM.
- Flash writes are energy- and time-expensive and wear-limited (endurance specified per datasheet **[DATASHEET-CHECK]**). Write to flash only on configuration change or before the *first* System OFF after counters change materially, not on every entry.

**Stack/heap verification [VERIFY]:** use stack painting (0xCDCDCDCD) and read high-water mark after soak test.

---

## 17. PCB Design Guidelines

### 17.1 Stack-up and layout
- 2-layer is workable with a module; **4-layer** preferred for custom nRF52840 chip-down designs. Keep a continuous ground reference under RF and sensor traces.
- Follow the **module vendor's antenna keep-out** exactly: no copper, traces, components or battery holder in the keep-out; place the antenna at a board edge or corner; do not place metal enclosures/batteries near the antenna **[VERIFY with range test]**.
- Chip-down variant: follow Nordic reference layout for crystal, DC/DC (L + C placement tight to DCC/DEC pins) and RF matching.

### 17.2 Power integrity
- 100 nF + 1 µF at every supply pin of the module; 10 µF within a few mm; **47 µF-class low-ESR bulk** at the cell terminals to handle radio bursts on coin-cell internal resistance **[ASSUMED, verify by observing VDD sag on scope during TX]**.
- Short, wide supply traces; star ground at the module.
- Place the current-measurement link (TP1) in the VBAT trunk upstream of all consumers, **without** adding resistance that distorts radio-burst sag.

### 17.3 Low-power-specific layout rules
- **Leakage control:** clean flux residue (ionic contamination causes µA-level leakage on high-impedance nets); use conformal coating where appropriate; avoid routing high-impedance nets near the supply.
- **No floating inputs:** every GPIO net either driven, pulled, or configured as disconnected in firmware.
- **Pull-up sizing:** I²C pull-ups from the always-on rail leak only while the bus is low; keep both lines high when idle. Choose values per bus capacitance and rise time.
- **Sensor placement:** keep the temperature sensor thermally isolated from the radio/regulator and the battery to avoid self-heating bias **[VERIFY]**.
- **Crystal (if fitted):** short traces, guard ring, no signals underneath; load capacitors `C = 2 × (C_L − C_stray)` where `C_L` is from the crystal datasheet and `C_stray` ≈ 2-5 pF **[ASSUMED]**.
- **Wake lines:** route wake inputs away from RF; add series resistor (e.g., 100 Ω-1 kΩ **[ASSUMED]**) and ESD protection on user-exposed pins; consider RC debounce for the button only if firmware debounce is not used.

### 17.4 Debug and test access
Tag-Connect SWD, test points for VDD, GND, SDA, SCL, ACC_INT1, BTN, TP1 current-measurement header; UART pads (unpopulated in production builds).

### 17.5 DFM checklist
Pin-1 markers, polarity of battery holder, silkscreen for programming connector, panelization with tooling holes, solder paste stencil for module castellations, test coverage of wake pins.

---

## 18. Test Plan and Test Cases

### 18.1 Test strategy
- **Levels:** unit (host-based tests for `protocol`, CRC, `retention`, FSM), integration (SoftDevice + sensors on DK), system (custom board, BLE central such as nRF Connect), and long-duration soak.
- **Tools:** nRF52840-DK, PPK2, logic analyzer, nRF Sniffer for BLE, nRF Connect (mobile/desktop), J-Link/RTT (development builds only), climatic chamber (optional).
- **Result column:** every entry is **PENDING** until you run it. Fill in measured values in the Final Report. Pass thresholds are **[TARGET]**.

### 18.2 Test cases (40)

| ID | Category | Test | Procedure (summary) | Expected / Pass criteria | Result |
|---|---|---|---|---|---|
| TC-01 | Boot | Cold boot | Insert cell / apply power | Device boots, starts fast advertising within [TARGET: 1 s] | PENDING |
| TC-02 | Boot | Reset reason logging | Power-cycle, WDT reset, soft reset | Correct `wake_reason` and counters each time | PENDING |
| TC-03 | Boot | Default config on blank flash | Erase flash, boot | Defaults loaded; no fault | PENDING |
| TC-04 | Advertising | Fast adv interval | Capture with sniffer for 30 s | Interval within spec of configured 100 ms (+ allowed random delay 0-10 ms) | PENDING |
| TC-05 | Advertising | Fast→slow transition | Observe after 30 s | Interval switches to 1000 ms | PENDING |
| TC-06 | Advertising | Slow→System OFF timeout | Wait 120 s unconnected | Enters System OFF; current drops to OFF-level [TARGET ≤ 2 µA] | PENDING |
| TC-07 | Advertising | Adv payload content | Inspect with nRF Connect | Name/UUID/flags correct | PENDING |
| TC-08 | Connection | Connect/disconnect | Connect from phone, disconnect | State transitions `ADV→CONN_IDLE→ADV` | PENDING |
| TC-09 | Connection | Parameter negotiation | Check applied params | Conn interval/latency/timeout within preferred ranges | PENDING |
| TC-10 | Connection | Notify telemetry | Enable CCCD | 20-byte frames at configured period, CRC valid | PENDING |
| TC-11 | Connection | Supervision timeout | Move central out of range | Link drops; device returns to advertising | PENDING |
| TC-12 | System ON | Idle current, advertising 1 s | PPK2 60 s capture | Average ≤ [TARGET 15 µA]; record actual | PENDING |
| TC-13 | System ON | Connected idle current | PPK2 capture, CI 500 ms, lat 4 | Average ≤ [TARGET 15 µA]; record actual | PENDING |
| TC-14 | System ON | Telemetry event current | Capture one sample+notify | Charge per event recorded; peripherals disabled after | PENDING |
| TC-15 | System ON | No busy-wait | Review capture/trace | CPU in sleep between events | PENDING |
| TC-16 | System OFF | Entry via opcode | Write `ENTER_SYSOFF` | Link closes cleanly; device enters System OFF | PENDING |
| TC-17 | System OFF | OFF current, no debugger | PPK2, DUT without debugger | ≤ [TARGET 2 µA] board level; record actual | PENDING |
| TC-18 | System OFF | OFF current with debugger attached | Repeat with J-Link | Expect higher/emulated; document difference | PENDING |
| TC-19 | System OFF | Entry refusal if wake pin asserted | Hold button during entry | Entry deferred; no wake loop | PENDING |
| TC-20 | System OFF | RAM-not-retained behaviour | Write sentinel, System OFF, wake | Sentinel absent unless retained; defined behaviour | PENDING |
| TC-21 | Wake | Button wake | Press button in System OFF | Wakes; reason = button; advertising starts | PENDING |
| TC-22 | Wake | Motion wake | Shake board | Wakes; reason = motion | PENDING |
| TC-23 | Wake | External RTC wake (if fitted) | Program alarm | Wakes at alarm; reason = RTC | PENDING |
| TC-24 | Wake | Wake mask honoured | Disable motion in mask | Motion does not wake | PENDING |
| TC-25 | Wake | Wake latency | Scope: wake edge → first adv packet | Measured; compare with [TARGET < 1 s] | PENDING |
| TC-26 | Wake | Spurious wake soak | 24 h on bench, no stimulus | 0 wakes [TARGET] | PENDING |
| TC-27 | Retention | GPREGRET persistence | Set code, System OFF, wake | Value preserved | PENDING |
| TC-28 | Retention | Counter persistence | Cycle System OFF 100× | `sysoff_cnt` increments exactly | PENDING |
| TC-29 | Retention | Corrupt retention record | Corrupt CRC deliberately | Detected; defaults loaded; flag `F_RET_CRC` | PENDING |
| TC-30 | Sensors | Temperature accuracy | Compare with reference thermometer | Within sensor spec band [DATASHEET-CHECK] | PENDING |
| TC-31 | Sensors | Sensor absent | Disconnect TMP117 | Sensor-fault flag; no lock-up; last-good not faked | PENDING |
| TC-32 | Sensors | I²C bus-stuck recovery | Hold SDA low briefly | Recovery clocks; bus restored | PENDING |
| TC-33 | Battery | VDD measurement accuracy | Compare with DMM at 3.0/2.7/2.4 V | Error within [TARGET ± 50 mV] | PENDING |
| TC-34 | Battery | Low-battery policy | Lower supply with bench source | Warning flag at threshold; critical → System OFF (button only) | PENDING |
| TC-35 | Fault | Watchdog reset | Force infinite loop in test build | WDT reset within timeout; counter++ | PENDING |
| TC-36 | Fault | HardFault capture | Trigger deliberate fault | Fault record stored; reported after reboot | PENDING |
| TC-37 | Fault | Invalid control frame | Send bad CRC/length | Ignored; `bad_frames++` | PENDING |
| TC-38 | RF | Range test | Measure RSSI/range LOS | Record; compare across TX power settings | PENDING |
| TC-39 | Stress | Mode-cycling soak | Automated: adv→conn→OFF→wake × 1000 | No lock-up, no memory growth | PENDING |
| TC-40 | Stress | Temperature sweep (optional) | 0 °C / 25 °C / 45 °C | Sleep current and BLE stability recorded | PENDING |

### 18.3 Traceability

| Objective (Part 1) | Test cases |
|---|---|
| O1 System ON | TC-12 to TC-15 |
| O2 System OFF | TC-16 to TC-20 |
| O3 Wake tuning | TC-21 to TC-26 |
| O4 Retention | TC-27 to TC-29 |
| O5 Connected low power | TC-09, TC-13, TC-14 |
| O6 Robustness | TC-31 to TC-37, TC-39 |
| O7 Portability | Build + TC-01, TC-10, TC-21 on nRF52833 |

### 18.4 Entry/exit criteria
Entry: firmware builds with logging off; PPK2 calibrated; board assembled and inspected. Exit: all TC with defined criteria executed, deviations documented, no open severity-1 defects.

*End of Part 3.*
