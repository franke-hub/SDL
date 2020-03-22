//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_SMS.cpp
//
// Purpose-
//       Test storage management subsystem.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/params.h>
#include <com/syslib.h>

#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Random.h>

#include "Test_SMS.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_SMS" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef STATS                       // Statistics, active iff defined
#undef  STATS                       // Default STATS
#endif

#include <com/ifmacro.h>

#undef  IFSTATS
#ifdef  STATS
#define IFSTATS(x) {x}
#else
#define IFSTATS(x) {}
#endif

//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
typedef long*        Address;       // Address type

//----------------------------------------------------------------------------
// Slot descriptor
//----------------------------------------------------------------------------
struct Slot                         // Slot descriptor
{
   unsigned long     subpool;       // Subpool index
   unsigned long     length;        // Allocated element length
   Address           address;       // Allocated element address
}; // struct Slot

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator
extern int             hcdm;        /* Hard Core Debug Mode control         */

//----------------------------------------------------------------------------
// Internal work areas
//----------------------------------------------------------------------------
   static unsigned long
                     mainloop;      // System test loop count
   static unsigned long
                     test_debug;    // Debug stop value
   static unsigned long
                     test_limit;    // Testcase loop limit
   static unsigned long
                     test_princ;    // Print increment value
   static unsigned long
                     test_print;    // Print when mainloop = this

   static unsigned long
                     error_count;   // Error summary counter
   static unsigned long
                     error_total;   // Error total count

//----------------------------------------------------------------------------
// General work areas
//----------------------------------------------------------------------------
   static Interval   interval;      // Interval timer
   static Slot*      sysslot;       // -> The slot table
   static Test_SMS   testObj;       // The test object

   static unsigned long
                     maxSlots;      // Number of allocation elements
   static unsigned long
                     maxSlot2;      // maxSlots / 2
   static unsigned long
                     maxAlloc;      // Maximum allocation length
   static unsigned long
                     minAlloc;      // Minimum allocation length
   static unsigned long
                     maxSubpool;    // Number of storage pools

   static unsigned long
                     init_debug;    // Debug stop value
   static unsigned long
                     init_limit;    // Testcase loop limit
   static unsigned long
                     init_print;    // Testcase print interval

   static unsigned char
                     sw_verify;     // Verify storage?

