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
//       worder.cpp
//
// Purpose-
//       Search for word match.
//
// Last change date-
//       2023/05/07
//
// Implementation notes-
//       worder table:+++-= abort:==--+ ... (ABATE)
//         '+' indicates yellow letter, '=' indicates green letter
//
//       Duplicate definitions may exist in dictionary. If they match they're
//       reported more than once.
//
//----------------------------------------------------------------------------
#include <list>                     // For std::list
#include <memory>                   // For std::unique_ptr
#include <string>                   // For std::string

#include <stdlib.h>                 // For exit, ...
#include <string.h>                 // For strcmp, ...
#include <sys/stat.h>               // For struct stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/List.h>               // For pub::List
#include <pub/Tokenizer.h>          // For pub::Tokenizer
#include <pub/utility.h>            // For pub::utility methods

using namespace pub::debugging;     // For debugf, ...
using namespace pub::fileman;       // For (typedef) Data, Line, ...
using std::string;                  // For (typedef) string

//----------------------------------------------------------------------------
// Dictionary definition
//----------------------------------------------------------------------------
#include "worder.hpp"               // For Dictionary, ...

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Dictionary      dict;        // (Word list) Dictionary

static int             count= 0;    // Number of letters in target word
static int             opt_debug= 0; // --debug

enum
{  DIM_ALPH= 128                    // Big enough for any ASCII character
,  DIM_CHAR= 128                    // Big enough for any ASCII character
,  DIM_WORD= 128                    // Big enough for any word
};

// Match control by [letter]
static char            maxis[DIM_ALPH]= {}; // Maximum match count in word
static char            minis[DIM_ALPH]= {}; // Minimum match count in word

// Match control by [word]
static char            known[DIM_WORD]= {}; // The known letter values

// Match control by [word][letter]
static char            notat[DIM_WORD][DIM_ALPH]= {}; // Known invalid letters

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Debug the Word list
//
//----------------------------------------------------------------------------
static void
   debug(const char* info= "")      // Debug the Word list
{
   dict.debug(info);

   debugf("\nKnown: '");
   for(int wx= 0; wx < count; ++wx)
     debugf("%c", known[wx] ? known[wx] : '-');
   debugf("'\n");

   debugf("\nLetter table:\n");
   for(int C= 0; C < DIM_ALPH; ++C) {
     if( (C < 'a' || C > 'z') && C != '@' )
       continue;
     string S= "Unknown";
     int maxi= maxis[C];
     if( maxi == 0 ) {
       S= "Does not occur";
       S= "Occurs 0 times";
     } else {
       int mini= minis[C];
       if( mini == maxi )
         S= pub::utility::to_string("Occurs %d time%s", mini
                                   , mini == 1 ? "" : "s");
       else
         S= pub::utility::to_string("Occurs %d..%d times", mini, maxi);
     }
     debugf("[%c] %s\n", C, S.c_str());
   }

   debugf("\n%s %s %s\n", __FILE__, __DATE__, __TIME__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Describe parameters and exit
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr,
     "worder {options} rule ...\n"
     "  List dictionary words known to match all rules\n"
     "\n"
     "Options:\n"
     "  --help\tDisplay this help message and exit\n"
     "  --debug\tDebugging display\n"
     "\n"
     "Rule: LLLLL?????\n"
     "  Where 'L' is '@' or any lower case character between 'a' and 'z'\n"
     "    ('@' does not appear in any word), and\n"
     "  '?' is either '-', '+', '=', or '%%', and\n"
     "    '-' indicates the letter doesn't appear at this position in a word\n"
     "        and, if it's not a duplicate, doesn't appear in any word\n"
     "    '+' indicates the letter appears at another position in a word\n"
     "    '=' indicates the letter appears at this position in a word\n"
     "    '%%' indicates the letter appears at any position in a word\n"
     "\n"
     "Example: worder steam--++- brake--=-+\n"
   );

   exit(EXIT_FAILURE);
}

