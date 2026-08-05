#!/bin/bash
# start_qemu_listener.sh - Independent QEMU Lifecycle Manager

# Navigate to the explicit project root workspace directory
cd ~/projects/Tics

# Ensure the Bin directory exists
mkdir -p ./Bin

# AUTOMATION HOOK: If the named pipe does not exist, create it automatically
if [ ! -p ./Bin/qemu_trigger ]; then
    echo "Creating named pipe at ./Bin/qemu_trigger..."
    mkfifo ./Bin/qemu_trigger
fi

echo "Tics QEMU Background Engine initialized. Waiting for trigger..."

# Infinite loop listening for compilation handshakes
while true; do
    # Halt and block until data enters the FIFO pipe
    read line < ~/projects/Tics/Bin/qemu_trigger
    
    # Perform a clean sweep: obliterate any stale emulator instances safely
    killall -9 qemu-system-riscv32 2>/dev/null
    killall -9 qemu-system-riscv64 2>/dev/null
    
    # Boot a fresh QEMU emulator instance in the background
    qemu-system-riscv32 -machine virt -cpu rv32 -smp 1 -m 128M -bios none -kernel ./Bin/Main.elf -display none -semihosting -s -S &
done
