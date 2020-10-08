//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Command processor.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <exception>

#include <stdio.h>

#include "Command.h"
#include "Exception.h"              // (Already included from Command.h)

//----------------------------------------------------------------------------
// The 'dirty' command: For quick and dirty testing
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   dirty(                           // The 'dirty' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   return 1;
}

INSTALL_COMMAND(dirty)

//----------------------------------------------------------------------------
// The 'hello' command: Hello world!
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   hello(                           // The 'hello' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   std::cout << "Hello from Main.cpp\n";
   return 0;
}

INSTALL_COMMAND(hello)

//----------------------------------------------------------------------------
// The 'unused' command: Demonstrate unused commands
//----------------------------------------------------------------------------
#ifndef _USE_UNUSED_COMMAND
#define _USE_UNUSED_COMMAND false
#endif

#if _USE_UNUSED_COMMAND
static int                          // Return code, always 0
   unused(                          // The 'unused' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{  printf("Unused via #ifdef\n");
   return 1;
}

INSTALL_COMMAND(unused)
#else
// Alternatively, you can make the command inline and comment out the INSTALL_
inline static int                   // Return code, always 0
   unused(                          // The 'unused' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   printf("Unused via comment out\n");
   return 1;
}

// INSTALL_COMMAND(unused)
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
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Parameter count
     const char*     argv[])        // Parameter vector
{
   (void)argc; (void)argv;          // Currently unused
   return INSTALL_COMMAND_AT.main(argc, argv);
}

#define __MAIN__ Main
#include "Main_one.hpp"
#if 0
  #include "Main_two.hpp"
#endif

