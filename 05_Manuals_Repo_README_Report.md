# ULP-BLE Node: Part 5 of 6: Manuals, Repository, README and Report (Sections 22-26)

**Document ID:** ULPBLE-DOC-05 | **Revision:** 0.1 (draft) | **Date:** 2026-10-20
Legend tags are defined in Part 1. Result fields are intentionally blank: fill them only with measured values.

---

## 22. User Manual

### 22.1 What the device does
The ULP-BLE Node measures temperature and battery voltage, detects motion, and talks to a phone or gateway over Bluetooth Low Energy. To save battery, it goes into a deep sleep (System OFF) when it is not needed and wakes when you press the button or move it.

### 22.2 Controls and indicators

| Item | Function |
|---|---|
| Button | Wakes the device from deep sleep. |
| Motion | Moving or shaking the device wakes it (if motion wake is enabled). |
| LED (if fitted) | Development units only. Production units have it disabled to save power. |

### 22.3 Getting started
1. Insert the battery with correct polarity (positive side up in the holder, per the marking).
2. The device starts advertising as **"ULP-BLE-xxxx"** (last digits from the device address). Fast advertising lasts about 30 s, then slows.
3. On your phone, open a BLE tool such as **nRF Connect**, scan, and connect to the device.
4. Enable notifications on the **Telemetry** characteristic to receive readings.

### 22.4 Everyday operation

| Situation | What happens |
|---|---|
| Not connected | Device advertises (fast, then slow) for the configured time, then enters deep sleep. |
| Deep sleep | Press the button or move the device to wake it; it begins advertising again. |
| Connected | Device sends a reading at the configured period (default **10 s [ASSUMED]**). |
| Disconnect | Device returns to advertising. |
| Low battery | Reported in the status flags. At a critical level the device sleeps and wakes **only on the button**. |

### 22.5 Settings (via the Config characteristic)
Advertising speeds, advertising durations, idle time before deep sleep, reading period, and which wake sources are enabled. Longer intervals save battery but make the device slower to find.

### 22.6 Commands (via the Control characteristic)
Go to deep sleep now, change advertising/reading period, request status, clear counters, soft reset. Commands from unpaired devices may be rejected on production firmware.

### 22.7 Troubleshooting

| Problem | What to try |
|---|---|
| Can't find the device | Press the button to wake it, then scan again; check battery. |
| Disconnects frequently | Move closer; avoid metal enclosures near the antenna. |
| Wakes by itself | Motion wake may be too sensitive; disable motion wake or increase the threshold in config. |
| Battery drains fast | Make sure it is not being kept connected or in fast advertising; contact support if it persists. |

### 22.8 Safety and care
Use only the specified battery type. Do not short, recharge, or incinerate primary cells. Keep away from extreme heat. Dispose of batteries per local regulations. Regulatory marks (e.g., BLE radio approval) depend on the final product and module; they are not claimed here **[VERIFY]**.

---

## 23. Developer Guide

### 23.1 Prerequisites
- **Hardware:** nRF52840-DK (and/or nRF52833-DK), PPK2, USB cable, optional custom board.
- **Software:** nRF5 SDK 17.1.0, matching S140 SoftDevice, GNU Arm Embedded toolchain (`arm-none-eabi-gcc`), `make`, `nrfjprog` / nRF Command Line Tools, SEGGER J-Link tools, optional SEGGER Embedded Studio.
- **Host-test:** a native C compiler and a unit-test framework (Unity or CMock **[EXAMPLE]**).

### 23.2 Build and flash

```bash
# 1. Set SDK path (adjust to your install)
export NRF5_SDK_ROOT=$HOME/sdk/nRF5_SDK_17.1.0_ddde560

# 2. Build for nRF52840 (production, logging off)
make -C build/armgcc BOARD=ulp_board_52840 LOG=0

# 3. Erase, flash SoftDevice then app, reset
nrfjprog --family NRF52 --eraseall
nrfjprog --family NRF52 --program $NRF5_SDK_ROOT/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex --sectorerase --verify
nrfjprog --family NRF52 --program build/armgcc/_build/ulp_ble.hex --sectorerase --verify
nrfjprog --family NRF52 --reset
```

