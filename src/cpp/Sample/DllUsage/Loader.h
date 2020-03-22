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
//       Loader.h
//
// Purpose-
//       Define the Dynamically Linked Loader object.
//
// Last change date-
//       2012/01/01
//
// Usage notes-
//       In order to avoid linking problems, only virtual methods in the
//       Interface or (or its derviation) may used. All Interface functionality
//       is provided within the DLL.
//
//       The dynamic load library must contain two entry points:
//       DLL_make, extern "C" Interface* DLL_make( void ), and
//       DLL_take, extern "C" void DLL_take(Interface*).
//       These functions are called from Loader::make/take, respectively.
//
//----------------------------------------------------------------------------
#ifndef LOADER_H_INCLUDED
#define LOADER_H_INCLUDED

#include <string>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Interface;

//----------------------------------------------------------------------------
//
// Class-
//       Loader
//
// Purpose-
//       DLL Loader
//
// Implementation notes-
//       Although the make and take methods are provided, the Loader class
//       is NOT derived from the Factory class.
//
//----------------------------------------------------------------------------
class Loader {                      // DLL Loader
//----------------------------------------------------------------------------
// Loader::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef    Interface* (*Makef)(void);
typedef    void (*Takef)(Interface*);

//----------------------------------------------------------------------------
// Loader::Attributes
//----------------------------------------------------------------------------
protected:
std::string            name;        // Name (ONLY USED FOR DEBUGGING)
void*                  handle;      // Library handle
Makef                  makef;       // MAKE Function pointer
Takef                  takef;       // TAKE Function pointer

//----------------------------------------------------------------------------
// Loader::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Loader( void );                 // Destructor
   Loader(                          // Constructor
     const char*       name);       // The name of the library

private:                            // Bitwise copy is prohibited
   Loader(const Loader&);           // Disallowed copy constructor
Loader&
   operator=(const Loader&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Loader::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void );                   // Debugging display

virtual Interface*                  // Resultant Interface Object
   make( void );                    // Create an Interface Object

virtual void
   take(                            // Recycle
     Interface*        object);     // This Interface object
}; // class Loader

#endif // LOADER_H_INCLUDED
