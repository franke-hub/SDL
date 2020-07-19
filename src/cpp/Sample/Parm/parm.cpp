//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       parm.cpp
//
// Purpose-
//       Parameter tester.
//
// Last change date-
//       2020/07/18
//
//----------------------------------------------------------------------------
#include<ctype.h>
#include<errno.h>
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

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
//
// Struct-
//       Master
//
// Purpose-
//       Master parameter area.
//
//----------------------------------------------------------------------------
struct Master                       // Master parameter area
{
   long                parmd;
   double              parmr;
   long                parmx;
   int                 firstFile;
}; // struct Master

static Master          masterDef;   // Master area
Master*                master= &masterDef; // -> Master area

//----------------------------------------------------------------------------
//
// Subroutine-
//       findBlank
//
// Purpose-
//       Find next blank in string.
//
//----------------------------------------------------------------------------
static inline const char*           // -> Next blank in string
   findBlank(                       // Find blank in string
     const char*       C)           // -> String
{
   int                 c;           // Current character

   for(;;)
   {
     c= *C;
     if( c == ' ' || c == '\t' || c == '\0' )
       break;

     C++;
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Skip over blanks in string.
//
//----------------------------------------------------------------------------
static inline const char*           // -> First non-blank in string
   skipBlank(                       // Skip blanks in string
     const char*       C)           // -> String
{
   int                 c;           // Current character

   for(;;)
   {
     c= *C;
     if( c != ' ' && c != '\t' )
       break;

     C++;
   }

   return C;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseDouble
//
// Purpose-
//       Parse a string, extracting a double value.
//
//----------------------------------------------------------------------------
static double                       // Return value
   parseDouble(                     // Extract double value from string
     const char*       C)           // -> String
{
   double              result;      // Resultant
   double              sign;        // Sign of resultant
   double              divisor;     // Divisor
   int                 c;           // Current character
   int                 decimal;     // TRUE if decimal point found
   int                 exponent;    // Exponent value
   int                 expsign;     // Sign of exponent

   // Handle sign of mantissa
   sign= 1.0;
   if( *C == '-' )
   {
     sign= -1.0;
     C++;
   }
   else if( *C == '+' )
     C++;

   // Insure some number is present
   if( *C == '\0' )
     errno= EINVAL;

   // Extract mantissa
   decimal= FALSE;
   divisor= 1.0;
   result=  0.0;
   for(;;)
   {
     c= *C;
     if( c == '.' )                 // If decimal point
     {
       if( decimal )                // If this is the second one
       {
         errno= EINVAL;
         break;
       }

       decimal= TRUE;
       C++;
       continue;
     }

     if( c < '0' || c > '9' )
       break;

     if( decimal )
       divisor *= 10.0;

     result *= 10.0;
     result += c - '0';
     C++;
   }

   // Extract exponent
   if( toupper(c) == 'E' )
   {
     C++;
     expsign= 1;
     if( *C == '-' )
     {
       expsign= -1;
       C++;
     }
     else if( *C == '+' )
       C++;

     exponent= 0;
     for(;;)
     {
       c= *C;
       if( c < '0' || c > '9' )
         break;

       exponent *= 10;
       exponent += c - '0';
       C++;
     }

     if( expsign < 0 )
       divisor *= pow(10.0, exponent);
     else
       divisor /= pow(10.0, exponent);
   }
   if( c != '\0' )
     errno= EINVAL;

   return (sign*result)/divisor;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseHex
//
// Purpose-
//       Parse a string, extracting a hexidecimal value.
//
//----------------------------------------------------------------------------
static long                         // Resultant value
   parseHex(                        // Convert hexidecimal to long
     const char*       C)           // -> String
{
   unsigned long       result;      // Resultant
   int                 c;           // Current character

   // Insure some number is present
   if( *C == '\0' )
     errno= EINVAL;

   // Skip over 0x prefix, if present
   if( C[0] == '0' && toupper(C[1]) == 'X' )
     C += 2;

   // Extract value
   result= 0;
   for(;;)
   {
     c= toupper(*C);
     if( c >= '0' && c <= '9' )
       c -= '0';

     else if( c >= 'A' && c <= 'F' )
       c= 10 + (c - 'A');

     else if( c == '\0' )
       break;

     else
     {
       errno= EINVAL;
       break;
     }

     if( (result & 0xf0000000) != 0 )
       errno= ERANGE;
     result <<= 4;
     result |= c;

     C++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseLong
//
// Purpose-
//       Parse a string, extracting a long value.
//
//----------------------------------------------------------------------------
static long                         // Resultant value
   parseLong(                       // Convert to long
     const char*       C)           // -> String
{
   long                result;      // Resultant
   int                 c;           // Current character
   int                 sign;        // Sign(result)

   sign= 0;
   if( *C == '+' || *C == '-' )
   {
     sign= 1;
     if( *C == '-' )
       sign= (-1);
     C++;
   }

   // Insure some number is present
   if( *C == '\0' )
     errno= EINVAL;

   // If 0x prefix, extract hexidecimal value
   if( C[0] == '0' && toupper(C[1]) == 'X' )
   {
     if( sign != 0 )                // Hex values cannot be signed
       errno= EINVAL;

     return parseHex(C);
   }

   // Extract value
   result= 0;
   for(;;)
   {
     c= *C;
     if( c < '0' || c > '9' )
     {
       if( c != '\0' )
         errno= EINVAL;
       break;
     }

     result *= 10;
     result += (c - '0');
     if( result < 0 )
       errno= ERANGE;

     C++;
   }

   // Account for sign
   if( sign < 0 )
     result= (-result);

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parmLhex
//
// Purpose-
//       Get long hexidecimal parameter.
//
//----------------------------------------------------------------------------
static int                          // TRUE if set
   parmLhex(                        // Get long hexidecimal parameter
     const char*       argument,    // The parameter argument
     const char*       parmName,    // The parameter name
     long*             result)      // Resultant
{
   unsigned            const L= strlen(parmName);

   if( memcmp(argument, parmName, L) != 0 )
     return FALSE;

   errno= 0;
   *result= parseHex(argument+L);
   if( errno != 0 )
     perror(argument);

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parmLdec
//
// Purpose-
//       Get long parameter.
//
//----------------------------------------------------------------------------
static int                          // TRUE if set
   parmLdec(                        // Get long parameter
     const char*       argument,    // The parameter argument
     const char*       parmName,    // The parameter name
     long*             result)      // Resultant
{
   unsigned            const L= strlen(parmName);

   if( memcmp(argument, parmName, L) != 0 )
     return FALSE;

   errno= 0;
   *result= parseLong(argument+L);
   if( errno != 0 )
     perror(argument);

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parmReal
//
// Purpose-
//       Get double parameter.
//
//----------------------------------------------------------------------------
static int                          // TRUE if set
   parmReal(                        // Get long parameter
     const char*       argument,    // The parameter argument
     const char*       parmName,    // The parameter name
     double*           result)      // Resultant
{
   unsigned            const L= strlen(parmName);

   if( memcmp(argument, parmName, L) != 0 )
     return FALSE;

   errno= 0;
   *result= parseDouble(argument+L);
   if( errno != 0 )
     perror(argument);

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "parm <options> <fileName ...>\n");
   fprintf(stderr, "\tTest of parameter analyzer.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Options\n");
   fprintf(stderr, "-parmd:value\n");
   fprintf(stderr, "\tSpecifies a decimal value.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-parmr:value\n");
   fprintf(stderr, "\tSpecifies a floating-point value.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "-parmx:value\n");
   fprintf(stderr, "\tSpecifies a hexidecimal value.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "fileName ...\n");
   fprintf(stderr, "\tSpecifies a list of file names.\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 error;       // Error count
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error=  0;                       // Default, no errors found
   verify= 0;                       // Default, no verification
   master->firstFile= argc;         // Default, no filename specified

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter
     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char
       errno= 0;                    // Default, no error
       if( strcmp("verify", argp) == 0 ) // If verify switch
         verify= TRUE;              // Get switch value

       else if( parmLdec(argp, "parmd:", &master->parmd) )
         ;

       else if( parmReal(argp, "parmr:", &master->parmr) )
         ;

       else if( parmLhex(argp, "parmx:", &master->parmx) )
         ;

       else if( strcmp(argp, "help") == 0 )
         error++;

       else if( strcmp(argp, "") == 0 ) // If standalone '-' parameter
       {
         master->firstFile= argi + 1;
         break;
       }
       else
       {
         error++;
         fprintf(stderr, "Invalid  control '%s'\n", argv[argi]);
       }

       if( errno != 0 )
         error++;
     }
     else                           // If not a switch
     {
       master->firstFile= argi;
       break;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error != 0 )                 // If error encountered
     info();

   if( verify )                     // If error encountered
     fprintf(stderr, "Verify specified\n");
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
   int                 i;

   memset(master, 0, sizeof(*master));
   parm(argc, argv);

   printf("parmd: %10ld 0x%.8lx\n", master->parmd, master->parmd);
   printf("parmr: %10f %10g\n",   master->parmr, master->parmr);
   printf("parmx: %10ld 0x%.8lx\n", master->parmx, master->parmx);
   for(i= master->firstFile; i<argc; i++)
     printf("File: '%s'\n", argv[i]);

   return 0;
}

