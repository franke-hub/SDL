//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       Test_pthread.h
//
// Purpose-
//       Include file for Test_pthread.cpp
//
// Last change date-
//       2026/02/26
//
//----------------------------------------------------------------------------
#ifndef TEST_PTHREAD_H_INCLUDED
#define TEST_PTHREAD_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       atomic_double
//
// Purpose-
//       Extend std::atomic_double with needed arithmetic operators.
//
//----------------------------------------------------------------------------
struct atomic_double : std::atomic<double> { // Atomic double value
//----------------------------------------------------------------------------
// atomic_double::Constructors/destructor
//----------------------------------------------------------------------------
public:
   atomic_double( void ) noexcept   // Default constructor
:  std::atomic<double>()
{  }

   atomic_double(double D) noexcept // Value constructor
:  std::atomic<double>(D)
{  }

// Destructor declaration not required

//----------------------------------------------------------------------------
// atomic_double::Operators
//----------------------------------------------------------------------------
// A complete implementation would need all arithmetic operators, but we only
// need the += operator
atomic_double&
   operator+=(double rhs) noexcept
{
   double _old= load();             // (The current value)
   for(;;) {                        // Atomic add
     double _new= _old + rhs;
     int cc= compare_exchange_strong(_old, _new);
     if( cc )
       break;
   }

   return *this;
}
}; // struct atomic_double
#endif // TEST_PTHREAD_H_INCLUDED
