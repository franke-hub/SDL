//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sorter.h
//
// Purpose-
//       Bubble sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SORTER_H_INCLUDED
#define SORTER_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include "Object.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Sorter
//
// Purpose-
//       Sorter base class.
//
//----------------------------------------------------------------------------
class Sorter {                      // Sorter base class
//----------------------------------------------------------------------------
// Sorter::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Sorter( void );                 // Destructor
   Sorter( void );                  // Constructor

//----------------------------------------------------------------------------
// Sorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const = 0;  // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array) = 0;  // Object array

//----------------------------------------------------------------------------
// Sorter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class Sorter

#endif // SORTER_H_INCLUDED
