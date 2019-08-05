//-----------------------------------------------------------------------------
// Copyright (c) 2019, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Start guard
//-----------------------------------------------------------------------------
#ifndef TicsGuard
#define TicsGuard

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// typedefs
//-----------------------------------------------------------------------------
typedef unsigned int size_t;

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
        NumInterruptFifoSlots = 32
    };

    enum {
        // TicsNameSpace flags.
        SafeModeFlag = 1,
        IsrMultipleSchedulingFlag = 8
    };

    // Users can use any priority between LowPriority and HighPriority. 
    enum PriorityType {
        TailPriority = 0, IdleTaskPriority = 1, LowPriority = 1000,
        MediumPriority = 3000, HighPriority = 4000, HeadPriority = 10000
    };

    // Tics reserves msg numbers 0 to 999. Users can create their own
    // msg numbers from 1,000 to 99,999.
    enum {
        // This must be the first and smallest defined msg number.
        FirstMsgNum = 0,
 
        // 1          2               3               4               5
        NullMsg,    AnyMsg,         RunMsg,         GoMsg,          StartMsg, 
        StopMsg,    DoneMsg,        ScheduleMsg,    HelloMsg,       RqstMsg, 
        GrantMsg,   TimeoutMsg,     WakeupMsg,      AskMsg,         ReplyMsg,
        OkayMsg,    DeleteTaskMsg,  NotifyMsg,      TimerDoneMsg,   OnMsg,
        OffMsg,     IsrMsg,

        // This must be the last and largest defined msg number.
        LastMsgNum = 99999
    };
};

//-----------------------------------------------------------------------------
// Using
//-----------------------------------------------------------------------------
using namespace TicsNameSpace;

//-----------------------------------------------------------------------------
// Class references
//-----------------------------------------------------------------------------
class TaskClass;
class FifoClass;
class NodeClass;
class MemoryMgrClass;

//-----------------------------------------------------------------------------
// FlagsClass
//
// Manage bit flags.
//-----------------------------------------------------------------------------
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
class MsgClass {
    // Data
public:
    static int IdCounter;
    int Id;
    int ListId;
    int ReceiverId;
    int MsgNum;
    PriorityType Priority;
    int Delay;
    unsigned int Checksum;
    TimerTickType EndTime;
    void* Ptr;
    int Data;
    TaskClass* Sender;
    TaskClass* Receiver;
    MsgClass* Next;
    MsgClass* Prev;
    void* OriginalThis;

    // Functions
public:
    void * operator new(size_t size);

    void operator delete(void * p);

    MsgClass();

    MsgClass(
        TaskClass* receiver, 
        int msgNum = GoMsg, 
        int data = 0, 
        void* ptr = 0,
        int delay = 0,
        PriorityType priority = MediumPriority,
        TaskClass* sender = 0);

    virtual ~MsgClass();

    void Init();

    void CheckParameters(bool fullCheck = true);

    bool Is(int msgNum)
    {
        return msgNum == MsgNum ? true : false;
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

    unsigned int ComputeChecksum();

    void VerifyChecksum();

    void UpdateChecksum(bool updateNeighbors = false);
};

//-----------------------------------------------------------------------------
// MsgList class.
//-----------------------------------------------------------------------------
class MsgListClass {
public:
    enum {
        DefaultMaxMsgs = 32
    };

    static int IdCounter;
    int Id;
    int NumMsgsInList;
    int MaxMsgs;
    MsgClass ActualHead;
    MsgClass ActualTail;
    MsgClass* Head;
    MsgClass* Tail;

    // Functions
    MsgListClass(int maxMsgs = DefaultMaxMsgs);

    bool IsHead(MsgClass* a)
    {
        return a == Head;
    }

    bool IsTail(MsgClass* a)
    {
        return a == Tail;
    }

    MsgClass* Unlink(MsgClass* a = 0);

    bool IsEmpty();

    bool IsFull();

    void Insert(MsgClass* a, MsgClass* b);

    void AddByPriority(MsgClass* a, bool addByTaskPriority = true);

    void Add(MsgClass* a);

    MsgClass* Remove(MsgClass* a = 0);

    void Flush();

    bool TaskExists(TaskClass* task);

    bool TaskExists(int taskId);

    virtual void Discard(TaskClass* task);

    virtual bool Discard(int msgId);

    void DoInsertSafetyChecks(MsgClass* a, MsgClass* b);

    void DoUnlinkSafetyChecks(MsgClass* a);
    
    void DoAddSafetyChecks(MsgClass* a);
    
    void DoRemoveSafetyChecks(MsgClass* a);
    
    void CheckListIntegrity();
};


//-----------------------------------------------------------------------------
// Task List class.
//-----------------------------------------------------------------------------
class TaskListClass : public MsgListClass {
public:
    // Data

    // Functions
    void Discard(TaskClass* task);

