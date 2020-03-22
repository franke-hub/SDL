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
//       BubbleSorter.h
//
// Purpose-
//       Bubble sort object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef BUBBLESORTER_H_INCLUDED
#define BUBBLESORTER_H_INCLUDED

#ifndef SORTER_H_INCLUDED
#include "Sorter.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       BubbleSorter
//
// Purpose-
//       Bubble sorter.
//
//----------------------------------------------------------------------------
class BubbleSorter : public Sorter {// Bubble sorter
//----------------------------------------------------------------------------
// BubbleSorter::Constructors
//----------------------------------------------------------------------------
public:
   ~BubbleSorter( void );           // Destructor
   BubbleSorter( void );            // Constructor

//----------------------------------------------------------------------------
// BubbleSorter::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // The class name
   getClassName( void ) const;      // Get class name

virtual void
   sort(                            // Sort the objects
     unsigned          count,       // Number of elements
     Object**          array);      // Object array

//----------------------------------------------------------------------------
// BubbleSorter::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class BubbleSorter

#endif // BUBBLESORTER_H_INCLUDED
