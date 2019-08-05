#pragma once
//-----------------------------------------------------------------------------
// Copyright (c) 2019, Tics Realtime (Michael McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define CPU types
//-----------------------------------------------------------------------------
#define VSX86 0
#define GCCARM32 1

//-----------------------------------------------------------------------------
// Select the CPU type
//-----------------------------------------------------------------------------
#define CPU_TYPE VSX86

//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// using
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// x86 task switch macros. Select this when running in Visual Studio C++.
//-----------------------------------------------------------------------------

#if CPU_TYPE == VSX86

#define SaveRegisters()                             \
    __asm {                                         \
        __asm pushad			        			\
    }

#define RestoreRegisters()                          \
    __asm {                                         \
	    __asm popad				    				\
    }

#define SetStackPointer(variable)	     		    \
    __asm {                                         \
	    __asm mov esp, variable			    	    \
    }

#define GetStackPointer(variable)   				\
    __asm {                                         \
        __asm mov variable, esp                     \
    }

#endif

//-----------------------------------------------------------------------------
/// ARM32 task switch macros.
///
/// See the following links:
///
/// ARM inline assembler doc
/// http://www.ethernut.de/en/documents/arm-inline-asm.html
///
/// ARM Assembler Guide
/// http://infocenter.arm.com/help/topic/com.arm.doc.dui0068b/DUI0068.pdf
//-----------------------------------------------------------------------------

#if CPU_TYPE == GCCARM32

#define SaveRegisters() asm volatile ("stmfd r13!, {r0-r12, r14}")

#define RestoreRegisters() asm volatile ("ldmfd r13!, {r0-r12, r14}")

#define GetStackPointer(var) asm volatile("mov %[result], r13" : [result]"=r" (var))

#define SetStackPointer(var) asm volatile("mov r13, %[value]" : : [value]"r" (var))

#endif
