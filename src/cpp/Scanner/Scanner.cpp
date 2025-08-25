//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Scanner.cpp
//
// Purpose-
//       Source file checker.
//
// Last change date-
//       2025/08/25
//
// Usage-
//       Scanner {path} options
//
// Options-
//       --help:       Display options and exit
//       --verbose{=n} Set verbosity, higher is more verbose
//       --all:        Check file mode, file format, and copyright text
//       --auto:       Enable auto-correct (alias of --x)
//       --copy:       [auto-correct] Verify copyright
//       --listx:      Get list of file extentions
//       --mode:       [auto-correct] Verify file mode (permissions)
//       --multi:      Allow multiple detections or corrections
//       --permits:    [auto-correct] Verify file mode (alias of --mode)
//       --unix:       [auto-correct] Verify unix file format
//       --x:          Enable auto-correct (alias of --auto)
//
// Usage notes-
//       scanner src --all --verbose --multi
//         * Detects all errors, corrects none
//
//       scanner src --all --verbose --multi --x
//         * Detects all errors, corrects where possible
//
//       scanner src --all --verbose
//         * Detects ONE error
//
//       scanner src --all --verbose --x
//         * Corrects ONE error (if possible)
//
//       scanner --verbose=2 or more
//         * Displays TYPE: Filename correspondence
//
//       scanner --verbose=3 or more
//         * Adds IGNORED file information
//
//       scanner --verbose=4 or more
//         * Adds File(name) year(value) display
//
//       scanner --verbose=5 or more
//         * Adds extensive debugging display
//
// Implementation notes-
//       TODO: Simplify scanning. All copyrights are identical except for
//             their leading comment control characters.
//
//----------------------------------------------------------------------------
#undef  _GNU_SOURCE                 // For strcasestr
#define _GNU_SOURCE                 // For strcasestr
#include <string>                   // For std::string

#include <cassert>                  // For bringup debugging
#include <cctype>                   // For isprint()
#include <cerrno>                   // For errno
#include <climits>                  // For INT_MAX, INT_MIN
#include <cstdio>                   // For printf, ...
#include <cstdlib>                  // For exit, ...
#include <cstring>                  // For strcmp, ...
#include <ctime>                    // For localtime, ...

#include <getopt.h>                 // For getopt_long()

#include "pub/Data.h"               // For pub::data classes
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Properties.h>         // For pub::Properties
#include <pub/Tokenizer.h>          // For pub::Tokenizer
#include <pub/utility.h>            // For pub::utility::to_string

using namespace pub::data;
using namespace pub::debugging;
using std::string;
using pub::List;
using pub::Tokenizer;
typedef Tokenizer::Iterator Iterator;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static bool                         // TRUE if blanks removed
   remove_trailing_blanks(          // Remove trailing blanks
     Data&             data);       // In this file

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             EARLY_YEAR= 2000; // Earliest allowed copyright year
static string          HOME;        // The HOME environment value + "/"
static Data            IGNORE;      // The list of file names to ignore
static string          OWNER_NAME= ""; // For non-standard owner (add .)

static pub::Properties props;       // Used as simple database
static struct tm       tod;         // The current year-corrected time of day

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const string    blanks= "                "; // Blank string

//----------------------------------------------------------------------------
// Copyright tables and controls
//----------------------------------------------------------------------------
#define COPY_TYPES 5
static Data*           data_none= nullptr; // Common: unmatchable copyright

static Data*           bash_gpl=  nullptr; // The GPL  bash copyright header
static Data*           bash_lgpl= nullptr; // The LGPL bash copyright header
static Data*           bash_mit=  nullptr; // The MIT  bash copyright header
static Data*           bash_sa40= nullptr; // The SA40 bash copyright header
static Data*           bash_zero= nullptr; // The NONE bash copyright header
static int             bash_count[COPY_TYPES]= {};

static Data*           code_gpl=  nullptr; // The GPL  code copyright header
static Data*           code_lgpl= nullptr; // The LGPL code copyright header
static Data*           code_mit=  nullptr; // The MIT  code copyright header
static Data*           code_sa40= nullptr; // The SA40 code copyright header
static Data*           code_zero= nullptr; // The NONE code copyright header
static int             code_count[COPY_TYPES]= {};

static Data*           html_gpl=  nullptr; // The GPL  html copyright header
static Data*           html_lgpl= nullptr; // The LGPL html copyright header
static Data*           html_mit=  nullptr; // The MIT  html copyright header
static Data*           html_sa40= nullptr; // The SA40 html copyright header
static Data*           html_zero= nullptr; // The NONE html copyright header
static int             html_count[COPY_TYPES]= {};

static Data*           lily_gpl=  nullptr; // The GPL  lily copyright header
static Data*           lily_lgpl= nullptr; // The LGPL lily copyright header
static Data*           lily_mit=  nullptr; // The MIT  lily copyright header
static Data*           lily_sa40= nullptr; // The SA40 lily copyright header
static Data*           lily_zero= nullptr; // The NONE lily copyright header
static int             lily_count[COPY_TYPES]= {};

static int             misc_count[COPY_TYPES]= {};

struct name2data {
   const char*         name;
   Data**              data;
};

static const name2data bash_table[]=
{  {  " GPL", &bash_gpl }
,  {  "LGPL", &bash_lgpl}
,  {  " MIT", &bash_mit }
,  {  "SA40", &bash_sa40}
,  {  "ZERO", &bash_zero}
,  {  nullptr, nullptr  }
};

static const name2data code_table[]=
{  {  " GPL", &code_gpl }
,  {  "LGPL", &code_lgpl}
,  {  " MIT", &code_mit }
,  {  "SA40", &code_sa40}
,  {  "ZERO", &code_zero}
,  {  nullptr, nullptr  }
};

static const name2data html_table[]=
{  {  " GPL", &html_gpl }
,  {  "LGPL", &html_lgpl}
,  {  " MIT", &html_mit }
,  {  "SA40", &html_sa40}
,  {  "ZERO", &html_zero}
,  {  nullptr, nullptr  }
};

