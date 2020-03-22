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
//       exCodon.cpp
//
// Purpose-
//       Exon/Intron DataBase scanner.
//
// Last change date-
//       2002/06/17
//
// Description-
//       This routine examines an Exon/Intron database file, looking for
//       all possible sequences of exons which surround an intron.  It does
//       this by reading the database into storage then scanning the
//       in-storage database for the sequences of interest.
//
//       While reading the database, it uses and Accumulator and an Extractor
//       to control the exact format of the in-storage database.  Different
//       types of Accumulator and Extractor objects are used to control the
//       database loading, and are selected by program option controls.
//
//       This routine is ONLY an Exon scanner.  The INTRON_SCANNER compile
//       time flag is a relic from the derived module.  This module has not
//       been compiled or tested (and makes no sense) as an intron scanner.
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
#ifdef INTRON_SCANNER
#define __SOURCE__        "inCodon" // Source file
#else
#define __SOURCE__        "exCodon" // Source file
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SORT
#undef  SORT                        // If defined, sort output by size
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
  #define Exon_Intron "Intron"
  #define UPPER_lower "lower"
#else
  #define Exon_Intron "Exon"
  #define UPPER_lower "upper"
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static EiDB            eidb;        // EiDB object
static EiDBLoader      loader;      // EiDB loader object
static List*           list;        // List array

static int             colZero;     // Column number zero
static int             fileName;    // The fileName parameter index
static int             maxSize;     // MaxSize parameter
static int             minSize;     // MinSize parameter

static char            sw_verbose;  // TRUE iff -v     option specified

