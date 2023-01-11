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
//       HeapSorter.cpp
//
// Purpose-
//       Heap sorter.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       10000 Timing: 7.15 (#3) Good for large arrays.
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "HeapSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "HEAP    " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       HeapSorter::~HeapSorter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HeapSorter::~HeapSorter( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HeapSorter::HeapSorter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HeapSorter::HeapSorter( void )   // Constructor
:  Sorter()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HeapSorter::getClassName
//
// Purpose-
//       Get the class name.
//
//----------------------------------------------------------------------------
const char*                         // The class name
   HeapSorter::getClassName( void ) const
{
   return "HeapSorter";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       subSort
//
// Purpose-
//       Move an item to its proper place in the heap array.
//
//----------------------------------------------------------------------------
static void
   subSort(                         // Move an item to its proper place
     int               parent,      // Item to move
     int               size,        // Size (topmost item + 1)
     Object**          heap)        // The heap to sort
{
   Object*             item= heap[parent];
   int                 child;

   while( (child= (parent + parent) + 1) < size )
   {
     if( child + 1 < size && heap[child]->compare(heap[child + 1]) < 0 )
       child++;

     if( item->compare(heap[child]) >= 0 )
       break;

     heap[parent]= heap[child];
     parent= child;
   }

   heap[parent]= item;
}

//----------------------------------------------------------------------------
//
// Method-
//       HeapSorter::sort
//
// Purpose-
//       Sort an array.
//
//----------------------------------------------------------------------------
void
   HeapSorter::sort(                // Sort an Object array
     unsigned          count,       // Number of elements
     Object**          array)       // Object array
{
   Object*             temp;

   int                 i;

   for(i= (count / 2)-1; i >= 0; i--)
     subSort(i, count, array);

   for(i= count-1; i >= 1; i--)
   {
     temp = array[0];
     array[0] = array[i];
     array[i] = temp;
     subSort(0, i, array);
   }
}

