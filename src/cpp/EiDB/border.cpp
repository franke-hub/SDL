//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       border.cpp
//
// Purpose-
//       Exon/Intron DataBase scanner.
//
// Last change date-
//       2003/10/19
//
// Description-
//       This routine examines an Exon/Intron database file, looking for
//       particular sequences of exons which surround an intron.  It does
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
//       This routine compiles into two modules, border and border3, based
//       upon the setting of the SEPARATE_BY_PHASE compile time parameter.
//       The SEPARATE_BY_PHASE generated module displays the results by
//       phase.  The default module summarizes the result.
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
#error "INTRON_SCANNER defined, not supported"
#define __SOURCE__        "INVALID" // Source file
#else
#define __SOURCE__         "border" // Source file
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

static char            sw_all16;    // TRUE iff -all16 option specified
static char            sw_all64;    // TRUE iff -all64 option specified
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
static const char*     ntide16[16]= { // Nucleotide sequences
   "AA",                            // 00
   "AC",                            // 01
   "AG",                            // 02
   "AT",                            // 03
   "CA",                            // 04
   "CC",                            // 05
   "CG",                            // 06
   "CT",                            // 07
   "GA",                            // 08
   "GC",                            // 09
   "GG",                            // 10
   "GT",                            // 11
   "TA",                            // 12
   "TC",                            // 13
   "TG",                            // 14
   "TT"};                           // 15

