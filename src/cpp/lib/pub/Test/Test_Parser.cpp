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
//       Test_Parser.cpp
//
// Purpose-
//       Test pub/Parser.h
//
// Last change date-
//       2020/12/30
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "pub/Parser.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       resultant
//
// Purpose-
//       Show the resultant.
//
//----------------------------------------------------------------------------
static void
   resultant(                       // Show resultant
     pub::Parser&      parser,      // For this parser,
     const char*       sect,        // This section, and
     const char*       parm)        // This parameter
{
   const char* value= parser.get_value(sect, parm);
   printf("'%s'= get_value(%s,%s)\n", value, sect, parm);
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
extern int                          // Return code
   main(void)                       // Mainline entry
{
   pub::Parser parser;
   long rc= parser.open("S/script/parser.inp");
   printf("%ld= open(S/script/parser.inp)\n", rc);
   parser.debug(); printf("\n\n");

   resultant(parser, nullptr, "This");
   resultant(parser, nullptr, "this");
   resultant(parser, nullptr, "that");
   resultant(parser, nullptr, "other");
   resultant(parser, nullptr, "StandardEmpty");
   resultant(parser, nullptr, "AlternateEmpty");
   resultant(parser, nullptr, "unknown");

   resultant(parser, "blank", "follow name");
   resultant(parser, "blank", " this name ");
   resultant(parser, "blank", "this name");
   resultant(parser, "blank", "that name");
   resultant(parser, "blank", "other name");
   resultant(parser, "blank", "unknown name");

   resultant(parser, "oldsect", "this");
   resultant(parser, "section", "this");
   resultant(parser, "newsect", "this");

   return 0;
}

