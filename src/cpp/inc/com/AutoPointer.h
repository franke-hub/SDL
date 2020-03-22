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
//       AutoPointer.h
//
// Purpose-
//       Automatic storage allocation/release object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef AUTOPOINTER_H_INCLUDED
#define AUTOPOINTER_H_INCLUDED

#ifndef DEBUG_H_INCLUDED
#include "Debug.h"                  // For throwf
#endif

//----------------------------------------------------------------------------
//
// Class-
//       AutoPointer
//
// Purpose-
//       Automatic storage allocation/release object
//
// Usage-
//       {{{{
//         AutoPointer aptr(sizeof(Thing)); // Allocate storage
//         // "NoStorageException" is thrown if storage cannot be allocated.
//         void* pointer= aptr.get(); // Retrieve the pointer
//         :                        // Any exit from scope releases the storage
//       }}}}
//
//       {{{{
//         char* pointer= malloc(sizeof(Thing)); // Allocate storage
//         assert( pointer != NULL ); // Assure not NULL
//         AutoPointer aptr(pointer); // Insure storage deallocation
//         :                        // Any exit from scope releases the storage
//       }}}}
//
//----------------------------------------------------------------------------
class AutoPointer {                 // Automatic storage allocation/release
protected:
void*                  pointer;     // -> Allocated storage

public:
inline
   ~AutoPointer( void )             // Destructor
{
   if( pointer != NULL )
   {
     free(pointer);
     pointer= NULL;
   }
}

inline
   AutoPointer(                     // Constructor
     unsigned long     size)        // Storage allocation length
:  pointer(malloc(size))
{
   if( pointer == NULL )
     throwf("No storage(%lu)\n", size);
}

inline
   AutoPointer(                     // Constructor
     void*             pointer)     // Pointer
:  pointer(pointer)
{
}

inline void*                        // -> Storage
   get( void ) const                // Return -> storage
{
   return pointer;
}

inline void*                        // -> Storage
   set(                             // Replace -> Storage
     void*             pointer)     // With this
{
   if( this->pointer != NULL )
     free(this->pointer);

   this->pointer= pointer;
   return pointer;
}

inline void*                        // -> Storage
   take( void )                     // Take over control of storage
{
   void* result= pointer;
   pointer= NULL;
   return result;
}
}; // class AutoPointer

#endif // AUTOPOINTER_H_INCLUDED
