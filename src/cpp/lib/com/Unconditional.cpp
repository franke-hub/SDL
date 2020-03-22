//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Unconditional.cpp
//
// Purpose-
//       Unconditional functions.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>

#include "com/Unconditional.h"

//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::malloc
//
// Purpose-
//       Allocate storage, throw exception on failure.
//
//----------------------------------------------------------------------------
void*                               // Resultant
   Unconditional::malloc(           // Allocate storage
     size_t            size)        // Number of bytes to allocate
{
   void*               result;      // Resultant

   result= ::malloc(size);
   if( result == NULL )
     throwf("Unconditional::malloc(%zu)\n", size);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::replace
//
// Purpose-
//       Replace an existing (allocated) string, either by duplicating its
//       replacement (if replace != NULL) or by freeing it
//
//----------------------------------------------------------------------------
char*                               // -> (Duplicated) replacement string
   Unconditional::replace(          // Replace an allocated string
     char*             current,     // -> Current string
     const char*       replace)     // -> Replacement string
{
   if( current != NULL )
   {
     free(current);
     current= NULL;
   }

   if( replace != NULL )
   {
     current= ::strdup(replace);
     if( current == NULL )
       throwf("Unconditional::replace(N/A,%s)\n", replace);
   }

   return current;
}

//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::strdup
//
// Purpose-
//       Duplicate string, throw exception on failure.
//
//----------------------------------------------------------------------------
char*                               // -> Duplicated string
   Unconditional::strdup(           // Duplicate
     const char*       source)      // This string
{
   char*               result;      // Resultant

   result= ::strdup(source);
   if( result == NULL )
     throwf("Unconditional::strdup(%s)\n", source);

   return result;
}

