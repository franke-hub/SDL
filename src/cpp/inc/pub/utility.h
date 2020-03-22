//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       utility.h
//
// Purpose-
//       Utility functions.
//
// Last change date-
//       2020/01/24
//
// Comparison operators-
//       sti_cc   Case insensitive compare: Returns <0, =0, >0
//       sti_lt   Case insensitive LT operator
//
//       sts_cc   Case sensitive compare: Returns <0, =0, >0
// *NOT* sts_lt   Case sensitive LT operator NOT PROVIDED. It's in std::string.
//
//----------------------------------------------------------------------------
#ifndef _PUB_UTILITY_H_INCLUDED
#define _PUB_UTILITY_H_INCLUDED

#include <string>                   // For std::string
#include <thread>                   // For std::thread

#include <ctype.h>                  // For toupper()
#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For ::FILE*

#include "config.h"                 // For _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Namespace-
//       utility
//
// Purpose-
//       Implement pub/utility functions.
//
// Implementation notes-
//       Placeholder: TODO: Need to restructure pub/utility.h
//
//----------------------------------------------------------------------------
namespace utility {
//----------------------------------------------------------------------------
//
// Subroutine-
//       atox
//
// Purpose-
//       Convert hexidecimal string to long.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//
//----------------------------------------------------------------------------
long                                // Resultant value
   atox(                            // Convert ASCII to hexidecimal
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::dump
//
// Purpose-
//       Dump formatter.
//
//----------------------------------------------------------------------------
void                                // Dump formatter
   dump(                            // Dump formatter
     FILE*             file,        // Output FILE
     const void*       addrp,       // Input data address
     size_t            size,        // Input data size
     const void*       addrv);      // Virtual data address (Can be omitted)

void                                // Dump formatter
   dump(                            // Dump formatter
     FILE*             file,        // Output FILE
     const void*       addrp,       // Input data address
     size_t            size);       // Input data size

//----------------------------------------------------------------------------
//
// Struct-
//       utility::sti_cc
//
// Purpose-
//       Define the case-insensitive string comparison operator.
//
//----------------------------------------------------------------------------
struct sti_cc {
int operator()(const std::string& L, const std::string& R) const;
}; // struct sti_cc

//----------------------------------------------------------------------------
//
// Struct-
//       utility::sti_lt
//
// Purpose-
//       Define the case-insensitive string less than operator.
//
//----------------------------------------------------------------------------
struct sti_lt {
bool operator()(const std::string& L, const std::string& R) const;
}; // struct sti_lt

//----------------------------------------------------------------------------
//
// Struct-
//       utility::sts_cc
//
// Purpose-
//       Define the case-sensitive string comparison operator.
//
//----------------------------------------------------------------------------
struct sts_cc {
int operator()(const std::string& L, const std::string& R) const;
}; // struct sts_cc

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::to_string
//
// Purpose-
//       Convert printf format/arguments to std::string.
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   to_string(                       // Create string from printf arguments
     const char*       fmt,         // Format string
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(1, 2);

std::string                         // Resultant
   to_stringv(                      // Create string from printf arguments
     const char*       fmt,         // Format string
     va_list           args)        // PRINTF arguments
   _ATTRIBUTE_PRINTF(1, 0);

std::string                         // Resultant
   to_string(                       // Create string from std::thread::id
     const std::thread::id& id);    // The std::thread::id

std::string                         // Resultant string
   to_string(                       // Get id string
     volatile const std::thread::id& id); // For this id

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::visify
//
// Purpose-
//       Change control characters in string to their C++ equivalents.
//
// Implementation notes-
//       That is, '\n' converted to "\\n" (or '\\', 'n'), etc
//
//----------------------------------------------------------------------------
std::string                         // The visual representation
   visify(                          // Get visual representation of
     const std::string&inp);        // This string
}  // namespace utility
}  // namespace _PUB_NAMESPACE
#endif // _PUB_UTILITY_H_INCLUDED
