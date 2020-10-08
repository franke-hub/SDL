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
//       exFind.cpp
//
// Purpose-
//       Exon/Intron DataBase, controlled sequence scan.
//
// Last change date-
//       2002/06/17
//
// Description-
//       This routine examines an Exon/Intron database file, looking for
//       sequence matches.  It does this by reading the database into storage
//       then scanning the in-storage database for the sequences of interest.
//
//       While reading the database, it uses and Accumulator and an Extractor
//       to control the exact format of the in-storage database.  Both the
//       label and the data items are accumulated.
//
//       When a sequence match is found, the associated label and data items
//       are written to an output file.
//
//       Although this routine contains an INTRON_SCANNER compile control,
//       this control is not used and its compilation has not been tested.
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
#define __SOURCE__       "EXFIND  " // Source file

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
  #define ACCUMULATOR  DataAccumulator
  #define EXTRACTOR    IntronExtractor
#else
  #define SCANNER_TYPE "Exon"
  #define ACCUMULATOR  DataAccumulator
  #define EXTRACTOR    ExonExtractor
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static EiDB            label;       // EiDB label database
static EiDB            eidb;        // EiDB Exon database
static List*           list;        // List array

static int             fileName;    // The fileName parameter index
static unsigned        minCol;      // Lowest column number
static unsigned        maxCol;      // Highest column number
static char            sw_exon;     // TRUE iff -exon option specified
static char            sw_rev;      // TRUE iff -rev option specified
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
   fprintf(stderr,"Usage: %s <Global options>", __SOURCE__);
   fprintf(stderr," filename");
   fprintf(stderr," <<Scan options> sequence ...>");
   fprintf(stderr,"\n");
   fprintf(stderr,"Exon/Interon database scanner\n");
   fprintf(stderr,"Scan an EiDB file looking for sequences\n"
                  "When a sequence is found, the header and all exons"
                  "are listed\n");
   fprintf(stderr,"\n\n");

   fprintf(stderr,"Global options:\n"
                  "-rev\n"
                  "\tUse right adjustment\n");
   fprintf(stderr,"\n\n");

   fprintf(stderr,"filename\n"
                  "\tThe name of the EiDB format file\n");
   fprintf(stderr,"\n\n");

   fprintf(stderr,"Scan options:\n"
                  "-min:column\n"
                  "\tMinimum column number\n"
                  "-max:column\n"
                  "\tMaximum column number\n");

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
   for(int j=1; j<argc; j++)        // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp("-help", argv[j]) == 0 )
         error= TRUE;

       if( strcmp("-exon", argv[j]) == 0 )
         sw_exon= TRUE;

       if( strcmp("-rev", argv[j]) == 0 )
         sw_rev= TRUE;

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
   EXTRACTOR           itemExtractor; // Exon/Intron Extractor
   Extractor           fullExtractor; // Full Extractor
   EiDBLoader          loader;      // The loader
   char*               item;        // Current exon or intron

   int                 rc;

   // Load the label database
   accumulator= new LabelAccumulator();
   if( accumulator->open(fileName) != 0 )
     return;
   rc= loader.load(label, *accumulator, fullExtractor);
   if( rc != 0 )
     exit(EXIT_FAILURE);
   delete accumulator;

   // Load the item database
   accumulator= new DataAccumulator();
   if( accumulator->open(fileName) != 0 )
     return;
   rc= loader.load(eidb, *accumulator, fullExtractor);
   if( rc != 0 )
     exit(EXIT_FAILURE);
   delete accumulator;

   // Extract the items
   assert( eidb.getLineCount() == label.getLineCount() );
   list= (List*)malloc(eidb.getLineCount()*sizeof(List));
   for(unsigned i= 0; i<eidb.getLineCount(); i++)
   {
     new(&list[i]) List();
     itemExtractor.load((char*)eidb.getLine(i));
     for(;;)
     {
       item= itemExtractor.next(i);
       if( item == NULL )
         break;

       list[i].append(item);
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
   scan(                            // Scan the data table
     const char*       fileName,    // Output file name
     FILE*             file,        // Output file handle
     const char*       target)      // Search target
{
   (void)fileName;                  // Currently unused

   unsigned            const m= eidb.getLineCount();
   unsigned            const L= strlen(target);

   const char*         S;           // -> String
   char*               item;        // Source database Item
   unsigned            col;         // Source column index
   unsigned            row;         // Source row index
   unsigned            count;       // Item match count
   unsigned            length;      // Item length
   unsigned            offset;      // Item offset

   int                 i;

   // Scan items
   fprintf(file, "\n");
   fprintf(file, "Scan: '%s' Columns[%d:%d]\n", target, minCol+1, maxCol);

   for(row= 0; row<m; row++)
   {
     count= 0;
     for(col=0;;col++)
     {
       item= (char*)list[row].getItem(col);
       if( item == NULL )
         break;

       offset= minCol;
       length= strlen(item);
       if( maxCol < L )
         length= 0;

       else if( length > (maxCol-L) )
         length= (maxCol-L);

       while( offset <= length )
       {
         S= wildstr(item+offset, target);
         if( S == NULL )
           break;
         offset= S-item;
         if( offset > length )
           break;
         offset++;

         if( count == 0 )
         {
           fprintf(file, "\n%s\n", label.getLine(row));
           for(i= 0; ;i++)
           {
             if( list[row].getItem(i) == NULL )
               break;
             if( i != 0 )
               fprintf(file, " .. ");
             fprintf(file, "%s", (char*)list[row].getItem(i));
           }
           fprintf(file, "\n");
         }
         count++;

         fprintf(file, "%s[%d], column[%d]\n", SCANNER_TYPE, col+1, offset);
       }
     }
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
   const char*         name;        // Output file name
   FILE*               file;        // Output file
   int                 argx;        // Argument index

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
   name= "<stdout>";                // Default, output to stdout
   file= stdout;
   minCol= 0;
   maxCol= unsigned(-1);
   for(argx= fileName+1; argx<argc; argx++)
   {
     if( *argv[argx] != '-' )
     {
       scan(name, file, argv[argx]);
     }
     else
     {
       if( memcmp(argv[argx], "-file:", 6) == 0 )
       {
         name= argv[argx]+6;
         file= fopen(name, "wb");
         if( file == NULL )
         {
           fprintf(stderr, "File(%s): ", name);
           perror("open failure: ");
           fprintf(stderr, "Using stdout\n");
           name= "<stdout>";
           file= stdout;
         }
       }
       else if( memcmp(argv[argx], "-min:", 5) == 0 )
       {
         minCol= atol(argv[argx]+5);
         if( minCol == 0 )
           minCol= 1;

         minCol--;
       }
       else if( memcmp(argv[argx], "-max:", 5) == 0 )
       {
         maxCol= atol(argv[argx]+5);
       }
       else
         fprintf(stderr, "Scan option '%s' ignored!\n", argv[argx]);
     }
   }

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   term();
   return 0;
}

