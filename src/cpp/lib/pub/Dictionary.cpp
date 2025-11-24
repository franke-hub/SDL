//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//-----------------------------------------------------------------------------
//
// Title-
//       Dictionary.cpp
//
// Purpose-
//       Dictionary method implementation.
//
// Last change date-
//       2025/11/23
//
//----------------------------------------------------------------------------
#include <memory>                   // For std:unique_ptr, make_unique
#include <string>                   // For std::string
#include <cstring>                  // For strcmp, ...

#include <stdlib.h>                 // For getenv
#include <sys/stat.h>               // For struct stat

#include <pub/Data.h>               // For namespace pub::data
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dictionary.h"         // For pub::Dictionary, implemented
#include "pub/List.h"               // For pub::List
#include <pub/Tokenizer.h>          // For pub::Tokenizer

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugf, errorf, ...
using namespace PUB::data;          // For (typedef) Data, Line, ...
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
// Default dictionary/rule table
//----------------------------------------------------------------------------
struct dict_rule_t {
const char*            dict;        // Dictionary name
const char*            rule;        // Rule name
};

dict_rule_t            dict_rule[]=
{  {"/usr/share/myspell/en_US-large.dic", "/usr/share/myspell/en_US-large.aff"}
,  {"/usr/share/myspell/en_US.dic", "/usr/share/myspell/en_US.aff"}
,  {"/usr/share/hunspell/en_US-large.dic", "/usr/share/hunspell-large/en_US.aff"}
,  {"/usr/share/hunspell/en_US.dic", "/usr/share/hunspell/en_US.aff"}
,  {nullptr, nullptr}
};

const char*            optional_lib[]= // Built-in optional dictionary libs
{  "Library/Spelling"
,  ".local/lib/Spelling"
,  nullptr
};

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
     errorf("%4d if_rule(%s,%s)\n", __LINE__, rule.c_str(), text.c_str());

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
     errorf("Malformed rule '%s', '[' without ']'\n", rule.c_str());
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

   string      HOME= getenv("HOME"); // The HOME directory
   struct stat info;                // Working info struct

   const char* dict= nullptr;
   const char* rule= nullptr;
   for(int i= 0; dict_rule[i].dict; ++i) {
     if( stat(dict_rule[i].rule, &info) == 0 ) { // If rule found
       dict= dict_rule[i].dict;
       rule= dict_rule[i].rule;
       if( stat(dict, &info) ) {    // If no associated dictionary
         fprintf(stderr, "ERROR: hunspell not properly installed\n"
                         "File(%s) exists but File(%s) does not\n"
                       , rule, dict);
         exit(1);
       }

       break;
     }
   }

   if( dict == nullptr || rule == nullptr ) {
     fprintf(stderr, "ERROR: hunspell not installed\n"
                     "File(%s) not found\n", dict_rule[1].rule);
     exit(1);
   }

   load_rule(rule);
   load_dict(dict);

   // Load built-in OPTIONAL dictionary libraries
   for(int i= 0; optional_lib[i]; ++i) {
     string lib= optional_lib[i];
     if( lib[0] != '/' ) {          // If not fully qualified name
       lib= HOME + "/" + lib;       // Get relative name (to HOME)
     }

     if( stat(lib.c_str(), &info) == 0 ) { // If library found
       if( S_ISDIR(info.st_mode) ) { // If it's a directory
         pub::data::Path path(lib.c_str());
         for(auto iter= path.list.begin(); iter != path.list.end(); ++iter) {
           pub::data::Name name= lib + "/" + iter->name;
           string extension= name.get_extension(name.name);
           if( extension == "dic" ) {
             load_dict(name.name.c_str());
           } else if( HCDM && VERBOSE > 1 ) {
             errorf("HCDM Skipping File(%s) extension(%s)\n"
                   , name.get_extension(name.name).c_str(), extension.c_str());
           }
         }
       } else {                     // If it's a single file
         load_dict(lib.c_str());
       }
     }
   }

   // Load user-supplied OPTIONAL dictionaries
   if( user_dict ) {
     for(size_t i= 0; user_dict[i]; ++i) {

       Name name(user_dict[i]);
       const char* full= name.name.c_str();
       // const char* path= name.path_name.c_str();
       // const char* file= name.file_name.c_str();
       if( stat(full, &info) == 0 )
         load_dict(full);
       else
         errorf("WARNING: Optional file(%s) not found\n", full);
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

   if( VERBOSE > 0 ) {
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
         if( HCDM && VERBOSE > 0 ) {
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

   if( HCDM && VERBOSE > 0 && affix )
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
void
   Dictionary::load_dict(           // Load a dictionary
     const char*       full_name)   // The dictionary name
{  if( HCDM ) debugf("Dictionary(%p)::load_dict(%s)\n", this, full_name);

   Name name(full_name);
   const char* path= name.path_name.c_str();
   const char* file= name.file_name.c_str();

   Data dict(path, file);           // The dictionary

   //-------------------------------------------------------------------------
   // Load the word list
   Line* line= dict.line().get_head(); // 1st (count) line skipped
   if( line == nullptr ) {
     errorf("WARNING: Empty dictionary(%s)\n", dict.full().c_str());
     return;
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
         errorf("%s unknown rule(%c)\n", line->text, mark[i]);
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

   // The list should contain at least one valid word
   if( list.get_head() == nullptr )
     errorf("WARNING: no valid words in dictionary(%s)\n", dict.full().c_str());
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
void
   Dictionary::load_rule(           // Load the rule table
     const char*       full_name)   // The rule table file name
{  if( HCDM ) debugf("Dictionary(%p)::load_rule(%s)\n", this, full_name);

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
           errorf("Invalid affix line '%s'\n", line->text);
           return;
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
         errorf("PFX rule(%s) unknown, ignored\n", line->text);
         continue;
       }

       // Add the rule to the list
       head->list.insert(head->list.end(), rule);
     } else {
       if( head.get() )             // If PFX/SFX in progress
         rule[head->index]= std::move(head);
     }
   }
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
   // Sort the Word list
   list.sort();

   // Delete duplicates
   Word* prior= list.get_head();    // Add the first Word
   if( prior )                      // If empty list
     return;                        // (Nothing to delete)

   Word* link= prior->get_next();
   while( link ) {
     if( prior->word == link->word ) { // If duplicate
       list.remove(link);
       free(link);
     } else {                       // If unique
       prior= link;
     }

     link= prior->get_next();
   }
}
} // namespace _LIBPUB_NAMESPACE
