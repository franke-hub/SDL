//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       testString.cpp
//
// Purpose-
//       Test std::string.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <string>

#include "Main.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#if defined(HCDM) && !defined(SCDM)
  #define SCDM
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     sourceString=
   "Source data string\n"
   "Used to test string constructor\n"
   ;

//----------------------------------------------------------------------------
//
// Subroutine-
//       demo
//
// Purpose-
//       Demo <string>
//
//----------------------------------------------------------------------------
static void
   demo( void )
{
   Logger::log("testString::demo()\n");

   string              string1= "abc";
   string              string2= string1;

   Logger::log("%4d string1(%p).c_str(%p)='%s' string2(%p).c_str(%p)='%s'\n", __LINE__,
       &string1, string1.c_str(), string1.c_str(),
       &string2, string2.c_str(), string2.c_str());

   string2= string1 + string2;
   Logger::log("%4d string1(%p).c_str(%p)='%s' string2(%p).c_str(%p)='%s'\n", __LINE__,
       &string1, string1.c_str(), string1.c_str(),
       &string2, string2.c_str(), string2.c_str());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testModification
//
// Purpose-
//       Show that string objects are passed by reference.
//
//----------------------------------------------------------------------------
static string                       // Resultant ("result string")
   testModification(                // Test modification
     string            source)      // Source string
{
   source= "result string";
   return source;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test00
//
// Purpose-
//       Simple string tests
//
//----------------------------------------------------------------------------
static void
   test00( void )
{
   string              string1= "abc";
   string              string2= string1;

   verify( string1 == string2 );

   string1= "this";
   string2= "this";
   verify( string1 == string2 );

   string2 += "that";
   verify( string1 != string2 );

   verify( strcmp(string1.c_str(), "this") == 0 );
   verify( strcmp(string2.c_str(), "thisthat") == 0 );

   string string3(sourceString+7, 11);
   verify( string3 == "data string");

   string1= "source string";
   string2= testModification(string1);
   verify( string1 == "source string" );
   verify( string2 == "result string");

}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testString
//
// Purpose-
//       Test <string>
//
//----------------------------------------------------------------------------
extern void
   testString( void )
{
   wtlc(LevelStd, "testString()\n");

   // String demo
   if( getLogLevel() < LevelStd )
     demo();

   // Test
   test00();
}

