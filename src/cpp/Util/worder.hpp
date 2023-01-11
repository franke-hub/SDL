//----------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       worder.hpp
//
// Purpose-
//       Dictionary definition and implementation.
//
// Last change date-
//       2023/01/11
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <string.h>                 // For strcmp, ...
#include <sys/stat.h>               // For struct stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/List.h>               // For pub::List
#include <pub/Tokenizer.h>          // For pub::Tokenizer
#include <pub/utility.h>            // For pub::utility methods

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
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
// Class-
//       Dictionary
//
// Purpose-
//       Word list container
//
// Implementation notes-
//       The Dictionary uses hunspell/myspell word lists.
//
//----------------------------------------------------------------------------
class Dictionary {                  // Word list container
//----------------------------------------------------------------------------
// Dictionary::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string                 string;

enum { DIM_CHAR= 128 };             // The number of valid ASCII characters
enum { NOSUGGEST= '!' };            // TODO: Remove hard-coding
enum { USE_DUPLICATE_DETECT= true }; // Detect duplicate inserts?

//----------------------------------------------------------------------------
// Dictionary::Exception (end_dereference)
//----------------------------------------------------------------------------
class end_dereferenced : public std::domain_error {
public:
  using std::domain_error::domain_error;
  end_dereferenced() : domain_error("end() dereferenced") {}
}; // class end_dereferenced

//----------------------------------------------------------------------------
// Dictionary::Word
//----------------------------------------------------------------------------
class Word : public pub::List<Word>::Link { // Dictionary word
//----------------------------------------------------------------------------
// Word::Attributes
//----------------------------------------------------------------------------
public:
std::string            word;        // The associated word

//----------------------------------------------------------------------------
// Word::Constructors
//----------------------------------------------------------------------------
public:
   Word(                            // Constructor
     std::string       text)        // The associated word
:  word(text) {}
}; // class Word

//----------------------------------------------------------------------------
// Dictionary::Iterator
//----------------------------------------------------------------------------
struct Iterator {
   typedef ptrdiff_t                        difference_type;
   typedef std::input_iterator_tag          iterator_category;
   typedef const std::string                value_type;
   typedef value_type*                      pointer;
   typedef value_type&                      reference;

   typedef Word                             _Link;
   typedef pub::List<Word>                  _List;
   typedef Iterator                         _Self;

   _Link*       _link= nullptr;     // The current Word (Link)
   _List* const _list= nullptr;     // The associated List<List>*

   Iterator() noexcept = default;

   Iterator(const Iterator& that) noexcept
   :  _link(that._link), _list(that._list) {}

explicit
   Iterator(_List* list) noexcept
   :  _link(list->get_head()), _list(list) {}

pointer
   get() const // noexcept
{  if( _link )
     return &_link->word;
   throw end_dereferenced();
}

operator bool()
{  return bool(_link); }

reference
   operator*() const // noexcept
{  if( _link )
     return (reference)(_link->word);
   throw end_dereferenced();
}

pointer
   operator->() const // noexcept
{  if( _link )
     return &_link->word;
   throw end_dereferenced();
}

_Self&
   operator++() noexcept
{
   if( _link )
     _link= _link->get_next();

   return *this;
}

_Self
   operator++(int) noexcept {
     _Self __tmp = *this;
     operator++();
     return __tmp;
   }

friend bool
   operator==(const _Self& lhs, const _Self& rhs) noexcept
{  return lhs._link == rhs._link; }

friend bool
   operator!=(const _Self& lhs, const _Self& rhs) noexcept
{  return lhs._link != rhs._link; }
}; // struct Iterator

//----------------------------------------------------------------------------
// Dictionary::Attributes
//----------------------------------------------------------------------------
protected:
pub::List<Word>        list;        // The word list
std::unique_ptr<affix_head>
                       rule[DIM_CHAR]; // The rule table [any ASCII character]

//----------------------------------------------------------------------------
// Dictionary::Constructors, destructor
//----------------------------------------------------------------------------
public:
   Dictionary( void ) = default;    // Default constructor (empty Dictionary)
   ~Dictionary( void );             // Destructor

//----------------------------------------------------------------------------
// Dictionary::Iterator access
//----------------------------------------------------------------------------
Iterator
   begin( void )                    // Get begin Iterator
{  return Iterator(&list); }

Iterator
   end( void )                      // Get end Iterator
{  return Iterator(); }

//----------------------------------------------------------------------------
// Dictionary::Methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "");   // Informational message

int                                 // Return code, 0 expected
   append(                          // Append/insert a dictionary
     const char*       name);       // Dictionary file name (without type)

void
   insert(                          // Insert word into dictionary
     string            word,        // The word to insert
     const char*       affix= nullptr); // "PFX"/"SFX" indicator

int                                 // Return code, 0 expected
   load( void );                    // Load all known dictionaries

//----------------------------------------------------------------------------
// Dictionary::Protected methods
//----------------------------------------------------------------------------
protected:
bool                                // TRUE if rule applies
   is_rule(                         // Does a rule apply?
     string            rule,        // Does this rule apply to
     string            text);       // This string?

void
   cleanup( void );                 // Remove and delete duplicate Words
}; // class Dictionary

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
{
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
//       Dictionary::load
//
// Purpose-
//       Load all known dictionaries
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Dictionary::load( void )         // Load all known dictionaries
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
     exit(2);
   }

   cleanup();                       // Clean up the Dictionary

   return 0;
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
