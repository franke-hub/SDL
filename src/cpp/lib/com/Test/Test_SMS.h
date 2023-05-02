//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_SMS.h
//
// Purpose-
//       Storage management subsystem test object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TEST_SMS_H_INCLUDED
#define TEST_SMS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Test_SMS
//
// Purpose-
//       Storage Management Subsystem test object.
//
//----------------------------------------------------------------------------
class Test_SMS {                    // Test_SMS
//----------------------------------------------------------------------------
// Test_SMS::Constructors
//----------------------------------------------------------------------------
public:
   Test_SMS();                      // Constructor
   ~Test_SMS();                     // Destructor

private:                            // Bitwise copy is prohibited
   Test_SMS(const Test_SMS&);       // Disallowed copy constructor
   Test_SMS& operator=(const Test_SMS&); // Disallowed assignment operator

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::allocate
//
// Purpose-
//       Allocate storage (without subpool)
//
//----------------------------------------------------------------------------
public:
void*                               // -> Allocated storage
   allocate(                        // Allocate storage
     unsigned long   size);         // Required length

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::allocate
//
// Purpose-
//       Allocate storage (with subpool)
//
//----------------------------------------------------------------------------
public:
void*                               // -> Allocated storage
   allocate(                        // Allocate storage
     unsigned long   size,          // Required length
     unsigned        subpool);      // Required subpool

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::debug
//
// Purpose-
//       Debug storage (at end of test)
//
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debug storage

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::release
//
// Purpose-
//       Release storage (without subpool)
//
//----------------------------------------------------------------------------
public:
void
   release(                         // Release storage
     void*           addr,          // Release address
     unsigned long   size);         // Release length

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::release
//
// Purpose-
//       Release storage (with subpool)
//
//----------------------------------------------------------------------------
public:
void
   release(                         // Release storage
     void*           addr,          // Release address
     unsigned long   size,          // Release length
     unsigned        subpool);      // Release subpool

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::release
//
// Purpose-
//       Release storage subpool
//
//----------------------------------------------------------------------------
public:
void
   release(                         // Release storage subpool
     unsigned        subpool);      // Release subpool

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getUsed
//
// Purpose-
//       Determine how many bytes are allocated.
//
// Notes-
//       If unknown, return a constant.
//
//----------------------------------------------------------------------------
public:
unsigned long                       // The available allocation size
   getUsed( void ) const;           // Get available allocation size

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getMaxSize
//
// Purpose-
//       Determine the largest allocation size.
//
//----------------------------------------------------------------------------
public:
unsigned long                       // The maximum allocation size
   getMaxSize( void ) const;        // Get maximum allocation size

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getMinSize
//
// Purpose-
//       Determine the smallest allocation size.
//
//----------------------------------------------------------------------------
public:
unsigned long                       // The minimum allocation size
   getMinSize( void ) const;        // Get minimum allocation size

//----------------------------------------------------------------------------
//
// Subroutine-
//       Test_SMS::getSubpools
//
// Purpose-
//       Determine how many subpools (if any) are supported.
//
// Notes-
//       Returns 0 if subpool release is not supported.
//
//----------------------------------------------------------------------------
public:
long                                // The number of supported subpools
   getSubpools( void ) const;       // Get number of supported subpools

//----------------------------------------------------------------------------
// Test_SMS::Attributes
//----------------------------------------------------------------------------
private:
   void*             imp;           // Object implementation
}; // class Test_SMS

#endif // TEST_SMS_H_INCLUDED
