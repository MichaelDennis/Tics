//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
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
// Define TaskC class
//-----------------------------------------------------------------------------
class TaskCClass : public TaskClass {
public:
    // Data
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// These will point to instances of TaskA, TaskB, and TaskC.
//-----------------------------------------------------------------------------
TaskAClass * TaskA;
TaskBClass * TaskB;
TaskCClass * TaskC;

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    int i = 0;

    while (true) {

        // Send a print request to TaskB.
        Send(TaskB, RqstMsg, i++);

        // Wait a bit before continuing.
        Pause(500);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//
// TaskB is a front-end for TaskC. TaskC prints a string to the screen.
// 
// While TaskB is waiting for a DoneMsg back from TaskC, TaskB can receive
// other requests. If TaskB's DropUnexpectedMsgsFlag is 1, then the incoming
// requests will be dropped until TaskB gets a DoneMsg from TaskC.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    MsgClass * msg;

    // 1. First run with the function call below commented out as shown.

    // 2. Observe from what TaskC prints out, that not all requests make it to TaskC.
    // The reason for this is that: (1) TaskA is sending requests faster than
    // TaskC can process them, and (2) TaskB's DropUnexpectedMsgsFlag is 1. This means
    // that msgs with  that come in from TaskA while TaskB is waiting for a DoneMsg
    // from TaskC, will be dropped.

    // 3. Uncomment the following function call and notice that all the requests make it to TaskC.
    // You'll also notice that an error eventually occurs, because msgs fill up in TaskB's
    // msg list (queue) faster than TaskC can process them.

    // 4. To turn msg dropping back on, call DropUnexpectedMsgs(true). 
    
    // If we're waiting for msgA and we get msgB, then drop it.
    SetFlag(DropUnexpectedMsgsFlag);

    while (true) {

        cout << endl << endl << "Num msgs in TaskB msg list is: " << MsgList.NumNodesInList << endl << endl;

        // Wait for a print request.
        msg = Wait(RqstMsg);

        // Forward the request to TaskC for printing.
        Send(TaskC, RqstMsg, msg->Data);

        // Wait for TaskC to finish.
        Wait(DoneMsg);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskC
//
// TaskC takes requests to print a string to the screen.
//-----------------------------------------------------------------------------
void TaskCClass::Task()
{
    MsgClass * msg;
    TaskClass * sender;

    while (true) {

        // Wait for a print request.
        msg = Wait(RqstMsg);

        // Save the Sender so that we can reply later. Note that we cannot use the Reply
        // function because the msg will be recycled when this task relinquishes control
        // when the Pause() function is called (see below).
        sender = msg->Sender;

        // Print the msg to the screen.
        cout << "This is request number " << msg->Data << endl;

        // Simulate TaskC doing work.
        Pause(1000);

        // Reply back to the sender (which is TaskB).
        Send(sender, DoneMsg);  
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
    TaskC = new TaskCClass();

    // Start tasking.
    Suspend();

    return 0;
}

