//-----------------------------------------------------------------------------
// Copyright (c) 2026, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Interrupt Handling
//
// Ideally interrupts should be handled inside the isr, but sometimes this
// isn't possible, and work must be deferred to a task.
//
// A standard Tics Send() msg cannot be sent from within an isr, because the
// interrupt could have occurred in the middle of a linked list update.
// So, from within an isr, interrupt-safe fifo msgs must be used to send
// a msg to a task.
//
// To send a msg to a task from within an isr, the isr sends a fifo msg to
// a waiting task using the Send function variant shown below.
//
// void Send(TaskClass *task, FifoClass *fifo, void *data);
//
// The receiving task waits for the msg from the isr by using the Wait function
// variant shown below.
//
// void TaskClass::Wait(FifoClass * fifo, void * data);
//
// The receiving task must know the class of the data arg so that the fifo data gets
// populated into the class data structure properly.
//
// ** Important things to know about isr behavior **
//
//  On entering an isr, all interrupts must remain disabled until the isr exits. This avoids the
// problem of another interrupt interferring with the current interrupt wjile it is writing
// to the fifo.
//
// Both the isr and the task that handles deferred isr data are written by the user.
//
// The user is responsible for freeing up and data blocks that were used to transfer
// data to the deferred data handling task. For example, if the class instance used
// to send data to the deferred data handling task included a pointer to a dynamic
// data block, then the user must manage freeing it.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>
#include <pthread.h> // Standard Linux threads library
#include <unistd.h>  // Standard Linux utilities header (for sleep)

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// Enums
//-----------------------------------------------------------------------------
enum TimeConstants
{
    OneSecondInMicroseconds = 1000000
};

//-----------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------
void Isr();
void *LinuxThreadWorker(void *argumentPointer);

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
// Tasks and Threads will be instantiated in main().
//-----------------------------------------------------------------------------
TaskBClass *TaskB;
pthread_t LinuxInterruptThread;

//-----------------------------------------------------------------------------
// A native Linux background thread simulating an asynchronous hardware interrupt.
// This loop yanks the execution context completely outside of the Tics scheduler.
//-----------------------------------------------------------------------------
void *LinuxThreadWorker(void *argumentPointer)
{
    while (true)
    {
        // Simulate a real-world asynchronous hardware interrupt trigger
        Isr();

        // Pause the host thread for exactly one second (1,000,000 microseconds)
        usleep(OneSecondInMicroseconds);
    }
    return 0;
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

    // Spawn the native asynchronous Linux thread to act as our hardware interrupt clock.
    // This executes LinuxThreadWorker independently in the background.
    pthread_create(&LinuxInterruptThread, 0, LinuxThreadWorker, 0);

    while (true)
    {
        // Wait for the isr to place the isr data into the fifo after which the Wait() function
        // copies the data from the fifo into the isrData structure.
        Wait(IsrFifo, &isrData);

        // Output the data that we just received from the isr.
        cout << "(" << isrData.A << ", " << isrData.B << ", " << isrData.C << ")" << endl;
    }
}

//-----------------------------------------------------------------------------
// Create tasks, spawn host threads, and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    int fifoSlotSize = sizeof(IsrDataClass);

    // Create a test fifo.
    IsrFifo = new FifoClass(fifoSlotSize, 8);

    // Instantiate the task that processes isr data. Add an optional task name for debugging.
    TaskB = new TaskBClass("TaskB");

    // Start tasking.
    Suspend();

    return 0;
}
