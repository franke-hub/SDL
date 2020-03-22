//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Word.h
//
// Purpose-
//       Word mapping.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef WORD_H_INCLUDED
#define WORD_H_INCLUDED

#include <stdexcept>                // For runtime_error
#include <map>                      // For std::map
#include <com/Subpool.h>            // For Subpool

//----------------------------------------------------------------------------
//
// Class-
//       Word
//
// Purpose-
//       Word :: Index mapping.
//
// Implementation note-
//       The 32 bit mapping is an intended implementation limit.
//       (Use multiple mappings if a larger limit is required.)
//
//----------------------------------------------------------------------------
class Word {                        // Word :: Index mapping
//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint32_t       Count;       // Generic Count
typedef uint32_t       Index;       // Generic Index
typedef const char*    Text;        // Word text
typedef size_t         Total;       // Generic total Count

struct Text_LT {                    // Text comparitor, used by w_map
inline constexpr bool operator()(Text lhs, Text rhs) const
{  return strcmp(lhs,rhs) < 0; }
}; // struct Text_LT

typedef std::map<Text, Index, Text_LT> w_map_t;

//----------------------------------------------------------------------------
// Word::Attributes
//----------------------------------------------------------------------------
protected:
w_map_t                w_map;       // Text to Index map
Subpool                w_pool;      // Text allocation subpool
Count                  w_size;      // Number of w_text*'s allocated
Count                  w_used;      // Number of w_text*'s used
Text*                  w_text;      // Word text by index (allocated in w_pool)

//----------------------------------------------------------------------------
// Word::Internal methods
//----------------------------------------------------------------------------
protected:
virtual Count                       // The updated w_size
   expand(                          // Expand the mapping (update w_size)
     Count             count);      // By this amount

//----------------------------------------------------------------------------
// Word::Constructors
//----------------------------------------------------------------------------
public:
   Word( void );                    // Constructor
   ~Word( void );                   // Destructor

//----------------------------------------------------------------------------
// Word::debug
//
// Debugging display.
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     unsigned          verbose= 0) const; // Verbosity. Larger is more verbose

//----------------------------------------------------------------------------
// Word::accessors
//----------------------------------------------------------------------------
Count get_used( void ) const        // Get used index count
{  return w_used; }

//----------------------------------------------------------------------------
// Word::index
//
// Extract Text given the Index.
// Extract Index given the Text.
//----------------------------------------------------------------------------
Text                                // The associated Text
   index(                           // Get Text
     Index             X) const;    // For this Index

Index                               // The associated Index
   index(                           // Get Index
     Text              T) const;    // For this Text

//----------------------------------------------------------------------------
// Word::insert
//
// Insert one word into dictionary.
//----------------------------------------------------------------------------
virtual Index                       // The associated Index
   insert(                          // Insert
     Text              T);          // This word

//----------------------------------------------------------------------------
// Word_refs::random_select
//
// Randomly select an Index
//----------------------------------------------------------------------------
virtual Index                       // The selection Index
   random_select( void );           // Randomly select an instance

//----------------------------------------------------------------------------
// Word::reset
//
// Reset the map.
//----------------------------------------------------------------------------
virtual void
   reset( void );                   // Reset the map

//----------------------------------------------------------------------------
// Word::(initializers)
//
// append: Append file to Word. Duplicates *ARE* allowed
// loader: Load file into Word. Duplicates *NOT* allowed
// File format: word\n...
//
// predef: Load Text into Word. Duplicates *NOT* allowed
// Text format: word\0...
//----------------------------------------------------------------------------
virtual void
   append(                          // Append (duplicates *ARE* allowed)
     const char*       name);       // The file name

virtual void
   loader(                          // Loader (duplicates *NOT* allowed)
     const char*       name);       // The file name

virtual void
   predef(                          // Loader in-place definitions
     const char*       name,        // The definition list
     size_t            size);       // The length of the list
}; // class Word

//----------------------------------------------------------------------------
//
// Class-
//       Word_refs
//
// Purpose-
//       Word reference counter.
//
//----------------------------------------------------------------------------
class Word_refs : public Word {     // Word reference counter
//----------------------------------------------------------------------------
// Word_refs::Attributes
//----------------------------------------------------------------------------
protected:
Count*                 w_refs;   // Word reference count by index
Total                  w_total;  // Total number of references

//----------------------------------------------------------------------------
// Word_refs::Internal methods
//----------------------------------------------------------------------------
protected:
virtual Count                       // The updated w_size
   expand(                          // Expand the mapping (update w_size)
     Count             count);      // By this amount

//----------------------------------------------------------------------------
// Word_refs::Constructors
//----------------------------------------------------------------------------
public:
   Word_refs( void );               // Constructor
   ~Word_refs( void );              // Destructor

//----------------------------------------------------------------------------
// Word_refs::debug
//
// Debugging display.
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     unsigned          verbose= 0) const; // Verbosity. Larger is more verbose

//----------------------------------------------------------------------------
// Word_refs::accessors
//----------------------------------------------------------------------------
Count get_count(Index X) const      // Get reference count by index
{  if( X >= w_used ) throw std::out_of_range("Word_refs::get_count");
   return w_refs[X]; }

Count get_count(Text  T) const      // Get reference count by Text
{  Index X= index(T);
   return w_refs[X];
}

Total get_total( void ) const       // Get Total reference count
{  return w_total; }

//----------------------------------------------------------------------------
// Word_refs::insert
//
// Insert word into dictionary.
//----------------------------------------------------------------------------
virtual Index                       // The associated Index
   insert(                          // Insert
     Text              T,           // This word
     Count             N);          // With this Count

virtual Index                       // The associated Index
   insert(                          // Insert
     Text              T);          // This word (Count 1)

//----------------------------------------------------------------------------
// Word_refs::random_select
//
// Randomly select Text (based on reference count)
//----------------------------------------------------------------------------
virtual Index                       // The selection Index
   random_select( void );           // Randomly select an instance

//----------------------------------------------------------------------------
// Word_refs::ref
//
// Count word reference.
//----------------------------------------------------------------------------
void
   ref(                             // Update reference
     Text              T);          // To this Text

//----------------------------------------------------------------------------
// Word_refs::reset
//
// Reset the map.
//----------------------------------------------------------------------------
virtual void
   reset( void );                   // Reset the map

//----------------------------------------------------------------------------
// Word_refs::(initializers)
//
// append: Append file to Word. Duplicates *ARE* allowed
// loader: Load file into Word. Duplicates *NOT* allowed
// File format: word:count\n...
//
// predef: Load Text into Word. Duplicates *NOT* allowed
// Text format: word\0...           // (Reference count 0)
//----------------------------------------------------------------------------
virtual void
   append(                          // Append (duplicates *ARE* allowed)
     const char*       name);       // The file name

virtual void
   loader(                          // Loader (duplicates *NOT* allowed)
     const char*       name);       // The file name

// No need to override Word::predef(const char*, size_t)
}; // class Word_refs

#endif // WORD_H_INCLUDED
