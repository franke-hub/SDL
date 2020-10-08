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
//       dupScan.cpp
//
// Purpose-
//       Exon/Intron DataBase, scan for duplicate Exons.
//
// Last change date-
//       2002/06/17
//
// Description-
//       This routine examines an Exon/Intron database file, looking for
//       duplicate sequences.  For each duplicate found, the duplicate is
//       listed.  If the database consists of matching lines A, B and C,
//       A will match B and C and B will match C.  (This is a simple scan
//       of lines which follow.)
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
#include "Extractor.h"
#include "EiDBLoader.h"
#include "List.h"
#include "new.h"
#include "wildstr.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "DUPSCAN " // Source file

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

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static EiDB            label;       // EiDB label database
static EiDB            eidb;        // EiDB Exon database
static List*           list;        // List array

static int             fileName;    // The fileName parameter index
static char            sw_exon;     // TRUE iff -exon option specified
static char            sw_verbose;  // TRUE iff -v   option specified

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
   fprintf(stderr,"Usage: %s <options> ", __SOURCE__);
   fprintf(stderr,"filename");
   fprintf(stderr,"\n");
   fprintf(stderr,"Exon/Interon database scanner\n");
   fprintf(stderr,"Scan an EiDB file looking for duplicate sequences\n");
   fprintf(stderr,"By default, the complete sequence is considered\n");
   fprintf(stderr,"\n\n");

   fprintf(stderr,"Options:\n"
                  "-exon\tSearch for duplicate exons\n");
   fprintf(stderr,"\n\n");

   fprintf(stderr,"filename\n"
                  "\tThe name of the EiDB format file\n");

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
   sw_exon=    FALSE;               // Default switch settings
   sw_verbose= FALSE;

   fileName=   (-1);                // Set fileName parameter index

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(int j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

       if( strcmp("-exon", argv[j]) == 0 )
         sw_exon= TRUE;

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
     if( argc != j+1 )
     {
       error= TRUE;
       fprintf(stderr, "Extra parameters!\n");
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
// setWild('n', "actg");
// setWild('y', "ct");
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
   ExonExtractor       exonExtractor; // Exon Extractor
   Extractor           fullExtractor; // Full Extractor
   EiDBLoader          loader;      // The loader
   char*               exon;        // Current exon

   int                 rc;

   accumulator= new DataAccumulator();
   if( accumulator->open(fileName) != 0 )
     return;
   rc= loader.load(eidb, *accumulator, fullExtractor);
   if( rc != 0 )
     exit(EXIT_FAILURE);
   delete accumulator;

   accumulator= new LabelAccumulator();
   if( accumulator->open(fileName) != 0 )
     return;
   rc= loader.load(label, *accumulator, fullExtractor);
   if( rc != 0 )
     exit(EXIT_FAILURE);
   delete accumulator;

   assert( eidb.getLineCount() == label.getLineCount() );
   list= (List*)malloc(eidb.getLineCount()*sizeof(List));
   for(unsigned i= 0; i<eidb.getLineCount(); i++)
   {
     new(&list[i]) List();
     if( sw_exon )
     {
       exonExtractor.load((char*)eidb.getLine(i));
       for(;;)
       {
         exon= exonExtractor.next(i);
         if( exon == NULL )
           break;

         list[i].append(exon);
       }
     }
     else
     {
       fullExtractor.load((char*)eidb.getLine(i));
       for(;;)
       {
         exon= fullExtractor.next(i);
         if( exon == NULL )
           break;

         list[i].append(exon);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan
//
// Function-
//       Scan the database, looking for duplicates.
//
//----------------------------------------------------------------------------
static void
   scan(int, char**)                // Scan the data table
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   unsigned            const m= eidb.getLineCount();

   unsigned            dupCount;    // Number of duplicates encountered
   unsigned            dupTotal;    // Number of duplicates encountered
   char*               sourceExon;  // Source database Exon
   char*               targetExon;  // Target database Exon

   unsigned            sourceCol;   // Source column index
   unsigned            targetCol;   // Target column index

   unsigned            sourceRow;   // Source row index
   unsigned            targetRow;   // Target row index

   // Scan items
   if( sw_verbose )
     fprintf(stderr, "Scanning\n");

   dupTotal= 0;
   for(sourceRow= 0; sourceRow<m; sourceRow++)
   {
     if( sw_verbose )
       fprintf(stderr, "%8d\r", sourceRow);

     sourceCol= 0;
     for(;;)
     {
       sourceExon= (char*)list[sourceRow].getItem(sourceCol);
       if( sourceExon == NULL )
         break;

       sourceCol++;
       dupCount= 0;
       for(targetRow= sourceRow+1; targetRow<m; targetRow++)
       {
         targetCol= 0;
         for(;;)
         {
           targetExon= (char*)list[targetRow].getItem(targetCol);
           if( targetExon == NULL )
             break;

           targetCol++;
           if( strcmp(sourceExon,targetExon) == 0 )
           {
             if( sw_exon )
             {
               if( dupCount == 0 )
                 printf("\n"
                        "  Exon match: %s\n"
                        "Exon[%3d] of: %s\n"
                        , sourceExon
                        , sourceCol, label.getLine(sourceRow)
                        );
               printf("Exon[%3d] of: %s\n"
                      , targetCol, label.getLine(targetRow)
                      );
             }
             else
             {
               if( dupCount == 0 )
                 printf("\n"
                        "Match: %s\n"
                        "Label: %s\n"
                        , sourceExon
                        , label.getLine(sourceRow)
                        );
               printf("Label: %s\n"
                      , label.getLine(targetRow)
                      );
             }

             dupCount++;
             dupTotal++;
           }
         }
       }
     }
   }

   if( sw_verbose )
     fprintf(stderr, "\n");
   printf("%8u Duplicates found\n", dupTotal);
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
   scan(argc, argv);                // Scan the data table

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   term();
   return 0;
}

