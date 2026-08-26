//-----------------------------------------------------------------------------
// Copyright (c) 2021, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Interrupt Handling
//
// Ideally you should handle interrupts inside the isr, but sometimes this
// isn't possible, and work must be deferred to a task.
//
// To send a msg to a task, the isr sends a fifo msg to a waiting task using the
// Send function variant shown below.
//
// void Send(TaskClass *task, FifoClass *fifo, void *data);
//
// The receiving task waits for the msg from the isr by using the Wait function
// variant shown below.
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
// Define TaskA, which will simulate interrupts by periodically
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
// Define TaskB, which will receive the interrupt data from the simulated isr.
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
// This task simulates an interrupt by calling a simulated isr once a second.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    while (true)
    {
        // Simulate an interrupt by calling the isr function.
        Isr();

        // Pause for one second.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// A simulated isr.
//-----------------------------------------------------------------------------
void Isr()
{
    // A data struct to hold the simulated isr readings.
    static IsrDataClass isrData;

    // Simulate the isr reading new data.
    isrData.A++;
    isrData.B++;
    isrData.C++;

    // Add the isr data to the isr fifo, which is the fifo that the isr data processing
    // task (TaskB in this example) waits on. See TaskB.
    Send(TaskB, IsrFifo, &isrData);
}

//-----------------------------------------------------------------------------
// Implement the task that processes the isr data that is sent to it from
// the isr. See the Isr() function above.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    IsrDataClass isrData;

    while (true)
    {
        // Wait for the isr to place the isr data into the fifo after which the Wait() function
        // copies the data from thhe fifo into the isrData structure.
        Wait(IsrFifo, &isrData);

        // Output the data that we just received from the isr.
        cout << "(" << isrData.A << ", " << isrData.B << ", " << isrData.C << ")" << endl;
    }
}

//-----------------------------------------------------------------------------
// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    int fifoSlotSize = sizeof(IsrDataClass);

    // Create a test fifo.
    IsrFifo = new FifoClass(fifoSlotSize, 8);

    // Instantiate the interrupt simulator task. Add an optional task name for debugging.
    TaskA = new TaskAClass("TaskA");

    // Instantiate the task that processes isr data. Add an optional task name for debugging.
    TaskB = new TaskBClass("TaskB");

    // Start tasking.
    Suspend();

    return 0;
}
