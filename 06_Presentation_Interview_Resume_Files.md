# ULP-BLE Node: Part 6 of 6: Presentation, Interview Q&A, Resume and File List (Sections 27-30)

**Document ID:** ULPBLE-DOC-06 | **Revision:** 0.1 (draft) | **Date:** 2026-10-20
Legend tags are defined in Part 1. Replace every `[fill]` with your own measured data before presenting or putting numbers on a resume.

---

## 27. Project Presentation Content (14 slides)

**Slide 1: Title**
Nordic nRF52840/nRF52833 Ultra-Low-Power BLE Firmware. System ON / System OFF power management. Name, organization, date.

**Slide 2: Problem**
- Battery BLE devices fail on average current, not on features.
- Common leaks: peripherals left on, floating GPIO, aggressive advertising, logging in field, no deep sleep.
- Goal: multi-month to multi-year battery life.

**Slide 3: Objectives**
- Implement and validate System ON and System OFF.
- Tune wake sources and state retention.
- Keep BLE connectivity reliable.
- Targets (not measurements): OFF ≤ 2 µA, slow adv/connected idle ≤ 15 µA.

**Slide 4: Solution overview**
Event-driven firmware on nRF5 SDK + S140; lifecycle ADV → CONNECTED → idle → System OFF → wake; custom GATT service for data and control.

**Slide 5: Hardware**
nRF52840 module (nRF52833 port), TMP117, LIS2DW12 (wake-on-motion), button, CR2477, SWD. Block diagram.

**Slide 6: Power modes explained**
- System ON: CPU sleeps, RTC + LFCLK alive, RAM retained, fast wake.
- System OFF: deepest state, wake only via GPIO SENSE/LPCOMP/NFC/reset; wake = reset.

**Slide 7: State machine**
Diagram from Part 2, Section 14: BOOT, ADV_FAST, ADV_SLOW, CONN_IDLE, CONN_ACTIVE, PREP_SYSOFF, SYSOFF, FAULT.

**Slide 8: Wake-source tuning**
Button (SENSE low), motion INT1 (SENSE high), optional external RTC. Guard: never enter System OFF with a wake pin asserted. Debounce and hold-off.

**Slide 9: State retention**
GPREGRET for tiny flags; `.noinit` record with magic + version + CRC in retained RAM section; FDS flash for config. Restore or default.

**Slide 10: BLE and peripheral power discipline**
Adv fast→slow→OFF; connection interval 500 ms, latency 4, timeout 6 s (constraint verified); TWIM/SAADC on only during use; logging off; unused pins parked.

**Slide 11: Reliability**
Watchdog with health bits, HardFault capture, CRC-protected frames, bus recovery, FMEA top risks.

**Slide 12: Test and measurement method**
40 test cases; PPK2 profiling on production build without debugger; sniffer for timing.

**Slide 13: Results (fill with measured data)**
Table: OFF current, adv avg, connected avg, wake latency, flash/RAM. Add PPK2 screenshots. Do not show targets as results.

**Slide 14: Lessons, future work, Q&A**
Lessons: pin leakage and debugger effects dominate; wake = reset requires design for retention. Future: DFU, secure control channel, nRF Connect SDK migration, Coded PHY study.

---

## 28. Project-Specific Interview Questions and Answers (36)

**Fundamentals**

1. **What is the difference between System ON and System OFF on the nRF52?**
System ON keeps the system powered with the CPU sleeping (WFE/WFI); the RTC, low-frequency clock, and enabled peripherals can run and wake the CPU, and RAM can stay retained. System OFF is the deepest state: nearly everything is powered down, only wake-capable logic remains, and leaving it causes a **reset**, so execution restarts from the reset vector.

2. **What can wake the nRF52840 from System OFF?**
GPIO via SENSE/DETECT, the low-power comparator (LPCOMP), an NFC field, a reset, or the debug interface. The internal RTC does not run in System OFF.

3. **Why does that matter for periodic telemetry?**
You cannot rely on an internal timer to wake from System OFF. Either stay in System ON with long intervals, or add an external RTC whose interrupt drives a SENSE pin.

4. **Why must you use `sd_power_system_off()` instead of writing registers directly?**
With the SoftDevice enabled, it owns the POWER peripheral. Direct register access can conflict with the SoftDevice's assumptions and is not allowed; the SoftDevice API is the supported route.

5. **What does your firmware do while in System ON and idle?**
It drains the scheduler and calls `nrf_pwr_mgmt_run()`, which sleeps with WFE (using `sd_app_evt_wait()` when the SoftDevice is enabled). All work is event-driven; there is no polling or busy-wait.

6. **How do you know why the device woke up?**
At boot, before enabling the SoftDevice, I read `RESETREAS`; the OFF bit indicates wake from System OFF via GPIO. Then I read and clear the GPIO `LATCH` to identify which pin triggered. Other bits identify WDT, soft reset or lockup.

7. **How does the SENSE mechanism work?**
A pin configured with `nrf_gpio_cfg_sense_input` has its sense level monitored in low-power state. When the pin matches the sense level, it generates a DETECT signal that wakes the system from System OFF (and sets a latch bit).

