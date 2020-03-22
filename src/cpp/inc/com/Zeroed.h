//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Zeroed.h
//
// Purpose-
//       Zeroed storage base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef ZEROED_H_INCLUDED
#define ZEROED_H_INCLUDED

#include "types.h"                  // For size_t

//----------------------------------------------------------------------------
//
// Class-
//       Zeroed
//
// Purpose-
//       Zeroed storage is zeroed upon allocation.
//
//----------------------------------------------------------------------------
class Zeroed {                      // Zeroed storage
//----------------------------------------------------------------------------
// Zeroed::Attributes
//----------------------------------------------------------------------------
protected:
   // This class contains no attributes

//----------------------------------------------------------------------------
// Zeroed::Constructors
//----------------------------------------------------------------------------
public:
// ~Zeroed( void );                 // *NO* default destructor
// Zeroed( void );                  // *NO* default constructor

//----------------------------------------------------------------------------
// Zeroed::operators
//----------------------------------------------------------------------------
static void*                        // -> Zeroed storage
   operator new(                    // Default operator new
     size_t            size);       // Size of object

static void
   operator delete(                 // Default operator delete
     void*             ptrVoid);    // -> Object

static void*                        // -> Zeroed storage
   operator new(                    // In-place operator new
     size_t            size,        // Size of object
     void*             ptrVoid);    // In-place address

static void
   operator delete(                 // In-place operator delete
     void*             ptrVoid,     // -> Object
     void*             dupVoid);    // In-place address

//----------------------------------------------------------------------------
// Zeroed::methods
//----------------------------------------------------------------------------
public:
   // This class contains no methods
}; // struct Zeroed

#endif // ZEROED_H_INCLUDED
