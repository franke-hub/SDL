//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Counter.h
//
// Purpose-
//       Debugging object reference Counter.
//
// Last change date-
//       2024/10/01
//
// Implementation note-
//       In static termination, if c_count != d_count, the values are always
//       displayed.
//       Counter.cpp can also be modified to display more aggressively.
//
//----------------------------------------------------------------------------
#ifndef COUNTER_H_INCLUDED
#define COUNTER_H_INCLUDED

#include <atomic>                   // For std::atomic<size>

//----------------------------------------------------------------------------
//
// Class-
//       Counter
//
// Purpose-
//       Object reference counter.
//
//----------------------------------------------------------------------------
class Counter {                     // Object reference counter
//----------------------------------------------------------------------------
// Counter::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::atomic<size_t>         counter_t; // The individual counter type

//----------------------------------------------------------------------------
// Counter::Attributes
//----------------------------------------------------------------------------
static counter_t       c_count;     // Number of constructors
static counter_t       d_count;     // Number of destructors

//----------------------------------------------------------------------------
// Counter::Constructors/destructor
//----------------------------------------------------------------------------
   Counter( void );                 // Constructor

virtual
   ~Counter( void );                // Destructor
}; // class Counter
#endif // COUNTER_H_INCLUDED
