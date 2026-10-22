//-----------------------------------------------------------------------------
/// Copyright (c) 2021, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Interrupt Handling
///
/// Ideally interrupts should be handled inside the isr, but sometimes this
/// isn't possible, and work must be deferred to a task.
///
/// A standard Tics Send() msg cannot be sent from within an isr, because the
/// interrupt could have occurred in the middle of a linked list update.
/// So, from within an isr, interrupt-safe fifo msgs must be used to send
/// a msg to a task.
///
/// To send a msg to a task from within an isr, the isr sends a fifo msg to
/// a waiting task using the Send function variant shown below.
///
/// void Send(TaskClass *task, FifoClass *fifo, void *data);
///
/// The receiving task waits for the msg from the isr by using the Wait function
/// variant shown below.
///
/// void TaskClass::Wait(FifoClass * fifo, void * data);
///
/// The receiving task must know the class of the data arg so that the fifo data gets
/// populated into the class data structure properly.
///
/// ** Important things to know about isr behavior **
///
///  On entering an isr, all interrupts must remain disabled until the isr exits. This avoids the
/// problem of another interrupt interfering with the current interrupt while it is writing
/// to the fifo.
///
/// Both the isr and the task that handles deferred isr data are written by the user.
///
/// The user is responsible for freeing up and data blocks that were used to transfer
/// data to the deferred data handling task. For example, if the class instance used
/// to send data to the deferred data handling task included a pointer to a dynamic
/// data block, then the user must manage freeing it.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
/// Namespaces
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
/// Prototypes
//-----------------------------------------------------------------------------
void Isr();

//-----------------------------------------------------------------------------
/// \brief This class defines the data object that will be sent from the isr
///  to the task.
///
/// We purposely create a data structure with an odd number of bytes for
/// testing, which means that each slot after the first one starts on an odd
/// boundary, which is fine because the fifo is completely byte oriented.
//-----------------------------------------------------------------------------
// Force the compiler to pack the class definition on 1 byte boundaries.
#pragma pack(push, 1)

class IsrDataClass
{
  public:
    // Data
    int A;
    int B;
    int C;
    char D;

    // Functions
    IsrDataClass(int a = 13, int b = 14, int c = 15, char d = 16) : A(a), B(b), C(c), D(d) {}
};

// Return to normal packing rules.
#pragma pack(pop)

//-----------------------------------------------------------------------------
/// \brief Define an interrupt data fifo for the isr. We arbitrarily choose to
/// set the number of items that the fifo can hold to 8, but the user can
/// change this.
//-----------------------------------------------------------------------------
FifoClass *IsrFifo;

//-----------------------------------------------------------------------------
/// \brief Define TaskA.
///
/// TaskA will simulate interrupts by periodically calling the isr() function.
//-----------------------------------------------------------------------------
class TaskAClass : public TaskClass
{
  public:
    /// Functions

    TaskAClass(const char *name = 0) : TaskClass(name) {}

    void Task();
};

//-----------------------------------------------------------------------------
/// \brief Define TaskB.
///
/// TaskB will receive the interrupt data from the simulated isr.
//-----------------------------------------------------------------------------
class TaskBClass : public TaskClass
{
  public:
    /// Functions

    TaskBClass(const char *name = 0) : TaskClass(name) {}

    void Task();
};

//-----------------------------------------------------------------------------
/// \brief Task pointers.
///
/// Tasks will be instantiated dynamically in main().
//-----------------------------------------------------------------------------
TaskAClass *TaskA;
TaskBClass *TaskB;

//-----------------------------------------------------------------------------
/// \brief TaskA
///
/// This task simulates an interrupt by calling a simulated isr function
/// once a second.
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
/// \brief The simulated isr function
///
/// This function updates sample data and sends it to TaskB by adding it to
/// the isrFifo. The send function
//-----------------------------------------------------------------------------
void Isr()
{
    /// A data struct to hold the simulated isr readings.
    static IsrDataClass isrData;

    /// Simulate the isr reading new data.
    isrData.A++;
    isrData.B++;
    isrData.C++;
    isrData.D++;

    // Add data to the ISR data fifo that the handler task (TaskB) waits on.
    // The Send() function invokes Schedule(), which pushes a pointer to TaskB
    // into the InterfaceFifo. At the next context switch, Tics processes the
    // InterfaceFifo and moves TaskB to the ReadyList so it can run.
    Send(TaskB, IsrFifo, &isrData);
}

//-----------------------------------------------------------------------------
/// \brief TaskB is the handler that receives the msg from the isr.
///
/// This task processes the isr data that is sent to it from the isr.
//-----------------------------------------------------------------------------
void TaskBClass::Task()
{
    IsrDataClass isrData;

    while (true)
    {
        /// Wait for the isr to arrive.
        Wait(IsrFifo, &isrData);

        /// Output the data that we just received from the isr.
        cout << "(" << isrData.A << ", " << isrData.B << ", " << isrData.C << ", "
             << (unsigned int)isrData.D << ")" << endl;
    }
}

//-----------------------------------------------------------------------------
/// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Get the fifo slot size.
    int fifoSlotSize = sizeof(IsrDataClass);

    // Create a fifo containing 8 slots, each of size fifoSlotSize. The isr will
    // use this fifo to send msgs to TaskB.
    IsrFifo = new FifoClass(fifoSlotSize, 8);

    // Instantiate the interrupt simulator task. Add an optional task name for debugging.
    TaskA = new TaskAClass("TaskA");

    // Instantiate the task that receives the isr data. Add an optional task name for debugging.
    TaskB = new TaskBClass("TaskB");

    // Start tasking.
    Suspend();

    return 0;
}
