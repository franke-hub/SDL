//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2026 Frank Eskesen.
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
//       2026/05/12
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
// Usage notes: copyright replacement-
//       Subdirectories ".remove.d" and ".revise.d" (Optionally present):
//       If auto-correct is enabled, a copyright matching .remove.d/XX
//       is converted into .revise.d/XX.
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
#include <cstdlib>                  // For exit, ...
#include <cstring>                  // For strcmp, ...
#include <ctime>                    // For localtime, ...

#include <getopt.h>                 // For getopt_long()
#include <sys/stat.h>               // For struct stat, ...

#include "pub/Data.h"               // For pub::data classes
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List, pub::Sort
#include <pub/Properties.h>         // For pub::Properties
#include <pub/Tokenizer.h>          // For pub::Tokenizer
#include <pub/utility.h>            // For pub::utility::to_string
#include <pub/utility.i>            // For pub::s2c

using namespace pub::debugging;
using std::string;
using pub::s2c;
using pub::Tokenizer;
typedef Tokenizer::Iterator Iterator;

using pub::data::Data;
using pub::data::File;
using pub::data::Line;
using pub::data::Path;
typedef pub::DHDL_list<Line>                  List;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  _USE_AUTOCORRECT_CODE= true      // Use auto_correct_code function?
,  _USE_AUTOCORRECT_HTML= true      // Use auto_correct_html function?
,  _USE_AUTOCORRECT_PREFIX= true    // Use auto_correct_prefix function?
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

static Data*           data_remove= nullptr; // Common: .remove file
static Data*           data_revise= nullptr; // Common: .revise file

static pub::Properties props;       // Used as simple database
static struct tm       tod;         // The current year-corrected time of day

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const string    blanks= "                "; // Blank string

//----------------------------------------------------------------------------
// Copyright tables and controls
//----------------------------------------------------------------------------
#define COPY_TYPES 6
#define MORE_TYPES 8
static Data*           data_none= nullptr; // Common: unmatchable copyright

static Data*           bash_gpl=  nullptr; // The GPL  bash copyright header
static Data*           bash_lgpl= nullptr; // The LGPL bash copyright header
static Data*           bash_mit=  nullptr; // The MIT  bash copyright header
static Data*           bash_mit0= nullptr; // The MIT0 bash copyright header
static Data*           bash_sa40= nullptr; // The SA40 bash copyright header
static Data*           bash_zero= nullptr; // The ZERO bash copyright header
static int             bash_count[COPY_TYPES]= {};

static Data*           code_gpl=  nullptr; // The GPL  code copyright header
static Data*           code_lgpl= nullptr; // The LGPL code copyright header
static Data*           code_mit=  nullptr; // The MIT  code copyright header
static Data*           code_mit0= nullptr; // The MIT0 code copyright header
static Data*           code_sa40= nullptr; // The SA40 code copyright header
static Data*           code_zero= nullptr; // The ZERO code copyright header
static int             code_count[COPY_TYPES]= {};

static Data*           html_gpl=  nullptr; // The GPL  html copyright header
static Data*           html_lgpl= nullptr; // The LGPL html copyright header
static Data*           html_mit=  nullptr; // The MIT  html copyright header
static Data*           html_mit0= nullptr; // The MIT0 html copyright header
static Data*           html_sa40= nullptr; // The SA40 html copyright header
static Data*           html_zero= nullptr; // The ZERO html copyright header
static int             html_count[COPY_TYPES]= {};

static Data*           lily_gpl=  nullptr; // The GPL  lily copyright header
static Data*           lily_lgpl= nullptr; // The LGPL lily copyright header
static Data*           lily_mit=  nullptr; // The MIT  lily copyright header
static Data*           lily_mit0= nullptr; // The MIT0 lily copyright header
static Data*           lily_sa40= nullptr; // The SA40 lily copyright header
static Data*           lily_zero= nullptr; // The ZERO lily copyright header
static int             lily_count[COPY_TYPES]= {};

static Data*           mark_gpl=  nullptr; // The GPL  mark copyright header
static Data*           mark_lgpl= nullptr; // The LGPL mark copyright header
static Data*           mark_mit=  nullptr; // The MIT  mark copyright header
static Data*           mark_mit0= nullptr; // The MIT0 mark copyright header
static Data*           mark_sa40= nullptr; // The SA40 mark copyright header
static Data*           mark_zero= nullptr; // The ZERO mark copyright header
static int             mark_count[COPY_TYPES]= {};

static int             misc_count[COPY_TYPES]= {};

static Data*           more_bsd3= nullptr; // The BSD3 copyright header
static Data*           more_bsl1= nullptr; // The BSL1 copyright header
static Data*           more_gpl=  nullptr; // The GPL  copyright header
static Data*           more_lgpl= nullptr; // The LGPL copyright header
static Data*           more_mit=  nullptr; // The MIT  copyright header
static Data*           more_mit0= nullptr; // The MIT0 copyright header
static Data*           more_sa40= nullptr; // The SA40 copyright header
static Data*           more_zero= nullptr; // The ZERO copyright header
static int             more_count[MORE_TYPES]= {};

static int             none_count= 0; // (Files without copyright statements)

//----------------------------------------------------------------------------
// Name to Data conversion
//----------------------------------------------------------------------------
struct name2data {
   const char*         name;
   Data**              data;
};

static const name2data bash_table[]=
{  {  " GPL", &bash_gpl }
,  {  "LGPL", &bash_lgpl}
,  {  " MIT", &bash_mit }
,  {  "MIT0", &bash_mit0}
,  {  "SA40", &bash_sa40}
,  {  "ZERO", &bash_zero}
,  {  nullptr, nullptr  }
};

static const name2data code_table[]=
{  {  " GPL", &code_gpl }
,  {  "LGPL", &code_lgpl}
,  {  " MIT", &code_mit }
,  {  "MIT0", &code_mit0}
,  {  "SA40", &code_sa40}
,  {  "ZERO", &code_zero}
,  {  nullptr, nullptr  }
};

