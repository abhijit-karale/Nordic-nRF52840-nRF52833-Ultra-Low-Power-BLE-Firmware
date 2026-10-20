# ULP-BLE Node: nRF52840 / nRF52833 Ultra-Low-Power BLE Firmware

Embedded C firmware for a battery-powered Bluetooth Low Energy peripheral on Nordic nRF52840 (primary) and nRF52833 (port target), built with an event-driven run-to-completion architecture, Nordic SoftDevice S140 stack, and strict low-power discipline.

The project implements and validates **System ON** and **System OFF** power regimes, tuned wake-up sources (button, accelerometer wake-on-motion, optional external RTC), CRC-16 protected **state retention** across deep sleep, peripheral power gating, and PPK2 energy profiling.

---

## Key Features

- **Dual Power-Mode Lifecycle:**
  `BOOT -> ADV_FAST (100 ms) -> ADV_SLOW (1000 ms) -> PREP_SYSOFF -> SYSOFF (Deep Sleep) -> (Wake = Reset) -> BOOT`
  Connected branch: `S_CONN_IDLE <-> S_CONN_ACTIVE` with slave latency = 4.
- **Deep Sleep (System OFF):**
  Uses `sd_power_system_off()`. Low-power hardware GPIO SENSE DETECT wakes the SoC. Board-level current target **<= 2 µA** with accelerometer armed.
- **Wake Loop Prevention Guard:**
  Firmware inspects all armed wake lines before sleep; defers System OFF entry if a wake line is actively asserted (e.g., held button).
- **State Retention Across Deep Sleep:**
  - `GPREGRET` hardware register for flags.
  - `.noinit.retention` RAM section with magic (`0x55AA1234`), struct version, boot counters, uptime, motion count, and **CRC-16/CCITT-FALSE**.
  - Fallback to safe defaults upon memory corruption (`TC-29`).
- **Peripheral Power Gating:**
  - **TWIM0 (I2C):** Enabled only during transaction (`hal_twi_acquire()`) and disconnected immediately (`hal_twi_release()`) with 9-pulse bus-recovery support.
  - **SAADC:** Direct internal VDD sampling (0.6V internal ref, 1/6 gain, no external resistor divider leakage); completely uninitialized between samples.
  - **GPIO:** Unused pins parked as `NRF_GPIO_PIN_INPUT_DISCONNECT` with no pull.
- **Custom GATT Service (128-bit Vendor Base):**
  - **Telemetry (0x0002):** Notify, exactly 20 bytes (fits default ATT MTU), packed with temperature x100, battery mV, power state, wake reason, flags, retained counters, and CRC-16.
  - **Config (0x0003):** Read/Write, 15 bytes, adaptive advertising intervals and wake masks.
  - **Control (0x0004):** Write without response, opcode framing (`ENTER_SYSOFF`, `SET_ADV`, `SET_TELEM`, `GET_STATUS`, `CLEAR_COUNTERS`, `SOFT_RESET`).
  - **Status (0x0005):** Read, 24 bytes, boot counts, bad frames, last fault code & PC.
- **Robustness & Diagnostics:**
  - Hardware watchdog (10s target) fed only by multi-source health monitor (`H_MAIN`, `H_BLE`, `H_TIMER`).
  - HardFault exception capture: stacked CPU context (R0-R3, R12, LR, PC, xPSR) preserved into `.noinit.fault_record` with CRC.

---

## Hardware Architecture

| Item | Component | Function |
|---|---|---|
| **SoC (Primary)** | Nordic nRF52840 (Raytac MDBT50Q) | Arm Cortex-M4F, 1MB Flash, 256kB RAM, BLE 5 |
| **SoC (Port Target)**| Nordic nRF52833 (Raytac MDBT50Q-512K) | Arm Cortex-M4F, 512kB Flash, 128kB RAM |
| **Temperature Sensor** | TI TMP117 (I2C 0x48) | High precision (+/-0.1 °C), one-shot low power mode |
| **Accelerometer** | ST LIS2DW12 (I2C 0x19) | Low power (~380 nA) wake-on-motion on INT1 |
| **Optional RTC** | Micro Crystal RV-3028-C7 (I2C 0x52) | Periodic wake from System OFF via alarm pin |
| **Battery** | CR2477 Coin Cell (3.0 V, ~1000 mAh) | Direct SoC supply + 47 µF low-ESR bulk cap |