Exact makefile variables, SoftDevice file name and SDK folder name must match your installation **[VERIFY]**.

### 23.3 Build configuration flags

| Flag | Meaning |
|---|---|
| `BOARD=ulp_board_52840 / ulp_board_52833 / pca10056 / pca10100` | Board support selection |
| `LOG=0/1` | Disable/enable `NRF_LOG` (production must be 0) |
| `ULP_USE_FREERTOS=0/1` | Baseline vs FreeRTOS tickless variant |
| `ULP_ENABLE_EXT_RTC=0/1` | Enable external RTC wake source |
| `ULP_RETAIN_RAM=0/1` | Retain a RAM section across System OFF |
| `ULP_DCDC=0/1` | DC/DC enable (needs inductor) |

### 23.4 Adding a new wake source
1. Add pin definition in `board_*.h` and a bit in `wake_mask_t`.
2. In `wake_mgr.c`: add arming code (`nrf_gpio_cfg_sense_input`) and decode (`LATCH`) mapping.
3. Add ack logic in the relevant driver (clear sensor interrupt before System OFF).
4. Add tests: TC-21 style wake test and TC-19 style "pin asserted at entry" test.
5. Update the Config/Status protocol docs.

### 23.5 Adding a new sensor
1. Write `drv_xxx.c` with `init`, `read`, `sleep`.
2. Use `hal_twi_acquire/release` around each transfer.
3. Ensure the sensor has a defined low-power state; confirm its sleep current on the bench.
4. Extend telemetry payload (watch the 20-byte limit or negotiate larger MTU and bump protocol version).

### 23.6 Low-power checklist (before every release)
- Logging, RTT, UART, LEDs disabled.
- All unused GPIO parked.
- TWIM/SAADC/UARTE verified disabled in idle (check peripheral `ENABLE` registers).
- DC/DC enabled only if hardware supports.
- Advertising and connection parameters reviewed.
- Retention save validated; wake pins verified at entry.
- Measured on PPK2 **without** debugger.

### 23.7 Coding standards
Naming: `module_verb_noun()`; types `_t`; constants `UPPER_CASE`; one responsibility per module; no hidden globals; Doxygen comments on public APIs; static analysis (cppcheck/clang-tidy) in CI.

### 23.8 Migration note
Nordic's recommended path for new designs is **nRF Connect SDK (Zephyr)**. Mapping: `app_timer` → Zephyr kernel timers; `nrf_pwr_mgmt` → Zephyr PM / `sys_poweroff()`; SENSE wake → GPIO wake configuration; SoftDevice → Zephyr Bluetooth LE Controller. Keep `power_mgr`, `wake_mgr`, `retention`, `protocol` free of SDK specifics so they port cleanly. **[VERIFY APIs for your NCS version]**

---

## 24. GitHub Repository Structure

```
ulp-ble-nrf52/
├── README.md
├── LICENSE
├── CHANGELOG.md
├── .gitignore
├── .github/workflows/ci.yml            # build + host unit tests + cppcheck
├── docs/
│   ├── 01_requirements.md
│   ├── 02_hardware_design.md
│   ├── 03_firmware_architecture.md
│   ├── 04_protocol_spec.md
│   ├── 05_power_analysis.md
│   ├── 06_pcb_guidelines.md
│   ├── 07_test_plan.md
│   ├── 08_debug_guide.md
│   ├── 09_fmea.md
│   ├── 10_user_manual.md
│   ├── 11_developer_guide.md
│   ├── 12_final_report.md
│   ├── 13_presentation.md
│   ├── 14_interview_qa.md
│   └── images/ (block diagram, state machine, PPK2 captures when available)
├── hardware/
│   ├── schematic/        # PDF + native EDA
│   ├── pcb/              # layout + gerbers
│   ├── bom/bom.csv
│   └── datasheets/       # links or local copies where licence allows
├── firmware/
│   ├── app/
│   │   ├── src/ (app_main.c, app_fsm.c, app_cfg.c, power_mgr.c, wake_mgr.c,
│   │   │         retention.c, ble_ulp_svc.c, protocol.c, telemetry.c, diag.c)
│   │   └── inc/
│   ├── drivers/ (drv_tmp117.c, drv_lis2dw12.c, drv_vbat.c)
│   ├── hal/ (hal_gpio_lp.c, hal_twi.c, hal_saadc.c, hal_wdt.c)
│   ├── boards/ (ulp_board_52840.h, ulp_board_52833.h, pca10056.h, pca10100.h)
│   ├── config/ (sdk_config.h, linker scripts incl. .noinit/retained section)
│   └── build/armgcc/ (Makefile, ld scripts)
├── tests/
│   ├── unit/ (protocol, crc16, retention, fsm)
│   ├── integration/ (python scripts using a BLE library, test_cases.csv)
│   └── results/        # empty until you add measured data
├── tools/
│   ├── ppk2_capture/   # scripts to export/process PPK2 CSV
│   ├── flash.sh
│   └── decode_fault.py
└── release/
    └── (tagged hex files + checksums)
```

