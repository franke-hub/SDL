//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
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
//       Common statistic collection.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJ_STATISTIC_H_INCLUDED
#define OBJ_STATISTIC_H_INCLUDED

#include <atomic>
#include "obj/config/config.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef std::atomic<size_t>  STATISTIC; // Statistical counter

//----------------------------------------------------------------------------
//
// Subroutine-
//       statistic
//
// Purpose-
//       Increment statistic
//
//----------------------------------------------------------------------------
static inline void
   statistic(                       // Increment statistic
     STATISTIC&        stat)        // (The statistic to update)
{
   if( config::USE_STATISTICS )
     stat++;
}
} // namespace _OBJ_NAMESPACE
#endif // OBJ_STATISTIC_H

