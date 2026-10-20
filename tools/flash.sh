#!/usr/bin/env bash
# Automated flashing script for Nordic nRF52840 / nRF52833 ULP-BLE Node

set -euo pipefail

HEX_FILE="firmware/build/armgcc/_build/ulp_ble.hex"
SOFTDEVICE_HEX="${NRF5_SDK_ROOT:-}/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex"

echo "=== Flashing ULP-BLE Firmware ==="

if ! command -v nrfjprog &> /dev/null; then
    echo "Error: nrfjprog (Nordic Command Line Tools) is not in PATH."
    exit 1
fi

echo "1. Erasing chip..."
nrfjprog --family NRF52 --eraseall

if [[ -f "${SOFTDEVICE_HEX}" ]]; then
    echo "2. Flashing SoftDevice S140: ${SOFTDEVICE_HEX}..."
    nrfjprog --family NRF52 --program "${SOFTDEVICE_HEX}" --sectorerase --verify
else
    echo "Note: SoftDevice HEX not found at ${SOFTDEVICE_HEX}. Skipping SoftDevice flash."
fi

if [[ -f "${HEX_FILE}" ]]; then
    echo "3. Flashing Application HEX: ${HEX_FILE}..."
    nrfjprog --family NRF52 --program "${HEX_FILE}" --sectorerase --verify
else
    echo "Error: Application HEX file ${HEX_FILE} not found. Please build first."
    exit 1
fi

echo "4. Resetting target..."
nrfjprog --family NRF52 --reset

echo "=== Flashing completed successfully ==="
