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
//       worder.cpp
//
// Purpose-
//       Search for word match.
//
// Last change date-
//       2022/09/19
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
// Constants for parameterization
//----------------------------------------------------------------------------
static constexpr const char*
                       DEBUGGING_STOP_WORD= "*";
//                       DEBUGGING_STOP_WORD= "error";

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  NOSUGGEST= '!'                   // TODO: Remove hard-coding
,  USE_DUPLICATE_DETECT= true       // Detect duplicate inserts?
}; // enum

// std::string typedefs and constants
typedef string::size_type size_type;
static const size_type npos= string::npos;

//----------------------------------------------------------------------------
//
// Struct-
//       affix_rule
//
// Purpose-
//       The prefix/suffix rule descriptor
//
//----------------------------------------------------------------------------
struct affix_rule {
string                 remove;      // The remove string
string                 insert;      // The insert string
string                 ifrule;      // The applicable ending rule
}; // struct affix_rule

//----------------------------------------------------------------------------
//
// Struct-
//       affix_head
//
// Purpose-
//       The prefix/suffix control entry
//
//----------------------------------------------------------------------------
struct affix_head {
std::list<affix_rule>  list;        // The rule list
int                    index= 0;    // The rule index
bool                   paired= true; // Combinable prefix/suffix?
bool                   prefix= false; // Prefix || suffix?
}; // struct affix_head

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef pub::DHDL_list<pub::fileman::Line>    data_list;

enum
{  DIM_ALPH= 'z' - 'a' + 1          // Big enough for the 'a'..'z' alphabet
,  DIM_CHAR= 128                    // Big enough for any ASCII char
,  DIM_WORD= 128                    // Big enough for any word
};

// By [letter]
// static char            avoid[DIM_ALPH]= {}; // Letters NOT in word
static char            maxis[DIM_ALPH]= {}; // Maximum match count in word
static char            minis[DIM_ALPH]= {}; // Minimum match count in word

// By [word]
static char            known[DIM_WORD]= {}; // The known letter values

// By [word][letter]
static char            notat[DIM_WORD][DIM_ALPH]= {}; // Known invalid letters

static Data            word_data;   // The word data container
static data_list&      word_list= word_data.line(); // The word container list
static std::unique_ptr<affix_head>
                       rule[DIM_CHAR]; // The rule table (any ASCII character)

static int             count= 0;    // Number of letters in target word
static int             opt_debug= 0; // --debug

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
   debugf("debug(%s)\n", info);

   if( word_list.get_head() )
     debugf("word_list{'%s'..'%s'}\n"
           , word_list.get_head()->text, word_list.get_tail()->text);
   else
     debugf("word_list{} (empty)\n");

   debugf("Known: '");
   for(int wx= 0; wx < count; ++wx)
     debugf("%c", known[wx] ? known[wx] : '-');
   debugf("'\n");

   debugf("\nLetter table:\n");
   for(int cx= 0; cx < DIM_ALPH; ++cx) {
     string S= "Unknown";
     int maxi= maxis[cx];
     if( maxi == 0 ) {
       S= "Does not occur";
       S= "Occurs 0 times";
     } else {
       int mini= minis[cx];
       if( mini == maxi )
         S= pub::utility::to_string("Occurs %d time%s", mini
                                   , mini == 1 ? "" : "s");
       else
         S= pub::utility::to_string("Occurs %d..%d times", mini, maxi);
     }
     debugf("[%c] %s\n", cx + 'a', S.c_str());
   }

   if( VERBOSE ) {
     debugf("\nRule table:\n");
     for(int i= 0; i < DIM_CHAR; ++i) {
       affix_head* head= rule[i].get();
       if( head ) {
         debugf("[%c] %s %s\n", i, head->prefix ? "PFX" : "SFX"
               , head->paired ? "Y" : "N");

         for(auto it= head->list.begin(); it != head->list.end(); ++it) {
           debugf("..Rem(%s) Ins(%s) '%s' %d\n"
                 , it->remove.c_str(), it->insert.c_str(), it->ifrule.c_str()
                 , it->ifrule[0]
                 );
         }
         debugf("\n");
       }
     }
   }

   debugf("\n%s %s %s\n", __FILE__, __DATE__, __TIME__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugging_stop
//
// Purpose-
//       Debugging word stop (Use with gdb)
//
//----------------------------------------------------------------------------
static bool                         // TRUE if word detected
   debugging_stop(string word)      // Debugging word stop
{  return word == DEBUGGING_STOP_WORD; }

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
     "Rule: LLLLL:?????\n"
     "  Where 'L' is any lower case character between 'a' and 'z', and\n"
     "  '?' is either '-', '+', or '=', and\n"
     "    '-' indicates letter doesn't appear at this position in a word\n"
     "        and, if it's not a duplicate, doesn't appear in any word\n"
     "    '+' indicates letter appears at some position in a word\n"
     "    '=' indicates letter appears at this position in a word\n"
     "\n"
     "Example: worder steam:--++- brake:--=-+\n"
   );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       insert
