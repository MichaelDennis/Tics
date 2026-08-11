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
// A task is scheduled to run by sending it a msg (the Send() function adds
// the receiving task to the ReadyList).
//
// The Schedule() function is for Tics internal us only. It adds a msg 
// named ScheduleMsg to the ReadyList. The Suspend() function does 
// add msgs named ScheduleMsg  
// is for Tics internal use only. It adds a msg to the ReadyList
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

        // 1. Schedule() is for Tics internal use only. It's shown here in the interest
        // completeness. Generaly, user's should never call Schedule. Schedule sends
        //  a msg to TaskB with a msg number of ScheduleMsg (i.e. it is added to the
        // ReadyList. The msg number ScheduleMsg has a special meaning to Tics, which is, 
        // Tics will run TaskB, but will not place the msg in TaskB's msg list. So,
        // that Schedule() is a convenient way to run a task, without requiring that
        // the task be burdened with processing a do-nothing msg. When a task is
        // first created Tics uses Schedule() to start the task. If Schedule()
        // didn't exist, then Tics would have to send a newly created task
        // a dummy msg to start it, and the task would have to pull the msg out of 
        // its msg list and just ignore it and let garbage collection dispose
        // of it.
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
        Pause(1000);
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

