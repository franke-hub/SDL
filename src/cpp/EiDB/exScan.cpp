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
//       exScan.cpp
//
// Purpose-
//       Exon/Intron Exon DataBase scanner.
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
//       to control the exact format of the in-storage database.  Different
//       types of Accumulator and Extractor objects are used to control the
//       database loading, and are selected by program option controls.
//
//       By default this routine in an Exon scanner.  When compiled with
//       the INTRON_SCANNER flag set it becomes an Intron scanner.  The
//       associated Makefile compiles this routine each way so that this
//       source compiles into both exScan and inScan.
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
#define __SOURCE__         "inScan" // Source file
#else
#define __SOURCE__         "exScan" // Source file
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

static int*            colArray;    // Column array
static const char**    txtArray;    // Text array

static int             colZero;     // Column number zero
static int             fileName;    // The fileName parameter index
static int             maxSize;     // MaxSize parameter
static int             minSize;     // MinSize parameter
static int             sw_verify;   // Verify column, if specified

static char            sw_verbose;  // TRUE iff -v     option specified

static char            sw_atg;      // TRUE iff -atg   option specified
static char            sw_first;    // TRUE iff -first option specified
static char            sw_last;     // TRUE iff -last  option specified
static char            sw_only;     // TRUE iff -only  option specified
static char            sw_out;      // TRUE iff -out   option specified
static char            sw_rev;      // TRUE iff -rev   option specified
static char            sw_sum;      // TRUE iff -sum   option specified
static char            sw_union;    // TRUE iff -union option specified
static char            sw_wild;     // TRUE iff -wild  option specified

