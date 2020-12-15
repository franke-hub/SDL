//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdPool.h
//
// Purpose-
//       Editor: Storage Pool descriptor
//
// Last change date-
//       2020/12/14
//
//----------------------------------------------------------------------------
#ifndef EDPOOL_H_INCLUDED
#define EDPOOL_H_INCLUDED

#include <Editor.h>                 // For namespace editor::debug
#include <pub/List.h>               // For pub::List

//----------------------------------------------------------------------------
//
// Class-
//       EdPool
//
// Purpose-
//       Editor Pool
//
// Implementation note-
//       Lines are allocated and deleted, but text is never deleted
//
//----------------------------------------------------------------------------
class EdPool : public pub::List<EdPool>::Link { // Editor text pool descriptor
//----------------------------------------------------------------------------
// EdPool::Enumerations and typedefs
public:
enum // Compile time constants
{  MIN_SIZE= 65536                  // Minimum text pool size
}; // Compile time constants

//----------------------------------------------------------------------------
// EdPool::Attributes
protected:
size_t                 used;        // Number of bytes used
size_t                 size;        // The total Pool size
char*                  data;        // The Pool data area

//----------------------------------------------------------------------------
// EdPool::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdPool(                          // Constructor
     size_t            _size= 0)    // The pool allocation block size
:  ::pub::List<EdPool>::Link()
,  used(0), size(_size < MIN_SIZE ? size_t(MIN_SIZE) : _size)
,  data(new char[size])
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdPool(%p)::EdPool(%zd)\n", this, _size);
}

//----------------------------------------------------------------------------
virtual
   ~EdPool( void )                  // Destructor
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdPool(%p)::~EdPool, used %6zd of %6zd\n", this
                          , used, size);

   delete [] data;                  // Delete the data
}

//----------------------------------------------------------------------------
// EdPool::Methods
//----------------------------------------------------------------------------
public:
char*                               // The allocated storage, nullptr if none
   allocate(                        // Get allocated storage
     size_t            size)        // Of this length
{
   char* result= nullptr;
   if( used + size <= this->size ) { // If storage is available
     result= data + used;
     used += size;
   }

   if( editor::debug::opt_hcdm )
     editor::debug::debugh("%p= EdPool(%p)::allocate(%zd)\n", result, this, size);

   return result;
}
}; // class EdPool
#endif // EDPOOL_H_INCLUDED