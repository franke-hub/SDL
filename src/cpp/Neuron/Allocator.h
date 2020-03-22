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
//       Allocator.h
//
// Purpose-
//       Define the base Neural Net Allocator.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef ALLOCATOR_H_INCLUDED
#define ALLOCATOR_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include "Object.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Allocator
//
// Purpose-
//       Allocator descriptor.
//
// Implementation notes-
//       NO implementation. (Scaffolded)
//       In particular, how do we call the load method to create an object
//       without having the object there already? What parameters are needed
//       for the load and dump methods?
//
// Implementation notes-
//       The update method will simply call all the objects in the Object
//       vector, in the order that they were originally added.
//
//       There is probably a need for stacked Allocator objects. The base
//       object will have very little associated storage, perhaps just a
//       file name. That file will begin loading the remainder of a
//       checkpoint. Need to account for multi-threading, too.
//
//----------------------------------------------------------------------------
class Allocator : public Object {   // Allocator descriptor
//----------------------------------------------------------------------------
// Allocator::Attributes
//----------------------------------------------------------------------------
protected:
void*                  addr;        // The Allocator storage address
unsigned int           size;        // The Allocator storage size
unsigned int           used;        // The number of allocated bytes

Object**               object;      // The Object vector
unsigned int           oSize;       // The number of available Object slots
unsigned int           oUsed;       // The number of used Object slots

//----------------------------------------------------------------------------
// Allocator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Allocator( void );              // Destructor
   Allocator(                       // Constructor
     unsigned int      size);       // The allocation size

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::address2offset
//
// Purpose-
//       Convert address to offset.
//
// Usage notes-
//       Used when dumping objects.
//
//----------------------------------------------------------------------------
public:
unsigned int                        // The assocated offset (0 if invalid)
   address2offset(                  // Convert address to offset
     void*             address);    // The address to convert

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::offset2address
//
// Purpose-
//       Convert offset to address.
//
// Usage notes-
//       Used when loading objects.
//
//----------------------------------------------------------------------------
public:
void*                               // The assocated address
   offset2address(                  // Convert offset to address
     unsigned int      offset);     // The offset to convert

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::allocate
//
// Purpose-
//       Allocate data storage
//
// Usage notes-
//       Allocated storage cannot be released.
//
//----------------------------------------------------------------------------
public:
void*                               // -> Allocated storage
   allocate(                        // Allocate permanent data storage
     unsigned int      length);     // Of this length

//----------------------------------------------------------------------------
//
// Method-
//       Allocator::storeObject
//
// Purpose-
//       Store an Object in the Object vector
//       (The vector is expanded if necessary.)
//
//----------------------------------------------------------------------------
public:
void
   storeObject(                     // Add an Object to the Object vector
     Object*           object);     // The Object to store

//----------------------------------------------------------------------------
// Allocator::Methods
//----------------------------------------------------------------------------
public:
virtual void
   dump( void );                    // Dump the Allocator

virtual void
   load( void );                    // Load the Allocator

virtual void
   update( void );                  // Read inputs, write outputs
}; // class Allocator

#endif // ALLOCATOR_H_INCLUDED
