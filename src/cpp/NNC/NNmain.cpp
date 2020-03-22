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
//       NNmain.cpp
//
// Purpose-
//       Neural Net control program.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/Interval.h>
#include <com/syslib.h>

#include "NN_com.h"
#include "NN_psv.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define PREFIX      "             " // Message spacing prefix

//----------------------------------------------------------------------------
// External areas
//----------------------------------------------------------------------------
NN_com*                nn_com;      // Pointer to global area
int                    HCDM;        // Hard-Core Debug mode

//----------------------------------------------------------------------------
// External routines
//----------------------------------------------------------------------------
extern void
   nnparm(                          // NEURON parameter control
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

//----------------------------------------------------------------------------
//
// Subsection-
//       _STKLEN
//
// Purpose-
//       Set alternate stack size.
//
//----------------------------------------------------------------------------
unsigned               _stklen= 0xFF00U; // Alternate stack size (large)
unsigned               _heaplen= 0; // Alternate heap length (small)

//----------------------------------------------------------------------------
//
// Subroutine-
//       NNMAIN
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Neuron control program
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Interval            timer;       // Interval timer
   NN_psv*             ptrpsv;      // -> Process State Vector
   NN::FileId          fileId;      // Initial neuron FileId
   NN::Offset          neuron;      // Initial neuron Offset
   NN::Value           resultant;   // Resultant

// int                 framesize;   // Current frame size
   int                 rc;          // Called routine return code

   //-------------------------------------------------------------------------
   // Global storage initialization
   //-------------------------------------------------------------------------
   nn_com= new NN_com();            // Construct inner objects

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   nnparm(argc, argv);              // Parameter analysis

   //-------------------------------------------------------------------------
   // Initialize trace
   //-------------------------------------------------------------------------
   if (NN_debug)                    // If debugging
     debugSetIntensiveMode();       // Intensive mode

   //-------------------------------------------------------------------------
   // Initialize VPS
   //-------------------------------------------------------------------------
   rc= NN_COM.pgs.warm(NN_COM.inpname, // Initialize VPS
                       0,
                       0);
   if (rc != 0 )                    // If initialization failed
   {
     printf("PGSINIT failed, RC= %d\n", rc);
     exit(EXIT_FAILURE);
   }
// framesize= NN_COM.pgs.getFrameSize();

#if 0
   printf("Warmstart\n");
   printf("%8d Framesize\n", framesize);
   printf("\n");
#endif

   //-------------------------------------------------------------------------
   // Internal initialization
   //-------------------------------------------------------------------------
   srand((unsigned)time(NULL));     // Initial random value

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   ptrpsv= (NN_psv*)nnuref(PSV_FILE, PSV_PART, PSV_OFFSET);// Locate the PSV
   if (ptrpsv == NULL)              // If PGS failure
   {
     printf("I/O failure, cannot get PSV\n");
     return 1;
   }

   NN_COM.clock= ptrpsv->clock;     // Set initial clock
   NN_COM.train= ptrpsv->train;

   fileId= ptrpsv->psvfileno;       // Set initial neuron
   neuron= ptrpsv->psvoffset;

   nnurel(PSV_FILE, PSV_PART, PSV_OFFSET); // Release the PSV

   //-------------------------------------------------------------------------
   // Update the clock - (allows the clock to be read)
   //-------------------------------------------------------------------------
   NN_COM.clock++;                  // Jump start

   //-------------------------------------------------------------------------
   // Activate the net
   //-------------------------------------------------------------------------
   timer.start();                   // Start the wall clock timer
   resultant= nnreadv(fileId, neuron); // Begin processing
   timer.stop();                    // Stop the wall clock timer

   //-------------------------------------------------------------------------
   // Statistics display
   //-------------------------------------------------------------------------
   printf("%s %14.3f Seconds execution time\n", PREFIX, timer.toDouble());
   printf("\n");

   printf("%s %14.3f Resultant\n", PREFIX, (double)resultant);
   printf("\n");

   printf("%s 0x%.8lX.%.8lX (%8ld) Clock\n", PREFIX,
          (long)NN_COM.train, (long)NN_COM.clock,
          (long)NN_COM.clock);
   printf("%s 0x%.8lX.%.8lX (%8ld) read_val()s\n", PREFIX,
          (long)NN_COM.read_val[0], (long)NN_COM.read_val[1],
          (long)NN_COM.read_val[1]);

   //-------------------------------------------------------------------------
   // Save the current clock
   //-------------------------------------------------------------------------
   ptrpsv= (NN_psv*)nnuchg(PSV_FILE, PSV_PART, PSV_OFFSET); // Locate the PSV
   if (ptrpsv != NULL)              // If PSV found
   {
     ptrpsv->clock= NN_COM.clock;   // Save the current clock
     ptrpsv->train= NN_COM.train;

     nnurel(PSV_FILE, PSV_PART, PSV_OFFSET); // Release the PSV
   }
   NN_COM.pgs.term();               // Terminate PGS

   return 0;
}

