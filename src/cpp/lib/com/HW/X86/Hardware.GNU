//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HW/X86/Hardware.GNU
//
// Purpose-
//       Hardware object implementation: X586 processor, GNU compiler
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <unistd.h>                 // For NULL

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getLR
//
// Purpose-
//       Return the link register.
//
//----------------------------------------------------------------------------
void*                               // The link register
   Hardware::getLR( void )          // Extract link register
{
   return __builtin_return_address(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getSP
//
// Purpose-
//       Return the stack pointer.
//
//----------------------------------------------------------------------------
void*                               // The stack pointer
   Hardware::getSP( void )          // Extract stack pointer
{
   return __builtin_frame_address(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getTSC
//
// Purpose-
//       Get timestamp counter.
//
//----------------------------------------------------------------------------
uint64_t                            // The timestamp counter
   Hardware::getTSC( void )         // Get timestamp counter
{
   uint64_t            result;      // Resultant

#ifdef __x86_64__                   // 64 bit assembler code
   asm volatile("\n"
"        rdtsc\n"                   // Read the clock
"        salq    $32, %%rdx\n"      // Position the upper resultant
"        orq     %%rdx, %%rax\n"    // Combine with lower resultant
       : "=a" (result)              // %0
       :                            // (No INP parameter)
       :
       );

   return result;
#else
   asm volatile("\n"                // 32 bit assembler non-PIC
"        rdtsc\n"                   // Read the clock
       : "=A" (result)              // %0
       :                            // (No INP parameter)
       :
       );
#endif

   return result;
}

