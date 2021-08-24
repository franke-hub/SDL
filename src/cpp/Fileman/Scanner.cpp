//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Scanner.cpp
//
// Purpose-
//       Source file checker.
//
// Last change date-
//       2021/08/24
//
// Verifications-
//       File permissions. (Auto-correctable)
//       Path permissions. (Auto-correctable)
//       Copyright statement. (Auto-correctable)
//       Copyright year matches last change date year. (Auto-correctable)
//       File line contains trailing blanks. (Auto-correctable)
//       Copyright matches some prototype. (Use --verify)
//
// Auto-correctione-
//       No change or error message occurs unless --verbose >= 0.
//       Option -x enables auto-correct.
//       Only one message or correction unless --multi specified.                           ct
//
// Usage notes-
//       --verbose=-101: Get list of file extention names
//
//       scanner src
//         * Detects errors that cannot be auto-corrected
//         * Adds copyright text check if --verify also specified
//           (A summary count for each detected license type is included)
//
//       scanner src --verbose
//         * Detects ONE correctable error
//         * Detects ALL correctable errors if --multi also specified
//
//       scanner src --verbose -x
//         * Corrects ONE error
//         * Corrects ALL errors if --multi also specified
//
//       scanner --verbose=2 or more
//         * Displays options
//         * Displays TYPE: Filename correspondence if --verify also specified
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
//----------------------------------------------------------------------------
#undef  _GNU_SOURCE                 // For strcasestr
#define _GNU_SOURCE                 // For strcasestr
#include <string>                   // For std::string

#include <assert.h>                 // For bringup debugging
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <stdio.h>                  // For printf, ...
#include <stdlib.h>                 // For exit, ...
#include <string.h>                 // For strcmp, ...
#include <time.h>                   // For localtime, ...

#include <pub/Debug.h>              // For debugging
#include <pub/Properties.h>         // For pub::Properties
#include <pub/Tokenizer.h>          // For pub::Tokenizer
#include <pub/utility.h>            // For pub::utility::to_string
#include "pub/Fileman.h"            // For pub::fileman classes

using namespace pub::debugging;
using namespace pub::fileman;
using std::string;
using pub::Tokenizer;
typedef Tokenizer::Iterator Iterator;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>            // For IFHCDM

#define ALLOW_HTML_EXT false        // Allow html copyrights?
#define OPT_EXTENSIONS -101         // Get list of file extensions

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
// Copyright tables and controls
//----------------------------------------------------------------------------
#define COPY_TYPES 4
static Data*           bash_gpl=  nullptr; // The GPL  bash copyright header
static Data*           bash_lgpl= nullptr; // The LGPL bash copyright header
static Data*           bash_mit=  nullptr; // The MIT  bash copyright header
static Data*           bash_none= nullptr; // The NONE bash copyright header
static int             bash_count[COPY_TYPES]= {0, 0, 0, 0}; // Usage counters

static Data*           code_gpl=  nullptr; // The GPL  code copyright header
static Data*           code_lgpl= nullptr; // The LGPL code copyright header
static Data*           code_mit=  nullptr; // The MIT  code copyright header
static Data*           code_none= nullptr; // The NONE code copyright header
static int             code_count[COPY_TYPES]= {0, 0, 0, 0}; // Usage counters

static Data*           html_gpl=  nullptr; // The GPL  html copyright header
static Data*           html_lgpl= nullptr; // The LGPL html copyright header
static Data*           html_mit=  nullptr; // The MIT  html copyright header
static Data*           html_none= nullptr; // The NONE html copyright header
static int             html_count[COPY_TYPES]= {0, 0, 0, 0}; // Usage counters

static Data*           lily_gpl=  nullptr; // The GPL  lily copyright header
static Data*           lily_sa30= nullptr; // The SA30 lily copyright header
static Data*           lily_sa25= nullptr; // The SA25 lily copyright header
static Data*           lily_none= nullptr; // The NONE lily copyright header
static int             lily_count[COPY_TYPES]= {0, 0, 0, 0}; // Usage counters