static const name2data html_table[]=
{  {  " GPL", &html_gpl }
,  {  "LGPL", &html_lgpl}
,  {  " MIT", &html_mit }
,  {  "MIT0", &html_mit0}
,  {  "SA40", &html_sa40}
,  {  "ZERO", &html_zero}
,  {  nullptr, nullptr  }
};

static const name2data lily_table[]=
{  {  " GPL", &lily_gpl }
,  {  "LGPL", &lily_lgpl}
,  {  " MIT", &lily_mit }
,  {  "MIT0", &lily_mit0}
,  {  "SA40", &lily_sa40}
,  {  "ZERO", &lily_zero}
,  {  nullptr, nullptr  }
};

static const name2data mark_table[]=
{  {  " GPL", &mark_gpl }
,  {  "LGPL", &mark_lgpl}
,  {  " MIT", &mark_mit }
,  {  "MIT0", &mark_mit0}
,  {  "SA40", &mark_sa40}
,  {  "ZERO", &mark_zero}
,  {  nullptr, nullptr  }
};

static const name2data*misc_table= bash_table;

static const name2data more_table[]=
{  {  "BSD3", &more_bsd3}
,  {  "BSL1", &more_bsl1}
,  {  " GPL", &more_gpl }
,  {  "LGPL", &more_lgpl}
,  {  " MIT", &more_mit }
,  {  "MIT0", &more_mit0}
,  {  "SA40", &more_sa40}
,  {  "ZERO", &more_zero}
,  {  nullptr, nullptr  }
};

//----------------------------------------------------------------------------
// Remove to revise association
//----------------------------------------------------------------------------
struct remove_revise_item : public pub::DHDL_list<remove_revise_item>::Link {
   Data*               remove= nullptr; // The remove Data
   Data*               revise= nullptr; // The revise Data
};

pub::DHDL_list<remove_revise_item>
                       remove_revise_list; // The remove_revise list

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
,  {"multi",     optional_argument, &opt_multi,      1}
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
,  OPT_MULTI= 6
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

   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_list
