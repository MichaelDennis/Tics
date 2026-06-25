//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
// Task Scheduling
//
// A task is scheduled to run in two ways:
//
// 1. By sending it a msg.
// 2. By scheduling it by calling Schedule(). (Mostly for Tics internal use only.)
//
// The difference is that for case 1, the task receives a msg, and for case
// 2, it does not. So if the task is waiting for a msg to arrive,
// it will never get past the Wait() call if it is scheduled with Schedule(),
// you'd have to send it a msg to get it past the Wait() call.
//
// However, if a task calls Suspend(), it can be waken up by scheduling
// it by calling Schedule().
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

        // Three ways to schedule tasks.

        // 1. Call Schedule(). This will schedule the task to run (by adding it to the Ready List).
        Schedule(TaskB);

        // 2. Send the task a msg. This will schedule the task to run AND
        // put the msg into the task's msg list prior to switching to it.
        Send(TaskB, HelloMsg);

        // 3. Send the task a msg named ScheduleMsg. This will schedule the task to run,
        // but will not put the msg into the task's msg list prior to switching to it. 
        // ScheduleMsg has special meaning to Tics, which is that the task will be
        // scheduled, but the ScheduleMsg will not be put into the task's msg list
        // prior to switching to it.
        Send(TaskB, ScheduleMsg);

        // Wait a bit.
        Pause(100);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    int counter = 0;

    while (true) {

        // Drop all the msgs we receive.
        Wait();

        // Let the user know that a msg was received.
        cout << "TaskB just received a msg..." << counter++ << endl;
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

