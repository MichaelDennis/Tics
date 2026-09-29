/*
MIT License

Copyright (c) 2026 Michael Dennis McDonnell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//-----------------------------------------------------------------------------
// Test World
//
// Simple program to print "Test World!" once a second.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Tics.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
/// Namespaces - std
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
/// Namespaces - TicsNameSpace
//-----------------------------------------------------------------------------
using namespace TicsNameSpace;

//-----------------------------------------------------------------------------
// Test  example.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define the Test Task class - you must inherit from TaskClass and
// implement the virtual function "Task".
//-----------------------------------------------------------------------------
class TestTaskClass : public TaskClass
{
  public:
    // Data

    // Functions
    TestTaskClass(const char *name) : TaskClass(name) {}
    void TestMsgPriorities();
    void BlowUpStack();
    void Task();
};

//-----------------------------------------------------------------------------
// Pointer to the Test task. Used in main() to create the Test task.
//-----------------------------------------------------------------------------
TestTaskClass *TestTask;

//-----------------------------------------------------------------------------
// Intentionally forces an infinite recursion loop to deliberately blow up the
// task's stack memory boundary layout for hardware failure validation.
//-----------------------------------------------------------------------------
void BlowUpTheStack()
{
    // Allocate a localized memory array payload buffer to accelerate the stack burn rate.
    volatile uint32_t stackSmasherBuffer[16];

    // Force a dummy compiler read-write modification check onto the memory array.
    stackSmasherBuffer[0] = 0xDEADBEEFUL;

    // Execute an unbroken recursive step call to drive the stack pointer past its limit.
    BlowUpTheStack();
}

//-----------------------------------------------------------------------------
// Test Tasks
//-----------------------------------------------------------------------------

void LogError() {}

void TestTaskClass::TestMsgPriorities()
{
    int msgPriorites[] = {1, 2, 3, 4, 5};
    int numMsgs = sizeof(msgPriorites) / sizeof(int);
    MsgClass *msg;

    // Call the Blowup test.

    // Send out msgs.
    for (int i = 0; i < numMsgs; i++)
    {
        // Send out 5 different msgs each at a higher priority that the last.
        Send(this, StartMsg, i, 0, msgPriorites[i]);
    }

    // Retrieve the msgs. We expect data values of 4, 3, 2, 1, 0 in that order.
    for (int i = 0; i < numMsgs; i++)
    {
        // Get the next msg.
        msg = Wait(StartMsg);

        // Check for the expected priority.
        if (msg->Priority != msgPriorites[numMsgs - i - 1])
        {
            // If we don't hav a match, then priorites are not working.
            LogError();
        }
    }
}

//-----------------------------------------------------------------------------
// Implement the Test Task function.
//-----------------------------------------------------------------------------
void TestTaskClass::Task(void)
{
    // The task body is always an infinite loop.
    while (true)
    {
        // Wait for a start testing msg.
        Wait(StartMsg);

        TestTask->TestMsgPriorities();
    }
}

//-----------------------------------------------------------------------------

int main()
{
    // Create the test object.
    TestTask = new TestTaskClass("TestTask");

    // Blowup the stack.
    // TestTask->BlowUpStack();

    // Create the Test task.
    TestTask = new TestTaskClass("Test");

    // Send a msg to get things started.
    TicsSystemTask.Send(TestTask, StartMsg);

    // Start tasking.
    Suspend();

    // We will never get here.
    return 0;
}
