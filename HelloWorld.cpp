/*
MIT License

Copyright (c) 2019 Michael Dennis McDonnell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
    Suspend();

    // We will never get here.
    return 0;
}

