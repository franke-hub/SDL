//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2022 Frank Eskesen.
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
//       2022/06/04
//
// Comparison operators-
//       op_lt_istr    Case insensitive s LT operator
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_UTILITY_H_INCLUDED
#define _LIBPUB_UTILITY_H_INCLUDED

#include <string>                   // For std::string
#include <thread>                   // For std::thread

#include <ctype.h>                  // For toupper()
#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For ::FILE*

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Namespace-
//       utility
//
// Purpose-
//       Implement pub/utility functions.
//
// Implementation notes (ato* routines)-
//       These routines DO NOT set errno= 0.
//       Leading white space is ignored, trailing white space is allowed.
//       Invalid (hexi)decimal characters are NOT allowed.
//
//----------------------------------------------------------------------------
namespace utility {
//----------------------------------------------------------------------------
// Volatile data (For avoiding compiler optimizations)
//----------------------------------------------------------------------------
extern volatile int    data;        // For any use
extern volatile int    unit;        // By convention, always 1
extern volatile int    zero;        // By convention, always 0
extern int nop( void );             // Returns zero. Don't tell the compiler!

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atoi
//
// Purpose-
//       Convert string to integer.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
int                                 // Resultant value
   atoi(                            // Convert ASCII to signed integer
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atol
//
// Purpose-
//       Convert string to long integer.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
long                                // Resultant value
   atol(                            // Convert ASCII to signed long
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atoui
//       utility::atoul
//
// Purpose-
//       Unsigned versions of atoi, atol.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant value
   atoui(                           // Convert ASCII to unsigned integer
     const char*       inp);        // Input string

unsigned long                       // Resultant value
   atoul(                           // Convert ASCII to unsigned long
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::atox
//
// Purpose-
//       Convert hexidecimal string to long.
//
// Implementation notes-
//       errno= EINVAL; // Indicates invalid value detected.
//       errno= ERANGE; // Indicates invalid range detected.
//
//----------------------------------------------------------------------------
long                                // Resultant value
   atox(                            // Convert ASCII to hexidecimal long
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::clock
//
// Purpose-
//       Get current epoch time, in nanoseconds.
//
//----------------------------------------------------------------------------
uint64_t                            // The nanoseconds since epoch start
   clock( void );                   // Get nanoseconds since epoch start

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

void                                // Dump formatter (trace and stdout)
   dump(                            // Dump formatter
     const void*       addrp,       // Input data address
     size_t            size);       // Input data size

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::find_space
//
// Purpose-
//       Find next whitespace (or '\0') character.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   find_space(                      // Find next space character
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::skip_space
//
// Purpose-
//       Find next non-whitespace character, including '\0'.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   skip_space(                      // Find next non-whitespace character
     const char*       inp);        // Input string

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::strcasecmp
//       utility::strncasecmp
//
// Purpose-
//       String compare, ignoring case.
//
// Implementation note-
//       Unsigned string comparison is used, allowing UTF8 comparison.
//
//       ::strcasecmp and ::strncasecmp might not be included in string.h,
//       so they are included here.
//
//----------------------------------------------------------------------------
int                                 // Resultant <0,=0,>0
   strcasecmp(                      // ASCII string insensitive compare
     const char*       L,           // Left hand side
     const char*       R);          // Right hand side

int                                 // Resultant <0,=0,>0
   strncasecmp(                     // ASCII string insensitive compare
     const char*       L,           // Left hand side
     const char*       R,           // Right hand side
     size_t            size);       // Maximum comparison length

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::wildchar::strcmp
//       utility::wildchar::strcasecmp
//
// Purpose-
//       String comparison with wild character substitution.
//
// Implementation note-
//       Character '*' matches any number of (including zero) characters.
//       Character '?' matches any single character.
//
//       Only the left hand string may contain wild charaacters.
//
//----------------------------------------------------------------------------
namespace wildchar {                // Wild character string comparison
int                                 // Resultant 0, !0
   strcmp(                          // String compare
     const char*       L,           // Left hand side (May contain wildchars)
     const char*       R);          // Right hand side

int                                 // Resultant 0, !0
   strcasecmp(                      // String insensitive compare
     const char*       L,           // Left hand side (May contain wildchars)
     const char*       R);          // Right hand side
}  // namespace wildchar

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::to_ascii
//
// Purpose-
//       Convert to ascii
//
//----------------------------------------------------------------------------
static inline const char*           // Resultant
   to_ascii(bool what)              // Convert boolean to ascii
{  return what ? "true" : "false"; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       utility::to_string
//
// Purpose-
//       Convert printf format/arguments to std::string.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(1, 2)
std::string                         // Resultant
   to_string(                       // Create string from printf arguments
     const char*       fmt,         // Format string
                       ...);        // PRINTF arguments

_LIBPUB_PRINTF(1, 0)
std::string                         // Resultant
   to_stringv(                      // Create string from printf arguments
     const char*       fmt,         // Format string
     va_list           args);       // PRINTF arguments

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

//============================================================================
// Operator structures
//============================================================================
//
// Struct-
//       utility::op_lt_istr
//
// Purpose-
//       Define the less than operator for case-insensitive strings.
//
//----------------------------------------------------------------------------
struct op_lt_istr {
bool operator()(const std::string& L, const std::string& R) const
{  return strcasecmp(L.c_str(), R.c_str()) < 0; }
}; // struct op_lt_istr
}  // namespace utility
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_UTILITY_H_INCLUDED
