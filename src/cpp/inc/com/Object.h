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
//       Object.h
//
// Purpose-
//       Garbage collected Object and associated helper objects.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Also defines Ref<T>, the Cloneable and String Objects, and the
//       global comparison operators (which use object.compare()).
//
//       Objects are deleted when they are no longer referenced.
//       Objects created in the runtime stack MUST NOT be Pointer referenced.
//
// Usage notes-
//       Use DebugObject to convert inline methods to outline methods.
//       (This may be required when debugging.)
//
//----------------------------------------------------------------------------
#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

#include <stdarg.h>                 // For String printf methods
#include <ostream>                  // For global operators
#include <stdint.h>                 // For int32_t (Precise size required)
#include <string>                   // String base class
#include <com/define.h>             // For NULL, _ATTRIBUTE_PRINTF

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class Ref;

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       Object base class.
//
//----------------------------------------------------------------------------
class Object {                      // Object base class
friend class Ref<Object>;           // (Only updater of reclaim/refCount)

//----------------------------------------------------------------------------
// Object::Attributes (Only used in _Ref class)
//----------------------------------------------------------------------------
private:
Object* volatile       reclaim;     // Reclaim chain pointer
int32_t volatile       refCount;    // Number of references to this Object

static int32_t volatile objectCount; // Number of allocated objects

//----------------------------------------------------------------------------
// Object::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
static void                         // Requires activation in Object.cpp
   objectCounter(                   // Update objCount
     int               count);      // With this count

public:
virtual inline
   ~Object( void ) { objectCounter(-1); } // Destructor

inline
   Object( void )                   // Default constructor
:  reclaim(NULL), refCount(0) { objectCounter(+1); }

public:                             // Bitwise copy/assignment allowed
inline
   Object(                          // Copy constructor
     const Object&     source)      // Source Object&
:  reclaim(NULL), refCount(0) { objectCounter(+1); }

inline Object&                      // (Always *this)
   operator=(                       // Assignment operator
     const Object&     source)
{  return *this; }

//----------------------------------------------------------------------------
// Object::Exceptions
//----------------------------------------------------------------------------
protected:
void
   compareCastException(            // Throw CompareCastException
     const char*       name) const; // For this class name

//----------------------------------------------------------------------------
// Object::Accessors
//----------------------------------------------------------------------------
public:
// getObjectCounter requires code activation in Object.cpp
static inline int getObjectCounter() { return objectCount; }

//----------------------------------------------------------------------------
// Object::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     object) const; // This Object&

virtual unsigned                    // A hash code value for this Object
   hashf( void ) const;             // Create hash code value for Object

virtual std::string                 // A String representation of this Object
   toString( void ) const;          // Represent this Object as a String
}; // class Object

//----------------------------------------------------------------------------
//
// Class-
//       Ref<Object>
//
// Purpose-
//       Refer to an Object. This is also the Ref<class T> base class.
//
//----------------------------------------------------------------------------
template<> class Ref<Object> {      // Ref<Object>, the Ref<T> base class.
//----------------------------------------------------------------------------
// Ref<Object>::Attributes
//----------------------------------------------------------------------------
protected:
Object* volatile       object;      // Associated Object*

//----------------------------------------------------------------------------
// Ref<Object>::Destructor/Constructors/Assignment
//----------------------------------------------------------------------------
public:
virtual inline
   ~Ref( void )                     // Destructor
{  set(NULL); }

inline
   Ref( void )                      // Default constructor
:  object(NULL) {}

inline
   Ref(                             // Copy constructor
     const Ref<Object>&source)      // Source Ref<Object>
:  object(NULL)
{  set(source.object); }

inline
   Ref(                             // Constructor
     Object&           source)      // Source Object&
:  object(NULL)
{  set(&source); }

inline
   Ref(                             // Constructor
     Object*           source)      // Source Object*
:  object(NULL)
{  set(source); }

inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<Object>= Ref<Object>
     const Ref<Object>&source)      // Source Ref<Object>
{  set(source.object);
   return *this;
}

inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<Object>= Object&
     Object&           source)      // Source Object&
{  set(&source);
   return *this;
}

inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<Object>= Object*
     Object*           source)      // Source Object*
{  set(source);
   return *this;
}

