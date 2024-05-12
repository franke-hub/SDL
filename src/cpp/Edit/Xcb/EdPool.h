//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
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
//       2024/05/05
//
//----------------------------------------------------------------------------
#ifndef EDPOOL_H_INCLUDED
#define EDPOOL_H_INCLUDED

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List

#include "Config.h"                 // For namespace config

//----------------------------------------------------------------------------
//
// Class-
//       EdPool
//
// Purpose-
//       Editor Pool
//
// Implementation note-
//       Lines are allocated and deleted, but pool text is only allocated.
//       Text is immutable. EdPools remain allocated until Editor completion.
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
     size_t            size_= 0)    // The pool allocation block size
:  ::pub::List<EdPool>::Link()
,  used(0), size(size_ < MIN_SIZE ? size_t(MIN_SIZE) : size_)
,  data(new char[size])
{  using namespace config; using namespace pub::debugging;

   if( opt_hcdm )
     traceh("EdPool(%p)::EdPool(%zd)\n", this, size_);
}

virtual
   ~EdPool( void )                  // Destructor
{  using namespace config; using namespace pub::debugging;

   if( opt_hcdm )
     traceh("EdPool(%p)::~EdPool, used %6zd of %6zd\n", this, used, size);

   delete [] data;                  // Delete the data
}

//----------------------------------------------------------------------------
// EdPool::Accessor methods
//----------------------------------------------------------------------------
public:
size_t get_size( void ) const { return size; }
size_t get_used( void ) const { return used; }

//----------------------------------------------------------------------------
// EdPool::Methods
//----------------------------------------------------------------------------
char*                               // The allocated storage, nullptr if none
   allocate(                        // Get allocated storage
     size_t            size)        // Of this length
{  using namespace config; using namespace pub::debugging;

   char* result= nullptr;
   if( used + size <= this->size ) { // If storage is available
     result= data + used;
     used += size;
   }

   if( opt_hcdm )
     traceh("%p= EdPool(%p)::allocate(%zd)\n", result, this, size);

   return result;
}
}; // class EdPool
#endif // EDPOOL_H_INCLUDED
