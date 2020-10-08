//----------------------------------------------------------------------------
//
//       Copyright (c) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       inp2out.cpp
//
// Purpose-
//       inp2out control command.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Reader.h>
#include <com/Tokenizer.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DEBUG FALSE                 // Debugging?
#define MAX_TOKEN 1000              // The maximum number of tokens

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     delim;       // The input token delimiter
static int             execute;     // Execution control
static int             verify;      // Verification control

static char            buffer[65536]; // Input line buffer
static int             tokenCount= 0; // The number of tokens
static const char*     token[MAX_TOKEN]; // The token array

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//       error
//       print
//
// Purpose-
//       Write a debug message onto stderr
//       Write an error message onto stderr
//       Write a message onto stdout
//
//----------------------------------------------------------------------------
static inline void
   debug(                           // Write debug message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   if( DEBUG )
   {
     fprintf(stderr, "DEBUG: ");

     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stderr, fmt, argptr);
     va_end(argptr);                // Close va_ functions
   }
}

static inline void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

static inline void
   print(                           // Write print message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stdout, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*       command)     // The command name
{
   error("%s [options] format data\n\n", command);

   error("This command converts stdin into a series of tokens,\n"
         "numbered from 1 to a maximum of 1000.\n"
         "The output from this command is the format data,\n"
         "with \"$n.\" in the format data replaced by the associated token.\n"
         "$$ in the format line is replace by a single $ character.\n"
         "DOS example:\n"
         "  date /t | %s $2. | %s -delim=/ mkdir foo\\$3.-$1.-$2.\n"
         "outputs the string:\n"
         "  mkdir foo\\2008-01-30\n\n"
         , command, command);

   error("options:\n"
         "-h\tDisplay this help message, then exit\n"
         "-delim=x\tUse 'x' as a delimiter instead of whitespace\n"
         "-execute\tDon't output the command; execute it\n"
         "--\tIgnore any additonal options;\n"
         "\tUsed to treat the remaining parameters as format data\n"
         );

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
static int                          // The first input argument
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator

   int                 i;

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   execute= FALSE;
   verify= FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 )  // If help request
         HELPI= TRUE;

       else if( strcmp("-execute", argp) == 0 )
         execute= TRUE;

       else if( strcmp("-verify", argp) == 0 )
         verify= TRUE;

       else if( memcmp("-delim=", argp, 7) == 0 )
         delim= argp+7;

       else if( strcmp("--", argp) == 0 )
       {
         argi++;
         break;
       }

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If format
       break;
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( verify )
   {
     error("     -delim: %s\n", delim == NULL ? "NULL" : delim);
     error("   -execute: %s\n", execute == 0 ? "FALSE" : "TRUE");
     error("Format Line: '");
     for(i= argi; i<argc; i++)
     {
       if( i != argi )
         error(" ");
       error("%s", argv[i]);
     }
     error("'\n");
   }

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info(argv[0]);
   }

   return argi;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadTokenArray
//
// Purpose-
//       Load the token array
//
//----------------------------------------------------------------------------
static void
   loadTokenArray( void )           // Load the token array
{
   FileReader          reader(NULL);// STDIN Reader

   int                 i;
   int                 rc;

   debug("loadTokenArray()\n");

   for(i= 0; i<MAX_TOKEN; i++)
     token[i]= NULL;

   // Load the tokens
   tokenCount= 1;
   while( tokenCount < MAX_TOKEN )
   {
     rc= reader.readLine(buffer, sizeof(buffer));
     if( rc < 0 )
       break;

     Tokenizer t(buffer, delim);
     while( tokenCount < MAX_TOKEN )
     {
       const char* token= t.nextToken();
       if( token == NULL )
         break;

       ::token[tokenCount++]= strdup(token);
     }
   }

   // Verify the tokens
   if( verify )
   {
     for(i= 1; i<tokenCount; i++)
       error(" token[%3d]: %s\n", i, token[i]);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       append
//
// Purpose-
//       Append to buffer.
//
//----------------------------------------------------------------------------
static int                          // New offset
   append(                          // Append to buffer
     int               offset,      // Current offset
     const char*       string,      // Append string
     int               length)      // Append length
{
   debug("append(%d,%s,%d)\n", offset, string, length);

   if( length < 0 )
     return offset;

   if( size_t(offset + length) >= sizeof(buffer) )
   {
     error("Buffer overflow\n");
     exit(EXIT_FAILURE);
   }

   memcpy(buffer+offset, string, length);
   return offset + length;
}

static int                          // New offset
   append(                          // Append to buffer
     int               offset,      // Current offset
     const char*       string)      // Append string
{
   return append(offset, string, strlen(string));
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
   int                 argi;        // Argument index
   int                 X= 0;        // Buffer output index

   int                 i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   debug("main()\n");
   delim= NULL;
   argi= parm(argc, argv);
   loadTokenArray();

   //-------------------------------------------------------------------------
   // Process the command data
   //-------------------------------------------------------------------------
   int first= TRUE;
   while( argi < argc )
   {
     if( !first )
       X= append(X, " ", 1);
     first= FALSE;

     const char* C= argv[argi++];
     while( *C != '\0' )
     {
       const char* cC= strchr(C, '$');
       if( cC == NULL )
       {
         X= append(X, C);
         break;
       }
       int dX= cC - C;

       X= append(X, C, dX);
       C += dX;

       // Look for properly formatted token
       if( memcmp(C, "$$", 2) == 0 )
       {
         X= append(X, "$", 1);
         C += 2;
         continue;
       }

       if( memcmp(C, "$*.", 3) == 0 )
       {
         for(i= 1; i<tokenCount; i++)
         {
           if( i != 1 )
             X= append(X, " ", 1);
           X= append(X, token[i]);
         }

         C += 3;
         continue;
       }

       int dY= 2;
       int tX= 0;
       for(dY= 1; C[dY] != '.'; dY++)
       {
         if( C[dY] < '0' || C[dY] > '9' )
         {
           tX= (-1);
           break;
         }

         tX *= 10;
         tX += (C[dY] - '0');
       }

       if( tX < 0 || dY == 1 )      // "$." is invalid
       {
         X= append(X, "$", 1);
         C += 1;
         continue;
       }

       C += (dY + 1);
       if( tX > 0 && tX < tokenCount )
         X= append(X, token[tX]);
     }
   }
   append(X, "", 1);

   int rc= 0;
   if( execute )
   {
     rc= system(buffer);
     if( rc != 0 )
       rc= 2;
   }
   else
     print("%s", buffer);

   return rc;
}

