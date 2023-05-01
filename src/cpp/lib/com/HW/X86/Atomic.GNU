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
//       HW/X86/Atomic.GNU
//
// Purpose-
//       Atomic library routines: X586 multiprocessor, GNU compiler
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include "unistd.h"                 // For NULL

//----------------------------------------------------------------------------
//
// csb (Compare-and-swap byte)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csb(                             // Compare-and-swap byte
     ATOMIC8*          swapAddr,    // -> Swap byte
     const int8_t      oldValue,    // Old value
     const int8_t      newValue)    // New value
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        lock cmpxchgb %%dl, (%%rdi)\n"
"        movl    $1, %%eax\n"
"        jnz     0f\n"
"        xorl    %%eax, %%eax\n"
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#else                               // 32 bit assembler code
   asm("\n"
"        lock cmpxchgb %%dl, (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#endif

   return cc;
}

//----------------------------------------------------------------------------
//
// csd (Compare-and-swap double)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csd(                             // Compare-and-swap double
     ATOMIC64*         swapAddr,    // -> Swap doubleword
     const int64_t     oldValue,    // Old value
     const int64_t     newValue)    // New value
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        lock cmpxchgq %%rdx, (%%rdi)\n"
"        movl    $1, %%eax\n"
"        jnz     0f\n"
"        xorl    %%eax, %%eax\n"
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#elif defined(__PIC__)              // 32 bit assembler w/PIC
// int32_t             upper= int32_t(newValue>>32);
// int32_t             lower= int32_t(newValue);

   asm("\n"
"        pushl   %%ebx\n"           // For PIC
"        movl    %4, %%ebx\n"       // For PIC, last instruction before cmpxchg
"        lock cmpxchg8b (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
"        popl    %%ebx\n"           // For PIC
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "A"   (oldValue)           // %2
       , "c"   (int32_t(newValue>>32)) // %3
       , "r"   (int32_t(newValue))     // %4
//     , "c"   (upper)              // %3
//     , "rm"  (lower)              // %4
       : "cc"
       );
#else                               // 32 bit assembler
   int32_t             upper= int32_t(newValue>>32);
   int32_t             lower= int32_t(newValue);

   asm("\n"
"        lock cmpxchg8b (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "A"   (oldValue)           // %2
       , "c"   (upper)              // %3
       , "b"   (lower)              // %4
       : "cc"
       );
#endif

   return cc;
}

//----------------------------------------------------------------------------
//
// csh (Compare-and-swap halfword)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csh(                             // Compare-and-swap halfword
     ATOMIC16*         swapAddr,    // -> Swap halfword
     const int16_t     oldValue,    // Old value
     const int16_t     newValue)    // New value
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        lock cmpxchgw %%dx, (%%rdi)\n"
"        movl    $1, %%eax\n"
"        jnz     0f\n"
"        xorl    %%eax, %%eax\n"
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#else                               // 32 bit assembler
   asm("\n"
"        lock cmpxchgw %%dx, (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#endif

   return cc;
}

//----------------------------------------------------------------------------
//
// csp (Compare-and-swap Pointer
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csp(                             // Compare-and-swap pointer
     ATOMICP**         swapAddr,    // -> Swap pointer
     const ATOMICP*    oldValue,    // Old value
     const ATOMICP*    newValue)    // New value
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        lock cmpxchgq %%rdx, (%%rdi)\n"
"        movl    $1, %%eax\n"
"        jnz     0f\n"
"        xorl    %%eax, %%eax\n"
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#else                               // 32 bit assembler
   asm("\n"
"        lock cmpxchgl %%edx, (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#endif

   return cc;
}

//----------------------------------------------------------------------------
//
// csw (Compare-and-swap word)
//
// cc= 1;
// atomic {
//   if ((*swapAddr) == oldValue) {
//     *swapAddr= newValue;
//     cc= 0;
//     }
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   csw(                             // Compare-and-swap word
     ATOMIC32*         swapAddr,    // -> Swap word
     const int32_t     oldValue,    // Old value
     const int32_t     newValue)    // New value
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        lock cmpxchgl %%edx, (%%rdi)\n"
"        movl    $1, %%eax\n"
"        jnz     0f\n"
"        xorl    %%eax, %%eax\n"
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#else                               // 32 bit assembler
   asm("\n"
"        lock cmpxchgl %%edx, (%%edi)\n"
"        jnz     0f\n"
"        xorl    %%edi, %%edi\n"
"0:\n"
       : "=D"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       , "a"   (oldValue)           // %2
       , "d"   (newValue)           // %3
       : "cc"
       );
#endif

   return cc;
}

//----------------------------------------------------------------------------
//
// isync (Instruction synch)
//
// Does not return until all prior machine instructions have completed.
//
//----------------------------------------------------------------------------
extern "C" void
   isync( void )                    // Instruction synch
{
   asm("wait\n");                   // Assembler code (multi-modal)
}

//----------------------------------------------------------------------------
//
// tsb (Test-and-set byte)
//
// cc= 1;
// atomic {
//   if (((*swapAddr) & 0x80) == 0)
//     cc= 0;
//   *swapAddr= 0xFF;
//   }
// return cc;
//
//----------------------------------------------------------------------------
extern "C" int                      // Condition code
   tsb(                             // Test-and-set byte
     ATOMIC8*          swapAddr)    // -> Test byte
{
   int                 cc;

#if defined(__x86_64__)             // 64 bit assembler code
   asm("\n"
"        xorl    %%eax, %%eax\n"    // (Pre-zero upper 24 bits)
"        movb    $0xFF, %%al\n"
"        lock xchgb %%al, (%%rdi)\n"
"        andb    $0x80, %%al\n"     // (If zero, %%eax is zero)
"        jz      0f\n"
"        movb    $1, %%al\n"        // (Only need to set lower byte)
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       : "cc"
       );
#else                               // 32 bit assembler
   asm("\n"
"        xorl    %%eax, %%eax\n"    // (Pre-zero upper 24 bits)
"        movb    $0xFF, %%al\n"
"        lock xchg %%al, (%%edi)\n"
"        andb    $0x80, %%al\n"     // (If zero, %%eax is zero)
"        jz      0f\n"
"        movb    $1, %%al\n"        // (Only need to set lower byte)
"0:\n"
       : "=a"  (cc)                 // %0
       : "D"   (swapAddr)           // %1
       : "cc"
       );
#endif

   return cc;
}

