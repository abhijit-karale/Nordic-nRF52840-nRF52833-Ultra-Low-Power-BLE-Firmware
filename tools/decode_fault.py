#!/usr/bin/env python3
"""
ULP-BLE HardFault Register Decoder
Parses stacked register frame captured into .noinit RAM and invokes
arm-none-eabi-addr2line to pinpoint fault source code location.
"""

import sys
import subprocess
import argparse

def decode_fault(elf_path, pc_hex, lr_hex):
    print("=" * 60)
    print("  ULP-BLE HardFault Crash Diagnostic Decoder")
    print("=" * 60)
    print(f"ELF binary : {elf_path}")
    print(f"Captured PC: {pc_hex}")
    print(f"Captured LR: {lr_hex}")
    print("-" * 60)

    for reg_name, addr_str in [("Program Counter (PC)", pc_hex), ("Link Register (LR)", lr_hex)]:
        try:
            cmd = ["arm-none-eabi-addr2line", "-e", elf_path, "-f", "-C", "-p", addr_str]
            res = subprocess.run(cmd, capture_output=True, text=True, check=True)
            print(f"{reg_name}: {res.stdout.strip()}")
        except FileNotFoundError:
            print(f"{reg_name}: {addr_str} (arm-none-eabi-addr2line not installed or not in PATH)")
        except subprocess.CalledProcessError as e:
            print(f"{reg_name}: Error running addr2line: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Decode HardFault stacked frame registers")
    parser.add_argument("--elf", default="firmware/build/armgcc/_build/ulp_ble.elf", help="Path to ELF binary")
    parser.add_argument("--pc", required=True, help="Program Counter hex (e.g. 0x000281F4)")
    parser.add_argument("--lr", required=True, help="Link Register hex (e.g. 0x00028A12)")
    args = parser.parse_args()

    decode_fault(args.elf, args.pc, args.lr)
