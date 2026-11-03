#!/bin/bash
# Scripts/launch_qemu.sh - FINAL CLEAN - no log, no cout, no growing files
# setsid so no popup, /dev/null so no disk growth overnight

set -e
TARGET_ARCH="$1"

killall -9 qemu-system-riscv32 qemu-system-arm 2>/dev/null || true
sleep 0.3

if [ "$TARGET_ARCH" = "riscv32" ]; then
    echo "Starting QEMU RISC-V32 Virt..."
    setsid qemu-system-riscv32 -machine virt -cpu rv32 -m 128M -nographic -bios none -kernel "${PWD}/Bin/Main.elf" -s -S </dev/null >/dev/null 2>&1 &
    echo "QEMU RISC-V32 started PID $!"
elif [ "$TARGET_ARCH" = "arm32" ]; then
    echo "Starting QEMU ARM32 MPS2-AN385..."
    setsid qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -nographic -kernel "${PWD}/Bin/Main.elf" -s -S </dev/null >/dev/null 2>&1 &
    echo "QEMU ARM32 started PID $!"
else
    echo "Usage: $0 riscv32|arm32"
    exit 1
fi

sleep 0.5
echo "QEMU ready on localhost:1234"
