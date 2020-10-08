//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       exPhase.cpp
//
// Purpose-
//       Exon/Intron DataBase, split exons by phase.
//
// Last change date-
//       2002/06/17
//
// Description-
//       This routine examines an Exon/Intron database file separating the
//       input file into three output files, separated by phase.  Only the
//       Exons are output, the associated Introns are replaced by a ".."
//       sequence in the output file.
//
//       Note that this results in Exons that are not in sequence since
//       Exons of different phase go in different output files.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Accumulator.h"
#include "common.h"
#include "EiDB.h"
#include "EiDBLoader.h"
#include "Extractor.h"
#include "List.h"
#include "new.h"
#include "Wildstr.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "EXPHASE " // Source file

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
  #define FALSE 0
#endif

#ifndef TRUE
  #define TRUE  1
#endif

#ifdef INTRON_SCANNER
  #define SCANNER_TYPE "Intron"
#else
  #define SCANNER_TYPE "Exon"
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static EiDB            label;       // EiDB label database
static EiDB            eidb;        // EiDB Exon database
static List*           list;        // List array

static int             fileName;    // The fileName parameter index

static char            sw_end;      // TRUE iff -end option specified
static char            sw_verbose;  // TRUE iff -v   option specified

#ifndef INTRON_SCANNER
static char            sw_atg;      // TRUE iff -atg option specified
static char            sw_wild;     // TRUE iff -wild option specified
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Function-
//       Display parameter information.
//
//----------------------------------------------------------------------------
static void
   info( void )
{
   fprintf(stderr,"Usage: %s ", __SOURCE__
                  "<-options> "
                  "filename\n"
                  );

   fprintf(stderr,"\n");
   fprintf(stderr,"Separate Exon/Interon database by phase,\n"
                  "creating three output files:\n"
                  "\tfilename.0, filename.1 and filename.2\n"
                  "\n"
                  "Each output file contains the input file data where all\n"
                  "exons began (or ended) in the associated phase in the\n"
                  "original data file.\n"
                  "Note that the output file exons can shift phase with\n"
                  "respect to their phase in the original data file.\n"
                  "Although all Exons in each output file began (or ended)\n"
                  "with the same phase they are not necessarily of integral\n"
                  "codon lengths.\n"
                  );

   fprintf(stderr,"\n");
   fprintf(stderr,"filename\n"
                  "\tThe name of the EiDB format file\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-Options:\n");
#ifndef INTRON_SCANNER
   fprintf(stderr,"-atg\n"
                  "\tStart the first Exon at the first ATG sequence.\n");
   fprintf(stderr,"-wild\n"
                  "\tAllow wild character matching.\n");
#endif
   fprintf(stderr,"-end\n"
                  "\tConsider the ending (not the starting) phase.\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Function-
//       Parameter analysis routine.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Analyze parameters
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
{
   int                 error;       // Error switch

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_verbose= FALSE;
#ifndef INTRON_SCANNER
   sw_atg=     FALSE;
   sw_wild=    FALSE;
#endif
   sw_end=     FALSE;

   fileName=   (-1);                // Set fileName parameter index

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(int j=1; j<argc; j++)        // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

#ifndef INTRON_SCANNER
       else if( strcmp("-atg", argv[j]) == 0 )
         sw_atg= TRUE;

       else if( strcmp("-wild", argv[j]) == 0 )
         sw_wild= TRUE;
#endif

       else if( strcmp("-end", argv[j]) == 0 )
         sw_end= TRUE;

       else                         // Switch list
       {
         for(int i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'v':              // -v (verbose)
               sw_verbose= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
       continue;
     }

     //-----------------------------------------------------------------------
     // Process a flat (non-switch) parameter
     //-----------------------------------------------------------------------
     fileName= j;                   // Set the filename index
     if( j != (argc-1) )
     {
       error= TRUE;
       fprintf(stderr, "Too many parameters\n");
     }
     break;
   }

   //-------------------------------------------------------------------------
   // Validate the parameters
   //-------------------------------------------------------------------------
   if( fileName < 0 )               // If fileName not specified
   {
     error= TRUE;
     fprintf(stderr, "Missing filename.\n");
   }

   if( error )                      // If an error was detected
   {
     info();                        // Tell how this works
     exit(EXIT_FAILURE);            // And exit, function aborted
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Function-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init(int, char**)                // Initialize
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   // Set wildcards
   #ifdef INTRON_SCANNER
     setWild('n', "actg");
     setWild('y', "ct");
     setWild('r', "ag");
     setWild('m', "ac");
     setWild('w', "at");
     setWild('s', "cg");
     setWild('k', "gt");
     setWild('b', "cgt");
     setWild('d', "agt");
     setWild('v', "acg");
     setWild('h', "act");

   #else
     setWild('N', "ACTG");
     setWild('Y', "CT");
     setWild('R', "AG");
     setWild('M', "AC");
     setWild('W', "AT");
     setWild('S', "CG");
     setWild('K', "GT");
     setWild('B', "CGT");
     setWild('D', "AGT");
     setWild('V', "ACG");
     setWild('H', "ACT");
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Function-
//       Terminate.
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Initialize
{
   eidb.empty();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Function-
//       Load the databases.
//
//----------------------------------------------------------------------------
static void
   load(                            // Load the data table
     const char*       fileName)    // File name to load
{
   Accumulator*        accumulator; // The Accumulator
   Extractor*          itemExtractor; // Exon/Intron Extractor
   Extractor           fullExtractor; // Full Extractor
   EiDBLoader          loader;      // The loader
   char*               item;        // Current exon or intron

   int                 rc;

   // Load the labels
   accumulator= new LabelAccumulator();
   if( accumulator->open(fileName) != 0 )
     exit(EXIT_FAILURE);

   rc= loader.load(label, *accumulator, fullExtractor);
   if( rc < 0 )
     exit(EXIT_FAILURE);
   if( rc > 0 )
     fprintf(stderr, "%s loaded with errors\n", fileName);

   delete accumulator;

   // Set itemExtractor
   #ifdef INTRON_SCANNER
     itemExtractor= new IntronExtractor();
     #ifdef HCDM
       printf("itemExtractor: IntronExtractor()\n");
     #endif

   #else
     if( sw_atg )
     {
       itemExtractor= new AtgExtractor(sw_wild);
       #ifdef HCDM
         printf("itemExtractor: AtgExtractor()\n");
       #endif
     }
     else
     {
       itemExtractor= new ExonExtractor();
       #ifdef HCDM
         printf("itemExtractor: ExonExtractor()\n");
       #endif
     }
   #endif

   // Load the items
   accumulator= new DataAccumulator();
   if( accumulator->open(fileName) != 0 )
     exit(EXIT_FAILURE);

   rc= loader.load(eidb, *accumulator, fullExtractor);
   if( rc < 0 )
     exit(EXIT_FAILURE);
   if( rc > 0 )
     fprintf(stderr, "%s loaded with errors\n", fileName);

   delete accumulator;
   assert( eidb.getLineCount() == label.getLineCount() );

   // Extract the items
   list= (List*)malloc(eidb.getLineCount()*sizeof(List));
   for(unsigned i= 0; i<eidb.getLineCount(); i++)
   {
     new(&list[i]) List();
     itemExtractor->load((char*)eidb.getLine(i));
     #ifdef HCDM
       printf("\n");
       printf("Line: '%s'\n", eidb.getLine(i));
     #endif
     for(;;)
     {
       item= itemExtractor->next(i);
       if( item == NULL )
         break;

       #ifdef HCDM
         printf("\n");
         printf("Item: '%s'\n", item);
       #endif
       list[i].append(item);
     }
   }

   // Delete the Extractor
   delete itemExtractor;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       split
//
// Function-
//       Split database by phase.
//
//----------------------------------------------------------------------------
static void
   split(                           // Split database by phase
     const char*       fileName,    // Output file name
     unsigned          inpPhase)    // Target phase
{
   unsigned            const m= eidb.getLineCount();

   FILE*               file;        // Output file handle
   char                name[1024];  // Output file name
   char*               item;        // Source database Item
   unsigned            col;         // Source column index
   unsigned            row;         // Source row index
   unsigned            count;       // Item match count
   unsigned            phase;       // Item phase
   unsigned            length;      // Item length
   unsigned            offset;      // Item offset
   char                line[88];    // Output line
   unsigned            selected;    // TRUE iff column selected

   // Create the phase filename
   sprintf(name, "%s.%d", fileName, inpPhase);
   file= fopen(name, "wb");
   if( file == NULL )
   {
     fprintf(stderr, "File(%s): ", name);
     perror("open failure: ");
     return;
   }

   // Split by phase
   memset(line, 0, sizeof(line));
   for(row= 0; row<m; row++)
   {
     count= 0;
     phase= 0;
     for(col=0;;col++)
     {
       item= (char*)list[row].getItem(col);
       if( item == NULL )
         break;

       length= strlen(item);
       selected= FALSE;
       if( sw_end )
       {
         if( ((phase+length)%3) == inpPhase )
           selected= TRUE;
       }
       else if( phase == inpPhase )
         selected= TRUE;

       if( selected )
       {
         if( count == 0 )
           fprintf(file, "%s\n", label.getLine(row));
         else
           fprintf(file, "..\n");

         for(offset= 0; offset<length; offset+=80)
         {
           if( (length-offset) > 80 )
             memcpy(line, &item[offset], 80);
           else
           {
             memcpy(line, &item[offset], length-offset);
             line[length-offset]= '\0';
           }
           fprintf(file, "%s\n", line);
         }
         count++;
       }

       phase += length;
       phase %= 3;
     }
     if( count > 0 )
       fprintf(file, "\n");
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
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   unsigned            phase;       // Specified phase

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis
   init(argc, argv);                // Initialize

   //-------------------------------------------------------------------------
   // Load the data table
   //-------------------------------------------------------------------------
   load(argv[fileName]);            // Load the data table

   //-------------------------------------------------------------------------
   // Scan the data table
   //-------------------------------------------------------------------------
   for(phase=0; phase<3; phase++)
   {
     split(argv[fileName], phase);
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   term();
   return 0;
}