static void
   info( int )                      // Parameter fault exit
{  fprintf(stderr, "\n"); info(); } // With leading "\n"

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
   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if( argc <= 1 ) {
     fprintf(stderr, "At least one parameter is needed\n");
     info(1);
   }

   if( strcmp(argv[1], "--help") == 0 )
     info();

   int argn= 1;
   if( strcmp(argv[1], "--debug") == 0 ) {
     argn= 2;
     opt_debug= true;
     if( argc < 3 )
       return;
   }

   // Get and verify count, using first parameter
   count= int(strlen(argv[argn]));
   if( count < 2 || (count & 1) ) { // Rule "" is invalid
     fprintf(stderr, "Malformed parameter '%s'\n", argv[argn]);
     info(1);
   }
   count /= 2;

   // Since we know the count, initialize the maxis array
   for(int C= 0; C < DIM_ALPH; ++C)
     maxis[C]= count;

   //=========================================================================
   for(int argi= argn; argi < argc; ++argi) { // For each parameter
     char* parm= argv[argi];
     bool valid= true;
     // Valid letters 'a' through'z' or '.' (an invalid character)
     for(int wx= 0; wx < count; ++wx) {
       if( (parm[wx] < 'a' || parm[wx] > 'z') && parm[wx] != '@' ) {
         valid= false;
         break;
       }
     }

     if( debugging_stop(parm) )     // (Use with gdb)
       printf("%4d STOP: %s\n", __LINE__, parm);

     char* desc= parm + count;
     for(int wx= 0; wx < count; ++wx) { // Valid codes:
       if(    desc[wx] != '-'       // '-' Not present
           && desc[wx] != '+'       // '+' Present, but not at this location
           && desc[wx] != '='       // '=' Present at this location
           && desc[wx] != '%' )     // '%' Present at some location
         valid= false;
     }

     if( desc[count] )              // Descriptor must have correct length
         valid= false;

     if( !valid ) {
       fprintf(stderr, "Malformed parameter '%s'\n", parm);
       info();
     }

     // Initialize letter occurrance counters
     int hits[DIM_ALPH]= {};        // By letter, number of '=,+,%' occurances
     int miss[DIM_ALPH]= {};        // By letter, number of missed occurances
     for(int wx= 0; wx<count; ++wx) {
       int C= parm[wx];
       int D= desc[wx];
       if( D == '=' || D == '+' || D == '%' )
         ++hits[C];
       else /* D == '-' */
         ++miss[C];
     }

     // Cross-check parameters, updating remaining letter occurance count
     for(int wx= 0; wx<count; ++wx) {
       int C= parm[wx];
       int D= desc[wx];
       if( D == '-' ) { //----------------------------------------------------
         // If character can't appear here
         if( minis[C] > hits[C] ) {
           fprintf(stderr, "Argument '%s'[%d] is '%c' but '%c' must be in "
                           "word at least %d time%s\n"
                  , parm, wx+1, D, C, minis[C], minis[C] == 1 ? "" : "s");
           info();
         }
         notat[wx][C]= true;        // Letter can't appear at this position

       } else if( D == '+' ) { //---------------------------------------------
         // If character appears elsewhere in word
         if( known[wx] == parm[wx] ) {
           fprintf(stderr, "Argument '%s'[%d] is '%c' but was already "
                           "defined != '%c'\n"
                  , parm, wx+1, parm[wx], known[wx]);
           info();
         }
         notat[wx][C]= true;        // It appears elsewhere, not here

       } else if( D == '=' ) { //---------------------------------------------
         // If character match found
         if( known[wx] && known[wx] != parm[wx] ) {
           fprintf(stderr, "Argument '%s'[%d] is '%c' but was already "
                           "defined == '%c'\n"
                  , parm, wx+1, parm[wx], known[wx]);
           info();
         }
         known[wx]= parm[wx];
       }
     }

     // Calculate minimum and maximum number of letter occurrances in word
     for(int wx= 0; wx<count; ++wx) {
       int C= parm[wx];
       int H= hits[C];
       if( minis[C] < H )
         minis[C]= H;               // Set new minimum

       if( miss[C] && maxis[C] > H )
         maxis[C]= H;               // Set new maximum
     }
   }

   // UNCHECKED: Special cases
   // Letter can only appear in one spot. (Uses notat table)
   //   (This can also check for some conflicting global rules.)
   //
   // Conflicting global rules:
   //   Too many '+' rules without '=' rule.
   //     Example: Global '+++--' '--+++' for the same letter
   //   (This prevents a null result which might be hard to find.)
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis
   dict.load();                     // Load the dictionary
   if( opt_debug ) {                // If --debug specified
     debug("--debug");
     if( count == 0 )
       return 0;
   }

   //-------------------------------------------------------------------------
   // Find matching words in dictionary
   //-------------------------------------------------------------------------
   for(auto it= dict.begin(); it != dict.end(); ++it) {
     const char* text= (*it).c_str();
     if( debugging_stop(text) )     // (Use with gdb)
       debugf("%4d STOP: %s\n", __LINE__, text);

     if( strlen(text) != size_t(count) ) // Ignore words of incorrect length
       continue;

     bool valid= true;
     for(int wx= 0; wx<count; ++wx) { // For each character
       int C= text[wx];
       if( maxis[C] == 0 ) {        // If word contains disallowed characters
         valid= false;
         break;
       }

       // Insure rule validity (for character position)
       for(int px= 1; px<argc; ++px) { // For each parameter (rule)
         char* parm= argv[px];
         char* desc= parm + count;
         int P= parm[wx];            // The parameter character
         int D= desc[wx];            // The descriptor character
         if( D == '+' || D == '-' ) { // If character cannot appear here
           if( P == C ) {           // It's not valid here
             valid= false;
             break;
           }
         } else if( D == '=' && P != C ) {  // If doesn't match required char
           valid= false;
           break;
         }
       }
       if( !valid )
         break;
     }
     if( !valid )
       continue;

     // Character-by-character rule processing is complete.
     // Verify that the word satisfies all minimum repetition counts
     char hits[DIM_CHAR]= {};       // The character hit count
     for(int wx= 0; wx < count; ++wx) { // Get repetition count by character
       int C= text[wx];
       if( notat[wx][C] ) {
         valid= false;
         break;
       }

       ++hits[C];
     }
     if( !valid )
       continue;

     // Verify repetition counts for all characters, not just those in the word
     for(int C= 0; C < DIM_ALPH; ++C) {
       if( hits[C] > maxis[C] || hits[C] < minis[C] ) {
         valid= false;
         break;
       }
     }

     if( valid )
       printf("%s\n", text);
   }

   return 0;
}