static int             misc_count[COPY_TYPES]= {0, 0, 0, 0}; // Usage counters

struct name2data {
   const char*         name;
   Data**              data;
};

static const name2data bash_table[]=
{  {  " GPL", &bash_gpl }
,  {  "LGPL", &bash_lgpl}
,  {  " MIT", &bash_mit }
,  {  "UNLI", &bash_none}
,  {  nullptr, nullptr  }
};

static const name2data code_table[]=
{  {  " GPL", &code_gpl }
,  {  "LGPL", &code_lgpl}
,  {  " MIT", &code_mit }
,  {  "UNLI", &code_none}
,  {  nullptr, nullptr  }
};

static const name2data html_table[]=
{  {  " GPL", &html_gpl }
,  {  "LGPL", &html_lgpl}
,  {  " MIT", &html_mit }
,  {  "UNLI", &html_none}
,  {  nullptr, nullptr  }
};

static const name2data lily_table[]=
{  {  " GPL", &lily_gpl }
,  {  "SA30", &lily_sa30}
,  {  "SA25", &lily_sa25}
,  {  "UNLI", &lily_none}
,  {  nullptr, nullptr  }
};

static const name2data*misc_table= bash_table;

//----------------------------------------------------------------------------
// Option values and controls
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_format= false; // Check/repair file (unix) format
static int             opt_index;   // Option index
static int             opt_multi= false; // Allow multiple changes
static int             opt_verbose= -1; // Verbosity (Higher is more)
static int             opt_verify= false; // Verify copyright
static int             opt_x= false; // Auto-correct extended mode

static const char*     OSTR= ":x";  // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true}
,  {"format",  no_argument,       &opt_format,  true}
,  {"multi",   no_argument,       &opt_multi,   true}
,  {"verify",  no_argument,       &opt_verify,  true}
,  {"verbose", optional_argument, &opt_verbose, true}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP= 0
,  OPT_VERBOSE= 4
,  OPT_SIZE= 5
};

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
//----------------------------------------------------------------------------
#if 0                               // (Included in /usr/include/string.h)
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
#endif                              // (Included in /usr/include/string.h)

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
   string base= HOME + "src/.C";    // The base copyright file path

   bash_gpl=  new Data(base, "B.GPL");  // Load the GPL  copyright
   bash_lgpl= new Data(base, "B.LGPL"); // Load the LGPL copyright
   bash_mit=  new Data(base, "B.MIT");  // Load the MIT  copyright
   bash_none= new Data(base, "B.NONE"); // Load the NONE copyright

   code_gpl=  new Data(base, "C.GPL");  // Load the GPL  copyright
   code_lgpl= new Data(base, "C.LGPL"); // Load the LGPL copyright
   code_mit=  new Data(base, "C.MIT");  // Load the MIT  copyright
   code_none= new Data(base, "C.NONE"); // Load the NONE copyright

   html_gpl=  new Data(base, "H.GPL");  // Load the GPL  copyright
   html_lgpl= new Data(base, ".LICENSE"); // Load the LGPL copyright
   html_mit=  new Data(base, "H.MIT");  // Load the MIT  copyright
   html_none= new Data(base, "H.NONE"); // Load the NONE copyright

   lily_gpl=  new Data(base, "L.GPL");  // Load the GPL  copyright
   lily_sa30= new Data(base, "L.SA30"); // Load the LGPL copyright
   lily_sa25= new Data(base, "L.SA25");  // Load the MIT  copyright
   lily_none= new Data(base, "L.NONE"); // Load the NONE copyright

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
   delete bash_gpl;
   delete bash_lgpl;
   delete bash_mit;
   delete bash_none;

   delete code_gpl;
   delete code_lgpl;
   delete code_mit;
   delete code_none;

   delete html_gpl;
   delete html_lgpl;
   delete html_mit;
   delete html_none;

   delete lily_gpl;
   delete lily_sa30;
   delete lily_sa25;
   delete lily_none;

   //-------------------------------------------------------------------------
   // Verify all IGNORE entries found
   //-------------------------------------------------------------------------
   if( opt_verbose > 2 ) {
     bool once= true;
     for(Line* line= IGNORE.line().get_head(); line; line= line->get_next())
     {
       if( once ) {
         fprintf(stderr, "\n.ignore files not found:\n");
         once= false;
       }

       fprintf(stderr, "%s\n", line->text);
     }
   }
   IGNORE.close();

   //-------------------------------------------------------------------------
   // Display verification statistics
   //-------------------------------------------------------------------------
   if( opt_verify ) {
     printf("\nBash format copyrights:\n");
     for(int i=  0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", bash_table[i].name, bash_count[i]);
     }

     printf("\nCode format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", code_table[i].name, code_count[i]);
     }

