/*

MIT License

Copyright (c) 2025 Michael Dennis McDonnell

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

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
// Copyright (c) 2025, Tics Realtime (Michael Dennis McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Start guard
//-----------------------------------------------------------------------------
#ifndef TicsGuard
#define TicsGuard

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdint.h>

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef unsigned int StackType;
typedef int32_t TimerTickType;

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define InRange(minValue, maxValue, value) (value <= maxValue && value >= minValue)

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
namespace TicsNameSpace {

    enum TicsNamespaceEnum {
        // The size of the Tics dynamic memory space.
        SizeMemoryMgr = 0x100000,
        // The default number of interrupt fifo slots.
        NumInterruptFifoSlots = 16,
        // The default size for an interrupt fifo slot.
        InterruptFifoSlotSize = 8,
        // The maximum number of msgs allowed in the array 
        // passed to the array version of TaskClass::Wait(int* msgs);
        MaxAllowedMsgsInRecv = 8,
        // Array end marker. A general marker used to mark the end
        // of an array.
        ArrayEndMarker = 99999,
         // TicsNameSpace flags.
        SafeModeFlag = 1, WatchDogFlag = 2
   };

    // Users can use any priority between LowPriority and HighPriority. 
    // For example, MyPriority = LowPriority + 1.
    enum NodePriorityEnum {
        TailPriority = 0, IdleTaskPriority = 1, LowPriority = 1000,
        MediumPriority = 3000, MediumHighPriority = 3001, HighPriority = 4000, HeadPriority = 10000
    };

    // Tics reserves msg numbers 0 to 999. Users can create their own
    // msg numbers in the range Users can define their own msg numbers 
    // in the range MinUserMsgNum to MaxUserMsgNum.
    enum MsgNumEnum {
        // This must be the first and smallest defined msg number.
        FirstMsgNum = 0,

        // 1        2               3               4               5
        NullMsg,    AnyMsg,         RunMsg,         GoMsg,          StartMsg,
        StopMsg,    DoneMsg,        ScheduleMsg,    HelloMsg,       RqstMsg,
        GrantMsg,   TimeoutMsg,     WakeupMsg,      AskMsg,         ReplyMsg,
        OkayMsg,    DeleteTaskMsg,  NotifyMsg,      OnMsg,          OffMsg,
        IsrMsg,     ResetMsg,       StatusMsg, SuccessMsg,          FailMsg,
        InvalidMsg,
        
        // Users can define their own msg numbers in the range between
        // MinUserMsgNum and MaxUserMsgNum.
        MinUserMsgNum = 1000,
        MaxUserMsgNum = 9999
    };

    // Error numbers. The explanation should be clear from the error name.
    // Don't confuse these with the inter-task communication msgs defined
    // above. These are just numbers that are passed to 
    // ErrorHandlerClass::Report(int errorNum).
    enum ErrorMsgEnum {
        ErrorMsgNotDefined = 1001,
        ErrorSenderOrReceiverNotDefined = 1002,
        ErrorMsgIsAlreadyInAList = 1003,
        ErrorDestinationMsgIsNotInAList = 1004,
        ErrorBothArgsPointToTheSameMsg = 1005,
        ErrorMsgCannotBeTheHeadOrTail = 1006,
        ErrorDestinationMsgCannotBeTheTail = 1007,
        ErrorListIdIsInvalid = 1008,
        ErrorCannotUnlinkFromAnEmptyList = 1009,
        ErrorCannotUnlinkAnInvalidMsg = 1010,
        ErrorMsgIsNotTheOriginalMsg = 1011,
        ErrorMsgToUnlinkIsNotInTheList = 1012,
        ErrorMsgToUnlinkIsNotInTheList2 = 1013,
        ErrorMsgToUnlinkIsInTheWrongList = 1014,
        ErrorUnlinkListIdFailure = 1015,
        ErrorAttemptToUnlinkHeadOrTail = 1016,
        ErrorMsgChecksumFailure = 1017,
        ErrorCannotAddAMsgToAFullList = 1018,
        ErrorCannotAddANullMsg = 1019,
        ErrorCannotAddAMsgThatIsAlreadyInAnotherList = 1020,
        ErrorCannotRemoveANullMsg = 1021,
        ErrorCannotRemoveANodeFromAnEmptyList = 1022,
        ErrorCannotRemoveTheHeadOrTailMsg = 1023,
        ErrorCannotRemoveAMsgIfItIsNotInAList = 1024,
        ErrorCouldNotAllocateMemory = 1025,
        ErrorByteAllocationRequestMustBeInMultiplesOfWords = 1026,
        ErrorDeAllocationSignatureMismatch = 1027,
        ErrorAttemptToDeallocateToTheWrongPool = 1028,
        ErrorDefaultStackSizeOutOfRange = 1029,
        ErrorCurrentSpIsBelowStackBottom = 1030,
        ErrorCurrentSpIsAboveStackTop = 1031,
        ErrorStackOverFlow = 1032,
        ErrorStackPadAreaWasWrittenTo = 1033,
        ErrorTheNextTaskToRunPtrIsNull = 1034,
        ErrorTheNextTaskToRunDoesNotExist = 1035,
        ErrorReturningFromATaskIsNotAllowed = 1036,
        ErrorMsgListIsFullCannotInsert = 1037,
        ErrorMsgListHeadOrTailCorruption = 1038,
        ErrorMsgListHeadOrTailLinkageIssue = 1039,
        ErrorMsgListHeadOrTailPriorityIssue = 1040,
        ErrorMsgListIntegrityCheckMsgCorruption = 1041,
        ErrorAttemptToRemoveAMsgFromAnEmptyList = 1042,
        ErrorNullTaskPointerInTaskExists = 1043,
        ErrorNullTaskPointerInSchedule = 1044,
        ErrorTaskDoesNotExistInSchedule = 1045,
        ErrorNullTaskPtrInCheckForInterrupts = 1046,
        ErrorAttemptToDeleteANonexistentTask = 1047,
        ErrorAttemptToDeleteTheCurrentTask = 1048,
        ErrorAttemptToDeleteASystemTask = 1049,
        ErrorAttemptToDeleteACorruptedMsg = 1050,
        ErrorMsgPriorityIsOutOfRange = 1051,
        ErrorInvalidMsgDelay = 1052,
        ErrorMsgReceiverTaskPtrIsNull = 1053,
        ErrorMsgSenderTaskPtrIsNull = 1054,
        ErrorReceiverTaskPtrIsNullInSend = 1055,
        ErrorReceiverTaskDoesNotExistInSendSafetyChecks = 1056,
        ErrorTaskDoesNotExist = 1057,
        ErrorMustHaveAtLeastTwoFifoSlots = 1058,
        ErrorCannotAllocateFifoMemory = 1059,
        ErrorAttemptToAddToAFullFifo = 1060,
        ErrorTaskIdMismatch = 1061,
        ErrorTaskIdMismatchCorruptedMsg = 1062,
        ErrorNullPointer = 1063,
        ErrorMaxAllowedMsgsInRecv = 1064,
    };
};

//-----------------------------------------------------------------------------
// Namespaces 
//-----------------------------------------------------------------------------
using namespace TicsNameSpace;

//-----------------------------------------------------------------------------
// References 
//-----------------------------------------------------------------------------
class TaskClass;
class FifoClass;
class MemNodeClass;
class MemMgrClass;

//-----------------------------------------------------------------------------
// TicsBaseClass
//
// Base class. Various classes are derived from the base class.
//-----------------------------------------------------------------------------
class TicsBaseClass {
public:
    // Data
    static int IdCounter;
    int Id;

    // Functions
    // TicsBaseClass constructor.
    TicsBaseClass(void)
    {
        // Object Id starts at 1. Zero is used to indicated that the Id has not been assigned.
        Id = ++IdCounter;
    }

    // TicsBaseClass destructor.
    ~TicsBaseClass(void)
    {
        // Bump the Id to indicate that the object has been deleted.
        Id++;
    }
};

//-----------------------------------------------------------------------------
// NodeClass
// 
// The base class from which all list node classes are derived.
//-----------------------------------------------------------------------------
class NodeClass : public TicsBaseClass {
public:
    // Data
    // Pointer to the next node in the list.
    NodeClass* Next;
    // Pointer to the previous node in the list.
    NodeClass* Prev;
    // 
    int Data;
    void* Ptr;
    int ListId = 0;
    int Priority;
    
    NodeClass(int data = 0, void* ptr = 0, int priority = MediumPriority) : 
        Next(0), Prev(0), Data(data), Ptr(ptr), ListId(0), Priority(priority)
    {
    }

    virtual ~NodeClass(void)
    {
    }

    bool ListIdIsValid(int listId)
    {
        // A ListId value of 0 means that the msg is not in a list.
        return ListId == 0 || listId == ListId;
    }

    bool IsInAList()
    {
        return ListId != 0;
    }

    void SetAsInAList(int listId)
    {
        ListId = listId;
    }

    void SetAsNotInAList()
    {
        ListId = 0;
    }
};

class FlagsClass {
public:
    // Data
    int Flags;

    // Functions
    // FlagsClass constructor. All flags are initialized to 0.
    FlagsClass(int flags = 0) : Flags(flags) {}
    // Set one or more flags with a mask.
    void Set(int mask) { Flags |= mask; }
    // Clear one or more flags with a mask.
    void Clr(int mask) { Flags &= (~mask); }
    // Check if one or more flags are set.
    bool IsSet(int mask) { return Flags & mask; }
    // Check if one or more flags are clear.
    bool IsClr(int mask) { return IsSet(mask) ? false : true; }
};


//-----------------------------------------------------------------------------
/// MsgClass
///
///  Used to send msgs between tasks.
//-----------------------------------------------------------------------------
class MsgClass : public NodeClass {
    // Data
public:
    // The id of the task that the msg will be sent to.
    int ReceiverId;
    // The msg number which will tell the receiving task what to do.
    int MsgNum;
    // The msg will by sent after waiting "Delay" system ticks.If Delay is 0, send immediately.
    TimerTickType Delay;
    // The system tick time at which the msg will be sent (Current time + Delay).
    TimerTickType EndTime;
    // The task that is sending the msg.
    TaskClass* Sender;
    // The task to which the msg will be sent.
    TaskClass* Receiver;

    // Functions
public:
    // Returns a block from the appropriate fixed block pool.
    void* operator new(size_t size);
    // Returns a block to the appropriate fixed block pool.
    void operator delete(void* p);
    // MsgClass constructor.
    MsgClass(
        // The task to which the msg will be sent.
        TaskClass* receiver,
        // The default msg that will be sent to the receiver task.
        int msgNum = StartMsg,
        // Optional msg data.
        int data = 0,
        // Optional pointer to msg data.
        void* ptr = 0,
        // The system tick time at which the msg will be sent (Current time + Delay).
        int delay = 0,
        // The priority of the msg.
        int priority = MediumPriority,
        // The sender of the msg.
        TaskClass* sender = 0);
        //  MsgClass destructor. 
        virtual ~MsgClass();
        // Performs dynamic initialization of the msg.
        void Init();
        // Validates the args.
        void CheckParameters(bool fullCheck = true);
        // Returns true if the arg matches the msg number.
        bool Is(int msgNum)
    {
        return msgNum == MsgNum ? true : false;
    }
};

//-----------------------------------------------------------------------------
//  List class.
//
// Manages a doubly linked list, ordered by priority.
//-----------------------------------------------------------------------------
class ListClass : public TicsBaseClass {
public:
    enum ListClassEnum {
        // The default number of nodes allowed in the list.
        DefaultMaxNodes = 32
    };
    // The current number of nodes in the list.
    int NumNodesInList;
    // The maximum number of nodes allowed in the list.
    int MaxNodes;
    // The head of the list.
    NodeClass ActualHead;
    // The tail of the list.
    NodeClass ActualTail;
    // A pointer to the head of the list.
    NodeClass* Head;
    // A pointer to the tail of the list.
    NodeClass* Tail;

    // Functions
    // The ListClass constructor.
    ListClass(int maxNodes = DefaultMaxNodes);
    // Returns true if the arg is the head.
    bool IsHead(NodeClass* a)
    {
        return a == Head;
    }
    // Returns true if the arg is the tail.
    bool IsTail(NodeClass* a)
    {
        return a == Tail;
    }
    // Unlinks the node from the list.
    NodeClass * Unlink(NodeClass* a = 0);
    // Returns true if the list is empty.
    bool IsEmpty(void);
    // Returns true if the list is not empty.
    bool IsNotEmpty(void);
    // Returns true if the list is full.
    bool IsFull(void);
    // Inserts node a after node b.
    void Insert(NodeClass* a, NodeClass* b);
    // Adds the node to the list according to its priority.
    void AddByPriority(NodeClass* a);
    // Adds the node to the end of the list.
    void Add(NodeClass* a);
    // Unlinks the node from the list. Defaults to the first node in the list.
    NodeClass* Remove(NodeClass* a = 0);
    // Remove and delete all the items in the list.
    void Flush();
    // Remove and delete all occurrences of a node from the list.
    bool Delete(int id);
   // Remove and delete all occurrences of a node from the list.
    bool Delete(NodeClass* node);
    // Run various checks on the list.
    void CheckListIntegrity(void);
    //  Run various check prior to inserting a node into a list.   
    void DoInsertSafetyChecks(NodeClass* a, NodeClass* b);
};

//-----------------------------------------------------------------------------
// MsgList class.
//
// Manages a list of MsgClass items.
//-----------------------------------------------------------------------------
    class MsgListClass : public ListClass {
    public:
        // Remove all references to a particular class from the list.
        bool RemoveTaskReferences(TaskClass* task);
};

//-----------------------------------------------------------------------------
// Task List class.
//
// A list of all tasks currently in the system.
//-----------------------------------------------------------------------------
class TaskListClass : public ListClass {
public:

    // Functions
    // Remove all task references from the task list.
    void RemoveTaskReferences(TaskClass* task, bool removeTheTaskItselfAlso = false);
    // Remove the task from the task list.
    void RemoveTask(TaskClass* task);
    // Add the task to th task list.
    void Add(TaskClass* task);
    // Return true if the task exists.
    bool TaskExists(TaskClass* task, int id = 0);
    // Return true if the task exists.
    bool TaskExists(int taskId);
};

//-----------------------------------------------------------------------------
// Delay List class.
//
// A list of all delayed tasks currently in the system.
//-----------------------------------------------------------------------------
class DelayListClass : public MsgListClass {
public:
    // Data
    // The system tick count at the time of the last CheckForTimeouts() call.
    TimerTickType LastTime;

    // Functions
    // Adds a msg to be sent out msg->Delay ticks later.
    void AddByDelay(MsgClass* a);
    // Checks the DelayList for expired msgs and sends them.
    void CheckForTimeouts();
};

//-----------------------------------------------------------------------------
// Stack class.
//
// This class manages a task's stack.
//-----------------------------------------------------------------------------
class StackClass {
public:
    // Data
    enum StackClassEnum {
        // Default stack size.
        DefaultStackSizeInBytes = (1024 * 4),
        // The pad is a warning area at the end of stack memory. 
        DefaultStackPadSizeInBytes = 128,
        // The stack must be at least this large.
        MinStackSizeInBytes = 2048,
        // The stack must not exceed this size.
        MaxStackSizeInBytes = (MinStackSizeInBytes * 16),
        // This pattern is written to the pad area as a visual aid.
        DefaultStackPadBytePattern = 0x22,
        // This pattern is written to the pad area as a visual aid.
        DefaultStackPadWordPattern = 0x22222222,
    };

    // Stack size in bytes.
    int StackSizeInBytes;
    // Stack pad size in bytes.
    int StackPadSizeInBytes;
    // Pointer to the top of the stack.
    StackType* StackTop;
    // Pointer to the bottom of the stack.
    StackType* StackBottom;
    // The stack pointer of a task prior to performing a context switch.
    StackType* SavedSp;

    // Functions
    // StackClass constructor.
    StackClass(
        int stackSizeInBytes = DefaultStackSizeInBytes,
        int stackPadSizeInBytes = DefaultStackPadSizeInBytes
    );
    // StackClass destructor.
    ~StackClass();

    // Checks the stack for validity.
    void Check();
};

//-----------------------------------------------------------------------------
// Fifo class.
//
// Manages a circular fifo queue.
//-----------------------------------------------------------------------------
class FifoClass {
public:
    // Data

    enum FifoClassEnum {
        DefaultNumFifoItems = 16
    };

    // Flag to indicate that the fifo space was malloc'ed.
    bool FifoSpaceWasAllocated;
    // Pointer to the oldest item in the fifo. Items are removed from the front.
    void* Front;
    // Pointer to the newest item in the fifo. Items are added to the rear.
    void* Rear;
    // This is a pointer to the start of the space where the fifo lives.
    void* FifoSpace;
    // The last valid byte in the fifo.
    char* LastFifoByte;
    // The size of a fifo slot.
    int SlotSizeInBytes;
    // The total number of slots in the fifo.
    int NumSlots;
    // The total size of the fifo space in bytes.
    int FifoSizeInBytes;
    // The number of items currently in the fifo.
    int NumItemsInFifo;

    // Functions
private:
    void* Bump(void* item);
    void* Remove(void);
public:
    FifoClass(int itemSizeInBytes, int numItems = NumInterruptFifoSlots, void* fifoSpace = 0);
    ~FifoClass();
    void Add(void* item);
    void* Remove(void* item);
    bool IsEmpty();
    bool IsNotEmpty();
    bool IsFull();
    int NumItems();
    void Reset();
    void* operator new(size_t size);
    void operator delete(void* p);
};

//-----------------------------------------------------------------------------
// TaskClass
//-----------------------------------------------------------------------------
class TaskClass : public NodeClass {
public:
    // Data

    enum TaskClassEnum {
        // Flag that tells whether a task has been started for the first time or not.
        TaskStartedFlag = 1,
        // If set, the task that received the msg will quietly drop it.
        DropUnexpectedMsgsFlag = 2,
        // If set, a task will be scheduled to run after it is created.
        ScheduleTaskOnCreationFlag = 4,
        // The default numTicks in the Pause(numTicks) member function.
        DefaultNumTicks = 1000
    };

    // Flag word that contains various flag bits.
    FlagsClass Flags;
    // Every task needs its own stack.
    StackClass Stack;
    // Optional task name. Used in debugging.
    const char* Name;
    // When a msg is added to the Ready List where this task is the receiver, 
    // the msg priority is set to this->Priority.
    int Priority;
    // When a msg is removed from the Ready List, it is inserted into
    // the receiver task's MsgList according to the msg priority.
    MsgListClass MsgList;

    // Functions
    // TaskClass constructor
    TaskClass(
        // Optional task name.
        const char* name = 0,
        // The priority used when a task is scheduled.
        int priority = MediumPriority,
        // The task will be scheduled by Tics when it is created.
        int flags = (ScheduleTaskOnCreationFlag),
        // A stack size of 0 gets the default stack size.
        int stackSizeInBytes = 0);


    // TaskClass destructor.
    virtual ~TaskClass(void);
    // The task must be implemented by the user.
    virtual void Task() = 0;
    // Returns true if the task exists.
    bool TaskExists(TaskClass* receiver = 0);
    // Returns true if the task exists.
    bool TaskExists(int id);
    // Deletes a sent msg with the given node id. Returns true if the 
    // msg was found and deleted.
    bool Cancel(int nodeId);
    // Deletes a sent msg. Returns true if the msg was found and deleted.
    bool Cancel(MsgClass* msg);
    // Adds the task to the Ready List. If the task arg is 0, then this task is used.
    void Schedule(TaskClass* task = 0);
    // Send a msg to another task.
    MsgClass* Send(
        // The task to send the msg to.
        TaskClass* task,
        // The msg number.
        int msgNum = NullMsg,
        // Optional msg data.
        int data = 0,
        // Optional pointer to msg data.
        void* ptr = 0,
        // The number of ticks to wait before sending out the msg.
        int delay = 0,
        // The msg will be added to the Ready List according to its priority.
        int priority = MediumPriority,
        // The sender of the msg, If 0, the sender defaults to this.
        TaskClass* sender = 0);
    // Send msg to a class instance reference. 
    MsgClass* Send(
        // A reference to the class to send the msg to.
        TaskClass& task,
        // The msg number.
        int msgNum = NullMsg,
        // Optional msg data.
        int data = 0,
        // Optional pointer to msg data.
        void* ptr = 0,
        // The number of ticks to wait before sending out the msg.
        int delay = 0,
        // The msg will be added to the Ready List according to its priority.
        int priority = MediumPriority,
        // The sender of the msg, If 0, the sender defaults to this.
        TaskClass* sender = 0);
    // Send a pre-made msg.
    MsgClass* Send(MsgClass* msg);
    // Reply to a received msg.
    void Reply(
        // The msg to reply to.
        MsgClass* receivedMsg,
        // The msg number.
        int msgNum = NullMsg,
        // Optional msg data.
        int data = 0,
        // Optional pointer to msg data.
        void* ptr = 0,
        // The number of ticks to wait before sending out the msg.
        int delay = 0,
        // The msg will be added to the Ready List according to its priority.
        int priority = MediumPriority,
        // The sender of the msg, If 0, the sender defaults to this.
        TaskClass* sender = 0);
    // Pause for the indicated number of ticks.
    void Pause(
        // The number of ticks to sleep.
        int numTicks = DefaultNumTicks,
        // The priority of the internal wake-up msg sent to the issuing task.
        int priority = MediumPriority);
    // A msg is sent to to the issuing task after the indicated number of ticks.
    MsgClass* StartTimer(
        // The number of timer ticks.
        int numTicks = DefaultNumTicks,
        // The priority of the wake-up msg sent to the issuing task.
        int priority = MediumPriority,
        // The msg number to send to the issuing task.
        int msgNum = TimeoutMsg);
    // Let other tasks run, then resume this task.
    void Yield(void);
    // Sleep until the indicated msg arrives. In AnyMsg is indicated, then the
    // task will wake up on the arrival of any msg number.
    MsgClass* Wait(
        // The msg number to wait for.
        int msgNum = AnyMsg);
    // Wait for any msg in an array.
    MsgClass* Wait(
        // Wake up the task on receiving any msg in the array.
        int* msgNumArray, 
        // The number of msgs in the array.
        int numMsgs);
    // Wait for an item to be added to a fifo.
    void Wait(
        // The fifo to wait on.
        FifoClass* fifo,
        // A pointer to the fifo slot that has data after the task resumes. 
        void* data);
    // Returns a pointer to the requested msg number if found in MsgList, otherwise
    // a 0 is returned.
    MsgClass* Recv(
        // The msg requested. AnyMsg means return the first msg in MsgList. 
        int msgNum = AnyMsg);
    // Search MsgList for any msg in the array and return it if found,
    MsgClass* Recv(
        // The msgNum array to search for a match in.
        int* msgNumArray, 
        // The number of msgs in the array.
        int numMsgs);
    // Suspend the current task and resume the task at the front of the Ready List.
    void Suspend();

    // Save the current task's context and restore the newTask's context.
    void SwitchTasks(
        // The task to switch to.
        TaskClass* newTask);
    // Remove all references to the task from MsgList.
    void DeleteFromMsgList(
        // The task to delete.
        TaskClass* task);
    // Creates a new instance of this class.
    void* operator new(size_t size);
    // Deletes as instance of this class.
    void operator delete(void* p);
    // Returns true if any of the bits in the mask are true. 
    bool GetFlag(int mask) {
        // Check the Flags against the mask.
         return Flags.IsSet(mask); }
    // Set one or more mask bits in the Flags.
    void SetFlag(int mask) { 
        // Set the mask bits in the Flags.
        Flags.Set(mask); }
    // Clear all the mask bits in the Flags.
    void ClrFlag(int mask) { Flags.Clr(mask); }
};
// Idle Task Class definition.
class IdleTaskClass : public TaskClass {
public:
    // Functions
    // Idle Task Class constructor.
    IdleTaskClass(
        // The task name.
        char* name = (char*)"IdleTask", 
        // Task priority.
        int priority = IdleTaskPriority) : TaskClass(name, priority) {}
        // Task function.
        void Task();
};

// All errors call the Report() method.
class ErrorHandlerClass {
public:
    // Data
    int ErrorNum;

    // Functions
    void Report(int errorNum = 0);
};

// This class provides various system level functions.
class TicsSystemTaskClass : public TaskClass {
public:
    // Functions
    TicsSystemTaskClass() :
        TaskClass(
            // Task name.
            "TicsSystemTask",
            // Task priority.
            MediumPriority,
            // Unexpected msgs are dropped.
            DropUnexpectedMsgsFlag)
    {
    }
    // The app task.
    void Task();
};

// This class provides utility functions used internally by Tics.
class TicsUtilsClass {
public:
    static void MemCopy(void* dst, void* src, int numChars);
    static void MemSet(void* dst, int numChars, char data);
};

// Each node in the memory mgr list, points to a memory pool of fixed size 
// memory blocks.
class NodeHeaderClass {
public:
    // Data
    enum NodeHeaderClassEnum {
        SignatureValue = 0x01234567
    };
    // Used to detect node corruption.
    int Signature;
    // Number of bytes requested 
    int NumBytesRequested;
    MemMgrClass* MemoryMgrPool;
    MemNodeClass* Next;

    // Functions
    void Initialize(int numBytesRequested, MemMgrClass* memoryMgrPool)
    {
        Next = 0;
        Signature = SignatureValue;
        NumBytesRequested = numBytesRequested;
        MemoryMgrPool = memoryMgrPool;
    }

    bool SignatureMatches()
    {
        return Signature == SignatureValue ? true : false;
    }

    bool MemoryMgrMatches(MemMgrClass* memoryMgrPool)
    {
        return MemoryMgrPool == memoryMgrPool ? true : false;
    }
};

class MemNodeClass : public NodeHeaderClass {
public:
    // Data
    unsigned int StartOfUserArea;

    // Functions
    MemNodeClass(int numBytesRequested, MemMgrClass* memoryMgrSource) : StartOfUserArea(0) {
        Initialize(numBytesRequested, memoryMgrSource);
    }

    void* UserArea()
    {
        return &StartOfUserArea;
    }

    void* SystemArea()
    {
        return this;
    }
};

class MemNodeListClass {
private:
    // Data
    MemNodeClass* Head;
    int NumNodesInList;

public:
    // Functions
    MemNodeListClass()
    {
        Head = 0;
        NumNodesInList = 0;
    }

    bool IsEmpty()
    {
        return NumNodesInList == 0;
    }

    void Add(MemNodeClass* item);
    MemNodeClass* Remove(int numBytesRequested);
};

//-----------------------------------------------------------------------------
// MemMgrClass
//
// The MemMgrClass manages a linked list of MemNodeClass objects, each of which
// contains a list of 
//-----------------------------------------------------------------------------
class MemMgrClass {
private:
    // Data
    char* Memory;
    int CurrentOffset;
    int MemorySizeInBytes;
    int NumBytesAvailable;
    MemNodeListClass NodeList;

    // Functions
    MemNodeClass* AllocateFromMemory(int numBytesRequested);
    MemNodeClass* AllocateFromList(int numBytesRequested);
    int NumBytesToAllocate(int numBytesRequested);

public:
    // Functions
    MemMgrClass(void* memory, int memorySizeInBytes);
    void* Allocate(int numBytesRequested);
    void DeAllocate(void* p);
};

//-----------------------------------------------------------------------------
// InterruptTableRowClass
//-----------------------------------------------------------------------------
class InterruptTableRowClass {
public:

    // Data
    FifoClass Fifo;


    // Functions
    InterruptTableRowClass(int fifoItemSizeInBytes, int numFifoItems = NumInterruptFifoSlots, void* fifoSpace = 0) : 
        Fifo(fifoItemSizeInBytes, numFifoItems, fifoSpace) {}
    bool DataAvailable(void) { return !Fifo.IsNotEmpty(); }
    // You must define this function to handle the fifo data.
    virtual void Handler() {}
};

//-----------------------------------------------------------------------------
// TicsNameSpace External Definitions
//-----------------------------------------------------------------------------

namespace TicsNameSpace {
    // External definitions.
    extern TicsSystemTaskClass TicsSystemTask;
    extern IdleTaskClass IdleTask;
    extern TaskClass* CurrentTask;
    extern void Suspend();
    extern ErrorHandlerClass ErrorHandler;
    extern TimerTickType ReadTickCount();
    extern FifoClass InterruptFifo;
    extern void Schedule(TaskClass* task, bool inIsr);
    extern void Send(TaskClass* task, FifoClass* fifo, void* data);
    extern MsgListClass ReadyList;
    extern FlagsClass TicsFlags;
};

//-----------------------------------------------------------------------------
// End guard
//-----------------------------------------------------------------------------
#endif				// TicsGuard
