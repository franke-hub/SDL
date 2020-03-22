//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Exception.cpp
//
// Purpose-
//       Exception object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#if defined(__GNUC__) && defined(_OS_LINUX) // For backtrace
  #include <execinfo.h>
  #include <ucontext.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>
#include <com/define.h>

#include "com/Exception.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BT_ARRAY_DIM 128

//----------------------------------------------------------------------------
//
// Method-
//       Exception::backtrace
//
// Purpose-
//       Display backtrace information.
//
//----------------------------------------------------------------------------
void
   Exception::backtrace( void )     // Display backtrace information
{
#if defined(__GNUC__) && defined(_OS_LINUX)
   //-------------------------------------------------------------------------
   // Caller's backtrace
   void*               array[BT_ARRAY_DIM];

   debugf("\n");
   int size= ::backtrace(array, BT_ARRAY_DIM);
   char** messages= backtrace_symbols(array, size);
   if( messages != NULL )
   {
     for(int i= 0; i < size && messages[i] != NULL; ++i)
       debugf("[bt]: [%2d] %s\n", i, messages[i]);

     free(messages);
   }
#endif
}

