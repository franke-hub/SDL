//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       syslib.cpp
//
// Purpose-
//       System library functions.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <com/Debug.h>
#include "com/syslib.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             whocares= 0; // A useless counter

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const char*            NULL_POINTER= NULL; // For offsetof macro

//----------------------------------------------------------------------------
// Undefs
//----------------------------------------------------------------------------
#undef malloc
#undef free
#undef new
#undef delete

//----------------------------------------------------------------------------
//
// Subroutine-
//       atox
//
// Purpose-
//       Convert hexidecimal string to long.
//
//----------------------------------------------------------------------------
extern "C" long                     // Resultant value
   atox(                            // Convert ascii to hexidecimal
     const char*       ptrstr)      // Pointer to string
{
   unsigned long       newvalue= 0; // Current value

   if( *ptrstr == '0' && toupper(ptrstr[1]) == 'X' )
     ptrstr += 2;
   if( *ptrstr == '\0' )            // If empty string
       errno= EINVAL;

   while (*ptrstr != '\0')          // Convert the string
   {
     unsigned long oldvalue= newvalue;
     newvalue <<= 4;                // Position new value
     if( (newvalue>>4) != oldvalue ) // If overflow
       errno= EINVAL;

     if (*ptrstr >= '0' && *ptrstr <= '9')// If numeric
       newvalue += *ptrstr - '0';   // Add on the character value
     else if (*ptrstr >= 'a' && *ptrstr <= 'f')// Lowercase a..f
       newvalue += (*ptrstr - 'a') + 10; // Add on the character value
     else if (*ptrstr >= 'A' && *ptrstr <= 'F')// Uppercase A..F
       newvalue += (*ptrstr - 'A') + 10; // Add on the character value
     else                           // If invalid character
       errno= EINVAL;

     ptrstr++;                      // Address the next character
   }

   return(newvalue);                // Return the integer value
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nop
//
// Purpose-
//       Reference a variable.
//
//----------------------------------------------------------------------------
extern "C" void
   nop(                             // No function
     const void*       ignored)     // Quasi-refer to this
{
   if (ignored == NULL)
     whocares++;
}

