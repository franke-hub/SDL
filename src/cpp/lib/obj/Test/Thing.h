//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Thing.h
//
// Purpose-
//       Self-checking Object (with link)
//
// Last change date-
//       2021/08/06
//
//----------------------------------------------------------------------------
#ifndef THING_H_INCLUDED
#define THING_H_INCLUDED

#include <cstddef>                   // For size_t
#include <stdint.h>                  // For uint32_t

//----------------------------------------------------------------------------
// Compile-time controls
//----------------------------------------------------------------------------
#define USE_THING_OBJ               // Use Object Thing
#undef  USE_THING_OBJ               // Use basic Thing

//----------------------------------------------------------------------------
// Typed dependencies
//----------------------------------------------------------------------------
#ifdef USE_THING_OBJ                 // If using Object Thing
  #include <obj/Object.h>
  #define MAKE_THING(ARGS) new Thing(ARGS)
  #define THING_BASE Thing_base, public obj::Object
  #define THING_PTR  obj::Ref_t<Thing>
#else                                // If using basic Thing
  #include <memory>
  class Thing;                       // (Forward reference)
  #define MAKE_THING(ARGS) std::make_shared<Thing>(ARGS)
  #define THING_BASE Thing_base
  #define THING_PTR  std::shared_ptr<Thing>
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Thing_base
//
// Purpose-
//       Self-checking Thing
//
//----------------------------------------------------------------------------
class Thing;                            // Forward reference
class Thing_base {                      // Self-checking Thing
//----------------------------------------------------------------------------
// Thing_base::Enumerations and Typedefs
//----------------------------------------------------------------------------
public:
enum // Generic enum
{  PrefixValidator= 0xfedcba9876543210L // Prefix validation word
,  SuffixValidator= 0x0123456789abcdefL // Suffix validation word
}; // enum

//----------------------------------------------------------------------------
// Thing_base::Attributes
//----------------------------------------------------------------------------
public:
static int             errorCount;  // Error counter
uint32_t               word[2];     // User words

protected:
size_t                 prefix;      // Validation prefix
intptr_t               posAddr;     // The address of this object
intptr_t               negAddr;     // The inverse address of this object
size_t                 checkword;   // Validation word
size_t                 suffix;      // Validation suffix

//----------------------------------------------------------------------------
// Thing_base::Operator new/delete
//----------------------------------------------------------------------------
public:
static size_t
   get_allocated( void );           // Return allocated object count

static inline void*                 // Resultant Thing*
   operator new(std::size_t size)   // Replacement operator new
{  return allocate(size); }

static inline void
   operator delete(void* addr)      // Replacement operator delete
{  deallocate(addr, size_t(-1)); }
static inline void
   operator delete(void* addr, size_t size) // Replacement operator delete
{  deallocate(addr, size); }

static void
   deallocate_all( void );          // Delete all internal storage

//----------------------------------------------------------------------------
// Thing_base::Constructor/Destructor/operator=
//----------------------------------------------------------------------------
public:
virtual
   ~Thing_base( void );             // Destructor

   Thing_base(                      // Constructor
     size_t            checkword= 0); // Check word

Thing_base&                         // Resultant (*this)
   operator=(                       // Assignment operator
     const Thing_base& source);     // Source Thing

//----------------------------------------------------------------------------
// Thing_base::Object methods
//----------------------------------------------------------------------------
virtual const std::string           // A String representation of this Object
   string( void ) const;            // Represent this Object as a String

//----------------------------------------------------------------------------
// Thing_base::Allocator
//----------------------------------------------------------------------------
public:
static void*                        // Resultant Thing*
   allocate(std::size_t);           // Allocate a Thing

static void
   deallocate(void*, std::size_t);  // Deallocate a Thing
static inline void
   deallocate(void* addr)           // Deallocate a Thing
{  deallocate(addr, std::size_t(-1)); }

//----------------------------------------------------------------------------
// Thing_base::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Error count
   check(                           // Count errors
     int               lineno,      // Caller's line number
     size_t            checkword= 0) const; // Check word

int                                 // Error count
   check( void ) const              // Count errors, no line number
{  return check(-1); }

static void
   debug_static( void );            // Debug allocator data
}; // class Thing_base

//----------------------------------------------------------------------------
//
// Class-
//       Thing
//
// Purpose-
//       Self-checking Object Thing
//
//----------------------------------------------------------------------------
class Thing : public THING_BASE {   // Self-checking Object Thing
public:
THING_PTR              link;        // Chain pointer

//----------------------------------------------------------------------------
// Thing::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Thing( void ) {}                // Destructor

   Thing(                           // Constructor
     size_t            checkword=0) // Check word
:  Thing_base(checkword), link() {}
}; // class Thing
#endif // THING_H_INCLUDED
