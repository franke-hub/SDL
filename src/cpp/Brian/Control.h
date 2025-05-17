//----------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Control.h
//
// Purpose-
//       The Control interface
//
// Last change date-
//       2025/01/20
//
// Implementation notes-
//       A Control is a replaceable interface. (The actual interface for a
//       control is defined by the Control subclass.)
//
//----------------------------------------------------------------------------
#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#include <map>                      // For std::map, ...
#include <string>                   // For std::string

#include "shared_ptr-debug.h"       // For shared_ptr debugging control
#include <pub/Named.h>              // For pub::Named, base class

//----------------------------------------------------------------------------
//
// Class-
//       Control
//
// Purpose-
//       All Controls are uniquely named.
//
//----------------------------------------------------------------------------
class Control : public pub::Named { // Control
//----------------------------------------------------------------------------
// Control::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Control*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator

//----------------------------------------------------------------------------
// Control::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Control(                         // Constructor
     const char*       name= nullptr); // The Control name

   Control(const Control&) = delete; // Disallowed copy constructor
   Control& operator=(const Control&) = delete; // Disallowed assignment operator

virtual
   ~Control( void );                // Destructor

//----------------------------------------------------------------------------
// Control::Static methods
//----------------------------------------------------------------------------
static Control*                     // The replaced Control
   insert(Control*);                // Insert (replace) the Control

static Control*                     // The Control mapped to the name
   locate(std::string);             // Locate the associated Control
}; // class Control
#endif // CONTROL_H_INCLUDED