static const name2data lily_table[]=
{  {  " GPL", &lily_gpl }
,  {  "LGPL", &lily_lgpl}
,  {  " MIT", &lily_mit }
,  {  "SA40", &lily_sa40}
,  {  "ZERO", &lily_zero}
,  {  nullptr, nullptr  }
};

static const name2data*misc_table= bash_table;

//----------------------------------------------------------------------------
// Option values and controls
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_auto= false; // Use auto-correct?
static int             opt_index;   // Option index
static int             opt_listx= false; // Create extension list
static int             opt_mode= false; // Verify file mode
static int             opt_unix= false; // Check/repair file (unix) format
static int             opt_multi= false; // Allow multiple errors
static int             opt_verbose= VERBOSE; // Verbosity (Higher is more)
static int             opt_copy= false; // Verify copyright

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",      no_argument,       &opt_help,    true}
,  {"verbose",   optional_argument, &opt_verbose,    1}
,  {"all",       no_argument,       nullptr,         0}
,  {"auto",      no_argument,       &opt_auto,    true}
,  {"listx",     no_argument,       &opt_listx,   true}
,  {"mode",      no_argument,       &opt_mode,    true}
,  {"multi",     no_argument,       &opt_multi,   true}
,  {"permits",   no_argument,       &opt_mode,    true}
,  {"unix",      no_argument,       &opt_unix,    true}
,  {"copy",      no_argument,       &opt_copy,    true}
,  {"x",         no_argument,       &opt_auto,    true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP= 0
,  OPT_VERBOSE= 1
,  OPT_ALL= 2
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       allow_multi
//
// Function-
//       Return if multiple changes allowed. (Otherwise exit)
//
//----------------------------------------------------------------------------
static inline void
   allow_multi( void )              // Exit if multiple changes disallowed
{
   if( opt_multi )
     return;

   exit(0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ends_with
//
// Function-
//       Does string end with string?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff string ends with value
   ends_with(                       // Does
     const string&     lhs,         // This string
     const string&     rhs)         // End with this string?
{
   size_t lh_size= lhs.size();
   size_t rh_size= rhs.size();

   return lh_size >= rh_size
       && lhs.compare(lh_size - rh_size, rh_size, rhs) == 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       findb
//       skipb
//
// Function-
//       Skip to next blank
//       Skip to next non-blank
//
//----------------------------------------------------------------------------
static string                       // The resultant substring
   findb(                           // Skip to next blank in
     string            str)         // This string
{
   for(size_t i= 0; i<str.size(); ++i) {
     if( str[i] == ' ' )
       return str.substr(i);
   }

   // (No blanks found)
   return "";
}

static string                       // The resultant substring
   skipb(                           // Skip to next non-blank in
     string            str)         // This string
{
   for(size_t i= 0; i<str.size(); ++i) {
     if( str[i] != ' ' )
       return str.substr(i);
   }

   // (Only blanks found)
   return "";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_token
//
// Function-
//       Return next token, updating input string
//
//----------------------------------------------------------------------------
static __attribute__ ((noinline)) string // The resultant token
   get_token(                       // Return the next token
     string&           str)         // Updating this string to the remainder
{
   string token= str;
   for(size_t i= 0; i<str.size(); ++i) {
     if( str[i] == ' ' ) {
       token= str.substr(0, i);
       str= str.substr(i);
       return token;
     }
   }

   // (No blanks found)
   str= "";
   return token;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       starts_with
//
// Function-
//       Does string start with string?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff string starts with value
   starts_with(                     // Does
     const string&     lhs,         // This string
     const string&     rhs)         // Start with this string?
{
   size_t lh_size= lhs.size();
   size_t rh_size= rhs.size();

   return lh_size >= rh_size && lhs.compare(0, rh_size, rhs) == 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strcasestr
//
// Function-
//       Case insensitive strstr.
//
// Implementation notes-
//       TODO: Move to utility.h
//
//----------------------------------------------------------------------------
#if 0                               // (Included in /usr/include/cstring)
static const char*                  // First match, or nullptr
   strcasestr(                      // Case insensitive strstr
     const char*       lhs,         // Source string
     const char*       rhs)         // Compare string
{
   if( *rhs == '\0' )               // Match empty string?
     return lhs;

   const char* result= lhs;         // Resultant, if match
   while( *result != '\0' ) {
     const char* lhs_test= result;
     const char* rhs_test= rhs;
     for(; *rhs_test != '\0'; ++rhs_test, ++lhs_test) {
       if( toupper(*lhs_test) != toupper(*rhs_test) )
         break;
     }
     if( *rhs_test == '\0' )
       return result;
     if( *lhs_test == '\0' )
       break;

     ++result;
   }

   return nullptr;
}
#endif                              // (Included in /usr/include/cstring)

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_data
//
// Function-
//       Insure Data file actually contains data.
//
// Implementation notes-
//       EXIT if error detected
//
//----------------------------------------------------------------------------
static void
   verify_data(                     // Verify Data file contains data
     Data*             data)        // (The Data file)
{
   using pub::data::Line;
   typedef pub::DHDL_list<Line>     Line_list;

   Line_list& line_list= data->line(); // Get the line list
   if( line_list.get_head() )       // If data present
     return;                        // Everything's OK

   fprintf(stderr, "Error: File(%s) is empty/missing\n", data->full().c_str());
   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Function-
//       Initialize.
//
// Implementation notes-
//       term() MUST delete only and all new Data(...) statements
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   // Get HOME environment variable
   const char* C= getenv("HOME");   // The HOME environment variable
   if( C == nullptr ) {
     fprintf(stderr, "Missing HOME envionment variable\n");
     exit(2);
   }
   HOME= C;
   if( !ends_with(HOME, "/") )
     HOME= HOME + "/";

   // Load copyright data
   string base= HOME + ".headings"; // The base license file path

   data_none= new Data(base, "README.md"); // Load an unmatchable copyright

   bash_gpl=  new Data(base, "B.GPL");  // Load the GPL  copyright
   bash_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   bash_mit=  new Data(base, "B.MIT");  // Load the MIT  copyright
   bash_sa40= data_none;                // Load the SA40 copyright (undefined)
   bash_zero= new Data(base, "B.ZERO"); // Load the NONE copyright

   code_gpl=  new Data(base, "C.GPL");  // Load the GPL  copyright
   code_lgpl= new Data(base, "C.LGPL"); // Load the LGPL copyright
   code_mit=  new Data(base, "C.MIT");  // Load the MIT  copyright
   code_sa40= new Data(base, "C.SA40"); // Load the SA40 copyright
   code_zero= new Data(base, "C.ZERO"); // Load the NONE copyright

   html_gpl=  new Data(base, "H.GPL");  // Load the GPL  copyright
   html_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   html_mit=  new Data(base, "H.MIT");  // Load the MIT  copyright
   html_sa40= new Data(base, "H.SA40"); // Load the SA40 copyright
   html_zero= new Data(base, "H.ZERO"); // Load the NONE copyright

   lily_gpl=  new Data(base, "L.GPL");  // Load the GPL  copyright
   lily_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   lily_mit=  data_none;                // Load the MIT  copyright (undefined)
   lily_sa40= new Data(base, "L.SA40"); // Load the SA40 copyright
   lily_zero= new Data(base, "L.ZERO"); // Load the NONE copyright

   // Verify data present
   verify_data(bash_gpl);
   verify_data(bash_lgpl);
   verify_data(bash_mit);
   verify_data(bash_sa40);
   verify_data(bash_zero);

   verify_data(code_gpl);
   verify_data(code_lgpl);
   verify_data(code_mit);
   verify_data(code_sa40);
   verify_data(code_zero);

   verify_data(html_gpl);
   verify_data(html_lgpl);
   verify_data(html_mit);
   verify_data(html_sa40);
   verify_data(html_zero);

   verify_data(lily_gpl);
   verify_data(lily_lgpl);
   verify_data(lily_mit);
   verify_data(lily_sa40);
   verify_data(lily_zero);

   // Get list of IGNORE files
   IGNORE.open(".", ".ignore");     // List of files to ignore
   Line fake(nullptr);              // Replacement for comment line
   for(Line* line= IGNORE.line().get_head(); line; line= line->get_next()) {
     if( line->text[0] == '\0' || line->text[0] == '#' ) {
       fake= *line;                 // Save links for line->get_next()
       IGNORE.line().remove(line, line);
       delete line;
       line= &fake;                 // Use the fake line with saved links
     }
   }

   // Get current date and time
   time_t now; time(&now);          // Seconds since epoch
   tod= *localtime(&now);           // Calendar date
   tod.tm_year += 1900;             // Correct the year
   tod.tm_mon  += 1;                // Correct the month
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
   term( void )                     // Terminate
{
   // Implementation note: see init(). Only delete new Data() objects.
   delete data_none;

   delete bash_gpl;
// delete bash_lgpl;
   delete bash_mit;
// delete bash_sa40;
   delete bash_zero;

   delete code_gpl;
   delete code_lgpl;
   delete code_mit;
   delete code_sa40;
   delete code_zero;

   delete html_gpl;
// delete html_lgpl;
   delete html_mit;
   delete html_sa40;
   delete html_zero;

   delete lily_gpl;
// delete lily_lgpl;
// delete lily_mit;
   delete lily_sa40;
   delete lily_zero;

   //-------------------------------------------------------------------------
   // Verify all IGNORE entries found
   //-------------------------------------------------------------------------
   if( true ) {
     // Locate and count ignored files and paths
     int paths= 0;
     int files= 0;
     for(Line* line= IGNORE.line().get_head(); line; line= line->get_next())
     {
       if( files == 0 && paths == 0 )
         fprintf(stderr, "Missing .ignores:\n");
       size_t L= strlen(line->text);
       if( L > 1 && strcmp(&line->text[L-2], "/*") == 0 ) { // If ignored path
         fprintf(stderr, "Path: %s\n", line->text);
         ++paths;
       } else {                     // If ignore file
         fprintf(stderr, "File: %s\n", line->text);
         ++files;
       }
     }
     if( paths == 0 )
       printf("*ALL* .ignore paths found\n");
     else
       fprintf(stderr, "%5d .ignore path%s not found\n", paths
              , paths == 1 ? "" : "s");

     if( files == 0 )
       printf("*ALL* .ignore files found\n");
     else
       fprintf(stderr, "%5d .ignore file%s not found\n", files
              , files == 1 ? "" : "s");
   }
   IGNORE.close();

   //-------------------------------------------------------------------------
   // Display verification statistics
   //-------------------------------------------------------------------------
   if( true ) {
     printf("\nBash format copyrights:\n");
     for(int i=  0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", bash_table[i].name, bash_count[i]);
     }

     printf("\nCode format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", code_table[i].name, code_count[i]);
     }

     printf("\nHtml format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", html_table[i].name, html_count[i]);
     }

     printf("\nLily format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", lily_table[i].name, lily_count[i]);
     }

     printf("\nMisc format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", misc_table[i].name, misc_count[i]);
     }
   }
}

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
   fprintf(stderr,"\n");
   fprintf(stderr,"Scanner {path} <options>\n");
   fprintf(stderr,"Options:\n"
                   "  --help\tWrite this help message and exit\n"
                   "  --verbose\t{=n} Verbosity, 1 if =n unspecified\n"
                   "\n"
                   "  --all\t\tCheck format, mode, and copyright\n"
                   "  --auto\tAuto-correct mode\n"
                   "  --copy\tVerify copyright text\n"
                   "  --listx\tList filename extensions\n"
                   "  --mode\tVerify file mode\n"
                   "  --multi\tAllow multiple errors/changes\n"
                   "  --permits\tVerify file mode (alias for --mode)\n"
                   "  --unix\tVerify unix file format\n"
                   "  --x\t\tAuto-correct mode (alias for --auto)\n"
          );

   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
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
   parm_int( void )                 // Extract and verify integer value
{
   errno= 0;
   int value= pub::utility::atoi(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name
                     , optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name
              , optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         switch( opt_index )
         {
           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           case OPT_ALL:
             opt_copy= true;
             opt_mode= true;
             opt_unix= true;
             break;

           default:
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'\n",optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x)\n", __LINE__, C, C);
         break;
     }
   }

   if( opt_help )
     info();

   if( opt_verbose )
   {
     fprintf(stderr, "%5d --verbose\n", opt_verbose);
     fprintf(stderr, "%5s --auto\n",    opt_auto  ? " true" : "false");
     fprintf(stderr, "%5s --copy\n",    opt_copy  ? " true" : "false");
     fprintf(stderr, "%5s --listx\n",   opt_listx ? " true" : "false");
     fprintf(stderr, "%5s --mode\n",    opt_mode  ? " true" : "false");
     fprintf(stderr, "%5s --multi\n",   opt_multi ? " true" : "false");
     fprintf(stderr, "%5s --unix\n",    opt_unix  ? " true" : "false");
     fprintf(stderr, "\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_copy_line
//
// Function-
//       Get copyright line
//
//----------------------------------------------------------------------------
static inline Line*                 // The copyright line, nullptr if missing
   get_copy_line(                   // Get copyright line
     Data&             data)        // From this Data
{
   Line* line= data.line().get_head();
   int count= 5;
   while( line ) {
     if( strcasestr(line->text, "copyright") )
        return line;
     if( --count == 0 )
       break;

     line= line->get_next();
   }

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_extension
//
// Function-
//       Get filename extension.
//
//----------------------------------------------------------------------------
static inline string                // The filename extension, "" if none
   get_extension(                   // Get filename extension
     const string&     name)        // For this filename
{
   const char* S= name.c_str();     // Get the associated string
   ssize_t L= name.length();        // Get the string length
   L--;
   while( L > 0 && S[L] != '.' )
     L--;

   if( L == 0 )
     return "";

   return string(S+L+1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_bash
//
// Function-
//       Is the specified file in bash format?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name is in "bash" format
   is_bash(                         // Is file in bash format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if( name == ".gitignore" || name == ".README" || name == "README"
       || starts_with(name, "Makefile") || ext == "py" )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_binary
//
// Function-
//       Is the file name extension a binary format type?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name specifies binary format
   is_binary(                       // Does the name specify binary format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if(    ext == "class"
       || ext == "gif"
       || ext == "gpg"
       || ext == "gz"
       || ext == "jar"
       || ext == "odt"
       || ext == "pdf"
       || ext == "png"
       || ext == "pyc"
       || ext == "tgz"
       || ext == "zip"
     ) return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_code
//
// Function-
//       Is the specified file in code format?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name is in "code" format
   is_code(                         // Is file in code format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if(    ext == "cpp"
       || ext == "h"
       || ext == "H"
       || ext == "hpp"
       || ext == "i"
       || ext == "c"
       || ext == "cs"
       || ext == "java"
       || ext == "js"
     ) return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_html
//
// Function-
//       Is the specified file in html format?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name is in "html" format
   is_html(                         // Is file in html format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if( ext == "html" || ext == "htm" || ext == "md" || ext == "xml" )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_lily
//
// Function-
//       Is the specified file in html format?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name is in "lily" format
   is_lily(                         // Is file in lily format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if( ext == "ly" )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_mark
//
// Function-
//       Is the specified file in markdown format?
//
//----------------------------------------------------------------------------
static inline bool                  // TRUE iff name is in "markdown" format
   is_mark(                         // Is file in markdown format?
     const string&     name)        // The filename
{
   string ext= get_extension(name);
   if( ext == "md" )
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_script
//
// Function-
//       Is this a script file?
//
//----------------------------------------------------------------------------
static inline bool                  // True if file contains a script heading
   is_script(                       // Does the data contain a script mark?
     Data&             data)        // The associated Data object
{
   Line* line= data.line().get_head(); // The first Line
   if( line == nullptr )
     return false;

   const char* text= line->text;
   if( text == nullptr )
     return false;

   if( memcmp(text, "#!", 2) != 0 )
     return false;

   return true;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       string2int
//
// Function-
//       Convert (positive integer) string to int. Result -1 if invalid
//
// Implementation note-
//       Limited error checking, limited value range.
//
//----------------------------------------------------------------------------
static int                          // The value
   string2int(                      // Convert string to int
     const string&     inps)        // The string
{
   int result= 0;
   const char* S= inps.c_str();
   if( *S == '\0' )
     return -1;

   while( *S != '\0' ) {
     result *= 10;
     if( *S < '0' || *S > '9' )
       return -1;

     result += (*S - '0');
     if( result < 0 )
       return -1;
     S++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trim
//
// Function-
//       Trim leading and trailing blanks from string
//
//----------------------------------------------------------------------------
static inline string                // The string, -leading and trailing blanks
   trim(                            // Trim
     const string&     inps)        // This string
{
   const char* S= inps.c_str();
   while( *S == ' ' )               // Skip blanks
     S++;
   size_t L= strlen(S);
   while( L > 0 && S[L-1] == ' ' )
     L--;
   return string(S, L);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_copy_date
//
// Function-
//       Verify copyright date.
//
//----------------------------------------------------------------------------
static int                          // The copyright year, -1 if invalid
   verify_copy_date(                // Verify copyright date
     const string&     date)        // The copyright yyyy or yyyy-yyyy string
{
   int c_year= -1;
   if( date.length() == 4 ) {       // Format yyyy
     c_year= string2int(date);
   } else if( date.length() == 9 ) { // Format yyyy-yyyy
     int f_year= string2int(date.substr(0, 4));
     if( f_year > 1900 ) {
       c_year= string2int(date.substr(5));
       if( c_year <= f_year || date[4] != '-' )
         c_year= -1;
     }
   }

   return c_year;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_last_date
//
// Function-
//       Verify last change date.
//
//----------------------------------------------------------------------------
static void
   verify_last_date(                // Verify last change date
     Data&             data,        // The associated Data object
     int               year)        // The copyright year
{
   string              full= data.full(); // Get the file name

   // Verify copyright year
   if( year < EARLY_YEAR )
     printf("File(%s) Early copyright(%d)\n", full.c_str(), year);

   // Find last change date line (NOT an error if missing)
   int lineno= 0;                   // The current line counter
   Line* line;                      // The current Line
   for(line= data.line().get_head(); line; line= line->get_next()) {
      if( strcasestr(line->text, "Last change date") )
        break;

      if( ++lineno > 40 )           // Must appear somewhat early in file
        return;                     // Nothing to compare against
   }
   if( line == nullptr )            // If not found
     return;                        // Nothing to compare against
   line= line->get_next();          // The actual last change date line
   if( line == nullptr ) {          // If not found
     printf("File(%s) Missing last change date\n", full.c_str());
     allow_multi();
     return;
   }

   const char* text= line->text;    // The last chage date text
   Tokenizer tok_text(text);        // Extract the date
   Iterator  tok_iter= tok_text.begin();
   string date= (++tok_iter)();     // The date string

   Tokenizer tok_date(date, "/");   // Extract the date components
   tok_iter= tok_date.begin();
   int l_yy= string2int(tok_iter());
   int l_mm= string2int((++tok_iter)());
   int l_dd= string2int((++tok_iter)());
   if( l_yy < 1 || l_mm < 1 || l_mm > 12 || l_dd < 1 || l_dd > 31
       || (++tok_iter)() != "" ) {
     printf("File(%s) Malformed last change date(%s)\n",
            full.c_str(), date.c_str());
     allow_multi();
     return;
   }

   bool future= false;
   if( l_yy > tod.tm_year || year > tod.tm_year )
     future= true;
   else if( l_yy == tod.tm_year )
   {
     if( l_mm > tod.tm_mon )
       future= true;
     else if( l_mm == tod.tm_mon ) {
       if( l_dd > tod.tm_mday )
         future= true;
     }
   }

   if( future ) {
     printf("File(%s) Future copy(%d) last(%s)\n", full.c_str(), year, text);
     allow_multi();
     return;
   }

   if( year == l_yy )
     return;

   //-------------------------------------------------------------------------
   // Correctable mismatch detected (and copyright line has been verified.)
   if( data.damaged() || data.changed() ) { // Cannot correct if file problem
     fprintf(stderr, "File(%s) damaged(%d)/changed(%d)\n", full.c_str(),
                     data.damaged(), data.changed());
     allow_multi();
     return;
   }

   if( opt_auto == false ) {
     printf("File(%s) Correctable last(%d) copy(%d)\n"
           , full.c_str(), l_yy, year);
   } else {
     Line* line= get_copy_line(data); // The copyright line
     Tokenizer tok_line(line->text); // Our line Tokenizer
     Iterator  tok_iter= tok_line.begin();

     // Verify copyright line (an internal assert)
     bool is_error= false;
     string comment= (tok_iter++)();  // Get/skip leading comment characters
     if( tok_iter() != "Copyright" )  // If missing copyright statement
       is_error= true;

     string S= (++tok_iter)();
     if( S != "(C)" && S != "(c)" )
       is_error= true;

     string s_year= (++tok_iter)();
     int f_year= string2int(s_year.substr(0,4));
     int t_year= f_year;
     if( s_year.length() == 9 )
       t_year= string2int(s_year.substr(5));
     if( f_year < 0 || t_year < f_year )
       is_error= true;

     string owner=  (++tok_iter).remainder();
     if( !ends_with(owner, ".") )
       is_error= true;

     if( is_error ) {             // Invalid copyright text (UNEXPECTED)
       fprintf(stderr, "%4d File(%s) Copy(%s) Invalid\n"
                     , __LINE__, full.c_str(), line->text);
       allow_multi();
       return;
     }

     if( l_yy < f_year ) {        // if last change date < from year
       printf("file(%s) copy(%s) last(%d) not correctable\n",
              full.c_str(), s_year.c_str(), l_yy);
       allow_multi();
       return;
     }

     t_year= l_yy;                // Use last change date as to_year
     string n_year= pub::utility::to_string("%4d", t_year);
     if( f_year != t_year )
       n_year= pub::utility::to_string("%4d-%4d", f_year, t_year);

     comment += " ";
     if( comment.length() < 9 )
       comment += string(9 - comment.length(), ' ');
     comment += "Copyright (c) " + n_year + " " + owner;

     Line* repl= data.get_line(comment);
     data.line().insert(line, repl, repl);
     data.line().remove(line, line);
     delete line;

     data.write();
     printf("File(%s) Corrected last(%d) copy(%s)\n", full.c_str(),
            l_yy, s_year.c_str());
   }

   allow_multi();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_copy_line
//
// Function-
//       Verify copyright line.
//
//----------------------------------------------------------------------------
static int                          // The copyright year, -1 if invalid
   verify_copy_line(                // Verify copyright line
     Data&             data,        // The associated Data object
     Line*             line)        // The copyright Line
{
   string              full= data.full(); // Get the file name
   string              text= line->text; // Get the line text
   Tokenizer           tok_line(line->text); // Our line Tokenizer
   Iterator            tok_iter= tok_line.begin();

   // Verify copyright line
   // The copyright line contains either a two character token or two blanks
   string comment= "  ";
   if( text[0] != ' ' )             // If a leading token exists
     comment= (tok_iter++)();       // Get/skip leading comment token
   if( tok_iter() != "Copyright" ) { // If missing copyright statement
     printf("File(%s) (c) Malformed(%s)\n", full.c_str(), text.c_str());
     allow_multi();
     return -1;
   }

   string S= (++tok_iter)();
   if( S != "(C)" && S != "(c)" ) {
     printf("File(%s) (c) Malformed(%s)\n", full.c_str(), text.c_str());
     allow_multi();
     return -1;
   }

   if( !ends_with(text, ".") ) {
     printf("File(%s) (c) Missing ending '.'\n", full.c_str());
     allow_multi();
     return -1;
   }

   string s_year= (++tok_iter)();
   int c_year= verify_copy_date(s_year);
   if( c_year > 0 ) {
     if( OWNER_NAME != "" ) {
       string owner((++tok_iter).remainder());
       if( owner != OWNER_NAME )
         printf("File(%s) (c) Non-standard owner(%s)\n",
                full.c_str(), owner.c_str());
     }
     return c_year;
   }

   //-------------------------------------------------------------------------
   // Invalid copyright year detected, possibly correctable
   string owner= s_year;
   string next;
   for(;;) {
     next= (++tok_iter)();
     if( next == "" ) {
       c_year= -1;
       break;
     }
     if( ends_with(next, ".") ) {
       next= next.substr(0, next.length()-1);
       c_year= verify_copy_date(next);
       if( c_year > 0 ) {
         if( (++tok_iter)() != "" ) // If ending date in the middle
           c_year= -1;            // Not correctable
       }

       break;
     }
     owner += " ";
     owner += next;
   }

   if( c_year < 0 ) {
     printf("File(%s) Invalid (c) date(%s)\n", full.c_str(), text.c_str());
     allow_multi();
     return -1;
   }

   if( data.damaged() || data.changed() ) {
     fprintf(stderr, "%4d File(%s) damaged(%d)/changed(%d)\n", __LINE__
                   , full.c_str(), data.damaged(), data.changed());
     return c_year;
   }

   //-------------------------------------------------------------------------
   // Correctable date detected
   if( opt_auto ) {                 // If auto-correct allowed
     text= comment + " ";
     if( text.length() < 9 )
       text += string(9 - text.length(), ' ');
     text += "Copyright (c) " + next + " " + owner + ".";

     Line* repl= data.get_line(text);
     data.line().insert(line, repl, repl);
     data.line().remove(line, line);
     delete line;

     data.write();
     printf("File(%s) Copyright line corrected\n", full.c_str());
   } else {                         // Auto-correct disallowed
     printf("File(%s) Copyright line correctable\n", full.c_str());
   }
   allow_multi();

   return c_year;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_copy_text
//
// Function-
//       Verify that the copyright matches a supported one.
//
//----------------------------------------------------------------------------
static void
   verify_copy_text(                // Verify copyright text
     Data&             data)        // For this data
{
   Line* line= get_copy_line(data); // The copyright line
   if( line == nullptr ) {          // If missing
     fprintf(stderr, "File(%s) Copyright missing\n", data.full().c_str());
     allow_multi();
     return;
   }

   typedef pub::DHDL_list<Line>     List; // The Data's line list type
   string       data_full= data.full(); // (The string must be persistent)
   const char*       full= data_full.c_str(); // Fully qualified file name
   int*             count= misc_count; // Default: misc (a.k.a bash)
   const name2data* table= misc_table;
   string file= data.file();
   if( is_bash(file) ) {
     count= bash_count;
     table= bash_table;
   } else if( is_code(file) ) {
     count= code_count;
     table= code_table;
   } else if( is_html(file) ) {
     count= html_count;
     table= html_table;
   } else if( is_lily(file) ) {
     count= lily_count;
     table= lily_table;
   } else if( false ) {
     printf("File(%s) MISC format\n", full);
     allow_multi();
   }

   string prefix= line->text;       // (The file's copyright line)
   prefix= get_token(prefix);       // (The file's copyright line prefix)
   for(int i= 0; table[i].name; i++) {
     Data* info= *table[i].data;
     Line* lhs= get_copy_line(*info)->get_next();
     if( lhs == nullptr ) {
       fprintf(stderr, "Table(%s) invalid, exiting\n", table[i].name);
       exit(1);
     }
     Line* rhs= line->get_next();

     // (LHS: copyright line; RHS: file line) Position: copy_line->get_next()
     // Verify the copyright text
     while( lhs ) {
       if( !rhs )                   // If file ends inside copyright
         break;

       // Compare line for line, ignoring prefix
       string lh_str= skipb(findb(lhs->text));
       string rh_str= skipb(findb(rhs->text));
       if( lh_str != rh_str )
         break;

       lhs= lhs->get_next();
       rhs= rhs->get_next();
     }

     // If copyright text matched, handle special cases
     if( lhs == nullptr ) {         // (Conditionally compiled)
       // Convert all markdown files to SA40 format
       if( false && is_mark(file) && strcmp(table[i].name, "SA40") != 0 ) {
         if( opt_auto ) {           // If auto-correct allowed
           List& list= data.line(); // (The File's line list)
           Line* lhs= get_copy_line(*info)->get_next();
           Line* rhs= line->get_next();
           Line* head= rhs;
           Line* tail= rhs;
           for(;;) {
             lhs= lhs->get_next();
             if( lhs == nullptr )
               break;
             tail= tail->get_next();
           }
           list.remove(head, tail);
           while( head != tail ) {  // Delete the removed lines
             Line* next= head->get_next();
             delete head;
             head= next;
           }

           List copy;               // The replacement list
           lhs= get_copy_line(*html_sa40)->get_next();
           while( lhs ) {
             copy.fifo( data.get_line(lhs->text) );
             lhs= lhs->get_next();
           }
           list.insert(line, copy.get_head(), copy.get_tail());

           data.write();
           printf("File(%s) format(%s=>SA40)\n", full, table[i].name);
         } else {                   // Auto-correct not allowed
           printf("File(%s) format(%s)\n", full, table[i].name);
         }
         allow_multi();
       } else

       // Look for HTML in GPL or MIT format
       if( false && is_html(file) && strcmp(table[i].name, " GPL") == 0 ) {
         printf("File(%s) GPL format\n", full);
         allow_multi();
       } else
       if( false && is_html(file) && strcmp(table[i].name, " MIT") == 0 ) {
         printf("File(%s) MIT format\n", full);
         allow_multi();
       } else

       // Look for CODE in SA40 format
       if( false && is_code(file) && strcmp(table[i].name, "SA40") == 0 ) {
         printf("File(%s) SA40 format\n", full);
         allow_multi();
       } else
       {}
     }

     // If copyright text matched, verify prefix consistency
     if( lhs == nullptr ) {
       Line* lhs= get_copy_line(*info);
       Line* rhs= line;
       while( lhs ) {
         string rh_str= rhs->text;
         string token= get_token(rh_str);
         if( prefix != token ) {
           if( opt_auto ) {         // If auto-correct allowed
             string replaced= rhs->text;
             string S= prefix;      // Default, no associated text
             string lh_str= skipb(lhs->text+2); // Get the associated text
             if( skipb(lh_str) != "" ) { // If associated text exists
               if( starts_with(lh_str, "SPDX-License-Identifier:") )
                 S += ' ';
                else
                 S += blanks.substr(0, 9 - prefix.size());
               S += lh_str;         // (We already know the text matches)
             }
             Line* line= data.get_line(S);

             // Correct the prefix
             List& list= data.line();   // (The File's line list)
             list.insert(rhs->get_prev(), line);
             list.remove(line->get_next());

             data.write();
             printf("File(%s) prefix('%s'=>'%s') in line\n'%s'\n"
                   , full, token.c_str(), prefix.c_str(), replaced.c_str());
           } else {
             printf("File(%s) prefix(%s) inconsistent line(%s)\n"
                   , full, prefix.c_str(), rhs->text);
           }
           allow_multi();
           break;
         }

         lhs= lhs->get_next();
         rhs= rhs->get_next();
       }

       // Update match count
       if( opt_verbose > 1 )
         printf("[%s]: '%s'\n", table[i].name, full);

       ++count[i];
       return;
     }

     // Copyright text did not match any of the supported versions.
     // Check for missing "SPDX-License-Identifier:"
     if( rhs && memcmp(lhs->text+3, "SPDX-License-Identifier:", 24) == 0 ) {
       if( opt_auto ) {             // If auto-correct allowed
         typedef pub::DHDL_list<Line>         List;
         List& list= data.line();   // (The file line list)

         string S(prefix);
         S += lhs->text + 2;
         Line* line= data.get_line(S); // (The SPDX-License-Id.. line)
         list.insert(rhs->get_prev(), line);

         data.write();
         printf("File(%s) %s SPDX-License-Identifier added\n"
               , full, table[i].name);
       } else {                     // Auto-correct disallowed
         printf("File(%s) %s SPDX-License-Identifier missing\n"
               , full, table[i].name);
       }
       allow_multi();
       return;
     }
   }

   printf("File(%s): No copyright match\n", full);
   allow_multi();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       copy_bash
//
// Function-
//       Handle a bash (.gitignore/.README/Makefile*) copyright
//
//----------------------------------------------------------------------------
static void
   copy_bash(                       // Handle a bash copyright
     Data&             data)        // The content
{
   if( data.file() == "README" ) {
     printf("File(%s) named README\n", data.full().c_str());
     allow_multi();
   }

   Line* line= get_copy_line(data);
   if( line == nullptr ) {
     printf("File(%s) (c) Missing\n", data.full().c_str());
     allow_multi();
     return;
   }

   int c_year= verify_copy_line(data, line);
   if( c_year > 0 )
     verify_last_date(data, c_year);

   verify_copy_text(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       copy_code
//
// Function-
//       Handle a code (.cpp,...) copyright
//
// Implementation note-
//       This also handles lily (.ly) format files.
//
//----------------------------------------------------------------------------
static void
   copy_code(                       // Handle a code copyright
     Data&             data)        // The content
{
   Line* line= get_copy_line(data);
   if( line == nullptr ) {
     printf("File(%s) (c) Missing\n", data.full().c_str());
     allow_multi();
     return;
   }

   int c_year= verify_copy_line(data, line);
   if( c_year > 0 )
     verify_last_date(data, c_year);

   verify_copy_text(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       copy_html
//
// Function-
//       Handle an html file copyright
//
//----------------------------------------------------------------------------
static void
   copy_html(                       // Handle an html file copyright
     Data&             data)        // The content
{
   Line* line= get_copy_line(data);
   if( line == nullptr ) {
     printf("File(%s) (c) Missing\n", data.full().c_str());
     allow_multi();
     return;
   }

   int c_year= verify_copy_line(data, line);
   if( c_year > 0 )
     verify_last_date(data, c_year);

   verify_copy_text(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       copy_misc
//
// Function-
//       Handle all other files (No error if missing copyright)
//
//----------------------------------------------------------------------------
static void
   copy_misc(                       // Handle an unspecified file copyright
     Data&             data)        // The content
{
   Line* line= get_copy_line(data);
   if( line == nullptr )            // (Missing copyright allowed)
     return;

   int c_year= verify_copy_line(data, line);
   if( c_year > 0 )
     verify_last_date(data, c_year);

   verify_copy_text(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_path
//
// Function-
//       Handle a directory.
//
//----------------------------------------------------------------------------
static void
   handle_path(                     // Handle a directory
     const string&     path)        // Path to directory
{
   Line*               line;        // Working Line*

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   if( opt_verbose > 4 )
     fprintf(stderr, "D: %s\n", path.c_str());

   //-------------------------------------------------------------------------
   // Handle items in this directory
   //-------------------------------------------------------------------------
   Path path_(path);                // The directory content
   for(File* file= path_.list.get_head(); file; file= file->get_next())
   {
     if( S_ISLNK(file->st.st_mode) ) // Ignore soft links (avoid duplicates)
       continue;

     if( opt_listx ) {
       string extension= get_extension(file->name);
       if( props.get_property(extension) == nullptr )
         props.insert(extension, extension);
     }

     if( opt_verbose > 4 )
       fprintf(stderr, "F: %.8x %10ld %s/%s\n", file->st.st_mode,
               file->st.st_size, path.c_str(), file->name.c_str());

     if( S_ISREG(file->st.st_mode) ) {
       string full= path + "/" + file->name; // The fully qualified name
       for(line= IGNORE.line().get_head(); line; line= line->get_next())
       {
         if( strcmp(line->text, full.c_str()) == 0 ) // If IGNORE file
           break;
       }
       if( line )                   // If IGNORE file
       {
         if( opt_verbose > 2 )
           fprintf(stderr, "SKIP: %s (file)\n", full.c_str());
         IGNORE.line().remove(line, line); // Remove the IGNORE line
         delete line;               // Delete it
         continue;                  // And ignore it
       }

       string name(file->name);
       if( is_binary(name) )        // Ignore binary formatted file types
         continue;

       Data data(path, name);
       if( data.damaged() ) {
         fprintf(stderr, "File(%s) Damaged\n", data.full().c_str());
         allow_multi();
         continue;
       }

       if( opt_mode ) {             // If examining file mode
         mode_t mode= file->st.st_mode & ACCESSPERMS;
         mode_t user= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
         mode_t exec= user    | S_IXUSR | S_IXGRP | S_IXOTH;
         mode_t want= user;
         if( is_script(data) )
           want= exec;
         else if( name == "!const" )
           want= S_IRUSR;

         if( mode != want ) {       // If correction required
           if( false )
             printf("%4d Scanner mode(%.3o) want(%.3o) opt_auto(%s)\n"
                   , __LINE__, mode, want, opt_auto ? "true" : "false");
           if( !opt_auto || (mode & S_IWUSR) == 0 ) { // If can't auto-correct
             printf("File: -%s%s%s%s%s%s%s%s%s %s unchanged\n"
                   , mode & S_IRUSR ? "r" : "-"
                   , mode & S_IWUSR ? "w" : "-"
                   , mode & S_IXUSR ? "x" : "-"
                   , mode & S_IRGRP ? "r" : "-"
                   , mode & S_IWGRP ? "w" : "-"
                   , mode & S_IXGRP ? "x" : "-"
                   , mode & S_IROTH ? "r" : "-"
                   , mode & S_IWOTH ? "w" : "-"
                   , mode & S_IXOTH ? "x" : "-"
                   , full.c_str());
           } else {                 // Auto-correct
             mode= file->st.st_mode & ~(ACCESSPERMS);
             mode |= want;
             chmod(full.c_str(), mode);
             printf("CHMOD File: %s\n", full.c_str());
           }
           allow_multi();
         }
       }

       if( opt_unix ) {             // Check unix file format?
         bool had_change= data.changed();
         bool had_blanks= remove_trailing_blanks(data);
         if( had_change || had_blanks ) { // If file changed
           if( opt_auto ) {
             if( had_change )
               printf("File(%s) ==> unix format\n", data.full().c_str());
             data.write();
             data.change(false);
           } else {
             if( had_change )
               printf("File(%s) NOT IN unix format\n", data.full().c_str());
             printf("File(%s) unchanged\n", data.full().c_str());
           }
           allow_multi();
         }
       }

       if( opt_copy ) {             // Check copyright?
         if( is_code(name) )        // Implementation note: Sort likely first
           copy_code(data);
         else if( is_bash(name) )
           copy_bash(data);
         else if( is_lily(name) )
           copy_code(data);
         else if( is_html(name) )
           copy_html(data);
         else
           copy_misc(data);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Process subdirectories
   //-------------------------------------------------------------------------
   for(File* file= path_.list.get_head(); file != nullptr; file= file->get_next())
   {
     if( S_ISDIR(file->st.st_mode) ) // Handle directory ignore
     {
       string full= path + "/" + file->name + "/*";
       for(line= IGNORE.line().get_head(); line; line= line->get_next())
       {
         if( full == line->text )
           break;
       }

       if( line ) {                 // If directory in IGNORE list
         if( opt_verbose > 2 )
           fprintf(stderr, "SKIP: %s (path)\n", full.c_str());
         IGNORE.line().remove(line, line); // Remove the IGNORE Line*
         delete line;               // Delete it
         continue;                  // And ignore it
       }

       full= path + "/" + file->name;
       if( opt_mode ) {
         mode_t mode= file->st.st_mode & ACCESSPERMS;
         mode_t user= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
         mode_t exec= user    | S_IXUSR | S_IXGRP | S_IXOTH;
         if( mode != exec ) {
           if( opt_auto ) {         // If auto-correct allowed
             mode= file->st.st_mode & ~(ACCESSPERMS);
             mode |= exec;
             chmod(full.c_str(), mode);
             printf("CHMOD Path: %s\n", full.c_str());
           } else {                 // Auto-correct disallowed
             printf("Path: -%s%s%s%s%s%s%s%s%s %s\n"
                    , mode & S_IRUSR ? "r" : "-"
                    , mode & S_IWUSR ? "w" : "-"
                    , mode & S_IXUSR ? "x" : "-"
                    , mode & S_IRGRP ? "r" : "-"
                    , mode & S_IWGRP ? "w" : "-"
                    , mode & S_IXGRP ? "x" : "-"
                    , mode & S_IROTH ? "r" : "-"
                    , mode & S_IWOTH ? "w" : "-"
                    , mode & S_IXOTH ? "x" : "-"
                    , full.c_str());
           }
           allow_multi();
         }
       }

       handle_path(full);           // Process the subdirectory
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       remove_trailing_blanks
//
// Function-
//       Remove trailing blanks, but don't update file
//
//----------------------------------------------------------------------------
static bool                         // TRUE if blanks removed
   remove_trailing_blanks(          // Remove trailing blanks
     Data&             data)        // In this file
{
   bool found= false;
   Line* line= data.line().get_head();
   while( line ) {
     size_t L= strlen(line->text);
     if( L > 0 && line->text[L-1] == ' ' ) {
       if( !found ) {
         found= true;
         printf("File(%s) correct%s line with ending blank(s)\n'%s'\n"
               , data.full().c_str(), opt_auto ? "ed" : "able", line->text);
       }

       if( opt_auto ) {
         while( L > 0 && line->text[L-1] == ' ' )
           --L;
         string S(line->text, L);
         Line* repl= data.get_line(S);
         data.line().insert(line, repl, repl);
         data.line().remove(line, line);
         delete line;
         line= repl;
       }
     }

     line= line->get_next();
   }

   return found;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 argi;        // Argument index

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   init();

   //-------------------------------------------------------------------------
   // List specified directories
   //-------------------------------------------------------------------------
   if( optind >= argc )
     handle_path(".");
   else {
     for(argi= optind; argi<argc; argi++)
       handle_path(argv[argi]);
   }

   //-------------------------------------------------------------------------
   // List extensions
   //-------------------------------------------------------------------------
   if( opt_listx ) {
     typedef pub::Properties::MapIter_t MapIter_t;
     printf("List of file types:\n");
     for(MapIter_t it= props.begin(); it != props.end(); ++it)
       printf("%s\n", it->first.c_str());
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   return(0);
}
