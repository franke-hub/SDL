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
//       HeapSorter.h
//
// Purpose-
//       Heap sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef HEAPSORTER_H_INCLUDED
#define HEAPSORTER_H_INCLUDED

#ifndef SORTER_H_INCLUDED
#include "Sorter.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       HeapSorter
//
// Purpose-
//       Heap sorter.
//
//----------------------------------------------------------------------------
class HeapSorter : public Sorter {  // Heap sorter
//----------------------------------------------------------------------------
// HeapSorter::Constructors
//----------------------------------------------------------------------------
public:
   ~HeapSorter( void );             // Destructor
   HeapSorter( void );              // Constructor

//----------------------------------------------------------------------------
// HeapSorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const;      // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array);      // Object array

//----------------------------------------------------------------------------
// HeapSorter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class HeapSorter

#endif // HEAPSORTER_H_INCLUDED
