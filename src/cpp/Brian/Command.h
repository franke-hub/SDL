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
//       Command.h
//
// Purpose-
//       A Command is a Named work handler.
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/Named.h>              // For pub::Named, base class
#include <pub/Object.h>             // For work resultant

//----------------------------------------------------------------------------
//
// Class-
//       Command
//
// Purpose-
//       A Command is a Named work handler
//
// Implementation notes-
//       The constructor inserts the Command into the map.
//       The destructor  removes the Command from the map.
//
//----------------------------------------------------------------------------
class Command : public pub::Named { // Command
//----------------------------------------------------------------------------
// Command::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Command*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator
typedef std::shared_ptr<pub::Object>          resultant; // Method work result

//----------------------------------------------------------------------------
// Command::Constructors/destructor
//----------------------------------------------------------------------------
   Command(                         // Constructor
     const char*       name);       // The Command name

   Command(const Command&) = delete; // Disallowed copy constructor
   Command& operator=(const Command&) = delete; // Disallowed assignment operator

virtual
   ~Command( void );                // Destructor

//----------------------------------------------------------------------------
// Command::Accessors
//----------------------------------------------------------------------------
static Map_t*                       // The Command Map*
   get_map( void );                 // Get the Command Map

static Command*                     // The associated Command, if present
   locate(std::string);             // Get associated Command

//----------------------------------------------------------------------------
// Command::Methods
//----------------------------------------------------------------------------
virtual resultant                   // Resultant, command dependent
   work(                            // Process the Command
     int               argc,        // Argument count
     char*             argv[]);     // Argument array
}; // class Command
#endif // COMMAND_H_INCLUDED
