//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       diag-shared_ptr.h
//
// Purpose-
//       std::shared_ptr debugging diagnostics.
//
// Last change date-
//       2023/12/05
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DIAG_SHARED_PTR_H_INCLUDED
#define _LIBPUB_DIAG_SHARED_PTR_H_INCLUDED

#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, ...
#include <string>                   // For std::string

#include "pub/bits/pubconfig.h"     // For _LIBPUB macros

namespace std { // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
namespace pub_diag {
//============================================================================
//
// Namespace-
//       std::pub_diag
//
// Purpose-
//       Namespace used for shared_ptr tracking.
//
// Implementation notes-
//       Since make_shared, shared_ptr, and weak_ptr are redefined when used,
//       they all must must reside somewhere in namespace std.
//       (We have to replace them no matter how they are used or declared.)
//       To avoid namespace collision, we redefine these names using both the
//       pub_diag:: qualifier and with different names. i.e.
//         #define make_shared pub_diag::make_debug
//         #define shared_ptr  pub_diag::debug_ptr
//         #define weak_ptr    pub_diag::dweak_ptr
//
//       This file must be included *AFTER* all system include files but
//       *BEFORE* any shared_ptr, make_shared, or weak_ptr declarations.
//
//       When enabled, the shared pointer debugging display is automatic
//       (See Diagnostic.cpp, GlobalDestructor.)
//       std::pub_diag::Debug_ptr::debug("caller-info"), defined below, can
//       also be invoked at any time.
//
// Sample usage-
//       Add a control file included by all file where tracking is desired:
//          #define USE_DEBUG_PTR   // (Swap lines to disable/enable)
//          #undef  USE_DEBUG_PTR   // (Swap lines to enable/disable)
//          #include "pub/bits/diag-shared_ptr.i"
//
//       In each constructor for objects containing shared_ptr objects, add:
//         INS_DEBUG_OBJ("name")    // ("name" is the name of the object)
//
//       In each destructor for objects containing shared_ptr objects, add:
//         REM_DEBUG_OBJ("name")    // ("name" is ignored)
//
//       Include the control file *AFTER* all system include files but *BEFORE*
//       *BEFORE* any shared_ptr, make_shared, weak_ptr declarations, or
//       constructors/destructors for objects containing trackable objects.
//
//       Note: the control file should be include as `#include "control-file"`
//       for dependency tracking.
//
//       The DEV library uses this diagnostic. File "dev/bits/devconfig.h"
//       is the control file. All DEV library include files include that file
//       either directly or indirectly.
//
// Implementation notes-
//       When USE_DEBUG_PTR is not defined in your control file, this file is
//       not included and the INS_DEBUG_OBJ and REM_DEBUG_OBJ macros do
//       nothing. There is *NO* runtime overhead if they're left in. However,
//       for production distributions, we recommend removing these macros (and
//       the associated control file) anyway. It leaves less code for the next
//       developer to look at.
//
// Implementation notes-
//       debug_ptr is a good but incomplete shared_ptr replacement. Only
//       functions needed or thought to be needed when debugging the pub/http
//       library were included. If addtional function are needed, just ask.
//
//============================================================================

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
template<class T> class debug_ptr;
template<class T> class dweak_ptr;

//----------------------------------------------------------------------------
//
// Class-
//       template<> (std::pub_diag::)debug_ptr<void>
//
// Purpose-
//       std::pub_diag::debug_ptr base class
//
// Implementation notes-
//       C_map: Container map <void*, string> // Container's address and name
//       R_map: Reference map <void*, void*>  // Reference's &ref, ref.get()
//
//----------------------------------------------------------------------------
template<> class debug_ptr<void> {  // The debug_ptr base class
//----------------------------------------------------------------------------
// debug_ptr<void>::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef shared_ptr<void>::element_type        element_type;

//----------------------------------------------------------------------------
// debug_ptr<void>::Constructors, destructor
//----------------------------------------------------------------------------
   debug_ptr( void );               // Default constructor
   ~debug_ptr( void );              // Destructor

//----------------------------------------------------------------------------
// debug_ptr<void>::Methods
//----------------------------------------------------------------------------
static void
   debug(const char* info="");      // Display everything

static void
   insert(                          // Add an object to the C_map
     const void*       that,        // The object's address
     std::string       name);       // The object's name

static void
   remove(                          // Remove an object from the C_map
     const void*       that);       // The object's address

static void
   update(                          // Update the reference map
     const void*       self,        // The object's address
     const void*       that);       // The referenced address
}; // class debug_ptr<void>

//----------------------------------------------------------------------------
//
// Class-
//       (std::pub_diag::)debug_ptr<>
//
// Purpose-
//       std::shared_ptr debugging extensions.
//
//----------------------------------------------------------------------------
template<class T>
class debug_ptr : public debug_ptr<void> { // A debugging shared pointer
//----------------------------------------------------------------------------
// debug_ptr::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef debug_ptr<void>   Debug_ptr; // (Also declared globally)

//----------------------------------------------------------------------------
// debug_ptr::Attributes
//----------------------------------------------------------------------------
protected:
shared_ptr<T>          ptr;         // The std::shared_ptr, not exposed

//----------------------------------------------------------------------------
// debug_ptr::Constructors, destructor
//----------------------------------------------------------------------------
public:
   debug_ptr( void ) = default;     // Default constructor

