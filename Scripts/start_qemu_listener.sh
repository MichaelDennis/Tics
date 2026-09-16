#!/bin/bash
# start_qemu_listener.sh - Generalized QEMU Lifecycle Manager

cd ~/projects/Tics
mkdir -p ./Bin

if [ ! -p ./Bin/qemu_trigger ]; then
    echo "Creating named pipe at ./Bin/qemu_trigger..."
    mkfifo ./Bin/qemu_trigger
fi

echo "Tics QEMU Background Engine initialized. Waiting for architecture string..."

while true; do
    # Read the architecture token directly out of the pipe ("riscv32" or "arm32")
    read TARGET_ARCH < ~/projects/Tics/Bin/qemu_trigger
    
    case "$TARGET_ARCH" in
        "riscv32")
            echo "Booting RISC-V32 Emulator Baseline..."
            killall -9 qemu-system-riscv32 2>/dev/null
            qemu-system-riscv32 -machine virt -cpu rv32 -smp 1 -m 128M -bios none -kernel ./Bin/Main.elf -display none -semihosting -s -S &
            ;;
            
        "arm32")
            echo "Booting ARM32 (Cortex-M3 MPS2) Emulator Baseline..."
            killall -9 qemu-system-arm 2>/dev/null
            qemu-system-arm -machine mps2-an385 -cpu cortex-m3 -nographic -kernel ./Bin/Main.elf -s -S &
            ;;
            
        *)
            echo "Unknown target signature received: $TARGET_ARCH"
            ;;
    esac
done