#ifdef INTRON_SCANNER
static const char*     acgt= "acgt";
#else
static const char*     acgt= "ACGT";
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
   fprintf(stderr,"Usage: %s ", __SOURCE__);
   fprintf(stderr,"<-options> filename <sequence ...>");
   fprintf(stderr,"\n");
   fprintf(stderr,"Exon/Intron %s database scanner.\n", Exon_Intron);
   fprintf(stderr,"Scan an EiDB database file looking for patterns.\n");
   fprintf(stderr,"\n\n");
   fprintf(stderr,"filename\n"
                  "\tThe name of the EiDB format database file.\n");
   fprintf(stderr,"sequence\n"
                  "\tA set of (%s case) search sequences.\n", UPPER_lower);

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
   fprintf(stderr,"-verify:column\n"
                  "\tVerify results for column using alternative check.\n");
   fprintf(stderr,"-out\n"
                  "\tDisplay the database, as loaded.\n");
   fprintf(stderr,"-rev\n"
                  "\tUse right adjustment.\n"
                  "\tNote: When using right adjustment, column number 1 is\n"
                  "\tconsidered the right-most column.\n");
   fprintf(stderr,"-sum\n"
                  "\tDisplay column summaries.\n");
   fprintf(stderr,"-union\n"
                  "\tCombine all %ss within a sequence.\n",
                  Exon_Intron);
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
   sw_last=    FALSE;
   sw_only=    FALSE;
   sw_out=     FALSE;
   sw_rev=     FALSE;
   sw_sum=     FALSE;
   sw_union=   FALSE;
   sw_wild=    FALSE;

   minSize=    (-1);
   maxSize=    (-1);
   sw_verify=  (0);

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

       else if( memcmp("-verify:", argv[j], 8) == 0 )
       {
         sw_verify= atol(argv[j]+8);
         if( sw_verify < colZero )
         {
           error= TRUE;
           fprintf(stderr, "Invalid column '%d'\n", sw_verify);
         }
       }

       else if( strcmp("-out", argv[j]) == 0 )
         sw_out= TRUE;

       else if( strcmp("-rev", argv[j]) == 0 )
         sw_rev= TRUE;

       else if( strcmp("-sum", argv[j]) == 0 )
         sw_sum= TRUE;

       else if( strcmp("-wild", argv[j]) == 0 )
         sw_wild= TRUE;

       else if( strcmp("-union", argv[j]) == 0 )
         sw_union= TRUE;

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
//       showRow
//
// Function-
//       Show a row (for debugging)
//
//----------------------------------------------------------------------------
static inline void
   showRow(                         // Display a row
     int               row)         // Element index
{
   printf("%6d: %6d: '%s'\n", row, colArray[row], txtArray[row]);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       swap
//
// Function-
//       Swap column array elements
//
//----------------------------------------------------------------------------
static inline void
   swap(                            // Swap array elements
     int               i,           // Element index
     int               j)           // Element index
{
   int                 iTemp;
   const char*         pTemp;

   #ifdef HCDM
     printf("swap(%d,%d)\n", i, j);
     showRow(i);
     showRow(j);
   #endif

   iTemp= colArray[i];
   pTemp= txtArray[i];
   colArray[i]= colArray[j];
   txtArray[i]= txtArray[j];
   colArray[j]= iTemp;
   txtArray[j]= pTemp;
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
   Accumulator*        accumulator;
   Extractor*          extractor;

   const char*         C;
   int                 rc;
   int                 row;
   EiDBLoader::LOADMODE
                       mode;

   int                 i;
   int                 m;

   // Set Accumuilator
   if( sw_union )
   {
     #ifdef INTRON_SCANNER
       accumulator= new IntronAccumulator();
     #else
       accumulator= new ExonAccumulator();
     #endif
   }
   else
     accumulator= new DataAccumulator();

   // Set Extractor
   #ifdef INTRON_SCANNER
     extractor= new IntronExtractor();

   #else
     if( sw_atg )
       extractor= new AtgExtractor(sw_wild);
     else
       extractor= new ExonExtractor();
   #endif

   // Load the data
   if( accumulator->open(fileName) != 0 )
     exit(EXIT_FAILURE);

   mode= loader.MODE_LEFTRIGHT;
   if( sw_rev == TRUE )
     mode= loader.MODE_RIGHTLEFT;

   rc= loader.load(eidb, *accumulator, *extractor, mode);
   if( rc < 0 )
     exit(EXIT_FAILURE);

   if( rc > 0 )
     fprintf(stderr, "%s loaded with errors\n", fileName);

   // Delete the work areas
   delete extractor;
   delete accumulator;

   // Allocate the auxiliary array
   colArray= NULL;
   txtArray= NULL;
   for(i= 0; (colArray == NULL || txtArray == NULL); i++)
   {
     if( colArray == NULL )
       colArray= (int*)malloc(eidb.getLineCount()*sizeof(int));
     if( txtArray == NULL )
       txtArray= (const char**)malloc(eidb.getLineCount()*sizeof(char*));
     if( colArray == NULL || txtArray == NULL )
     {
       if( i== 0 )
         fprintf(stderr, "No storage, removing some EiDB lines\n");
       eidb.trim();

       if( colArray != NULL )
         free(colArray);
       if( txtArray != NULL )
         free(txtArray);

       colArray= NULL;
       txtArray= NULL;
     }
   }

   if( sw_out )
   {
     m= eidb.getLineCount();
     for(row= 0; row<m; row++)
     {
       C= eidb.getLine(row);
       txtArray[row]= C;
       colArray[row]= strlen(C);
     }

     #ifdef SORT
       for(row= 0; row<m; row++)
       {
         if( (row%1000) == 0 )
           fprintf(stderr, "Sorting: %6d of %6d\n", row, m);
         for(i= row+1; i<m; i++)
         {
           if( colArray[row] < colArray[i] )
             swap(i, row);
         }
       }
     #endif

     for(row= 0; row<m; row++)
     {
       printf("%6d: '%s'\n", row, txtArray[row]);
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scan
//
// Function-
//       Scan the database.
//
// Notes-
//       Row #                colArray
//       -----                --------
//       0                    ---------------------
//       :                    strlen(row)
//       inpLower             ---------------------
//       :                    offset(next match)
//       inpFirst             ---------------------
//       :                    offset(first match)
//       inpCount             ---------------------
//       :                    strlen(row)
//       eidb.getLineCount()  ---------------------
//
//----------------------------------------------------------------------------
static void
   scan(                            // Scan the data table
     int               argc,        // Argument count
     char*             argv[])      // Argument vector
{
   int                 const m= eidb.getLineCount();

   const char*         C;           // -> Line
   unsigned            ACount, CCount, GCount, TCount; // Accumulators
   unsigned            oCount;      // (Error) accumulator
   int                 col;         // Column index
   int                 firsts;      // Number of first occurrances
   int                 inpCount;    // Number of input elements
   int                 inpFirst;    // Lower index of valid first elements
   int                 inpLower;    // Lower index of valid elements
   int                 item;        // Item index
   int                 L;           // Line length
   int                 lCounter;    // Valid line counter
   int                 row;         // Row index
   const char*         S;           // -> Line substring
   int                 totals;      // Total number of occurrances
   const char*         W;           // -> Wild character

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

     if( sw_union )
       printf("          -union: YES. All %ss in a sequence are combined.\n",
              Exon_Intron);
     else
       printf("          -union:  NO. %ss sequences within a gene are "
                                      "separated.\n",
              Exon_Intron);

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

   // Skip summary if not wanted
   if( !sw_sum )
     goto itemScan;

   // Create the initial column table
   inpLower= 0;
   inpFirst= 0;
   inpCount= m;
   for(row= 0; row<m; row++)
   {
     C= eidb.getLine(row);
     txtArray[row]= C;
     colArray[row]= strlen(C);
   }

   // Totals counter
   ACount= 0;
   CCount= 0;
   GCount= 0;
   TCount= 0;
   oCount= 0;
   for(row= 0; row<m; row++)
   {
     C= eidb.getLine(row);
     while( *C != '\0' )
     {
       if( *C == acgt[0] )
         ACount++;

       else if( *C == acgt[1] )
         CCount++;

       else if( *C == acgt[2] )
         GCount++;

       else if( *C == acgt[3] )
         TCount++;

       else
       {
         oCount++;
         if( sw_wild )
         {
           W= getWild(*C);
           if( W != NULL )
           {
             while( *W != '\0' )
             {
               if( *W == acgt[0] )
                 ACount++;

               else if( *W == acgt[1] )
                 CCount++;

               else if( *W == acgt[2] )
                 GCount++;

               else if( *W == acgt[3] )
                 TCount++;

               W++;
             }
           }
         }
       }

       C++;
     }
   }
   printf("     Column -------%c's -------%c's -------%c's -------%c's "
                      "----Others\n",
          acgt[0], acgt[1], acgt[2], acgt[3]);
   printf("      Total %10u %10u %10u %10u %10u\n",
          ACount, CCount, GCount, TCount, oCount);

   // Column counter
   for(col= 0; col<eidb.getLargest(); col++)
   {
     ACount= 0;
     CCount= 0;
     GCount= 0;
     TCount= 0;
     oCount= 0;
     for(row= inpLower; row<inpCount; row++)
     {
       C= txtArray[row];
       L= colArray[row];
       if( col >= L )
       {
         swap(row, inpLower);
         inpLower++;
       }
       else
       {
         C += col;
         if( *C == acgt[0] )
           ACount++;

         else if( *C == acgt[1] )
           CCount++;

         else if( *C == acgt[2] )
           GCount++;

         else if( *C == acgt[3] )
           TCount++;

         else
         {
           oCount++;
           if( sw_wild )
           {
             W= getWild(*C);
             if( W != NULL )
             {
               while( *W != '\0' )
               {
                 if( *W == acgt[0] )
                   ACount++;

                 else if( *W == acgt[1] )
                   CCount++;

                 else if( *W == acgt[2] )
                   GCount++;

                 else if( *W == acgt[3] )
                   TCount++;

                 W++;
               }
             }
           }
         }
       }
     }
     printf(" %10u %10u %10u %10u %10u %10u\n", col + colZero,
            ACount, CCount, GCount, TCount, oCount);
   }

   // Scan items
itemScan:
   for(item= fileName+1; item<argc; item++)
   {
     printf("\n");
     printf("Sequence '%s'\n", argv[item]);
     L= strlen(argv[item]);

     // Create the initial column table
     inpLower= 0;
     inpFirst= 0;
     inpCount= 0;
     for(row= 0; row<m; row++)
     {
       C= eidb.getLine(row);
       if( sw_wild )
         S= wildstr(C, argv[item]);
       else
         S= strstr(C, argv[item]);
       txtArray[row]= C;
       colArray[row]= strlen(C);

       if( S != NULL )
       {
         swap(row, inpCount);
         colArray[inpCount]= S - C;
         txtArray[inpCount]= C;
         inpCount++;
       }
     }
     if( inpCount == 0 )
     {
       printf("  (Does not occur)\n");
       continue;
     }

     if( sw_verify != 0 )
     {
       printf("     Column      First      Total      Count\n");
       lCounter= 0;
       firsts= 0;
       totals= 0;
       for(row= 0; row<m; row++)
       {
         C= eidb.getLine(row);
         if( strlen(C) >= (sw_verify+L-colZero) )
         {
           lCounter++;
           if( (memcmp(C+sw_verify-colZero, argv[item], L) == 0)
               ||(sw_wild && (wildseg(C+sw_verify-colZero, argv[item]) == 0)) )
           {
             totals++;
             if( sw_wild )
               S= wildstr(C, argv[item]);
             else
               S= strstr(C, argv[item]);
             if( (S-C+colZero) == sw_verify )
               firsts++;
           }
         }
       }
       printf(" %10u %10u %10u %10u\n\n",
              sw_verify, firsts, totals, lCounter);
     }

     printf("     Column      First      Total      Count\n");
     for(;;)
     {
       #ifdef HCDM
         printf("\n");
         printf("inpLower(%d) inpFirst(%d) inpCount(%d) inpTotal(%d)\n",
                inpLower, inpFirst, inpCount, m);
         for(row= 0; row<m; row++)
           showRow(row);
       #endif

       if( inpLower >= inpCount )
         break;

       col= eidb.getLargest();
       firsts= 0;
       totals= 0;
       for(row= inpLower; row<inpCount; row++)
       {
         if( colArray[row] < col )
         {
           col= colArray[row];
           firsts= 0;
           totals= 0;
         }

         if( col == colArray[row] )
         {
           totals++;
           if( row >= inpFirst )
             firsts++;
         }
       }
       lCounter= inpCount - inpLower;
       for(row= 0; row<inpLower; row++)
       {
         if( colArray[row] >= col+L )
           lCounter++;
       }
       for(row= inpCount; row<m; row++)
       {
         if( colArray[row] >= col+L )
           lCounter++;
       }

       printf(" %10u %10u %10u %10u\n",
              col + colZero, firsts, totals, lCounter);
       for(row= inpLower; row<inpCount; row++)
       {
         if( col == colArray[row] )
         {
           #ifdef HCDM
             printf("Match Row(%d) Col(%d) inpFirst(%d) inpLower(%d)\n",
                     row, col, inpFirst, inpLower);
             showRow(row);
           #endif
           C= txtArray[row];
           if( sw_wild )
             S= wildstr(C+col+1, argv[item]);
           else
             S= strstr(C+col+1, argv[item]);
           if( S != NULL )
           {
             colArray[row]= S - C;
             if( row > inpFirst )
               swap(row, inpFirst);
           }
           else
           {
             colArray[row]= strlen(txtArray[row]);
             if( row > inpFirst )
             {
               swap(row, inpFirst);
               if( inpFirst > inpLower )
                 swap(inpFirst, inpLower);
             }
             else if( row > inpLower )
               swap(row, inpLower);

             inpLower++;
           }

           if( row >= inpFirst )
             inpFirst++;
         }
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

