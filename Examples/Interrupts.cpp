//-----------------------------------------------------------------------------
// Copyright (c) 2021, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Interrupt Handling
//
// Ideally you should handle interrupts inside the isr, but sometimes this
// isn't possible, and work must be deferred to a task.
//
// To send a msg to a task, the isr sends a fifo msg to a waiting task using the Send function
// variant shown below.
//
// void TicsNameSpace::Send(TaskClass * task, FifoClass * fifo, void * data);
//
// The receiving task waits for the msg from the isr by using the Wait function variant shown below.
//
// void TaskClass::Wait(FifoClass * fifo, void * data);
//
// ** Important things to know about isr behavior **
//
// There must be a separate isr fifo for each isr / task pair. This is best explained by example.
//
// Let's assume we have one isr that handles 3 separate events. The isr decides to defer event
// handling for each event to 3 separate tasks. To accomplish this, we must also create 3 separate
// fifos, one for each task. So, for example, when event1 is received by isrA, isrA would issue this
// call: Send(task1, fifo1, data1). And similarly, when event2 is received by isrA, isrA would issue
// this call: Send(task2, fifo2, data2). And similarly for event3.
//
// This also means that only isrA can use fifo1, fido2, and fifo3. If isrB wants
// to send msgs to a task, isrB must create its own personal fifo, one for each task that isrB needs
// to talk to. Note also that a task that expects a msg from an isr must know the name of the fifo
// to wait on with the Wait function. The task must also know the type of the void data pointer so
// that it can properly cast the data.
//
// However, the isr does not have to have a fifo for each event. It could just have one fifo that is
// serviced by one task. The data could then point to a struct that has the event number as the
// first entry. The following entries in the struct would depend upon the event number. The task
// could then dispatch the data to the appropriate task (the task that can handle the event). So in
// this example, we have a dispatcher task
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
// Prototypes
//-----------------------------------------------------------------------------
void Isr();

//-----------------------------------------------------------------------------
// Test code
//-----------------------------------------------------------------------------

void printReadyList(const char *func)
{
    int numNodes = ReadyList.NumNodesInList;
    MsgClass *msg = 0;

    // Print num items in the ReadyList.
    cout << endl << endl << func << " " << "Num items in ReadyList is " << numNodes << endl << endl;

    // Get a pointer to the first msg in the list.
    msg = (MsgClass *)ReadyList.Head->Next;

    // Print out each msg in the ReadyList.
    for (int i = 0; i < numNodes; i++)
    {
        if (msg->Receiver->Name != 0)
        {
            cout << "Node " << i << " Receiver task = " << msg->Receiver->Name << endl;
        }

        msg = (MsgClass *)msg->Next;
    }
}

//-----------------------------------------------------------------------------
// This class defines the data object that will be sent from the isr to the task.
//-----------------------------------------------------------------------------
class IsrDataClass
{
  public:
    // Data
    int A;
    int B;
    int C;

    // Functions
    IsrDataClass(int a = 13, int b = 14, int c = 15) : A(a), B(b), C(c) {}
};

//-----------------------------------------------------------------------------
// Define an interrupt data fifo for the isr. We arbitrarily
// choose to set the number of items that the fifo can hold to 8,
// but the user can change this.
//-----------------------------------------------------------------------------
FifoClass *IsrFifo;

//-----------------------------------------------------------------------------
// Define TaskA, which will simulate interrupts buy periodically
// calling the isr() function.
//-----------------------------------------------------------------------------
class TaskAClass : public TaskClass
{
  public:
    // Functions

    TaskAClass(const char *name = 0) : TaskClass(name) {}

    void Task();
};

//-----------------------------------------------------------------------------
// Define TaskB, which will receive the interrupt data from the isr.
//-----------------------------------------------------------------------------
class TaskBClass : public TaskClass
{
  public:
    // Functions

    TaskBClass(const char *name = 0) : TaskClass(name) {}

    void Task();
};

//-----------------------------------------------------------------------------
// Tasks will be instantiated in main().
//-----------------------------------------------------------------------------
TaskAClass *TaskA;
TaskBClass *TaskB;

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    for (;;)
    {
        // Simulate an interrupt by calling the isr function.
        Isr();

        // Pause for a bit.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Interrupt handler. The raw interrupt vector (jumps) directly to this function.
//-----------------------------------------------------------------------------
void Isr()
{
    static IsrDataClass isrData;

    // Simulate the isr reading data.
    isrData.A++;
    isrData.B++;
    isrData.C++;

    // Add the isr data to the isr fifo, and schedule TaskB.
    Send(TaskB, IsrFifo, &isrData);
}

//-----------------------------------------------------------------------------
// Implement TaskB.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    IsrDataClass p;
    int lastValue = 0;

    for (;;)
    {
        cout << "Number of fifo items = " << IsrFifo->NumItemsInFifo << endl << endl;

        // Wait for the isr to place the data object into the fifo.
        Wait(IsrFifo, &p);

        if (IsrFifo->NumItemsInFifo >= 1)
        {
            cout << "(" << p.A << ", " << p.B << ", " << p.C << ")" << endl;
            lastValue = p.A;
        }
    }
}

//-----------------------------------------------------------------------------
// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    int n = sizeof(IsrDataClass);

    // Create the isr fifo.
    IsrFifo = new FifoClass(n, 8);

    // Instantiate the interrupt simulator task.
    TaskA = new TaskAClass("TaskA");

    // Instantiate the isr task.
    TaskB = new TaskBClass("TaskB");

    // Start tasking.
    Suspend();

    return 0;
}
