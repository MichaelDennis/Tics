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
// Tics Test Suite File
//
// This file contains various code that tests the Tics RTOS.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define the test classes.
//-----------------------------------------------------------------------------

class TicsTestClass : public TicsBaseClass
{
  public:
    // Data

    // Functions

    // Constructor.
    TicsTestClass() {}
};

//-----------------------------------------------------------------------------
// Pointer the hello task. Used in main() to create the hello task.
//-----------------------------------------------------------------------------
HelloTaskClass *HelloTask;

//-----------------------------------------------------------------------------
// Implement the Hello Task function.
//-----------------------------------------------------------------------------
void HelloTaskClass::Task(void)
{
    // Counter initialization.
    int i = 0;

    // The task body is always an infinite loop.

    while (true)
    {
        // Output the string "Hello World!World.cpp" followed by a counter value.
        cout << "Hello World! " << i++ << endl;

        // Sleep for one second.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// Create HelloTask and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Create the hello task.
    HelloTask = new HelloTaskClass("Hello");

    // Start tasking.
    Suspend();

    // We will never get here.
    return 0;
}
