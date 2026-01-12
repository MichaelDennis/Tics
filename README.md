**The Tics C++ Based RTOS Project**
This page houses the source code for the Tics RTOS.

**Tics RTOS — Feature Overview** 
Tics is a lightweight, message-based real-time operating system designed for clarity, portability, and 
rapid prototyping. This brief summary will be replaced with a full README later.

**Core Features**
• Cooperative multitasking: Each task runs until it voluntarily yields control, typically by 
waiting for a msg.
• Interrupt-driven preemption: ISRs can perform all required work or schedule a task to run 
later.
• Deterministic ISR behavior: After completing its work, every ISR returns to the task that was 
interrupted.

**Architecture**
• Implemented in C++, with a small amount of assembly used only for context switching.
• Portable context switcher: The assembly portion is minimal and easy to adapt to new 
processors.
• Runs on Windows and Linux (WSL): Ideal for rapid development, debugging, and 
simulation.

**Programming Model**
• Small, simple API: Only a handful of RTOS calls, making the system easy to learn and reason 
about.
• Message-based inter-task communication:
• Tasks can send, receive, wait for, and reply to messages.
• Messages may be deleted even while in transit.

**Memory Management**
• Custom memory system: Dynamic fixed-block allocation.
• Dynamic creation and deletion: Tasks and messages can be created or destroyed at runtime.

**This is a Temporary README file**
This is a temporary README and will be replaced with a more complete version in the future