   debug_ptr(std::nullptr_t)        // Default constructor alias
:  debug_ptr<void>(), ptr() { }

template<class Y>
   explicit debug_ptr(Y* y)         // Copy constructor
:  debug_ptr<void>(), ptr(y)
{
   update(this, ptr.get());         // (Copy into accounting)
}

   debug_ptr(const debug_ptr& copy) // Copy constructor
:  debug_ptr<void>(), ptr(copy.ptr)
{
   update(this, ptr.get());         // (Copy into accounting)
}

   debug_ptr(const shared_ptr<T>& copy) // Copy constructor
:  debug_ptr<void>(), ptr(copy)
{
   update(this, ptr.get());         // (Copy into accounting)
}

template<class Y>
   debug_ptr(const debug_ptr<Y>& copy) // Copy constructor
:  debug_ptr<void>(), ptr((shared_ptr<Y>)copy)
{
   update(this, ptr.get());         // (Copy into accounting)
}

   debug_ptr(debug_ptr&& move)      // Move constructor
:  debug_ptr<void>(), ptr(move.ptr)
{
   update(this, ptr.get());         // (Move into accounting)
   update(&move, nullptr);          // (Move from accounting)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~debug_ptr( void )               // Destructor
{  reset(); }

//----------------------------------------------------------------------------
// debug_ptr::Accessor methods
//----------------------------------------------------------------------------
T* get( void ) const                // Get shared_ptr content
{  return ptr.get(); }

void reset( void ) noexcept         // Reset the shared_ptr
{  update(this, nullptr); ptr.reset(); }

size_t use_count( void ) const noexcept // Get the (approximate) use count
{  return ptr.use_count(); }

//----------------------------------------------------------------------------
// debug_ptr::Operators
//----------------------------------------------------------------------------
// Assignment operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
debug_ptr& operator=(const debug_ptr& copy) // Copy assignment
{  ptr= copy.ptr;
   update(this, ptr.get());         // (Move into accounting)
   return *this;
}

debug_ptr& operator=(const shared_ptr<T>& copy) // Copy assignment
{  ptr= copy;
   update(this, ptr.get());         // (Move into accounting)
   return *this;
}

debug_ptr& operator=(debug_ptr&& move) // Move assignment
{  ptr= move.ptr;
   update(this, ptr.get());         // (Move into accounting)
   update(&move, nullptr);          // (Move from accounting)
   return *this;
}

debug_ptr& operator=(std::nullptr_t) // Null assignment
{  reset(); return *this; }

// Casting operators - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   explicit operator shared_ptr<T>() const
{  return ptr; }

// Other operators - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
T& operator*( void ) const noexcept
{  return *ptr; }

T* operator->( void ) const noexcept
{  return ptr.operator->(); }

   explicit operator bool() const noexcept
{  return (bool)ptr; }

#if 0
// Global operators- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
template<class U>
debug_ptr<T>
   static_pointer_cast(const debug_ptr<U>& rhs) noexcept
{  return ptr.static_pointer_cast(rhs); }

template<class U>
debug_ptr<T>
   dynamic_pointer_cast(const debug_ptr<U>& rhs) noexcept
{  return ptr.dynamic_pointer_cast(rhs); }

template<class U>
debug_ptr<T>
   const_pointer_cast(const debug_ptr<U>& rhs) noexcept
{  return ptr.const_pointer_cast(rhs); }
#endif

//----------------------------------------------------------------------------
// debug_ptr::Static methods
//----------------------------------------------------------------------------
static inline void
   debug(const char* info="")       // Display everything
{  Debug_ptr::debug(info); }

static inline void
   insert(                          // Add an object to the container map
     void*             that,        // The object's address
     std::string       name)        // The object's name
{  Debug_ptr::insert(that, name); }

static inline void
   remove(                          // Remove an object from the container map
     void*             that)        // The object's address
{  Debug_ptr::remove(that); }

static inline void
   update(                          // Update the reference map
     void*             self,        // The referencer's address
     void*             that)        // The referenced address
{  Debug_ptr::update(self, that); }
}; // class debug_ptr<>

//----------------------------------------------------------------------------
// The static Debug_ptr type
//----------------------------------------------------------------------------
typedef debug_ptr<void>   Debug_ptr;

//----------------------------------------------------------------------------
//
// Class-
//       (std::pub_diag::)dweak_ptr<>
//
// Purpose-
//       std::shared_ptr debugging extensions.
//
//----------------------------------------------------------------------------
template<class T>
class dweak_ptr {                   // A debugging weak pointer
//----------------------------------------------------------------------------
// dweak_ptr::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef dweak_ptr<void>   Weak_ptr; // (Also declared globally)

//----------------------------------------------------------------------------
// dweak_ptr::Attributes
//----------------------------------------------------------------------------
protected:
weak_ptr<T>            ptr;         // The std::weak_ptr, not exposed

//----------------------------------------------------------------------------
// dweak_ptr::Constructors, destructor
//----------------------------------------------------------------------------
public:
   constexpr dweak_ptr( void ) = default; // Default constructor

