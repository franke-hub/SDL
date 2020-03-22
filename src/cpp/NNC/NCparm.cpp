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
//       NCparm.cpp
//
// Purpose-
//       Neural Net Compiler, Parameter analysis.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>

#include <com/define.h>
#include <com/FileName.h>
#include <com/params.h>
#include <com/syslib.h>

#include "NC_com.h"
#include "NC_sys.h"
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCPARM  " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifdef  _OS_DOS                     // If this is a DOS compile
#define PARM_CHR                '/' // Switch control parameter
#define PARM_STR                "/" // Switch control parameter
#else                               // If not DOS

#define PARM_CHR                '-' // Switch control parameter
#define PARM_STR                "-" // Switch control parameter
#endif                              // __TURBOC__

//----------------------------------------------------------------------------
//
// Subroutine-
//       TF
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
// Segment-
//       INFO
//
// Purpose-
//       Display usage information.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Display usage information
{
   fprintf(stderr,
           "NC input-filedesc " PARM_STR"controls\n\n"
   PARM_STR"d:       Debugging level\n"
   PARM_STR"jig:     Development jig\n"
   PARM_STR"list     Generate compiler listing\n"
   PARM_STR"msghdr-  Do not print message headers\n"
   PARM_STR"symtab   Generate symbol table listing\n"
           );
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Segment-
//       NCPARM
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
extern void
   ncparm(                          // Parameter analysis routine
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
   NC_COM.max_stmt= 4096;
   NC_COM.sw_msghdr= TRUE;

   NC_COM.vps_framesize= NN_cfg::VPS_FRAMESIZE;
   NC_COM.vps_fileno=    NN_cfg::VPS_FILENO;
   NC_COM.vps_partno=    NN_cfg::VPS_PARTNO;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if (argc == 2 && *argv[1] == '?')// If query request
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

       else if (swname("d:", argp)) // If Debug level switch
         NC_debug= swatol("d:", argp);// Get parameter value

       else if (swname("jig:", argp))// If development jig
         NC_jig= swatol("jig:", argp);// Get parameter value

       else if (swname("list", argp)) // If listing
         NC_COM.sw_listing= swatob("list", argp);

       else if (swname("msghdr", argp))// If message headers
         NC_COM.sw_msghdr= swatob("msghdr", argp);

       else if (swname("symtab", argp))// If symbol table listing
         NC_COM.sw_symtab= swatob("symtab", argp);

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s' ignored\n",
                         argv[argi]);
       }
     }
     else                           // If not a switch parameter
     {
       if (NC_COM.inpname == NULL)  // If first file specifier
         NC_COM.inpname= argp;      // Set the input filename
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
   if (NC_COM.inpname == NULL)      // If input filename omitted
   {
     fprintf(stderr, "Input filename must be specified\n\n");
     info();
   }

   fileName.reset();
   fileName.append(NC_COM.inpname);
   fileName.append(".nnc");
   strcpy(NC_COM.outfile, fileName.getFileName());
   NC_COM.outname= NC_COM.outfile;

   fileName.reset();
   fileName.append(NC_COM.inpname);
   fileName.append(".n");
   strcpy(NC_COM.inpfile, fileName.getFileName());
   NC_COM.inpname= NC_COM.inpfile;

   if (error)                       // If error encountered
   {
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   if (verify)                      // If verification required
   {
     fprintf(stderr," Input file: %s\n",     NC_COM.inpname);
     fprintf(stderr,"Output file: %s\n",     NC_COM.outname);
     fprintf(stderr,"%8d Debug level\n",     NC_debug);
     fprintf(stderr,"%8s Message headers\n", tf(NC_COM.sw_msghdr));
     fprintf(stderr,"%8d Development jig\n", NC_jig);
     c= getchar();
     if (c == 27)
       exit(EXIT_FAILURE);
   }
}

