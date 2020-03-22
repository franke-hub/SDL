//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
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
//       Editor: Text buffer pool.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDPOOL_H_INCLUDED
#define EDPOOL_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdPool
//
// Purpose-
//       Define editor text pool.
//
//----------------------------------------------------------------------------
class EdPool : public List<void> {  // Editor text buffer storage pool
//----------------------------------------------------------------------------
// EdPool::Constructors
//----------------------------------------------------------------------------
public:
   ~EdPool( void );                 // Destructor

   EdPool( void );                  // Constructor

//----------------------------------------------------------------------------
// EdPool::Methods
//----------------------------------------------------------------------------
public:
char*                               // -> Allocated text string
   allocate(                        // Allocate text from ring
     unsigned          size,        // Of this length
     unsigned          align = 8);  // and this alignment

void
   release(                         // Release text string
     char*             string);     // -> Text string

void
   reset( void );                   // Release all pool storage

//----------------------------------------------------------------------------
// EdPool::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EdPool::Attributes
//----------------------------------------------------------------------------
protected:
}; // class EdPool

#endif // EDPOOL_H_INCLUDED
