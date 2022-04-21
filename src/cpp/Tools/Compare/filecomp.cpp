//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
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
//       2022/04/21
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
#include <string>                   // For std::string

#include <ctype.h>                  // For isprint, ...
#include <getopt.h>                 // For getopt_long, ...
#include <errno.h>                  // For errno
#include <stdio.h>                  // For FILE*, ...
#include <limits.h>                 // For INT_MAX, INT_MIN, ...
#include <stdlib.h>                 // For strtol, ...
#include <string.h>                 // For strcht, ...

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/utility.h>            // For pub::utility::wildchar::strcmp

using namespace pub::debugging;     // For debugging functions
using namespace pub::fileman;       // For pub::fileman classes

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
   Data wild(Name::get_path_name(wild_name), Name::get_file_name(wild_name));
   Data file(Name::get_path_name(file_name), Name::get_file_name(file_name));

   if( wild.damaged() || file.damaged() ) {
     if( opt_verbose ) {
       if( wild.damaged() )
         fprintf(stderr, "Unable to open(%s)\n", wild_name);
       if( file.damaged() )
         fprintf(stderr, "Unable to open(%s)\n", file_name);
       return 2;
     }
   }

   // Compare the files             // Line by line compare
   pub::DHDL_list<Line>& wild_list= wild.line();
   pub::DHDL_list<Line>& file_list= file.line();
   Line* wild_line= wild_list.get_head();
   Line* file_line= file_list.get_head();

   size_t line= 0;                  // The current compared file line number
   for(;;) {
     ++line;

     if( wild_line == nullptr && file_line == nullptr )
       break;

     if( wild_line == nullptr || file_line == nullptr ) {
       if( opt_verbose )
         fprintf(stderr, "Wildfile(%s)::Testfile(%s) line(%zd) mismatch\n"
                       , wild_line ? wild_line->text : "EOF"
                       , file_line ? file_line->text : "EOF"
                       , line);
       return 1;
     }

     const char* W= wild_line->text;
     const char* F= file_line->text;
     if( strcmp(W, "*") == 0 ) {    // If line match sequence
       wild_line= wild_line->get_next();
       if( wild_line == nullptr )   // If match until end of file
         break;                     // Files compare equal
       W= wild_line->text;
       if( strcmp(W, "*") == 0 )    // If matching "*" line
         W= "\\*";                  // Make it explicit

       // Find matching line
       size_t first_line= line;
       const char* first_text= F;
       while( pub::utility::wildchar::strcmp(W, F) != 0 ) {
         ++line;
         file_line= file_line->get_next();
         if( file_line == nullptr ) {
           if( opt_verbose )
             fprintf(stderr, "%zd '%s' No matching line for '*'\n"
                           , first_line, first_text);
           return 1;
         }
         F= file_line->text;
       }
     }

     if( pub::utility::wildchar::strcmp(W, F) != 0 ) {
       if( opt_verbose )
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
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(int, char**)                // Initialize
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
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

           default: {
//             if( size_t(opt_index) < OPNO ) {
//               std::string S= (argv[optind-1] + 2); // Strip out leading "--"
//               size_t X= S.find('='); // Strip out trailing "=value"
//               if( X != std::string::npos )
//                 S= S.substr(0, X);
//               if( parm_f(S, optarg) )
//                 opt_help= true;
//             } else {
               opt_help= true;
               fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                               opt_index);
//             }
             break;
           }
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
//       term
//
// Purpose-
//       Termination cleanup
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Wrapper::main
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

   rc= init(argc, argv);
   if( rc ) return rc;

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   try {
     rc= compare();
     if( opt_verbose ) {
       if( rc == 0 )
         printf("OK: Files '%s' and '%s' compare equal\n"
               , wild_name, file_name);
       else if( rc == 1 )
         printf("NG: Files '%s' and '%s' data mismatch\n"
               , wild_name, file_name);
       else
         printf("NG: Parameter error\n");
     }
   } catch(pub::Exception& x) {
     fprintf(stderr, "pub::exception(%s)\n", x.what());
     rc= 2;
   } catch(std::exception& x) {
     fprintf(stderr, "std::exception(%s)\n", x.what());
     rc= 2;
   } catch(const char* x) {
     fprintf(stderr, "const char*(%s) exception\n", x);
     rc= 2;
   } catch(...) {
     fprintf(stderr, "Exception ...\n");
     rc= 2;
   }

   //-------------------------------------------------------------------------
   // Termination cleanup and exit
   //-------------------------------------------------------------------------
   term();
   return rc;
}
