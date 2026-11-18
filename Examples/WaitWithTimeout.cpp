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
class TaskAClass : public TaskClass
{
  public:
    // Data
    enum
    {
        TimeoutValue = 5000
    };

    // Functions
    void Task();

    // Constructor
    TaskAClass(const char *name = 0) : TaskClass(name) {}
};

//-----------------------------------------------------------------------------
// Define TaskB class
//-----------------------------------------------------------------------------
class TaskBClass : public TaskClass
{
  public:
    // Data
    enum
    {
        LenSleepTimes = 4
    };
    // Functions
    void Task();
    // Constructor
    TaskBClass(const char *name = 0) : TaskClass(name) {}
};

//-----------------------------------------------------------------------------
// These will point to instances of TaskA and TaskB.
//-----------------------------------------------------------------------------
TaskAClass *TaskA;
TaskBClass *TaskB;

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    MsgClass *msg;
    int helloMsgId;
    int timeoutMsgId;
    int counter = 0;
    TimerTickType startingTickCount;
    TimerTickType endingTickCount;
    TimerTickType dt;

    while (true)
    {
        // Return value from calling Cancel().
        bool CancelStatus = true;

        // Send TaskB a msg and save the msg id, which is not the msg number. The msg number is
        // HelloMsg.
        helloMsgId = Send(TaskB, HelloMsg)->Id;

        // Send ourselves (this) a delayed msg, and save the msg id.
        timeoutMsgId = Send(this, TimeoutMsg, 0, TimeoutValue)->Id;

        // Get the starting tick count.
        startingTickCount = ReadTickCount();

        // Now wait for any msg, regardless of its msg number.
        msg = Wait();

        switch (msg->MsgNum)
        {
        case HelloMsg:
            // Okay, we received a response back from TaskB, so cancel the timeout msg.
            CancelStatus = Cancel(msg, timeoutMsgId);
            cout << "HelloMsg- received from TaskB. Canceled timeout msg. Status = " << CancelStatus
                 << endl;
            break;

        case TimeoutMsg:
            // Okay, we timed out, so cancel the hello msg.
            CancelStatus = Cancel(msg, helloMsgId);
            cout << "*** Timeout msg received in TaskA. Canceled hello msg. Status = "
                 << CancelStatus << endl;
            break;
        }

        // Print the msg count in all msg lists.
        cout << "counter = " << counter++ << endl;
        cout << "DelayList numNodes = " << DelayList.NumNodesInList << endl;
        cout << "ReadyList numNodes = " << ReadyList.NumNodesInList << endl;
        cout << "TaskA MsgList numNodes = " << MsgList.NumNodesInList << endl;
        cout << "TaskB MsgList numNodes = " << TaskB->MsgList.NumNodesInList << endl;

        // Get the dt between the starting and ending tick counts.
        endingTickCount = ReadTickCount();

        // Calculate the dt between the starting and ending tick counts.
        dt = endingTickCount - startingTickCount;

        // Print the dt in milliseconds.
        cout << "dt = " << dt << " ms" << endl;

        // Wait a bit so that we don't overflow the msg list.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// \brief This task receives a msg from TaskA and replies back to it. It will
// ignore every 10th msg, which will cause TaskA to timeout and cancel the
// HelloMsg that it sent to TaskB. This is a test to see if the Cancel() function
// works properly.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    MsgClass *msg;

    for (int i = 0;; i++)
    {
        // Wait for a msg named HelloMsg (from TaskA).
        msg = Wait(HelloMsg);

        // Do not reply on every 10th msg. This will force a timeout,
        // because TaskA is waiting for a reply, it will timeout and cancel the HelloMsg.
        // This is a test to see if the Cancel() function works properly.
        if (i % 10 == 0)
        {
            // Just wait a bit to simulate a long processing time, but do not reply to the msg.
            Pause(2000);

            // By continuing we are ignoring the HelloMsg from TaskA
            // which is a request to reply back to TaskA. This will cause TaskA to timeout.
            continue;
        }

        // Reply back to the sender (which is TaskA).
        Reply(msg, HelloMsg);

        // If the msg queue is filling up, flush it.
        if (MsgList.NumNodesInList > 10)
        {
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
