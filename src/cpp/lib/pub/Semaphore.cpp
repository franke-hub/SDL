//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Semaphore.cpp
//
// Purpose-
//       Semaphore object methods.
//
// Last change date-
//       2026/01/12
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Semaphore.h"          // For pub::Semaphore

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (Generic) enum

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Semaphore::debug(const char* info) const // Debugging display
{
   if( info == nullptr )
     info= "Semaphore";

   debugf("Semaphore(%p)::debug(%s) count(%d)\n", this, info, count);
}
}  // namespace _LIBPUB_NAMESPACE