//
// Function-
//       Debug a sequence of lines
//
//----------------------------------------------------------------------------
static inline void                  // (Only used when debugging)
   debug_list(                      // Debug a sequence of lines
     const char*       info,        // Information
     const Line*       head,        // The first line
     const Line*       tail)        // The last  line
{
   debugf("\ndebug_list(%s,{%p,%p})\n", info, head, tail);
   const Line* line= head;          // The current line
   for(;;) {                        // Display the sequence
     if( line == nullptr ) {        // If sequence ended without tail
       debugf("*ERROR* tail(%p) not in sequence\n", tail);
       break;
     }
     debugf("[%p] %s\n", line, line->text);
     if( line == tail )
       break;

     line= line->get_next();
   }
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
     const string      str)         // This string
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
     const string      str)         // This string
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
//       trim
//
// Function-
//       Trim leading and trailing blanks
//
//----------------------------------------------------------------------------
static void
   trim(                            // Remove leading and trailing blanks
     string&           str)         // From this string
{
   // Remove leading blanks
   while( str.size() > 0 && str[0] == ' ' ) {
     str= str.substr(1);
   }

   // Remove trailing blanks
   while( str.size() > 1 && str.substr(str.size()-1, 1) == " " ) {
     str= str.substr(0, str.size()-1);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_token
//
// Function-
//       Return next blank-delimited token, updating input string
//
//----------------------------------------------------------------------------
#if 0  // CURRENTLY UNUSED
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
#endif // CURRENTLY UNUSED

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
   List& list= data->line();        // Get the line list
   if( list.get_head() )            // If data present
     return;                        // Everything's OK

   errorf("Error: File(%s) is empty/missing\n", s2c(data->full()));
   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init_remove_revise
//
// Function-
//       Initialize the remove_revise list .
//
//----------------------------------------------------------------------------
static void
   init_remove_revise( void )       // Initialize
{
   Path*               remove= nullptr; // The .remove.d Path
   Path*               revise= nullptr; // The .revise.d Path
   remove_revise_item  rr_item;     // (Working remove_revise item)

   int                 rc;          // (Working return code)

   struct stat info= {};            // STAT information
   rc= stat(".remove.d", &info);
   if( rc == 0 && S_ISDIR(info.st_mode) )
     remove= new Path(".remove.d");
   else if( opt_verbose )
     errorf("Directory(.remove.d) missing or invalid\n");

   rc= stat(".revise.d", &info);
   if( rc == 0 && S_ISDIR(info.st_mode) )
     revise= new Path(".revise.d");
   else if( opt_verbose )
     errorf("Directory(.revise.d) missing or invalid\n");

   if( remove && revise ) {         // If paths found
     pub::DHDL_sort<File>& rem_li= remove->list;
     pub::DHDL_sort<File>& rev_li= revise->list;

     for(auto rem_it= rem_li.begin(); rem_it != rem_li.end(); ++rem_it) {
       File* rem_file= rem_it.get();
       string rem_name= rem_file->name;

       bool match= false;           // Default, no match
       for(auto rev_it= rev_li.begin(); rev_it != rev_li.end(); ++rev_it) {
         File* rev_file= rev_it.get();
         string rev_name= rev_file->name;

         if( rev_name == rem_name ) {
           rr_item.remove= new Data();
           rc= rr_item.remove->open(".remove.d", rem_name);
           if( rc != 0 ) {
             errorf("ERROR: %d= open(%s,%s)\n", rc, ".remove.d", s2c(rem_name));
             delete rr_item.remove;
             break;
           }

           rr_item.revise= new Data();
           rc= rr_item.revise->open(".revise.d", rev_name);
           if( rc != 0 ) {
             errorf("ERROR: %d= open(%s,%s)\n", rc, ".revise.d", s2c(rev_name));
             delete rr_item.remove;
             delete rr_item.revise;
             break;
           } else {
             match= true;
           }
         }
       }
       if( match ) {                // If match found
         remove_revise_item* rr_copy= new remove_revise_item();
         *rr_copy= rr_item;
         remove_revise_list.fifo(rr_copy);
       } else {
         errorf("ERROR: .remove.d/%s without .revise.d/%s\n"
               , s2c(rem_name), s2c(rem_name));
       }
     }
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
// Implementation notes-
//       term() MUST delete only and all new Data(...) statements
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   // Load copyright data
   string base= "/home/data/SDL/.headings"; // The license file directory

   data_none= new Data(base, "README.md"); // Load an unmatchable copyright

   bash_gpl=  new Data(base, "B.GPL");  // Load the GPL  copyright
   bash_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   bash_mit=  new Data(base, "B.MIT");  // Load the MIT  copyright
   bash_mit0= new Data(base, "B.MIT0"); // Load the MIT0 copyright
   bash_sa40= data_none;                // Load the SA40 copyright (disallowed)
   bash_zero= new Data(base, "B.ZERO"); // Load the ZERO copyright

   code_gpl=  new Data(base, "C.GPL");  // Load the GPL  copyright
   code_lgpl= new Data(base, "C.LGPL"); // Load the LGPL copyright
   code_mit=  new Data(base, "C.MIT");  // Load the MIT  copyright
   code_mit0= new Data(base, "C.MIT0"); // Load the MIT0 copyright
   code_sa40= new Data(base, "C.SA40"); // Load the SA40 copyright
   code_zero= new Data(base, "C.ZERO"); // Load the ZERO copyright

   html_gpl=  new Data(base, "H.GPL");  // Load the GPL  copyright
   html_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   html_mit=  new Data(base, "H.MIT");  // Load the MIT  copyright
   html_mit0= new Data(base, "H.MIT0"); // Load the MIT0 copyright
   html_sa40= new Data(base, "H.SA40"); // Load the SA40 copyright
   html_zero= new Data(base, "H.ZERO"); // Load the ZERO copyright

   lily_gpl=  new Data(base, "L.GPL");  // Load the GPL  copyright
   lily_lgpl= data_none;                // Load the LGPL copyright (disallowed)
   lily_mit=  data_none;                // Load the MIT  copyright (disallowed)
   lily_mit0= data_none;                // Load the MIT0 copyright (disallowed)
   lily_sa40= new Data(base, "L.SA40"); // Load the SA40 copyright
   lily_zero= new Data(base, "L.ZERO"); // Load the ZERO copyright

   mark_gpl=  html_gpl;             // Load the GPL  copyright
   mark_lgpl= data_none;            // Load the LGPL copyright (disallowed)
   mark_mit=  html_mit;             // Load the MIT  copyright
   mark_mit0= html_mit0;            // Load the MIT0 copyright
   mark_sa40= html_sa40;            // Load the SA40 copyright
   mark_zero= html_zero;            // Load the ZERO copyright

   more_bsd3= new Data(base, "M.BSD3"); // Load the BSD3 copyright
   more_bsl1= new Data(base, "M.BSL1"); // Load the BSL1 copyright
   more_gpl=  new Data(base, "M.GPL");  // Load the GPL  copyright
   more_lgpl= new Data(base, "M.LGPL"); // Load the LGPL copyright
   more_mit=  new Data(base, "M.MIT");  // Load the MIT  copyright
   more_mit0= new Data(base, "M.MIT0"); // Load the MIT0 copyright
   more_sa40= new Data(base, "M.SA40"); // Load the SA40 copyright
   more_zero= new Data(base, "M.ZERO"); // Load the ZERO copyright

   // Verify data present
   verify_data(data_none);

   verify_data(bash_gpl);
   verify_data(bash_lgpl);
   verify_data(bash_mit);
   verify_data(bash_mit0);
   verify_data(bash_sa40);
   verify_data(bash_zero);

   verify_data(code_gpl);
   verify_data(code_lgpl);
   verify_data(code_mit);
   verify_data(code_mit0);
   verify_data(code_sa40);
   verify_data(code_zero);

   verify_data(html_gpl);
   verify_data(html_lgpl);
   verify_data(html_mit);
   verify_data(html_mit0);
   verify_data(html_sa40);
   verify_data(html_zero);

   verify_data(lily_gpl);
   verify_data(lily_lgpl);
   verify_data(lily_mit);
   verify_data(lily_mit0);
   verify_data(lily_sa40);
   verify_data(lily_zero);

   verify_data(more_bsd3);
   verify_data(more_bsl1);
   verify_data(more_gpl);
   verify_data(more_lgpl);
   verify_data(more_mit);
   verify_data(more_mit0);
   verify_data(more_sa40);
   verify_data(more_zero);

   //-------------------------------------------------------------------------
   // Initialize the remove_revise_list
   //-------------------------------------------------------------------------
   init_remove_revise();

   // Get list of IGNORED files
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
//       term_remove_revise
//
// Function-
//       Deallocate the remove_revise list.
//
//----------------------------------------------------------------------------
static void
   term_remove_revise( void )       // Remove_revise list termination
{
   for(;;) {                        // Delete the remove_revise_list items
     remove_revise_item* rr_item= remove_revise_list.remq();
     if( rr_item == nullptr )
       break;

     delete rr_item->remove;
     delete rr_item->revise;
     delete rr_item;
   }
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
   delete bash_mit0;
// delete bash_sa40;
   delete bash_zero;

   delete code_gpl;
   delete code_lgpl;
   delete code_mit;
   delete code_mit0;
   delete code_sa40;
   delete code_zero;

   delete html_gpl;
// delete html_lgpl;
   delete html_mit;
   delete html_mit0;
   delete html_sa40;
   delete html_zero;

   delete lily_gpl;
// delete lily_lgpl;
// delete lily_mit;
// delete lily_mit0;
   delete lily_sa40;
   delete lily_zero;

   delete more_bsd3;
   delete more_bsl1;
   delete more_gpl;
   delete more_lgpl;
   delete more_mit;
   delete more_mit0;
   delete more_sa40;
   delete more_zero;

   delete data_remove;
   delete data_revise;

   //-------------------------------------------------------------------------
   // Delete the remove_revise_list items
   //-------------------------------------------------------------------------
   term_remove_revise();

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
         errorf("Missing .ignores:\n");
       size_t L= strlen(line->text);
       if( L > 1 && strcmp(&line->text[L-2], "/*") == 0 ) { // If ignored path
         errorf("Path: %s\n", line->text);
         ++paths;
       } else {                     // If ignore file
         errorf("File: %s\n", line->text);
         ++files;
       }
     }
     if( paths == 0 )
       debugf("*ALL* .ignore paths found\n");
     else
       errorf("%5d .ignore path%s not found\n", paths
             , paths == 1 ? "" : "s");

     if( files == 0 )
       debugf("*ALL* .ignore files found\n");
     else
       errorf("%5d .ignore file%s not found\n", files, files == 1 ? "" : "s");
   }
   IGNORE.close();

   //-------------------------------------------------------------------------
   // Display verification statistics
   //-------------------------------------------------------------------------
   if( true ) {
     debugf("\nBash format copyrights:\n");
     for(int i=  0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", bash_table[i].name, bash_count[i]);
     }

     debugf("\nCode format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", code_table[i].name, code_count[i]);
     }

     debugf("\nHtml format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", html_table[i].name, html_count[i]);
     }

     debugf("\nLily format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", lily_table[i].name, lily_count[i]);
     }

     debugf("\nMark format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", mark_table[i].name, mark_count[i]);
     }

     debugf("\nMisc format copyrights:\n");
     for(int i= 0; i < COPY_TYPES; ++i ) {
       debugf("%s: %6d\n", misc_table[i].name, misc_count[i]);
     }

     debugf("\nMore format copyrights:\n");
     for(int i= 0; i < MORE_TYPES; ++i ) {
       debugf("%s: %6d\n", more_table[i].name, more_count[i]);
     }

     // No copyright found
     debugf("NONE: %6d\n", none_count);
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
   errorf("\n");
   errorf("Scanner {path} <options>\n");
   errorf("Options:\n"
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
       errorf("--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       errorf("--%s, no value specified\n", OPTS[opt_index].name);
     else
       errorf("--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
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
           case OPT_MULTI:
             if( optarg )
               opt_multi= parm_int();
             break;

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
           errorf("Option requires an argument '%s'\n", argv[optind-1]);
         else
           errorf("Option requires an argument '-%c'\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           errorf("Unknown option '%s'\n", argv[optind-1]);
         else if( isprint(optopt) )
           errorf("Unknown option '-%c'\n",optopt);
         else
           errorf("Unknown option character '0x%x'\n", optopt);
         break;

       default:
         errorf("%4d ShouldNotOccur ('%c',0x%x)\n", __LINE__, C, C);
         break;
     }
   }

   if( opt_help )
     info();

   if( opt_verbose ) {
     debugf("%5d --verbose\n", opt_verbose);
     debugf("%5s --auto\n",    opt_auto  ? " true" : "false");
     debugf("%5s --copy\n",    opt_copy  ? " true" : "false");
     debugf("%5s --listx\n",   opt_listx ? " true" : "false");
     debugf("%5s --mode\n",    opt_mode  ? " true" : "false");
     debugf("%5d --multi\n",   opt_multi);
     debugf("%5s --unix\n",    opt_unix  ? " true" : "false");
     debugf("\n");
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
   const char* S= s2c(name);        // Get the associated string
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
//       has_copyright
//
// Function-
//       Test exact copyright match.
//
//----------------------------------------------------------------------------
static bool                         // TRUE if copyright files match
   has_copyright(                   // Does file have copyright?
     Data&             data,        // For this data file
     Data*             copy)        // And this copyright file
{
   Line* lhs= get_copy_line(*copy);
   if( lhs == nullptr )
     return false;
   lhs= lhs->get_next();
   if( lhs == nullptr )
     return false;

   Line* rhs= get_copy_line(data);
   if( rhs == nullptr )
     return false;
   rhs= rhs->get_next();
   while( lhs ) {
     if( !rhs )                     // If file ends inside copyright
       return false;                // (No match)

     // Compare line for line, including prefix
     if( strcmp(lhs->text, rhs->text) != 0 )
       return false;

     lhs= lhs->get_next();
     rhs= rhs->get_next();
   }

   return true;
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
       || ext == "a"
       || ext == "avi" || ext == "AVI"
       || ext == "bmp"
       || ext == "db"
       || ext == "dll"
       || ext == "exe"
       || ext == "gif"
       || ext == "gpg"
       || ext == "gz"
       || ext == "jar"
       || ext == "jpeg" || ext == "JPEG"
       || ext == "jpg" || ext == "JPG"
       || ext == "mov" || ext == "MOV"
       || ext == "mp4" || ext == "MP4"
       || ext == "o"
       || ext == "odt"
       || ext == "pcx"
       || ext == "pdf"
       || ext == "png" || ext == "PNG"
       || ext == "pyc"
       || ext == "swf"
       || ext == "tif"
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
//       Is the specified file in html or xml format?
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
   const char* S= s2c(inps);
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
   const char* S= s2c(inps);
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
//       replace_copyright
//
// Function-
//       Replace copyright text
//
//----------------------------------------------------------------------------
static void
   replace_copyright(               // Replace copyright text
     Data&             data,        // For this data file
     Data*             have_copy,   // Having this copyright
     Data*             want_copy)   // Replacement copyright
{
   if( HCDM )
     debugf("replace_copyright(%s,%s,%s)\n", s2c(data.full())
           , s2c(have_copy->full()), s2c(want_copy->full()));

   typedef pub::DHDL_list<Line>     List; // The Data's line list type

   Line* lhs= get_copy_line(*have_copy)->get_next();
   Line* rhs= get_copy_line(data)->get_next();
   Line* head= rhs;
   Line* tail= rhs;
   for(;;) {
     lhs= lhs->get_next();
     if( lhs == nullptr )
       break;

     tail= tail->get_next();
   }
   Line* after= head->get_prev();   // (The line to insert after)

   List& list= data.line();         // (The File's line list)
   list.remove(head, tail);
   while( head != tail ) {          // Delete the removed lines
     Line* next= head->get_next();
     delete head;
     head= next;
   }

   List insert;                     // The replacement list
   lhs= get_copy_line(*want_copy)->get_next();
   while( lhs ) {
     insert.fifo( data.get_line(lhs->text) );
     lhs= lhs->get_next();
   }
   list.insert(after, insert.get_head(), insert.get_tail());

   data.write();
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
//       verify_copy_ending
//
// Function-
//       Verify copyright has a token ending with "."
//
//----------------------------------------------------------------------------
static void
   verify_copy_ending(              // Verify copyright token ending
     const string      file,        // The file name
     const Line*       line)        // The copyright line
{
   Tokenizer lt(line->text);        // Our line Tokenizer
   for(Iterator ti= lt.begin(); ti != lt.end(); ++ti) {
     if( ends_with(ti(), ".") )
       return;
   }

   errorf("File(%s) (c) Missing ending '.'\n", s2c(file));
   allow_multi();
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
     errorf("File(%s) Early copyright(%d)\n", s2c(full), year);

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
     errorf("File(%s) Missing last change date\n", s2c(full));
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
     errorf("File(%s) Malformed last change date(%s)\n",
            s2c(full), s2c(date));
     allow_multi();
     return;
   }

   bool future= false;
   if( l_yy > tod.tm_year || year > tod.tm_year ) {
     future= true;
   } else if( l_yy == tod.tm_year ) {
     if( l_mm > tod.tm_mon ) {
       future= true;
     } else if( l_mm == tod.tm_mon ) {
       if( l_dd > tod.tm_mday ) {
         future= true;
       }
     }
   }

   if( future ) {
     errorf("File(%s) Future copy(%d) last(%s)\n", s2c(full), year, text);
     allow_multi();
     return;
   }

   if( year == l_yy )
     return;

   //-------------------------------------------------------------------------
   // Correctable mismatch detected (and copyright line has been verified.)
   if( data.damaged() || data.changed() ) { // Cannot correct if file problem
     errorf("File(%s) damaged(%d)/changed(%d)\n", s2c(full)
           , data.damaged(), data.changed());
     allow_multi();
     return;
   }

   if( opt_auto == false ) {
     errorf("File(%s) Correctable last(%d) copy(%d)\n", s2c(full), l_yy, year);
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
       errorf("%4d File(%s) Copy(%s) Invalid\n"
             , __LINE__, s2c(full), line->text);
       allow_multi();
       return;
     }

     if( l_yy < f_year ) {        // if last change date < from year
       errorf("file(%s) copy(%s) last(%d) not correctable\n"
             , s2c(full), s2c(s_year), l_yy);
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
     errorf("File(%s) Corrected last(%d) copy(%s)\n", s2c(full)
           , l_yy, s2c(s_year));
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

   // Verify copyright line: must begin with a token or a blank
   string comment= "  ";
   if( text[0] != ' ' )             // If a leading token exists
     comment= (tok_iter++)();       // Get/skip leading comment token
   if( tok_iter() != "Copyright" ) { // If missing copyright statement
     errorf("File(%s) Line '%s' is missing 'Copyright' token\n"
           , s2c(full), s2c(text));
     allow_multi();
     return -1;
   }

   // Verify the copyright symbol
   string S= (++tok_iter)();
   if( S != "(C)" && S != "(c)" ) {
     errorf("File(%s) (c) Malformed(%s)\n", s2c(full), s2c(text));
     allow_multi();
     return -1;
   }

   // Verify that the copyright statement ends with a "."
   verify_copy_ending(full, line);

   // Verify the copyright year
   string s_year= (++tok_iter)();
   int c_year= verify_copy_date(s_year);
   if( c_year > 0 ) {
     if( OWNER_NAME != "" ) {
       string owner((++tok_iter).remainder());
       if( owner != OWNER_NAME )
         errorf("File(%s) (c) Non-standard owner(%s)\n", s2c(full), s2c(owner));
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
     errorf("File(%s) Invalid (c) date(%s)\n", s2c(full), s2c(text));
     allow_multi();
     return -1;
   }

   if( data.damaged() || data.changed() ) {
     errorf("%4d File(%s) damaged(%d)/changed(%d)\n", __LINE__
           , s2c(full), data.damaged(), data.changed());
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
     debugf("File(%s) Copyright line corrected\n", s2c(full));
   } else {                         // Auto-correct disallowed
     errorf("File(%s) Copyright line correctable\n", s2c(full));
   }
   allow_multi();

   return c_year;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_code
//
// Function-
//       Auto-convert code files with "SA40" copyright into "GPL" copyright
//
//----------------------------------------------------------------------------
static void
   auto_correct_code(               // Automatically correct code files
     Data&             data,        // For this data file
     Data*             copy,        // And this copyright file
     const string      type)        // And this copyright type
{
   if( _USE_AUTOCORRECT_CODE && is_code(data.file()) ) {
     (void)copy;                    // (Auto-correct not implemented)

     if( HCDM )
       debugf("auto_correct_code(%s,%s,%s)\n"
             , s2c(data.full()), s2c(copy->full()), s2c(type));

     // Disallow "SA40" copyright (but allow for java)
     if( type == "SA40" && get_extension(data.file()) != "java" ) {
       errorf("Code file(%s) has disallowed SA40 copyright\n"
             , s2c(data.full()));
       allow_multi();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_html
//
// Function-
//       Auto-correct html files with "GPL" or "MIT" copyright
//
//----------------------------------------------------------------------------
static void
   auto_correct_html(               // Automatically correct html files
     Data&             data,        // For this data file
     Data*             copy,        // And this copyright file
     const string      type)        // And this copyright type
{
   if( _USE_AUTOCORRECT_HTML &&is_html(data.file()) ) {
     if( HCDM )
       debugf("auto_correct_html(%s,%s,%s)\n"
             , s2c(data.full()), s2c(copy->full()), s2c(type));

     const char* disallowed= nullptr; // The disallowed type
     if( type == " GPL" ) {         // HTML cannot have "GPL" format
       disallowed= " GPL";
     } else
     if( type == " MIT" ) {         // HTML cannot have "MIT" format
       disallowed= " MIT";
     }

     if( disallowed ) {             // If disallowed type detected
       if( opt_auto ) {             // If auto-correcting
         replace_copyright(data, copy, html_sa40);
         debugf("File(%s) copyright(%s=>SA40)\n"
               , s2c(data.full()), s2c(type));
       } else {
         errorf("File(%s) has disallowed %s copyright\n"
              , s2c(data.full()), disallowed);
       }

       allow_multi();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_mark
//
// Function-
//       Auto-correct markdown files: require "SA40" copyright
//
//----------------------------------------------------------------------------
static void
   auto_correct_mark(               // Automatically correct markdown files
     Data&             data,        // For this data file
     Data*             copy,        // And this copyright file
     const string      type)        // For this data type name
{
   if( HCDM )
     debugf("auto_correct_mark(%s,%s,%s)\n"
           , s2c(data.full()), s2c(copy->full()), s2c(type));

   if( is_mark(data.file()) ) {
     if( type != "SA40" ) {
       if( opt_auto ) {             // If auto-correcting
         replace_copyright(data, copy, html_sa40);
         debugf("Markdown file(%s) format(%s=>SA40)\n"
               , s2c(data.full()), s2c(type));

       } else {
         errorf("Markdown file(%s) requires SA40 copyright\n"
               , s2c(data.full()));
       }

       allow_multi();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_prefix
//
// Function-
//       Auto-correct inconsistent prefix
//
//----------------------------------------------------------------------------
static void
   auto_correct_prefix(             // Automatically correct prefix
     Data&             data,        // For this data file
     Data*             copy,        // And this copyright file
     const string      prefix)      // And this prefix
{
   if( _USE_AUTOCORRECT_PREFIX ) {  // If function is enabled
   if( HCDM )
       debugf("auto_correct_prefix(%s,%s,%s)\n"
             , s2c(data.full()), s2c(copy->full()), s2c(prefix));

     typedef pub::DHDL_list<Line>     List; // The Data's line list type

     Tokenizer tokenizer(prefix, " ");
     Iterator  iterator= tokenizer.begin();
     iterator.set_quote(false);
     string prefix_token= iterator();

     Line* lhs= get_copy_line(*copy)->get_next();
     Line* rhs= get_copy_line(data)->get_next();
     bool  corrected= false;
     while( lhs ) {
       bool is_valid= false;        // Default, invalid format
       string rh_str= rhs->text;
       if( rh_str == prefix_token ) { // If no data except for prefix_token
         is_valid= true;
       } else if( starts_with(rh_str, prefix_token) ) {
         if( starts_with(rh_str, (prefix_token + " SPDX-License")) )
           is_valid= true;
         else if( starts_with(rh_str, prefix) ) {
           if( rh_str[prefix.size()] != ' ' ) // If consistent alignment
             is_valid= true;
           else if( opt_verbose > 1 ) // ERROR: Inconsistent alignment
             errorf("ERROR: alignment inconsistent\n");
         }
       } else if( opt_verbose > 1 ) { // ERROR: Does not start with prefix_token
         errorf("ERROR: prefix_token(%s) line(%s)\n"
               , s2c(prefix_token), rhs->text);
       }

       if( !is_valid ) {
         if( opt_auto ) {             // If auto-correct allowed
           string old_text= rhs->text;
           string new_text= prefix_token;
           string lh_str= skipb(findb(lhs->text)); // Get the associated text
           if( skipb(lh_str) != "" ) { // If associated text exists
             if( starts_with(lh_str, "SPDX-License-Identifier:") )
               new_text= prefix_token + " " + lh_str;
             else
               new_text= prefix + lh_str;
           }
           Line* line= data.get_line(new_text);

           // Correct the prefix
           List& list= data.line();   // (The File's line list)
           list.insert(rhs->get_prev(), line);
           list.remove(line->get_next());

           if( !corrected ) {
             corrected= true;
             debugf("File(%s) prefix modified:\n", s2c(data.full()));
           }
           debugf("old: '%s'\nnew: '%s'\n", s2c(old_text), line->text);
         } else {
           errorf("File(%s) Inconsistent copyright format (unchanged)\n"
                 , s2c(data.full()));
           allow_multi();
         }
       }

       lhs= lhs->get_next();
       rhs= rhs->get_next();
     }

     if( corrected ) {
       data.write();
       allow_multi();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_spdx
//
// Function-
//       Handle missing "SPDX-License-Identifier:"
//
//----------------------------------------------------------------------------
static bool                         // TRUE if SPDX identifier added
   auto_correct_spdx(               // Handle missing SPDX identifier
     Data&             data,        // For this data file
     const char*       type,        // And this copyright type
     const string&     prefix,      // And this copyright prefix
     Line*             lhs,         // And this copyright Line
     Line*             rhs)         // And this File Line
{
   if( HCDM )
     debugf("auto_correct_spdx(%s,%s)\n", s2c(data.full()), type);

   bool changed= false;             // Default, not changed

   if( rhs && memcmp(lhs->text+3, "SPDX-License-Identifier:", 24) == 0 ) {
     if( opt_auto ) {               // If auto-correct allowed
       List& list= data.line();     // (The file line list)

       string S(prefix);
       trim(S);
       S += " ";
       S += skipb(findb(lhs->text));
       Line* line= data.get_line(S); // (Adding the SPDX-License-Identifier)
       list.insert(rhs->get_prev(), line);

       data.write();
       debugf("File(%s) %s SPDX-License-Identifier added\n"
             , s2c(data.full()), type);
       changed= true;
     } else {                       // Auto-correct not allowed
       errorf("File(%s) %s SPDX-License-Identifier missing\n"
             , s2c(data.full()), type);
     }

     if( opt_multi < 0 )            // If stricter multi enforcement
       exit(1);
     allow_multi();
   }

   return changed;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_correct_type
//
// Function-
//       Make sure type is valid for file
//
//----------------------------------------------------------------------------
static void
   auto_correct_type(               // Automatically correct code files
     Data&             data,        // For this data file
     Data*             copy,        // And this copyright file
     const string      type)        // And this copyright type
{
   if( HCDM )
     debugf("auto_correct_type(%s,%s,%s)\n"
           , s2c(data.full()), s2c(copy->full()), s2c(type));

   // .gitignore files must have "MIT0" format
   if( true && data.file() == ".gitignore" ) {
     if( type == "MIT0" )           // If already "MIT-0" format
       return;

     if( opt_auto ) {               // If auto-correcting
       replace_copyright(data, copy, bash_mit0);
       debugf("File(%s) copyright(%s=>MIT0)\n"
             , s2c(data.full()), s2c(type));
     } else {
       errorf("File(%s) requires MIT-0 copyright\n", s2c(data.full()));
     }

     allow_multi();
     return;
   }

   // HTML files cannot have "GPL", "MIT", or "MIT0" format
   if( true && is_html(data.file()) ) {
     const char* disallowed= nullptr; // The disallowed type
     if( type == " GPL" ) {
       disallowed= "GPL";
     } else
     if( type == " MIT" ) {
       disallowed= "MIT";
     } else
     if( type == "MIT0" ) {
       disallowed= "MIT0";
     }

     if( disallowed ) {             // If disallowed type detected
       if( opt_auto ) {             // If auto-correcting
         replace_copyright(data, copy, html_sa40);
         debugf("File(%s) copyright(%s=>SA40)\n"
               , s2c(data.full()), s2c(type));
       } else {
         errorf("File(%s) has disallowed %s copyright\n"
              , s2c(data.full()), disallowed);
       }

       allow_multi();
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       auto_update
//
// Function-
//       Handle copyright .remove => .revise update
//
//----------------------------------------------------------------------------
static bool                         // TRUE if copyright converted
   auto_update(                     // Handle copyright update
     Data&             data)        // For this data file
{
   if( HCDM )
     debugf("auto_update(%s)\n", s2c(data.full()));

   bool changed= false;             // Default, not changed
   remove_revise_item* rr_item= remove_revise_list.get_head();
   while( rr_item ) {
     if( has_copyright(data, rr_item->remove) ) {
       if( opt_auto ) {             // If auto-correcting
         replace_copyright(data, rr_item->remove, rr_item->revise);
         data.write();
         debugf("File(%s) copyright updated\n", s2c(data.full()));
         changed= true;
       } else {
         errorf("File(%s) copyright update required\n", s2c(data.full()));
       }

       if( opt_multi < 0 )          // If stricter multi enforcement
         exit(1);
       allow_multi();
       break;
     }

     rr_item= rr_item->get_next();
   }

   return changed;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify_copy_text
//
// Function-
//       Verify that the copyright matches a supported one.
//
// Implementation notes-
//       For miscellaneous extensions, if the copyright date line isn't found
//       this function is not invoked.
//
//----------------------------------------------------------------------------
static void
   verify_copy_text(                // Verify copyright text
     Data&             data)        // For this data
{
   Line* line= get_copy_line(data); // The copyright line
   if( line == nullptr ) {          // If missing
     errorf("File(%s) Copyright missing\n", s2c(data.full()));
     allow_multi();
     return;
   }

   string       data_full= data.full(); // (The string must be persistent)
   const char*       full= s2c(data_full); // Fully qualified file name
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
   } else if( is_mark(file) ) {
     count= mark_count;
     table= mark_table;
   } else {                         // Miscellaneous (misc_count, misc_table)
     if( false ) {                  // (Not an error)
       debugf("File(%s) MISC format\n", full);
       allow_multi();
     }
   }

   string prefix= line->text;       // (The file's copyright line)
   prefix= prefix.substr(0, strcasestr(line->text, "copyright") - line->text);
   for(int i= 0; table[i].name; i++) {
     Data* copy= *table[i].data;
     Line* lhs= get_copy_line(*copy)->get_next();
     if( lhs == nullptr ) {
       errorf("Table(%s) invalid, exiting\n", table[i].name);
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

     if( lhs == nullptr ) {         // If copyright text matched
       if( HCDM )
         debugf("copyright_match(%s,%s)\n", s2c(data.full()), table[i].name);

       // Handle special cases
       string type= table[i].name;  // The copyright type
       auto_correct_code(data, copy, type); // Auto-detect code files
       auto_correct_html(data, copy, type); // Auto-detect html files
       auto_correct_mark(data, copy, type); // Auto-correct markdown files
       auto_correct_type(data, copy, type); // Auto-detect type mismatch

       auto_correct_prefix(data, copy, prefix); // Correct prefix inconsistency

       // Update match count
       if( opt_verbose > 2 )
         debugf("[%s]: '%s'\n", table[i].name, full);

       ++count[i];
       return;
     }

     // Copyright text did not match any of the supported versions.
     // Auto-correct missing SPDX identifier
     if( auto_correct_spdx(data, table[i].name, prefix, lhs, rhs) )
       return;
   }

   // No copyright match detected
   if( HCDM )
     debugf("non-standard copyright(%s)\n", s2c(data.full()));

   if( auto_update(data) )          // Auto-correct remove/replace copyrights
     return;

   // Check for other copyright formats
   for(int i= 0; more_table[i].name; ++i) {
     Data* copy= *more_table[i].data;
     Line* lhs= get_copy_line(*copy);
     if( lhs == nullptr ) {         // (Should not occur)
       errorf("More(%s) invalid, exiting\n", more_table[i].name);
       exit(1);
     }

     lhs= lhs->get_next();
     Line* rhs= line->get_next();

     // (LHS: copyright line; RHS: file line) Position: copy_line->get_next()
     // Verify the copyright text
     while( lhs ) {
       if( !rhs )                   // If file ends inside copyright
         break;

       // Compare line for line, ignoring prefix
       string lh_str= lhs->text;    // No prefix in copyright file
       string rh_str= rhs->text;
       if( rh_str.size() > 0 && rh_str[0] != ' ' )
         rh_str= skipb(findb(rhs->text));

       if( lh_str.size() > rh_str.size() )
         break;

       rh_str= rh_str.substr(0, lh_str.size());
       if( lh_str != rh_str )
         break;

       lhs= lhs->get_next();
       rhs= rhs->get_next();
     }

     if( lhs == nullptr ) {         // If copyright found
       // Update match count
       if( opt_verbose > 1 )
         debugf("[%s]: '%s'\n", more_table[i].name, full);

       // Handle special cases
       string type= more_table[i].name; // The copyright type
       auto_correct_code(data, copy, type); // Auto-detect code files
       auto_correct_html(data, copy, type); // Auto-detect html files
       auto_correct_mark(data, copy, type); // Auto-correct markdown files
       auto_correct_type(data, copy, type); // Auto-detect type mismatch

       ++more_count[i];
       return;
     }
   }

   errorf("File(%s): No copyright match\n", full);
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
     errorf("File(%s) named README\n", s2c(data.full()));
     allow_multi();
   }

   Line* line= get_copy_line(data);
   if( line == nullptr ) {
     errorf("File(%s) (c) Missing\n", s2c(data.full()));
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
     errorf("File(%s) (c) Missing\n", s2c(data.full()));
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
     errorf("File(%s) (c) Missing\n", s2c(data.full()));
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
//       copy_mark
//
// Function-
//       Handle an mark file copyright
//
//----------------------------------------------------------------------------
static void
   copy_mark(                       // Handle an mark file copyright
     Data&             data)        // The content
{
   Line* line= get_copy_line(data);
   if( line == nullptr ) {
     errorf("File(%s) (c) Missing\n", s2c(data.full()));
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
   if( line == nullptr ) {          // (Missing copyright allowed)
     ++none_count;
     if( opt_verbose > 1 )
       errorf("[NONE]: '%s'\n", s2c(data.full()));
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
     debugf("D: %s\n", s2c(path));

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
       debugf("F: %.8x %10ld %s/%s\n", file->st.st_mode
             , file->st.st_size, s2c(path), s2c(file->name));

     if( S_ISREG(file->st.st_mode) ) {
       string full= path + "/" + file->name; // The fully qualified name
       for(line= IGNORE.line().get_head(); line; line= line->get_next())
       {
         if( strcmp(line->text, s2c(full)) == 0 ) // If IGNORE file
           break;
       }
       if( line ) {                 // If IGNORE file
         if( opt_verbose > 2 )
           debugf("SKIP: %s (file)\n", s2c(full));
         IGNORE.line().remove(line, line); // Remove the IGNORE line
         delete line;               // Delete it
         continue;                  // And ignore it
       }

       string name(file->name);
       if( is_binary(name) )        // Ignore binary formatted file types
         continue;

       Data data(path, name);
       if( data.damaged() ) {
         errorf("File(%s) Damaged\n", s2c(data.full()));
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
           errorf("File(%s) mode(%.3o) want(%.3o)\n"
                 , s2c(full), mode, want);
           if( !opt_auto || (mode & S_IWUSR) == 0 ) { // If can't auto-correct
             errorf("Mode: -%s%s%s%s%s%s%s%s%s unchanged\n"
                   , mode & S_IRUSR ? "r" : "-"
                   , mode & S_IWUSR ? "w" : "-"
                   , mode & S_IXUSR ? "x" : "-"
                   , mode & S_IRGRP ? "r" : "-"
                   , mode & S_IWGRP ? "w" : "-"
                   , mode & S_IXGRP ? "x" : "-"
                   , mode & S_IROTH ? "r" : "-"
                   , mode & S_IWOTH ? "w" : "-"
                   , mode & S_IXOTH ? "x" : "-"
                   );
           } else {                 // Auto-correct
             mode= file->st.st_mode & ~(ACCESSPERMS);
             mode |= want;
             chmod(s2c(full), mode);
             debugf("CHMOD File: %s\n", s2c(full));
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
               debugf("File(%s) ==> unix format\n", s2c(data.full()));
             data.write();
             data.change(false);
           } else {
             if( had_change )
               errorf("File(%s) NOT IN unix format\n", s2c(data.full()));
             errorf("File(%s) unchanged\n", s2c(data.full()));
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
         else if( is_mark(name) )
           copy_mark(data);
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
           debugf("SKIP: %s (path)\n", s2c(full));
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
             chmod(s2c(full), mode);
             debugf("CHMOD Path: %s\n", s2c(full));
           } else {                 // Auto-correct disallowed
             errorf("Path: -%s%s%s%s%s%s%s%s%s %s\n"
                    , mode & S_IRUSR ? "r" : "-"
                    , mode & S_IWUSR ? "w" : "-"
                    , mode & S_IXUSR ? "x" : "-"
                    , mode & S_IRGRP ? "r" : "-"
                    , mode & S_IWGRP ? "w" : "-"
                    , mode & S_IXGRP ? "x" : "-"
                    , mode & S_IROTH ? "r" : "-"
                    , mode & S_IWOTH ? "w" : "-"
                    , mode & S_IXOTH ? "x" : "-"
                    , s2c(full));
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
         errorf("File(%s) correct%s line with ending blank(s)\n'%s'\n"
               , s2c(data.full()), opt_auto ? "ed" : "able", line->text);
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
     debugf("List of file types:\n");
     for(MapIter_t it= props.begin(); it != props.end(); ++it)
       debugf("%s\n", s2c(it->first));
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   return(0);
}
