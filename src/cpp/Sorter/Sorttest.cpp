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
//       Sorttest.cpp
//
// Purpose-
//       Sort tester.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <com/Random.h>

#include "Object.h"
#include "BubbleSorter.h"
#include "HeapSorter.h"
#include "MergeSorter.h"
#include "QuickSorter.h"
#include "ShellSorter.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SORTTEST" // Source file

#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#define MAX_COUNT              2048 // Largest sort size

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static unsigned        count;       // Number of Objects in array
static Object**        array;       // (Sorted) Object array
static Object**        picks;       // (Sorted) Object array
static Object**        unsorted;    // Unsorted Object array

//----------------------------------------------------------------------------
//
// Subroutine-
//       createArray
//
// Purpose-
//       Create an Object array.
//
//----------------------------------------------------------------------------
static void
   createArray( void )              // Create an Object array
{
   unsigned            i;

   array= new Object*[count];
   picks= new Object*[count];
   unsorted= new Object*[count];

   for(i= 0; i<count; i++)
     array[i]= new Object(i);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       deleteArray
//
// Purpose-
//       Delete an Object array.
//
//----------------------------------------------------------------------------
static void
   deleteArray( void )              // Create an Object array
{
   unsigned            i;

   for(i= 0; i<count; i++)
     delete array[i];

   delete array;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       randomize
//
// Purpose-
//       Copy the Object to the unsorted array
//
//----------------------------------------------------------------------------
static void
   randomize(                       // Unsort the array
     int               count)       // Unsort counter
{
   for(int i= 0; i<count; i++)
     picks[i]= array[i];

   for(int i= 0; i<count; i++)
   {
     int x= RNG.get() % count;
     while( picks[x] == NULL )
     {
       x++;
       if( x == count )
         x= 0;
     }

     unsorted[i]= picks[x];
     picks[x]= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify
//
// Purpose-
//       Sort and test the resultant
//
//----------------------------------------------------------------------------
static int                          // Error counter
   verify(                          // Sort and test.
     int               count,       // Working count
     Sorter*           sorter)      // Sorter
{
   randomize(count);
   sorter->sort(count, unsorted);
   for(int i= 0; i<count; i++)
   {
     if( array[i] != unsorted[i] )
     {
       fprintf(stderr, "Sort(%s) error\n", sorter->getClassName());
       return 1;
     }
   }

   #ifdef HCDM
//   printf("OK Sort(%s) %d\n", sorter->getClassName(), count);
   #endif
   return 0;
}

static int                          // Error counter
   verify(                          // Sort and test.
     Sorter*           sorter)      // Sorter
{
   time_t              start;       // Test start time
   time_t              finish;      // Test completion time

   int                 i;

   start= time(NULL);
   for(i= count; i>=0; i--)
   {
     memset(unsorted, 0, count*sizeof(Object*));
     if( verify(i, sorter) > 0 )
     {
       fprintf(stderr, "Sort(%s) error\n", sorter->getClassName());
       return 1;
     }
   }
   finish= time(NULL);

   #ifdef HCDM
     printf("OK Sort(%s) %d seconds\n",
            sorter->getClassName(), (int)(finish-start));
   #endif
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               errorCount= 0; // Error counter
   Sorter*           sorter;        // The current Sorter
   const char*       test;          // Specific test

   // Initialize
   count= MAX_COUNT;
   test= NULL;
   if( argc > 1 )
     count= atol(argv[1]);
   if( argc > 2 )
     test= argv[2];

   createArray();

   // Test BubbleSorter
   sorter= new BubbleSorter();
   if( test == NULL || strcmp(test, "bubble") == 0 )
     errorCount += verify(sorter);
   delete sorter;

   // Test HeapSorter
   sorter= new HeapSorter();
   if( test == NULL || strcmp(test, "heap") == 0 )
     errorCount += verify(sorter);
   delete sorter;

   // Test MergeSorter
   sorter= new MergeSorter();
   if( test == NULL || strcmp(test, "merge") == 0 )
     errorCount += verify(sorter);
   delete sorter;

   // Test QuickSorter
   sorter= new QuickSorter();
   if( test == NULL || strcmp(test, "quick") == 0 )
     errorCount += verify(sorter);
   delete sorter;

   // Test ShellSorter
   sorter= new ShellSorter();
   if( test == NULL || strcmp(test, "shell") == 0 )
     errorCount += verify(sorter);
   delete sorter;

   // Terminate
   deleteArray();

   printf("Errorcount: %d\n", errorCount);
   return errorCount;
}

