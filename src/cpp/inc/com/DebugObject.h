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
//       DebugObject.h
//
// Purpose-
//       Converts Object.h methods to outline methods for debugging.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Compile derived Objects using this header.
//       Methods that are already outline are not duplicated.
//
//----------------------------------------------------------------------------
#ifndef DEBUGOBJECT_H_INCLUDED
#define DEBUGOBJECT_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include "Object.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class DebugRef;

//----------------------------------------------------------------------------
//
// Class-
//       DebugObject
//
// Purpose-
//       Object base substitution, no inline methods.
//
//----------------------------------------------------------------------------
class DebugObject : public Object { // DebugObject base class
//----------------------------------------------------------------------------
// Object::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~DebugObject( void );            // Destructor

   DebugObject( void );             // Default constructor

   DebugObject(                     // Copy constructor
     const Object&     source);     // Source DebugObject

DebugObject&                        // (Always *this)
   operator=(                       // Assignment operator
     const Object&     source);
}; // class DebugObject

//----------------------------------------------------------------------------
//
// Class-
//       DebugRef<Object>
//
// Purpose-
//       Refer to an Object. This is also the DebugRef<class T> base class.
//
//----------------------------------------------------------------------------
template<> class DebugRef<Object> : public Ref<Object> { // DebugRef<Object>, the DebugRef<T> base class.
//----------------------------------------------------------------------------
// DebugRef<Object>::Destructor/Constructors/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~DebugRef( void );               // Destructor

   DebugRef( void );                // Default constructor

   DebugRef(                        // Constructor
     Object&           object);     // Source Object

   DebugRef(                        // Constructor
     Object*           object);     // Source -> Object

   DebugRef(                        // Copy constructor
     const Ref<Object>&source);     // Source Ref<Object>

DebugRef<Object>&                   // (*this)
   operator=(                       // DebugRef<Object>= Ref<Object>
     const Ref<Object>&source);     // Source Ref<Object>

DebugRef<Object>&                   // (*this)
   operator=(                       // DebugRef<Object>= object
     Object&           object);     // Source object

DebugRef<Object>&                   // (*this)
   operator=(                       // DebugRef<Object>= &object
     Object*           object);     // Source -> Object

//----------------------------------------------------------------------------
// DebugRef<Object>::Operators, Equality operators compare ADDRESSES, not objects
//----------------------------------------------------------------------------
public:
int                                 // TRUE iff get() == source.get()
   operator==(                      // Compare associated (Object*)s
     const Ref<Object>&source) const; // Source Ref<Object>

int                                 // TRUE iff get() != source.get()
   operator!=(                      // Compare associated (Object*)s
     const Ref<Object>&source) const; // Source Ref<Object>

//----------------------------------------------------------------------------
// DebugRef<Object>::Methods
//----------------------------------------------------------------------------
public:
Object*                             // -> Object (May be NULL)
   get( void ) const;               // Get associated Object*

Object&                             // Object& (Exception if NULL)
   use( void ) const;               // Get associated Object reference
}; // class DebugRef<Object>

//----------------------------------------------------------------------------
//
// Class-
//       DebugRef<T>
//
// Purpose-
//       Typed reference to an Object.
//
// Usage notes-
//       These methods remain inline since they refer to outline DebugRef
//
//----------------------------------------------------------------------------
template<class T>
class DebugRef : public DebugRef<Object> { // Typed reference
//----------------------------------------------------------------------------
// DebugRef::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~DebugRef( void ) {}             // Destructor

   DebugRef( void )                 // Default constructor
:  DebugRef<Object>() {}

   DebugRef(                        // Copy constructor
     const Ref<T>&     source)      // Source Ref<T>
:  DebugRef<Object>(source.object) {}

   DebugRef(                        // Constructor
     T&                object)      // Source Object
:  DebugRef<Object>(object) {}

   DebugRef(                        // Constructor
     T*                object)      // Source -> Object
:  DebugRef<Object>(object) {}

DebugRef<T>&                        // (*this)
   operator=(                       // DebugRef<T>= Ref<T>
     const Ref<T>&     source)      // Source Ref<T>
{  set((T*)source.object);
   return *this;
}

DebugRef<T>&                        // (*this)
   operator=(                       // DebugRef<T>= object
     T&                object)      // Source Object
{  set(&object);
   return *this;
}

DebugRef<T>&                        // (*this)
   operator=(                       // DebugRef<T>= -> Object
     T*                object)      // Source -> Object
{  set(object);
   return *this;
}

//----------------------------------------------------------------------------
// DebugRef<T>::Operators
//----------------------------------------------------------------------------
public:
T&                                  // Associated T&
   operator*( void ) const          // *DebugRef<T>
{  return use(); }

T*                                  // Associated T*
   operator->( void ) const         // DebugRef<T>->
{  return get(); }

//----------------------------------------------------------------------------
// DebugRef<T>::Methods
//----------------------------------------------------------------------------
public:
T*                                  // -> Object (May be NULL)
   get( void ) const                // Get associated T*
{  return (T*)object; }

T&                                  // Object& (Exception if NULL)
   use( void ) const                // Get associated Object reference
{  return (T&)DebugRef<Object>::use(); }
}; // class DebugRef<T>

//----------------------------------------------------------------------------
//
// Class-
//       DebugCloneable
//
// Purpose-
//       Adds clone method to Object
//
//----------------------------------------------------------------------------
class DebugCloneable : public Cloneable { // A DebugCloneable Object
//----------------------------------------------------------------------------
// DebugCloneable::Constructor
//----------------------------------------------------------------------------
public:
virtual
   ~DebugCloneable( void );         // Destructor
   DebugCloneable( void );          // Default constructor
   DebugCloneable(                  // Copy constructor
     const Cloneable&  source);

virtual Cloneable*                  // Cloneable*
   clone( void ) const = 0;         // Duplicate this object
}; // class DebugCloneable

//----------------------------------------------------------------------------
//
// Class-
//       DebugString
//
// Purpose-
//       Extend std::string as an Object
//
//----------------------------------------------------------------------------
class DebugString : public String { // DebugString Object
//----------------------------------------------------------------------------
// DebugString::Constructors/Destructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~DebugString( void );            // Destructor
   DebugString( void );             // Default constructor
   DebugString(                     // Copy constructor
     const String&     source);     // Source String

   DebugString(                     // std::string constructor
     std::string       source);     // Source std::string

   DebugString(                     // Construct a DebugString
     const char*       format,      // Format text
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(2,3);          // (printf argument validation)

   DebugString(                     // Construct a DebugString
     const char*       format,      // Format text
     va_list           argptr)      // The remaining arguments
   _ATTRIBUTE_PRINTF(2,0);          // (printf argument validation)

public:                             // Bitwise copy is allowed
DebugString&                        // (Always *this)
   operator=(                       // Assignment operator
     const String&     source);     // Source String

DebugString&                        // (Always *this)
   operator=(                       // Assignment operator
     const std::string&source);
}; // class DebugString

//----------------------------------------------------------------------------
//
// Description-
//       Global operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
// The Object.h inline methods invoke DebugObject outline methods.

//----------------------------------------------------------------------------
//
// Description-
//       Macros
//
// Purpose-
//       Converts compilation to use outline methods defined here.
//
//       Convert Object to DebugObject
//       Convert Cloneable to DebugCloneable
//       Convert String to DebugString
//       Convert Ref to DebugRef
//
//----------------------------------------------------------------------------
#define Object DebugObject
#define Cloneable DebugCloneable
#define String DebugString
#define Ref DebugRef

#endif // DEBUGOBJECT_H_INCLUDED