//----------------------------------------------------------------------------
// Ref<Object>::Operators, Equality operators compare ADDRESSES, not objects
//----------------------------------------------------------------------------
public:
inline int                          // TRUE iff get() == source.get()
   operator==(                      // Compare associated (Object*)s
     const Ref<Object>&source) const // Source Ref<Object>
{  return ((void*)object == (void*)source.object); }

inline int                          // TRUE iff get() != source.get()
   operator!=(                      // Compare associated (Object*)s
     const Ref<Object>&source) const // Source Ref<Object>
{  return ((void*)object != (void*)source.object); }

inline Object&                      // Associated Object&
   operator*( void ) const          // *(Ref<Object>)
{  return use(); }

inline Object*                      // Associated Object*
   operator->( void ) const         // (Ref<Object>)->
{  return get(); }

//----------------------------------------------------------------------------
// Ref<Object>::Exceptions
//----------------------------------------------------------------------------
protected:
void
   nullPointerException( void ) const; // Throw "NullPointerException"

//----------------------------------------------------------------------------
// Ref<Object>::Methods
//----------------------------------------------------------------------------
public:
inline Object*                      // Object* (May be NULL)
   get( void ) const                // Get associated Object*
{  return object; }

inline Object&                      // Object& (Exception if NULL)
   use( void ) const                // Get associated Object reference
{  Object* O= object;
   if( O == NULL )
     nullPointerException();

   return *O;
}

void
   set(                             // Change associated Object
     Object*           object);     // Object* (May be NULL)
}; // class Ref<Object>

//----------------------------------------------------------------------------
//
// Class-
//       Ref<T>
//
// Purpose-
//       Typed reference to an Object.
//
// Usage notes-
//       Note that ALL Ref<T> objects are similar to autopointer objects.
//       When a Ref<T> goes out of scope, the object reference is removed.
//
//----------------------------------------------------------------------------
template<class T>
class Ref : public Ref<Object> {    // Typed reference
//----------------------------------------------------------------------------
// Ref::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
inline virtual
   ~Ref( void ) {}                  // Destructor

inline
   Ref( void )                      // Default constructor
:  Ref<Object>() {}

inline
   Ref(                             // Copy constructor
     const Ref<T>&     source)      // Source Ref<T>&
:  Ref<Object>(source.object) {}

inline
   Ref(                             // Constructor
     T&                source)      // Source T&
:  Ref<Object>(source) {}

inline
   Ref(                             // Constructor
     T*                source)      // Source T*
:  Ref<Object>(source) {}

inline Ref<T>&                      // (*this)
   operator=(                       // Ref<T>= Ref<T>&
     const Ref<T>&     source)      // Source Ref<T>&
{  set(source.object);
   return *this;
}

inline Ref<T>&                      // (*this)
   operator=(                       // Ref<T>= T&
     T&                source)      // Source T&
{  set(&source);
   return *this;
}

inline Ref<T>&                      // (*this)
   operator=(                       // Ref<T>= T*
     T*                source)      // Source T*
{  set(source);
   return *this;
}

// These override Ref<Object>::operator=, which require runtime checking
inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<T>= Ref<Object>
     const Ref<Object>&source)      // Source Ref<Object>
{  Object* o= source.get();
   if( o != NULL && dynamic_cast<T*>(o) == NULL ) // If not also a Ref<T>
     throw "RefAssignmentException";  // ERROR: Source content NOT a T*

   set(o);
   return *this;
}

inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<T>= Object&
     Object&           source)      // Source Object&
{  Object* o= &source;
   if( dynamic_cast<T*>(o) == NULL ) // Source NULL is NullPointerException
     throw "RefAssignmentException";  // ERROR: Source content NOT a T*

   set(o);
   return *this;
}

inline Ref<Object>&                 // (*this)
   operator=(                       // Ref<T>= Object*
     Object*           source)      // Source Object*
{  if( source != NULL && dynamic_cast<T*>(source) == NULL )
     throw "RefAssignmentException";  // ERROR: Source is NOT a T*

   set(source);
   return *this;
}

//----------------------------------------------------------------------------
// Ref<T>::Operators
//----------------------------------------------------------------------------
public:
inline T&                           // Associated T&
   operator*( void ) const          // *(Ref<T>)
{  return use(); }

inline T*                           // Associated T*
   operator->( void ) const         // (Ref<T>)->
{  return get(); }

//----------------------------------------------------------------------------
// Ref<T>::Methods
//----------------------------------------------------------------------------
public:
inline T*                           // T* (May be NULL)
   get( void ) const                // Get associated T*
{  return (T*)object; }

