//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
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
//       2007/01/01
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

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugMalloc
//
// Purpose-
//       Debugging version of malloc()
//
//----------------------------------------------------------------------------
extern "C" void*                    // Allocated storage address
   debugMalloc(                     // Debugging version of malloc
     const char*       file,        // File name
     int               line,        // File line
     size_t            size)        // Allocation length
{
   void*               resultant;   // Resultant

   resultant= malloc(size);
   tracef("%s %d: %p= malloc(%ld)\n", file, line, resultant, (long)size);

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugFree
//
// Purpose-
//       Debugging version of free()
//
//----------------------------------------------------------------------------
extern "C" void
   debugFree(                       // Debugging version of malloc
     const char*       file,        // File name
     int               line,        // File line
     void*             addr)        // Release address
{
   tracef("%s %d: free(%p)\n", file, line, addr);
   free(addr);
}

//----------------------------------------------------------------------------
//
// Function-
//       ::operator new(size_t, const char*, int)
//
// Purpose-
//       Traced ::operator new().
//
//----------------------------------------------------------------------------
void*                               // -> Allocated storage
   operator new(                    // In-place new operator
     size_t            size,        // Size of object
     const char*       file,        // File name
     int               line)        // File line
{
   void*               resultant;   // Pointer to storage

   resultant= ::operator new(size); // Allocate the storage
   tracef("%s %d: %p= operator new(%ld)\n", file, line, resultant, (long)size);

   return resultant;
}

//----------------------------------------------------------------------------
//
// Function-
//       ::operator delete(void*, size_t, const char*, int)
//
// Purpose-
//       Traced ::operator delete().
//
//----------------------------------------------------------------------------
void
   operator delete(                 // In-place delete operator
     void*             addr,        // Address of object
     const char*       file,        // File name
     int               line)        // File line
{
   tracef("%s %d: operator delete(%p)\n", file, line, addr);
   ::operator delete(addr);         // Release the storage
}

