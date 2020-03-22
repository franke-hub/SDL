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
//       QuickSorter.h
//
// Purpose-
//       Quick sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef QUICKSORTER_H_INCLUDED
#define QUICKSORTER_H_INCLUDED

#ifndef SORTER_H_INCLUDED
#include "Sorter.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       QuickSorter
//
// Purpose-
//       Quick sorter.
//
//----------------------------------------------------------------------------
class QuickSorter : public Sorter { // Quick sorter
//----------------------------------------------------------------------------
// QuickSorter::Constructors
//----------------------------------------------------------------------------
public:
   ~QuickSorter( void );            // Destructor
   QuickSorter( void );             // Constructor

//----------------------------------------------------------------------------
// QuickSorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const;      // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array);      // Object array

//----------------------------------------------------------------------------
// QuickSorter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class QuickSorter

#endif // QUICKSORTER_H_INCLUDED
