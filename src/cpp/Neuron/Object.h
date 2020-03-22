//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//--------------------------------------------------------------------------
//
// Title-
//       Object.h
//
// Purpose-
//       Define the base Neural Net Object.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       Object descriptor.
//
// Description-
//       Neural Net Object require the load, dump, and update methods to be
//       overridden. (The base Object provides no function for these methods.)
//
//----------------------------------------------------------------------------
class Object {                      // Object descriptor
//----------------------------------------------------------------------------
// Object::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Object( void );                 // Destructor
   Object( void );                  // Default constructor

//----------------------------------------------------------------------------
// Object::Methods
//----------------------------------------------------------------------------
public:
virtual void
   dump( void );                    // Dump the Object (NEEDS FILE PARAMETER)

virtual void
   load( void );                    // Load the Object (NEEDS FILE PARAMETER)

virtual void
   update( void );                  // Read inputs, write outputs
}; // class Object

#endif // OBJECT_H_INCLUDED