//----------------------------------------------------------------------------
// Statistics
//----------------------------------------------------------------------------
#ifdef STATS
static unsigned long stat_curAlloc; // Current number of bytes allocated
static unsigned long stat_curSlots; // Current number of slots allocated
static unsigned long stat_maxAlloc; // Largest number of bytes allocated
static unsigned long stat_maxSlots; // Largest number of slots allocated
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       diagnostics
//
// Function-
//       Display the state of the Slot allocation table
//
//----------------------------------------------------------------------------
static void
   diagnostics( void )              // Slot allocation table display
{
   unsigned int      pool;          // The subpool index
   long              size;          // The request length
   void*             addr;          // The element address
   long              tlength;       // Total allocated storage length

   int               i, j;          // General index variable

   //-------------------------------------------------------------------------
   // Display the Slot allocation table
   //-------------------------------------------------------------------------
   for( i=0; i<maxSlot2; i++ )      // Display the slot table
   {
     pool= sysslot[i].subpool;      // Get the subpool index
     size= sysslot[i].length;       // Get the element length
     addr= sysslot[i].address;      // Get the element address

     tracef("[%5d] SP=%3d, L=%6ld, A=%p    ", i, pool, size, addr);

     j= i+maxSlot2;
     pool= sysslot[j].subpool;      // Get the subpool index
     size= sysslot[j].length;       // Get the element length
     addr= sysslot[j].address;      // Get the element address

     tracef("[%5d] SP=%3d, L=%6ld, A=%p\n", j, pool, size, addr);
   }

   tlength= 0;                      // Initialize for summation
   for( i=0; i<maxSlots; i++ )      // Sum allocated storage
   {
     if( sysslot[i].address != NULL ) // If the slot is allocated
       tlength += sysslot[i].length;// Account for it
   }

   tracef("Total current allocation: %lu bytes\n", tlength);
   debugFlush();

   testObj.debug();                 // Error debug
   debugFlush();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstart
//
// Function-
//       Testcase checkpoint initialization.
//
//----------------------------------------------------------------------------
static void
   checkstart(                      // Testcase initialize checkpoint
     uint32_t        iterations,    // Number of iterations required
     uint32_t        interval,      // Printing interval value
     uint32_t        debugstop)     // Debugging mainloop value
{
   //-------------------------------------------------------------------------
   // Initialize error counters
   //-------------------------------------------------------------------------
   error_total= 0;                  // No errors encountered
   error_count= 0;                  // No errors encountered

   //-------------------------------------------------------------------------
   // Initialize loop controls
   //-------------------------------------------------------------------------
   mainloop= 0;                     // MAIN loop counter
   test_limit= iterations;          // Number of iterations
   test_print= interval;            // Set first print value
   test_princ= interval;            // Set print interval
   test_debug= debugstop;           // Set debugging stop

   if( test_print > test_limit )    // If first print point is out of range
     test_print= test_limit;        // Print on last iteration
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkpoint
//
// Function-
//       Testcase checkpoint analysis.
//
//----------------------------------------------------------------------------
static int                          // TRUE if iteration count exceeded
   checkpoint( void )               // Testcase checkpoint analysis
{
   //-------------------------------------------------------------------------
   // Breakpoint analysis
   //-------------------------------------------------------------------------
   if( mainloop == test_debug )     // If diagnostics required
   {
     diagnostics();                 // Diagnostics
     hcdm= TRUE;                    // Hard Core Debug Mode
// abort();                       // and abort
   }

   //-------------------------------------------------------------------------
   // Checkpoint analysis
   //-------------------------------------------------------------------------
   if( mainloop == test_print )     // If checkprint required
   {
     debugf("%s %10lu of %10lu", __SOURCE__, // Print checkprint info
            mainloop, test_limit);

     if( error_count != 0 )         // If exception(s) encountered
       debugf(", %10lu of %10lu",
              error_count, test_princ);

     debugf("\n");

     error_total += error_count;    // Accumulate error counter
     error_count= 0;                // Reset for next interval
     test_print += test_princ;

     if( mainloop >= test_limit )   // If testcase ended
       return(TRUE);                // Exit, end of test

     if( test_print > test_limit )  // If next print point is out of range
       test_print= test_limit;      // Print on last iteration
   }

   //-------------------------------------------------------------------------
   // Checkpoint control - always returns FALSE
   //-------------------------------------------------------------------------
   mainloop++;                      // Increment MAINLOOP control
   return(FALSE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_begin
//
// Function-
//       Begin a test function.
//
//----------------------------------------------------------------------------
static void
   test_begin(                      // Begin a test
     const char*     string)        // Testcase name
{
   debugf("\n");
   debugf("%s %s started\n", __SOURCE__, string);  // Begin test

   IFSTATS(
     stat_curAlloc= 0;
     stat_curSlots= 0;
     stat_maxAlloc= 0;
     stat_maxSlots= 0;
   )

   interval.start();                // Start the interval
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_finis
//
// Function-
//       Complete a test.
//
//----------------------------------------------------------------------------
static void
   test_finis(                      // Complete a test
     const char*     string)        // Testcase name
{
   interval.stop();                 // Stop the interval
   debugf("%s Elapsed time: %8.4f seconds\n", __SOURCE__, interval.toDouble());

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   IFSTATS(
     debugf("%s %10lu Max allocated bytes\n", __SOURCE__, stat_maxAlloc);
     debugf("%s %10lu Max allocated slots\n", __SOURCE__, stat_maxSlots);
   )

   testObj.debug();                 // No problem debug

   debugf("%s %s complete\n", __SOURCE__, string); // End test
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocate
//
// Function-
//       Allocate an element.
//
//----------------------------------------------------------------------------
static void
   allocate(                        // Allocate an element
     unsigned int    ndxslot)       // The slot index
{
   unsigned int      pool;          // The request subpool
   unsigned int      size;          // The request length

   Address           addr;          // Allocated address
   int               wc;            // Word count

   int               i;

   pool= RNG.get()&0x7fffffff; // Set the subpool
   if( maxSubpool == 0 )            // If subpools are not supported
     pool &= 0x00FF;                // Limit value to 256 anyway

   size= RNG.get()%maxAlloc; // Limit the length
   size= max(size, minAlloc);       // Insure some request length

   sysslot[ndxslot].length=  size;  // Set the length

   //-------------------------------------------------------------------------
   // Allocate element storage
   //-------------------------------------------------------------------------
   if( maxSubpool > 0 )             // If subpools are supported
   {
     pool %= maxSubpool;            // Limit the subpool
     addr= (Address)testObj.allocate(size, pool); // Allocate storage
   }
   else                             // If subpools are not supported
     addr= (Address)testObj.allocate(size); // Allocate storage

   if( addr == NULL )               // If allocation failure
   {
     debugf("%s Mainloop: %ld, ", __SOURCE__, mainloop);
     debugf("Element allocation failure\n");
     debugf("Allocation request: %u bytes\n", size);

     diagnostics();                 // Drive diagnostics

     exit(EXIT_FAILURE);
   }

   sysslot[ndxslot].subpool= pool;  // Set the subpool
   sysslot[ndxslot].address= addr;  // Set the storage address

   //-------------------------------------------------------------------------
   // Debugging analysis
   //-------------------------------------------------------------------------
   IFHCDM(
     if( addr < (Address)0x00010000 ) // If invalid address
     {
       debugf("%s Mainloop: %ld, ", __SOURCE__, mainloop);
       debugf("Invalid allocation address\n");
       abort();
     }
   )

   IFSCDM(
     tracef("[0x%.8X] Allocate SP(%3d) ADDR(%p) LEN(%.4x)\n", ndxslot,
                      pool, addr, size);
   )

   //-------------------------------------------------------------------------
   // Initialize element storage
   //-------------------------------------------------------------------------
   if( sw_verify )
   {
     wc= trunc(size, sizeof(*addr))/sizeof(*addr); // Compute word count
     for( i=0; i<wc; i++ )          // Initialize storage
     {
       *addr= ndxslot;              // Initialize a storage word
       addr++;                      // Address the next word
     }
   }

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   IFSTATS(
     stat_curSlots++;
     if( stat_curSlots > stat_maxSlots )
       stat_maxSlots= stat_curSlots;

     stat_curAlloc += size;
     if( stat_curAlloc > stat_maxAlloc )
       stat_maxAlloc= stat_curAlloc;
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       release
//
// Function-
//       Release an element.
//
//----------------------------------------------------------------------------
static void
   release(                         // Release an element
     int             ndxslot)       // The slot index
{
   unsigned int      pool;          // The subpool index
   long              size;          // The request length

   Address           addr;          // Release address
   int               wc;            // Word count

   int               i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   pool= sysslot[ndxslot].subpool;  // Get the subpool index
   size= sysslot[ndxslot].length;   // Get the element length
   addr= sysslot[ndxslot].address;  // Set the storage address

   wc= trunc(size, sizeof(*addr))/sizeof(*addr);// Get word count

   //-------------------------------------------------------------------------
   // Debugging analysis
   //-------------------------------------------------------------------------
   IFSCDM(
     tracef("[0x%.8X] Release  SP(%3d) ADDR(%p) LEN(%.4x)\n", ndxslot,
                      pool, addr, size);
   )

   IFHCDM(
     if( addr < (Address)0x00010000 ) // If invalid release address
     {
       debugf("%s Mainloop: %ld, ", __SOURCE__, mainloop);
       debugf("Invalid release address(%p)\n", addr);
       exit(EXIT_FAILURE);
     }
   )

   //-------------------------------------------------------------------------
   // Verify element storage
   //-------------------------------------------------------------------------
   if( sw_verify )
   {
     for( i=0; i<wc; i++ )          // Validate storage
     {
       if( *(addr+i) != ndxslot )   // Validate a storage word
       {
         debugf("Mainloop: %ld, ", mainloop);
         debugf("Storage corrupted\n");
         debugf("[0x%.8X] *ERROR*  SP(%3d) ADDR(%p) offset(0x%.8X)\n"
                ">>>> Contains 0x%.8lX but should contain 0x%.8X\n",
                ndxslot, pool, addr, i*4,
                *addr, ndxslot);
         exit(EXIT_FAILURE);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Indicate storage freed
   //-------------------------------------------------------------------------
   for( i=0; i<wc; i++ )            // Validate storage
     *(addr+i)= C2L(">>>>FREE");    // Indicate storage released

   //-------------------------------------------------------------------------
   // Release element storage
   //-------------------------------------------------------------------------
   if( maxSubpool > 0 )             // If subpools are supported
     testObj.release(addr, size, pool); // Release storage
   else                             // If subpools are not supported
     testObj.release(addr, size);// Release storage

   sysslot[ndxslot].address= NULL;  // Indicate storage released

   //-------------------------------------------------------------------------
   // Statistics
   //-------------------------------------------------------------------------
   IFSTATS(
     stat_curSlots--;
     stat_curAlloc -= size;
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       releaseAllStorage
//
// Function-
//       Release all allocated Slots
//
//----------------------------------------------------------------------------
static void
   releaseAllStorage( void )        // Release all allocated slots
{
   int               i;             // General index variable

   //-------------------------------------------------------------------------
   // Release all allocated slots
   //-------------------------------------------------------------------------
   for( i=0; i<maxSlots; i++ )      // Release all storage
   {
     if( sysslot[i].address != NULL ) // If the Slot is allocated
       release(i);                  // Release the element
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       releaseSubpool
//
// Function-
//       Release one subpool.
//
//----------------------------------------------------------------------------
static void
   releaseSubpool(                  // Release all allocated slots
     unsigned int    pool)          // The subpool index
{
   int               i;             // General index variable

   //-------------------------------------------------------------------------
   // Release the subpool
   //-------------------------------------------------------------------------
   testObj.release(pool);           // Release the subpool

   for( i=0; i<maxSlots; i++ )      // Account for release
   {
     if( sysslot[i].address != NULL // If the Slot was allocated
         &&sysslot[i].subpool == pool ) // to this pool
     {
       sysslot[i].address= NULL;

       //---------------------------------------------------------------------
       // Statistics
       //---------------------------------------------------------------------
       IFSTATS(
         stat_curSlots--;
         stat_curAlloc -= sysslot[i].length;
       )
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_001
//
// Function-
//       Random allocation/release test
//
//----------------------------------------------------------------------------
static void
   test_001(                        // Random allocation/release
     int32_t         limit_1,       // Iteration count
     int32_t         print_1,       // Checkpoint count
     int32_t         debug_1)       // Debugging count
{
   unsigned int      ndxslot;       // The slot index

   //-------------------------------------------------------------------------
   // Random allocation/release test
   //-------------------------------------------------------------------------
   checkstart(limit_1,              // Initialize checkpoint controls
              print_1,
              debug_1);

   while( !checkpoint() )           // Random allocation/release
   {
     ndxslot= RNG.get()%maxSlots; // Select a slot
     if( sysslot[ndxslot].address == NULL ) // If not allocated
       allocate(ndxslot);           // Allocate
     else                           // If already allocated
       release(ndxslot);            // Release

     if( (RNG.get()%50000) == 0 ) // Every now and then
     {
       if( maxSubpool > 0 )         // If subpools are supported
         releaseSubpool(RNG.get()%maxSubpool); // Release a subpool
       else                         // If subpools are not supported
         releaseAllStorage();       // Release all allocated slots
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testcase
//
// Function-
//       Testcase driver.
//
//----------------------------------------------------------------------------
static void
   testcase(                        // Test driver
     int32_t         limit_1,       // Number of test iterations
     int32_t         print_1,       // Iterations between prints
     int32_t         debug_1)       // Debugging stop iteration
{
   unsigned long     freeInitial;   // Initial free storage size
   unsigned long     freeFinal;     // Final   free storage size

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   freeInitial= testObj.getUsed();  // Determine free storage size

   //-------------------------------------------------------------------------
   // Allocation/release random function test
   //-------------------------------------------------------------------------
   test_begin("Random allocation/release");// Set up for test
   test_001(limit_1, print_1, debug_1);// Perform test
   test_finis("Random allocation/release");// Complete the test

   //-------------------------------------------------------------------------
   // Look for storage leaks
   //-------------------------------------------------------------------------
   releaseAllStorage();             // Release all Slots
   freeFinal= testObj.getUsed();    // Determine free storage size

   if( freeInitial != freeFinal )   // If storage was lost or gained
   {
     debugf("%s Available storage size changed.\n", __SOURCE__);
     debugf("Initial size: 0x%.8lX, final size: 0x%.8lX\n",
            freeInitial, freeFinal);
     diagnostics();                 // Drive diagnostics
   }
}

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
   size_t            size;          // Allocation size

   //-------------------------------------------------------------------------
   // Initialization started
   //-------------------------------------------------------------------------
   debugf("%s Initialization started\n", __SOURCE__);

   #if defined(HCDM) || defined(SCDM)
     debugSetIntensiveMode();
   #endif

   //-------------------------------------------------------------------------
   // Allocate the Slot table
   //-------------------------------------------------------------------------
   size= maxSlots*sizeof(Slot);
   sysslot= (Slot*)malloc(size);    // Allocate the slot table
   if( sysslot == NULL )            // If allocation failure
   {
     debugf("%s %d: No storage(%ld)\n", __SOURCE__, __LINE__, (long)size);
     exit(EXIT_FAILURE);
   }
   memset(sysslot, 0, size);        // Clear the slot table

   //-------------------------------------------------------------------------
   // Initialize storage management
   //-------------------------------------------------------------------------
   if( minAlloc < testObj.getMinSize() )
     minAlloc= testObj.getMinSize();

   if( maxAlloc > testObj.getMaxSize() )
     maxAlloc= testObj.getMaxSize();

   maxSubpool= testObj.getSubpools();

   //-------------------------------------------------------------------------
   // Initialization complete
   //-------------------------------------------------------------------------
   debugf("%s Initialization complete\n", __SOURCE__);
   debugf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static volatile void                // (Does not return)
   info( void )                     // Informational exit
{
   fprintf(stderr, "Parameters:\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-debug:\n");
   fprintf(stderr, "  Debugging abort point.\n");
   fprintf(stderr, "-limit:\n");
   fprintf(stderr, "  Number of iterations.\n");
   fprintf(stderr, "-print:\n");
   fprintf(stderr, "  Number of iterations between prints.\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-mem:\n");
   fprintf(stderr, "  Number of megabytes to allocate.\n");
   fprintf(stderr, "-minSize:\n");
   fprintf(stderr, "  Minimum element allocation size.\n");
   fprintf(stderr, "-maxSize:\n");
   fprintf(stderr, "  Maximum element allocation size.\n");

   fprintf(stderr, "\n");
   fprintf(stderr, "-verify-\n");
   fprintf(stderr, "  Do not verify storage.\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               argi;          // Argument index
   char*             argp;          // Argument pointer

   int               error;         // Error encountered indicator

   unsigned long     maxTotal;      // Total storage size

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error=  FALSE;                   // Default, no error
   sw_verify= TRUE;                 // Default, verification

   init_debug= 0;
   init_limit= 0;
   init_print= 0;

   maxTotal=   8;
   minAlloc=   1;
   maxAlloc=   4096;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if (argc > 1 && *argv[1] == '?') // If query request
   {
     info();                        // Display options
     exit(EXIT_FAILURE);            // And exit, function complete
   }

   for (argi=1; argi<argc; argi++)  // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if (*argp == '-')              // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if (swname("verify", argp))
         sw_verify= swatob("verify", argp);

       else if (swname("debug:", argp))
         init_debug= swatol("debug:", argp);

       else if (swname("limit:", argp))
         init_limit= swatol("limit:", argp);

       else if (swname("print:", argp))
         init_print= swatol("print:", argp);

       else if (swname("mem:", argp))
         maxTotal= swatol("mem:", argp);

       else if (swname("maxSize:", argp))
         maxAlloc= swatol("maxSize:", argp);

       else if (swname("minSize:", argp))
         minAlloc= swatol("minSize:", argp);

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If not a switch parameter
     {
       {
         error= TRUE;
         fprintf(stderr, "Unknown parameter: '%s'\n", argp);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();

   if( init_limit == 0 )
     init_limit= 100000;

   if( init_debug == 0 )
     init_debug= init_limit+1;

   if( init_print == 0 )
     init_print= init_limit / 10;

   if( maxTotal == 0 )
     maxTotal= 1;

   if( minAlloc == 0 )
     minAlloc= 1;

   if( maxAlloc == 0 )
     maxAlloc= 4096;

   maxSlots= maxTotal * 1024 * 1024;
   maxSlots /= (minAlloc + maxAlloc) / 4;

   if( (maxSlots & 1) != 0 )
     maxSlots++;

   if( maxSlots < 16 )
     maxSlots= 16;

   maxSlot2= maxSlots / 2;

   debugf("  %10s -verify\n",    sw_verify ? "TRUE" : "FALSE" );
   debugf("  %10lu -debug:\n",   init_debug );
   debugf("  %10lu -limit:\n",   init_limit );
   debugf("  %10lu -print:\n",   init_print );
   debugf("  %10lu -mem:\n",     maxTotal );
   debugf("  %10lu -minSize:\n", minAlloc );
   debugf("  %10lu -maxSize:\n", maxAlloc );
   debugf("  %10lu slots\n",     maxSlots);
   debugf("\n\n");
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
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);
   hcdm= FALSE;

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   init();

   //-------------------------------------------------------------------------
   // Test
   //-------------------------------------------------------------------------
   testcase(init_limit, init_print, init_debug);

   return 0;
}

