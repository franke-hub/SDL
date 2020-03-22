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
//       MergeSorter.h
//
// Purpose-
//       Merge sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MERGESORTER_H_INCLUDED
#define MERGESORTER_H_INCLUDED

#ifndef SORTER_H_INCLUDED
#include "Sorter.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       MergeSorter
//
// Purpose-
//       Merge sorter.
//
//----------------------------------------------------------------------------
class MergeSorter : public Sorter { // Merge sorter
//----------------------------------------------------------------------------
// MergeSorter::Constructors
//----------------------------------------------------------------------------
public:
   ~MergeSorter( void );            // Destructor
   MergeSorter( void );             // Constructor

//----------------------------------------------------------------------------
// MergeSorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const;      // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array);      // Object array

//----------------------------------------------------------------------------
// MergeSorter::Attributes
//----------------------------------------------------------------------------
protected:
   unsigned            mergeCount;  // Number of elements
   Object**            mergeArray;  // Temporary array
}; // class MergeSorter

#endif // MERGESORTER_H_INCLUDED
