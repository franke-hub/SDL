//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       built_in.h
//
// Purpose-
//       Define the (local) built-in functions.
//
// Last change date-
//       2019/03/16
//
//----------------------------------------------------------------------------
#ifndef BUILT_IN_H_INCLUDED
#define BUILT_IN_H_INCLUDED

#include <string>
#include <stdarg.h>

#include "define.h"
#include "Exception.h"

//----------------------------------------------------------------------------
//
// Class-
//       built_in
//
// Purpose-
//       Class for built-in functions
//
//----------------------------------------------------------------------------
class built_in {
public:
enum { BUFFER_SIZE= 512 };          // to_string default buffer size

static inline char*                 // Always buffer
   to_buffer(                       // Format a string, return buffer
     char*             buffer,      // The buffer and resultant
     size_t            length,      // The buffer length
     const char*       fmt,         // The PRINTF format string
     va_list           args)        // The PRINTF argument list
   _ATTRIBUTE_PRINTF(3, 0)
{
   vsnprintf(buffer, length, fmt, args);
   return buffer;
}

static inline char*                 // Always buffer
   to_buffer(                       // Format a string, return buffer
     char*             buffer,      // The buffer and resultant
     size_t            length,      // The buffer length
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(3, 4)
{
   va_list args;
   va_start(args, fmt);
     vsnprintf(buffer, length, fmt, args);
   va_end(args);

   return buffer;
}

static inline std::string           // Resultant string
   to_stringv(                      // Format a string
     const char*       fmt,         // The PRINTF format string
     va_list           args)        // The PRINTF argument list
   _ATTRIBUTE_PRINTF(1, 0)
{
   std::string         result;      // Resultant

   char autobuff[BUFFER_SIZE];      // Working buffer
   char* buffer= autobuff;          // -> Buffer

   va_list copy;
   va_copy(copy, args);
   size_t L= vsnprintf(buffer, sizeof(autobuff), fmt, copy);
   va_end(copy);

   if( L < sizeof(autobuff) )       // If the normal case
     result= std::string(buffer);
   else
   {
     buffer= (char*)malloc(L+1);
     if( buffer == nullptr )
       throw NoStorageException("to_string");

     vsnprintf(buffer, L, fmt, args);
     result= std::string(buffer);
     free(buffer);
   }

   return result;
}

static inline std::string           // Resultant string
   to_string(                       // Format a string
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(1, 2)
{
   std::string         result;      // Resultant

   va_list args;
   va_start(args, fmt);
     result= to_stringv(fmt, args);
   va_end(args);

   return result;
}
}; // class built_in

#endif // BUILT_IN_H_INCLUDED
