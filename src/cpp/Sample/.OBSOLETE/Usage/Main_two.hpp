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
//       Main_two.hpp
//
// Purpose-
//       Included command processor, alternate implementation to Main_one.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       This was originally developed for implementation testing.
//
//----------------------------------------------------------------------------
#include <exception>

#include <stdio.h>
#include <string.h>

#include "Command.h"
#include "TemplateException.h"

namespace Main_two {
static Command         command;

//----------------------------------------------------------------------------
// Our local Command dictionary
//----------------------------------------------------------------------------
static class Main {
public:
//----------------------------------------------------------------------------
// The 'botched' command: Throws NullPointerException("BOTCHED")
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   botched(                         // The 'botched' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   throw NullPointerException("BOTCHED");
   return 2;                        // You can't get here from there
}

//----------------------------------------------------------------------------
// The test 'echo' command
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   echo(                            // The echo command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{  for(int i= 0; i<argc; i++)
     printf("[%2d] '%s'\n", i, argv[i]);
   return 0;
}

//----------------------------------------------------------------------------
// The 'hello' command: Hello world!
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   hello(                           // The 'hello' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   std::cout << "Hello from Main_two.hpp\n";
   return 0;
}

//----------------------------------------------------------------------------
// The test 'list' command (Local 'list' command.)
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   list(                            // The built-in 'list' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   std::string list= command.list();
   std::cout << "Command list: " << list << std::endl << std::flush;
   return 0;
}

//----------------------------------------------------------------------------
//
// Command-
//       template
//
// Purpose-
//       Test TemplateException.h, now OBSOLETE.
//
// Implementation note-
//       While TemplateException.h works, the class structure is cumbersome.
//       It's easier to allow the exception class to be extracted from the
//       Exception object than to build class name during the constructor,
//       like TemplateException does. Note that std::runtime_error.what()
//       only provides the descriptor, not the class name, so it's what
//       users of the std::runtime_exception would expect.
//
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   template_test(                   // Test TemplateException.h, now OBSOLETE
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   (void)argc; (void)argv;          // Currently unused
   try {
     throw TemplateException("IS: TemplateException");
   } catch(TemplateException& x) {
     printf("catch(TemplateExeception.what(%s))\n", x.what());
   }

   try {
     throw TemplateKeyError("IS: TemplateKeyError");
   } catch(TemplateException& x) {
     printf("catch(TemplateExeception.what(%s))\n", x.what());
   }

   return 0;
}

Main( void ) {                      // The constructor
command.set("echo", echo);
command.set("hello", hello);
command.set("list", list);
} // end Main constructor

//----------------------------------------------------------------------------
//
// Method-
//       Main::main
//
// Purpose-
//       The Main_two command processor.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Parameter count
     const char*     argv[])        // Parameter vector
{
   int result= 0;

   if( argc > 1 )                   // We ony run specific tests
     result= command.main(argc, argv, false);
   else
     std::cout << "rc(0)" << std::endl;

   return result;
}
} main_instance;
} // namespace Main_two

#ifndef __MAIN__
#define __MAIN__ Main_two

int main(int argc, const char* argv[]) {
   return Main_two::main_instance.main(argc, argv);
}
#else
static int
   two(int argc, const char* argv[])
{  return Main_two::main_instance.main(argc, argv); }
INSTALL_COMMAND(two)
#endif

