//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       timing.cpp
//
// Purpose-
//       Timing test.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/timeb.h>

#include <com/syslib.h>

#ifdef _OS_WIN
  #define sleep(x) _sleep(x*1000)
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define LOOP_COUNT     3200000000UL // Number of iterations
#define ARRAY_SIZE          4000000 // Size of data array (in words)

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
typedef void           (PROGRAM)(void); // A timing program

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
static unsigned long*  array;       // Allocated data array

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   array= (unsigned long*)malloc(ARRAY_SIZE*sizeof(unsigned long));
   assert( array != NULL );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   free(array);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tod
//
// Purpose-
//       Return the current time of day.
//
//----------------------------------------------------------------------------
static double                       // The time of day
   tod( void )                      // Get the the current time
{
   double              result;      // Resultant time
   struct timeb        ticker;      // UTC time base

   ftime(&ticker);                  // UTC (since epoch)
   result  = (double)ticker.time;
   result += (double)ticker.millitm / 1000.0;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeCheck
//
// Purpose-
//       Run a timing test.
//
//----------------------------------------------------------------------------
static void
   timeCheck( void )                // Check - 10 second sleep
{
   sleep(10);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeGranularity
//
// Purpose-
//       Run a timing test.
//
//----------------------------------------------------------------------------
static void
   timeGranularity( void )          // Granularity test - does nothing
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeInstructionLoop
//
// Purpose-
//       Run a simple timing loop.
//
//----------------------------------------------------------------------------
static void
   timeInstructionLoop( void )      // Simple loop, no array references
{
   unsigned long       count;       // Iteration number
   unsigned long       index;       // Array index

   index= 0;
   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     index++;
   }

   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     index++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeMemoryLoop
//
// Purpose-
//       Run a memory access timing loop.
//
//----------------------------------------------------------------------------
static void
   timeMemoryLoop( void )           // Memory access loop
{
   unsigned long       count;       // Iteration number
   unsigned long       index;       // Array index

   index= 0;
   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     array[index]= index;
     index++;
   }

   index= 0;
   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     if( array[index] != index )
     {
       fprintf(stderr, "%4d: Memory fault detected [%lu]=%lu\n",
                       __LINE__, index, array[index]);
       exit(EXIT_FAILURE);
     }
     index++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timeSimpleLoop
//
// Purpose-
//       Run a simple timing loop.
//
//----------------------------------------------------------------------------
static void
   timeSimpleLoop( void )           // Simple loop
{
   unsigned long       count;       // Iteration number
   unsigned long       index;       // Array index

   index= 0;
   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     array[0]= 0;
     index++;
   }

   for(count= 0; count<LOOP_COUNT; count++)
   {
     if( index >= ARRAY_SIZE )
       index= 0;

     if( array[0] != 0 )
     {
       fprintf(stderr, "%4d: Memory fault detected [%lu]=%lu\n",
                       __LINE__, 0UL, array[0]);
       exit(EXIT_FAILURE);
     }
     index++;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       timing
//
// Purpose-
//       Run a timing test.
//
//----------------------------------------------------------------------------
static void
   timing(                          // Run a timing test
     const char*       name,        // Test name
     PROGRAM*          code)        // The test
{
   double              finish;      // Ending time
   double              start;       // Starting time

   start= tod();
   (*code)();
   finish= tod();

   printf("%s: %8.3f\n", name, (finish-start));
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
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   init();

   timing(" Granularity", &timeGranularity);
   timing(" Check 10sec", &timeCheck);
   timing(" Instruction", &timeInstructionLoop);
   timing("      Simple", &timeSimpleLoop);
   timing("      Memory", &timeMemoryLoop);

   term();
   return 0;
}

