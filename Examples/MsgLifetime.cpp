//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Msg Lifetime
//
// This example shows that a received msg is valid until the task is suspended.
//
// A task is suspended in any of the following circumstances:
// 
// (1) The Wait() function is called and the msg waited for is not available, or
// (2) The Pause() function is called, which suspends the current task for a number of clock ticks, or
// (3) The Yield() function is called, or,
// (4) The Suspend() function is called directly. 
// 
// In this example, TaskA sends TaskB a msg, then expects a reply. TaskB
// can reply to TaskA by using the Reply() function, or by using
// Send() by sending a msg to msg->Sender, however, msg is only valid
// while in the task that received the msg is active. When the task relinquishes 
// control by any of the four methods shown above, control will be returned
// to Tics, and Tics will delete the msg. When the task resumes after
// invoking any of the 4 methods listed above, the msg will not be valid
// meaning that msg->Sender will not be available. See the code in TaskB
// for better understanding.
// 
// 
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
// Msg Lifetime
//
// Shows how long a msg is valid.
//-----------------------------------------------------------------------------

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

        // Send TaskB a msg.
        Send(TaskB, HelloMsg);

        // Now wait for a response msg named HelloMsg from TaskB.
        Wait(HelloMsg);

        // Print something to the screen.
        cout << "In TaskA, just received response back from TaskB." << endl;

        // Now wait a second.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    MsgClass * msg;
    TaskClass * sender;

    while (true) {

        // Wait for a msg named HelloMsg (from TaskA).
        msg = Wait(HelloMsg);

        // Save the Sender of the msg.
        sender = msg->Sender;

        // The following call returns back to Tics, which deletes the msg pointed
        // to by the "msg" variable used in the above statement. In fact, any Tics 
        // system call that results in the task going into a suspended mode, will 
        // result in the msg being deleted and recycled.
        Pause(1000);

        // Now the Reply() function call below will not work, since msg is invalid 
        // after returning from the Pause call above.
        // Reply(msg, HelloMsg);

        // The Send call below won't work either, for the same reason.
        // Send(msg->Sender, HelloMsg);

        // However, the call below will work, since we're using the sender that was saved
        // when the msg was valid. Normally, this is unnecessary, since you'll typically
        // be using a msg when it is still valid. 
        Send(sender, HelloMsg);
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

