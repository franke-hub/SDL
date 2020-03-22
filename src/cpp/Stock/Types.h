//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Types.h
//
// Purpose-
//       Define the common types.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <stdint.h>

#ifndef DEFINE_H_INCLUDED
#include <com/define.h>
#endif

//----------------------------------------------------------------------------
// Controls
//----------------------------------------------------------------------------
#ifndef STATISTICS
#undef  STATISTICS                  // If defined, keep statistics
#endif

//----------------------------------------------------------------------------
// Type definitions
//----------------------------------------------------------------------------
typedef unsigned char
                     Boolean;       // Boolean type
typedef unsigned char*
                     String;        // String type
typedef uint32_t     Tick;          // Clock tick type
typedef float        Value;         // Value type
typedef float        Weight;        // Weight type

#endif // TYPES_H_INCLUDED
