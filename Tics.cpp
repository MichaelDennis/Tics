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

//-----------------------------------------------------------------------------
//                                    
// Copyright (c) 2024, Tics Realtime (Michael McDonnell)
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/// \brief A basic overview of the operation of the Tics RTOS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <time.h>
#include <stdlib.h>
#include "Tics.hpp"
#include "TicsTaskSwitch.hpp"
#include <iostream>

using namespace std;

//-----------------------------------------------------------------------------
// Globals, externs, and statics.
//-----------------------------------------------------------------------------
// Initialize the Tics object instance counter.
int TicsBaseClass::IdCounter = 0;


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
namespace TicsNameSpace {

    // Data

    // Msgs are created by allocating a memory block from this area.
    // An instance of MemAiMgrTaskClass class is created to manage this space.
    // (See the definition of MemoryMgr below).
    int MemoryMgrSpace[SizeMemoryMgr / sizeof(int)];

    // Create an instance of MemAiMgrTaskClass to allow for allocation and
    // deallocation of memory blocks using the space provided by
    // MemoryMgrSpace (defined above). You can think of the MemAiMgrTaskClass
    // as being similar to malloc(), with member functions to allocate and
    // deallocate memory. A chunk of memory needs to be provided to the
    // MemAiMgrTaskClass constructor, which creates a pool from which
    // memory blocks of various sizes are allocated and deallocated. 
    MemAiMgrTaskClass MemoryMgr(MemoryMgrSpace, SizeMemoryMgr);

    // Various flags used by Tics.
    FlagsClass TicsFlags(SafeModeFlag);

    // List of tasks waiting to run.
    MsgListClass ReadyList;

    // List of tasks that currently exist in the system (whether they are in the ReadyList or not).
    TaskListClass TaskList;

    // List of msgs that will be sent out after so many clock ticks.
    // A task is put to sleep (see Pause() or StartTimer()) by the task 
    // sending itself a delayed msg, then waiting for the msg, which
    // suspends the task until a msg is sent to it.
    DelayListClass DelayList;

    // List of msgs that are marked for deletion. Msgs are valid while in the task that
    // was waiting for the msg. Once the task relinquishes control, Tics deletes the msg.
    MsgListClass DeleteList;

    // A task that Tics maintains for its own use.
    TicsSystemTaskClass TicsSystemTask;

    // Pointer to the task that is currently running.
    TaskClass* CurrentTask = 0;

    // This task runs when no other tasks are ready to run (it's priority is lower than any user or system task).
    IdleTaskClass IdleTask;
    
    // Isr's schedule tasks to run by adding them to this fifo. 
    FifoClass InterruptFifo(sizeof(TaskClass*), NumInterruptFifoSlots);
 
    // All errors are handled by calling ErrorHandler.Report().
    ErrorHandlerClass ErrorHandler;

    // The Isr Handler Table is scanned on each context switch to see if an interrupt is pending, and if so, the handler is called.
    // The check can be disabled if interrupts are not used (our recommendation).
    // Note: The InterruptTable is not used. It is kept here for possible future use.
    InterruptTableRowClass InterruptTable[] = {
        // Each row defines the args for the constructor of FifoClass. The following row is an example and you must replace it.
        // Note that the second and third args default to the values shown, so no need to specify them unless you want to override. 
        {InterruptFifoSlotSize, NumInterruptFifoSlots, 0}
    };

    // TicsNameSpace functions.
    TimerTickType ReadTickCount();
    void CheckForSystemEvents();
    void CheckForInterrupts();
    void CheckForInterrupts2();
    void Schedule(TaskClass* task, bool inIsr = false);
    void Send(TaskClass * task, FifoClass * fifo, void * data);
};

//-----------------------------------------------------------------------------
// Namespaces in use.
//-----------------------------------------------------------------------------
using namespace TicsNameSpace;


//-----------------------------------------------------------------------------
/// \brief StackClass constructor. Allocates stack space and defines the stack protective pad.
///
/// Allocate memory for the stack memory pool, and define the protective "pad"
/// area at the bottom of the stack, which is used to detect pending stack overflow.
/// All task stack memory is allocated from this pool.
///
/// \param stackSizeInBytes - The total number of bytes to reserve for the 
/// stack memory pool.
///
/// \param stackPadSizeInBytes - The pad is an area at the bottom of stack memory. 
/// Any writes to this area will generate an error.
//-----------------------------------------------------------------------------

StackClass::StackClass(int stackSizeInBytes, int stackPadSizeInBytes)
{
    // SavedSp is used to save the SP of the task being switched out.
    SavedSp = 0;

    // Since the default stack size may be changed by the user, we check it here.
    if (DefaultStackSizeInBytes > MaxStackSizeInBytes ||
        DefaultStackSizeInBytes < MinStackSizeInBytes) {
        ErrorHandler.Report(ErrorDefaultStackSizeOutOfRange);
    }

    // Check the stack size parameter and correct it if necessary.
    if (stackSizeInBytes > MaxStackSizeInBytes ||
        stackSizeInBytes < MinStackSizeInBytes) {
        stackSizeInBytes = DefaultStackSizeInBytes;
    }

    // Get the stack size in multiples of sizeof(StackType).
    StackSizeInBytes = (stackSizeInBytes / (int) sizeof(StackType)) * (int) sizeof(StackType);

    // Set the pad size. The pad is a low water mark at the bottom of the stack.
    StackPadSizeInBytes = stackPadSizeInBytes;

    // Allocate stack memory.
    StackBottom = (StackType *)MemoryMgr.Allocate(StackSizeInBytes);

    // Compute stack top pointer. (The SP is decremented first, bringing it 
    // to the top of the stack, so no need to subtract 1 in the equation below.)
    StackTop = StackBottom + (StackSizeInBytes / sizeof(StackType));

    // Fill the entire stack area with a pattern. Used as a way to detect stack overflow.
    TicsUtilsClass::MemSet(StackBottom, StackSizeInBytes, DefaultStackPadBytePattern);
}

//-----------------------------------------------------------------------------
/// \brief StackClass destructor. Deallocates stack memory.
//-----------------------------------------------------------------------------
StackClass::~StackClass(void)
{
    // Put the stack memory back on the proper free list.
    MemoryMgr.DeAllocate(StackBottom);
}

//-----------------------------------------------------------------------------
/// \brief Make various checks to insure that the stack has not been corrupted.
//-----------------------------------------------------------------------------
void StackClass::Check(void)
{
    StackType* currentSp = 0;
    int stackPadSizeInWords = (StackPadSizeInBytes / (int) sizeof(StackType));
    int unusedStackSizeInBytes;

    // Read the current CPU stack pointer.
    GetStackPointer(currentSp);

    // Check if the stack pointer is within an allowable range.
    if (currentSp < StackBottom) {
        ErrorHandler.Report(ErrorCurrentSpIsBelowStackBottom);
    }
    else if (currentSp > StackTop) {
        ErrorHandler.Report(ErrorCurrentSpIsAboveStackTop);
    }

    // Compute the size of the unused stack area.
    unusedStackSizeInBytes = (currentSp - StackBottom) * (int) sizeof(StackType);

    // Check to see if the stack pointer is in the pad area.
    if (unusedStackSizeInBytes < StackPadSizeInBytes) {
        ErrorHandler.Report(ErrorStackOverFlow);
    }

    // If the forbidden stack area has been written to, then report an error. 
    // In the rare case where sizeof(StackType) is > sizeof(int),the extra bytes
    // will not be filled nor will they be checked.
    for (int i = 0; i < stackPadSizeInWords; i++) {
        if (StackBottom[i] != DefaultStackPadWordPattern) {
            ErrorHandler.Report(ErrorStackPadAreaWasWrittenTo);
        }
    }
}

//--------------------TaskClass Member Functions-------------------

//-----------------------------------------------------------------------------
/// \brief Adds a taskNodeClass instance to the TaskList.
//-----------------------------------------------------------------------------
void TaskListClass::Add(TaskClass* task)
{
    ListClass::Add((NodeClass*)task);
}

//-----------------------------------------------------------------------------
/// \brief Perform various tasks that need to be handled prior to performing
/// a context switch by calling SwitchTasks().
/// 
/// Note: The very first time that a task is invoked, it can't be "resumed".
/// This condition is handled in function SwitchTasks(), which 
/// is called at the end of this function.
//-----------------------------------------------------------------------------
void TaskClass::Suspend(void)
{
    TaskClass* newTask;
    MsgClass* msg;

    // Delete msgs that have already been processed by tasks. 
    if (DeleteList.IsNotEmpty()) {
        DeleteList.Flush();
    }

        // Check for timeouts and isr msgs.
        CheckForSystemEvents();

        // If there are msgs in the Ready List, then process the next msg. 
        if (ReadyList.IsNotEmpty()) {

            // Get the next Ready List msg.
            msg = (MsgClass*)ReadyList.Remove();

            // Get the next task to run.
            newTask = msg->Receiver;

            // Make sure that the receiver task is a non-null pointer.
            if (newTask == 0) {
                ErrorHandler.Report(ErrorTheNextTaskToRunPtrIsNull);
            }

            // Make sure the receiver task exists.
            if (newTask->TaskExists() == false) {
                ErrorHandler.Report(ErrorTheNextTaskToRunDoesNotExist);
            }

            // Make sure that the receiver task was not deleted and returned to the free list or recycled.
            if (msg->ReceiverId != newTask->Id) {
                ErrorHandler.Report(ErrorTaskIdMismatchCorruptedMsg);
            }

            // A ScheduleMsg is just a wakeup msg; it has no data or meaning to the task,
            // so there is no need to add the msg to the task's msg list. (We will still
            // switch to the new task, we just don't put the ScheduleMsg in its msg list.)
            if (msg->MsgNum == ScheduleMsg) {
                // Delete the ScheduleMsg, since it will not be put into the task's msg list.
                delete msg;
            }
            else {
                // Add the msg to the task's msg list. 
                newTask->MsgList.AddByPriority(msg);
            }
        }
        else {
            // Otherwise, if the ReadyList is empty, then run the IdleTask.
            newTask = &IdleTask;
        }

        // Switch to the new task.
        SwitchTasks(newTask);
}


