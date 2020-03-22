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
//       MergeSorter.cpp
//
// Purpose-
//       Merge sorter.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       An extremely efficient sorter that uses a temporary array.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "MergeSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "MERGE   " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       MergeSorter::~MergeSorter
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   MergeSorter::~MergeSorter( void ) // Destructor
{
   if( mergeArray != NULL )
     free(mergeArray);

   mergeArray= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       MergeSorter::MergeSorter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   MergeSorter::MergeSorter( void )   // Constructor
:  Sorter()
,  mergeCount(0)
,  mergeArray(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       MergeSorter::getClassName
//
// Purpose-
//       Get the class name.
//
//----------------------------------------------------------------------------
const char*                         // The class name
   MergeSorter::getClassName( void ) const
{
   return "MergeSorter";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       mergeArray
//
// Purpose-
//       Merge the data descriptor.
//
//----------------------------------------------------------------------------
static void
   mergeArray(                      // Sort the data
     int               bot,         // Leftmost index
     int               mid,         // Midpoint index
     int               top,         // Rightmost index
     Object**          array,       // Object array
     Object**          merge)       // Object merge array
{
   int                 const count= (top-bot) + 1; // Number of elements
   int                 mergeIndex;  // Working merge index
   int                 left;        // Working left index

   int                 i;

   left= mid - 1;
   mergeIndex= bot;
   while( bot <= left && mid <= top )
   {
     if( array[bot]->compare(array[mid]) <= 0 )
     {
       merge[mergeIndex]= array[bot];
       mergeIndex++;
       bot++;
     }
     else
     {
       merge[mergeIndex]= array[mid];
       mergeIndex++;
       mid++;
     }
   }

   while( bot <= left )
   {
     merge[mergeIndex]= array[bot];
     bot++;
     mergeIndex++;
   }

   while( mid <= top )
   {
     merge[mergeIndex]= array[mid];
     mid++;
     mergeIndex++;
   }

   for(i=0; i<=count; i++)
   {
     array[top]= merge[top];
     top--;
   }}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sortArray
//
// Purpose-
//       Merge sort the data descriptor.
//
//----------------------------------------------------------------------------
static void
   sortArray(                       // Sort the data
     int               bot,         // Leftmost index
     int               top,         // Rightmost index
     Object**          array,       // Object array
     Object**          merge)       // Object merge array
{
   int                 mid;         // Midpoint index

   if( top > bot )
   {
     mid= (bot+top)/2;
     sortArray(bot, mid, array, merge);
     sortArray(mid+1, top, array, merge);
     mergeArray(bot, mid+1, top, array, merge);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       MergeSorter::sort
//
// Purpose-
//       Sort an array.
//
//----------------------------------------------------------------------------
void
   MergeSorter::sort(               // Sort an Object array
     unsigned          count,       // Number of elements
     Object**          array)       // Object array
{
   if( mergeArray == NULL || count > mergeCount )
   {
     if( mergeArray != NULL )
       free(mergeArray);

     mergeArray= (Object**)malloc(count * sizeof(Object*));
     mergeCount= count;
     assert( mergeArray != NULL );
   }

   sortArray(0, count-1, array, mergeArray);
}