    bool Discard(int msgId);
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
        DefaultStackSizeInBytes = (4096 * 4),
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
    StackType * SavedSp;

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

    // Functions
private:
    void* Bump(void* item);
    void* Remove();
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
    void * operator new(size_t size);
    void operator delete(void * p);
};

//-----------------------------------------------------------------------------
// TaskClass
//-----------------------------------------------------------------------------
class TaskClass {
public:
    // Data

    enum {
        // TaskClass flags.
        TaskStartedFlag = 1,
        DropUnexpectedMsgsFlag = 2,
        ScheduleTaskOnCreationFlag = 4,
        NoStackFlag = 8,
        DefaultNumTicks = 100
    };

    FlagsClass Flags;
    static int IdCounter;
    StackClass Stack;
    const char* Name;
    int Id;
    PriorityType Priority;
    MsgListClass MsgList;

    // Functions
    TaskClass(
        const char* name = 0,
        PriorityType priority = MediumPriority,
        int flags = (ScheduleTaskOnCreationFlag),
        int stackSizeInBytes = 0);

    virtual ~TaskClass();

    virtual void Task() = 0;

    bool TaskExists(TaskClass* receiver = 0);

    bool TaskExists(int id);

    bool CancelMsg(int msgId);

    bool CancelMsg(MsgClass * msg);

    void Schedule(TaskClass* task = 0);

    MsgClass * Send(
        TaskClass* task,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        PriorityType priority = MediumPriority,
        TaskClass* sender = 0);

    MsgClass* Send(
        TaskClass& task,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        PriorityType priority = MediumPriority,
        TaskClass* sender = 0);   

    MsgClass* Send(MsgClass* msg);

    void Reply(
        MsgClass* receivedMsg,
        int msgNum = NullMsg,
        int data = 0,
        void* ptr = 0,
        int delay = 0,
        PriorityType priority = MediumPriority,
        TaskClass* sender = 0);

    void Pause(
        int numTicks = DefaultNumTicks,
        PriorityType priority = MediumPriority);

    MsgClass * StartTimer(
        int numTicks = DefaultNumTicks,
        PriorityType priority = MediumPriority, 
        int msgNum = TimerDoneMsg);

    void Yield();

    MsgClass* Wait(int msgNum = AnyMsg);

    void Wait(FifoClass * fifo, void * data);
    
    MsgClass* Recv(int msgNum = AnyMsg);

    void Suspend();

    void SwitchTasks(TaskClass * newTask);

    void DoSendSafetyChecks(MsgClass* msg);

    void RemoveFromMsgList(TaskClass* task);

    void * operator new(size_t size);

    void operator delete(void * p);

    bool GetFlag(int mask) { return Flags.IsSet(mask); }

    void SetFlag(int mask) { Flags.Set(mask); }

    void ClrFlag(int mask) { Flags.Clr(mask); }
};

class IdleTaskClass : public TaskClass {
public:
    // Functions
    IdleTaskClass(char * name = (char*) "IdleTask", PriorityType priority = IdleTaskPriority) :
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
    MemoryMgrClass * MemoryMgrPool;
    NodeClass * Next;

    // Functions
    void Initialize(int numBytesRequested, MemoryMgrClass * memoryMgrPool)
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

    bool MemoryMgrMatches(MemoryMgrClass * memoryMgrPool)
    {
        return MemoryMgrPool == memoryMgrPool ? true : false;
    }
};

class NodeClass : public NodeHeaderClass {
public:
    // Data
    unsigned int StartOfUserArea;

    // Functions
    NodeClass(int numBytesRequested, MemoryMgrClass * memoryMgrSource) {
        Initialize(numBytesRequested, memoryMgrSource);
    }

    void * UserArea()
    {
        return &StartOfUserArea;
    }

    void * SystemArea()
    {
        return this;
    }
};

class NodeListClass {
private:
    // Data
    NodeClass * Head;
    int NumNodesInList;

public:
    // Functions
    NodeListClass()
    {
        Head = 0;
        NumNodesInList = 0;
    }

    bool IsEmpty()
    {
        return NumNodesInList == 0 ? true : false;
    }

    void Add(NodeClass * item);
    NodeClass * Remove(int numBytesRequested);
};

class MemoryMgrClass {
private:
    // Data
    char * Memory;
    int CurrentOffset;
    int MemorySizeInBytes;
    NodeListClass NodeList;

    // Functions
    NodeClass * AllocateFromMemory(int numBytesRequested);
    NodeClass * AllocateFromList(int numBytesRequested);
    int NumBytesToAllocate(int numBytesRequested);

public:
    // Functions
    MemoryMgrClass(void * memory, int memorySizeInBytes);
    void * Allocate(int numBytesRequested);
    void DeAllocate(void * p);
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
    extern void Send(TaskClass * task, FifoClass * fifo, void * data);
    extern MsgListClass ReadyList;
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
