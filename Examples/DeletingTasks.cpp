//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Deleting Tasks
//
// This example shows how to delete a task. A task cannot delete itself - it can
// only be deleted by another task. (
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>

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
    int loopCounter = 0;
    MsgClass * msg;

    for (;;) {

        // Loop counter.
        cout << "LoopCounter = " << loopCounter++ << endl;

        // Create an instance of TaskB.
        TaskB = new TaskBClass();

        // Write that TaskB was just created by TaskA.
        cout << "TaskB was just created by TaskA" << endl;

        // Send TaskB a msg.
        Send(TaskB, HelloMsg);

        // Send TaskB a msg.
        Send(TaskB, GoMsg);
        Send(TaskB, GoMsg);
        Send(TaskB, GoMsg);

        // Now wait for a response msg named HelloMsg from TaskB.
        msg = Wait(HelloMsg);

        // Print out the data we received.
        cout << "TaskA received a HelloMsg back from TaskB.\n";

        // Delete TaskB. Note: a task must never delete itself - it must be deleted
        // by another task.
        delete TaskB;

        // Let the user know that we deleted TaskB.
        cout << "TaskB was deleted by TaskA.\n\n";

        // Now wait a second.
        Pause(50);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    MsgClass * msg;

    for (;;) {

        // Wait for a msg named HelloMsg (from TaskA).
        msg = Wait(HelloMsg);

        // Reply back to the sender (which is TaskA).
        Reply(msg, HelloMsg);
    }
}

//-----------------------------------------------------------------------------
// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Instantiate the task.
    TaskA = new TaskAClass();

    // Start tasking.
    Suspend();

    return 0;
}

