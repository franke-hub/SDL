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
//       Viewer.cpp
//
// Purpose-
//       Viewer master control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>

#include <gui/Window.h>
using namespace GUI;

#include "JpegDecoder.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   unsigned int        errorCount= 0;
   Window              window;

   JpegDecoder decoder(window);
   errorCount += decoder.decode(argv[1]);

   if( errorCount == 0 )
   {
     window.setAttribute(Object::VISIBLE, TRUE);
     window.change();
     window.wait();

     // For destructors, avoid visiblity
     window.setAttribute(Object::VISIBLE, FALSE);
   }

   if( errorCount != 0 )
     errorCount= 1;

   return errorCount;
}

