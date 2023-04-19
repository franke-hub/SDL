//----------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Hardware.i
//
// Purpose-
//       System hardware interfaces implementation.
//
// Last change date-
//       2023/04/15
//
// Implementation notes-
//       This file is only meant to be included by ../Hardware.h
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HARDWARE_BITS_I_INCLUDED
#define _LIBPUB_HARDWARE_BITS_I_INCLUDED

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Namespace-
//       Hardware
//
// Purpose-
//       System hardware accessor namespace
//
//----------------------------------------------------------------------------
namespace Hardware {                // System hardware accessor namespace
//----------------------------------------------------------------------------
// getLR: get the caller's return address
//----------------------------------------------------------------------------
static inline void*                 // The caller's return address
   getLR( void )                    // Get link register
{  return __builtin_extract_return_addr(__builtin_return_address(0)); }

//----------------------------------------------------------------------------
// getSP: get the current stack pointer
//----------------------------------------------------------------------------
static inline void*                 // The stack pointer
   getSP( void )                    // Get stack pointer
{  return __builtin_frame_address(0); }
//{  return *((void**)__builtin_frame_address(0)); }

//----------------------------------------------------------------------------
// getTSC: get the current time stamp counter
//----------------------------------------------------------------------------
static inline uint64_t              // The time stamp counter
   getTSC( void )                   // Get time stamp counter
{
   uint64_t            result;      // Resultant

#ifdef __x86_64__                   // 64 bit assembler code
   asm volatile("\n"
"        rdtsc\n"                   // Read the time stamp counter
"        salq    $32, %%rdx\n"      // Position the upper resultant
"        orq     %%rdx, %%rax\n"    // Combine with lower resultant
       : "=a" (result)              // %0
       :                            // (No INP parameter)
       :
       );

   return result;
#else
   asm volatile("\n"                // 32 bit assembler code
"        rdtsc\n"                   // Read the time stamp counter
       : "=A" (result)              // %0
       :                            // (No INP parameter)
       :
       );
#endif

   return result;
}
} // namespace Hardware
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HARDWARE_BITS_I_INCLUDED
