//----------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Hardware.cpp
//
// Purpose-
//       System hardware interfaces implementation.
//
// Last change date-
//       2026/09/08
//
// Implementation notes-
//       Hardware is a struct rather than a namespace so that its methods are
//       ordinary (non-inline) function calls rather than being optimized away
//       or inlined into their caller. get_LR and get_SP each report
//       information about their *direct* caller; wrapping either in a
//       forwarding call changes what they report, so don't do that.
//
//       These methods are simulated unless using the GNU compiler and,
//       for get_TSC, X86 hardware. If unsupported:
//           get_LR   returns nullptr
//           get_SP   returns nullptr
//           get_TSC  (atomically) returns its invocation count
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic_uint64_t

#include "pub/Hardware.h"           // For pub::Hardware, implemented

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Hardware::get_LR: get the caller's return address
//----------------------------------------------------------------------------
#if defined(__GNUC__)               // GNU compiler required
void*                               // The Link Register
   Hardware::get_LR( void )         // Get Link Register
{  return __builtin_extract_return_addr(__builtin_return_address(0)); }

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void*
   Hardware::get_LR( void )
{  return nullptr; }
#endif

//----------------------------------------------------------------------------
// get_SP: get the current stack pointer
//----------------------------------------------------------------------------
#if  defined(__GNUC__)              // GNU compiler required
void*                               // The Stack Pointer
   Hardware::get_SP( void )         // Get Stack Pointer
{  return __builtin_frame_address(0); }

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void*
   Hardware::get_SP( void )
{  return nullptr; }
#endif

//----------------------------------------------------------------------------
// get_TSC: get the current time stamp counter
//----------------------------------------------------------------------------
#if defined(__GNUC__) && defined(_HW_X86) // GNU compiler, x86 required
uint64_t                            // The Time Stamp Counter
   Hardware::get_TSC( void )        // Get Time Stamp Counter
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

#else // (Simulated) - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uint64_t
   Hardware::get_TSC( void )
{  static std::atomic<std::uint64_t> tsc(0); return ++tsc; }
#endif
_LIBPUB_END_NAMESPACE
