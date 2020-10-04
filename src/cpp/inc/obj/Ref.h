//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Ref.h
//
// Purpose-
//       Object Reference Object.
//
// Last change date-
//       2020/10/03
//
// Implementation notes-
//       The implementation may perform garbage collection either in the
//       foreground or in a background thread. If an application uses many
//       Object references there can be a noticable operational delay when
//       a memory-intensive task completes.
//
//       Method gc() does not return until all garbage collection completes.
//       If any other threads are actively creating, referencing, and
//       de-referencing Objects, garbage collection may NEVER complete.
//
//----------------------------------------------------------------------------
#ifndef OBJ_REF_H_INCLUDED
#define OBJ_REF_H_INCLUDED

#include "Object.h"                 // Ref instances refer to Objects
#include "Exception.h"              // Exceptions can be thrown

namespace _OBJ_NAMESPACE {
namespace config {
//----------------------------------------------------------------------------
// Configuration controls
//----------------------------------------------------------------------------
enum Ref                            // Controls
{  USE_OBJECT_COUNT= true           // true iff object_count is valid
}; // enum
}  // namespace config

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class Ref_t;      // Typed reference
typedef Ref_t<Object>  Ref;         // Generic Ref

//----------------------------------------------------------------------------
//
// Class-
//       Ref_t<Object>, a.k.a Ref
//
// Purpose-
//       Refer to an Object and manage it's reference count.
//
//----------------------------------------------------------------------------
template <> class Ref_t<Object> {   // Object reference
//----------------------------------------------------------------------------
// Ref::Attributes
//----------------------------------------------------------------------------
protected:
static std::atomic<size_t>
                       object_count; // The number of referenced Objects
std::atomic<Object*>   object;      // The associated Object

//----------------------------------------------------------------------------
// Methods intended for bringup debugging.
//     count_object:         Increment/decrement object_count (protected)
//     debug_static:         Statistical counter display
//     get_object_count:     Extract (instantaneous) object_count
//----------------------------------------------------------------------------
protected:
static inline void
   count_object(                    // Update object_count
     int               count)       // Increment/decrement value
{
   if( config::Ref::USE_OBJECT_COUNT ) // (Compiled out if false)
     object_count += count;         // Update object_count
}

public:
static void                         // Bringup debugging display
   debug_static( void );            // Bringup debugging display

static inline size_t                // The (instantaneous) object count
   get_object_count()               // Extract object_count
{  return object_count.load(); }

//----------------------------------------------------------------------------
// Ref::Destructor/Constructors/Assignment
//----------------------------------------------------------------------------
public:
inline
   ~Ref_t( void )                   // Destructor
{  set(nullptr); }

inline
   Ref_t( void )                    // Default constructor
:  object(nullptr) { }

inline
   Ref_t(                           // Copy constructor
     const Ref_t&      source)      // Source Ref_t&
:  object(nullptr)
{  set(source.object); }

inline
   Ref_t(                           // Constructor
     Object&           source)      // Source Object&
:  object(nullptr)
{  set(&source); }

inline
   Ref_t(                           // Constructor
     Object*           source)      // Source Object*
:  object(nullptr)
{  set(source); }

inline Ref_t&                       // (*this)
   operator=(                       // Ref_t= Ref_t&
     const Ref_t&      source)      // Source Ref_t&
{  set(source.object);
   return *this;
}

inline Ref_t&                       // (*this)
   operator=(                       // Ref_t= Object&
     Object&           source)      // Source Object&
{  set(&source);
   return *this;
}

inline Ref_t&                       // (*this)
   operator=(                       // Ref_t= Object*
     Object*           source)      // Source Object*
{  set(source);
   return *this;
}

//----------------------------------------------------------------------------
// Ref::Operators (Equality operators compare ADDRESSES, not Objects.)
//----------------------------------------------------------------------------
public:
inline bool                         // true iff get() == source.get()
   operator==(                      // Compare associated (Object*)s
     const Ref_t&      source) const // Source Ref_t&
{  return ((void*)object == (void*)source.object); }

inline bool                         // true iff get() == source.get()
   operator==(                      // Compare associated (Object*)s
     nullptr_t         empty) const // Source nullptr
{  (void)empty; return (object == nullptr); } // (Unused nullptr parameter)

inline bool                         // true iff get() != source.get()
   operator!=(                      // Compare associated (Object*)s
     const Ref_t&      source) const // Source Ref_t&
{  return ((void*)object != (void*)source.object); }

inline bool                         // true iff get() == source.get()
   operator!=(                      // Compare associated (Object*)s
     nullptr_t         empty) const // Source nullptr
{  (void)empty; return (object != nullptr); } // (Unused nullptr parameter)

inline Object&                      // Associated Object&
   operator*( void ) const          // *(Ref_t)
{  return use(); }

inline Object*                      // Associated Object*
   operator->( void ) const         // (Ref_t)->
{  Object* O= object;
   if( O == nullptr )
     throw NullPointerException("Ref");

   return O;
}

//----------------------------------------------------------------------------
// Ref::Methods
//----------------------------------------------------------------------------
public:
// Run the garbage collector.
// (This is never useful when used outside of the Ref implementation.)
static void
   collect( void );                 // Run garbage collector

// Complete any pending garbage collection. (Normally not required.)
// This might be useful between memory-intensive computations if the Ref
// implementation performs background collection.
static bool                         // TRUE iff garbage collected
   gc( void );                      // Wait for garbage collection completion

inline Object*                      // Object*
   get( void ) const                // Get associated Object*
{  return object; }

void
   set(                             // Change associated Object
     Object*           object);     // Object* (May be nullptr)

inline Object&                      // Object&
   use( void ) const                // Get associated Object reference
{  Object* O= object;
   if( O == nullptr )
     throw NullPointerException("Ref");

   return *O;
}
}; // class Ref_t<Object>

//----------------------------------------------------------------------------
//
// Class-
//       Ref_t<T>
//
// Purpose-
//       Typed reference to an Object.
//
// Implementation notes-
//       Deleted methods prevent inadvertent downgrading of the associated
//       (class T*)object to an (Object*)object.
//
// Implementation notes-
//       Ref_t::operator=(nullptr_t) and Ref_t::set(nullptr_t) are required
//       in order to use void Ref_t::set(Object*) = delete;
//
//----------------------------------------------------------------------------
template<class T>
class Ref_t : public Ref {          // Typed reference
//----------------------------------------------------------------------------
// Ref_t::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
inline virtual
   ~Ref_t( void ) {}                // Destructor

inline
   Ref_t( void )                    // Default constructor
:  Ref() {}

inline
   Ref_t(                           // Copy constructor
     const Ref_t<T>&   source)      // Source Ref_t<T>&
:  Ref(source.object) {}

inline
   Ref_t(                           // Constructor
     T&                source)      // Source T&
:  Ref(source) {}

inline
   Ref_t(                           // Constructor
     T*                source)      // Source T*
:  Ref(source) {}

inline Ref_t<T>&                    // (*this)
   operator=(                       // Ref_t<T>= Ref_t<T>&
     const Ref_t<T>&   source)      // Source Ref_t<T>&
{  Ref::set(source.object);
   return *this;
}

inline Ref_t<T>&                    // (*this)
   operator=(                       // Ref_t<T>= T&
     T&                source)      // Source T&
{  Ref::set(&source);
   return *this;
}

inline Ref_t<T>&                    // (*this)
   operator=(                       // Ref_t<T>= T*
     T*                source)      // Source T*
{  Ref::set(source);
   return *this;
}

inline Ref_t<T>&                    // (*this)
   operator=(                       // Ref_t<T>= T*
     std::nullptr_t    source)      // Source std::nullptr_t
{  Ref::set(source);
   return *this;
}

// Deleted constructors and operators (prevent downgrade of Ref_t to Ref)
explicit Ref_t(const Ref&) = delete; // Disallow Ref_t::Ref_t(const Ref&)
Ref_t& operator=(const Ref&) = delete; // Disallow Ref_t::operator=(Ref&)
Ref_t& operator=(const Object&) = delete; // Disallow Ref_t::operator=(Object&)
Ref_t& operator=(const Object*) = delete; // Disallow Ref_t::operator=(Object*)
void set(Object*) = delete;         // Disallow Ref_t::set(Object*)

//----------------------------------------------------------------------------
// Ref_t<T>::Operators
//----------------------------------------------------------------------------
public:
inline T&                           // Associated T&
   operator*( void ) const          // *(Ref<T>)
{  return (T&)Ref::operator*(); }

inline T*                           // Associated T*
   operator->( void ) const         // (Ref<T>)->
{  return (T*)Ref::operator->(); }

//----------------------------------------------------------------------------
// Ref_t<T>::Methods
//----------------------------------------------------------------------------
public:
inline T*                           // T*
   get( void ) const                // Get associated T*
{  return (T*)Ref::get(); }

inline void
   set(                             // Change associated T*
     T*                object)      // T* (May be nullptr)
{  Ref::set(object); }

inline void
   set(                             // Change associated T*
     std::nullptr_t    object)      // To nullptr
{  Ref::set(nullptr); }

inline T&                           // T&
   use( void ) const                // Get associated T&
{  return (T&)Ref::use(); }
}; // class Ref_t<T>
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_REF_H_INCLUDED
