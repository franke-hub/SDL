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
//       QuickSorter.cpp
//
// Purpose-
//       Quick sorter.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       10000 Timing: 5.21 (#1) A stack intensive, but quick sorter.
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "QuickSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "QUICK   " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       QuickSorter::~QuickSorter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   QuickSorter::~QuickSorter( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       QuickSorter::QuickSorter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   QuickSorter::QuickSorter( void )   // Constructor
:  Sorter()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       QuickSorter::getClassName
//
// Purpose-
//       Get the class name.
//
//----------------------------------------------------------------------------
const char*                         // The class name
   QuickSorter::getClassName( void ) const
{
   return "QuickSorter";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sortDescriptor
//
// Purpose-
//       Quick sort the data descriptor.
//
//----------------------------------------------------------------------------
static void
   sortDescriptor(                  // Sort the data
     int               inpBot,      // Leftmost index
     int               inpTop,      // Rightmost index
     Object**          array)       // Object array
{
   int                 bot= inpBot; // Working left index
   int                 top= inpTop; // Working right index
   Object*             pivot= array[inpBot]; // Pivot object

   while( bot < top )
   {
     while( bot < top && array[top]->compare(pivot) >= 0 )
       top--;
     if( bot != top )
     {
       array[bot]= array[top];
       bot++;
     }

     while( bot < top && array[bot]->compare(pivot) <= 0 )
       bot++;
     if( bot != top )
     {
       array[top]= array[bot];
       top--;
     }
   }

   array[bot]= pivot;
   if( inpBot < bot )
     sortDescriptor(inpBot, bot-1, array);

   if( inpTop > bot )
     sortDescriptor(bot+1, inpTop, array);
}

//----------------------------------------------------------------------------
//
// Method-
//       QuickSorter::sort
//
// Purpose-
//       Sort an array.
//
//----------------------------------------------------------------------------
void
   QuickSorter::sort(               // Sort an Object array
     unsigned          count,       // Number of elements
     Object**          array)       // Object array
{
   sortDescriptor(0, count-1, array);
}