//
// Purpose-
//       Insert word into list
//
//----------------------------------------------------------------------------
static void
   insert(                          // Insert word into dictionary
     string            word,        // The word to insert
     const char*       affix= nullptr) // "PFX"/"SFX" indicator
{
   if( USE_DUPLICATE_DETECT ) {
     int max_search= 32;            // Maximum search length
     for(Line* line= word_list.get_tail(); line; line= line->get_prev()) {
       if( word == line->text ) {   // If duplicate
         if( HCDM && VERBOSE ) {
           if( affix )
             debugf("Insert(%s) %s skipped duplicate\n", word.c_str(), affix);
           else
             debugf("Insert(%s) skipped duplicate\n", word.c_str());
         }
         return;
       }

       if( --max_search == 0 )      // Limit search length
         break;
     }
   }

   if( HCDM && VERBOSE && affix )
     debugf("Insert(%s) %s\n", word.c_str(), affix);

   word_list.fifo(word_data.get_line(word));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_rule
//
// Purpose-
//       Does the rule apply to the string?
//
//----------------------------------------------------------------------------
static bool
   if_rule(string rule, string text) // Does the rule apply to the string?
{
   if( debugging_stop(text) )       // (Use with gdb)
     debugf("%4d if_rule(%s,%s)\n", __LINE__, rule.c_str(), text.c_str());

   if( rule[0] != '[' ) {           // If character match rule
     if( rule == "." )
       return true;

     size_t L= rule.size();
     if( text.size() < L )
       return false;

     if( text.substr(text.size()-L, L) == rule )
       return true;

     return false;
   }

   size_type X= rule.find(']');
   if( X == npos ) {                // (Should not occur)
     debugf("Malformed rule '%s', '[' without ']'\n", rule.c_str());
     return false;
   }

   size_t L= rule.size() - X;      // Length of search
   if( text.size() < L )
     return false;

   size_t O= text.size() - L;
   bool is_not= (rule[1] == '^');
   for(size_t i= 1; i<X; ++i) {
     if( rule[i] == text[O] ) {
       if( is_not )
         return false;
       if( L == 1 )
         return true;
       for(size_t j= L; j>0; --j) {
         if( rule[rule.size() - j] != text[text.size() - j] )
           return false;
       }
       return true;
     }
   }
   if( is_not ) {
     if( L == 1 )
       return true;
     for(size_t j= L; j>0; --j) {
       if( rule[rule.size() - j] != text[text.size() - j] )
         return false;
     }
     return true;
   }

   return false;
}

static inline bool                  // Conditional debugging version
   is_rule(string rule, string text)
{
   bool rc= if_rule(rule, text);
   if( HCDM && VERBOSE > 1 )
     debugf("%c= is_rule(%s,%s)\n", rc ? 'T':'F', rule.c_str(), text.c_str());
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Purpose-
//       Load the Word list
//
//----------------------------------------------------------------------------
static void
   load( void )                     // Load the Word list
{
   struct stat info;
   const char* path= "/usr/share/hunspell";
   if( stat("/usr/share/hunspell/en_US.dic", &info) ) {
     path= "/usr/share/myspell";
     if( stat("/usr/share/myspell/en_US.dic", &info) ) {
       fprintf(stderr, "No word list found\n");
       exit(1);
     }
   }

   Data ffix(path, "en_US.aff");    // The affix table
   Data dict(path, "en_US.dic");    // The dictionary

   //-------------------------------------------------------------------------
   // Load the word affinity table
   std::unique_ptr<affix_head> head; // The current rule heading
   for(Line* line= ffix.line().get_head(); line; line= line->get_next()) {
     pub::Tokenizer izer(line->text);
     auto it= izer.begin();
     string text= it();
     if( text == "PFX" || text == "SFX" ) {
       // Handle affix_head
       if( head.get() == nullptr ) {
         head= std::move(std::make_unique<affix_head>());

         if( text == "PFX" )
           head->prefix= true;

         text= (++it)();
         if( text.size() != 1 || text[0] >= DIM_CHAR ) {
           debugf("Invalid affix line '%s'\n", line->text);
           exit(2);
         }
         head->index= text[0];

         text= (++it)();
         if( text == "N" )
           head->paired= false;
         continue;
       }

       // Handle affix_rule
       affix_rule rule;
       ++it;                        // Ignore identifier
       text= (++it)();              // Get remove
       rule.remove= text;

       text= (++it)();              // Get insert
       rule.insert= text;

       // Rule (add 's) causes the Tokenizer to consider the rest of the line
       // to be the insert. We don't want to add 's to words anyway.
       text= (++it)();              // Get ifrule
       if( text == "" )
         continue;
       rule.ifrule= text;

       // Verify prefix rule (Only the first prefix rule rules)
       if( head->prefix && (rule.remove != "0" || rule.ifrule != ".") ) {
         debugf("PFX rule(%s) unknown, ignored\n", line->text);
         continue;
       }

       // Add the rule to the list
       head->list.insert(head->list.end(), rule);
     } else {
       if( head.get() )             // If PFX/SFX in progress
         rule[head->index]= std::move(head);
     }
   }

   //-------------------------------------------------------------------------
   // Load the word list
   Line* line= dict.line().get_head(); // (The first line is skipped)
   for(line= line ? line->get_next() : line; line; line= line->get_next()) {
     string text= line->text;

     // Separate word and affinity controls
     size_type X= text.find('/');
     string mark;
     if( X != npos ) {
       mark= text.substr(X+1);
       text= text.substr(0, X);
     }

     // If word contains invalid characters, ignore it
     bool valid= true;
     for(size_type i= 0; i<text.size(); ++i) {
       int C= text[i];
       if( C < 'a' || C > 'z' ) {
         valid= false;
         break;
       }
     }
     if( !valid )
       continue;

     insert(text);                  // Insert the base word

     // Handle affix word extensions
     int prefix_ix= 0;
     string prefix[16];             // (The prefix string array)
     for(size_t i= 0; i < mark.size(); ++i) {
       if( mark[i] == NOSUGGEST )
         continue;

       affix_head* head= rule[(int)mark[i]].get();
       if( head == nullptr ) {
         debugf("%s unknown rule(%c)\n", line->text, mark[i]);
         continue;
       }

       // Handle prefix extension
       if( head->prefix ) {
         auto it= head->list.begin(); // (Only one rule supported)
         if( it != head->list.end() ) { // If that one rule exists
           insert(it->insert + text, "PFX");
           if( head->paired )
             prefix[prefix_ix++]= it->insert;
         }
         continue;
       }

       // Handle suffix extension
       for(auto it= head->list.begin(); it != head->list.end(); ++it) {
         if( is_rule(it->ifrule, text) ) {
           string T= text;
           if( it->remove != "0" )
             T= text.substr(0, text.size()-1);
           T += it->insert;
           insert(T, "SFX");
           if( prefix_ix && head->paired ) {
             for(int p= 0; p<prefix_ix; ++p)
               insert(prefix[p] + T, "PFX/SFX");
           }
         }
       }
     }
   }

   // The word_list must contain at least one word
   if( word_list.get_head() == nullptr ) {
     debugf("ERROR: no words in dictionary\n");
     exit(2);
   }
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
   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   if( argc <= 1 ) {
     fprintf(stderr, "At least one parameter is needed\n");
     info();
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

   // Get count, using first valid parameter
   const char* X= strchr(argv[argn], ':');
   if( X == nullptr ) {
     fprintf(stderr, "Malformed parameter '%s'\n", argv[argn]);
     info();
   }
   count= int(X - argv[argn]);      // The first parameter defines the count

   // Since we know the count, initialize the maxis array
   for(int cx= 0; cx < DIM_ALPH; ++cx)
     maxis[cx]= count;

   //=========================================================================
   for(int argi= argn; argi < argc; ++argi) { // For each parameter
     char* parm= argv[argi];
     bool valid= true;
     for(int wx= 0; wx < count; ++wx) { // Only lower case latin letters valid
       if( parm[wx] < 'a' || parm[wx] > 'z' ) {
         valid= false;
         break;
       }
     }

     if( debugging_stop(parm) )     // (Use with gdb)
       printf("%4d STOP: %s\n", __LINE__, parm);

     if( parm[count] != ':' )       // Characters must have correct length
       valid= false;

     char* desc= parm + count + 1;
     for(int wx= 0; wx < count; ++wx) { // Only '-', '+', and '=' are valid
       if( desc[wx] != '-' && desc[wx] != '+' && desc[wx] != '=' )
         valid= false;
     }

     if( desc[count] )              // Descriptor must have correct length
       valid= false;

     if( !valid ) {
       fprintf(stderr, "Malformed parameter '%s'\n", parm);
       info();
     }

     // Initialize letter occurrance counters
     int hits[DIM_ALPH]= {};        // By letter, number of '='/'+' occurances
     int miss[DIM_ALPH]= {};        // By letter, number of missed occurances
     for(int wx= 0; wx<count; ++wx) {
       int C= parm[wx];
       int cx= C - 'a';
       int D= desc[wx];
       if( D == '=' || D == '+' )
         ++hits[cx];
       else /* D == '-' */
         ++miss[cx];
     }

     // Cross-check parameters, updating remaining letter occurance count
     for(int wx= 0; wx<count; ++wx) {
       int C= parm[wx];
       int cx= C - 'a';

       int D= desc[wx];
       if( D == '-' ) { //----------------------------------------------------
         // If character can't appear here
         if( minis[cx] > hits[cx] ) {
           fprintf(stderr, "Argument '%s'[%d] is '%c' but '%c' must be in "
                           "word at least %d time%s\n"
                  , parm, wx+1, D, C, minis[cx], minis[cx] == 1 ? "" : "s");
           info();
         }
         notat[wx][cx]= true;       // Letter can't appear at this position

       } else if( D == '+' ) { //---------------------------------------------
         // If character appears elsewhere in word
         if( known[wx] == parm[wx] ) {
           fprintf(stderr, "Argument '%s'[%d] is '%c' but was already "
                           "defined != '%c'\n"
                  , parm, wx+1, parm[wx], known[wx]);
           info();
         }
         notat[wx][cx]= true;       // It appears elsewhere, not here

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
       int cx= C - 'a';
       int H= hits[cx];
       if( minis[cx] < H )
         minis[cx]= H;              // Set new minimum

       if( miss[cx] && maxis[cx] > H )
         maxis[cx]= H;              // Set new maximum
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
   load();                          // Load and expand the dictionary
   if( opt_debug )                  // If --debug specified
     debug("--debug");

   //-------------------------------------------------------------------------
   // Find matching words in dictionary
   //-------------------------------------------------------------------------
   for(Line* line= word_list.get_head(); line; line= line->get_next()) {
     const char* text= line->text;
     if( debugging_stop(text) )     // (Use with gdb)
       debugf("%4d STOP: %s\n", __LINE__, text);

     if( strlen(text) != size_t(count) ) // Ignore words of incorrect length
       continue;

     bool valid= true;
     for(int wx= 0; wx<count; ++wx) { // For each character
       int C= text[wx];
       int cx= C - 'a';
       if( maxis[cx] == 0 ) {       // Disallow words with invalid characters
         valid= false;
         break;
       }

       // Insure rule validity (for character position)
       for(int px= 1; px<argc; ++px) { // For each parameter (rule)
         char* parm= argv[px];
         char* desc= parm + count + 1;
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
       int cx= C - 'a';
       if( notat[wx][cx] ) {
         valid= false;
         break;
       }

       ++hits[cx];
     }
     if( !valid )
       continue;

     // Verify repetition counts for all characters, not just those in the word
     for(int cx= 0; cx < DIM_ALPH; ++cx) {
       if( hits[cx] > maxis[cx] || hits[cx] < minis[cx] ) {
         valid= false;
         break;
       }
     }

     if( valid )
       printf("%s\n", text);
   }

   return 0;
}
