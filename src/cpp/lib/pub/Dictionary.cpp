//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Dictionary.cpp
//
// Purpose-
//       Dictionary method implementation.
//
// Last change date-
//       2024/01/08
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <string.h>                 // For strcmp, ...
#include <sys/stat.h>               // For struct stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dictionary.h"         // For pub::Dictionary, implemented
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/List.h>               // For pub::List
#include <pub/Tokenizer.h>          // For pub::Tokenizer

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugf, ...
using namespace PUB::fileman;       // For (typedef) Data, Line, ...
using std::string;                  // For (typedef) string

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  NOSUGGEST= '!'                   // TODO: Remove hard-coding
,  USE_DUPLICATE_DETECT= true       // Detect duplicate inserts?
}; // enum

// std::string typedefs and constants
typedef std::string                 string;
typedef string::size_type           size_type;
static const size_type npos=        string::npos;

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugging_stop
//
// Purpose-
//       Debugging word stop (Use with gdb)
//
//----------------------------------------------------------------------------
static constexpr const char*
                       DEBUGGING_STOP_WORD= ".";
//                       DEBUGGING_STOP_WORD= "error";

static bool                         // TRUE if word detected
   debugging_stop(string word)      // Debugging word stop
{  return word == DEBUGGING_STOP_WORD; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       if_rule
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

//----------------------------------------------------------------------------
//
// Subroutine-
//       quick_sort
//
// Purpose-
//       Quick sort the Word array
//
//----------------------------------------------------------------------------
static void
   quick_sort(                      // (Partially) sort the Word array
     size_t            inp_bot,     // Leftmost index
     size_t            inp_top,     // Rightmost index
     Dictionary::Word**array)       // Word array
{
   size_t              bot= inp_bot; // Working left index
   size_t              top= inp_top; // Working right index
   Dictionary::Word*   pword= array[inp_bot]; // Pivot Word
   std::string         pvstr= pword->word; // Pivot String

   while( bot < top ) {
     while( bot < top && array[top]->word >= pvstr )
       top--;
     if( bot != top ) {
       array[bot]= array[top];
       bot++;
     }

     while( bot < top && array[bot]->word <= pvstr )
       bot++;
     if( bot != top ) {
       array[top]= array[bot];
       top--;
     }
   }

   array[bot]= pword;
   if( inp_bot < bot )
     quick_sort(inp_bot, bot-1, array);

   if( inp_top > bot )
     quick_sort(bot+1, inp_top, array);
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::Dictionary
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   Dictionary::Dictionary(          // Constructor
     const char*       user_dict[]) // OPTIONAL dictionary files
:  list()
{  if( HCDM ) debugf("!Dictionary(%p)\n", this);

   struct stat info;
   const char* rule= "/usr/share/hunspell/en_US.aff";
   const char* dict= "/usr/share/hunspell/en_US.dic";
   if( stat(rule, &info) ) {
     rule= "/usr/share/myspell/en_US.aff";
     dict= "/usr/share/myspell/en_US.dic";
     if( stat(rule, &info) ) {
       fprintf(stderr, "Default word list not found\n");
       return;
     }
   }

   load_rule(rule);
   load_dict(dict);
   if( user_dict ) {                // If user_list specified
     for(size_t i= 0; user_dict[i]; ++i) {
       Name name(user_dict[i]);
       const char* full= name.name.c_str();
       // const char* path= name.path_name.c_str();
       // const char* file= name.file_name.c_str();
       if( stat(full, &info) == 0 )
         load_dict(full);
       else
         fprintf(stderr, "Optional file(%s) not found\n", full);
     }
   }

   cleanup();                       // Remove duplicates, etc
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::~Dictionary
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Dictionary::~Dictionary( void )   // Destructor: empty the Word list
{  if( HCDM ) debugf("~Dictionary(%p)\n", this);

   // Delete the Word list
   Word* head= list.reset();
   while( head ) {
     Word* next= head->get_next();
     delete head;
     head= next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Dictionary::debug(               // Debugging display
     const char*       info)        // Caller information
{
   debugf("Dictionary(%p)::debug(%s)\n", this, info);

   if( list.get_head() )
     debugf("word list{'%s'..'%s'}\n"
           , list.get_head()->word.c_str(), list.get_tail()->word.c_str());
   else
     debugf("list{} (empty)\n");

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
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::append
//
// Purpose-
//       Append file to dictionary
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Dictionary::append(              // Add file to dictionary
     const char*       name)        // The file name
{  (void)name; // NOT CODED YET
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::insert
//
// Purpose-
//       Insert word into list
//
//----------------------------------------------------------------------------
void
   Dictionary::insert(              // Insert word into dictionary
     string            word,        // The word to insert
     const char*       affix)       // "PFX"/"SFX" indicator
{
   if( USE_DUPLICATE_DETECT ) {
     int max_search= 8;             // Maximum search length
     for(Word* item= list.get_tail(); item; item= item->get_prev()) {
       if( word == item->word ) {   // If duplicate
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

   list.fifo(new Word(word));
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::is_rule
//
// Purpose-
//       Does a rule apply?
//
//----------------------------------------------------------------------------
bool                                // TRUE if rule applies
   Dictionary::is_rule(             // Does a rule apply?
     string            rule,        // Does this rule apply to
     string            text)        // This string?
{
   bool rc= if_rule(rule, text);
   if( HCDM && VERBOSE > 1 )
     debugf("%c= is_rule(%s,%s)\n", rc ? 'T':'F', rule.c_str(), text.c_str());
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::load_dict
//
// Purpose-
//       Load a dictionary
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Dictionary::load_dict(           // Load a dictionary
     const char*       full_name)   // The dictionary name
{
   Name name(full_name);
   const char* path= name.path_name.c_str();
   const char* file= name.file_name.c_str();

   Data dict(path, file);           // The dictionary

   //-------------------------------------------------------------------------
   // Load the word list
   Line* line= dict.line().get_head(); // 1st (count) line skipped
   if( line == nullptr ) {
     debugf("ERROR: Empty dictionary(%s)\n", dict.full().c_str());
     exit(2);
   }

   for(line= line->get_next(); line; line= line->get_next()) {
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

     if( debugging_stop(text) )     // (Use with gdb)
       debugf("%4d load(%s)\n", __LINE__, text.c_str());

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

   // The list must contain at least one valid word
   if( list.get_head() == nullptr ) {
     debugf("ERROR: no valid words in dictionary(%s)\n", dict.full().c_str());
     return 2;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::load_rule
//
// Purpose-
//       Load the rule table
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Dictionary::load_rule(           // Load the rule table
     const char*       full_name)   // The rule table file name
{
   Name name(full_name);
   const char* path= name.path_name.c_str();
   const char* file= name.file_name.c_str();

   Data ffix(path, file);           // The affix table

   //-------------------------------------------------------------------------
   // Load the word affinity (rule) table
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
           return 2;
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

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Dictionary::cleanup
//
// Purpose-
//       Sort the dictionary, removing and deleting duplicate Words
//
//----------------------------------------------------------------------------
void
   Dictionary::cleanup( void )      // Clean up the Dictionary
{
   // QuickSort the Word list
   size_t count= 0;
   Word* word= list.get_head();     // Count the list entries
   while( word ) {
     ++count;
     word= word->get_next();
   }

   if( count == 0 )                 // (Should not occur)
     return;

   Word** temp= (Word**)malloc(count * sizeof(Word*)); // Create sortable array
   if( temp == nullptr )            // If storage not available
     return;                        // Don't bother removing duplicates

   word= list.reset();              // (Empties the list)
   for(size_t i= 0; i<count; ++i) { // Fill the array
     temp[i]= word;
     word= word->get_next();
   }

   quick_sort(0, count-1, temp);    // Sort the array

   // Restore the Word list, omitting and deleting duplicates
   list.fifo(temp[0]);              // Add the first Word
   std::string last= temp[0]->word;
   for(size_t i= 1; i<count; ++i) { // Add the remaining Words, removing dups
     Word* item= temp[i];
     if( last == item->word ) {
       delete item;                 // Remove duplicate
     } else {                       // Insert unique Word
       list.fifo(item);
       last= item->word;
     }
   }

   free(temp);
}
} // namespace _LIBPUB_NAMESPACE
