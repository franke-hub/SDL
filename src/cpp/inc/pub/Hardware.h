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
//       Hardware.h
//
// Purpose-
//       System hardware interfaces.
//
// Last change date-
//       2023/04/15
//
// Implementation notes-
//       Currently only implemented for X86 architecture and GNU compiler.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HARDWARE_H_INCLUDED
#define _LIBPUB_HARDWARE_H_INCLUDED

#include <stdint.h>                 // For uint64_t
#include "bits/pubconfig.h"         // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Namespace pub::Hardware, system hardware accessor namespace
//----------------------------------------------------------------------------
namespace Hardware {                // System hardware accessor namespace
//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getLR
//
// Purpose-
//       Return the link register (return address).
//
//----------------------------------------------------------------------------
static inline void*                 // The caller's return address
   getLR( void );                   // Get link register

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getSP
//
// Purpose-
//       Return the stack pointer.
//
//----------------------------------------------------------------------------
static inline void*                 // The stack pointer
   getSP( void );                   // Get stack pointer

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::getTSC
//
// Purpose-
//       Return the current timestamp counter
//
// Notes-
//       The timestamp counter is a high resolution elapsed time measurement
//       device. The lowest valid low order bit is updated each clock cycle.
//       Note: On some processors some of the low order bits may not change.
//
//----------------------------------------------------------------------------
static inline uint64_t              // The timestamp counter
   getTSC( void );                  // Get timestamp counter
} // namespace Hardware
_LIBPUB_END_NAMESPACE

#if defined(_HW_X86) && defined(__GNUC__)
#  include "bits/Hardware.i"        // (The implementation)
#else
#  error "Not implemented"          // Requires X86 and GNU compiler
#endif

#endif // _LIBPUB_HARDWARE_H_INCLUDED
