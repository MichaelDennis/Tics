//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Sample code that times out if a msg is not responded to in certain
// period of time.
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

    // Constructor
    TaskAClass(const char* name = 0) : TaskClass(name) 
    {
    }
};

//-----------------------------------------------------------------------------
// Define TaskB class
//-----------------------------------------------------------------------------
class TaskBClass : public TaskClass {
public:
    // Data
    enum { LenSleepTimes = 4 };
    // Functions
    void Task();
    // Constructor
    TaskBClass(const char* name = 0) : TaskClass(name) 
    {
    }
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
    MsgClass * msg;
    int helloMsgId;
    int timeoutMsgId;
    int counter = 0;

    while (true) {
        // Send TaskB a msg and save the msg id, which is not the msg number. The msg number is HelloMsg.
        helloMsgId = Send(TaskB, HelloMsg)->Id;

        // Send ourselves (this) a delayed msg, and save the msg id. 
        timeoutMsgId = Send(this, TimeoutMsg)->Id;

        // Now wait for any msg, regardless of its msg number.
        msg = Wait();

        switch (msg->MsgNum) {

            case HelloMsg:
            // Okay, we received a response back from TaskB, so cancel the timeout msg.
            Cancel(msg, timeoutMsgId);
            cout << "HelloMsg- received from TaskB. Canceled timeout msg." << endl;
            break;

            case TimeoutMsg:
            // Okay, we timed out, so cancel the hello msg.
            Cancel(msg, helloMsgId);
            cout << "*** Timeout msg received in TaskA. Canceled hello msg." << endl;
            break;
        }

        // Print the msg count in all msg lists.
        cout << "counter = " << counter++ << endl;
        cout << "DelayList numNodes = " << DelayList.NumNodesInList << endl;
        cout << "ReadyList numNodes = " << ReadyList.NumNodesInList << endl;
        cout << "TaskA MsgList numNodes = " << MsgList.NumNodesInList << endl;
        cout << "TaskB MsgList numNodes = " << TaskB->MsgList.NumNodesInList << endl;

         // Wait a bit so that we don't overflow the msg list.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    MsgClass * msg;

    for (int i = 0; ; i++) {

        // Wait for a msg named HelloMsg (from TaskA).
        msg = Wait(HelloMsg);

        // Do not reply on every nth msg, just for variety.
        if (i % 10 == 0) {
            Pause(2000);
            continue;
        }

        // Reply back to the sender (which is TaskA).
        Reply(msg, HelloMsg);

        // If the msg queue is filling up, flush it.
        if (MsgList.NumNodesInList > 10) {
            // Clear the msg list.
            MsgList.Flush();
            cout << "TaskB MsgList Flushed!" << endl;
        }
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

