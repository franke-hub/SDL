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
//       Pool.h
//
// Purpose-
//       Storage pool.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef POOL_H_INCLUDED
#define POOL_H_INCLUDED

#ifndef LIST_H_INCLUDED
#include "List.h"
#endif

//----------------------------------------------------------------------------
// Foward references
//----------------------------------------------------------------------------
class PoolLink;

//----------------------------------------------------------------------------
//
// Class-
//       Pool
//
// Purpose-
//       Storage pool.
//
//----------------------------------------------------------------------------
class Pool : private List<PoolLink> {   // Storage pool
//----------------------------------------------------------------------------
// Pool::Attributes
//----------------------------------------------------------------------------
private:
// None defined

//----------------------------------------------------------------------------
// Pool::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Pool( void );                   // Destructor
   Pool( void );                    // Constructor

//----------------------------------------------------------------------------
// Pool::Diagnostic methods
//----------------------------------------------------------------------------
public:
virtual int                         // TRUE if coherent
   isCoherent( void ) const;        // Diagnostic coherency check

virtual void
   diagnosticDump( void ) const;    // Diagnostic storage dump

//----------------------------------------------------------------------------
// Pool::methods
//----------------------------------------------------------------------------
public:
virtual void*                       // -> Allocated storage
   allocate(                        // Allocate from Pool
     unsigned long     size);       // Required size

virtual void
   release(                         // Return to Pool
     void*             addr,        // Release address
     unsigned long     size);       // Release size

virtual void
   reset( void );                   // Delete all Pool storage
}; // class Pool

#endif // POOL_H_INCLUDED