static const char*     ntide64[64]= { // Nucleotide sequences
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
   fprintf(stderr,"-all16\n"
                  "\tUse all possible nucleotide pair sequences.\n");
   fprintf(stderr,"-all64\n"
                  "\tUse all possible nucleotide triplet sequences.\n");
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
static void
   parm(                            // Analyze parameters
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
{
   int                 error;       // Error switch

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   sw_verbose=  TRUE;               // Default switch settings
   sw_all16=   FALSE;
   sw_all64=   FALSE;
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

       else if( strcmp("-all16", argv[j]) == 0 )
         sw_all16= TRUE;

       else if( strcmp("-all64", argv[j]) == 0 )
         sw_all64= TRUE;

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
   init(int, char**)                // Initialize
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   list= NULL;                      // No list exists

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
   for(unsigned row= 0; row<eidb.getLineCount(); row++)
      list[row].empty();
   free(list);

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
   EiDBLoader::LOADMODE
                       loadMode;    // Loading mode

   int                 rc;
   int                 i;

   // Load the items
   accumulator= new DataAccumulator();
   if( accumulator->open(fileName) != 0 )
     exit(EXIT_FAILURE);

   loadMode= loader.MODE_LEFTRIGHT;
   if( sw_rev == TRUE )
     loadMode= loader.MODE_RIGHTLEFT;

   rc= loader.load(eidb, *accumulator, fullExtractor, loadMode);
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
   for(unsigned row= 0; row<eidb.getLineCount(); row++)
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
     for(unsigned row= 0; row<eidb.getLineCount(); row++)
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
//       head
//
// Function-
//       Heading for scan.
//
//----------------------------------------------------------------------------
static void
   head( void )                     // Display heading
{
   printf("\n");
   printf("Nucleotide scan:\n"
          #ifdef SEPARATE_BY_PHASE
            "\t Phase: Phase under consideration\n"
          #endif
          "\t   End: Ending exon nucleotide sequence\n"
          "\t    ..: The Intron sequence\n"
          "\t   Beg: Beginning exon nucleotide sequence\n"
          "\tmatchs: Number of nucleotides matching the combined sequence\n"
          "\tfinals: Number of nucleotides matching the ending sequence\n"
          "\tfirsts: Number of nucleotides matching the beginning sequence\n"
          "\n"

          #ifdef SEPARATE_BY_PHASE
            "Phase "
          #endif
          " End ..  Beg matchs [finals .. firsts]\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan
//
// Function-
//       Scan the database, looking for exon splits.
//
//----------------------------------------------------------------------------
static void
   scan(                            // Scan the data table
     const char*       endide,      // Ending nucleotide
     const char*       begide)      // Beginning nucleotide
{
   int                 const m= eidb.getLineCount();
   int                 const sizeEnd= strlen(endide);
   int                 const sizeBeg= strlen(begide);

   int                 bCount;      // Number of beginning matches
   int                 bFound;      // Beginning match on this item?
   int                 eCount;      // Number of ending matches
   int                 eFound;      // Ending match on this item?
   const char*         item;        // Current exon or intron
   int                 mCount;      // Number of matches
   int                 phaseHave;   // Phase of the current row
   int                 phaseWant;   // Phase under consideration
   int                 L;           // Item length
   int                 row;         // Row index
   const char*         showEnd;     // For display
   const char*         showBeg;     // For display
   char                workEnd[5];  // For display
   char                workBeg[5];  // For display

   int                 i;

   // Phase control
   phaseWant= 0;                    // For GCC, when not SEPARATE_BY_PHASE

   #ifdef SEPARATE_BY_PHASE
     for(phaseWant= 0; phaseWant<3; phaseWant++)
     {
       printf("  [%d] ", phaseWant);
   #endif

       // Initialize
       mCount= 0;
       bCount= 0;
       eCount= 0;

       // Scan the database
       for(row= 0; row<m; row++)
       {
         phaseHave= 0;

         eFound= 0;
         for(i=0;;i++)
         {
           item= (char*)list[row].getItem(i);
           if( item == NULL )
             break;

           L= strlen(item);
           bFound= 0;

           // Count starting item
           if( i > 0                // Don't count the first item
               && L >= sizeBeg      // Don't count if too small
               && compare(item, begide, sizeBeg) == 0 ) // Don't count mismatch
           {
             #ifdef SEPARATE_BY_PHASE
               if( phaseHave == phaseWant )
             #endif
                 bFound= 1;         // Found match
           }
           bCount += bFound;

           // Check for match
           if( bFound != 0 && eFound != 0 )
             mCount++;

           // Count ending item
           eCount += eFound;
           eFound= 0;
           if( L >= sizeEnd         // Don't count if too small
               && compare(item+L-sizeEnd, endide, sizeEnd) == 0 )
                                    // Don't count mismatch
           {
             #ifdef SEPARATE_BY_PHASE
               if( phaseHave == phaseWant )
             #endif
                 eFound= 1;         // Found match
           }

           #ifdef SEPARATE_BY_PHASE
             phaseHave += L;
             phaseHave %= 3;
           #endif
         }
       }

       showEnd= endide;
       if( strlen(endide) < sizeof(workEnd) )
       {
         memset(workEnd, ' ', sizeof(workEnd));
         strcpy(workEnd + sizeof(workEnd) - strlen(endide) - 1, endide);
         showEnd= workEnd;
       }

       showBeg= begide;
       if( strlen(begide) < sizeof(workBeg) )
       {
         memset(workBeg, ' ', sizeof(workBeg));
         strcpy(workBeg + sizeof(workBeg) - strlen(begide) - 1, begide);
         showBeg= workBeg;
       }

       printf("%s .. %s %6d [%6d .. %6d]\n",
              showEnd, showBeg, mCount, eCount, bCount);
   #ifdef SEPARATE_BY_PHASE
     }
   #else
     if( false )
       printf("%d", phaseWant + phaseHave); // Make compiler happy
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       split
//
// Purpose-
//       Split then scan a parameter argument.
//
//----------------------------------------------------------------------------
static void                     
   split(                           // Split then scan
     const char*       string)      // Argument
{
   char*               C;           // Working char*
   char                work[4096];  // Working argument

   if( strlen(string) >= sizeof(work) )
   {
     fprintf(stderr, "Argument(%s) too long, maximum(%zd)\n",
             string, sizeof(work));
     return;
   }

   strcpy(work, string);
   C= strchr(work, '.');
   if( C == NULL )
   {
     fprintf(stderr, "Argument(%s) missing '.' delimiter\n", string);
     return;
   }

   *C= '\0';
   C++;

   if( work[0] == '\0' || *C == '\0' )
   {
     fprintf(stderr, "Argument(%s) invalid syntax\n", string);
     return;
   }

   scan(work, C);
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
   head();
   if( sw_all16 )
   {
     for(i=0; i<16; i++)
       for(j=0; j<16; j++)
         scan(ntide16[i], ntide16[j]);
   }

   if( sw_all64 )
   {
     for(i=0; i<64; i++)
       for(j=0; j<64; j++)
         scan(ntide64[i], ntide64[j]);
   }

   for(i= fileName+1; i<argc; i++)
     split(argv[i]);

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   term();
   return 0;
}

