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
//       BubbleSorter.cpp
//
// Purpose-
//       Bubble sorter.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       10000 Timing: 622.09 (#5) CPU intensive sorter, simple but slow.
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "BubbleSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "BUBBLE  " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       BubbleSorter::~BubbleSorter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   BubbleSorter::~BubbleSorter( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       BubbleSorter::BubbleSorter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   BubbleSorter::BubbleSorter( void ) // Constructor
:  Sorter()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       BubbleSorter::getClassName
//
// Purpose-
//       Get the class name.
//
//----------------------------------------------------------------------------
const char*                         // The class name
   BubbleSorter::getClassName( void ) const
{
   return "BubbleSorter";
}

//----------------------------------------------------------------------------
//
// Method-
//       BubbleSorter::sort
//
// Purpose-
//       Sort an array.
//
//----------------------------------------------------------------------------
void
   BubbleSorter::sort(              // Sort an Object array
     unsigned          count,       // Number of elements
     Object**          array)       // Object array
{
   unsigned            i, j;
   Object*             temp;

   for(i= 0; i<count; i++)
   {
     for(j= i+1; j<count; j++)
     {
       if( array[i]->compare(array[j]) > 0 )
       {
         temp= array[i];
         array[i]= array[j];
         array[j]= temp;
       }
     }
   }
}

