//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Edit.cpp
//
// Purpose-
//       Editor: Mainline.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EdLine.h"
#include "EdRing.h"
#include "Editor.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>            // Must follow define/undef HCDM

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Editor*                editor= NULL;// -> THE Edit object

#ifdef HCDM
static const char*     edparm[]=    // Default edit object, for bringup test
  #ifdef _OS_WIN
     {"", "makeproj.incl"};
  #else
     {"", "makefile.incl"};
  #endif
#endif

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
   main(                            // Editor
     int               argc,        // Argument count
     const char*       argv[])      // Array of argument pointers
{
   int                 i;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   IFHCDM( if( argc < 2 ) return main(2, edparm); )
   if (argc < 2)
   {
     printf("No filename specified\n");
     return 2;
   }

   //-------------------------------------------------------------------------
   // Debugging hook
   //-------------------------------------------------------------------------
   #if defined(HCDM) && FALSE
     debugSetIntensiveMode();
     tracef("%.4lX= sizeof(Editor)\n", (long)sizeof(Editor));
     tracef("%.4lX= sizeof(EdLine)\n", (long)sizeof(EdLine));
     tracef("%.4lX= sizeof(EdRing)\n", (long)sizeof(EdRing));
   #endif

// try {
     //-----------------------------------------------------------------------
     // Initialize the editor
     //-----------------------------------------------------------------------
     try {
       editor= new Editor();        // Create the Editor object
     } catch(...) {
       throw "No storage";
     }

     //-----------------------------------------------------------------------
     // Load the initial rings
     //-----------------------------------------------------------------------
     for(i= 1; i<argc; i++)          // Load the initial rings
       editor->insertRing(argv[i]);

     //-----------------------------------------------------------------------
     // Edit
     //-----------------------------------------------------------------------
     editor->run();

// } catch(const char* X) {
//   fprintf(stderr, "Exception(%s)\n", X);
// } catch(...) {
//   fprintf(stderr, "Exception(...)\n");
// }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   delete editor;                   // Destroy the object
   return 0;                        // Normal completion
}

