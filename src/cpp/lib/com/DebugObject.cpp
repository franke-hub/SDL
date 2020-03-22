//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DebugObject.cpp
//
// Purpose-
//       DebugObject implementation methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Atomic.h>
#include <com/AutoPointer.h>
#include <com/Debug.h>

#include "com/DebugObject.h"
#undef  Object
#undef  Cloneable
#undef  String
#undef  Ref

#ifdef _OS_WIN
  #define vsnprintf _vsnprintf

  #ifndef va_copy
    #define va_copy(dest, src) dest= src
  #endif
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#include "com/ifmacro.h"

//----------------------------------------------------------------------------
//
// Method-
//       DebugObject::*
//
// Purpose-
//       DebugObject implementaton.
//
//----------------------------------------------------------------------------
   DebugObject::~DebugObject( void ) // Destructor
{  IFHCDM( debugf("DebugObject(%p)::~DebugObject()\n", this); )
}

   DebugObject::DebugObject( void ) // Default constructor
:  Object()
{  IFHCDM( debugf("DebugObject(%p)::DebugObject()\n", this); )
}

   DebugObject::DebugObject(        // Copy constructor
     const Object&     source)      // Source DebugObject
:  Object(source)
{  IFHCDM( debugf("DebugObject(%p)::DebugObject(Object& %p)\n", this, &source); )
}

DebugObject&                        // (Always *this)
   DebugObject::operator=(          // Assignment operator
     const Object&     source)
{  IFHCDM( debugf("DebugObject(%p)::operator=(Object& %p)\n", this, &source); )

   Object::operator=(source);
   return (*this);
}

//----------------------------------------------------------------------------
//
// Method-
//       DebugRef<Object>::*
//
// Purpose-
//       DebugRef<Object> implementation.
//
//----------------------------------------------------------------------------
   DebugRef<Object>::~DebugRef( void ) // Destructor
{  IFHCDM( debugf("DebugRef<Object>(%p)::~DebugRef()\n", this); )
}

   DebugRef<Object>::DebugRef( void ) // Default constructor
:  Ref<Object>()
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef()\n", this); )
}

   DebugRef<Object>::DebugRef(      // Constructor
     Object&           object)      // Source Object
:  Ref<Object>(object)
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Object& %p)\n", this, &object); )
}

   DebugRef<Object>::DebugRef(      // Constructor
     Object*           object)      // Source -> Object
:  Ref<Object>(object)
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Object* %p)\n", this, object); )
}

   DebugRef<Object>::DebugRef(      // Copy constructor
     const Ref<Object>&source)      // Source Ref<Object>
:  Ref<Object>(source)
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Ref& %p)\n", this, &source); )
}

DebugRef<Object>&                   // (*this)
   DebugRef<Object>::operator=(     // DebugRef<Object>= Ref<Object>
     const Ref<Object>&source)      // Source Ref<Object>
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Ref& %p)\n", this, &source); )

   Ref<Object>::operator=(source);
   return (*this);
}

DebugRef<Object>&                   // (*this)
   DebugRef<Object>::operator=(     // DebugRef<Object>= object
     Object&           object)      // Source object
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Object& %p)\n", this, &object); )

   Ref<Object>::operator=(object);
   return (*this);
}

DebugRef<Object>&                   // (*this)
   DebugRef<Object>::operator=(     // DebugRef<Object>= &object
     Object*           object)      // Source -> Object
{  IFHCDM( debugf("DebugRef<Object>(%p)::DebugRef(Object* %p)\n", this, object); )

   Ref<Object>::operator=(object);
   return (*this);
}

int                                 // True iff get() == source.get()
   DebugRef<Object>::operator==(    // DebugRef<Object>= &object
     const Ref<Object>&source) const // Source Ref<Object>
{  IFHCDM( debugf("DebugRef<Object>(%p)::operator==(Ref& %p)\n", this, &source); )

   return Ref<Object>::operator==(source);
}

