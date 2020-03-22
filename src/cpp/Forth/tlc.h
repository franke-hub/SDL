//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tlc.h
//
// Purpose-
//       Common include, defines macros and externals.
//
// Last change date-
//       2020/01/25
//
//----------------------------------------------------------------------------
#ifndef TLC_H_INCLUDED
#define TLC_H_INCLUDED

#include <map>                      // For std::map
#include <string>                   // For std::string
#include <stdint.h>                 // For size_t, ...
#include <stdio.h>                  // For putchar, ...
#include <stdlib.h>                 // For malloc, ...
#include <string.h>                 // For strcmp, strcpy, ...

#include <pub/Debug.h>              // For pub::Debug::throwf
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For dump, to_string

using pub::Debug;
using pub::Exception;
using pub::utility::to_string;
using namespace pub::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef CHECK
#define CHECK                       // If defined, checking active
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef CHECK
#define IFCHECK(X) {X}
#define ELCHECK(X) { }
#else
#define IFCHECK(X) { }
#define ELCHECK(X) {X}
#endif

//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
typedef void     (*Code)(void);     // A Code element
typedef intptr_t   Data;            // A Data stack entry
typedef void*      Word;            // An internal Code word

//----------------------------------------------------------------------------
//
// Struct-
//       Stack<type>
//
// Purpose-
//       Define the generic Stack
//
//----------------------------------------------------------------------------
template <class T>
struct Stack {                      // The stack
size_t                 size= 0;     // Number of elements
size_t                 used= 0;     // Number of elements used
T*                     item= nullptr; // The T[] array

T operator[](size_t index)          // Index operator
{
   if( index >= used )
     pub::debugging::throwf("Stack::[%zd] range(%zd)", index, used);

   return item[index];
}

T                                   // The removed element
   pop( void )                      // Pop element from Stack
{
   if( used == 0 )                  // If underflow
     pub::debugging::throwf("Stack::pop underflow");

   return item[--used];
}

size_t                              // The element index
   push(                            // Add element to Stack
     T                 element)     // The element to add
{
   if( used >= size )               // If overflow
     pub::debugging::throwf("Stack::push overflow");

   item[used]= element;
   return used++;
}

T                                   // The top Stack element
   top( void )                      // Get top Stack element
{
   if( used == 0 )                  // If underflow
     pub::debugging::throwf("Stack::top underflow");

   return item[used-1];
}
}; // struct Stack

//----------------------------------------------------------------------------
//
// Class-
//       WordMap
//
// Purpose-
//       The Name to Word Map
//
// Implementation notes-
//       Can't make locate (and therefor operator[](std::string)) const
//
//----------------------------------------------------------------------------
class WordMap {                     // The Name to Word Map
//----------------------------------------------------------------------------
// WordMap::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Word*>
                       Map_t;
typedef Map_t::iterator
                       MapIter_t;

//----------------------------------------------------------------------------
// WordMap::Attributes
//----------------------------------------------------------------------------
protected:
Map_t                  map;         // The actual map

//----------------------------------------------------------------------------
// WordMap::Constructors
//----------------------------------------------------------------------------
public:                             // Attribute accessors
   ~WordMap( void ) {}
   WordMap( void )
:  map() {}

//----------------------------------------------------------------------------
// WordMap::Methods
//----------------------------------------------------------------------------
public:                             // Attribute accessors
MapIter_t begin() noexcept
{  return map.begin(); }

MapIter_t end() noexcept
{  return map.end(); }

//----------------------------------------------------------------------------
// WordMap::operators
//----------------------------------------------------------------------------
public:
Word*                               // The associated Word, if present
   locate(std::string name)         // Get associated Word
{
   Word* word= nullptr;             // The associated Word*

   const MapIter_t mi= map.find(name);
   if( mi != map.end() )            // If it's mapped
     word= mi->second;

   return word;
}

Word*                               // The associated Word, if present
   locate(const char* name)         // Get associated Word
{  return locate(std::string(name)); }

void
   insert(std::string name, Word* word) // Insert Word* into Map
{
   const MapIter_t mi= map.find(name);
   if( mi != map.end() )            // If it's already mapped
     throw Exception(to_string("Word::insert(%s) duplicated",
                     name.c_str()));

   map[name]= word;
}

Word&                               // The associated Word
   operator[](std::string name)     // Locate Word, name must be registered
{
   Word* word= locate(name);        // Get associated Word
   if( word == nullptr )            // If the Word isn't mapped
       throw Exception(to_string("WordMap::[%s] not found",
                       name.c_str()));

   return *word;
}
}; // class WordMap
#endif // TLC_H_INCLUDED