### Pin Assignments

| Signal | MCU Pin | Direction | Low-Power State |
|---|---|---|---|
| I2C SDA | P0.26 | Bidir | Disconnected / S0D1 when active |
| I2C SCL | P0.27 | Out | Disconnected / S0D1 when active |
| BTN | P0.11 | In | Internal pull-up, SENSE LOW |
| ACC_INT1 | P0.28 | In | Pull-down, SENSE HIGH |
| TEMP_ALERT | P0.29 | In | Input, no pull |
| RTC_INT | P0.30 | In | SENSE LOW |
| LED | P0.13 | Out | Disconnected in production |
| UART TX/RX | P0.06 / P0.08 | Out / In | Disconnected in production |

---

## Repository Structure

```
.
├── 01_Overview_Hardware_Architecture.md    # Documentation Part 1
├── 02_Firmware_Architecture.md             # Documentation Part 2
├── 03_Power_PCB_TestPlan.md                # Documentation Part 3
├── 04_Debug_Performance_FMEA.md            # Documentation Part 4
├── 05_Manuals_Repo_README_Report.md        # Documentation Part 5
├── 06_Presentation_Interview_Resume_Files.md # Documentation Part 6
├── README.md
├── LICENSE
├── CHANGELOG.md
├── hardware/
│   └── bom/bom.csv                         # Bill of Materials
├── firmware/
│   ├── app/
│   │   ├── inc/ (protocol.h, app_fsm.h, retention.h, wake_mgr.h, ble_ulp_svc.h, ...)
│   │   └── src/ (app_main.c, app_fsm.c, retention.c, wake_mgr.c, ble_ulp_svc.c, ...)
│   ├── drivers/ (drv_tmp117.c/.h, drv_lis2dw12.c/.h, drv_vbat.c/.h)
│   ├── hal/     (hal_gpio_lp.c/.h, hal_twi.c/.h, hal_saadc.c/.h, hal_wdt.c/.h)
│   ├── boards/  (ulp_board_52840.h, ulp_board_52833.h, pca10056.h, pca10100.h)
│   ├── config/  (sdk_config.h, ulp_ble_nrf52840.ld, ulp_ble_nrf52833.ld)
│   ├── sdk_compat/ (Clean Nordic CMSIS & SoftDevice API compatibility headers)
│   └── build/armgcc/ (Makefile)
├── tests/
│   ├── unit/ (run_tests.py, test_suite.c)
│   ├── integration/ (test_cases.csv)
│   └── results/ (PPK2 profiling and soak test logs)
└── tools/
    ├── ppk2_capture/export_ppk2.py          # PPK2 current trace analysis tool
    ├── decode_fault.py                     # HardFault addr2line decoder
    ├── flash.sh                            # Automated bash flashing script
    └── flash.bat                           # Automated Windows flashing script
```

---

## Build & Flash Instructions

### Prerequisites
- GNU Arm Embedded Toolchain (`arm-none-eabi-gcc`)
- GNU Make
- Nordic Command Line Tools (`nrfjprog`)
- Nordic nRF5 SDK 17.1.0 + SoftDevice S140 (optional: project includes self-contained SDK compatibility headers)

### Compilation

```bash
# Build for nRF52840 (production, logging off)
make -C firmware/build/armgcc BOARD=ulp_board_52840 LOG=0

# Build for nRF52833 port target
make -C firmware/build/armgcc BOARD=ulp_board_52833 LOG=0

# Build for nRF52840-DK (PCA10056) with development logging
make -C firmware/build/armgcc BOARD=pca10056 LOG=1
```

### Flashing

```bash
# Using Makefile
make -C firmware/build/armgcc flash

# Or using the automated flashing script
tools/flash.sh      # Linux / macOS
tools\flash.bat     # Windows
```

---

## Verification & Unit Testing

Execute the test suite verifying CRC-16/CCITT-FALSE, 20-byte Telemetry framing, Config characteristic serialization, Control opcode parsing, and retention corruption recovery:

```bash
python tests/unit/run_tests.py
```

### Traceability to Test Plan (ULPBLE-DOC-03)
The 40 benchmark verification test cases (TC-01 through TC-40) are cataloged in `tests/integration/test_cases.csv`.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