8. **What is the hazard of entering System OFF while a wake pin is already at its sense level?**
The device wakes immediately and may loop. I check the pin state before entry, clear sensor interrupt latches, and defer entry until released.

**Power design**

9. **Where does most average current go in a BLE peripheral?**
Radio events (advertising/connection), then baseline sleep current, then peripherals and leakage. With a 1 s advertising interval, advertising events typically dominate the average; I verify this with PPK2 rather than assume.

10. **How did you tune advertising?**
Fast advertising (~100 ms) for a bounded window for quick discovery, then slow (~1 s), then System OFF. Average current scales roughly inversely with interval, so shortening the fast window matters. Values are tuned by measurement.

11. **What connection parameters did you choose and why?**
Interval ≈ 500 ms, slave latency 4, supervision timeout 6 s. Latency lets the peripheral skip events when it has no data, saving energy. I verified `timeout > (1 + latency) × interval × 2` (5000 ms < 6000 ms).

12. **Why enable the DC/DC converter, and what is the risk?**
It lowers current during radio activity compared with the LDO. The risk is enabling it when the inductor/capacitors are not on the board, which can break operation. I gate it with a hardware-variant macro.

13. **What peripherals did you power-gate and how?**
TWIM and SAADC are enabled only during a transaction/sample and then disabled; UART/RTT logging is compiled out in production; sensors are placed in shutdown/one-shot modes; unused GPIO are input with buffer disconnected.

14. **Why disable logging in production?**
UART and RTT keep clocks/peripherals active and raise current; they also change timing. All power measurements use a no-log build.

15. **Why measure without the debugger?**
An attached debugger can add current and can emulate System OFF rather than fully entering it, so readings are not representative.

16. **How would you estimate battery life?**
`Life = usable capacity / I_avg`, with I_avg computed from per-event charge and periods, then validated by measuring. I also account for cell derating, pulse capability, self-discharge, and leakage. My documented numbers are assumptions until measured.

17. **What would you check if System OFF current is higher than expected?**
Disconnected debugger, floating/pulled GPIO, external pull-up contention, peripherals left enabled, sensor sleep state, retained RAM amount, flux leakage, and a minimal-firmware baseline to bisect.

**Retention**

18. **What survives System OFF?**
By default, RAM does not. GPREGRET registers (small) survive, retained RAM sections survive if configured, and flash obviously persists. Peripheral registers are reset.

19. **How did you design retention?**
A small CRC-protected record in a retained RAM section (magic, version, payload, CRC-16) for counters/state; flash (FDS) for configuration that changes rarely; GPREGRET for tiny flags like "woke from user shutdown". On boot I validate and fall back to defaults.

20. **Why not retain all RAM?**
Each retained block adds sleep current. Retain only what's needed.

21. **Why limit flash writes?**
Energy cost, time, and finite endurance. Write only on configuration changes or material counter changes.

22. **What happens if power drops during a retention write?**
The CRC fails on restore; the firmware loads defaults, sets a flag, and continues; it never trusts partial data.

**BLE and protocol**

23. **Why keep the telemetry frame at 20 bytes?**
It fits the default ATT MTU (23 bytes minus 3 for header) so it works without MTU negotiation on any central.

24. **Why CRC-16 in a notify frame if BLE already has link-layer CRC?**
The link-layer CRC protects the radio packet; the application CRC guards against logic/format errors and persisted data, and enables a consistent frame spec reused for storage and testing.

25. **How would you secure the Control characteristic?**
Require LE Secure Connections pairing and an encrypted link for writes, optionally an authorization token; reject otherwise and count attempts.

26. **What happens when the central rejects your connection-parameter update?**
The firmware works with whatever parameters the central accepts. I log the applied values and keep the design correct for a range, not a single point.

**RTOS and architecture**

27. **Why did you choose event-driven bare-metal for the baseline?**
Lower overhead, fewer wakeups, simpler analysis. The application is small and mostly reactive. A FreeRTOS tickless variant is documented for teams needing task isolation.

28. **How does FreeRTOS tickless idle interact with the SoftDevice?**
The port uses RTC1 for the tick and calls `sd_app_evt_wait()`; the SoftDevice reserves certain interrupt priorities, so FreeRTOS-aware ISRs must respect priority limits.

29. **What interrupt priorities can the application use with the SoftDevice?**
The SoftDevice reserves some levels (0, 1, 4 on nRF52); the application uses the others, with FreeRTOS `FromISR` calls restricted to allowed levels. Verify against the SoftDevice spec.

**Reliability**

30. **How does your watchdog avoid masking hangs?**
It is fed only by a periodic timer callback that checks health bits set by the main flow, BLE activity, and timer ticks. A stuck main loop stops setting its bit, so the watchdog fires.

31. **What do you do on a HardFault?**
Store stacked registers (PC, LR, xPSR, etc.) in a `.noinit` record with CRC, then reset; after reboot the fault is reported and logged and decoded with `addr2line`.

