//-----------------------------------------------------------------------------
// Copyright (c) 2025, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Requester / Responder
//
// Sample code to show how a requester and responder would be implemented in Tics.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
/*
MIT License

Copyright (c) 2025 Michael Dennis McDonnell

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
// Requester / Responder example.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Msg numbers. All user msg numbers start at 1000. Msg numbers 0-999 are
// reserved by Tics.
//-----------------------------------------------------------------------------
enum {
    RequestMsgNum = 1000, 
    ResponseMsgNum
};

//-----------------------------------------------------------------------------
// Define the Requester Task class - you must inherit from TaskClass and 
// implement the virtual function "Task".
//-----------------------------------------------------------------------------
class RequesterTaskClass : public TaskClass {
public:
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// Define the ResponderTask class - you must inherit from TaskClass and 
// implement the virtual function "Task".
//-----------------------------------------------------------------------------
class ResponderTaskClass : public TaskClass {
public:
    // Functions
    void Task();
};

//-----------------------------------------------------------------------------
// Pointer the tasks. Could be put into a namespace. Just put in global space 
// for this demo.
//-----------------------------------------------------------------------------
RequesterTaskClass* RequesterTask;
ResponderTaskClass* ResponderTask;

//-----------------------------------------------------------------------------
// Dummy msg processing function.
//-----------------------------------------------------------------------------
void ProcessMsg(MsgClass* msg)
{
    switch (msg->MsgNum) {

    case RequestMsgNum:
        // Process the msg.
        cout << "Processing the msg." << "     " << endl;
        break;
    }
}

//-----------------------------------------------------------------------------
// The Requester Task.
//-----------------------------------------------------------------------------
void RequesterTaskClass::Task()
{
    int counter = 0;

    while (true) {

        // Send a request to the responder.
        Send(ResponderTask, RequestMsgNum);

        cout << "Sent request number " << counter << ", waiting for a response." << endl;

        // Wait for the response, then drop it.
        Wait(ResponseMsgNum);

        cout << "Just got a response from request number " << counter++ << endl << endl;

        // Wait a bit for the purposes of this demo.
        Pause(1000);
    }
}

//-----------------------------------------------------------------------------
// The Responder Task.
//-----------------------------------------------------------------------------
void ResponderTaskClass::Task()
{
    MsgClass* msg;

    while (true) {

        // Wait for a Request msg.
        msg = Wait(RequestMsgNum);

        // Call a dummy msg processor. 
        ProcessMsg(msg);

        // Reply back to the sender.
        Reply(msg, ResponseMsgNum, msg->Data);
    }
}

//-----------------------------------------------------------------------------
// Create the tasks and start tasking.
//-----------------------------------------------------------------------------
int main()
{
    // Create the tasks.
    RequesterTask = new RequesterTaskClass();
    ResponderTask = new ResponderTaskClass();

    // Start tasking.
    Suspend();

    // We will never get here.
    return 0;
}

