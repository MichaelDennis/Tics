#!/bin/bash
# start_qemu_listener.sh - Independent QEMU Lifecycle Manager

# Navigate to the explicit project root workspace directory
cd ~/projects/Tics

echo "Tics QEMU Background Engine initialized. Waiting for trigger..."

# Infinite loop listening for compilation handshakes
while true; do
    # Halt and block until data enters the FIFO pipe (0% CPU consumption)
    read line < ~/projects/Tics/Bin/qemu_trigger
    
    # Perform a clean sweep: obliterate any stale emulator instances safely
    killall -9 qemu-system-riscv32 2>/dev/null
    killall -9 qemu-system-riscv64 2>/dev/null
    
    # Boot a fresh QEMU emulator instance in the background
    qemu-system-riscv32 -machine virt -cpu rv32 -smp 1 -m 128M -bios none -kernel ./Bin/Main.elf -display none -semihosting -s -S &
done
