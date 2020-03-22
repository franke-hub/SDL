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
//       Main_one.hpp
//
// Purpose-
//       Included command processor, contains confirmed tests.
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

namespace Main_one {
//----------------------------------------------------------------------------
// Our local Command dictionary
//----------------------------------------------------------------------------
#undef  INSTALL_COMMAND_AT
#define INSTALL_COMMAND_AT (::Main_one::command)

static Command         command;

//----------------------------------------------------------------------------
// The 'botched' command: Throws NullPointerException("BOTCHED")
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   botched(                         // The 'botched' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   throw NullPointerException("BOTCHED");
   return 2;                        // You can't get here from there
}

// INSTALL_COMMAND(botched)
INSTALL_COMMAND_NAME("botched", botched, Main_one::botched)

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

INSTALL_COMMAND_NAME("echo", echo, Main_one::echo)

//----------------------------------------------------------------------------
// The 'hello' command: Hello world!
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   hello(                           // The 'hello' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{
   std::cout << "Hello from Main_one.hpp\n";
   return 0;
}

INSTALL_COMMAND_NAME("hello", hello, Main_one::hello)

//----------------------------------------------------------------------------
// The test 'list' command (Local 'list' command.)
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   list(                            // The built-in 'list' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{  std::string list= command.list();
   std::cout << "Command list: " << list << std::endl << std::flush;
   return 0;
}

INSTALL_COMMAND_NAME("list", list, Main_one::list)

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

INSTALL_COMMAND_NAME("template", template_test, Main_one::template_test)

//----------------------------------------------------------------------------
// The 'try' command: (Uses reserved keyword try for command name)
//----------------------------------------------------------------------------
static int                          // Return code, always 0
   try_(                            // The 'try' command
     int               argc,        // Argument count
     const char*       argv[])      // Arguments
{  std::cout << "try worked. It does nothing but write this message.\n";
   return 0;
}

INSTALL_COMMAND_NAME("try", try_, Main_one::try_)


//----------------------------------------------------------------------------
//
// Subroutine-
//       Main_one::main
//
// Purpose-
//       The Main_one command processor.
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
} // namespace Main_one

#undef  INSTALL_COMMAND_AT
#define INSTALL_COMMAND_AT DEFAULT_COMMAND_AT
INSTALL_COMMAND_NAME("one", Main_one, Main_one::main)

