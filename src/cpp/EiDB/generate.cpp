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
//       generate.cpp
//
// Purpose-
//       Sequence generator.
//
// Last change date-
//       2003/03/01
//
// Usage-
//       generate <options> filename ...
//
// Inputs-
//       File "filename" contains the probability table.
//
// Outputs-
//       (stdout)
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <Random.h>
#include <Reader.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "GENERATE" // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_LINE         0x00100000 // Maximum size of line
#define MAX_LIST            1000000 // The number of string vectors
#define LIST_DIGITS               6 // The number of digits in MAX_LIST

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             fileArgx;    // Input filename argument index
static int             swSHOW=  FALSE; // Display each gene?

#ifdef HCDM
static int             swHCDM=  TRUE;
#else
static int             swHCDM= FALSE;
#endif

#ifdef SCDM
static int             swSCDM=  TRUE;
#else
static int             swSCDM= FALSE;
#endif

static int             geneCount= 4;// Number of genes to generate
static char**          gList;       // Gene array
static int*            gSize;       // Gene size array
static char*           iLine;       // Input data line
static int             initSeed;    // Initial randomizing seed
static const char*     inpName= NULL; // Input filename
static const char*     outName= NULL; // Output filename
static int             peakMin;     // Minimum gene size control
static int             peakMax;     // Nominal gene size control
static double          peakScale;   // Nominal gene size control
static char**          pList;       // Probability string vector
static int             pTotal[3];   // Totals, by phase
static double          pWeight[3];  // Weighted totals, by phase
static int             swSALL;      // TRUE iff -symmshow
static int             swSTOP;      // Valueof(-stop:) parameter
static int             swSYMM;      // TRUE iff -symmetric or -symmshow
static int             useWeights= FALSE; // TRUE iff using weights

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Skip blank characters
//
//----------------------------------------------------------------------------
static char*                        // The next non-blank
   skipBlank(                       // Skip blanks in string
     char*               string)    // The string
{
   while( *string == ' ' )
     string++;

   return string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strip
//
// Purpose-
//       Strip leading and trailing blanks
//
//----------------------------------------------------------------------------
static char*                        // First non-blank
   strip(                           // Strip leading and trailing blanks
     char*               string)    // The string
{
   int                   L= strlen(string);

   while( L > 0 && string[L-1] == ' ' )
   {
     L--;
     string[L]= '\0';
   }

   return skipBlank(string);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initTotals
//
// Purpose-
//       Initialize total values
//
//----------------------------------------------------------------------------
static void
   initTotals( void )               // Initialize totals
{
   pTotal[0]= 0;
   pTotal[1]= 0;
   pTotal[2]= 0;

   pWeight[0]= 0.0;
   pWeight[1]= 0.0;
   pWeight[2]= 0.0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       termTotals
//
// Purpose-
//       Display totals
//
//----------------------------------------------------------------------------
static void
   termTotals( void )               // Display totals
{
   int                 sigma;       // Total match count
   double              total;       // Weighted total

   int                 i;

   total= pWeight[0] + pWeight[1] + pWeight[2];
   if( useWeights && total == 0.0 )
     total= 1.0;

   printf("\n");
   printf("All string totals:\n");
   sigma= pTotal[0] + pTotal[1] + pTotal[2];
   if( sigma == 0 )
     printf("  No matches\n");
   else
   {
     for(i= 0; i<3; i++)
     {
       printf("Phase[%d] %8d  (%6.2f%%)",
              i, pTotal[i], 100.0 * (double)pTotal[i] / (double)sigma);
       if( useWeights )
         printf("   Weighted %11.2f  (%6.2f%%)",
         pWeight[i], 100.0 * pWeight[i] / total);
       printf("\n");
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialization processing
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   iLine= (char*) malloc(MAX_LINE * sizeof(char));
   assert( iLine != NULL );

   gList= (char**)malloc(geneCount * sizeof(char*));
   assert( gList != NULL );
   memset(gList, 0, geneCount * sizeof(char*));

   gSize= (int*)  malloc(geneCount * sizeof(int));
   assert( gSize != NULL );
   memset(gSize, 0, geneCount * sizeof(int));

   pList= (char**)malloc(MAX_LIST * sizeof(char*));
   assert( pList != NULL );
   memset(pList, 0, MAX_LIST * sizeof(char*));

   Random::setSeed(initSeed);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Termination processing
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   free(iLine);
   free(gList);
   free(gSize);
   free(pList);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*       sourceName)  // The source fileName
{
   fprintf(stderr, "%s <options> filename scan.item ...\n", sourceName);
   fprintf(stderr, "Generate and analyze pseudo-genes\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Inputs:\n");
   fprintf(stderr, "\t\"filename\" contains descriptor data.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Outputs:\n");
   fprintf(stderr, "\t(stdout)\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-gene:n\tGenerate n genes\n");
   fprintf(stderr, "-output:fileName\tWrite random data to fileName\n");
   fprintf(stderr, "-peakmax:n\tSize of gene always accepted\n");
   fprintf(stderr, "-peakmin:n\tSize of gene always rejected\n");
   fprintf(stderr, "-peakscale:n\tGene acceptance scale factor\n");
   fprintf(stderr, "-seed:n\tSet initial randomizing seed\n");
   fprintf(stderr, "-show\tDisplay generated genes\n");
   fprintf(stderr, "-stop:n\tStop after n sequences (ignoring stops)\n");
   fprintf(stderr, "-symmetric\tCut generated genes, check symmetry\n");
   fprintf(stderr, "-symmscdm\t-symmetric + show individual resultants\n");
   fprintf(stderr, "-v\tverify data\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Debugging options:\n");
   fprintf(stderr, "-hcdm\tDisplay intermediate data\n");
   fprintf(stderr, "-scdm\tDisplay most internal function calls\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Example:\n");
   fprintf(stderr, "  %s -gene:32 generate.dat AG.GT .CAAT GAA.T\n",
                   sourceName);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   int                 argx;        // Argument index
   int                 error;       // TRUE if error encountered
   int                 verify;      // TRUE if verify required

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   verify= FALSE;
   initSeed= time(NULL);
   peakMax= 0;
   peakMin= 0;
   peakScale= 2.0;

   swSALL= FALSE;
   swSYMM= FALSE;
   swSTOP= 0;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(argx=1; argx<argc; argx++)   // Examine the parameter list
   {
     if( *argv[argx] == '-' )       // If this is a switch list
     {
       if( strcmp(argv[argx], "-help") == 0 )
         error= TRUE;
       else if( strcmp(argv[argx], "-show") == 0 )
         swSHOW= TRUE;
       else if( memcmp(argv[argx], "-gene:", 6) == 0 )
         geneCount= atol(argv[argx] + 6);
       else if( memcmp(argv[argx], "-output:", 8) == 0 )
         outName= argv[argx] + 8;
       else if( memcmp(argv[argx], "-peakmax:", 9) == 0 )
         peakMax= atol(argv[argx] + 9);
       else if( memcmp(argv[argx], "-peakmin:", 9) == 0 )
         peakMin= atol(argv[argx] + 9);
       else if( memcmp(argv[argx], "-peakscale:", 11) == 0 )
         peakScale= atof(argv[argx] + 11);
       else if( memcmp(argv[argx], "-seed:", 6) == 0 )
         initSeed= atol(argv[argx] + 6);
       else if( memcmp(argv[argx], "-stop:", 6) == 0 )
         swSTOP= atol(argv[argx] + 6);
       else if( strcmp(argv[argx], "-symmetric") == 0 )
         swSYMM= TRUE;
       else if( strcmp(argv[argx], "-symmscdm") == 0 )
       {
         swSALL= TRUE;
         swSYMM= TRUE;
       }
       else if( strcmp(argv[argx], "-hcdm") == 0 )
         swHCDM= TRUE;
       else if( strcmp(argv[argx], "-scdm") == 0 )
         swSCDM= TRUE;
       else if( strcmp(argv[argx], "-v") == 0 )
         verify= TRUE;
       else
       {
         error= TRUE;
         fprintf(stderr, "Invalid option '%s'\n", argv[argx]);
       }
       continue;
     }
     else                           // File name/scan parameter
     {
       if( inpName == NULL )
       {
         inpName= argv[argx];
         fileArgx= argx;
       }
       else
       {
         if( strstr(argv[argx], ":") != NULL )
           useWeights= TRUE;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Validate positional parameters
   //-------------------------------------------------------------------------
   if( peakMin < 0 )
   {
     error= TRUE;
     fprintf(stderr, "-peakMin(%d) must be positive\n", peakMax);
   }
   if( peakMax < peakMin )
   {
     error= TRUE;
     fprintf(stderr, "-peakMax(%d) < -peakMin(%d)\n", peakMax, peakMin);
   }

   if( inpName == NULL )
   {
     error= TRUE;
     fprintf(stderr, "No files specified\n");
   }

   if( swSTOP < 0 || swSTOP == 1 )
   {
     error= TRUE;
     fprintf(stderr, "-stop(%d) must be more than(1)\n", swSTOP);
   }
   if( swSTOP >= (MAX_LINE/3) )
   {
     error= TRUE;
     fprintf(stderr, "-stop(%d) must be less than(%d)\n", swSTOP, MAX_LINE/3);
   }

   if( error )
     info(argv[0]);

   if( verify )
   {
     printf("       File: %s\n", inpName);
     printf("      -gene: %d\n", geneCount);
     printf("    -output: %s\n", outName == NULL ? "<not specified>" : outName);
     printf("   -peakmax: %d\n", peakMax);
     printf("   -peakmin: %d\n", peakMin);
     printf(" -peakscale: %f\n", peakScale);
     printf("      -seed: %d\n", initSeed);
     printf("      -show: %s\n", swSHOW ? "TRUE" : "FALSE");
     printf("      -stop: %d\n", swSTOP);
     printf(" -symmetric: %s\n", swSYMM ? "TRUE" : "FALSE");
     printf("      -hcdm: %s\n", swHCDM ? "TRUE" : "FALSE");
     printf("      -scdm: %s\n", swSCDM ? "TRUE" : "FALSE");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       rdError
//
// Purpose-
//       Indicate Reader format error.
//
//----------------------------------------------------------------------------
static int                          // Return code (always 1)
   rdError(                         // Indicate Reader format error
     Reader&           reader,      // The Reader
     const char*       fmt,         // PRINTF string
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer
   int                 line;        // Line number

   line= reader.getLine();
   fprintf(stderr, "File(%s) Line(%4d): ", reader.getFilename(), line);

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions
   fprintf(stderr, "\n");

   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseValue
//       Leading blanks are skipped.
//
// Purpose-
//       Parse a string, extracting a probability value.
//
// Returns-
//       Return (value, range 1..9999)
//       String (The value delimiter)
//
//----------------------------------------------------------------------------
long                                // Return value
   parseValue(                      // Extract probability value from string
     char*&            C)           // -> String (updated)
{
   long                result;      // Resultant
   long                digits;      // Number of digits

   C= skipBlank(C);                 // Skip leading blanks

   digits= 0;
   result= 0;
   for(;;)                          // Find the decimal point
   {
     if( *C == '.' )                // If decimal point
     {
       C++;
       break;
     }

     if( *C != '0' )                // If invalid character
       return (-1);                 // Return, invalid syntax

     C++;
   }

   for(;;)
   {
     if( digits >= LIST_DIGITS )    // If too many digits
       break;

     if( *C < '0' || *C > '9' )     // If invalid numeric character
       break;                       // Done

     result *= 10;
     result += *C - '0';
     C++;
     digits++;
   }

   while( digits < LIST_DIGITS )
   {
     result *= 10;
     digits++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       freeDescriptor
//
// Purpose-
//       Free any allocated descriptors
//
//----------------------------------------------------------------------------
static void
   freeDescriptor( void )           // Free the descriptor file
{
   int                 index;       // Probability index
   char*               ptrC;        // Working -> char

   if( swSCDM )
     printf("%4d freeDescriptors()\n", __LINE__);

   ptrC= NULL;
   for(index= 0; index<MAX_LIST; index++) // Clear the list
   {
     if( pList[index] != ptrC && pList[index] != NULL )
     {
       ptrC= pList[index];
       free(ptrC);
     }
     pList[index]= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadDescriptor
//
// Purpose-
//       Load the descriptor file
//
//----------------------------------------------------------------------------
static void
   loadDescriptor(                  // Load the descriptor file
     const char*       fileName)    // The source fileName
{
   int                 index;       // Probability index
   char*               ptrC;        // Working -> char
   char*               ptrS;        // Working -> char
   Reader              reader;      // The associated Reader
   int                 value;       // Probability value

   int                 rc;

   if( swSCDM )
     printf("%4d loadDescriptors(%s)\n", __LINE__, fileName);

   rc= reader.open(fileName);
   if( rc != 0 )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("Open error");
     exit(EXIT_FAILURE);
   }

   index= 0;
   for(;;)                          // Read reader lines
   {
     rc= reader.readLine(iLine, MAX_LINE);
     if( rc == EOF )
       break;

     // Ignore empty and comment lines
     ptrC= strip(iLine);
     if( *ptrC == '\0' )
       continue;
     if( *ptrC == '*' )
       continue;

     // Parse the line
     value= parseValue(ptrC);
     if( *ptrC != ' ' )
     {
       if( *ptrC >= '0' && *ptrC <= '9' )
         rdError(reader, "Too many digits in number");
       else
         rdError(reader, "Invalid probability syntax");
       continue;
     }

     if( value < 1 || value >= MAX_LIST )
     {
       rdError(reader, "Invalid probability");
       continue;
     }

     ptrC= skipBlank(ptrC);
     if( *ptrC == '\0' )
     {
       rdError(reader, "Missing codon string");
       continue;
     }

     ptrS= strdup(ptrC);
     assert( ptrS != NULL );
     while( value > 0 )
     {
       if( index < MAX_LIST )
         pList[index]= ptrS;

       index++;
       value--;
     }

     if( (strlen(ptrS) % 3) != 0 )
       rdError(reader, "Codon(%s)'s length(%d) not a multiple of 3",
                       ptrS, strlen(ptrS));

     while( *ptrC != '\0' )
     {
       if( *ptrC != 'A'
           && *ptrC != 'C'
           && *ptrC != 'G'
           && *ptrC != 'T' )
       {
         rdError(reader, "Invalid character(%c) in codon string", *ptrC);
         fprintf(stderr, "Codon(%s) is used anyway\n", ptrS);
         break;
       }

       ptrC++;
     }
   }
   reader.close();

   if( index != MAX_LIST )
   {
     fprintf(stderr, "File(%s) probability sum(%d), not(%d)\n",
             fileName, index, MAX_LIST);
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isStopCodon
//
// Purpose-
//       Determine whether a codon is a stop codon
//
//----------------------------------------------------------------------------
static int                          // TRUE iff stop codon
   isStopCodon(                     // Is codon a stop codon?
     const char*       codon)       // Check codon
{
   if( strcmp(codon, "TAA") == 0
       || strcmp(codon, "TAG") == 0
       || strcmp(codon, "TGA") == 0 )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       generate
//
// Purpose-
//       Generate a random sequence
//
//----------------------------------------------------------------------------
static void
   generate(                        // Generate a random sequence
     int               length,      // sizeof(result)
     char*             result)      // Resultant string
{
   int                 count;       // Sequence count
   double              delta;       // Peak difference
   int                 index;       // Random value
   int                 L;           // Length of entry
   RandomP             randProb;    // Random probability object
   int                 rSize;       // Result length
   char*               ptrC;        // Working -> char

   strcpy(result, "ATG");           // Start the sequence
   rSize= 3;
   for(count=1;;count++)            // Add onto string
   {
     if( swSTOP > 0 && count == swSTOP )
       break;

     index= Random::get();          // Get next random value
     index &= 0x7fffffff;           // Insure positive
     index %= MAX_LIST;
     ptrC= pList[index];
     L= strlen(ptrC);
     if( (rSize + L) >= length )
     {
       fprintf(stderr, "Error(%4d) No stop codon\n", __LINE__);
       return;
     }

     // Check stop codon
     if( isStopCodon(ptrC) && rSize < peakMax && swSTOP == 0 )
     {
       if( rSize < peakMin )
         continue;

       delta= (double)(peakMax - rSize) /
              (double)(peakMax - peakMin); // Peak fraction
       if( peakScale <= 1.0 )
         randProb.set(delta);
       else
         randProb.set(pow(peakScale, -delta));

       if( !randProb.isTrue() )
         continue;
     }

     strcpy(result + rSize, ptrC);
     rSize += L;

     // End if stop codon
     if( isStopCodon(ptrC) && swSTOP == 0 )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sortDescriptor
//
// Purpose-
//       HEAP sort the data descriptor.
//
//----------------------------------------------------------------------------
void
   sortDescriptor(                  // Sort the data
     int               parent,      // Leftmost index
     int               count)       // Rightmost index
{
   char*               tAddr;       // Temporary text
   int                 tSize;       // Temporary size
   int                 child;       // Working index

   tAddr= gList[parent];
   tSize= gSize[parent];
   while( (child= (parent + parent) + 1) < count )
   {
     if( child + 1 < count && gSize[child] < gSize[child + 1] )
       child++;

     if( tSize >= gSize[child] )
       break;

     gList[parent]= gList[child];
     gSize[parent]= gSize[child];
     parent= child;
   }

   gList[parent]= tAddr;
   gSize[parent]= tSize;
}

void
   sortDescriptor( void )           // Sort the data
{
   char*               tAddr;       // Temporary text
   int                 tSize;       // Temporary size

   int                 i;

   for(i= (geneCount/2)-1; i>=0; i--)
     sortDescriptor(i, geneCount);

   for(i= geneCount-1; i>=1; i--)
   {
     tAddr=    gList[0];
     gList[0]= gList[i];
     gList[i]= tAddr;

     tSize=    gSize[0];
     gSize[0]= gSize[i];
     gSize[i]= tSize;
     sortDescriptor(0, i);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       hcdmDescriptor
//
// Purpose-
//       Static data, Hard Core Debug Mode
//
//----------------------------------------------------------------------------
void
   hcdmDescriptor( void )           // Describe the data
{
   #if( FALSE )
     if( swHCDM && geneCount >= 3 )
     {
       geneCount= 3;
       gList[0]= "ATGCAGGTGCCAGGTCCCAGGTCGTAA";
       gList[1]= "ATGCCAGGTGCGAAGGTGGGGAAACCCTTTCTAGGGGGTAAGGTGTGA";
       gList[2]= "ATGCCACCAGGTAGTGTACTGCAAGGTAGAAGGTGTCTGACCAGGTGG"
                 "GAGGTGCTGTCGAAAACCCAAGGTAAGGTGCAGGTGTAA";
     }
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dataDescriptor
//
// Purpose-
//       Describe the data.
//
//----------------------------------------------------------------------------
void
   dataDescriptor( void )           // Describe the data
{
   double              total;       // Total length

   int                 i;

   if( swSCDM )
     printf("%4d dataDescriptor()\n", __LINE__);

   if( swSCDM )
     printf("%4d ...sizing\n", __LINE__);
   for(i= 0; i<geneCount; i++)
     gSize[i]= strlen(gList[i]);

   if( swSCDM )
     printf("%4d ...sorting\n", __LINE__);
   sortDescriptor();

   total= 0.0;
   for(i= 0; i<geneCount; i++)
   {
     total += strlen(gList[i]);
     if( swHCDM )
       printf("[%4d] %6zd %s\n", i, strlen(gList[i])/3, gList[i]);
   }

   printf("\n");
   printf("%d Genes, following lengths are in codons\n", geneCount);
   if( geneCount == 0 )
     exit(EXIT_FAILURE);

   printf(" Minimum length: %6zd\n", strlen(gList[0])/3);
   printf("  Median length: %6zd\n", strlen(gList[geneCount/2])/3);
   printf(" Maximum length: %6zd\n", strlen(gList[geneCount-1])/3);
   printf(" Average length: %9.2f\n", (total / 3.0) / (double)geneCount);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       writeDescriptor
//
// Purpose-
//       Write the descriptor data.
//
//----------------------------------------------------------------------------
void
   writeDescriptor(                 // Write the descriptor data
     const char*       fileName)    // Output fileName
{
   const char*         ptrS;        // Working char*
   char                string[128]; // Working string
   FILE*               writer;      // Writer

   int                 i;

   if( fileName == NULL )
     return;

   writer= fopen(fileName, "wb");
   if( writer == NULL )
   {
     fprintf(stderr, "File(%s): ", fileName);
     perror("Open failure");
     exit(EXIT_FAILURE);
   }

   for(i= 0; i<geneCount; i++)
   {
     // The pseudo-gene header
     fprintf(writer, "> %d_pseudoGene;  seed %d file %s gene %d; \n",
                    i, initSeed, inpName, i);

     // The pseudo-gene itself
     ptrS= gList[i];
     for(;;)
     {
       if( strlen(ptrS) < 80 )
         break;

       memcpy(string, ptrS, 80);
       string[80]= '\0';
       fprintf(writer, "%s\n", string);

       ptrS += 80;
     }

     if( *ptrS != '\0' )
       fprintf(writer, "%s\n", ptrS);
     fprintf(writer, "\n");
   }

   fclose(writer);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scanString
//
// Purpose-
//       Analyze the scan string.
//
// Other outputs-
//       iLine = the scan string.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   scanString(                      // Analyze the scan string
     const char*       string,      // INP: Argument
     int&              pScan,       // OUT: Scan phase
     int&              weight)      // OUT: Associated weight
{
   int                 count;       // Number of '.' characters
   int                 exponent;    // Weight divisor

   int                 i;

   // Copy the input string to the work area
   count= 0;
   pScan= 0;
   i= 0;
   while( *string != '\0' )
   {
     if( *string == ':' )
       break;

     if( *string == '.' )
     {
       count++;
       pScan= i % 3;
     }
     else
       iLine[i++]= *string;

     string++;
   }
   iLine[i]= '\0';

   if( count == 0 )
     return "Contains no '.' character";

   if( count != 1 )
     return "Contains multiple '.' characters";

   if( i == 0 )
     return "Invalid sequence";

   // Extract the weight
   weight= 0;
   if( *string == ':' )             // If weight specified
   {
     count= 0;
     exponent= 1;
     string++;
     while( *string != '\0' )
     {
       if( *string == '.' )
       {
         if( count != 0 )
           return "Invalid number format";
         count= 1;
       }
       else if( *string < '0' || *string > '9' )
         return "Invalid number format";
       else
       {
         weight *= 10;
         weight += (int)(*string - '0');
         if( count != 0 )
           exponent *= 10;
       }
       string++;
     }

     while( exponent < MAX_LIST )
     {
       weight *= 10;
       exponent *= 10;
     }
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       scanDescriptor
//
// Purpose-
//       Scan the descriptor data.
//
//----------------------------------------------------------------------------
void
   scanDescriptor(                  // Examine the data
     const char*       inpstr)      // Argument
{
   const char*         string;      // Working string

   int                 phase;       // Working phase
   int                 pScan;       // Scan phase
   int                 sigma;       // Total match count
   int                 weight;      // Weight

   int                 lTotal[3];   // Line total by phase
   int                 sTotal[3];   // Scan total by phase

   int                 i;

   printf("\n");
   printf("Scan string %s\n", inpstr);

   string= scanString(inpstr, pScan, weight);
   if( string != NULL )
   {
     printf("  %s\n", string);
     return;
   }

   sTotal[0]= sTotal[1]= sTotal[2]= 0;
   for(i= 0; i<geneCount; i++)
   {
     lTotal[0]= lTotal[1]= lTotal[2]= 0;
     string= gList[i];
     for(;;)
     {
       string= strstr(string, iLine);
       if( string == NULL )
         break;

       phase= string - gList[i] + pScan;
       phase %= 3;
       lTotal[phase]++;
       string++;
     }

     if( swSHOW )
     {
       string= gList[i];
       printf("(%3d,%3d,%3d) %6zd",
              lTotal[0], lTotal[1], lTotal[2], strlen(string)/3);

       for(int j= 0; string[j] != '\0'; j++)
       {
         if( (j%3) == 0 )
           printf(" ");
         printf("%c", string[j]);
       }
       printf("\n");
     }

     sTotal[0] += lTotal[0];
     sTotal[1] += lTotal[1];
     sTotal[2] += lTotal[2];
   }

   if( swSHOW )
     printf("\n");

   pTotal[0] += sTotal[0];
   pTotal[1] += sTotal[1];
   pTotal[2] += sTotal[2];
   pWeight[0] += (double)sTotal[0] * (double)weight/(double)MAX_LIST;
   pWeight[1] += (double)sTotal[1] * (double)weight/(double)MAX_LIST;;
   pWeight[2] += (double)sTotal[2] * (double)weight/(double)MAX_LIST;;

   sigma= sTotal[0] + sTotal[1] + sTotal[2];
   if( sigma == 0 )
     printf("  No matches\n");
   else
   {
     for(i= 0; i<3; i++)
     {
       printf("Phase[%d] %8d  (%6.2f%%)",
              i, sTotal[i], 100.0 * (double)sTotal[i] / (double)sigma);
       if( weight > 0 )
         printf("   Weighted %11.2f",
                (double)sTotal[i] * (double)weight/(double)MAX_LIST);
       printf("\n");
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       freeWeight
//
// Purpose-
//       Free the weight descriptors
//
//----------------------------------------------------------------------------
static void
   freeWeights( void )              // Free the weight descriptors
{
   int                 index;       // Probability index

   for(index= 0; index<MAX_LIST; index++) // Clear the list
     pList[index]= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadWeights
//
// Purpose-
//       Load the weight descriptors
//
//----------------------------------------------------------------------------
static void
   loadWeights(                     // Load the weight descriptors
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   const char*         string;      // Working string

   int                 index;       // Probability index
   int                 pScan;       // Scan phase
   int                 weight;      // Weight

   int                 i;

   if( swSCDM )
     printf("%4d loadWeights()\n", __LINE__);

   if( !useWeights )
     return;

   index= 0;
   for(i= fileArgx+1; i<argc; i++)
   {
     if( *argv[i] == '-' )          // If switch parameter
       continue;                    // Ignore it

     string= scanString(argv[i], pScan, weight);
     if( string != NULL )
     {
       fprintf(stderr, "Error(%4d) Invalid scan '%s'\n", __LINE__, argv[i]);
       continue;
     }

     while( weight > 0 )
     {
       if( index < MAX_LIST )
         pList[index]= (char*)argv[i];

       index++;
       weight--;
     }
   }

   if( index != MAX_LIST )
     fprintf(stderr, "Scan probability sum(%d), not(%d)\n",
             index, MAX_LIST);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       symmetricAnalysis
//
// Purpose-
//       Generate secondary sequences, analyze them for symmetry.
//
// Notes-
//       This destroys the gSize array.
//
//----------------------------------------------------------------------------
void
   symmetricAnalysis(               // Symmetric analysis
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   const char*         string;      // Working string

   int                 geneIndex;   // Working gList index
   int                 count;       // Number of scans
   int                 index;       // Current scan string index
   int                 phase0;      // First split phase
   int                 phase1;      // Secont split phase
   int                 pScan;       // Scan phase
   int                 sigma;       // Total
   int                 substr;      // Substring index
   int                 substr0;     // First split point
   int                 substr1;     // Secont split point
   int                 tAsy;        // Total asymmetric
   int                 tSym;        // Total symmetric
   double              total;       // Total length
   int                 weight;      // Weight

   int                 symmetry[3][3]; // Origin, completion phase counts

   int                 i, j;

   if( !swSYMM )
     return;

   if( swSCDM )
     printf("%4d symmetricAnalysis()\n", __LINE__);

   // Diagnostic
   if( swSALL )
   {
     printf("\n");
     printf(" Gene Number    Start   Ending     Length Comment\n");
   }

   // Initialize the counts
   for(i= 0; i<3; i++)
     for(j= 0; j<3; j++)
       symmetry[i][j]= 0;

   // Insure at least one valid sequence specified
   string= "Not found";
   for(index= fileArgx+1; index<argc; index++)
   {
     if( *argv[index] == '-' )      // If switch parameter
       continue;                    // Ignore it

     string= scanString(argv[index], pScan, weight);
     if( string == NULL )
       break;
   }
   if( string != NULL )
   {
     printf("  No usable scan strings!\n");
     return;
   }

   // ------------------------------------------------------------------------
   // Generate the split sequences
   // :
   index= fileArgx;
   geneIndex= 0;
   for(i= 0; i<geneCount; i++)
   {
     substr0= substr1= (-1);
     if( swHCDM )
       printf("..Considering gene[%d]\n", i+1);

     for(count= 0; count<MAX_LIST*2; count++)
     {
       index= Random::get() % MAX_LIST;
       if( pList[index] == NULL )
         break;
       scanString(pList[index], pScan, weight);

       if( swHCDM )
         printf("..Considering scan '%s'%d %7.5f\n", iLine, pScan,
                (double)weight/(double)MAX_LIST);

       // Cut using the scan string
       substr= (Random::get() % gSize[i]);
       string= strstr(gList[i] + substr, iLine);
       if( string == NULL )
         string= strstr(gList[i], iLine);
       if( string == NULL )
       {
         if( swHCDM )
           printf("....Does not occur\n");
         continue;
       }

       substr= string - gList[i];
       if( substr0 < 0 )
       {
         substr0= substr + pScan;
         if( swHCDM )
           printf("..Selected %d\n", substr0);
         continue;
       }

       substr1= substr + pScan;
       if( substr1 == substr0 )
       {
         if( swHCDM )
           printf("..Duplicate %d (skipped)\n", substr0);
         continue;
       }

       if( swHCDM )
         printf("..Selected %d\n", substr1);
       if( substr0 > substr1 )
       {
         substr= substr0;
         substr0= substr1;
         substr1= substr;
       }

       gSize[geneIndex]= substr1 - substr0;
       phase0= substr0 % 3;
       phase1= substr1 % 3;
       if( swSALL )
         printf("%12d %8d %8d %10d %s\n",
                i+1, phase0, phase1, gSize[geneIndex],
                phase0 == phase1 ? "Symmetric" : "Asymmetric");

       symmetry[phase0][phase1]++;
       geneIndex++;
       break;
     }
     if( substr1 < 0 || substr1 == substr0 )
     {
       if( swSALL )
         printf("%12d        -        -          - Unused\n", i+1);
       else if( swSCDM || swHCDM )
       {
         if( substr1 < 0 )
           printf("%12d  No split point found\n", i+1);
         else
           printf("%12d One split point found\n", i+1);
       }
     }
   }
   geneCount= geneIndex;
   // :
   // Generate the split sequences
   // ------------------------------------------------------------------------

   // Sort by size
   if( swSCDM )
     printf("%4d ...sorting\n", __LINE__);
   sortDescriptor();

   total= 0.0;
   for(i= 0; i<geneCount; i++)
   {
     total += gSize[i];
     if( swHCDM )
       printf("[%4d] %6d\n", i, gSize[i]);
   }

   // Symmetric analysis
   printf("\n");
   printf("Symmetric analysis:\n");

   printf("\n");
   printf("%d Genes, following lengths are in nucleotides\n", geneCount);
   if( geneCount == 0 )
     return;

   printf(" Minimum length: %6d\n", gSize[0]);
   printf("  Median length: %6d\n", gSize[geneCount/2]);
   printf(" Maximum length: %6d\n", gSize[geneCount-1]);
   printf(" Average length: %9.2f\n", total / (double)geneCount);

   sigma= tSym= tAsy= 0;
   for(i= 0; i<3; i++)
     for(j= 0; j<3; j++)
     {
       sigma += symmetry[i][j];
       if( i == j )
         tSym += symmetry[i][j];
       else
         tAsy += symmetry[i][j];
     }

   total= sigma;
   if( total == 0.0 )
     total= 1;

   printf("\n");
   for(i= 0; i<3; i++)
     printf("    %d,%d: %8d  (%6.2f%%)\n", i, i, symmetry[i][i],
            100.0 * (double)symmetry[i][i]/total);
   printf("    ---- --------  ---------\n");
   printf("    %13d  (%6.2f%%) Symmetric\n",
          tSym, 100.0 * (double)tSym/total);

   printf("\n");
   sigma= 0;
   for(i= 0; i<3; i++)
     for(j= 0; j<3; j++)
       if( i != j )
         printf("    %d,%d: %8d  (%6.2f%%)\n", i, j, symmetry[i][j],
                100.0 * (double)symmetry[i][j]/total);
   printf("    ---- --------  ---------\n");
   printf("    %13d  (%6.2f%%) Asymmetric\n",
          tAsy, 100.0 * (double)tAsy/total);
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
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   int                 i;

   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   parm(argc, argv);
   init();

   //-------------------------------------------------------------------------
   // Generate the gene array
   //-------------------------------------------------------------------------
   loadDescriptor(inpName);
   for(i=0; i<geneCount; i++)
   {
     generate(MAX_LINE, iLine);
     gList[i]= strdup(iLine);
     assert( gList[i] != NULL );
     if( swSCDM && (i%1000) == 999 )
     {
       fprintf(stderr, ".");
       fflush(stderr);
     }
   }
   if( swSCDM && geneCount >= 1000 )
     fprintf(stderr, "\n");
   freeDescriptor();

   //-------------------------------------------------------------------------
   // Describe the gene array
   //-------------------------------------------------------------------------
   hcdmDescriptor();
   dataDescriptor();
   writeDescriptor(outName);

   //-------------------------------------------------------------------------
   // Analyze the list
   //-------------------------------------------------------------------------
   initTotals();
   for(i= fileArgx+1; i<argc; i++)
   {
     if( *argv[i] != '-' )
       scanDescriptor(argv[i]);
   }
   termTotals();

   //-------------------------------------------------------------------------
   // Symmetric analysis
   //-------------------------------------------------------------------------
   loadWeights(argc, argv);
   symmetricAnalysis(argc, argv);
   freeWeights();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

