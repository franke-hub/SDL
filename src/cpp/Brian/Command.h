//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
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
//       2019/01/01
//
//----------------------------------------------------------------------------
#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <map>
#include <string>

#include <pub/Debug.h>
#include <pub/Named.h>

//----------------------------------------------------------------------------
//
// Class-
//       Command
//
// Purpose-
//       A Command is a Named work handler
//
//----------------------------------------------------------------------------
class Command : public pub::Named { // Command
//----------------------------------------------------------------------------
// Command::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// Command::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Command( void ) {}              // Destructor
   Command(                         // Constructor
     const char*       name)        // The Command name
:  pub::Named(name) {}

   Command(const Command&) = delete; // Disallowed copy constructor
   Command& operator=(const Command&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Command::Accessors
//----------------------------------------------------------------------------
public:
// std::string get_name() const     // Return the Named attribute

//----------------------------------------------------------------------------
// Command::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work(                            // Process the Command
     int               argc,        // Argument count
     char*             argv[]);     // Argument array
}; // class Command

//----------------------------------------------------------------------------
//
// Class-
//       CommandMap
//
// Purpose-
//       The name to Command map
//
//----------------------------------------------------------------------------
extern class CommandMap {           // The CommandMap
//----------------------------------------------------------------------------
// CommandMap::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Command*>
                       Map_t;       // The Map_t
typedef Map_t::iterator
                       MapIter_t;

//----------------------------------------------------------------------------
// CommandMap::Attributes
//----------------------------------------------------------------------------
protected:
static Map_t           map;         // The actual map

public:                             // Attribute accessors
MapIter_t begin() noexcept
{  return map.begin(); }

const MapIter_t begin() const noexcept
{  return map.begin(); }

MapIter_t end() noexcept
{  return map.end(); }

const MapIter_t end() const noexcept
{  return map.end(); }

//----------------------------------------------------------------------------
// CommandMap::operators
//----------------------------------------------------------------------------
public:
Command*                            // The associated Command, if present
   locate(std::string name) const;  // Get associated Command

void
   remove(Command* command);        // Remove associated Command

Command&                            // The associated Command
   operator[](std::string name) const; // Locate Command, name must be registered

Command&                            // The associated Command
   operator[](Command* command);    // Insert Command, name must be unique
}  CommandMap;

#endif // COMMAND_H_INCLUDED
