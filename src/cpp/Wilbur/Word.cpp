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
//       Word.cpp
//
// Purpose-
//       Word.h object methods.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp

#include <map>                      // For std::map
#include <stdexcept>                // For std::runtime_error

#include <com/Debug.h>
#include <com/Must.h>
#include <com/Random.h>
#include <com/Subpool.h>
#include "Context.h"

#include "Word.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define EXPAND_COUNT   65536        // Expansion count
#define INITIAL_SIZE 1048576        // Initial w_size

#ifndef SCDM
#define SCDM false                  // Soft Core Debug Mode?
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     predef_array[]= // Predefined words
{  ""                               // The empty word
,  nullptr                          // End of list delimiter
}; // predef_array[]

//----------------------------------------------------------------------------
//
// Protected method-
//       Word::expand
//
// Purpose-
//       Expand the mapping Index
//
//----------------------------------------------------------------------------
Word::Count                         // The new w_size
   Word::expand(                    // Expand the map Index
     Count             count)       // With this count
{  if( SCDM ) debugf("Word(%p)::expand(%d)\n", this, count);

   if( count == 0 )                 // If no increase
     return w_size;

   Count new_size= w_size + count;
   if( new_size < w_size )
     throw std::overflow_error("Word::expand");

   // Copy the w_text array
   Text* new_text= (Text*)Must::malloc(new_size * sizeof(Text*));
   for(Count i= 0; i<w_used; i++)
     new_text[i]= w_text[i];

   for(Count i= w_used; i<new_size; i++)
     new_text[i]= nullptr;

   // Replace the w_text array
   Must::free(w_text);
   w_text= new_text;
   w_size= new_size;

   return new_size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::Word
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Word::Word( void )               // Constructor
:  w_map(), w_pool(), w_size(0), w_used(0), w_text(nullptr)
{  if( SCDM ) debugf("Word(%p)::Word()\n", this);

   expand(INITIAL_SIZE);            // Create the initial map
   reset();                         // Load the predefined words
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::~Word
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Word::~Word( void )             // Destructor
{  if( SCDM ) debugf("Word(%p)::~Word()\n", this);

   Must::free(w_text);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Word::debug(                     // Debugging display
     unsigned          verbose) const // Verbosity. Larger is more verbose
{
   debugf("Word(%p)::debug(%u)\n", this, verbose);
   debugf("..w_text(%p) w_size(%d) w_used(%d)\n", w_text, w_size, w_used);

   if( verbose )
   {
     for(Index i= 0; i<w_used; i++)
       debugf("[%8d] %s\n", i, w_text[i]);

     if( verbose > 4 )
     {
       debugf("w_map(%p)\n", &w_map);
       for(Index i= 0; i<w_used; i++)
       {
         Text  T= w_text[i];
         Index I= index(T);
         debugf("[%8d] %s\n", I, T);
       }
     }

     if( verbose == 101 )
       w_pool.diagnosticDump();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::index
//
// Purpose-
//       Convert Index to Text
//       Convert Text to Index
//
//----------------------------------------------------------------------------
Word::Text                          // The associated Text
   Word::index(                     // Get associated Text
     Index             X) const     // For this Index
{  if( X >= w_used )
     throw std::range_error("Word::index");

   return w_text[X];
}

Word::Index                         // The associated Index
   Word::index(                     // Get associated Index
     Text              T) const     // For this Text
{
   w_map_t::const_iterator it= w_map.find(T); // Locate word index
   if( it == w_map.end() )          // If non-existent
     throw std::range_error("Word::index");

   return it->second;               // Get associated index
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::insert
//
// Purpose-
//       Insert reference to Text
//
//----------------------------------------------------------------------------
Word::Index                         // The new Index
   Word::insert(                    // Insert reference
     Text              T)           // To this Text
{
   w_map_t::iterator it= w_map.find(T); // Locate word index
   if( it != w_map.end() )          // If duplicate
   {
     fprintf(stderr, "Word::insert(%s) duplicates(%d)\n",
                     T, it->second);
     throw std::invalid_argument("duplicate");
   }

   if( w_used >= w_size )
     expand(EXPAND_COUNT);

   Index X= w_used;               // Next index
   char* copy= w_pool.strdup(T);  // Create copy in subpool
   w_map[copy]= X;                // Insert into map
   w_text[w_used++]= copy;        // Insert into map text array

   return X;                      // Return new Index
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::random_select
//
// Purpose-
//       Randomly select a Text instance
//
//----------------------------------------------------------------------------
Word::Index                         // The selection Index
   Word::random_select( void )      // Randomly select an instance
{
   Total selector= Random::standard.get(); // Generate selector
   while( selector == 0 )
     selector= Random::standard.get();

   selector= selector % w_used;
   return selector;
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::reset
//
// Purpose-
//       Reset the Word reference table
//
//----------------------------------------------------------------------------
void
   Word::reset( void )              // Reset
{  if( SCDM ) debugf("Word(%p)::reset()\n", this);

   // Delete subpool storage
   w_pool.release();

   // Reset the counters
   w_used= 0;

   // Reset the w_map
   w_map.clear();

   // Load the predefined words (one at a time)
   for(Index i= 0; predef_array[i]; i++)
     predef(predef_array[i], strlen(predef_array[i])+1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::append
//
// Purpose-
//       Append file
//
//----------------------------------------------------------------------------
void
   Word::append(                    // Append file
     const char*       name)        // The file name
{
   throw std::runtime_error("NOT CODED YET");
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::loader
//
// Purpose-
//       File loader
//
//----------------------------------------------------------------------------
void
   Word::loader(                    // File loader
     const char*       name)        // The file name
{
   throw std::runtime_error("NOT CODED YET");
}

//----------------------------------------------------------------------------
//
// Method-
//       Word::predef
//
// Purpose-
//       Insert pre-defined Text
//
//----------------------------------------------------------------------------
void
   Word::predef(                    // Text loader
     const char*       name,        // The Text
     size_t            size)        // The Text size
{
   while( size > 0 )                // Insert pre-defined Text
   {
     size_t L= strlen(name) + 1;
     if( L > size )
       throw std::length_error("Word::predef");

     w_map_t::iterator it= w_map.find(name); // Locate word index
     if( it != w_map.end() )          // If duplicate
     {
       fprintf(stderr, "Word::predef(%s) duplicates(%d)\n",
                       name, it->second);
       throw std::invalid_argument("duplicate");
     }

     if( w_used >= w_size )
       expand(EXPAND_COUNT);

     w_map[name]= w_used;         // Add to map
     w_text[w_used++]= name;      // Add to Index

     name += L;
     size -= L;
   }
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Word_refs::expand
//
// Purpose-
//       Expand the mapping Index
//
//----------------------------------------------------------------------------
Word::Count                         // The new w_size
   Word_refs::expand(               // Expand the map Index
     Count             count)       // With this count
{  if( SCDM ) debugf("Word_refs(%p)::expand(%d)\n", this, count);

   if( count == 0 )                 // If no increase
     return w_size;

   Count new_size= w_size + count;
   if( new_size < w_size )
     throw std::overflow_error("Word_refs::expand");

   Count* new_refs= nullptr;
   try {
     // Copy the w_refs array
     new_refs= (Count*)Must::malloc(new_size * sizeof(Count*));
     new_size= Word::expand(count);

     for(Count i= 0; i<w_used; i++)
       new_refs[i]= w_refs[i];

     for(Count i= w_used; i<w_size; i++)
       new_refs[i]= 0;
   } catch( ... ) {
     Must::free(new_refs);
     throw;
   }

   // Replace the w_refs array
   Must::free(w_refs);
   w_refs= new_refs;

   return new_size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::Word_refs
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Word_refs::Word_refs( void )     // Constructor
:  Word(), w_refs(nullptr), w_total(0)
{  if( SCDM ) debugf("Word_refs(%p)::Word_refs()\n", this);

   // Allocate the refs table
   w_refs= (Count*)Must::malloc(w_size * sizeof(Count*));

   for(Count i= 0; i<w_size; i++)   // Initialize to zero
     w_refs[i]= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::~Word_refs
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Word_refs::~Word_refs( void )    // Destructor
{  if( SCDM ) debugf("Word_refs(%p)::~Word_refs()\n", this);

   Must::free(w_refs);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Word_refs::debug(                // Debugging display
     unsigned          verbose) const // Verbosity. Larger is more verbose
{
   Word::debug();

   if( verbose )
   {
     for(Index i= 0; i<w_used; i++)
       debugf("[%8d] %10d %s\n", i, w_refs[i], w_text[i]);
     debugf("[--------] %10zd *TOTAL*\n", w_total);

     if( verbose == 101 )
       w_pool.diagnosticDump();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::insert
//
// Purpose-
//       Insert reference to Text
//
//----------------------------------------------------------------------------
Word::Index                         // The new Index
   Word_refs::insert(               // Insert reference
     Text              T,           // To this Text
     Count             count)       // With this count
{
   Index X= Word::insert(T);        // Insert into map
   w_refs[X]= count;                // Update individual count
   w_total += count;                // Update total count

   return X;                        // Return new Index
}

Word::Index                         // The new Index
   Word_refs::insert(               // Insert reference
     Text              T)           // To this Text
{  return insert(T, 1); }

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::random_select
//
// Purpose-
//       Randomly select a Text instance (probability based)
//
//----------------------------------------------------------------------------
Word::Index                         // The selection Index
   Word_refs::random_select( void ) // Randomly select an instance
{
   Total selector= Random::standard.get(); // Generate selector
   while( selector == 0 )
     selector= Random::standard.get();

   selector= selector % w_total;
   for(Index i= 1; i < w_used; i++)
   {
     if( w_refs[i] > selector ) return i;
     selector -= w_refs[i];
   }

   throwf("%4d Word.cpp SHOULD NOT OCCUR", __LINE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::ref
//
// Purpose-
//       Count Text reference
//
//----------------------------------------------------------------------------
void
   Word_refs::ref(                  // Count reference
     Text              T)           // To this Text
{
   Index               X= 0;        // The associated index

   w_map_t::iterator it= w_map.find(T); // Locate word index
   if( it == w_map.end() )          // If non-existent
     X= insert(T, 0);               // Insert into map
   else
     X= it->second;

   if( X != 0 )                     // If a countable index
   {
     w_refs[X]++;                   // Update individual count
     w_total++;                     // Update total count
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::reset
//
// Purpose-
//       Reset the map
//
//----------------------------------------------------------------------------
void
   Word_refs::reset( void )         // Reset
{  if( SCDM ) debugf("Word_refs(%p)::reset()\n", this);

   // Reset the counters
   w_total= 0;

   // Reset the base class
   Word::reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::append
//
// Purpose-
//       Append file
//
//----------------------------------------------------------------------------
void
   Word_refs::append(               // Append file
     const char*       name)        // The file name
{
   throw std::runtime_error("NOT CODED YET");
}

//----------------------------------------------------------------------------
//
// Method-
//       Word_refs::loader
//
// Purpose-
//       File loader
//
//----------------------------------------------------------------------------
void
   Word_refs::loader(               // File loader
     const char*       name)        // The file name
{
   throw std::runtime_error("NOT CODED YET");
}

