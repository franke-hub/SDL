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
//       Hardware.h
//
// Purpose-
//       System hardware interfaces.
//
// Last change date-
//       2026/09/08
//
// Implementation notes-
//       These methods are simulated unless using the GNU compiler and,
//       for get_TSC, X86 hardware. If unsupported:
//           get_LR  returns nullptr
//           get_SP  returns nullptr
//           get_TSC (atomically) returns its invocation count
//
//       get_TSC returns a 64 bit value even on 32 bit hardware.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HARDWARE_H_INCLUDED
#define _LIBPUB_HARDWARE_H_INCLUDED

#include <cstdint>                  // For uint64_t

#include "bits/pubconfig.h"         // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Namespace pub::Hardware, system hardware accessor namespace
//
// Implementation note-
//       Hardware is a struct rather than a namespace so that its methods are
//       ordinary (non-inline) function calls rather than being optimized away
//       or inlined into their caller. get_LR and get_SP each report
//       information about their *direct* caller; wrapping either in a
//       forwarding call changes what they report, so don't do that.
//----------------------------------------------------------------------------
struct Hardware {                   // System hardware accessor functions
//----------------------------------------------------------------------------
//
// Method-
//       Hardware::get_LR
//
// Purpose-
//       Return the link register (the caller's return address).
//
//----------------------------------------------------------------------------
static void*                        // The caller's return address
   get_LR( void );                  // Get link register

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::get_SP
//
// Purpose-
//       Return the stack pointer.
//
//----------------------------------------------------------------------------
static void*                        // The stack pointer
   get_SP( void );                  // Get stack pointer

//----------------------------------------------------------------------------
//
// Method-
//       Hardware::get_TSC
//
// Purpose-
//       Return the current timestamp counter.
//
// Notes-
//       The timestamp counter is a high resolution elapsed time measurement
//       device. The lowest valid low order bit is updated each clock cycle.
//       Note: On some processors some of the low order bits may not change.
//
//----------------------------------------------------------------------------
static uint64_t                     // The timestamp counter
   get_TSC( void );                 // Get timestamp counter
}; // struct Hardware
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HARDWARE_H_INCLUDED
