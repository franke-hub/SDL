//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Plugin.h
//
// Purpose-
//       Define the Plugin attachment object.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       This is a base class for Plugin attachments. Derived classes
//       provide the Plugin function, which is provided in a derived
//       Interface class.
//
//----------------------------------------------------------------------------
#ifndef PLUGIN_H_INCLUDED
#define PLUGIN_H_INCLUDED

#ifndef LOADER_H_INCLUDED
#include "Loader.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Interface;

//----------------------------------------------------------------------------
//
// Class-
//       Plugin
//
// Purpose-
//       Plugin attachment.
//
//----------------------------------------------------------------------------
class Plugin : protected Loader {   // Plugin attachment
//----------------------------------------------------------------------------
// Plugin::Attributes
//----------------------------------------------------------------------------
protected:
Interface*             interface;   // The associated Interface object

//----------------------------------------------------------------------------
// Plugin::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Plugin( void );                 // Destructor
   Plugin(                          // Constructor
     const char*       name);       // The name of the library

private:                            // Bitwise copy is prohibited
   Plugin(const Plugin&);           // Disallowed copy constructor
Plugin&
   operator=(const Plugin&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Plugin::Methods
//----------------------------------------------------------------------------
public:
}; // class Plugin

#endif // PLUGIN_H_INCLUDED
