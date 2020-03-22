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
//       sample.cpp
//
// Purpose-
//       Sample POSIX usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#define IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM_SIZE 10                 // GENERIC element count

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             swDebug;     // Debugging control

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "%s <options>\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-v\tVerify parameters\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               error;         // TRUE if error encountered
   int               verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               swDebug= TRUE;
               break;

             case 'h':              // -h (help)
               error= TRUE;
               break;

             case 'v':              // -v (verify)
               verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // Argument
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "%10d debug\n", swDebug);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testGETENV
//
// Purpose-
//       Test GETENV, PUTENV
//
//----------------------------------------------------------------------------
static void
   testGETENV( void )               // Test GETENV, PUTENV
{
   const char*         ptrOld;      // -> Old name
   const char*         ptrNew;      // -> New name

   ptrOld= getenv("JUNK");
   if( ptrOld != NULL )
     printf("On entry, JUNK='%s'\n", ptrOld);

   putenv((char*)"JUNK=foo.bar");
   ptrOld= getenv("JUNK");
   putenv((char*)"JUNK=bar.foo");
   ptrNew= getenv("JUNK");

   assert( strcmp("foo.bar", ptrOld) == 0 );
   assert( strcmp("bar.foo", ptrNew) == 0 );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testRANDOM
//
// Purpose-
//       Test RAND, SRAND
//
//----------------------------------------------------------------------------
static void
   testRANDOM( void )               // Test RAND, SRAND
{
   int                 R;           // A random number
   int                 olds[DIM_SIZE]; // A set of random numbers

   int                 i;

   srand(time(NULL));               // Randomize

   R= rand();                       // Get the initial seed
   srand(R);                        // (NOT AN OPTION!)
   for(i= 0; i<DIM_SIZE; i++)
   {
     olds[i]= rand();
   }

   srand(R);
// printf("%.8x seed\n", R);
// printf("-----old -----new\n");
   for(i= 0; i<DIM_SIZE; i++)
   {
     assert( olds[i] == rand() );
//   printf("%.8x %.8x\n", olds[i], rand());
   }
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Run the test suite
   //-------------------------------------------------------------------------
   testGETENV();
   testRANDOM();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   printf("No errrors\n");
   return 0;
}

