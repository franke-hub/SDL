//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       pub/diag-counter.h
//
// Purpose-
//       Diagnositic class/struct instance counter, used for debugging.
//
// Last change date-
//       2025/01/15
//
// Implementation notes-
//       To verify that a class or struct object doesn't leak memory, i.e.
//       that the same number of destructors as constructors are invoked,
//       just add a pub::diag::counter to it.
//
//       During static termination, diag::counter verifies that the number of
//       constructor and destructor invocations are equal, e.g. that there
//       isn't some sort of storage leak. (An informational debugging message
//       is displayed when they're not equal.)
//
//       Note that if static global objects contain diag::counters, their
//       destruction order is indeterminate. Allow for this when getting a
//       constructors != destructors error message.
//
//       To avoid confusion, it's best to put counters in only one class or
//       struct at a time. The counter only counts constructor and destructor
//       invocations, it doesn't keep track of individual instances.
//
//       Counters do not require structure space and usually do not have a
//       significant performance impact. Even so, it's best not to leave
//       counter instances in production code.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DIAG_COUNTER_H_INCLUDED
#define _LIBPUB_DIAG_COUNTER_H_INCLUDED

#include <atomic>                   // For std::atomic<size_t>

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace diag {
//----------------------------------------------------------------------------
//
// Class-
//       pub::diag::Counter
//
// Purpose-
//       Object reference counter.
//
//----------------------------------------------------------------------------
class Counter {                     // Object reference counter
//----------------------------------------------------------------------------
// pub::diag::Counter::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::atomic<std::size_t>    counter_t; // The individual counter type

//----------------------------------------------------------------------------
// pub::diag::Counter::Attributes
//----------------------------------------------------------------------------
static counter_t       c_count;     // Number of constructors
static counter_t       d_count;     // Number of destructors
static int             hcdm;        // Hard Core Debug Mode?

//----------------------------------------------------------------------------
// pub::diag::Counter::Constructors/destructor
//----------------------------------------------------------------------------
   Counter( void );                 // Constructor

virtual
   ~Counter( void );                // Destructor

//----------------------------------------------------------------------------
// pub::diag::Counter::debug || Display constructor and destructor counts
//----------------------------------------------------------------------------
static void
   debug(                           // Debugging display
     const char*       info= "");   // Caller information

//----------------------------------------------------------------------------
// pub::diag::Counter::set_hcdm || Set/clear Hard Core Debug Mode
//----------------------------------------------------------------------------
static void
   set_hcdm(bool);                  // Set/clear Hard Core Debug Mode
}; // class Counter
}  // namespace diag
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DIAG_COUNTER_H_INCLUDED
