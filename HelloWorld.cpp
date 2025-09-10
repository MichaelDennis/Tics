
//-----------------------------------------------------------------------------
// Copyright (c) 2024, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Hello World
//
// Simple program to print "Hello World!" once a second.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
/*
MIT License

Copyright (c) 2024 Michael Dennis McDonnell

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

#include "Tics.hpp"
#include "TicsTaskSwitch.hpp"
#include <iostream>

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// Hello World example.
// 
// The purpose of this Hello World example is to:
// 
// 1. Show how tasks are created and used in the Tics real-time operating system. The
// Tics RTOS is a simple message based RTOS written in C++. It provides for multi-tasking,
// msg passing, msg waiting, and timer management.
// 
// 2. Present a simple Hello World program that prints a msg to the screen once per second.
// 
// This example does the following:
// 
// 1. Creates a task class named HelloTaskClass. All task classes must inherit
// from the base task class named TaskClass, that is provided by the Tics RTOS.
// 
// 2. All tasks must implement the TaskClass member function named Task(),
// which is the task ("thread") that controls the task class.
// 
// 3. The HelloTaskClass implements the Task() function as a simple task that
// prints a msg to the screen once each second.
// 
// 4. The 1-second is measured using the Tics RTOS function named Pause().
// 
// 5. The task is instantiated in the main() function like this:
// HelloTask = new HelloTaskClass();
// 
// 6. Instantiating the HelloTaskClass() adds a pointer to the task instance,
// to the Ready List, which is a doubly linked list of tasks waiting to run.
// of tasks waiting to run. HelloTask is added to the list according to its priority.
// All tasks are created with a default priority that can be changed.
// 
// 7. In the main() function, the Tics RTOS function named Suspend() is called
// to start the task after the task has been created (using the "new" operator).
// The Suspend() function suspends the currently running
// task, and runs the next task in the Ready List queue. The HelloTask is
// in the Ready List queue because it was added to the ReadyList when it was created
// with the C++ "new" operator.
// 
// 8. When the HelloTask runs it calls the Tics RTOS Pause() function to put
// the HelloTask to sleep for 1 second (1000 ticks).
// 
// 9. Since the HelloTask is asleep, and it is the only user task, the IdleTask() runs.
// 
// 10. After 1 second, the HelloTask is resumed, and continues where it left off.
// 
// A separate section will be added later to explain how task switching works.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define the Hello Task class - you must inherit from TaskClass and 
// implement the virtual function "Task".
//-----------------------------------------------------------------------------
class HelloTaskClass : public TaskClass {
public:
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// Pointer the task. Used in main() to create the task.
//-----------------------------------------------------------------------------
HelloTaskClass * HelloTask;

//-----------------------------------------------------------------------------
// Implement the Hello Task function.
//-----------------------------------------------------------------------------
void HelloTaskClass::Task(void)
{
    int i = 0;

    int oneSecond = CLOCKS_PER_SEC;

    // The task body is always an infinite loop.

    for (;;) {

        // Write to the screen.
        cout << "Hello World! " << i++ << endl;

        // Now wait a second before we write again.
        Pause(oneSecond);
    }
}

class MyClass {
    public:
    int a;
    int b;
    int c;
};

MyClass My;

//-----------------------------------------------------------------------------
// Create HelloTask and start tasking.
//-----------------------------------------------------------------------------
int main()
{
     // Create the hello task.
    HelloTask = new HelloTaskClass();

    My.a = 1;
    My.b = 2;
    My.c = 3;

    // Start tasking.
    Suspend();

    // We will never get here.
    return 0;
}