//-----------------------------------------------------------------------------
/// \brief Checks for the existence of a task object in the TaskList.
///
/// A convenience function added to TaskClass. It checks the Task List
/// for the existence of a task with the indicated task id.
///
/// \param taskId - The task id to match.
///
/// \return true if the task with Id == taskId exists in the list, false otherwise.
//-----------------------------------------------------------------------------
bool TaskClass::TaskExists(int taskId)
{
    return TaskList.TaskExists(taskId);
}

//-----------------------------------------------------------------------------
/// \brief Saves the current task's context, and switches to the new task.
///
/// Save the current task's registers on its stack, load the new task's
/// stack pointer, then pop the new task's registers off its stack, 
/// and return to the new task (since after popping the registers, the
/// new task's return address is on the stack).
///
/// \param newTask - A pointer to the task to switch to.
//-----------------------------------------------------------------------------
void TaskClass::SwitchTasks(TaskClass * newTask)
{
     StackType * tempSp = 0;

     //MDM Check the TaskList integrity.
     TaskList.CheckListIntegrity();

    // Save the currently running task's registers on the current task's stack.
    SaveRegisters();

    // Save the current task's stack pointer into a local variable.
    GetStackPointer(tempSp);

    // Save the stack pointer so we can resume this task later.
    // Note: the very first time we come through here, there is no current 
    // task running, (signified by CurrentTask being zero), so no need to save 
    // the SP.
    if (CurrentTask != 0) {
        // Save the current stack pointer in the current task object.
        CurrentTask->Stack.SavedSp = tempSp;

        // Do a stack check before we switch to the new task.
        CurrentTask->Stack.Check();
    }

    // Make the current task the newTask.
    CurrentTask = newTask;

    // If this task has not yet been started, then call it directly (since 
    // there is no context to restore).
    if (CurrentTask->Flags.IsClr(TaskStartedFlag)) {

        // Get the new task's stack pointer.
        tempSp = CurrentTask->Stack.StackTop;

        // Load the stack pointer register with the top of the stack.
        SetStackPointer(tempSp);

        // Mark the new task as started.
        CurrentTask->Flags.Set(TaskStartedFlag);

        // Call the new task directly.
        CurrentTask->Task();

        // If we've ended up here, then the task has executed a return, which is not allowed.
        ErrorHandler.Report(ErrorReturningFromATaskIsNotAllowed);
    }
    else {
        // Get the new task's stack pointer to a variable so that we can 
        // access it in assembly language.
        tempSp = CurrentTask->Stack.SavedSp;

        // Load the stack pointer register.
        SetStackPointer(tempSp);

        // Restore the registers from the new stack.
        RestoreRegisters();

        // Note that all local variables are now invalid, since the
        // the stack pointer register was changed above.

        // Make sure the new stack is valid.
        CurrentTask->Stack.Check();

        // We will now return to the new task, since we have changed the stack
        // to the new task's stack and its return address is now the stack.
    }
}

//-----------------------------------------------------------------------------
/// \brief For this list, delete all msgs whose Receiver or Sender task matches 
/// the indicated task. 
/// 
/// \param task - See the description above.
//-----------------------------------------------------------------------------
bool MsgListClass::RemoveTaskReferences(TaskClass * task)
{
    NodeClass* nextNode;
    MsgClass* msg;
    TaskClass* receiverTask;
    TaskClass* senderTask;
    bool nodeFound = false;

    for (NodeClass* node = Head->Next; node != Tail; node = nextNode) {

        // Save the next node, because we may delete the current node and lose the next pointer.
        nextNode = node->Next;

        // Convert to a msg object.
        msg = (MsgClass*)node;

        // Get the Receiver TaskClass object.
        receiverTask = msg->Receiver;

        // Get the Sender TaskClass object.
        senderTask = msg->Sender;

        // If we have a match, remove and delete the msg.
        if (receiverTask == task || senderTask == task) {
            Remove(msg);
            delete msg;
            nodeFound = true;
        }
    }

    return nodeFound;
}


//-----------------------------------------------------------------------------
/// \brief Remove and delete one or more occurrences of a node from a list.
///
/// \param node - A pointer to the node to remove from the list.
//-----------------------------------------------------------------------------
bool ListClass::DeleteNode(NodeClass * compareNode)
{
    NodeClass * node;
    NodeClass * next;
    bool nodeWasDeleted = false;

    for (node = Head->Next; node != Tail; node = next) {

        // We need to save this because tempNode may be deleted in the if test below.
        next = node->Next;

        // If the node matches, then delete it.
        if (node == compareNode) {
            Remove(node);
            delete node;
            nodeWasDeleted = true;
        }
    }

    return nodeWasDeleted;
}
    

//-----------------------------------------------------------------------------
/// \brief Remove and delete all occurrences of a node from a list.
///
/// \param node - The id of the node to remove from the list.
//-----------------------------------------------------------------------------
bool ListClass::DeleteNode(int id)
{
    NodeClass* node;
    NodeClass* next;
    bool nodeWasDeleted = false;

    for (node = Head->Next; node != Tail; node = next) {

        // We need to save this because tempNode may be deleted in the if test below.
        next = node->Next;

        // If it matches, then delete the node.
        if (node->Id == id) {
            Remove(node);
            delete node;
            nodeWasDeleted = true;
        }
    }

    return nodeWasDeleted;
}

//-----------------------------------------------------------------------------
/// \brief Insert a msg into a list. Inserts msg a after msg b.
///
/// \param a - Msg to insert after msg b.
///
/// \param b - Msg after which msg a is inserted.
//-----------------------------------------------------------------------------
void ListClass::Insert(NodeClass* a, NodeClass* b)
{
    // Check to see if we're at the maximum allowed number of msgs.
    if (IsFull()) {
        ErrorHandler.Report(ErrorMsgListIsFullCannotInsert);
    }

    //MDM
    DoInsertSafetyChecks(a, b);

    // Insert the msg.
    a->Next = b->Next;
    a->Prev = b;
    b->Next->Prev = a;
    b->Next = a;

    // Mark the msg as being in this list.
    a->SetAsInAList(Id);

    // Bump the list contents count.
    NumNodesInList++;
}

//-----------------------------------------------------------------------------
/// \brief Unlink a msg from the list, but don't delete it.
///
/// \param a - A pointer to the msg to unlink.
///
/// \return A pointer to the unlinked msg.
//-----------------------------------------------------------------------------
NodeClass* ListClass::Unlink(NodeClass* a)
{
    // Unlink the msg.
    a->Prev->Next = a->Next;
    a->Next->Prev = a->Prev;

    // Mark the msg as not being in a list.
    a->SetAsNotInAList();

    // Zero the removed msg links.
    a->Next = a->Prev = 0;

    // Since the msg is no longer in the list, decrement the number of msgs in the list.
    NumNodesInList--;

    // Return a pointer to the unlinked msg.
    return a;
}

//-----------------------------------------------------------------------------
/// \brief Add a msg to the list according to its priority.
///
/// Search starting at the end of the list, since we'll probably add at the
/// end, because all msgs are typically at the same priority. Per the second
/// parameter, (addByTaskPriority), the msg can be added according to the priority
/// of the task to receive the msg, or by the Priority field of the msg
/// itself. Tics adds msgs to the ReadyList according to Receiver priority,
/// and by msg priority when the msg is moved to the msg list of the receiving 
/// task.
///
/// \param a - The msg to add.
///
/// \param addByTaskPriority - If true, add the msg into the list by the 
/// priority of the receiving task, (the Receiver field of the msg), otherwise,
/// add by the Priority field of the msg itself.
//-----------------------------------------------------------------------------
void ListClass::AddByPriority(NodeClass* a)
{
    NodeClass* b;
    int aPriority;
    int bPriority;

    // Get the priority.
    aPriority = a->Priority;

    // Scan the list by priority in reverse order, and insert accordingly. We scan in
    // reverse order because most of the time we'll be adding to the end of the list.
    for (b = Tail->Prev; b != Head; b = b->Prev) {

        bPriority = b->Priority;

        if (bPriority >= aPriority) {
            // Insert msg a after msg b.
            Insert(a, b);
            return;
        }
    }
    
    // If we've fallen to here, the list is either empty, or the msg priority is higher than
    // any msg in the list. In either case, we want to add the msg after the head.
    Insert(a, Head);
}

//-----------------------------------------------------------------------------
/// \brief Add a msg to the end of the list.
//-----------------------------------------------------------------------------
void ListClass::Add(NodeClass* a)
{
    // Insert the msg after the last item in the list (which means in front of the tail).
    Insert(a, Tail->Prev);
}

//-----------------------------------------------------------------------------
/// \brief Add msg a to the delay list.
///
/// The Delay List is a list whose msgs will be sent out after their respective timers
///  expire. The Delay List is sorted by the msg EndTime field. Smallest EndTime's are 
/// at the front of the list.
///
/// \param a - The msg to add to the Delay List.
//-----------------------------------------------------------------------------
void DelayListClass::AddByDelay(MsgClass* a)
{
    MsgClass* b = (MsgClass*)Head->Next;
    NodeClass* node;

    for (node = Head->Next; node != Tail; node = node->Next) {

        // Convert to a msg.
        b = (MsgClass*)node;

        // Insert msg a in front of msg b if its end time is sooner.
        if (a->EndTime < b->EndTime) {
            break;
        }
    }

    // Insert msg a in front of msg b (after the msg in front of msg b).
    // If we didn't break, b is the Tail, so we're inserting after Tail->Prev,
    // which is the last msg in the list. This is the case when the list 
    // is empty, or a->EndTime is greater than every msg in the list.
    Insert(a, b->Prev);
}

