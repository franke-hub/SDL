//----------------------------------------------------------------------------
//
//       Copyright (c) 2006 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.cpp
//
// Purpose-
//       Sample C++ program -- used to test inclusion.
//
// Last change date-
//       2006/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

#if 0
#include <nonexistant.h>
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "Sample  " // Source file

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int           nestLevel;     // Nesting level
static const char*   nestName[32];  // Nesting name

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       begin
//
// Purpose-
//       Begin nesting level
//
//----------------------------------------------------------------------------
extern void
   begin(                           // Begin nesting level
     const char*     whoami)        // This is who I am
{
   nestName[nestLevel++]= whoami;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       finis
//
// Purpose-
//       End nesting level
//
//----------------------------------------------------------------------------
extern void
   finis( void )                    // End nesting level
{
   nestLevel--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       level
//
// Purpose-
//       Display nesting level
//
//----------------------------------------------------------------------------
extern void
   level( void )                    // Show nesting level
{
   int               i;

   for(i=0; i<nestLevel; i++)
   {
     printf("%s", nestName[i]);
     if( i == (nestLevel-1) )
       printf("  ");
     else
       printf("::");
   }

}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iam
//
// Purpose-
//       Display "iam" message
//
//----------------------------------------------------------------------------
extern void
   iam(                             // Display "iam" message
     const char*     whoami)        // This is who I am
{
   printf("iam(%s)\n", whoami);     // Display message
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inca
//
// Purpose-
//       Include <iam> message
//
//----------------------------------------------------------------------------
extern void
   inca(                            // Include <iam> message
     const char*     whoami)        // This is who I am
{
   level();
   printf("#include <%s>\n", whoami); // Display message
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       incq
//
// Purpose-
//       Include "iam" message
//
//----------------------------------------------------------------------------
extern void
   incq(                            // Include "iam" message
     const char*     whoami)        // This is who I am
{
   level();
   printf("#include \"%s\"\n", whoami); // Display message
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(                            // Mainline code
     int             argc,          // Argument count
     char           *argv[])        // Argument array
{
   int               rc= 0;         // Called routine return code

   nestLevel= 0;
   begin("main");

   incq("inc.cpp");
#include "inc.cpp"
   printf("\n");

   inca("Sample.h");
#include <Sample.h>
   printf("\n");

   incq("Sample blank.h");
#include "Sample blank.h"
   printf("\n");

   incq("Sample.h");
#include "Sample.h"
   printf("\n");

   inca("foo.h");
#include <foo.h>
   printf("\n");

   incq("foo.h");
#include "foo.h"
   printf("\n");

   inca("sys/foo.h");
#include <sys/foo.h>
   printf("\n");

   incq("sys/foo.h");
#include "sys/foo.h"
   printf("\n");

   inca("sys/sys.h");
#include <sys/sys.h>
   printf("\n");

   incq("sys/sys.h");
#include "sys/sys.h"
   printf("\n");

   return rc;                       // Normal completion
}

