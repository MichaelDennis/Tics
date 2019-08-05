//-----------------------------------------------------------------------------
// Copyright (c) 2019, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Hello World
//
// Simple program to print "Hello World!" once a second.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "tics.hpp"
#include <iostream>
#include <conio.h>

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
using namespace std;

//-----------------------------------------------------------------------------
// Hello world example.
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
// Pointer the task. Used in main() to create the task, and also to send a
// msg to the task.
//-----------------------------------------------------------------------------
HelloTaskClass * HelloTask;

//-----------------------------------------------------------------------------
// Implement the Hello Task function.
//-----------------------------------------------------------------------------
void HelloTaskClass::Task()
{
    // The task body is always an infinite loop.

    for (;;) {

        // Write to the screen.
        cout << "Hello World! ";

        // Now wait a second before we write again.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Create HelloTask and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Create the task.
    HelloTask = new HelloTaskClass();

    // Start tasking.
    Yield();

    // We will never get here.
    return 0;
}

