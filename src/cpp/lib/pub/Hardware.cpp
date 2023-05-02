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
//       Hardware.cpp
//
// Purpose-
//       System hardware interfaces implementation.
//
// Last change date-
//       2023/05/02
//
// Implementation notes-
//       Hardware is a struct rather than a namespace so that Hardware::getLR
//       calls are not optimized away.
//
//----------------------------------------------------------------------------

#include <atomic>                   // For atomic_uint64_t
#include "pub/Hardware.h"

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Hardware::getLR: get the caller's return address
//----------------------------------------------------------------------------
#if !defined(__GNUC__)              // GNU compiler required
void* Hardware::getLR( void ) { return nullptr; }
#else
void*                               // The caller's return address
   Hardware::getLR( void )          // Get link register
{  return __builtin_extract_return_addr(__builtin_return_address(0)); }
#endif

//----------------------------------------------------------------------------
// getSP: get the current stack pointer
//----------------------------------------------------------------------------
#if !defined(__GNUC__)              // GNU compiler required
void* Hardware::getSP( void ) { return nullptr; }
#else
void*                               // The stack pointer
   Hardware::getSP( void )          // Get stack pointer
{  return __builtin_frame_address(0); }
#endif

//----------------------------------------------------------------------------
// getTSC: get the current time stamp counter
//----------------------------------------------------------------------------
#if !defined(__GNUC__) || !defined(_HW_X86) // GNU compiler, x86 required
uint64_t Hardware::getTSC( void )
{  static atomic_uint64_t tsc= 0; return ++tsc; }
#else
uint64_t                            // The time stamp counter
   Hardware::getTSC( void )         // Get time stamp counter
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
#endif
_LIBPUB_END_NAMESPACE
