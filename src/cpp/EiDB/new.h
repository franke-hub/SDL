//----------------------------------------------------------------------------
//
//       Copyright (C) 2004 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       new.h
//
// Purpose-
//       In-place operator new.
//
// Last change date-
//       2004/02/10
//
//----------------------------------------------------------------------------
#ifndef NEW_H_INCLUDED
#define NEW_H_INCLUDED

inline void*                        // ptrVoid
   operator new(                    // In-place operator new
     size_t          size,          // Size of object
     void*           ptrVoid)       // In-place address
{
   return ptrVoid;
}

#endif // NEW_H_INCLUDED
