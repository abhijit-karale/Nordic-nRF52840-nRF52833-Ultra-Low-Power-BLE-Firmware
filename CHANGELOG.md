# Changelog

All notable changes to the ULP-BLE Node firmware will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-10-20

### Added
- Complete embedded C firmware implementation on Nordic nRF52840 (primary) and nRF52833 (port target).
- Dual power-mode finite state machine (`S_BOOT`, `S_ADV_FAST`, `S_ADV_SLOW`, `S_CONN_IDLE`, `S_CONN_ACTIVE`, `S_PREP_SYSOFF`, `S_SYSOFF`, `S_FAULT`).
- System OFF deep sleep sequencer with wake pin latch guard preventing immediate wake loops.
- Tuned wake-up sources: Button (active-low with internal pull-up), Accelerometer INT1 (active-high wake-on-motion), optional external RTC alarm.
- State retention subsystem using `GPREGRET` and `.noinit.retention` RAM with CRC-16 validation and default fallback.
- Ephemeral power-gated drivers for TI TMP117 (one-shot mode) and ST LIS2DW12 (latched wake-on-motion interrupt).
- SAADC direct internal VDD battery monitoring with 4-sample moving average filter and critical 2.2V low-battery protection.
- Custom 128-bit ULP GATT service: 20-byte Telemetry (Notify), Config (Read/Write), Control (Write without response), Status (Read).
- Hardware watchdog timer (10s) with multi-source health monitor (`H_MAIN`, `H_BLE`, `H_TIMER`).
- HardFault exception handler with stacked register frame capture into `.noinit` memory.
- Host unit test suite and automated Python test runner for protocol and state machine verification.
- PPK2 profiling analysis script and HardFault diagnostic decoder.
