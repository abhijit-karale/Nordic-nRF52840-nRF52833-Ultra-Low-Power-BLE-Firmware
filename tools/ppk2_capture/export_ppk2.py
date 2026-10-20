#!/usr/bin/env python3
"""
ULP-BLE Power Profiling & PPK2 Data Analysis Utility
Parses Power Profiler Kit 2 (PPK2) CSV exports, computes average current,
peak pulse current, charge per event (uC), and project battery life.
"""

import sys
import csv
import argparse

def analyze_ppk2_csv(filepath, nominal_mah=600.0):
    print(f"Reading PPK2 capture: {filepath}...")
    total_samples = 0
    total_current_ua = 0.0
    max_current_ua = 0.0
    min_current_ua = float("inf")

    try:
        with open(filepath, "r", encoding="utf-8") as f:
            reader = csv.reader(f)
            header = next(reader, None)
            
            # Find column index for current in uA or mA
            curr_idx = 0
            is_ma = False
            if header:
                for idx, col in enumerate(header):
                    col_lower = col.lower()
                    if "current(ua)" in col_lower or "current [ua]" in col_lower or "ua" in col_lower:
                        curr_idx = idx
                        is_ma = False
                        break
                    elif "current(ma)" in col_lower or "current [ma]" in col_lower or "ma" in col_lower:
                        curr_idx = idx
                        is_ma = True
                        break

            for row in reader:
                if not row or len(row) <= curr_idx:
                    continue
                try:
                    val = float(row[curr_idx])
                    if is_ma:
                        val *= 1000.0  # Convert mA to uA
                    total_current_ua += val
                    total_samples += 1
                    if val > max_current_ua:
                        max_current_ua = val
                    if val < min_current_ua:
                        min_current_ua = val
                except ValueError:
                    continue

    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found.")
        return

    if total_samples == 0:
        print("No valid current samples found in file.")
        return

    avg_current_ua = total_current_ua / total_samples
    avg_current_ma = avg_current_ua / 1000.0

    # Battery life estimation
    hours = (nominal_mah / avg_current_ma) if avg_current_ma > 0 else float("inf")
    days = hours / 24.0

    print("=" * 60)
    print("  PPK2 Current Profiling Analysis Report")
    print("=" * 60)
    print(f"Total Samples Analyzed : {total_samples:,}")
    print(f"Average Current (I_avg) : {avg_current_ua:.2f} uA ({avg_current_ma:.4f} mA)")
    print(f"Peak Current (I_peak)   : {max_current_ua:.2f} uA")
    print(f"Minimum Current (I_min) : {min_current_ua:.2f} uA")
    print("-" * 60)
    print(f"Battery Capacity Assumed: {nominal_mah:.0f} mAh (derated CR2477)")
    print(f"Estimated Continuous Run: {hours:.1f} hours ({days:.1f} days / {days/365.25:.2f} years)")
    print("=" * 60)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze Nordic PPK2 current capture CSV")
    parser.add_argument("csv_file", help="Path to exported PPK2 CSV log file")
    parser.add_argument("--capacity", type=float, default=600.0, help="Usable battery capacity in mAh (default: 600)")
    args = parser.parse_args()

    analyze_ppk2_csv(args.csv_file, args.capacity)
