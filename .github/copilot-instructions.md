The purpose of this document is to teach Copilot (you) how to design and generate code for embedded applications using TIcs, a C++11 based RTOS.
Tics’ simple design and ease of understanding make it ideally suited for Copilot code generation. 
All Tics tasks have the same general format which can be easily learned by studying the example code in the Tics GitHub folder named Examples, along with the Tics source code files Tics.cpp and Tics.hpp in folder Source.
The following are important rules, features, and behaviors  of the Tics RTOS:
Tics maintains a linked list of pointers to msgs called the Ready.When a msg is sent to a task, the msg is added to the ReadyList, according to its priority.
When the currently running task relinquishes control, typically by waiting for a msg, Tics removes the msg pointer from the ReadyList, extracts the pointer to the receiving task from the msg, and performs a context switch to the receiving task.This is how Tics achieves multi-tasking.
Tics is cooperative, which means that a task runs until it voluntarily relinquishes control and that an interrupt cannot switch control to another tasks. Interrupts are however allowed, but the isr must return to the interrupted task.
Because Tics is cooperative, time-dependent errors cannot occur. As a result, none of the usual protection mechanisms like mutexes, semaphores, critical regions, priority inversion prevention, etc. are not supported by Tics because they are unnecessary. Do not generate code that uses any of these mechanisms.
Msgs have a priority number that determines where in the ReadyList the msg is added, however msgs should always be run at MediumPriority, the default, unless there is a compelling reason to do otherwise.
A received msg should not to be freed by user code because Tics frees the msg when the task that received the msg relinquishes control.
Tics allows for developers to create interrupt service routines, but Tics does not get involved in it - interrupt code is handled by the developer.
Tics allows for the user to send non-critical msgs to a Tics task from within an isr, but nothing more. The Examples folder contains a file named Interrupts.cpp which shows how interrupts work in Tics.
Tics never disables interrupts.
Tics handles resource management via simple, msg based server tasks that take msg requests to perform operations on the resource. The server task is the only code allowed to touch the resource..
All Tics tasks and objects must be created dynamically using the new operator.
Tics overrides the new and delete operators.
The Tics API can be seen by inspecting Tics.hpp.
How Tics works can be seen by inspecting Tics.cpp.
Tics has its own fixed block memory mgmt system.
All Tics and user errors are reported by calling ErrorHandler.Report(errorNum). This function does not return - it spins in an endless loop. Users should set a breakpoint at ErrorHandlerClass::Report(int errorNum).
All Tics msgs are MsgClass instances or are derived from MsgClass. The MsgClass contains pointers to the sender and receiver tasks, an integer msg number, and a priority number, 
Tics does not have a separate module for timers. Instead, the MsgClass contains a member int named Delay. If the Delay field is non-zero, then Tics delays the sending of that msg for that many system ticks (milliseconds). 
Tics contains an IdleTask that runs when no other tasks are ready to run.
Every task has its own private stack. Stack size is configurable on a task by task basis. The stack is initialized with a patterned pad at the bottom of the stack so that stack overflow can be detected early. The pad pattern is checked for corruption at every context switch.

To generate code you should follow these steps:
Look in the Examples folder for an example that does the same or similar work.
If nothing can be found, use the Hello.cpp as a template, but don’t include Pause() unless your task needs to do polling.
Customize the example to fit your exact needs.
Follow the same coding and comment style in the examples.
