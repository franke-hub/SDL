//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       AutoDelete.h
//
// Purpose-
//       Automatic storage allocation/release object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef AUTODELETE_H_INCLUDED
#define AUTODELETE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       AutoDelete<class T>
//
// Purpose-
//       Automatic object deletion
//
// Usage-
//       {{{{
//         Thing* thing= new Thing(); // Allocate object
//         AutoDelete<Thing> autoThing(thing); // Insure object deletion
//         :                        // Any exit from scope deletes the object
//       }}}}
//
//       Note: auto is a keyword and cannot be used as an object name.
//
//----------------------------------------------------------------------------
template<class T>
class AutoDelete {                  // AutoDelete<T>
protected:
T*                     object;      // -> Object

public:
inline
   ~AutoDelete( void )              // Destructor
{
   if( object != NULL )
   {
     delete object;
     object= NULL;
   }
}

inline
   AutoDelete(                      // Constructor
     T*                object)      // Associated object
:  object(object)
{
}

inline T*                           // -> Object
   reset( void )                    // Reset this object
{
   T* result= object;
   object= NULL;
   return result;
}
}; // class AutoDelete

template<> class AutoDelete<void> {
protected:
void*                  buffer;      // -> Buffer

public:
inline
   ~AutoDelete( void )              // Destructor
{
   if( buffer != NULL )
   {
     free(buffer);
     buffer= NULL;
   }
}

inline
   AutoDelete(                      // Constructor
     void*             buffer)      // Associated storage
:  buffer(buffer)
{
}

inline void*                       // -> buffer
   reset( void )                    // Reset this object
{
   void* result= buffer;
   buffer= NULL;
   return result;
}
}; // class AutoDelete<void>

#endif // AUTODELETE_H_INCLUDED
