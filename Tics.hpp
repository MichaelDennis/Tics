/*
MIT License

Copyright (c) 2024 Michael Dennis McDonnell

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
// Copyright (c) 2024, Tics Realtime (Michael Dennis McDonnell)
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

//-----------------------------------------------------------------------------
// typedefs
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define InRange(minValue, maxValue, value) (value <= maxValue && value >= minValue)

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------
typedef unsigned int StackType;
typedef unsigned long long TimerTickType;

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------
namespace TicsNameSpace {

    enum {
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
    };

    enum {
        // TicsNameSpace flags.
        SafeModeFlag = 1,WatchDogFlag = 2
    };

    // Users can use any priority between LowPriority and HighPriority. 
    enum {
        TailPriority = 0, IdleTaskPriority = 1, LowPriority = 1000,
        MediumPriority = 3000, MediumHighPriority = 3001, HighPriority = 4000, HeadPriority = 10000
    };

    // Tics reserves msg numbers 0 to 999. Users can create their own
    // msg numbers in the range low priority to high priority.
    enum {
        // This must be the first and smallest defined msg number.
        FirstMsgNum = 0,

        // 1        2               3               4               5
        NullMsg,    AnyMsg,         RunMsg,         GoMsg,          StartMsg,
        StopMsg,    DoneMsg,        ScheduleMsg,    HelloMsg,       RqstMsg,
        GrantMsg,   TimeoutMsg,     WakeupMsg,      AskMsg,         ReplyMsg,
        OkayMsg,    DeleteTaskMsg,  NotifyMsg,      OnMsg,          OffMsg,
        IsrMsg,     ResetMsg,       StatusMsg, SuccessMsg,       FailMsg,
        CassetteTouchedMsg, InvalidMsg, 

        // This must be the last and largest defined msg number.
        LastMsgNum = 99999
    };

    enum {
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
        ErrorAttempToUnlinkHeadOrTail = 1016,
        ErrorMsgChecksumFailure = 1017,
        ErrorCannotAddAMsgToAFullList = 1018,
        ErrorCannotAddANullMsg = 1019,
        ErrorCannotAddAMsgThatIsAlreadyInAnotherList = 1020,
        ErrorCannotRemoveANullMsg = 1021,
        ErrorCannotRemoveAMsgFromAnEmptyList = 1022,
        ErrorCannotRemoveTheHeadOrTailMsg = 1023,
        ErrorCannotRemoveAMsgIfItIsNotInAList = 1024,
        ErrorCouldNotAllocateMemory = 1025,
        ErrorByteAllocationRequestMustBeInMultiplesOfWords = 1026,
        ErrorDeAllocationSignatureMismatch = 1027,
        ErrorAttempToDeAllocateToTheWrongPool = 1028,
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
        ErrorMusthaveAtLeastTwoFifoSlots = 1058,
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
class MemAiMgrTaskClass;

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
    TicsBaseClass(void)
    {
        // Object Id starts at 1. Zero is used to indicated that the Id has not been assigned.
        Id = ++IdCounter;
    }

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
    NodeClass* Next;
    NodeClass* Prev;
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
    FlagsClass(int flags = 0) : Flags(flags) {}
    void Set(int mask) { Flags |= mask; }
    void Clr(int mask) { Flags &= (~mask); }
    bool IsSet(int mask) { return Flags & mask; }
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
    int ReceiverId;
    int MsgNum;
    int Delay;
    TimerTickType EndTime;
    TaskClass* Sender;
    TaskClass* Receiver;

    // Functions
public:
    void* operator new(size_t size);

    void operator delete(void* p);

    MsgClass(
        TaskClass* receiver,
        int msgNum = StartMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        int priority = MediumPriority,
        TaskClass* sender = 0);

    virtual ~MsgClass();

    void Init();

    void CheckParameters(bool fullCheck = true);

    bool Is(int msgNum)
    {
        return msgNum == MsgNum ? true : false;
    }
};

//-----------------------------------------------------------------------------
//  List class.
//-----------------------------------------------------------------------------
class ListClass : public TicsBaseClass {
public:
    enum {
        DefaultMaxNodes = 32
    };

    int NumNodesInList;
    int MaxNodes;
    NodeClass ActualHead;
    NodeClass ActualTail;
    NodeClass* Head;
    NodeClass* Tail;

    // Functions

    ListClass(int maxNodes = DefaultMaxNodes);

    bool IsHead(NodeClass* a)
    {
        return a == Head;
    }

    bool IsTail(NodeClass* a)
    {
        return a == Tail;
    }

    NodeClass * Unlink(NodeClass* a = 0);

    bool IsEmpty(void);

    bool IsNotEmpty(void);

    bool IsFull(void);

    void Insert(NodeClass* a, NodeClass* b);

    void AddByPriority(NodeClass* a);

    void Add(NodeClass* a);

    NodeClass* Remove(NodeClass* a = 0);

    void Flush();

    bool DeleteNode(int id);

    bool DeleteNode(NodeClass* node);

    void CheckListIntegrity(void);
    
    void DoInsertSafetyChecks(NodeClass* a, NodeClass* b);
};

//-----------------------------------------------------------------------------
// MsgList class.
//-----------------------------------------------------------------------------
    class MsgListClass : public ListClass {
    public:
        bool RemoveTaskReferences(TaskClass* task);
};

//-----------------------------------------------------------------------------
// Task List class.
//-----------------------------------------------------------------------------
class TaskListClass : public ListClass {
public:

    // Functions

    void RemoveTaskReferences(TaskClass* task, bool removeTheTaskItselfAlso = false);

    void RemoveTask(TaskClass* task);

    void Add(TaskClass* task);

    bool TaskExists(TaskClass* task, int id = 0);

    bool TaskExists(int taskId);
};

class DelayListClass : public MsgListClass {
public:
    // Data
    TimerTickType LastTime;

    // Functions
    void AddByDelay(MsgClass* a);
    void CheckForTimeouts();
};

class StackClass {
public:
    // Data
    enum {
        DefaultStackSizeInBytes = (1024 * 4),
        DefaultStackPadSizeInBytes = 128,
        MinStackSizeInBytes = 2048,
        MaxStackSizeInBytes = (MinStackSizeInBytes * 16),
        DefaultStackPadBytePattern = 0x22,
        DefaultStackPadWordPattern = 0x22222222,
    };

    int StackSizeInBytes;
    int StackPadSizeInBytes;
    StackType* StackTop;
    StackType* StackBottom;
    StackType tempStackItem;
    StackType* SavedSp;

    // Functions
    StackClass(
        int stackSizeInBytes = DefaultStackSizeInBytes,
        int stackPadSizeInBytes = DefaultStackPadSizeInBytes
    );

    ~StackClass();

    void Check();
};

class FifoClass {
public:
    // Data

    enum {
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

    enum {
        // TaskClass flags.
        TaskStartedFlag = 1,
        DropUnexpectedMsgsFlag = 2,
        ScheduleTaskOnCreationFlag = 4,
        DefaultNumTicks = 1000
    };

    FlagsClass Flags;
    static int IdCounter;
    StackClass Stack;
    const char* Name;
    int Priority;
    MsgListClass MsgList;

    // Functions
    TaskClass(
        const char* name = 0,
        int priority = MediumPriority,
        int flags = (ScheduleTaskOnCreationFlag),
        int stackSizeInBytes = 0);

    virtual ~TaskClass(void);

    virtual void Task() = 0;

    bool TaskExists(TaskClass* receiver = 0);

    bool TaskExists(int id);

    bool CancelMsg(int nodeId);

    bool CancelMsg(MsgClass* msg);

    void Schedule(TaskClass* task = 0);

    MsgClass* Send(
        TaskClass* task,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        int priority = MediumPriority,
        TaskClass* sender = 0);

    MsgClass* Send(
        TaskClass& task,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        int priority = MediumPriority,
        TaskClass* sender = 0);

    MsgClass* Send(MsgClass* msg);

    void Reply(
        MsgClass* receivedMsg,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        int priority = MediumPriority,
        TaskClass* sender = 0);

    void Pause(
        int numTicks = DefaultNumTicks,
        int priority = MediumPriority);

    MsgClass* StartTimer(
        int numTicks = DefaultNumTicks,
        int priority = MediumPriority,
        int msgNum = TimeoutMsg);

    void Yield(void);

    MsgClass* Wait(int msgNum = AnyMsg);

    MsgClass* Wait(int* msgNumArray, int numMsgs);

    void Wait(FifoClass* fifo, void* data);

    MsgClass* Recv(int msgNum = AnyMsg);

    MsgClass* Recv(int* msgNumArray, int numMsgs);

    void Suspend();

    void SwitchTasks(TaskClass* newTask);

    void DeleteFromMsgList(TaskClass* task);

    void* operator new(size_t size);

    void operator delete(void* p);

    bool GetFlag(int mask) { return Flags.IsSet(mask); }

    void SetFlag(int mask) { Flags.Set(mask); }

    void ClrFlag(int mask) { Flags.Clr(mask); }
};

class IdleTaskClass : public TaskClass {
public:
    // Functions
    IdleTaskClass(char* name = (char*)"IdleTask", int priority = IdleTaskPriority) :
        TaskClass(name, priority)
    {
    }
   void Task();
};

class ErrorHandlerClass {
public:
    // Data
    int ErrorNum;

    // Functions
    void Report(int errorNum = 0);
};

class TicsSystemTaskClass : public TaskClass {
public:
    // Functions
    TicsSystemTaskClass() :
        TaskClass(
            "TicsSystemTask",
            MediumPriority,
            DropUnexpectedMsgsFlag)
    {
    }

    void Task();
};

class TicsUtilsClass {
public:
    static void MemCopy(void* dst, void* src, int numChars);
    static void MemSet(void* dst, int numChars, char data);
};

class NodeHeaderClass {
public:
    // Data
    enum {
        SignatureValue = 0x01234567
    };
    int Signature;
    int NumBytesRequested;
    MemAiMgrTaskClass* MemoryMgrPool;
    MemNodeClass* Next;

    // Functions
    void Initialize(int numBytesRequested, MemAiMgrTaskClass* memoryMgrPool)
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

    bool MemoryMgrMatches(MemAiMgrTaskClass* memoryMgrPool)
    {
        return MemoryMgrPool == memoryMgrPool ? true : false;
    }
};

class MemNodeClass : public NodeHeaderClass {
public:
    // Data
    unsigned int StartOfUserArea;

    // Functions
    MemNodeClass(int numBytesRequested, MemAiMgrTaskClass* memoryMgrSource) : StartOfUserArea(0) {
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

class MemAiMgrTaskClass {
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
    MemAiMgrTaskClass(void* memory, int memorySizeInBytes);
    void* Allocate(int numBytesRequested);
    void DeAllocate(void* p);
};

//-----------------------------------------------------------------------------
// DebuggerClass
//-----------------------------------------------------------------------------
class DebuggerClass {
public:
    // Data

    // Functions
    void DisplayReadyList();
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
//-----------------------------------------------------------------------------
// Helper Functions
//-----------------------------------------------------------------------------
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
// For Testing - throw away later.
//-----------------------------------------------------------------------------

class DtSlotClass {
public:
    // Data
    enum { Available = -1 };
    int Dt;
    int Hits;

    // Functions
    DtSlotClass() : Dt(Available), Hits(0)
    {

    }
    bool IsAvailable(int dt)
    {
        if (dt == Available) {
            return true;
        }
        else {
            return false;
        }
    }
};

class DtSlotArrayClass {
public:
    // Data
    enum { LenDSlotArray = 50 };
    DtSlotClass DtSlot[LenDSlotArray];

    // Functions
    bool Add(int dt)
    {
        // Look for a slot with the same dt value.
        for (int i = 0; i < LenDSlotArray; i++) {
            if (DtSlot[i].Dt == dt) {
                DtSlot[i].Hits++;
                return true;
            }
        }

        // Look for any empty slot.
        for (int i = 0; i < LenDSlotArray; i++) {
            if (DtSlot[i].IsAvailable(dt)) {
                DtSlot[i].Dt = dt;
                DtSlot[i].Hits++;
                return true;
            }
        }

        return false;
    }
};


//-----------------------------------------------------------------------------
// End guard
//-----------------------------------------------------------------------------
#endif				// TicsGuard