if( ALLOW_HTML_EXT ) {
     printf("\nHtml format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       printf("%s: %6d\n", html_table[i].name, html_count[i]);
     }
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
   fprintf(stderr,"Copyright checker\n");

   fprintf(stderr,"\n");
   fprintf(stderr,"Copyright <options>\n");
   fprintf(stderr,"Options:\n"
                   "  -x\t\tAuto-correct mode\n"
                   "  --format\tCheck file format\n"
                   "  --multi\tAllow multiple changes\n"
                   "  --verify\tVerify copyright text\n"
                   "  --verbose\t{=n} Verbosity, 1 if =n unspecified\n"
          );

   exit(2);
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
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
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

           default:
             break;
         }
         break;
       }}}}

       case 'x':
         opt_x= true;
         break;

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

   if( opt_verbose > 1 )
   {
     fprintf(stderr, "-x\t\t%d\n", opt_x);
     fprintf(stderr, "--format\t%d\n", opt_format);
     fprintf(stderr, "--multi\t\t%d\n", opt_multi);
     fprintf(stderr, "--verify\t%d\n", opt_verify);
     fprintf(stderr, "--verbose\t%d\n", opt_verbose);
   }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_opt
//
// Purpose-
//       Display getopt debugging information.
//
//----------------------------------------------------------------------------
static inline void                  // TODO: REMOVE
   debug_opt(                       // Display getopt debugging information
     int               line = 0)    // Caller's line number
{
   const char* opt_name= "<<null>>";
   if( opt_index < OPT_SIZE )
     opt_name= OPTS[opt_index].name;
   else
     opt_name= "<<INVALID>>";
   fprintf(stderr, "%4d index(%d:%s) arg(%s) err(%d) ind(%d) opt(%c)\n",
                   line, opt_index, opt_name, optarg, opterr, optind, optopt);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_copyline
//
// Function-
//       Get copyright line (#3). Error message if not found
//
//----------------------------------------------------------------------------
static inline Line*                 // The third data line, nullptr if none
   get_copyline(                    // Get copyright line
     Data&             data)        // From this Data
{
   Line* line= data.line().get_head();
   if( line ) line= line->get_next();
   if( line ) line= line->get_next();

   int count= 3;
   while( line != nullptr && count > 0 ) {
     if( strcasestr(line->text, "copyright") )
        return line;

     count--;
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
   if( name == ".gitignore" || name == ".README" || name == "README"
       || starts_with(name, "Makefile") )
     return true;

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
   if( ext == "cpp" || ext == "h" || ext == "hpp" || ext == "i" || ext == "c"
       || ext == "cs" || ext == "py" || ext == "java" || ext == "js"
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
   if( ext == "html" || ext == "htm" || ext == "xml" )
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
//       string2int
//
// Function-
//       Convert string to int. Result -1 if invalid
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
//       verify_date_copy
//
// Function-
//       Verify copyright date.
//
//----------------------------------------------------------------------------
static int                          // The copyright year, -1 if invalid
   verify_date_copy(                // Verify copyright date
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
//       verify_date_last
//
// Function-
//       Verify last change date.
//
//----------------------------------------------------------------------------
static void
   verify_date_last(                // Verify last change date
     Data&             data,        // The associated Data object
     int               year)        // The copyright year
{
   string              full= data.full(); // Get the file name

   // Verify copyright year
   if( year < EARLY_YEAR )
     printf("File(%s) Early copyright(%d)\n", full.c_str(), year);

   // Find last change date line
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
     return;
   }

   if( year == l_yy )
     return;

   //-------------------------------------------------------------------------
   // Correctable mismatch detected (and copyright line has been verified.)
   if( data.damaged() || data.changed() ) { // Cannot correct if file problem
     fprintf(stderr, "File(%s) damaged(%d)/changed(%d)\n", full.c_str(),
                     data.damaged(), data.changed());
     return;
   }

   if( opt_verbose >= 0 ) {
     if( opt_x == false ) {
       printf("File(%s) Correctable last(%d) copy(%d)\n",
              full.c_str(), l_yy, year);
     } else {
       Line* line= get_copyline(data); // The copyright line
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

       if( is_error ) {             // Should not occur, but ignorable
         fprintf(stderr, "%4d file(%s) <<PROGRAM FAULT>> copy(%s)\n", __LINE__,
                         full.c_str(), line->text);
         return;
       }

       if( l_yy < f_year ) {        // if last change date < from year
         printf("file(%s) copy(%s) last(%d) not correctable\n",
                full.c_str(), s_year.c_str(), l_yy);
         return;
       }

       t_year= l_yy;                // Use last change date as to_year
       string n_year= pub::utility::to_string("%4d", t_year);
       if( f_year != t_year )
         n_year= pub::utility::to_string("%4d-%4d", f_year, t_year);

       comment + " ";
       if( comment.length() < 9 )
         comment += string(9 - comment.length(), ' ');
       comment += "Copyright (c) " + n_year + " " + owner;

       Line* repl= data.get_line(comment);
       data.line().insert(line, repl, repl);
       data.line().remove(line, line);
       delete line;

       printf("File(%s) Corrected last(%d) copy(%s)\n", full.c_str(),
              l_yy, s_year.c_str());
       data.write();
     }

     if( !opt_multi )
       exit(0);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_copy
//
// Function-
//       Verify copyright line. (Uses verify_date_copy)
//
//----------------------------------------------------------------------------
static int                          // The copyright year, -1 if invalid
   verify_copy(                     // Verify copyright string
     Data&             data,        // The associated Data object
     Line*             line)        // The copyright Line
{
   string              full= data.full(); // Get the file name
   string              text= line->text; // Get the line text
   Tokenizer           tok_line(line->text); // Our line Tokenizer
   Iterator            tok_iter= tok_line.begin();

   // Verify copyright line
   string comment= (tok_iter++)();  // Get/skip leading comment characters
   if( tok_iter() != "Copyright" ) { // If missing copyright statement
     printf("File(%s) (c) Malformed(%s)\n", full.c_str(), text.c_str());
     return -1;
   }

   string S= (++tok_iter)();
   if( S != "(C)" && S != "(c)" ) {
     printf("File(%s) (c) Malformed(%s)\n", full.c_str(), text.c_str());
     return -1;
   }

   if( !ends_with(text, ".") ) {
     printf("File(%s) (c) Missing ending '.'\n", full.c_str());
     return -1;
   }

   string s_year= (++tok_iter)();
   int c_year= verify_date_copy(s_year);
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
       c_year= verify_date_copy(next);
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
     return -1;
   }

   if( data.damaged() || data.changed() ) {
     fprintf(stderr, "File(%s) damaged(%d)/changed(%d)\n",
                     full.c_str(), data.damaged(), data.changed());
     return c_year;
   }

   //-------------------------------------------------------------------------
   // Correctable date detected
   if( opt_verbose >= 0 ) {         // If user is interested
     if( opt_x ) {                  // If auto-correct allowed
       text= comment + " ";
       if( text.length() < 9 )
         text += string(9 - text.length(), ' ');
       text += "Copyright (c) " + next + " " + owner + ".";

       Line* repl= data.get_line(text);
       data.line().insert(line, repl, repl);
       data.line().remove(line, line);
       delete line;

       printf("File(%s) Copyright line corrected\n", full.c_str());
       data.write();
     } else {                         // Auto-correct disallowed
       printf("File(%s) Copyright line correctable\n", full.c_str());
     }
     if( !opt_multi )
       exit(0);
    }

   return c_year;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_info
//
// Function-
//       Verify copyright matches standard.
//
//----------------------------------------------------------------------------
static void
   verify_info(                     // Verify copyright information
     Data&             data)        // For this data
{
   Line* line= get_copyline(data);  // The copyright line
   if( line == nullptr ) {          // If missing
     fprintf(stderr, "File(%s) (c) now missing\n", data.full().c_str());
     return;
   }

   int* count= misc_count;          // Default: MISC format
   const name2data* table= misc_table;
   string file= data.file();
   if( is_code(file) ) {
     count= code_count;
     table= code_table;
   } else if( is_bash(file) ) {     // BASH files: GPL/LGPL disallowed
     count= bash_count + 2;
     table= bash_table + 2;
   } else if( is_lily(file) ) {
     count= lily_count;
     table= lily_table;
   } else if( is_html(file) ) {
     count= html_count;
     table= html_table;
   }

   for(int i= 0; table[i].name; i++) {
     Data* info= *table[i].data;
     Line* lhs= info->line().get_head();
     if( lhs ) lhs= lhs->get_next();
     if( lhs ) lhs= lhs->get_next();
     if( lhs ) lhs= lhs->get_next();
     Line* rhs= line;
     if( rhs ) rhs= rhs->get_next();
     while( lhs ) {
       if( !rhs )
         break;

       if( strcmp(lhs->text+2, rhs->text+2) != 0 )
         break;

       lhs= lhs->get_next();
       rhs= rhs->get_next();
     }
     if( lhs == nullptr ) {
       if( opt_verbose > 1 )
         printf("%s: %s\n", table[i].name, data.full().c_str());

       ++count[i];
       return;
     }
   }

   printf("File(%s) Copyright not valid\n", data.full().c_str());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_bash
//
// Function-
//       Handle a bash (.gitignore/.README/Makefile*) format file
//
//----------------------------------------------------------------------------
static void
   handle_bash(                     // Handle a code format file
     Data&             data)        // The content
{
   if( data.file() == "README" )
     printf("File(%s) named README\n", data.full().c_str());

   Line* line= get_copyline(data);
   if( line == nullptr ) {
     printf("File(%s) (c) Missing\n", data.full().c_str());
     return;
   }

   int c_year= verify_copy(data, line);
   if( c_year > 0 )
     verify_date_last(data, c_year);

   if( opt_verify )
     verify_info(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_code
//
// Function-
//       Handle a code (.cpp,...) format file
//
// Implementation note-
//       This also handles python (.py) and lily (.ly) format files.
//
//----------------------------------------------------------------------------
static void
   handle_code(                     // Handle a code format file
     Data&             data)        // The content
{
   Line* line= get_copyline(data);
   if( line == nullptr ) {
     printf("File(%s) (c) Missing\n", data.full().c_str());
     return;
   }

   int c_year= verify_copy(data, line);
   if( c_year > 0 )
     verify_date_last(data, c_year);

   if( opt_verify )
     verify_info(data);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_data
//
// Function-
//       Handle all file types, looking for trailing blanks
//
//----------------------------------------------------------------------------
static void
   handle_data(                     // Handle all data files
     Data&             data)        // The content
{
   bool found= false;
   Line* line= data.line().get_head();
   while( line ) {
     size_t L= strlen(line->text);
     if( L > 0 && line->text[L-1] == ' ' ) {
       if( !found ) {
         found= true;
         printf("File(%s) correct%s line with ending blank(s)\n'%s'\n"
               , data.full().c_str(), opt_x ? "ed" : "able", line->text);
       }

       if( opt_x ) {
         while( L > 0 && line->text[L-1] == ' ' )
           --L;
         string S(line->text, L);
         Line* repl= data.get_line(S);
         data.line().insert(line, repl, repl);
         data.line().remove(line, line);
         delete line;
         line= repl;
         if( !opt_multi )
           break;
       }
     }

     line= line->get_next();
   }

   if( found ) {
     if( opt_x ) {
       data.write();
       data.change(false);
     }

     if( !opt_multi )
       exit(0);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_html
//
// Function-
//       Handle html files
//
//----------------------------------------------------------------------------
static void
   handle_html(                     // Handle an html file
     Data&             data)        // The content
{  (void)data; // TODO: NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_misc
//
// Function-
//       Handle all other files (No error if missing copyright)
//
//----------------------------------------------------------------------------
static void
   handle_misc(                     // Handle an unspecified file
     Data&             data)        // The content
{
   Line* line= get_copyline(data);
   if( line == nullptr )
     return;

   int c_year= verify_copy(data, line);
   if( c_year > 0 )
     verify_date_last(data, c_year);

   if( opt_verify )
     verify_info(data);
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
     if( opt_verbose == OPT_EXTENSIONS ) {
       string extension= get_extension(file->name);
       if( props.get_property(extension) == nullptr )
         props.insert(extension, extension);

       continue;
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
           fprintf(stderr, "IGNORED File(%s)\n", full.c_str());
         IGNORE.line().remove(line, line); // Remove the IGNORE line
         delete line;               // Delete it
         continue;                  // And ignore it
       }

       if( opt_verbose >= 0 ) {
         mode_t mode= file->st.st_mode & ACCESSPERMS;
         mode_t want= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
         mode_t exec= want    | S_IXUSR | S_IXGRP | S_IXOTH;
         if( mode != want && mode != exec ) {
           if( opt_x && (mode & S_IXUSR) == 0 ) { // If auto-correct allowed
             mode= file->st.st_mode & ~(ACCESSPERMS);
             mode |= want;
             chmod(full.c_str(), mode);
             printf("CHMOD File: %s\n", full.c_str());
           } else {                 // Auto-correct disallowed
             printf("File: -%s%s%s%s%s%s%s%s%s %s\n"
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
           if( !opt_multi )
             exit(0);
         }
       }


       string name(file->name);
       Data data(path, name);
       if( data.damaged() ) {
         fprintf(stderr, "File(%s) Damaged\n", data.full().c_str());
         continue;
       }

       if( opt_format ) {           // Check file format?
         if( data.changed() ) {     // If not in Unix mode
           if( opt_x ) {
             printf("File(%s) ==> unix format\n", data.full().c_str());
             data.write();
             data.change(false);
           } else {
             printf("File(%s) NOT unix format\n", data.full().c_str());
           }
           if( !opt_multi )
             exit(0);
         }
       }

       if( opt_verbose >= 0 )
         handle_data(data);         // Common file checking
       if( is_code(name) )          // Implementation note: Sort likely first
         handle_code(data);
       else if( is_bash(name) )
         handle_bash(data);
       else if( is_lily(name) )
         handle_code(data);
       else if( is_html(name) )
         handle_html(data);
       else
         handle_misc(data);
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
           fprintf(stderr, "IGNORED Path(%s)\n", full.c_str());
         IGNORE.line().remove(line, line); // Remove the IGNORE Line*
         delete line;               // Delete it
         continue;                  // And ignore it
       }

       full= path + "/" + file->name;
       if( opt_verbose >= 0 ) {
         mode_t mode= file->st.st_mode & ACCESSPERMS;
         mode_t want= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
         mode_t exec= want    | S_IXUSR | S_IXGRP | S_IXOTH;
         if( mode != exec ) {
           if( opt_x ) {            // If auto-correct allowed
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
           if( !opt_multi )
             exit(0);
         }
       }

       handle_path(full);           // Process the subdirectory
     }
   }
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
   else
   {
     for(argi= optind; argi<argc; argi++)
       handle_path(argv[argi]);
   }

   //-------------------------------------------------------------------------
   // Handle bringup cases
   //-------------------------------------------------------------------------
   if( opt_verbose == OPT_EXTENSIONS ) {
     typedef pub::Properties::MapIter_t MapIter_t;
     printf("List of file types:\n");
     for(MapIter_t it= props.begin(); it != props.end(); ++it) {
       printf("%s\n", it->first.c_str());
     }

     return 0;
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   return(0);
}
