//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_parser.cpp
//
// Purpose-
//       Test pub/Parser.h
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "pub/Parser.h"             // For pub::Parser

#define PUB _LIBPUB_NAMESPACE

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
     PUB::Parser&      parser,      // For this parser,
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
   PUB::Parser parser;
   long rc= parser.open("S/script/inp/parser.inp");
   printf("%ld= open(S/script/inp/parser.inp)\n", rc);
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

