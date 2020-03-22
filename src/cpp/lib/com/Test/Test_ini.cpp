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
//       Test_ini.cpp
//
// Purpose-
//       ParseINI tests.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "com/ParseINI.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       resultant
//
// Purpose-
//       Show the resultant.
//
//----------------------------------------------------------------------------
void
   resultant(                       // Show resultant
     ParseINI&       object,        // This object
     const char*     sect,          // Object section
     const char*     name)          // Object name
{
   const char*       value;

   value= object.getValue(sect, name);
   if( sect == NULL )
     sect= "NULL";

   printf("'%s'= getValue([%s],'%s')\n", value, sect, name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       TEST_INI
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(void)                       // Mainline entry
{
   ParseINI          inTest;        // ParseINI object

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   inTest.construct();
   inTest.open("test_ini.ini");
   printf("\n\n");

   //-------------------------------------------------------------------------
   // Resultant tests
   //-------------------------------------------------------------------------
   resultant(inTest, NULL, "This");
   resultant(inTest, NULL, "this");
   resultant(inTest, "section", "this");
   resultant(inTest, NULL, "that");
   resultant(inTest, NULL, "other");
   resultant(inTest, NULL, "StandardEmpty");
   resultant(inTest, NULL, "AlternateEmpty");
   resultant(inTest, NULL, "unknown");

   resultant(inTest, NULL, " this value ");
   resultant(inTest, NULL, "this value");
   resultant(inTest, NULL, "that value");
   resultant(inTest, NULL, "other value");
   resultant(inTest, NULL, "unknown value");

   return 0;
}