---

## 25. Complete README.md

````markdown
# ULP-BLE Node: nRF52840 / nRF52833 Ultra-Low-Power BLE Firmware

Embedded C firmware for a battery-powered Bluetooth Low Energy peripheral on Nordic nRF52840 (primary) and nRF52833 (port target), using the nRF5 SDK and SoftDevice S140. The project implements and validates **System ON** and **System OFF** power modes, with tuned wake-up sources and state retention across deep sleep.

> **Status:** documentation and firmware architecture complete; **measurement results pending**. No performance figures in this repository are measured unless listed under `tests/results/`.

## Features
- System ON low-power operation (WFE sleep, RTC1 timers, adaptive advertising, slave latency)
- System OFF deep sleep with GPIO SENSE wake (button, accelerometer motion, optional external RTC)
- State retention via GPREGRET, CRC-protected `.noinit` record and FDS flash records
- Peripheral power gating (TWIM, SAADC, UARTE), unused-pin parking
- Custom GATT service: Telemetry (notify), Config, Control, Status
- Watchdog with health monitor, HardFault capture, reset-reason tracking
- Optional FreeRTOS tickless variant

## Hardware
| Item | Part (example) |
|---|---|
| SoC / module | nRF52840 (Raytac MDBT50Q family) / nRF52833 variant |
| Temperature sensor | TMP117 (I²C) |
| Accelerometer | LIS2DW12 (I²C + INT1 wake) |
| Battery | CR2477 |
| Debug | SWD (Tag-Connect), nRF52840-DK for bring-up |

Key pins (proposal): SDA P0.26, SCL P0.27, BTN P0.11, ACC_INT1 P0.28, RTC_INT P0.30. Always check against your board.

## Architecture
Layers: Application (FSM, telemetry, config) → Services (power_mgr, wake_mgr, retention, protocol, BLE service) → Drivers/HAL → Nordic SDK/SoftDevice → Hardware.

State machine: `BOOT → ADV_FAST → ADV_SLOW → (timeout) PREP_SYSOFF → SYSOFF → (wake = reset) BOOT`, with `CONN_IDLE/CONN_ACTIVE` while connected.

## Build and flash
```bash
export NRF5_SDK_ROOT=<path to nRF5_SDK_17.1.0>
make -C firmware/build/armgcc BOARD=ulp_board_52840 LOG=0
tools/flash.sh   # SoftDevice S140 then application, then reset
```
Requirements: `arm-none-eabi-gcc`, `make`, nRF Command Line Tools (`nrfjprog`).

## Quick start
1. Flash the DK. 2. Open nRF Connect, scan for `ULP-BLE-xxxx`, connect. 3. Enable Telemetry notifications. 4. Write `ENTER_SYSOFF` to Control, then press the button to wake.

## Protocol summary
Telemetry notify frame: 20 bytes, little-endian, CRC-16/CCITT-FALSE. Fields: version/type, sequence, length, temperature×100, VBAT mV, power state, wake reason, flags, motion count, System-OFF count, uptime, CRC. See `docs/04_protocol_spec.md`.

