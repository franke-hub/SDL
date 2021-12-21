//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Statistic.h
//
// Purpose-
//       Integer statistic counter.
//
// Last change date-
//       2021/11/10
//
// Usage notes-
//       The minimum value is the minimum value after a maximum's detected.
//
//----------------------------------------------------------------------------
#ifndef _PUB_STATISTIC_H_INCLUDED
#define _PUB_STATISTIC_H_INCLUDED

#include <atomic>                   // For std::atomic_uint64_t, ...
#include <stdint.h>                 // For uint64_t

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Struct-
//       pub::Statistic
//
// Purpose-
//       Statistic counter.
//
// Implementation note-
//       Use current.load() to get current value, etc. for maximum/minimum.
//
//----------------------------------------------------------------------------
struct Statistic {                  // Statistic
std::atomic_int64_t    counter= 0;  // Total increment count
std::atomic_int64_t    current= 0;  // Current value
std::atomic_int64_t    maximum= 0;  // Highest value
std::atomic_int64_t    minimum= 0;  // Lowest value after a maximum

//----------------------------------------------------------------------------
// pub::Statistic::Methods
//----------------------------------------------------------------------------
int64_t                             // Current value
   inc( void )                      // Increment value
{
   counter++;                       // Increment the counter

   // Increment the current value
   int64_t old_value= current.load(); // The current value
   int64_t new_value= old_value + 1;
   while( !current.compare_exchange_weak(old_value, new_value) )
     new_value= old_value + 1;

   // Update the maximum and minimum values (if they've changed)
   old_value= maximum.load();
   while( old_value < new_value ) { // If necessary, update the maximum
     if( maximum.compare_exchange_strong(old_value, new_value) ) {
       // The maximum was successfully updated. Update the minimum.
       // Note that if some other thread has also updated the maximum and
       // then also updates the minimum, it's not lowered here.
       old_value= minimum.load();
       while( old_value < new_value ) {
         if( minimum.compare_exchange_strong(old_value, new_value) )
           break;
       }
       break;
     }
   }

   return new_value;
}

int64_t                             // Current value
   dec( void )                      // Increment value
{
   // Decrement the current value
   int64_t old_value= current.load(); // The current value
   int64_t new_value= old_value - 1;
   while( !current.compare_exchange_weak(old_value, new_value) )
     new_value= old_value - 1;

   // Decrement the minimum value (if it's changed)
   old_value= minimum.load();
   while( old_value > new_value ) { // If necessary, update the minimum
     if( minimum.compare_exchange_strong(old_value, new_value) )
       break;
   }

   return new_value;
}
}; // struct Statistic
}  // namespace _PUB_NAMESPACE
#endif // _PUB_STATISTIC_H_INCLUDED
