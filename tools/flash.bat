@echo off
rem Automated flashing script for Nordic nRF52840 / nRF52833 ULP-BLE Node on Windows

set HEX_FILE=firmware\build\armgcc\_build\ulp_ble.hex
set SOFTDEVICE_HEX=%NRF5_SDK_ROOT%\components\softdevice\s140\hex\s140_nrf52_7.2.0_softdevice.hex

echo === Flashing ULP-BLE Firmware ===

where nrfjprog >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: nrfjprog (Nordic Command Line Tools) is not in PATH.
    exit /b 1
)

echo 1. Erasing chip...
nrfjprog --family NRF52 --eraseall
if %errorlevel% neq 0 exit /b %errorlevel%

if exist "%SOFTDEVICE_HEX%" (
    echo 2. Flashing SoftDevice S140: %SOFTDEVICE_HEX%...
    nrfjprog --family NRF52 --program "%SOFTDEVICE_HEX%" --sectorerase --verify
) else (
    echo Note: SoftDevice HEX not found at %SOFTDEVICE_HEX%. Skipping SoftDevice.
)

if exist "%HEX_FILE%" (
    echo 3. Flashing Application HEX: %HEX_FILE%...
    nrfjprog --family NRF52 --program "%HEX_FILE%" --sectorerase --verify
) else (
    echo Error: Application HEX %HEX_FILE% not found. Please build first.
    exit /b 1
)

echo 4. Resetting target...
nrfjprog --family NRF52 --reset

echo === Flashing completed successfully ===
