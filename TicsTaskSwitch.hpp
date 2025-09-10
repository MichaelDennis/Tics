#pragma once
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
// Copyright (c) 2024, (Michael Dennis McDonnell)
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define CPU types
//-----------------------------------------------------------------------------
#define VSX86 0
#define GCCARM32 1
#define GCCX86 2


//-----------------------------------------------------------------------------
// Select the CPU type
//-----------------------------------------------------------------------------
#define CPU_TYPE GCCX86

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
// x86 task switch macros. Select this when running on Linux using g++.
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

#if CPU_TYPE == GCCX86

#define SaveRegisters() \
    asm volatile (      \
        "pushal"        \
    )

#define RestoreRegisters() \
    asm volatile (         \
        "popal"            \
    )

#define SetStackPointer(variable) \
    asm volatile (                \
        "movl %0, %%esp"          \
        :                         \
        : "r" (variable)          \
    )

#define GetStackPointer(variable) \
    asm volatile (                \
        "movl %%esp, %0"          \
        : "=r" (variable)         \
    )

#endif

