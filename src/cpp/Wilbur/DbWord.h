//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbWord.h
//
// Purpose-
//       The word database, associating words with permanent indexes.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DBWORD_H_INCLUDED
#define DBWORD_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbWord
//
// Purpose-
//       The word database.
//
// Identification-
//       NAME: Wilbur/DbWord.db
//       NAME: Wilbur/DbWord_ixWord.db
//       PROG: src/cpp/Wilbur/DbWord.cpp
//       THIS: src/cpp/Wilbur/DbWord.h
//
// Usage notes-
//       This database contains all the words in a language, thus allowing
//       words of any length to fit into a 32-bit computer word.
//
// Usage notes-
//       While only one thread may construct or destroy a given instance,
//       each DbWord object is otherwise thread-safe.
//
// Usage notes-
//       The longest allowed word length is specified by MAX_WORD_LENGTH
//       An area of this length(+1) must be provided by callers of methods
//       that return word values, i.e. getValue, and nextIndex. The insert
//       method disallows longer words.
//
// Usage notes-
//       Method getIndex only returns index values for the default language.
//       Method nextValue only returns word values for the default language.
//       The default language is specified by the constructor.
//
// Usage notes-
//       Word values should be stored ISO-8859 lower case only. It is up to
//       the caller to verify that this requirement is met. The insert
//       method only disallows words of improper length.
//
// Implementation notes-
//       The high order 8 bits of an index are reserved as a language mask,
//       reserving space for 255 languages. All languages are within the
//       same database; certain instance methods (see above) only return
//       values for the default language.
//
// Implementation notes-
//       For words less than four characters in length the word itself
//       is always the index. Leading blanks are used for one and two
//       character words. E.g, the index for english "a" is 0x01202061.
//
//       Since the SPace is the lowest ASCII word == value index, there
//       is an intrinsic limit of 0x001fffff (word != index) entries.
//       This allows more than 2 million word!=index entries per language,
//       which should be OK.
//
// Implementation notes (Special database entries)-
//       0x00000000/""     (Always present in database)
//       0xLL000000/"_nn"  (The language name mask)
//       0xLLmmmmmm/"*"    (The higest allocated index for language LL)
//
// Implementation notes-
//       struct DbWordIndex {       // The DbWord index
//         uint32_t    index;       // The index (NETWORK format)
//       }; // struct DbWordIndex
//
//       struct DbWordValue {       // The DbWord value
//         char        value[1];    // The value (w/o '\0' terminator)
//       }; // struct DbWordValue
//
//----------------------------------------------------------------------------
class DbWord : public DbBase {      // The word database
//----------------------------------------------------------------------------
// DbWord::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbIndex;     // INDEX to VALUE database
Db*                    ixValue;     // VALUE to INDEX database

uint32_t               language;    // Default language (index)
uint32_t               langMask;    // Default language mask

//----------------------------------------------------------------------------
// DbWord::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 0                // High order 16 bits of uint64_t index
,  MAX_VALUE_LENGTH= 255            // (See usage notes)
}; // enum

//----------------------------------------------------------------------------
// DbWord::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbWord( void );                 // Destructor
   DbWord(                          // Constructor
     const char*       lang= "_en");// The (default) language name

private:                            // Bitwise copy is prohibited
   DbWord(const DbWord&);           // Disallowed copy constructor
   DbWord& operator=(const DbWord&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DbWord::Methods
//----------------------------------------------------------------------------
public:
virtual uint32_t                    // The index (0 if error/missing)
   getIndex(                        // Get index
     const char*       value);      // For this value

inline uint32_t                     // The index (0 if error/missing)
   getIndex(                        // Get index
     std::string&      value)       // For this value
{  return getIndex(value.c_str());
}

virtual char*                       // Result (NULL if error/missing)
   getValue(                        // Get value
     uint32_t          index,       // For this index
     char*             result);     // (OUTPUT) Result string

virtual uint32_t                    // The index (0 if error)
   insert(                          // Get index (insert if missing)
     const char*       value);      // For this value

inline uint32_t                     // The index (0 if error)
   insert(                          // Get index (insert if missing)
     std::string&      value)       // For this value
{  return insert(value.c_str());
}

virtual uint32_t                    // Result (0 if error/missing)
   nextIndex(                       // Get next index
     uint32_t          index,       // After this index
     char*             value= NULL);// If not NULL, associated value

virtual char*                       // Result (NULL if error/missing)
   nextValue(                       // Get next value
     char*             value,       // (IN/OUT) Current/Next value
     uint32_t*         index= NULL);// If not NULL, associated index

virtual int                         // Return code (0 if error)
   remove(                          // Remove
     uint32_t          index);      // This index

protected:
void
   reset( void );                   // Reset the database
}; // class DbWord

#endif // DBWORD_H_INCLUDED