   dweak_ptr(const dweak_ptr& copy) // Copy constructor
:  ptr(copy.ptr) {}

template<class Y>
   dweak_ptr(const debug_ptr<Y>& copy) // Copy constructor
:  ptr((shared_ptr<Y>)copy) {}

   dweak_ptr(dweak_ptr&& move)      // Move constructor
:  ptr(move.ptr) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~dweak_ptr( void ) = default;    // Destructor

//----------------------------------------------------------------------------
// dweak_ptr::Operators
//----------------------------------------------------------------------------
// Assignment operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
dweak_ptr& operator=(const dweak_ptr& copy) // Copy assignment
{  ptr= copy;
   return *this;
}

dweak_ptr& operator=(dweak_ptr&& move) // Move assignment
{  ptr= move;
   return *this;
}

dweak_ptr& operator=(std::nullptr_t) // Null assignment
{  ptr= nullptr;
   return *this;
}

template<class Y>
dweak_ptr& operator=(debug_ptr<Y>& from) // Downgrade assignment
{  ptr= (shared_ptr<Y>)from;
   return *this;
}

// Casting operators - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   explicit operator weak_ptr<T>() const
{  return ptr; }

//----------------------------------------------------------------------------
// dweak_ptr::Methods
//----------------------------------------------------------------------------
debug_ptr<T>
   lock( void ) const noexcept
{
   debug_ptr<T> ret= ptr.lock();
   return ret;
}

size_t use_count( void ) const noexcept // Get the (approximate) use count
{  return ptr.use_count(); }
}; // class dweak_ptr<>

//----------------------------------------------------------------------------
// The static Weak_ptr type
//----------------------------------------------------------------------------
typedef dweak_ptr<void>   Weak_ptr;

//----------------------------------------------------------------------------
//
// Subroutine-
//       std::pub_diag::make_debug
//
// Purpose-
//       Replacement for std::make_shared for debug_ptr objects
//
// Implementation notes-
//       Only the C++11 version is (partially) supported, and std::shared_ptr
//       storage optimization does not occur.
//
//----------------------------------------------------------------------------
template<class T, class... Args>
   inline debug_ptr<T>
     make_debug(Args&&... args)
   {
     T* P= new T(args...);
     debug_ptr<T> Q(P);
     return Q;
   }
}  // namespace pub_diag
}  // namespace std

//----------------------------------------------------------------------------
//
// Namespace-
//       (global)
//
// Purpose-
//       Define (some) global operators
//
// Implemenation notes-
//       TODO: Define (more) global operators
//
//----------------------------------------------------------------------------
#define __PUB std::pub_diag

// Casting operators - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
namespace std {
// template<T,U>
//   std::pub_diag::debug_ptr<T>
//     std::pub_diag::dynamic_pointer_cast(const std::pub_diag::debug_ptr<U)&)
template<class T, class U>
__PUB::debug_ptr<T>
   dynamic_pointer_cast(const __PUB::debug_ptr<U>& r) noexcept
{
   std::shared_ptr<U> ptr= (std::shared_ptr<U>)r;
   auto p= dynamic_cast<typename std::shared_ptr<T>::element_type*>(ptr.get());
   if( p ) {
     std::shared_ptr<T> out{ptr, p};
     return out;                    // (Uses copy constructor)
   }

   return __PUB::debug_ptr<T>();
}  // dynamic_pointer_cast
}  // namespace std

// Comparison operators- - - - - - - - - - - - - - - - - - - - - - - - - - - -
template<class T, class U>
bool operator==(const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() == rhs.get(); }

template<class T, class U>
bool operator!=(const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() != rhs.get(); }

template<class T, class U>
bool operator<=(const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() <= rhs.get(); }

template<class T, class U>
bool operator>=(const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() >= rhs.get(); }

template<class T, class U>
bool operator< (const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() <  rhs.get(); }

template<class T, class U>
bool operator> (const __PUB::debug_ptr<T>& lhs,
                const __PUB::debug_ptr<U>& rhs) noexcept
{  return lhs.get() >  rhs.get(); }

#undef __PUB
#endif // _LIBPUB_DIAG_SHARED_PTR_H_INCLUDED