//-----------------------------------------------------------------------------
/// \brief Check for delayed msgs that are ready to be sent.
///
/// Delayed msgs are msgs that will be sent out at a later time, designated by
/// the msg EndTime field. The EndTime field is in units of system ticks (see
/// the function ReadTickCount()). Delayed msgs are held in the Delay List.
/// This function scans the Delay List, and sends out any msgs whose EndTime
/// is past the current time.
//-----------------------------------------------------------------------------
void DelayListClass::CheckForTimeouts()
{
    NodeClass* node;
    NodeClass* nextNode;
    MsgClass* msg;
    MsgClass* nextMsg;
    TimerTickType currentTime;

    // If the list is empty, we have no timers to process.
    if (IsEmpty()) {
        return;
    }
    
    // Read the current clock tick.
    currentTime = ReadTickCount();

    // If there is no time change, there is no need to check for expired timers.
    if (LastTime == currentTime) {
        return;
    }

    // Look for delayed msgs that are ready to be sent.
    for (node = Head->Next; node != Tail; node = nextNode) {

        // If the msg is timed out, it will be removed from the list, and added to
        // the ReadyList (see below). Once removed, msg->Next will not point 
        // to the next msg in the list. So, we need to save the next msg in 
        // the list so that the for-loop works properly.
        nextNode = node->Next;

        // Get the msg.
        msg = (MsgClass*)node;

        // Get the next msg.
        nextMsg = (MsgClass*)nextNode;

        // If we're past the delayed msg end time, then dispatch it.
        if (currentTime >= msg->EndTime) {
            // Remove the delayed msg from the delay list.
            Remove(msg);

            // Add the delayed msg to the Ready List.
            ReadyList.AddByPriority(msg);

            // If there are no other timed-out msgs, then exit.
            // Remember, the msgs are sorted by the end time, so if
            // the next msg has not timed out, then no other msg in the list
            // is timed out either. Note: if nextMsg is the Tail, then the
            // if condition will always fail, since EndTime is initialized to 0.
            if (currentTime < nextMsg->EndTime) {
                break;
            }
        }
    }
    // Save for the next time we enter this function.
    LastTime = currentTime;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the list is empty.
///
/// \return true if the list is empty, false otherwise.
//-----------------------------------------------------------------------------
bool ListClass::IsEmpty(void)
{
    // If the msg count is 0, then there are no msgs in the list.
    return NumNodesInList == 0;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the list is not empty.
///
/// \return true if the list is not empty, false otherwise.
//-----------------------------------------------------------------------------
bool ListClass::IsNotEmpty(void)
{
    // If the msg count is not 0, then there are msgs in the list.
    return NumNodesInList != 0;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the list is full.
///
/// \return true if the list is full, false otherwise.
//-----------------------------------------------------------------------------
bool ListClass::IsFull(void)
{
    // The maximum number of msgs allowed in the list is MaxMsgs.
    return NumNodesInList >= MaxNodes;
}

//-----------------------------------------------------------------------------
/// \brief Remove the indicated msg from the list, but don't delete it.
///
/// Removes the indicated msg. If the msg argument is not specified, it will 
/// assume its default value of 0, in which case the msg at the head of the
/// list is returned. If the msg is not found, an error is generated.
///
/// \param a - The msg to remove. If 0, the msg at the head of the list is returned.
///
/// \return The removed msg.
//-----------------------------------------------------------------------------
NodeClass* ListClass::Remove(NodeClass* a)
{
    NodeClass* node;

    // If the list is empty, then error.
    if (IsEmpty()) {
        ErrorHandler.Report(ErrorAttemptToRemoveAMsgFromAnEmptyList);
    }

    // If no argument was specified, it will assume its default value of 0,
    // in which case we return the msg at the head of the list. 
    if (a == 0) {
        // We know that there is a msg at Head->Next because we previously checked for an empty list.
        a = Head->Next;
    }

    // Unlink the msg from the list.
    node = Unlink(a);

    // Return a pointer to the unlinked msg.
    return node;
}

//-----------------------------------------------------------------------------
/// \brief Checks for the existence of a task object in the Task List.
///
/// The Task List contains a list of msgs whose Receiver field points
/// to an active task. This function traverses the task list looking for a match
/// between the function parameter, (task), and msg->Receiver. Although this is a
/// general MsgListClass function, it is typically applied only to TaskList.
///
/// \param task - A pointer to the task object to match.
/// \param id - The node id. If non-zero, it is used as a double check. If 
/// the id does not match task->Id, then the node has been recycled and 
/// therefore it is invalid and a value of false will be returned.
///
/// \return true if the task argument exists and is valid, false otherwise.
//-----------------------------------------------------------------------------
bool TaskListClass::TaskExists(TaskClass* task, int id)
{
    TaskClass* tempTask;

    // Check the task pointer.
    if (task == 0) {
        ErrorHandler.Report(ErrorNullTaskPointerInTaskExists);
    }

    // Look for the task in the list.
    for (NodeClass * node = Head->Next; node != Tail; node = node->Next) {

        // Convert to a TaskClass object.
        tempTask = (TaskClass*)node;

        if (tempTask == task) {
            if (id == 0) {
                // The id has not been specified, (0 means it is not specified) so ignore it. Match found.
                return true;
            }
            else if (tempTask->Id == id) {
                return true;
            }
            else {
                return false;
            }
        }
    }

    // No match was found.
    return false;
}

//-----------------------------------------------------------------------------
/// \brief Checks for the existence of a task object in the TaskList.
///
/// The Task List contains a list of TaskClass objects. Each object is
/// checked for an id match. a value of true is returned if a match is
/// found, otherwise, false is returned.
/// 
/// \param id - The task id to match.
///
/// \return true if the task with Id == id exists in the list, false otherwise.
//-----------------------------------------------------------------------------
bool TaskListClass::TaskExists(int id)
{
    TaskClass* task;

    // Look for the task in the list.
    for (NodeClass* node = Head->Next; node != Tail; node = node->Next) {

        // Convert to a TaskClass object.
        task = (TaskClass*)node;

        if (task->Id == id) {
            // Match found.
            return true;
        }
    }

    // No match was found.
    return false;
}

//-----------------------------------------------------------------------------
/// \brief Remove and delete all the items in the list.
//-----------------------------------------------------------------------------
void ListClass::Flush()
{
    NodeClass* node;
    NodeClass* nextNode;

    // Remove and delete each node.
    for (node = Head->Next; node != Tail; node = nextNode) {
        nextNode = node->Next;
        Remove(node);
        delete node;
    }
}

//-----------------------------------------------------------------------------
/// \brief MemNdeListClass constructor.
///
/// The list is doubly linked, with permanent Head and Tail nodes, which makes
/// for easier coding and understanding. The list is initialized so that Head
/// and Tail point to each other. ActualHead and ActualTail are NodeClass
/// objects, and Head and Tail are pointers to them.
///
/// \param maxNodes - The maximum number of msgs allowed in the list.
//-----------------------------------------------------------------------------
ListClass::ListClass(int maxNodes) : MaxNodes(maxNodes)
{
    // Assign head and tail. These msgs are never removed from the list, they act as markers.
    Head = &ActualHead;
    Tail = &ActualTail;

    // Set Head and Tail as being in a list. This is needed for error checking.
    Head->SetAsInAList(Id);
    Tail->SetAsInAList(Id);

    // Head priority is higher than any msg priority.
    Head->Priority = HeadPriority;

    // Tail priority is lower than any msg priority.
    Tail->Priority = TailPriority;

    // Link head to tail and tail to head.
    Head->Prev = Head->Next = Tail;
    Tail->Prev = Tail->Next = Head;

    // Initialize the number of msgs in the list.
    NumNodesInList = 0;
}

//-----------------------------------------------------------------------------
/// \brief Sends a msg from an isr to a task.
///
/// The normal TaskClass::Send() function cannot be used from within an isr
/// because the linked list links can get corrupted. Instead, a FifoClass
/// object is used. The isr adds data items to the fifo and the task retrieves
/// the objects from the fifo. The isr must use this function to send data
/// to a task, if that is required. The preferred method is for the isr to handle
/// all necessary work, but if work must be deferred to a task, then this function
/// must be used, or the isr can simply do what this function does, i.e., add data to a
/// fifo, then schedule the task to run. When the task runs, it retrieves the data
/// from the fifo, either directly or by using function
/// TaskClass::Wait(FifoClass * fifo, void * data).
///
/// \param task - The task to send the data to.
/// \param fifo - The task's fifo. The fifo can only be used by one isr.
/// \param data - The data (msg) to be copied into the fifo slot.
//-----------------------------------------------------------------------------
void TicsNameSpace::Send(TaskClass * task, FifoClass * fifo, void * data)
{
    // Add the data block into the task's fifo.
    fifo->Add(data);

    // Add the task to the Interrupt fifo.
    Schedule(task, true);
}

//-----------------------------------------------------------------------------
/// \brief Waits for a fifo msg from an isr.
///
/// See the description given above for TicsNameSpace::Send().
//-----------------------------------------------------------------------------
void TaskClass::Wait(FifoClass * fifo, void * data)
{
    if (fifo->IsNotEmpty()) {
        // Copy the data from the fifo into the data block.
        fifo->Remove(data);
    }
    else {
        // Suspend until the isr schedules the task.
        Suspend();
    }
}

//-----------------------------------------------------------------------------
/// \brief Remove the task from the task list.
///
/// \param task - The task to remove.
//-----------------------------------------------------------------------------
void TaskListClass::RemoveTask(TaskClass* task)
{
    if (task == 0) {
        ErrorHandler.Report(ErrorNullPointer);
    }

    // Delete the task object from the task list.
    DeleteNode(task);
}

//-----------------------------------------------------------------------------
/// \brief For each task in the Task List, remove from its msg list all 
/// occurrences of the argument "task".
///
/// \param task - The task to remove.
//-----------------------------------------------------------------------------
void TaskListClass::RemoveTaskReferences(TaskClass* task, bool removeTheTaskItselfAlso)
{
    NodeClass* node;
    TaskClass* tempTask;

    // For each task in the Task List, remove from its msg list all occurrences of task (the arg to this function).
    for (node = Head->Next; node != Tail; node = node->Next) {

        // The node is actually a task object, so make the conversion.
        tempTask = (TaskClass*) node;

        //MDM Check TaskList integrity.
        tempTask->MsgList.CheckListIntegrity();

        // Remove all occurrences of the task from tempTask's msg list.
        tempTask->MsgList.RemoveTaskReferences(task);
    }

    // Now remove the task itself from the task list.
    if (removeTheTaskItselfAlso) {
        DeleteNode(task);
    }
}

//-----------------------------------------------------------------------------
/// \brief Read and return the tick count from the system clock.
///
/// We default to just reading the C clock function, which typically ticks
/// each millisecond. You can choose to replace the call to clock() with
/// a call to your own hardware clock source if you'd like, however, since
/// this function is not meant to be accurate, clock() should be fine, assuming
/// your compiler supports it.
///
/// Note: the coarser the clock granularity the better. For example,
/// 10 millisecond granularity is preferable to 1 ms. We recommend 
/// 10 ms or higher. It's up to you to decide what granularity you
/// can live with.
///
/// \return The current system tick count reading.
//-----------------------------------------------------------------------------
TimerTickType TicsNameSpace::ReadTickCount()
{
    // Replace this with your own hardware clock time.
    return (TimerTickType) clock();
}

//-----------------------------------------------------------------------------
/// \brief Add the indicated task to the Ready List. 
///
/// If inIsr is true, the task is added to the Interrupt Fifo, and will be 
/// transferred to the Ready List on the next task switch, otherwise, if inIsr
/// is false, the task is added to the Ready List.
///
/// Note: Isr's must not re-enable interrupts - ALL interrupts must remain disabled
/// for the duration of the isr.
///
/// \param task - The task to schedule.
///
/// \param inIsr - Set to true if this function is being called from an isr.
//-----------------------------------------------------------------------------
void TicsNameSpace::Schedule(TaskClass* task, bool inIsr)
{
    // Make sure we have a non-null pointer.
    if (task == 0) {
        ErrorHandler.Report(ErrorNullTaskPointerInSchedule);
    }

    // If we're in an interrupt service routine...
    if (inIsr) {
        // Schedule the task by adding it to the Interrupt Fifo, rather
        // than the Ready List, to avoid list corruption.
        // All interrupts must remain disabled within the isr, otherwise
        // the Interrupt Fifo can be corrupted.
        InterruptFifo.Add(&task);
    }
    else {
        // Add the task to the Ready List.
        ReadyList.AddByPriority(new MsgClass(task, ScheduleMsg));
    }
}

//-----------------------------------------------------------------------------
/// \brief If there are any tasks in the Interrupt Fifo, move them to the 
/// Ready List.
///
/// Tasks can't be scheduled directly (by adding to the Ready List) from within 
/// an isr (otherwise, list corruption could occur). Instead, tasks are scheduled
/// by adding them to the Interrupt Fifo, and then later moved to the
/// Ready List. This function is called at each task switch.
//-----------------------------------------------------------------------------
void TicsNameSpace::CheckForInterrupts()
{
    TaskClass* task;

    // If the Interrupt Fifo is not empty, then remove the task from it, and schedule it.
    while (InterruptFifo.IsNotEmpty()) {

        // Get the task from the Interrupt Fifo.
        InterruptFifo.Remove(&task);

        // Check for an invalid task.
        if (task == 0) {
            ErrorHandler.Report(ErrorNullTaskPtrInCheckForInterrupts);
        }

        // Schedule the task.
        Schedule(task);
    }
}


//-----------------------------------------------------------------------------
/// \brief If there are any tasks in the Interrupt Fifo, move them to the 
/// Ready List.
///
/// Tasks can't be scheduled directly (by adding to the Ready List) from within 
/// an isr (otherwise, list corruption could occur). Instead, tasks are scheduled
/// by adding them to the Interrupt Fifo, and then later moved to the
/// Ready List. This function is called at each task switch.
/// 
/// Note: This function is not used. It is kept for future use.
//-----------------------------------------------------------------------------
void TicsNameSpace::CheckForInterrupts2()
{
    int numItemsInTable;

    // Determine the number of items in the table.
    numItemsInTable = sizeof(InterruptTable) / sizeof(InterruptTableRowClass);

    // Check each table entry. If data is available in the handler fifo, then call the handler.
    for (int i = 0; i < numItemsInTable; i++) {

        // If data is available in the handler fifo, then call the handler.
        if (InterruptTable[i].DataAvailable()) {
            // Call the interrupt handler.
            InterruptTable[i].Handler();
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Check for interrupt msgs and msg timeouts. For Tics system use only.
//-----------------------------------------------------------------------------
void TicsNameSpace::CheckForSystemEvents()
{
    // Check for msgs in the Interrupt Fifo.
    CheckForInterrupts();

    // Check for expired timers.
    DelayList.CheckForTimeouts();
}

//-----------------------------------------------------------------------------
/// \brief Start tasks running.
///
/// Call this function from main() to start Tics running.
/// Typically you would have created tasks in main() prior
/// to invoking this function.
//-----------------------------------------------------------------------------
void TicsNameSpace::Suspend()
{
    // Suspend the current task, and run the next task in the Ready List.
    TicsSystemTask.Suspend();
}

//-----------------------------------------------------------------------------
/// \brief TaskClass constructor. See Tics.hpp for parameter default values.
///
/// \param name - An optional name for the task. The name defaults to 0.
///
/// \param priority - The task priority.
///
/// \param flags - Various flag settings. Flag bits are OR'ed into Flags.
/// The DropUnexpectedMsgsFlag means that if a task is waiting for msgA, and
/// it receives msgB, then msgB is dropped, rather than keeping msgB in
/// the task's msg list. The ScheduleTaskOnCreationFlag means that a ScheduleMsg
/// will be sent to the task on creation, which means that the task will be put
/// into the ReadyList on creation (this is the default). This forces the task
/// to run at startup time.
///
/// \param stackSizeInBytes - The desired task stack size in bytes. If set equal
/// to 0, a default stack size of DefaultStackSizeInBytes is assigned.
///
/// Note: All the parameters are defaulted. See the definition of the
/// TaskClass constructor in Tics.hpp.
//-----------------------------------------------------------------------------
TaskClass::TaskClass(
    const char* name,
    int priority,
    int flags,
    int stackSizeInBytes):
    Flags(flags),
    Stack(stackSizeInBytes),
    Name(name),
    Priority(priority), NodeClass()
{
    // Add the task to the list of active tasks.
    TaskList.Add(this);

    // Schedule this task if we're allowed to do so.
    if (Flags.IsSet(ScheduleTaskOnCreationFlag)) {
        Schedule(this);
    }
}

//-----------------------------------------------------------------------------
/// \brief TaskClass destructor. Removes all references to this task, then 
/// Removes the task itself from the task list.
//-----------------------------------------------------------------------------
TaskClass::~TaskClass(void)
{
    // Make sure the task exists.
    if (TaskExists(this) == false) {
        ErrorHandler.Report(ErrorAttemptToDeleteANonexistentTask);
    }

    // You can only delete a task from within another task - a task cannot delete itself because
    // we need that task's stack to exit this function. The StackClass destructor will be
    // called because there is an instance of StackClass defined in TaskClass. The StackClass
    // destructor will delete the stack.
    if (CurrentTask == this) {
        ErrorHandler.Report(ErrorAttemptToDeleteTheCurrentTask);
    }

    // You can't delete system tasks - they're an integral part of Tics.
    if (this == &TicsSystemTask || this == &IdleTask) {
        ErrorHandler.Report(ErrorAttemptToDeleteASystemTask);
    }

    // Remove all the msg's from this task's msg list.
    MsgList.Flush();

    //MDM Check TaskList integrity.
    TaskList.CheckListIntegrity();
    // Remove all occurrences of this task from the msg list of all other tasks.
    TaskList.RemoveTaskReferences(this);

    //MDM Check TaskList integrity.
    ReadyList.CheckListIntegrity();
    // Remove all occurrences of this task object from the Ready List.
    ReadyList.RemoveTaskReferences(this);

    //MDM Check TaskList integrity.
    DelayList.CheckListIntegrity();
    // Remove all occurrences of this task object from the Delayed Msg List.
    DelayList.RemoveTaskReferences(this);

    //MDM Check TaskList integrity.
    TaskList.CheckListIntegrity();
    // Unlink (remove) the task itself from the task list.
    TaskList.Remove(this);

    // Bump the task id to indicate that the task has been deleted.
    Id++;
}

//-----------------------------------------------------------------------------
/// \brief Remove all references to the task from this task's msg list.
///
/// \param task - The task that we want to remove from this task's msg list.
//-----------------------------------------------------------------------------
void TaskClass::DeleteFromMsgList(TaskClass* task)
{
    NodeClass* nextNode;
    MsgClass* msg;

    for (NodeClass* node = MsgList.Head->Next; node != MsgList.Tail; node = nextNode) {
        
        // Save for use in the for-loop.
        nextNode = node->Next;

        // Convert to msg.
        msg = (MsgClass*)node;

        // If the task is referenced, by either Sender or Receiver, remove it.
        if (msg->Receiver == task || msg->Sender == task) {
            // Remove and delete the task from the task's msg list.
            MsgList.Remove(msg);
            delete msg;
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Send a msg to this task after the indicated number of ticks. 
/// 
/// \param numTicks - The number of ticks to wait before the msg is sent to
/// this task.
///
/// \param priority - The priority of the msg.
/// \param msgNum - The msg number of the msg.
//-----------------------------------------------------------------------------
MsgClass * TaskClass::StartTimer(int numTicks, int priority, int msgNum)
{
    // Send a delayed msg to this task.
    return Send(this, msgNum, 0, 0, numTicks, priority);
}

//-----------------------------------------------------------------------------
/// \brief Put the task to sleep for the indicated number of ticks.
///
/// \param numTicks - The number of ticks to sleep.
/// \param priority - The priority of the msg.
//-----------------------------------------------------------------------------
void TaskClass::Pause(int numTicks, int priority)
{
    // Start a timer for this task.
    StartTimer(numTicks, priority);

    // Wait for the delayed msg.
    Wait(TimeoutMsg);
}

//-----------------------------------------------------------------------------
/// \brief Remove the msg with msg number msgNum from this task's msg list,
///  and return a pointer to it. 
///
/// As the msg list is traversed, each msg that mismatches is removed from the
/// list and deleted if DropUnexpectedMsgs is true, otherwise, the msg remains
/// in the list. If a match is found, no further traversal of the list occurs.
///
/// When and if a msg match is found, the msg is removed from the task's msg list,
/// and added to the Delete List. On the next task switch, the msg will be
/// removed from the Delete List and deleted. This means that the msg is available
/// to the current task until it performs a task switch (by waiting for a
/// msg, for example).
///
/// If the msg number is AnyMsg, then the first msg in the list is returned,
/// assuming that the list is not empty. In other words, AnyMsg means to 
/// return any msg, regardless of the msg number.
///
/// \param msgNum - The msg number of the msg to remove.
///
/// \return Returns the msg if found, otherwise 0.
//-----------------------------------------------------------------------------
MsgClass* TaskClass::Recv(int msgNum)
{
    NodeClass* nextNode;
    MsgClass* msg;

    // If the list is empty, then return 0.
    if (MsgList.IsEmpty()) {
        return 0;
    }

    // Traverse the list looking for a msg with the desired msg number.
    for (NodeClass* node = MsgList.Head->Next; node != MsgList.Tail; node = nextNode) {

        // Save the next msg for use in the for loop.
        nextNode = node->Next;

        // Convert the node to a msg.
        msg = (MsgClass*)node;

        // See if we have a match to the msg we're waiting for. AnyMsg will match any msg.
        if (msgNum == AnyMsg || msgNum == msg->MsgNum) {
            // Remove the found msg from the msg list.
            MsgList.Remove(msg);
            // Add the msg to the delete list, which means that the msg will be deleted on the next Suspend() call.
            DeleteList.Add(msg);
            // Return a pointer to the msg. The msg will be valid until the current task suspends.
            return msg;
        }
        else {
            // Drop the unexpected msg if so enabled.
            if (Flags.IsSet(DropUnexpectedMsgsFlag)) {
                // Remove the msg to drop.
                MsgList.Remove(msg);
                // Delete the msg.
                delete msg;
            }
        }
    }

    // The msg was not found, so return NULL.
    return 0;
}

//-----------------------------------------------------------------------------
/// \brief Check each msg number in the array and check if it is in the
/// task's msg list, and if so, return a pointer to it, otherwise,
/// return a null pointer.
//-----------------------------------------------------------------------------

MsgClass* TaskClass::Recv(int* msgNumArray, int numMsgs)
{
    int i;
    MsgClass* msg = 0;
    
    // Check for invalid numMsgs.
    if (numMsgs > MaxAllowedMsgsInRecv) {
        ErrorHandler.Report(ErrorMaxAllowedMsgsInRecv);
    }

    for (i = 0; i < numMsgs; i++) {
        msg = Recv(msgNumArray[i]);
        if (msg != 0) {
            return msg;
        }
    }

    // No msgs in the MsgNumArray were found. Return a null pointer.
    return 0;
}

//-----------------------------------------------------------------------------
/// \brief Wait for a msg with a particular msg number.
///
/// If the msg is found, then remove it from the list, and return a pointer 
/// to it, otherwise, suspend and wait to be rescheduled, which will occur 
/// when another msg is sent to this task, at which point the task will resume,
/// and check its msg list again to see if the newly arrived msg matches. If
/// the newly arrived msg does not match, the task again suspends. If the msgNum
/// parameter is not specified, then the first msg in the list is returned.
///
/// \param msgNum - The msg number of the msg to wait for.
///
/// \return Returns A pointer to the msg.
//-----------------------------------------------------------------------------
MsgClass* TaskClass::Wait(int msgNum)
{
    MsgClass* msg;

    for (;;) {
        // Get the msg.
        msg = Recv(msgNum);

        // If we have the msg in this task's msg list, then remove it and return it.
        if (msg != 0) {
            return msg;
        }
        else {
            // The msg was not found, so suspend until we get another msg.
            Suspend();
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Wait for any of the msgs listed in an array.
///
/// If the msg is found, then remove it from the list, and return a pointer 
/// to it, otherwise, suspend and wait to be rescheduled, which will occur 
/// when another msg is sent to this task, at which point the task will resume,
/// and check its msg list again to see if the newly arrived msg matches. If
/// the newly arrived msg does not match, the task again suspends. If the msgNum
/// parameter is not specified, then the first msg in the list is returned.
///
/// \param msgNumArray - An array containing a list of msg numbers. If any
/// of these msg numbers is found in the task's msg list, then remove it
/// and return to the caller, otherwise, suspend until one of the msg numbers
/// is found.
///
/// \param numMsgs - The number of msgs in msgNumArray.
/// 
/// \return Returns a pointer to the msg.
//-----------------------------------------------------------------------------
MsgClass* TaskClass::Wait(int* msgNumArray, int numMsgs)
{
    MsgClass* msg;

    for (;;) {
        // Get the msg.
        msg = Recv(msgNumArray, numMsgs);

        // If we have the msg in this task's msg list, then remove it and return it.
        if (msg != 0) {
            return msg;
        }
        else {
            // The msg was not found, so suspend until we get another msg.
            Suspend();
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Delete a previously sent msg.
///
/// Attempt to a remove previously sent msg with msg id msgId from the system. 
///
/// \param nodeId - Obtainable from the msg returned by Send() (msg->Id).
///
/// \return true if the msg was canceled, otherwise false.
//-----------------------------------------------------------------------------
bool TaskClass::CancelMsg(int nodeId)
{
    // If the msg has not been deleted, it should be in one of the following
    // lists. Once it is found in one of the lists, there is no need to check
    // the others.

    // Check the Delay List.
    if (DelayList.DeleteNode(nodeId)) {
        return true;
    }
    // Check the Ready List.
    else if (ReadyList.ListClass::DeleteNode(nodeId)) {
        return true;
    }
    // Check the task list
    else if (TaskList.DeleteNode(nodeId)) {
        return true;
    }
    else {
        return false;
    }
}

//-----------------------------------------------------------------------------
/// \brief Delete a previously sent msg.
///
/// Attempt to remove the msg from the system. Use this function only if
/// you know the msg has not been deleted. If it might be deleted, use the
/// version of CancelMsg that takes a msg Id as a parameter.
///
/// \param msg - Obtainable from the msg returned by Send().
///
/// \return true if the msg was canceled, otherwise false.
//-----------------------------------------------------------------------------
bool TaskClass::CancelMsg(MsgClass * msg)
{
    // Cancel the msg based on its msg id.
    return CancelMsg(msg->Id);
}

//-----------------------------------------------------------------------------
/// \brief Add a task to the Ready List.
///
/// \param task - The task to add to the Ready List. If no parameter is specified,
/// then "this" task is used.
//-----------------------------------------------------------------------------
void TaskClass::Schedule(TaskClass* task)
{
    TaskClass* taskToSchedule;

    // If no parameter was specified in the call, then default to "this" task object.
    if (task == 0) {
        taskToSchedule = this;
    }
    else {
        taskToSchedule = task;
    }

    // Schedule the task to run. (Without the namespace qualifier we'd have recursion).
    TicsNameSpace::Schedule(taskToSchedule);
}

//-----------------------------------------------------------------------------
/// \brief Reschedule this task.
///
/// Schedule this task to run, then suspend it. The task will
/// run again when it bubbles to the top of the Ready List.
//-----------------------------------------------------------------------------
void TaskClass::Yield()
{
    // Schedule this task to run again.
    Schedule();

    // Suspend this task. Eventually it will run again, since it has been scheduled.
    Suspend();
}

//-----------------------------------------------------------------------------
/// \brief Reply to the sender of a msg.
///
/// This is a convenience function. You can also reply by sending a msg to
/// msg->Sender, which is what this function does.
///
/// \param receivedMsg - The msg you received.
/// \param msgNum - The msg number to reply with.
/// \param data - The data to reply with.
/// \param ptr - The pointer to reply with.
/// \param delay - The delay to reply with.
/// \param priority - The priority of the reply msg.
/// \param sender - The sender of the reply (used for aliasing).
//-----------------------------------------------------------------------------
void TaskClass::Reply(MsgClass* receivedMsg, int msgNum, int data, void* ptr, int delay, int priority, TaskClass* sender)
{
    // Reply by sending a msg to the sender of the received msg.
    Send(receivedMsg->Sender, msgNum, data, ptr, delay, priority, sender);
}

//-----------------------------------------------------------------------------
/// \brief Initialize a MsgClass object.
//-----------------------------------------------------------------------------
void MsgClass::Init()
{
    // Remember the receiver task id. Used for error checking.
    ReceiverId = Receiver->Id;
}


//-----------------------------------------------------------------------------
/// \brief MsgClass constructor. See Tics.hpp for parameter defaults.
///
/// \param receiver - A pointer to the task you're sending the msg to.
/// \param msgNum - The msg number.
/// \param data - The msg data, if any.
/// \param ptr - Pointer to a data packet, if any.
/// \param delay - The number of clock ticks to wait before sending the msg.
/// \param priority - The msg priority.
/// \param sender - A pointer to the task that is sending the msg.
//-----------------------------------------------------------------------------
MsgClass::MsgClass(
    TaskClass* receiver,
    int msgNum,
    int data,
    void* ptr,
    int delay,
    int priority,
    TaskClass* sender) : NodeClass(data, ptr, priority),
    ReceiverId(0),
    MsgNum(msgNum),
    Delay(delay),
    EndTime(0),
    Sender(sender),
    Receiver(receiver)
{
    // Initialize the msg.
    Init();
}

//-----------------------------------------------------------------------------
/// \brief MsgClass destructor.
//-----------------------------------------------------------------------------
MsgClass::~MsgClass(void)
{
    // Check for corruption.
    if (Receiver != 0 && ReceiverId != Receiver->Id) {
        ErrorHandler.Report(ErrorAttemptToDeleteACorruptedMsg);
    }
}

//-----------------------------------------------------------------------------
/// \brief Check msg parameters.
///
/// \param fullCheck - If false, don't check for a receiver or sender.
//-----------------------------------------------------------------------------
void MsgClass::CheckParameters(bool fullCheck)
{
    // Low and high priorities are valid if the msg is Head or Tail.
    if (InRange(LowPriority, HighPriority, Priority) == false) {
        ErrorHandler.Report(ErrorMsgPriorityIsOutOfRange);
    }

    // Check for an invalid Delay value.
    if (Delay < 0) {
        ErrorHandler.Report(ErrorInvalidMsgDelay);
    }

    // Make sure we have a non-zero receiver task.
    if (fullCheck && Receiver == 0) {
        ErrorHandler.Report(ErrorMsgReceiverTaskPtrIsNull);
    }

    // Make sure we have a non-zero sender task.
    if (fullCheck && Sender == 0) {
        ErrorHandler.Report(ErrorMsgSenderTaskPtrIsNull);
    }
}

//-----------------------------------------------------------------------------
/// \brief Allocate space for a TaskClass object.
///
/// \param size - The number of bytes to allocate.
///
/// \return A pointer to the allocated memory.
//-----------------------------------------------------------------------------
void * TaskClass::operator new(size_t size)
{
    // Allocate a block of memory for the task object.
    void * p = MemoryMgr.Allocate((int) size);

    return p;
}

//-----------------------------------------------------------------------------
/// \brief Free up space for a TaskClass object.
///
/// \param p - A pointer to the allocated space.
//-----------------------------------------------------------------------------
void TaskClass::operator delete(void * p)
{
    // Deallocate the task memory block.
    MemoryMgr.DeAllocate(p);
}


//-----------------------------------------------------------------------------
/// \brief Allocate space for a MsgClass object.
///
/// \param size - The number of bytes to allocate.
///
/// \return A pointer to the allocated memory.
//-----------------------------------------------------------------------------
void * MsgClass::operator new(size_t size)
{
    // Allocate a block of memory for the msg object.
    void * p = MemoryMgr.Allocate((int) size);

    return p;
}

//-----------------------------------------------------------------------------
/// \brief Free up space for a MsgClass object.
///
/// \param p - A pointer to the allocated space.
//-----------------------------------------------------------------------------
void MsgClass::operator delete(void * p)
{
    // Deallocate the msg memory block.
    MemoryMgr.DeAllocate(p);
}

//-----------------------------------------------------------------------------
/// \brief Allocate space for a FifoClass object.
///
/// \param size - The number of bytes to allocate.
///
/// \return A pointer to the allocated memory.
//-----------------------------------------------------------------------------
void * FifoClass::operator new(size_t size)
{
    // Allocate space for the FifoClass object.
    void * p = MemoryMgr.Allocate((int) size);

    return p;
}

//-----------------------------------------------------------------------------
/// \brief Free up space for a FifoClass object.
///
/// \param p - A pointer to the allocated space.
//-----------------------------------------------------------------------------
void FifoClass::operator delete(void * p)
{
    // Deallocate the FifoClass memory block.
    MemoryMgr.DeAllocate(p);
}


//-----------------------------------------------------------------------------
/// \brief Send a msg to a task.
///
/// \param receiver - A pointer to the task that is to receive the msg.
/// \param msgNum - The msg number.
/// \param data - Integer data associated with the msg, if any.
/// \param ptr - A pointer to data associated with the msg, if any.
/// \param delay - The number of ticks to wait before sending the msg, if any.
/// \param priority - Determines where in the ReadyList and the receiver's msg list the msg is inserted.
/// \param sender - A pointer to the sender of the msg (used for replying).
///
/// \return A pointer to the msg that was sent.
//-----------------------------------------------------------------------------
MsgClass* TaskClass::Send(
    TaskClass* receiver,
    int msgNum,
    int data,
    void* ptr,
    int delay,
    int priority,
    TaskClass* sender)
{
    MsgClass* msg = 0;

    // Create the msg.
    msg = new MsgClass(receiver, msgNum, data, ptr, delay, priority, sender);

    // Send the created msg, and return a pointer to it.
    return Send(msg);
}

//-----------------------------------------------------------------------------
/// \brief Send a msg to a task.
///
/// \param receiver - A reference to the task to receive the msg.
/// \param msgNum - The msg number.
/// \param data - Integer data associated with the msg, if any.
/// \param ptr - A pointer to data associated with the msg, if any.
/// \param delay - The number of ticks to wait before sending the msg, if any.
/// \param priority - Determines where in the receiver's msg list the msg is inserted.
/// \param sender - The sender of the msg (used for replying).
///
/// \return A pointer to the msg that was sent.
///
/// If a task is statically allocated you can refer to it with a reference,
/// but you have to make sure that the instantiation of the task happens
/// after Tics globals have been instantiated.
//-----------------------------------------------------------------------------
MsgClass* TaskClass::Send(
TaskClass& receiver,
int msgNum,
int data,
void* ptr,
int delay,
int priority,
TaskClass* sender)
{
    // Send the msg using the version of Send that accepts a pointer (instead of a reference) to the task.
    return Send(&receiver, msgNum, data, ptr, delay, priority, sender);
}

//-----------------------------------------------------------------------------
/// \brief Send a msg to a task.
///
/// \param msg - The msg to send.
///
/// \return A pointer to the msg that was sent.
//-----------------------------------------------------------------------------
MsgClass * TaskClass::Send(MsgClass* msg)
{
    // If the sender is 0, then make the sender this task.
    if (msg->Sender == 0) {
        
        // Make the sender this task.
        msg->Sender = this;
    }

    // If this not a delayed msg, then schedule the receiver task to run.
    if (msg->Delay == 0) {
        ReadyList.AddByPriority(msg);
    }
    else {
        // Compute the delay end time.
        msg->EndTime = (TimerTickType)ReadTickCount() + (TimerTickType)msg->Delay;

        // Add the delayed msg to the delay list. The task will be added to the Ready List when the timer expires.
        DelayList.AddByDelay(msg);
    }

    // Return the msg that was sent.
    return msg;
}

//-----------------------------------------------------------------------------
/// \brief Determine if a task exists.
///
/// \param receiver - A pointer to the task to check for existence.
///
/// \return true if the task exists, false otherwise.
//-----------------------------------------------------------------------------
bool TaskClass::TaskExists(TaskClass* receiver)
{
    TaskClass * task = receiver;

    // If no parameter was specified, then make the task this task.
    if (task == 0) {
        task = this;
    }
  
    // Return true if the task exists in the Task List.
    return TaskList.TaskExists(task);
}

//-----------------------------------------------------------------------------
/// \brief The IdleTask.
///
/// The IdleTask runs when no other tasks are ready to run. It continuously
/// checks to see if a task has been added to the Ready List, in which case,
/// it suspends itself so that the other tasks can run. It also checks for
/// timeouts and interrupts.
//-----------------------------------------------------------------------------
void IdleTaskClass::Task()
{
    for (;;) {

        // Check for timeouts and interrupts.
        CheckForSystemEvents();

        // Check to see if there are any tasks that are ready to run.
        if (ReadyList.IsEmpty() == false) {
            // There is work to do, so suspend this task and run the task at the front of the Ready List.
            Suspend();
        }
        else {

            // There is no work to do, since the Ready List is empty.
            //
            // If you want to save power, this is where you would put your "sleep" 
            // instruction. It's your choice as to whether the hardware timer should 
            // be kept running when in sleep mode.
            //
            // Otherwise, do nothing here and the system will continuously poll for
            // system events - this is the mode we recommend as it's simpler.
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief The Tics System Task. This is a general purpose task that
/// we may add cmds to in the future.
///
/// We envision that the system task may be useful in the future. Currently,
/// it simply accepts a request to delete a task. Note that a task can be
/// deleted by any task except the task itself, so there is no need to send
/// a message here to delete a task.
//-----------------------------------------------------------------------------
void TicsSystemTaskClass::Task()
{
    MsgClass * msg;
    TaskClass * task;

    for (;;) {

        // Wait for a request msg.
        msg = Wait();

        // Process the request.
        switch (msg->MsgNum) {
        
        case DeleteTaskMsg:
            task = (TaskClass*)msg->Ptr;
            if (TaskExists(task) == false) {
                ErrorHandler.Report(ErrorTaskDoesNotExist);
            }
            else {
                delete task;
            }
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Copy a block of memory.
///
/// \param dst - Where to copy to.
/// \param src - Where to copy from.
/// \param numChars - The number of bytes to copy.
//-----------------------------------------------------------------------------
void TicsUtilsClass::MemCopy(void* dst, void* src, int numChars)
{
    int i;
    int temp;

    // Consolidate the lower bits.
    temp = (int)dst | (int)src | numChars;

    // If all of the above are multiples of sizeof(int), (lower 2 bits are 0), then copy int's.
    if ((temp & (sizeof(int) - 1)) == 0) {

        int numInts = numChars / (int) sizeof(int);
        int* d = (int*) dst;
        int* s = (int*) src;

        for (i = 0; i < numInts; i++) {
            *d++ = *s++;
        }
    }
    else {
        char* d = (char*) dst;
        char* s = (char*) src;

        for (i = 0; i < numChars; i++) {
            *d++ = *s++;
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Copy a byte to a block of memory.
///
/// \param dst - Where to copy to.
/// \param numChars - The number of bytes to copy.
/// \param data - The byte to copy.
//-----------------------------------------------------------------------------
void TicsUtilsClass::MemSet(void* dst, int numChars, char data)
{
    int i;
    char* d = (char*) dst;

    for (i = 0; i < numChars; i++) {
        *d++ = data;
    }
}

//-----------------------------------------------------------------------------
/// \brief Fifo class constructor.
///
/// \param slotSizeInBytes - The size of the fifo array item (slot).
/// \param numSlots - The number of array items.
/// \param fifoSpace - A pointer to an area at least 
/// (slotSizeInBytes * numSlots) in size that will house the fifo slots. If
/// this parameter is 0, then the constructor will allocate space.
//-----------------------------------------------------------------------------
FifoClass::FifoClass(
    int slotSizeInBytes, 
    int numSlots,
    void* fifoSpace) : 
    FifoSpaceWasAllocated(false),
    FifoSpace(fifoSpace)
{
    // We must have at least 2 slots. One is wasted, the other holds data.
    if (numSlots < 2) {
        ErrorHandler.Report(ErrorMusthaveAtLeastTwoFifoSlots);
    }

    // Assign slot size.
    SlotSizeInBytes = slotSizeInBytes;

    // Assign the total number of slots in the fifo, including the unused slot.
    NumSlots = numSlots;

    // The number of bytes in the fifo.
    FifoSizeInBytes = SlotSizeInBytes * NumSlots;

    // Allocate fifo memory if the user did not specify it.
    if (FifoSpace == 0) {

        // Allocate fifo space.
        FifoSpace = MemoryMgr.Allocate(FifoSizeInBytes);

        // Check for errors.
        if (FifoSpace == 0) {
            ErrorHandler.Report(ErrorCannotAllocateFifoMemory);
        }

        // Remember that we allocated fifo space.
        FifoSpaceWasAllocated = true;
    }
    else {
        // Remember that we did not allocate fifo space.
        FifoSpaceWasAllocated = false;
    }

    // Point to the last fifo byte. Used to determine when to wrap around the fifo pointers.
    LastFifoByte = (char*)FifoSpace + FifoSizeInBytes - 1;

    // Point front and rear pointers to the first item in the fifo.
    Front = Rear = FifoSpace;

    // Init the number of items in the fifo.
    NumItemsInFifo = 0;

}


//-----------------------------------------------------------------------------
/// \brief Fifo class destructor
///
/// Free the fifo array space if it was allocated (as opposed to being provided
/// in the constructor parameter list).
//-----------------------------------------------------------------------------
FifoClass::~FifoClass()
{
    // Free the fifo space that was allocated in the constructor.
    if (FifoSpaceWasAllocated) {
        MemoryMgr.DeAllocate(FifoSpace);
    }
}

//-----------------------------------------------------------------------------
/// \brief Increment the parameter and return it, applying wrap-around when the end of the
/// fifo array is reached. 
///
/// This function is used by the Add and Remove methods to advance the front
/// and rear pointers.
///
/// \param item -	The pointer to increment.
/// \returns		Returns the incremented pointer with wrap-around applied
///					as necessary.
//-----------------------------------------------------------------------------
void* FifoClass::Bump(void* item)
{
    char * nextItemPtr;

    nextItemPtr = (char*)item + SlotSizeInBytes;

    if (nextItemPtr > LastFifoByte) {
        return FifoSpace;
    }
    else {
        return nextItemPtr;
    }
}

//-----------------------------------------------------------------------------
/// \brief Adds an item to the fifo. If the fifo is full, an error is generated.
///
/// \param item - A pointer to the item to add to the fifo.
//-----------------------------------------------------------------------------
void FifoClass::Add(void* item)
{
    // If the fifo is full, then error.
    if (IsFull()) {
        ErrorHandler.Report(ErrorAttemptToAddToAFullFifo);
    }

    // Advance to the next slot.
    Rear = Bump(Rear);


    // Copy the item to the slot.
    TicsUtilsClass::MemCopy(Rear, item, SlotSizeInBytes);

    // Bump the number of items in the fifo.
    NumItemsInFifo++;
}

//-----------------------------------------------------------------------------
/// \brief Returns a pointer to the current fifo item to be removed.
///
/// The item contents must be used on return, otherwise
/// the slot may be overwritten after a task switch.
///
/// \returns Returns 0 if the fifo is empty, otherwise it returns
/// a pointer to the next item for removal.
//-----------------------------------------------------------------------------
void* FifoClass::Remove(void)
{
    //MDM Check for reentancy.
    static int reentrant = 0;

    // If there are no items in the fifo, then return.
    if (IsEmpty()) {
        // No items to remove.
        return 0;
    }

    // Advance to the next item.
    Front = Bump(Front);

    //MDM Remove the following if test.
    if (NumItemsInFifo <= 0) {
        NumItemsInFifo = NumItemsInFifo;
    }

    // Update the number of items in the fifo.
    NumItemsInFifo--;

    //MDM Remove this.
    reentrant--;

    // Return the item at the front of the fifo.
    return Front;
}

//-----------------------------------------------------------------------------
/// \brief Copies the next fifo item to remove into the parameter.
///
/// \return Zero if the fifo is empty, otherwise, a pointer to the removed item. 
//-----------------------------------------------------------------------------
void* FifoClass::Remove(void* item)
{
    // If there are no items in the fifo, then return.
    if (IsEmpty()) {
        // No items to remove.
        return 0;
    }

    // Copy the slot to the item.
    TicsUtilsClass::MemCopy(item, Remove(), SlotSizeInBytes);

    return item;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the fifo is empty, false otherwise.
//-----------------------------------------------------------------------------
bool FifoClass::IsEmpty()
{
    // If Front == Rear, then there are no items in the fifo.
    return Front == Rear;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the fifo is not empty, false otherwise.
//-----------------------------------------------------------------------------
bool FifoClass::IsNotEmpty()
{
    // If Front != Rear, then there are one or more items in the fifo.
    return Front != Rear;
}

//-----------------------------------------------------------------------------
/// \brief Returns true if the fifo is full, false otherwise.
//-----------------------------------------------------------------------------
bool FifoClass::IsFull()
{
    // If advancing Rear points it to Front, then the fifo is full.
    return Bump(Rear) == Front;
}

//-----------------------------------------------------------------------------
/// \brief Returns the number of items in the fifo.
//-----------------------------------------------------------------------------
int FifoClass::NumItems()
{
    //MDM Remove the following if statement.
    if (NumItemsInFifo < 0) {
        NumItemsInFifo = NumItemsInFifo;
    }
    return NumItemsInFifo;
}

//-----------------------------------------------------------------------------
/// \brief Performs all necessary fifo resets.
//-----------------------------------------------------------------------------
void FifoClass::Reset()
{
    // Reset the front and rear pointers.
    Front = Rear = FifoSpace;
};

//-----------------------------------------------------------------------------
/// \brief All errors vector here.
///
/// The default behavior is to loop indefinitely. The user can modify this
/// as he wishes. However, this function should never return, since most
/// errors are indicators of a catastrophic system failure, and no further
/// processing should occur. Note also that there are no explanations of the
/// error numbers. The source of the error is determined by searching the code
/// for the unique error number.
/// 
/// You can also choose to replace the body of this function with a "throw"
/// to be caught in main() typically, or in an initialization function.
///
/// \param - The error number reported from the caller.
//-----------------------------------------------------------------------------
void ErrorHandlerClass::Report(int errorNum)
{
    volatile int count = 0;

    for (;;) {
        // For use while in the debugger.Set count to 1 to break.
        if (count == 1) {
    		break;
    	}
    }
}

//-----------------------------------------------------------------------------
/// \brief Add a memory node to a list.
///
/// \param node - The memory node to add.
//-----------------------------------------------------------------------------
void MemNodeListClass::Add(MemNodeClass * node)
{
    // If the list is empty, then assign the head to this node.
    if (IsEmpty()) {
        Head = node;
        node->Next = 0;
    }
    else {
        // Make node the new first node in the list.
        node->Next = Head;

        // Head points to the first node in the list.
        Head = node;
    }

    // Bump the list node count, since we just added a new node.
    NumNodesInList++;
}

//-----------------------------------------------------------------------------
/// \brief Remove a node with the specified block size.
///
/// \param numBytesRequested - The block size of the requested node.
///
/// \return A pointer to the node if found, otherwise, 0.
//-----------------------------------------------------------------------------
MemNodeClass * MemNodeListClass::Remove(int numBytesRequested)
{
    MemNodeClass * node;
    MemNodeClass * prevNode = 0;

    // If the list is empty, we can't remove.
    if (IsEmpty()) {
        return 0;
    }

    // Traverse the list looking for a node the same size as the
    // number of bytes requested and return a pointer to it if found.
    for (node = Head; node != 0; node = node->Next) {

        // See if this node can accommodate the number of bytes requested.
        if (node->NumBytesRequested == numBytesRequested) {

            // Decrement the number of nodes in the list, since we'll be removing the node.
            NumNodesInList--;

            // If we're at the first node, then point Head to the next node.
            if (prevNode == 0) {
                // Since we'll be removing the head, the next node becomes the new Head.
                Head = node->Next;
            }
            else {
                // If we're in the middle of the list, then the next pointer of the previous
                // node should point around the node we're removing.
                prevNode->Next = node->Next;
            }

            // Return the node.
            return node;
        }

        // The current node becomes the previous node for our next iteration.
        prevNode = node;
    }

    // No matches were found, so return 0.
    return 0;
}

//-----------------------------------------------------------------------------
/// \brief Round up desired number of bytes to allocate to an aligned value.
///
/// \param numBytesRequested - The size of the desired memory block.
///
/// \return The number of bytes to allocate.
//-----------------------------------------------------------------------------
int MemAiMgrTaskClass::NumBytesToAllocate(int numBytesRequested)
{
    unsigned int mask;
    int numBytesToAllocate;
    int allocationWordSizeInBytes;

    // Use the larger of StackType or int as memory boundary granularity.
    if (sizeof(StackType) > sizeof(int)) {
        allocationWordSizeInBytes = (int) sizeof(StackType);
    }
    else {
        allocationWordSizeInBytes = sizeof(int);
    }

    // We'll use this in the next few operations.
    mask = (unsigned int) (allocationWordSizeInBytes - 1);

    // Adjust for the node header and add "mask" bytes to adjust if not on a word boundary.
    numBytesToAllocate = numBytesRequested + (int) sizeof(NodeHeaderClass) + (int) mask;

    // Round down to make sure we are on a word boundary.
    numBytesToAllocate &= ~mask;

    // Return the number of bytes requested plus overhead.
    return numBytesToAllocate;
}

//-----------------------------------------------------------------------------
/// \brief Allocate a memory block.
///
/// \param numBytesRequested - The number of bytes requested.
///
/// \return A pointer to the memory block.
//-----------------------------------------------------------------------------
void * MemAiMgrTaskClass::Allocate(int numBytesRequested)
{
    MemNodeClass * node;

    // Try to allocate from a list first, since it's faster and preserves free memory space.
    if ((node = AllocateFromList(numBytesRequested)) != 0) {
        // We need to preserve the node header, so the user's free space begins below the header.
        return node->UserArea();
    }
    else if ((node = AllocateFromMemory(numBytesRequested)) != 0) {
        // We need to preserve the node header, so the user's free space begins below the header.
        return node->UserArea();
    }
    else {
        // Couldn't allocate, so report an error.
        ErrorHandler.Report(ErrorCouldNotAllocateMemory);
    }

    // No memory was found, so return 0. (To satisfy the compiler,
    // since this statement will never be reached (we don't
    // return from the error handler).)
    return 0;
}

//-----------------------------------------------------------------------------
/// \brief Allocate a fixed block from a memory block list.
///
/// \param numBytesRequested - The block size of the requested node.
///
/// \return A pointer to the node if found, otherwise, 0.
//-----------------------------------------------------------------------------
MemNodeClass * MemAiMgrTaskClass::AllocateFromList(int numBytesRequested)
{
    // If the list is not empty, return the node at the front of the list,
    // otherwise, if the list is empty, 0 is returned.
    return NodeList.Remove(numBytesRequested);
}

//-----------------------------------------------------------------------------
/// \brief Create a new memory block and add it to the memory block list.
///
/// \param numBytesRequested - The block size of the requested memory block.
///
/// \return A pointer to the memory node if found, otherwise, 0.
//-----------------------------------------------------------------------------
MemNodeClass * MemAiMgrTaskClass::AllocateFromMemory(int numBytesRequested)
{
    char * p;
    int numBytesToAllocate;
    MemNodeClass * node;

    // Add sizeof(NodeHeaderClass) because we need space for the header.
    numBytesToAllocate = NumBytesToAllocate(numBytesRequested);

    // numBytesToAllocate must be in multiples of words.
    if ((numBytesToAllocate & (sizeof(int) - 1)) != 0) {
        ErrorHandler.Report(ErrorByteAllocationRequestMustBeInMultiplesOfWords);
    }

    // Compute the number of bytes available in the memory block.
    NumBytesAvailable = MemorySizeInBytes - CurrentOffset;

    // If this allocation will go past the end of the memory block, then return 0.
    if (numBytesToAllocate <= NumBytesAvailable) {

        // The current offset is the start of the allocated memory.
        p = &Memory[CurrentOffset];

        // Update the offset.
        CurrentOffset += numBytesToAllocate;

        // Cast to a node so that we can initialize the header.
        node = (MemNodeClass *)p;

        // Initialize the header.
        node->Initialize(numBytesRequested, this);

        // Return the newly allocated memory block.
        return node;
    }
    else {
        // No memory could be allocated, so return 0.
        return 0;
    }
}

//-----------------------------------------------------------------------------
/// \brief Add a block that is no longer in use back to the list.
///
/// \param p - A pointer to the memory block, not including the header.
//-----------------------------------------------------------------------------
void MemAiMgrTaskClass::DeAllocate(void* p)
{
    // Point to the top of the node.
    MemNodeClass * node = (MemNodeClass*)((char *)p - sizeof(NodeHeaderClass));

    // Check the signature to make sure that the node has not been corrupted.
    if (node->SignatureMatches() == false) {
        ErrorHandler.Report(ErrorDeAllocationSignatureMismatch);
    }

    // Make sure that we're deallocating to the proper pool.
    if (node->MemoryMgrMatches(this) == false) {
        ErrorHandler.Report(ErrorAttempToDeAllocateToTheWrongPool);
    }

    // Add the node into the free list.
    NodeList.Add(node);
}

//-----------------------------------------------------------------------------
/// \brief Memory block manager constructor.
///
/// \param memory - A pointer to the space to be used for memory block allocation.
/// \param memorySizeInBytes - The size of memory pointed to by parameter 1.
//-----------------------------------------------------------------------------
MemAiMgrTaskClass::MemAiMgrTaskClass(void * memory, int memorySizeInBytes) :
    Memory((char *)memory), CurrentOffset(0),
    MemorySizeInBytes(memorySizeInBytes), NumBytesAvailable(0)
{
}

//-----------------------------------------------------------------------------
/// \brief Apply various checks to make sure that the list has not been corrupted.
/// 
/// This function is only called if we are in SafeMode.
//-----------------------------------------------------------------------------
void ListClass::CheckListIntegrity(void)
{
    NodeClass* node;

    // Check head and tail pointers.
    if (Head != &ActualHead || Tail != &ActualTail) {
        ErrorHandler.Report(ErrorMsgListHeadOrTailCorruption);
    }

    // Head->Prev should point to the tail, and Tail->Next should point to the head.
    if (Head->Prev != Tail || Tail->Next != Head) {
        ErrorHandler.Report(ErrorMsgListHeadOrTailLinkageIssue);
    }

    // Check head and tail priorities.
    if (Head->Priority != HeadPriority || Tail->Priority != TailPriority) {
        ErrorHandler.Report(ErrorMsgListHeadOrTailPriorityIssue);
    }

    // Make sure that we can traverse the list and check each node.
    for (node = Head; ; node = node->Next) {

        /* MDM 
        // Make sure that the node has not been inadvertently altered.
        node->VerifyChecksum();

        // Make sure that the node is still at its original address.
        if (node != node->OriginalThis) {
            ErrorHandler.Report(ErrorMsgListIntegrityCheckMsgCorruption);
        }
        */

        // If we're at the end of the list, then break.
        if (node == Tail) {
            break;
        }
    }
}

//-----------------------------------------------------------------------------
/// \brief Make checks prior to inserting a msg into a list. Checks for 
/// proper insertion of msg a after msg b.
///
/// \param a - The msg to add.
/// \param b - the msg to add after.
//-----------------------------------------------------------------------------
void ListClass::DoInsertSafetyChecks(NodeClass* a, NodeClass* b)
{
    // Both msgs must be defined.
    if (a == 0 || b == 0) {
        ErrorHandler.Report(ErrorMsgNotDefined);
    }

    /* MDM
    // Msgs must always define a sender and receiver.
    if (a->Sender == 0 && a->Receiver == 0) {
        ErrorHandler.Report(ErrorSenderOrReceiverNotDefined);
    }

    // If msg a is already in a list, then we can't insert it.
    if (a->IsInAList()) {
        ErrorHandler.Report(ErrorMsgIsAlreadyInAList);
    }

    // Msg b must be in a list.
    if (b->IsInAList() == false) {
        ErrorHandler.Report(ErrorDestinationMsgIsNotInAList);
    }

    */

    // Msgs a and b cannot both point to the same msg.
    if (a == b) {
        ErrorHandler.Report(ErrorBothArgsPointToTheSameMsg);
    }

    // Msg a can't be the head or tail.
    if (a->Priority >= Head->Priority || a->Priority <= Tail->Priority) {
        ErrorHandler.Report(ErrorMsgCannotBeTheHeadOrTail);
    }

    // Msg b can't be the Tail - you can't add after the Tail.
    if (b == Tail) {
        ErrorHandler.Report(ErrorDestinationMsgCannotBeTheTail);
    }

    // Make sure that the msg has the correct list id.
    if (a->ListIdIsValid(Id) == false) {
        ErrorHandler.Report(ErrorListIdIsInvalid);
    }

    // Make sure that the list is intact.
    CheckListIntegrity();
}