int                                 // True iff get() != source.get()
   DebugRef<Object>::operator!=(    // DebugRef<Object>= &object
     const Ref<Object>&source) const // Source Ref<Object>
{  IFHCDM( debugf("DebugRef<Object>(%p)::operator!=(Ref& %p)\n", this, &source); )

   return Ref<Object>::operator!=(source);
}

Object*
   DebugRef<Object>::get( void ) const // Get assocated Object*
{  IFHCDM( debugf("DebugRef<Object>(%p)::get()\n", this); )

   return Ref<Object>::get();
}

Object&
   DebugRef<Object>::use( void ) const // Use assocated Object*
{  IFHCDM( debugf("DebugRef<Object>(%p)::use()\n", this); )

   return Ref<Object>::use();
}

//----------------------------------------------------------------------------
//
// Method-
//       DebugCloneable::*
//
// Purpose-
//       DebugCloneable implementation.
//
//----------------------------------------------------------------------------
   DebugCloneable::~DebugCloneable( void ) // Destructor
{  IFHCDM( debugf("DebugCloneable(%p)::~DebugCloneable()\n", this); )
}

   DebugCloneable::DebugCloneable( void ) // Default constructor
:  Cloneable()
{  IFHCDM( debugf("DebugCloneable(%p)::DebugCloneable()\n", this); )
}

   DebugCloneable::DebugCloneable(  // Copy constructor
     const Cloneable&  source)
:  Cloneable(source)
{  IFHCDM( debugf("DebugCloneable(%p)::DebugCloneable(Cloneable& %p)\n", this, &source); )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       format
//
// Purpose-
//       Create string from printf format arguments
//
//----------------------------------------------------------------------------
static std::string                  // Resultant
   stringf(                         // Create string from printf arguments
     const char*       format,      // Format string
     va_list           argptr)      // PRINTF arguments
{
   std::string         result;      // Resultant

   char workBuff[512];              // Working buffer
   char* buffer= workBuff;          // -> Buffer

   va_list outptr;
   va_copy(outptr, argptr);
   unsigned L= vsnprintf(buffer, sizeof(workBuff), format, outptr);
   va_end(outptr);

   if( L < sizeof(workBuff) )       // If the normal case
     result= std::string(buffer);
   else
   {
     AutoPointer work(L+1);         // Allocate a work buffer
     buffer= (char*)work.get();

     vsnprintf(buffer, L, format, argptr);
     result= std::string(buffer);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DebugString::*
//
// Purpose-
//       DebugString implementation.
//
//----------------------------------------------------------------------------
   DebugString::~DebugString( void ) // Destructor
{  IFHCDM( debugf("String(%p)::~String()\n", this); )
}

   DebugString::DebugString(        // Constructor
     const char*       format,      // Format string
                       ...)         // PRINTF arguments
:  String()
{
   IFHCDM( debugf("String(%p)::String(%s,...)\n", this, format); )

   va_list             argptr;

   va_start(argptr, format);        // Initialize va_ functions
   *this= stringf(format, argptr);  // Construct via assignment
   va_end(argptr);                  // Close va_ functions
}

   DebugString::DebugString(        // Copy constructor
     const char*       format,      // Format string
     va_list           argptr)      // PRINTF arguments
:  String(format, argptr)
{
   IFHCDM( debugf("DebugString(%p)::DebugString(%s,va_list)\n", this, format); )
}

DebugString&
   DebugString::operator=(          // Assignment operator
     const String&     source)      // Source string
{  IFHCDM( debugf("DebugString(%p)::operator=(String& %p)\n", this, &source); )

   String::operator=(source);
   return (*this);
}

DebugString&
   DebugString::operator=(          // Assignment operator
     const std::string&source)      // Source string
{  IFHCDM( debugf("DebugString(%p)::operator=(std::string& %p)\n", this, &source); )

   String::operator=(source);
   return (*this);
}