32. **How do you recover a stuck I²C bus?**
Disable TWIM, clock SCL up to nine times with SDA released until SDA goes high, issue STOP, re-enable and re-initialize the sensor.

33. **What is the highest-risk failure in your FMEA and how is it mitigated?**
Brown-out under TX with a high-resistance coin cell. Mitigation: low-ESR bulk capacitor, TX power policy, reset-reason logging, and scope verification of VDD sag.

**Process and judgement**

34. **How did you validate low-power claims?**
PPK2 captures under production firmware, documented conditions (voltage, temperature, parameters), repeated runs, and a bisecting method from a minimal baseline. I report measured values separately from targets.

35. **What would you change if the project had to support firmware updates?**
Add a power-aware bootloader (DFU), keep retention format versioned, and make sure update windows do not violate the energy budget, plus signing for security.

36. **What would you do differently on nRF Connect SDK?**
Use Zephyr power management and devicetree for pin/wake configuration, keep the platform-independent modules (`protocol`, `retention`, `power_mgr` policy) and replace the SDK-specific HAL.

---

## 29. Resume Bullet Points

Use only after you replace brackets with your own measured data. If you did not measure something, delete that clause.

1. Developed **Embedded C** firmware on **Nordic nRF52840/nRF52833** (nRF5 SDK, S140 SoftDevice) implementing and validating **System ON and System OFF** low-power modes for battery-powered BLE devices.
2. Designed an event-driven power-state machine (advertising → connected → deep sleep) with tuned **wake-up sources** (GPIO SENSE button, accelerometer motion interrupt) and CRC-protected **state retention** across System OFF using GPREGRET, retained RAM and flash records.
3. Reduced average current by gating peripherals (TWIM, SAADC, UARTE), parking unused GPIO, enabling DC/DC, and tuning BLE advertising/connection parameters; verified with **Nordic PPK2** profiling *[add your measured result, e.g., "System OFF at X µA, connected idle at Y µA"]*.
4. Built a custom **GATT service** with a CRC-16-protected 20-byte telemetry frame, configuration and control characteristics, plus watchdog health monitoring and HardFault capture for field diagnostics.
5. Authored the test plan (40 cases), FMEA, PCB low-power layout guidelines and developer documentation; optional **FreeRTOS** tickless variant designed for task-based deployments.

---

## 30. Complete List of Required Project Documents / Files

| # | Document / file | Purpose | In this package |
|---|---|---|---|
| 1 | Project overview and abstract | Context | Part 1 §1 |
| 2 | Requirements / objectives | Scope, targets | Part 1 §2 |
| 3 | Features and applications | Product definition | Part 1 §3 |
| 4 | BOM (`bom.csv`) | Procurement | Part 1 §4 (convert to CSV) |
| 5 | MCU selection and pin map | Hardware decision | Part 1 §5 |
| 6 | Schematic (PDF + EDA files) | Hardware design | To create from §6 |
| 7 | PCB layout and Gerbers | Fabrication | To create from Part 3 §17 |
| 8 | Block diagram | System view | Part 1 §7 |
| 9 | Architecture document | HW/SW design | Part 1 §8, Part 2 §9-10 |
| 10 | Firmware source tree | Implementation | Skeletons in Part 2; repo in Part 5 §24 |
| 11 | `sdk_config.h`, linker scripts, Makefile | Build | To create |
| 12 | Protocol / frame specification | Interfaces | Part 2 §12-13 |
| 13 | State machine and flowcharts | Behaviour | Part 2 §14 |
| 14 | RTOS design (optional) | FreeRTOS variant | Part 2 §11 |
| 15 | Error handling and watchdog design | Robustness | Part 2 §15 |
| 16 | Power and memory analysis | Budgets | Part 3 §16 |
| 17 | PCB design guidelines | Layout rules | Part 3 §17 |
| 18 | Test plan and 40 test cases | Verification | Part 3 §18 |
| 19 | Test results (CSV, PPK2 captures) | Evidence | **Empty: you create** (`tests/results/`) |
| 20 | Debugging guide | Support | Part 4 §19 |
| 21 | Performance analysis | Metrics | Part 4 §20 |
| 22 | FMEA / reliability | Risk | Part 4 §21 |
| 23 | User manual | End user | Part 5 §22 |
| 24 | Developer guide | Team onboarding | Part 5 §23 |
| 25 | Repository structure | Organization | Part 5 §24 |
| 26 | README.md | Entry point | Part 5 §25 |
| 27 | Final project report | Summary | Part 5 §26 (results pending) |
| 28 | Presentation (14 slides) | Communication | Part 6 §27 |
| 29 | Interview Q&A (36) | Career prep | Part 6 §28 |
| 30 | Resume bullets | Career | Part 6 §29 |
| 31 | Release notes / CHANGELOG, LICENSE | Governance | To create |
| 32 | CI workflow (build, host tests, static analysis) | Quality | To create |

**What you must still produce yourself:** the actual firmware source, schematic/PCB files, bench measurements, and filled results. Nothing here substitutes for running the firmware on hardware.

*End of Part 6.*
