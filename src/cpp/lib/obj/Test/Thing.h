//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
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
//       2018/01/01
//
// Implementation note-
//       Main module must declare and initialize errorCount:
//       int Thing::errorCount= 0;
//
//----------------------------------------------------------------------------
#ifndef THING_H_INCLUDED
#define THING_H_INCLUDED

#include "com/Debug.h"

#include "obj/Object.h"
#include "obj/ifmacro.h"

//----------------------------------------------------------------------------
//
// Class-
//       Thing
//
// Purpose-
//       Self-checking Object
//
//----------------------------------------------------------------------------
class Thing : public Object {       // Self-checking Object
//----------------------------------------------------------------------------
// Thing::Enumerations and Typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{
  PrefixValidator=      0x76543210L,// Prefix validation word
  SuffixValidator=      0x89abcdefL // Suffix validation word
}; // enum

//----------------------------------------------------------------------------
// Thing::Attributes
//----------------------------------------------------------------------------
public:
static int             errorCount;  // Error counter

protected:
   long                prefix;      // Validation prefix

public:
   Ref                 link;        // Chain pointer
   long                word[2];     // User words

protected:
   intptr_t            posAddr;     // The address of this object
   intptr_t            negAddr;     // The inverse address of this object
   size_t              checkword;   // Validation word
   long                suffix;      // Validation suffix

//----------------------------------------------------------------------------
// Thing::Operator new/delete
//----------------------------------------------------------------------------
public:
static size_t
   get_allocated( void );           // Return allocated object count

static void*                        // Resultant Thing*
   operator new(std::size_t);       // Replacement operator new

static void
   operator delete(void*);          // Replacement operator delete

static void
   operator delete(void*, std::size_t); // Replacement operator delete

// Unexpected new/delete operators
static void*                        // Resultant []Thing*
   operator new[](std::size_t size) // Replacement operator new[]
{  debugf("Thing::operator new[](%zd)\n", size);
   return ::operator new[](size);
}

static void
   operator delete[](void* addr, std::size_t size) // Replacement operator delete[]
{  debugf("Thing::operator delete[](%p,%zd)\n", addr, size);
   ::operator delete[](addr, size);
}

//----------------------------------------------------------------------------
// Thing::Constructor/Destructor/operator=
//----------------------------------------------------------------------------
public:
virtual
   ~Thing( void )                   // Destructor
{
   IFHCDM(
     printf("%4d: Thing(%p)::~Thing() %zd\n", __LINE__, this, checkword);
   )

   check(__LINE__);
}

   Thing(                           // Default constructor
     long              checkword=0) // Check word
:  Object()
,  prefix(PrefixValidator)
,  link()
,  checkword(checkword)
,  suffix(SuffixValidator)
{
   IFHCDM(
     printf("%4d: Thing(%p)::Thing(%zd)\n", __LINE__, this, checkword);
   )

   posAddr= (intptr_t)this;
   negAddr= ~posAddr;

   word[0]= word[1]= 0;
}

Thing&                              // Resultant (*this)
   operator=(                       // Assignment operator
     const Thing&      source);     // Source Thing

//----------------------------------------------------------------------------
// Thing::Object methods
//----------------------------------------------------------------------------
virtual const std::string           // A String representation of this Object
   string( void ) const             // Represent this Object as a String
{
   return ::obj::built_in::to_string("Thing(%p)::string %zd", this, checkword);
}

//----------------------------------------------------------------------------
// Thing::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Error count
   check(                           // Count errors
     int               lineno,      // Caller's line number
     size_t            checkword=0) const // Check word
{
   IFHCDM(
     if( false )
       printf("%4d: Thing(%p)::check(%zd)\n", lineno, this, checkword);
   )

   if( prefix != PrefixValidator )
   {
     errorCount++;
     errorf("%4d: Thing(%p).check() prefix(%.8lx)\n", lineno, this, prefix);
   }

   if( (intptr_t)this != posAddr )
   {
     errorCount++;
     errorf("%4d: Thing(%p).check() posAddr(%.8zx)\n", lineno, this, (size_t)posAddr);
   }

   if( ~((intptr_t)this) != negAddr )
   {
     errorCount++;
     errorf("%4d: Thing(%p).check() negAddr(%.8zx)\n", lineno, this, (size_t)negAddr);
   }

   if( suffix != SuffixValidator )
   {
     errorCount++;
     errorf("%4d: Thing(%p).check() suffix(%.8lx)\n", lineno, this, suffix);
   }

   if( checkword != this->checkword && checkword != 0 )
   {
     errorCount++;
     errorf("%4d: Thing(%p).check(%.8zx) checkword(%.8zx)\n", lineno, this,
             checkword, this->checkword);
   }

   return errorCount;
}

virtual int                         // Error count
   check( void ) const              // Count errors, no line number
{
   return check(-1);
}

static void
   debug_static( void );            // Debug allocator data

virtual void
   debug( void ) const              // Debug this Object
{
   debugf("%4d: Thing(%p)::debug()\n", __LINE__, this);
   check(__LINE__);
}
}; // class Thing

#endif // THING_H_INCLUDED
