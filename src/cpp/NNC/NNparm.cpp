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
//       NNparm.cpp
//
// Purpose-
//       Neural Net Parameter analysis.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/FileName.h>
#include <com/params.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NNPARM  " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define PARM_CHR                '-' // Switch control parameter
#define PARM_STR                "-" // Switch control parameter

//----------------------------------------------------------------------------
//
// Subroutine-
//       tf
//
// Purpose-
//       Return " TRUE" or "FALSE" character string.
//
//----------------------------------------------------------------------------
static const char*
   tf(                              // TRUE or FALSE
     int               value)       // The name of the parameter
{
   if (value)                       // If the value is TRUE
     return(" TRUE");               // Return " TRUE"

   return("FALSE");                 // Otherwise, return "FALSE"
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Display usage information.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Display usage information
{
   fprintf(stderr,
       "NEURON input-filedesc <output-filedesc> " PARM_STR "controls\n\n"
       PARM_STR "g        Graphic trace\n"
       PARM_STR "d        Debugging trace\n"
       PARM_STR "t        Internal trace\n"
       PARM_STR "time     Development timer\n"
       PARM_STR "jig:     Development jig\n"
       );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnparm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
extern void nnparm(int, char**);    // (Defined where used)
extern void
   nnparm(                          // Parameter analysis routine
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   FileName            fileName;    // FileName object
   int                 error;       // Error encountered indicator
   int                 verify;      // Verification control
   int                 c;

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   verify= 0;                       // Default, no verification

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if (argc == 2 && *argv[1] == '?') // If query request
   {
     info();                        // Display options
     exit(EXIT_FAILURE);            // And exit, function complete
   }

   error= FALSE;                    // Default, no errors found
   for (argi=1; argi<argc; argi++)  // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if (*argp == PARM_CHR)         // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

       if (swname("v", argp))       // If verify switch
         verify= swatob("v", argp); // Get switch value

       else if (swname("d", argp))  // If Debug trace switch
         NN_debug= swatob("d", argp);// Get switch value

       else if (swname("g", argp))  // If Graphics trace switch
         NN_graph= swatob("g", argp);// Get switch value

       else if (swname("t", argp))  // If Internal trace switch
         NN_trace= swatob("t", argp);// Get switch value

       else if (swname("time", argp))// If Timing trace switch
         NN_timer= swatob("time", argp);// Get switch value

       else if (swname("jig:", argp))// If development jig
         NN_jig= swatol("jig:", argp);// Get parameter value

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s' ignored\n",
                         argv[argi]);
       }
     }
     else                           // If not a switch parameter
     {
       if (NN_COM.inpname == NULL)  // If first file specifier
         NN_COM.inpname= argp;      // Set the input filename
       else if (NN_COM.outname == NULL)// If second file specifier
         NN_COM.outname= argp;      // Set the output filename
       else                         // If too many filenames
       {
         error= TRUE;
         fprintf(stderr, "Unknown parameter: '%s'\n", argp);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if (NN_COM.inpname == NULL)      // If input filename omitted
   {
     fprintf(stderr, "Input filename must be specified\n\n");
     info();
   }

   fileName.reset();
   fileName.append(NN_COM.inpname);
   fileName.append(".nnc");
   strcpy(NN_COM.inpfile, fileName.getFileName());
   NN_COM.inpname= NN_COM.inpfile;

   if (NN_COM.outname != NULL)
   {
     fileName.reset();
     fileName.append(NN_COM.inpname);
     fileName.append(".nnc");
     strcpy(NN_COM.outfile, fileName.getFileName());
     NN_COM.outname= NN_COM.outfile;
   }

   if (error)                       // If error encountered
   {
     fprintf(stderr, "ESC to exit, any other key to continue\n");
     fflush(stderr);
     c= getchar();
     if (c == 27)
       exit(EXIT_FAILURE);
   }

   if (verify)                      // If verification required
   {
     fprintf(stderr,"%8s Debug\n",           tf(NN_debug));
     fprintf(stderr,"%8s General Trace\n",   tf(NN_trace));
     fprintf(stderr,"%8s Timing Trace\n",    tf(NN_timer));
     fprintf(stderr,"%8d Development jig\n", NN_jig);
     c= getchar();
     if (c == 27)
       exit(EXIT_FAILURE);
   }
}

