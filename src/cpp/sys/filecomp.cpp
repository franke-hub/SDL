//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       filecomp.cpp
//
// Purpose-
//       Compare two files using '?' and '*' wildcard characters.
//
// Last change date-
//       2023/05/19
//
// Implementation notes-
//       Compares actual output with expected output. Originally implemented
//       to regression test program outputs containing timestamps.
//
//       Files may not contain '\0' characters.
//       Comparison does not differentiate line endings "\r\n" and "\n".
//
// Usage-
//       filecomp wildfile testfile
//
//       wildfile: the file containing wildcard characters.
//       testfile: the file without wildcard characters.
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <ctype.h>                  // For isprint, ...
#include <getopt.h>                 // For getopt_long, ...
#include <errno.h>                  // For errno
#include <stdio.h>                  // For printf, fprintf, ...
#include <limits.h>                 // For INT_MAX, INT_MIN, ...
#include <stdlib.h>                 // For strtol, ...
#include <string.h>                 // For strchr, strcmp, strerror, ...
#include <sys/stat.h>               // For struct stat

#include "filecomp.hpp"             // For internal classes and methods

using std::string;                  // (Shortcut)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, larger is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static int info( void );            // Informational exit

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char*           wild_name;   // Wildcard file name
static char*           file_name;   // Compared file name

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_hcdm= HCDM; // Hard Core Debug Mode
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index
static int             opt_verbose= VERBOSE; // Verbosity

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long parameter: longopts
{  {"help",    no_argument,       &opt_help,      true}
,  {"hcdm",    no_argument,       &opt_hcdm,      true}
,  {"verbose", optional_argument, &opt_verbose,      1}

,  {0, 0, 0, 0}                     // (End of Wrapper internal option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_VERBOSE

,  OPT_SIZE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       compare
//
// Purpose-
//       Compare files.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 if files compare equal
   compare( void )                  // Compare files
{
   Data wild(wild_name);
   Data file(file_name);

   // Compare the files             // Line by line compare
   Line* wild_line= wild.get_head();
   Line* file_line= file.get_head();

   size_t line= 0;                  // The current compared file line number
   for(;;) {
     ++line;

     if( wild_line == nullptr && file_line == nullptr )
       break;

     if( wild_line == nullptr || file_line == nullptr ) {
       fprintf(stderr, "Wildfile(%s)::Testfile(%s) line(%zd) mismatch\n"
                     , wild_line ? wild_line->get_text() : "EOF"
                     , file_line ? file_line->get_text() : "EOF"
                     , line);
       return 1;
     }

     const char* W= wild_line->get_text();
     const char* F= file_line->get_text();
     if( strcmp(W, "*") == 0 ) {    // If line match sequence
       wild_line= wild_line->get_next();
       if( wild_line == nullptr )   // If match until end of file
         break;                     // Files compare equal
       W= wild_line->get_text();
       if( strcmp(W, "*") == 0 )    // If matching "*" line
         W= "\\*";                  // Make it explicit

       // Find matching line
       size_t first_line= line;
       const char* first_text= F;
       while( wildchar::strcmp(W, F) != 0 ) {
         ++line;
         file_line= file_line->get_next();
         if( file_line == nullptr ) {
           if( opt_verbose )
             fprintf(stderr, "%zd '%s' No matching line for '*'\n"
                           , first_line, first_text);
           return 1;
         }
         F= file_line->get_text();
       }
     }

     if( wildchar::strcmp(W, F) != 0 ) {
       fprintf(stderr, "%zd '%s'::'%s' mismatch\n", line, W, F);
       return 1;
     }
     wild_line= wild_line->get_next();
     file_line= file_line->get_next();
   }

   // Files compare equal
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine0
//       info
//
// Purpose-
//       Informational exit.
//
//----------------------------------------------------------------------------
static int                         // Main program return code
   info( void )                    // Informational exit
{
   fprintf(stderr, "\n\n"
"%s <options> wildfile testfile\n"
"Options:\n"
"  --help\tThis help message\n"
"  --hcdm\tHard Core Debug Mode\n"
"  --verbose\t{=n} Verbosity\n"
"\n"
"Compare two files, the first allowing \"wildcard\" characters '\\', '?',\n"
"and '*'. The '\\' character is an escape character that's followed by any\n"
"character including a wildcard character that's no longer wild. The '?'\n"
"character matches one character and '*' matches any number of characters.\n"
"\n"
"As a special case, a line containing '*' matches any number of lines.\n"
, __FILE__);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       a2i
//
// Purpose-
//       Convert string to integer, setting errno.
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   a2i(                             // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       p2i
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   p2i(const char* V, const char* N) // Extract and verify parameter
{
   int value= a2i(V);
   if( errno ) {
     opt_help= true;
     if( N == nullptr )
       N= "parameter";

     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", N, V);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", N);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", N, V);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if OK)
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     switch( C ) {
       case 0:
       {{{{
         switch( opt_index ) {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= p2i(optarg, OPTS[opt_index].name);
             break;

           default:
               opt_help= true;
               fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                               opt_index);
             break;

         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 ) {
           if( strchr(argv[optind-1], '=') )
             fprintf(stderr, "Option has no argument '%s'.\n"
                           , argv[optind-1]);
           else
             fprintf(stderr, "Option requires an argument '%s'.\n"
                           , argv[optind-1]);
         } else {
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         }
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.", optopt);
         else
           fprintf(stderr, "Unknown option character '0x.2%x'."
                         , (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).", __LINE__
                       , C, (C & 0x00ff));
         opt_help= true;
         break;
     }
   }

   if( opt_help == false ) {
     if( (argc - optind) != 2 ) {
       fprintf(stderr, "Two arguments required");
       info();
       return 2;
     }

     wild_name= argv[optind];
     file_name= argv[optind + 1];
   }

   if( opt_help ) {
     info();
     exit(0);
   }

   return 0;
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
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);
   if( rc ) return rc;

   //-------------------------------------------------------------------------
   // Compare the files
   //-------------------------------------------------------------------------
   try {
     rc= compare();
     if( opt_verbose || rc != 0 ) {
       if( rc == 0 )
         printf("PASS: Files '%s' and '%s' compare equal\n"
               , wild_name, file_name);
       else if( rc == 1 )
         printf("FAIL: Files '%s' and '%s' data mismatch\n"
               , wild_name, file_name);
       else
         printf("FAIL: error %d\n", rc);
     }
   } catch(std::exception& x) {
     fprintf(stderr, "std::exception(%s)\n", x.what());
     rc= 2;
   } catch(...) {
     fprintf(stderr, "Exception ...\n");
     rc= 2;
   }

   return rc;
}
