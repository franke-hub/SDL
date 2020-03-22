//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Mem.cpp
//
// Purpose-
//       Memory test.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <com/params.h>
#include <com/syslib.h>
#include <com/Unconditional.h>

#include "Test_Mem.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_MEM" // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
extern unsigned        quiet;       // Test quietly

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static unsigned*       memAddr;     // Memory address
static size_t          memSize;     // Memory length
static unsigned        repeats;     // Number of times to repeat test

static unsigned*       genAddr;     // Allocated address
static size_t          genSize;     // Allocated size

//----------------------------------------------------------------------------
//
// Subroutine-
//       roundf
//
// Purpose-
//       Round a value upwards.
//
//----------------------------------------------------------------------------
static inline uintptr_t             // Rounded resultant
   roundf(                          // Round (upwards)
     uintptr_t         value,       // Original value
     long              factor)      // Rounding factor (power of 2)
{
   uintptr_t           resultant;

   resultant= value + factor - 1;   // Round upwards
   resultant &= ~(factor-1);        // Truncate downward
   return resultant;                // Return resulatant
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
static void
   info( void )                     // Informational exit
{
   fprintf(stderr, "memtest <Controls> <Size>\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Controls\n");
   fprintf(stderr, "  -verify\n");
   fprintf(stderr, "    Verify parameters\n");
   fprintf(stderr, "  -quiet\n");
   fprintf(stderr, "    Test quietly\n");
   fprintf(stderr, "  -repeat:\n");
   fprintf(stderr, "    Number of times to repeat test\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Size\n");
   fprintf(stderr, "  Number of bytes\n");
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
   parm(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 parmX;       // Positional parameter index
   int                 error;       // Error encountered indicator
   int                 verify;      // Verification control
// int                 c;

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no errors found
   verify= 0;                       // Default, no verification
   parmX= 0;                        // Initial positional parameter index

   memSize= 0x40000000;             // Storage allocation size (1G)
   repeats= 1;                      // Number of times to repeat test

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if( swname("verify", argp) ) // If verify switch
         verify= swatob("verify", argp); // Get switch value

       else if( swname("quiet", argp) ) // If quiet switch
         quiet= swatob("quiet", argp); // Get switch value

       else if( swname("repeat:", argp) ) // If repeat switch
         repeats= swatol("repeat:", argp); // Get switch value

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If positional parameter
     {
       switch(parmX)                // Process positional parameter
       {
         case 0:
           parmX++;
           memSize= atox(argp);
           break;

         default:
           error= TRUE;
           fprintf(stderr, "Invalid positional parmeter: '%s'\n", argp);
           break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();

   if( verify )                     // If verification required
   {
     fprintf(stderr,    " %10s quiet\n", quiet ? "TRUE" : "FALSE");
     fprintf(stderr, " 0x%.8zx memSize\n", memSize);
     fprintf(stderr,    " %10d repeats\n", repeats);
   }

   //-------------------------------------------------------------------------
   // Allocate memory
   //-------------------------------------------------------------------------
   genSize= memSize + 4096;         // Set allocation size
   genAddr= (unsigned*)must_malloc(genSize);   // Allocate storage
   memAddr= (unsigned*)roundf((uintptr_t)genAddr, 4096);
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   unsigned            count;

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   parm(argc, argv);
   printf("%s %d: Addr(%p:%p) Size(%zu)\n", __SOURCE__, __LINE__,
          memAddr, ((char*)memAddr)+memSize-1, memSize);

   //-------------------------------------------------------------------------
   // Test a storage range
   //-------------------------------------------------------------------------
   for(count=1; count<=repeats; count++)
   {
     memtest0(memAddr, memSize);    // Test memory
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return 0;
}

