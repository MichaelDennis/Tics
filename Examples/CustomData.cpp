//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Another example of creating custom msg fields. See the comments for details.
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
// Talking tasks with Custom Data
//
// Inter-task communication example with a custom msg.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define custom msg class.
//-----------------------------------------------------------------------------
class CustomMsgClass : public MsgClass {
public:
    // Data
    int X;
    int Y;
    int Z;

    // Functions
    CustomMsgClass(TaskClass * task, int msgNum = HelloMsg, int x = 0, int y = 0, int z = 0) :
        MsgClass(task, msgNum), X(x), Y(y), Z(z)
    {
        // Nothing to do here, since the necessary work is done in the initialization list.
    }
};

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
// TaskA and TaskB will be instantiated in main().
//-----------------------------------------------------------------------------
TaskAClass * TaskA;
TaskBClass * TaskB;

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    int counter = 0;
    int x, y, z;
    CustomMsgClass * msg;

    while (true) {

        // Set the msg data.
        x = counter++;
        y = counter++;
        z = counter++;

        // Create a custom msg.
        msg = new CustomMsgClass(TaskB, HelloMsg, x, y, z);
        
        // Send TaskB the msg.
        Send(msg);

        // Now wait for a response msg from TaskB. We really don't
        // care what the msg number is, so we don't specify an arg to Wait(), which means
        // that we will resume on receiving any msg.
        Wait();

        // Now wait a second.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    CustomMsgClass * msg;

    while (true) {

        // Wait for a msg named HelloMsg (from TaskA).
        msg = (CustomMsgClass *) Wait(HelloMsg);

        // Print out the data we received.
        cout << endl << "TaskB received x,y,z data of (" 
            << msg->X << ", " << msg->Y << ", " << msg->Z << ")." << endl;

        // Reply back to the sender.
        Reply(msg, HelloMsg);
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