static char            sw_atg;      // TRUE iff -atg   option specified
static char            sw_first;    // TRUE iff -first option specified
static char            sw_last;     // TRUE iff -last  option specified
static char            sw_only;     // TRUE iff -only  option specified
static char            sw_out;      // TRUE iff -out   option specified
static char            sw_rev;      // TRUE iff -rev   option specified
static char            sw_wild;     // TRUE iff -wild  option specified

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
#define MAX_CODON                64 // Number of sequences
static const char*     codon[MAX_CODON]= { // Codon sequences
   "AAA",                           // 00
   "AAC",                           // 01
   "AAG",                           // 02
   "AAT",                           // 03
   "ACA",                           // 04
   "ACC",                           // 05
   "ACG",                           // 06
   "ACT",                           // 07
   "AGA",                           // 08
   "AGC",                           // 09
   "AGG",                           // 10
   "AGT",                           // 11
   "ATA",                           // 12
   "ATC",                           // 13
   "ATG",                           // 14
   "ATT",                           // 15

   "CAA",                           // 16
   "CAC",                           // 17
   "CAG",                           // 18
   "CAT",                           // 19
   "CCA",                           // 20
   "CCC",                           // 21
   "CCG",                           // 22
   "CCT",                           // 23
   "CGA",                           // 24
   "CGC",                           // 25
   "CGG",                           // 26
   "CGT",                           // 27
   "CTA",                           // 28
   "CTC",                           // 29
   "CTG",                           // 30
   "CTT",                           // 31

   "GAA",                           // 32
   "GAC",                           // 33
   "GAG",                           // 34
   "GAT",                           // 35
   "GCA",                           // 36
   "GCC",                           // 37
   "GCG",                           // 38
   "GCT",                           // 39
   "GGA",                           // 40
   "GGC",                           // 41
   "GGG",                           // 42
   "GGT",                           // 43
   "GTA",                           // 44
   "GTC",                           // 45
   "GTG",                           // 46
   "GTT",                           // 47

   "TAA",                           // 48
   "TAC",                           // 49
   "TAG",                           // 50
   "TAT",                           // 51
   "TCA",                           // 52
   "TCC",                           // 53
   "TCG",                           // 54
   "TCT",                           // 55
   "TGA",                           // 56
   "TGC",                           // 57
   "TGG",                           // 58
   "TGT",                           // 59
   "TTA",                           // 60
   "TTC",                           // 61
   "TTG",                           // 62
   "TTT"};                          // 63

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
   fprintf(stderr,"Usage: %s ", __SOURCE__);
   fprintf(stderr,"<-options> filename");
   fprintf(stderr,"\n");
   fprintf(stderr,"Exon/Intron %s database scanner.\n", Exon_Intron);
   fprintf(stderr,"Scan an EiDB database file looking for Exons which\n"
                  "surround Introns.\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"filename\n"
                  "\tThe name of the EiDB format database file.\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"-Options:\n");
#ifndef INTRON_SCANNER
   fprintf(stderr,"-atg\n"
                  "\tStart the first Exon at the first ATG sequence.\n");
   fprintf(stderr,"-first\n"
                  "\tIgnore the first %s if it begins a sequence.\n",
                  Exon_Intron);
   fprintf(stderr,"-last\n"
                  "\tIgnore the last %s if it completes a sequence.\n",
                  Exon_Intron);
   fprintf(stderr,"-only\n"
                  "\tInvert the action of -first and -last.\n");
#endif
   fprintf(stderr,"-maxsize:value\n"
                  "\tIgnore database rows with more than <value> columns.\n");
   fprintf(stderr,"-minsize:value\n"
                  "\tIgnore database rows with less than <value> columns.\n");
   fprintf(stderr,"-out\n"
                  "\tDisplay the database, as loaded.\n");
   fprintf(stderr,"-rev\n"
                  "\tUse right adjustment.\n"
                  "\tNote: When using right adjustment, column number 1 is\n"
                  "\tconsidered the right-most column.\n");
   fprintf(stderr,"-wild\n"
                  "\tAllow wild character matching.\n");

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
extern void
   parm(                            // Analyze parameters
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
{
   int                 error;       // Error switch

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_verbose=  TRUE;               // Default switch settings
   sw_atg=     FALSE;
   sw_first=   FALSE;
   sw_out=     FALSE;
   sw_rev=     FALSE;
   sw_wild=    FALSE;

   minSize=    (-1);
   maxSize=    (-1);

   fileName=   (-1);                // Set fileName parameter index
   colZero=    1;                   // Alias for column[0]

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

       else if( strcmp("-first", argv[j]) == 0 )
       {
         sw_first= TRUE;
         loader.setIgnoreFirst(TRUE);
       }

       else if( strcmp("-last", argv[j]) == 0 )
       {
         sw_last= TRUE;
         loader.setIgnoreLast(TRUE);
       }

       else if( strcmp("-only", argv[j]) == 0 )
       {
         sw_only= TRUE;
         loader.setIgnoreOnly(TRUE);
       }
#endif

       else if( memcmp("-maxsize:", argv[j], 9) == 0 )
       {
         maxSize= atol(argv[j]+9);
         loader.setMaxSize(maxSize);
       }

       else if( memcmp("-minsize:", argv[j], 9) == 0 )
       {
         minSize= atol(argv[j]+9);
         loader.setMinSize(minSize);
       }

       else if( strcmp("-out", argv[j]) == 0 )
         sw_out= TRUE;

       else if( strcmp("-rev", argv[j]) == 0 )
         sw_rev= TRUE;

       else if( strcmp("-wild", argv[j]) == 0 )
         sw_wild= TRUE;

       else if( strcmp("-v-", argv[j]) == 0 )
         sw_verbose= FALSE;

       else                         // Switch list
       {
         for(int i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
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
     if( fileName > 0 )
     {
       error= TRUE;
       fprintf(stderr, "Unexpected parameter '%s'\n", argv[j]);
     }

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

   // Display summary information
   if( sw_verbose )
   {
     printf("=============================================================\n");
     printf("%16s: %s %s\n", __SOURCE__, __DATE__, __TIME__);
     printf("        Database: %s\n", argv[fileName]);
#ifndef INTRON_SCANNER
     if( sw_atg )
       printf("            -atg: YES. The ATG sequence begins each Exon.\n");
     else
       printf("            -atg:  NO. Any character can begin an Exon.\n");

     if( sw_first )
       printf("          -first: YES. The first %s in a gene is ignored.\n",
              Exon_Intron);
     else
       printf("          -first:  NO. The first %s in a gene is used.\n",
              Exon_Intron);

     if( sw_last )
       printf("           -last: YES. The last %s in a gene is ignored.\n",
              Exon_Intron);
     else
       printf("           -last:  NO. The last %s in a gene is used.\n",
              Exon_Intron);

     if( sw_only )
       printf("           -only: YES. Inverts the action of "
              "-first and -last.\n");
     else
       printf("           -only:  NO. (default)\n");
#endif

     if( sw_rev )
       printf("            -rev: YES. Sequences go right to left.\n");
     else
       printf("            -rev:  NO. Sequences go left to right.\n");

     if( sw_wild )
       printf("           -wild: YES. Wild characters are always expanded.\n"
              "                       Characters match wild equivalents.\n");
     else
       printf("           -wild:  NO. Wild characters are never expanded.\n"
              "                       Character matches are exact.\n");

     if( maxSize == (-1) )
       printf("        -maxsize:  NO. No maximum row size.\n");
     else
       printf("        -maxsize: %3d. Rows containing more than maxsize "
                                      "characters\n"
              "                       are excluded from the database.\n",
              maxSize);

     if( minSize == (-1) )
       printf("        -minsize:  NO. No minimum row size.\n");
     else
       printf("        -minsize: %3d. Rows containing fewer than minsize "
                                      "characters\n"
              "                       are excluded from the database.\n",
              minSize);
     printf("=============================================================\n");
     printf("\n");
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
   init(                            // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
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
//       Load the data table.
//
//----------------------------------------------------------------------------
static void
   load(                            // Load the data table
     const char*       fileName)    // File name to load
{
   Accumulator*        accumulator; // Accumulator
   Extractor*          itemExtractor; // Item Extractor
   Extractor           fullExtractor; // Full Extractor

   int                 col;         // Current column number
   const char*         item;        // Current exon or intron
   int                 row;         // Current database row
   EiDBLoader::LOADMODE
                       mode;        // Loading mode

   int                 rc;
   int                 i;

   // Load the items
   accumulator= new DataAccumulator();
   if( accumulator->open(fileName) != 0 )
     exit(EXIT_FAILURE);

   mode= loader.MODE_LEFTRIGHT;
   if( sw_rev == TRUE )
     mode= loader.MODE_RIGHTLEFT;

   rc= loader.load(eidb, *accumulator, fullExtractor, mode);
   if( rc < 0 )
     exit(EXIT_FAILURE);
   if( rc > 0 )
     fprintf(stderr, "%s loaded with errors\n", fileName);

   delete accumulator;

   // Set Extractor
   #ifdef INTRON_SCANNER
     itemExtractor= new IntronExtractor();

   #else
     if( sw_atg )
       itemExtractor= new AtgExtractor(sw_wild);
     else
       itemExtractor= new ExonExtractor();
   #endif

   // Extract the items
   list= (List*)malloc(eidb.getLineCount()*sizeof(List));
   for(row= 0; row<eidb.getLineCount(); row++)
   {
     new(&list[row]) List();
     itemExtractor->load((char*)eidb.getLine(row));
     for(;;)
     {
       item= itemExtractor->next(row);
       if( item == NULL )
         break;

       list[row].append((void*)item);
     }
   }

   delete itemExtractor;

   // Display the database
   if( sw_out )
   {
     for(row= 0; row<eidb.getLineCount(); row++)
     {
       printf("%6d: ", row);

       col= 0;
       for(i=0;;i++)
       {
         item= (char*)list[row].getItem(i);
         if( item == NULL )
           break;

         if( i > 0 )
         {
           if( (col%3) == 0 )
             printf(" ");
           printf("..");
         }

         while( *item != '\0' )
         {
           if( (col%3) == 0 && col > 0 )
             printf(" ");

           printf("%c", *item);
           item++;
           col++;
         }
       }
       printf("\n");
     }
     printf("\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       compare
//
// Function-
//       Compare strings based on sw_wild
//
//----------------------------------------------------------------------------
static inline int                   // Resultant
   compare(                         // Compare
     const char*       source,      // Source string
     const char*       target,      // Compare sting
     unsigned          length= 3)   // String length
{
   if( sw_wild != 0 )
     return wildcmp(source,target,length);

   return memcmp(source,target,length);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       head0
//
// Function-
//       Heading for phase0 scan.
//
//----------------------------------------------------------------------------
static void
   head0( void )                    // Display heading
{
   printf("\n");
   printf("Phase 0 scan:\n"
          "\t   End: Ending exon codon sequence\n"
          "\t    ..: The Intron sequence\n"
          "\t   Beg: Beginning exon codon sequence\n"
          "\tmatchs: Number of codons matching the combined phase 0 sequence\n"
          "\tfinals: Number of codons matching the ending phase 0 sequence\n"
          "\tfirsts: Number of codons matching the beginning phase 0 sequence\n"
          "\n"
          "End .. Beg matchs [finals .. firsts]\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan0
//
// Function-
//       Scan the database, looking for phase0 exon splits.
//
//----------------------------------------------------------------------------
static void
   scan0(                           // Scan the data table
     const char*       endex,       // Ending codon
     const char*       begex)       // Beginning codon
{
   int                 const m= eidb.getLineCount();

   int                 bCount;      // Number of beginning matches
   int                 bFound;      // Beginning match on this item?
   int                 col;         // Current origin column
   const char*         item;        // Current exon or intron
   int                 eCount;      // Number of ending matches
   int                 eFound;      // Ending match on this item?
   int                 mCount;      // Number of matches
   int                 L;           // Item length
   int                 row;         // Row index

   int                 i;

   // Initialize
   mCount= 0;
   bCount= 0;
   eCount= 0;

   // Scan the database
   for(row= 0; row<m; row++)
   {
     col= 0;
     eFound= 0;
     for(i=0;;i++)
     {
       item= (char*)list[row].getItem(i);
       if( item == NULL )
         break;

       L= strlen(item);
       bFound= 0;

       // Count starting item
       if( i > 0                    // Don't count the first item
           && (col%3) == 0          // Don't count if not phase 0 start
           && L >= 3                // Don't count if too small
           && compare(item, begex) == 0 ) // Don't count mismatch
         bFound= 1;                 // Found match
       bCount += bFound;

       // Check for match
       if( bFound != 0 && eFound != 0 )
         mCount++;

       // Count ending item
       eCount += eFound;
       eFound= 0;
       col += L;                    // Position at ending column
       if( (col%3) == 0             // Don't count if not phase 0 end
           && L >= 3                // Don't count if too small
           && compare(item+L-3, endex) == 0 ) // Don't count mismatch
         eFound= 1;                 // Found match

       #if 0                        // Phase 0 HCDM
       {{
         char        begit[8];
         char        endit[8];

         const char* begbug= "GAC";
         const char* endbug= "TTT";

         memset(begit,'\0',sizeof(begit));
         memset(endit,'\0',sizeof(endit));

         strcpy(begit, "***");
         strcpy(endit, "***");
         if( L >= 3 )
         {
           for(int j=0; j<3; j++)
           {
             begit[j]= item[j];
             endit[j]= item[j+L-3];
           }
         }

         if(    compare(begex,begbug) == 0
                &&
                compare(endex,endbug) == 0
           )
         {
           printf("%s .. %s bFound(%d,%s) eFound(%d,%s) "
                  "%4d:%3d [%4d..%4d] '%s'\n"
                  , begex, endex
                  , bFound, begit
                  , eFound, endit
                  , row, i, col-L, col, item);
         }
       }}
       #endif
     }
   }

   printf("%s .. %s %6d [%6d .. %6d]\n", endex, begex, mCount, eCount, bCount);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       head1
//
// Function-
//       Heading for phase1 scan.
//
//----------------------------------------------------------------------------
static void
   head1( void )                    // Display heading
{
   printf("\n");
   printf("Phase 1 scan:\n"
          "\t   End: Ending exon sequence\n"
          "\t    ..: The Intron sequence\n"
          "\t   Beg: Beginning exon sequence\n"
          "\tmatchs: Number of exons matching the combined phase 1 sequence\n"
          "\tfinals: Number of exons matching the ending phase 1 sequence\n"
          "\tfirsts: Number of exons matching the beginning phase 1 sequence\n"
          "\n"
          " End..Beg  matchs [finals .. firsts]\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan1
//
// Function-
//       Scan the database, looking for phase1 exon splits.
//
//----------------------------------------------------------------------------
static void
   scan1(                           // Scan the data table
     const char*       exon)        // Exon
{
   int                 const m= eidb.getLineCount();

   char                endex[2];    // Ending exon sequence
   char                begex[3];    // Beginning exon sequence
   int                 bCount;      // Number of beginning matches
   int                 bFound;      // Beginning match on this item?
   int                 col;         // Current origin column
   const char*         item;        // Current exon or intron
   int                 eCount;      // Number of ending matches
   int                 eFound;      // Ending match on this item?
   int                 mCount;      // Number of matches
   int                 L;           // Item length
   int                 row;         // Row index

   int                 i;

   // Initialize
   mCount= 0;
   bCount= 0;
   eCount= 0;

   endex[0]= exon[0];
   endex[1]= '\0';
   begex[0]= exon[1];
   begex[1]= exon[2];
   begex[2]= '\0';

   // Scan the database
   for(row= 0; row<m; row++)
   {
     col= 0;
     eFound= 0;
     for(i=0;;i++)
     {
       item= (char*)list[row].getItem(i);
       if( item == NULL )
         break;

       L= strlen(item);
       bFound= 0;

       // Count starting item
       if( i > 0                    // Don't count the first item
           && (col%3) == 1          // Don't count if not phase 1 start
           && L >= 2                // Don't count if too small
           && compare(item, begex, 2) == 0 ) // Don't count mismatch
         bFound= 1;                 // Found match
       bCount += bFound;

       // Check for match
       if( bFound != 0 && eFound != 0 )
         mCount++;

       // Count ending item
       eCount += eFound;
       eFound= 0;
       col += L;                    // Position at ending column
       if( (col%3) == 1             // Don't count if not phase 1 end
           && L >= 1                // Don't count if too small
           && compare(item+L-1, endex, 1) == 0 ) // Don't count mismatch
         eFound= 1;                 // Found match

       #if 0                        // Phase 1 HCDM
       {{
         char        begit[8];
         char        endit[8];

         const char* begbug= "GG";
         const char* endbug= "G";

         memset(begit,'\0',sizeof(begit));
         memset(endit,'\0',sizeof(endit));

         endit[0]= '*';
         begit[0]= '*';
         begit[1]= '*';
         if( L >= 1 )
         {
           endit[0]= item[L-1];

           if( L >= 2 )
           {
             begit[0]= item[0];
             begit[1]= item[1];
           }
         }

         if(    compare(begex,begbug,2) == 0
                &&
                compare(endex,endbug,1) == 0
           )
         {
           printf(" %s..%s bFound(%d,%s) eFound(%d,%s) "
                  "%4d:%3d [%4d..%4d] '%s'\n"
                  , begex, endex
                  , bFound, begit
                  , eFound, endit
                  , row, i, col-L, col, item);
         }
       }}
       #endif
     }
   }

   printf("   %s..%s   %6d [%6d .. %6d]\n",
          endex, begex, mCount, eCount, bCount);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       head2
//
// Function-
//       Heading for phase1 scan.
//
//----------------------------------------------------------------------------
static void
   head2( void )                    // Display heading
{
   printf("\n");
   printf("Phase 2 scan:\n"
          "\t   End: Ending exon sequence\n"
          "\t    ..: The Intron sequence\n"
          "\t   Beg: Beginning exon sequence\n"
          "\tmatchs: Number of exons matching the combined phase 2 sequence\n"
          "\tfinals: Number of exons matching the ending phase 2 sequence\n"
          "\tfirsts: Number of exons matching the beginning phase 2 sequence\n"
          "\n"
          " End..Beg  matchs [finals .. firsts]\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan2
//
// Function-
//       Scan the database, looking for phase2 exon splits.
//
//----------------------------------------------------------------------------
static void
   scan2(                           // Scan the data table
     const char*       exon)        // Exon
{
   int                 const m= eidb.getLineCount();

   char                endex[3];    // Ending exon sequence
   char                begex[2];    // Beginning exon sequence
   int                 bCount;      // Number of beginning matches
   int                 bFound;      // Beginning match on this item?
   int                 col;         // Current origin column
   const char*         item;        // Current exon or intron
   int                 eCount;      // Number of ending matches
   int                 eFound;      // Ending match on this item?
   int                 mCount;      // Number of matches
   int                 L;           // Item length
   int                 row;         // Row index

   int                 i;

   // Initialize
   mCount= 0;
   bCount= 0;
   eCount= 0;

   endex[0]= exon[0];
   endex[1]= exon[1];
   endex[2]= '\0';
   begex[0]= exon[2];
   begex[1]= '\0';

   // Scan the database
   for(row= 0; row<m; row++)
   {
     col= 0;
     eFound= 0;
     for(i=0;;i++)
     {
       item= (char*)list[row].getItem(i);
       if( item == NULL )
         break;

       L= strlen(item);
       bFound= 0;

       // Count starting item
       if( i > 0                    // Don't count the first item
           && (col%3) == 2          // Don't count if not phase 2 start
           && L >= 1                // Don't count if too small
           && compare(item, begex, 1) == 0 ) // Don't count mismatch
         bFound= 1;                 // Found match
       bCount += bFound;

       // Check for match
       if( bFound != 0 && eFound != 0 )
         mCount++;

       // Count ending item
       eCount += eFound;
       eFound= 0;
       col += L;                    // Position at ending column
       if( (col%3) == 2             // Don't count if not phase 2 end
           && L >= 2                // Don't count if too small
           && compare(item+L-2, endex, 2) == 0 ) // Don't count mismatch
         eFound= 1;                 // Found match

       #if 0                        // Phase 1 HCDM
       {{
         char        begit[8];
         char        endit[8];

         const char* begbug= "G";
         const char* endbug= "GG";

         memset(begit,'\0',sizeof(begit));
         memset(endit,'\0',sizeof(endit));

         endit[0]= '*';
         endit[1]= '*';
         begit[0]= '*';
         if( L >= 1 )
         {
           begit[0]= item[0];

           if( L >= 2 )
           {
             endit[0]= item[L-2];
             endit[1]= item[L-1];
           }
         }

         if(    compare(begex,begbug,1) == 0
                &&
                compare(endex,endbug,2) == 0
           )
         {
           printf(" %s..%s bFound(%d,%s) eFound(%d,%s) "
                  "%4d:%3d [%4d..%4d] '%s'\n"
                  , begex, endex
                  , bFound, begit
                  , eFound, endit
                  , row, i, col-L, col, item);
         }
       }}
       #endif
     }
   }

   printf("  %s..%s    %6d [%6d .. %6d]\n",
          endex, begex, mCount, eCount, bCount);
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
   int                 i, j;

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
   head0();
   for(i=0; i<MAX_CODON; i++)
     for(j=0; j<MAX_CODON; j++)
       scan0(codon[i], codon[j]);   // Phase 0 scan

   head1();
   for(i=0; i<MAX_CODON; i++)
     scan1(codon[i]);               // Phase 1 scan

   head2();
   for(i=0; i<MAX_CODON; i++)
     scan2(codon[i]);               // Phase 2 scan

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   term();
   return 0;
}

