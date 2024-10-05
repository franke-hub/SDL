//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ConsoleCommand.h
//
// Purpose-
//       The ConsoleCommand is the standard command handler.
//
// Last change date-
//       2024/09/30
//
// Implementation notes-
//       argv[0] *INP* The input string
//       argv[1] *OUT* The command name
//
//----------------------------------------------------------------------------
#ifndef CONSOLE_COMMAND_H_INCLUDED
#define CONSOLE_COMMAND_H_INCLUDED

#include "Command.h"

//----------------------------------------------------------------------------
//
// Class-
//       ConsoleCommand
//
// Purpose-
//       The ConsoleCommand is the standard command handler.
//
//----------------------------------------------------------------------------
class ConsoleCommand : public Command { // The standard command handler
//----------------------------------------------------------------------------
// ConsoleCommand::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// ConsoleCommand::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleCommand( void )           // Constructor
:  Command("Console") {}

   ConsoleCommand(const ConsoleCommand&) = delete; // Disallowed copy constructor
   ConsoleCommand& operator=(const ConsoleCommand&) = delete; // Disallowed assignment operator

virtual
   ~ConsoleCommand( void ) {}       // Destructor

//----------------------------------------------------------------------------
// ConsoleCommand::Methods
//----------------------------------------------------------------------------
public:
virtual Command::resultant          // Resultant
   work(                            // Process the ConsoleCommand
     int               argc,        // Argument count
     char*             argv[]);     // Argument array
}; // class ConsoleCommand
#endif // CONSOLE_COMMAND_H_INCLUDED
