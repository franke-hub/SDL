//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Unconditional.h
//
// Purpose-
//       Unconditional functions throw(char*) on failure.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef UNCONDITIONAL_H_INCLUDED
#define UNCONDITIONAL_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Unconditional
//
// Purpose-
//       Unconditional static functions throw(char*) on failure.
//
//----------------------------------------------------------------------------
class Unconditional {               // Unconditional static functions
public:                             // All (static) methods are public
//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::malloc
//
// Purpose-
//       Allocate storage, throw exception on failure.
//
//----------------------------------------------------------------------------
static void*                        // -> Allocated storage (Never NULL)
   malloc(                          // Unconditionally allocate storage
     size_t            size);       // Of this size

//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::replace
//
// Purpose-
//       Replace an existing (allocated) string with a new allocated string.
//
// Notes-
//       The current and/or replacement strings may be NULL.
//
// Usage-
//       static char* foo= NULL;
//       foo= replace(foo, "old");
//       foo= replace(foo, "new");
//       :
//       foo= replace(foo, NULL);
//
//----------------------------------------------------------------------------
static char*                        // -> Replacement string (Never NULL)
   replace(                         // Replace an allocated string
     char*             current,     // -> Current (allocated) string
     const char*       replace);    // -> Replacement string

//----------------------------------------------------------------------------
//
// Method-
//       Unconditional::strdup
//
// Purpose-
//       Copy string, throw exception on failure.
//
//----------------------------------------------------------------------------
static char*                        // -> String copy (Never NULL)
   strdup(                          // Duplicate
     const char*       source);     // This string
}; // class Unconditional

//----------------------------------------------------------------------------
// Associated inline functions
//----------------------------------------------------------------------------
static inline void*
   must_malloc(                     // Unconditionally allocate storage
     size_t            size)        // Of this size
{  return Unconditional::malloc(size); }

static inline char*                 // -> Replacement string (Never NULL)
   must_replace(                    // Replace an allocated string
     char*             current,     // -> Current (allocated) string
     const char*       replace)     // -> Replacement string
{  return Unconditional::replace(current,replace); }

static inline char*                 // -> String copy (Never NULL)
   must_strdup(                     // Duplicate
     const char*       source)      // This string
{  return Unconditional::strdup(source); }

#endif // UNCONDITIONAL_H_INCLUDED
