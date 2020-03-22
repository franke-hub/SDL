//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Subpool.h
//
// Purpose-
//       Storage allocation pool.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef SUBPOOL_H_INCLUDED
#define SUBPOOL_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Subpool
//
// Purpose-
//       Storage allocation pool.
//
// Implementation note-
//       Storage is allocated from the Subpool using Subpool::allocate.
//       All Subpool storage is released when Subpool::release is invoked.
//       Subpool storage is semi-permanent in that no part of a subpool can
//       be released. Only the entire Subpool can be deallocated.
//
//       The allocate and strdup methods throw exceptions rather than
//       returning nullptr.
//
//----------------------------------------------------------------------------
class Subpool {                     // Storage allocation pool
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
public:
struct PoolLink;

//----------------------------------------------------------------------------
// Subpool::Attributes
//----------------------------------------------------------------------------
protected:
PoolLink*              head;        // -> First PoolLink
PoolLink*              tail;        // -> Last  PoolLink

//----------------------------------------------------------------------------
// Subpool::Constructors
//----------------------------------------------------------------------------
public:
   ~Subpool( void );                // Destructor
   Subpool( void );                 // Constructor

//----------------------------------------------------------------------------
// Subpool::Diagnostic methods
//----------------------------------------------------------------------------
public:
void
   diagnosticDump( void ) const;    // Diagnostic storage dump

//----------------------------------------------------------------------------
// Subpool::methods
//----------------------------------------------------------------------------
public:
void*                               // Allocated storage
   allocate(                        // Allocate from Subpool
     size_t            size);       // Required size

void
   release( void );                 // Delete all Subpool storage

char*                               // The duplicated string
   strdup(                          // Duplicate
     const char*       inp);        // This character string
}; // class SubPool

#endif // SUBPOOL_H_INCLUDED