inline T&                           // T& (Exception if NULL)
   use( void ) const                // Get associated T&
{  return (T&)Ref<Object>::use(); }
}; // class Ref<T>

//----------------------------------------------------------------------------
//
// Class-
//       Cloneable
//
// Purpose-
//       Adds clone method to Object
//
// Implementation notes-
//       This is NOT an interface. Cloneable objects MUST BE derived from
//       the Cloneable object.
//
//----------------------------------------------------------------------------
class Cloneable : public Object {   // A Cloneable Object
//----------------------------------------------------------------------------
// Cloneable::Constructor
//----------------------------------------------------------------------------
public:
virtual inline
   ~Cloneable( void ) {}            // Destructor

inline
   Cloneable( void ) {}             // Default constructor

inline                              // Bitwise copy is allowed
   Cloneable(                       // Copy constructor
     const Cloneable&  source) {}   // Source Cloneable&

//----------------------------------------------------------------------------
// Cloneable::Methods
//----------------------------------------------------------------------------
public:
virtual Cloneable*                  // Duplicate Cloneable*
   clone( void ) const = 0;         // Duplicate this object
}; // class Cloneable

//----------------------------------------------------------------------------
//
// Class-
//       String
//
// Purpose-
//       Extend std::string as an Object
//
//----------------------------------------------------------------------------
class String : public Cloneable, public std::string { // The std::string Object
//----------------------------------------------------------------------------
// String::Constructors/Destructor/Assignment
//----------------------------------------------------------------------------
public:
virtual inline
   ~String( void ) {}               // Destructor

inline
   String( void )                   // Default constructor
:  Cloneable(), std::string() {}

inline
   String(                          // Copy constructor
     const String&     source)      // Source String
:  Cloneable(), std::string(source) {}

inline
   String(                          // std::string constructor
     std::string       source)      // Source std::string
:  Cloneable(), std::string(source) {}

   String(                          // Construct a String
     const char*       format,      // Format text
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(2,3);          // (printf argument validation)

   String(                          // Construct a String
     const char*       format,      // Format text
     va_list           argptr)      // The remaining arguments
   _ATTRIBUTE_PRINTF(2,0);          // (printf argument validation)

public:                             // Bitwise copy is allowed
inline String&                      // (Always *this)
   operator=(                       // Assignment operator
     const String&     source)      // Source String&
{  std::string::operator= (source);
   return *this;
}

inline String&                      // (Always *this)
   operator=(                       // Assignment operator
     const std::string&source)      // Source std::string&
{  std::string::operator= (source);
   return *this;
}

//----------------------------------------------------------------------------
// String::Methods
//----------------------------------------------------------------------------
public:
virtual Cloneable*                  // Duplicate String*
   clone( void ) const;             // Duplicate this object

virtual int                         // Result (<0,=0,>0)
   compare(                         // Compare to
     const Object&     source) const; // Source

virtual unsigned                    // A hash code value for this Object
   hashf( void ) const;             // Create hash code value for Object

virtual std::string                 // A String representation of this Object
   toString( void ) const;          // Represent this Object as a String
}; // class String

//----------------------------------------------------------------------------
//
// Description-
//       Global operators
//
// Purpose-
//       Global comparison operators, global cout << operator.
//
//----------------------------------------------------------------------------
inline int                          // Resultant
   operator==(                      // Compare (L::R) for equality
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) == 0); }

inline int                          // Resultant
   operator!=(                      // Compare (L::R) for inequality
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) != 0); }

inline int                          // Resultant
   operator<=(                      // Compare (L::R) for lesser || equality
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) <= 0);
}

inline int                          // Resultant
   operator>=(                      // Compare (L::R) for greater || equality
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) >= 0); }

inline int                          // Resultant
   operator<(                       // Compare (L::R) for lesser
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) < 0); }

inline int                          // Resultant
   operator>(                       // Compare (L::R) for greater
     Object&           L,           // Left parameter
     Object&           R)           // Right paramaeter
{  return (L.compare(R) > 0); }

inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const Object&     object)      // (This Object)
{  return stream << object.toString(); }

inline std::ostream&                // (stream)
   operator<<(                      // Append to output stream
     std::ostream&     stream,      // (This stream)
     const Ref<Object>&ref)         // (This Ref<Object>)
{  return stream << ref.use().toString(); }

#endif // OBJECT_H_INCLUDED
