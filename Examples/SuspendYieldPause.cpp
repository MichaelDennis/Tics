//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
// This example explains how Suspend, Yield, Pause are used. See the comments
// in TaskA and TaskB.
//
// Suspend
//
// Suspend puts a task to sleep until it is scheduled with the Schedule() function,
// or a task sends the suspended task a msg.
//
// Pause(m)
//
// Pause puts a task to sleep for "m" number of system clock ticks, and schedules
// the task to run when "m" clocks have gone by.
//
// Yield
//
// Yield schedules the active task, which adds it to the ready list, then 
// invokes Suspend(), which runs the task at the front of the ready list.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// Define TaskA class
//-----------------------------------------------------------------------------
class TaskAClass : public TaskClass {
public:
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// Define TaskB class
//-----------------------------------------------------------------------------
class TaskBClass : public TaskClass {
public:
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// These will point to instances of TaskA and TaskB.
//-----------------------------------------------------------------------------
TaskAClass * TaskA;
TaskBClass * TaskB;

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    while (true) {
        // Wait for a request to wake up TaskB.
        Wait(RqstMsg);

        // Okay, now send TaskB a wake up msg.
        Send(TaskB, GrantMsg);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    while (true) {

        // Send a msg to TaskA to request a msg to wake us up after the Suspend() call.
        Send(TaskA, RqstMsg);

        // Suspending a task means that it will not run again until someone sends
        // it a msg, or schedules it with Schedule(). Suspend is used only
        // by the Tics system code; users shouldn't use it.
        Suspend();

        // Wait for the GrantMsg from TaskA.
        Wait(GrantMsg);

        // If we get to here, then someone sent us a msg, or scheduled us.
        cout << "Someone woke up TaskB." << endl;

        // Yield() lets other tasks run, after which this task will resume.
        // (That's assuming all tasks and msgs are at the same priority.)
        // Yield() is equivalent to Pause(0).
        Yield();

        // Pause(m) lets other tasks run until "m" clock ticks are up, after
        // which this task is scheduled to run again. Note: The following
        // is only useful for those wanting to learn Tics internals.
        
        // At each task switch the internal Pause tick count is adjusted by
        // reading the number of clock ticks that have occurred since the
        // last task switch. The clock tick value is returned from function 
        // TicsSystem.ReadTickCount(). ReadTickCount() uses the C
        // clock() function to read ticks, but the user can modify the 
        // ReadTickCount() function to read from his own hardware timer.
        //
        // Pause Accuracy
        //
        // Pause is not meant to be used for precise timing. It is good for timing
        // applications that don't require precision (polling a keypad for example).

        cout << "Pausing for 1 second." << endl;

        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Instantiate the tasks.
    TaskA = new TaskAClass();
    TaskB = new TaskBClass();

    // Start tasking.
    Suspend();

    return 0;
}