## Power methodology
Measure with Nordic PPK2, production build (`LOG=0`), debugger disconnected. See `docs/05_power_analysis.md`. Targets (not measurements): System OFF ≤ 2 µA board-level; slow advertising and connected-idle ≤ 15 µA average.

## Testing
40 test cases in `docs/07_test_plan.md`; unit tests in `tests/unit`; record results in `tests/results/`.

## Known limitations
- System OFF wake is a reset; periodic wake from System OFF needs an external RTC.
- nRF5 SDK is in maintenance mode; see the migration notes for nRF Connect SDK.
- Control characteristic should require an encrypted bonded link in production.

## Contributing / License
Pull requests welcome with tests. License: MIT (or choose per your needs).
````

---

## 26. Final Project Report

### 26.1 Title page
**Nordic nRF52840 / nRF52833 Ultra-Low-Power BLE Firmware: System ON / System OFF Power Management**
Author: *[Your name]* | Organization: *[Your organization]* | Date: *[date]* | Version: 0.1

### 26.2 Abstract
This project develops Embedded C firmware on the Nordic nRF5 SDK with a SoftDevice BLE stack to minimize energy consumption of a battery-powered BLE device. It implements and validates System ON and System OFF low-power modes, tunes wake-up sources (button, motion interrupt, optional external RTC), and manages retention of state across deep sleep. A custom GATT service transports representative sensor data so that low-power behaviour can be exercised under a realistic workload.

### 26.3 Introduction and motivation
Battery life is the limiting product requirement for BLE endpoints. A coherent power-state strategy across the whole lifecycle, from advertising through connection to deep sleep, is necessary to meet it.

### 26.4 Requirements and objectives
See Part 1, Section 2.2 (objectives O1-O8).

### 26.5 System design
Hardware (Part 1): module-based SoC, TMP117, LIS2DW12, button, CR2477. Firmware (Part 2): event-driven layered architecture, power-state FSM, retention strategy, protocol.

### 26.6 Implementation highlights
- DC/DC enabled post-SoftDevice init; low-power mode set.
- `sd_power_system_off()` entry only after persistence and wake arming; guarded against asserted wake pins.
- Boot-time reset-reason and GPIO latch decode to identify the wake source.
- GPIO parking and peripheral gating discipline.
- Watchdog fed only through a health monitor.

### 26.7 Verification methodology
Test plan of 40 cases (Part 3); PPK2-based power profiling; sniffer-based BLE timing.

### 26.8 Results (to be completed with measured data only)

| Metric | Target | Measured | Test ID | Conditions |
|---|---|---|---|---|
| System OFF current | ≤ 2 µA | *[fill]* | TC-17 | |
| Adv 1 s average | ≤ 15 µA | *[fill]* | TC-12 | |
| Connected idle average | ≤ 15 µA | *[fill]* | TC-13 | |
| Wake-to-first-adv | < 1 s | *[fill]* | TC-25 | |
| Flash / RAM use | ≤ 100 kB / ≤ 25 kB | *[fill]* | Build map | |
| Spurious wakes in 24 h | 0 | *[fill]* | TC-26 | |

### 26.9 Discussion
*[Compare measured to targets; explain deviations: leakage, DC/DC, crystal start, central parameter policy.]*

### 26.10 Risks and reliability
See FMEA (Part 4, Section 21). Top risks: brown-out under TX, no-wake failure, unauthorized control.

### 26.11 Conclusion and future work
Conclusion *[after results]*. Future work: DFU with power-aware bootloader, LE Secure Connections enforcement, migration to nRF Connect SDK, Coded PHY option analysis, energy-harvesting variant, production test fixture.

### 26.12 References (consult the current versions)
Nordic nRF52840 Product Specification; nRF52833 Product Specification; nRF5 SDK 17.1.0 documentation; S140 SoftDevice Specification; Nordic Online Power Profiler for BLE; PPK2 User Guide; TI TMP117 datasheet; ST LIS2DW12 datasheet; Bluetooth Core Specification.

*End of Part 5.*
