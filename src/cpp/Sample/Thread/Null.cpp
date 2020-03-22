//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Null.cpp
//
// Purpose-
//       Empty library routine, used to build an empty library.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NULL_LIB" // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       nop
//
// Purpose-
//       Do nothing in a way that the compiler can't tell.
//
//----------------------------------------------------------------------------
extern void
   nop(                             // Do nothing, but don't tell compiler
     void*             reference)   // Reference this variable
{
   const char*         message= "";

   if( reference == NULL )
     printf("%s", message);
}

