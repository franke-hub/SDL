//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//-----------------------------------------------------------------------------
//
// Title-
//       Dictionary.h
//
// Purpose-
//       Dictionary definition and implementation.
//
// Last change date-
//       2025/11/23
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DICTIONARY_H_INCLUDED
#define _LIBPUB_DICTIONARY_H_INCLUDED

#include <list>                     // For std::list
#include <string>                   // For std::string
#include <cstring>                  // For strcmp, ...

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/List.h"               // For pub::List

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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

//----------------------------------------------------------------------------
// Dictionary::Exception (end_dereference)
//----------------------------------------------------------------------------
class end_dereferenced : public std::domain_error {
public:
  using std::domain_error::domain_error;
  end_dereferenced() : domain_error("end() dereferenced") {}
}; // class Dictionary::end_dereferenced

//----------------------------------------------------------------------------
// Dictionary::Word
//----------------------------------------------------------------------------
class Word : public pub::Sort<Word>::Link { // Dictionary word
// Word::Attributes
public:
std::string            word;        // The associated word

// Word::Constructor
public:
   Word(                            // Constructor
     std::string       text)        // The associated word
:  word(text) {}

// Word::operator<                  // (Used for sorting)
virtual bool operator<(const Base& _that) const override
{
   const Word* that= static_cast<const Word*>(&_that);
   return word < that->word;
}
}; // class Dictionary::Word

//----------------------------------------------------------------------------
// Dictionary::Rules
//----------------------------------------------------------------------------
protected:
struct affix_rule {
string                 remove;      // The remove string
string                 insert;      // The insert string
string                 ifrule;      // The applicable ending rule
}; // struct Dictionary::affix_rule

struct affix_head {
std::list<affix_rule>  list;        // The rule list
int                    index= 0;    // The rule index
bool                   paired= true; // Combinable prefix/suffix?
bool                   prefix= false; // Prefix || suffix?
}; // struct Dictionary::affix_head

//----------------------------------------------------------------------------
// Dictionary::Iterator
//----------------------------------------------------------------------------
typedef Sort<Word>::iterator        Iterator;

//----------------------------------------------------------------------------
// Dictionary::Attributes
//----------------------------------------------------------------------------
protected:
pub::Sort<Word>        list;        // The word list

std::unique_ptr<affix_head>
                       rule[DIM_CHAR]= {}; // Rule table, ASCII char index

//----------------------------------------------------------------------------
// Dictionary::Constructors, destructor
//----------------------------------------------------------------------------
public:
   Dictionary(                      // Default constructor
     const char*       user_dict[]= nullptr); // OPTIONAL dictionary files

   ~Dictionary( void );             // Destructor

//----------------------------------------------------------------------------
// Dictionary::Iterator access
//----------------------------------------------------------------------------
Iterator
   begin( void )                    // Get begin Iterator
{  return list.begin(); }

Iterator
   end( void )                      // Get end Iterator
{  return list.end(); }

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

//----------------------------------------------------------------------------
// Dictionary::Protected methods
//----------------------------------------------------------------------------
protected:
bool                                // TRUE if rule applies
   is_rule(                         // Does a rule apply?
     string            rule,        // Does this rule apply to
     string            text);       // This string?

void
   load_dict(const char*);          // Load a dictionary

void
   load_rule(const char*);          // Load the dictionary rule table

void
   cleanup( void );                 // Remove and delete duplicate Words
}; // class Dictionary
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DICTIONARY_H_INCLUDED
