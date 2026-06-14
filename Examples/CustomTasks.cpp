//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Custom Tasks
//
// This example shows how to add data members to a task.
//
// Two instances of the same task are created, each with different instance
// data values.
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
    // Data
    int SampleIntervalInMs;
    int PortNum;

    // Functions
    TaskAClass(const char * name = 0, int sampleIntervalInMs = 1000, int portNum = 0) :
       SampleIntervalInMs(sampleIntervalInMs), PortNum(portNum), TaskClass(name)
    {
    }
    void Task();
};

//-----------------------------------------------------------------------------
// Pointers to the task instances that are created in main().
//-----------------------------------------------------------------------------
TaskAClass * TaskA0;
TaskAClass * TaskA1;

//-----------------------------------------------------------------------------
// Simulate reading a port.
//-----------------------------------------------------------------------------
int ReadPort(int portNum)
{
    // Just return a dummy data value. Here we just return the port number.
    return portNum;
}

//-----------------------------------------------------------------------------
// Implement TaskA.
//-----------------------------------------------------------------------------
void TaskAClass::Task()
{
    int portReading;

    for (;;) {
        // Read the port value.
        portReading = ReadPort(PortNum);

        // Print out the name of the task instance.
        cout << "\n Task instance " << Name << endl;

        // Output the reading.
        cout << "Reading for port " << PortNum << " is: " << portReading << "." << endl;

        // Now pause a bit.
        Pause(SampleIntervalInMs);
    }
}

//-----------------------------------------------------------------------------
// Create tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Sample interval of 1000 ticks (1 second), on port number 2.
    TaskA0 = new TaskAClass("TaskA0", 1000, 2);

    // Sample interval of 4000 ticks (4 seconds), on port number 4.
    TaskA1 = new TaskAClass("TaskA1", 4000, 4);

    // Start tasking.
    Suspend();

    return 0;
}

