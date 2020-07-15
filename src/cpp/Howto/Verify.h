//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Verify.h
//
// Purpose-
//       Define the Things used in testing
//
// Last change date-
//       2020/07/15
//
// Implementation notes-
//       Test source must include Verify.i
//
//----------------------------------------------------------------------------
#ifndef _VERIFY_H_INCLUDED
#define _VERIFY_H_INCLUDED

#include <sstream>                  // Used by id_string
#include <pub/Debug.h>
#include <pub/ifmacro.h>
#include <pub/utility.h>

using _PUB_NAMESPACE::utility::to_string;

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define MUST_EQ(actual, expect) \
        ne_error(__LINE__, #actual, actual, expect)

#define MUST_NOT(expression) \
        is_error(__LINE__, #expression)

#define VERIFY(expression) \
        if_error(__LINE__, #expression, expression)

//----------------------------------------------------------------------------
//
// Subroutine-
//       if_error
//
// Purpose-
//       Verify expression.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   if_error(                        // Assert false
     int               line,        // Calling line number
     const char*       expr,        // Expression
     int               valid)       // Expected true
{
   int result= 0;

   if( !valid )
   {
     result= 1;
     pub::debugging::debugf("%4d: Error: VERIFY(%s)\n", line, expr);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_error
//
// Purpose-
//       Count error.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   is_error(                        // Assert false
     int               line,        // Calling line number
     const char*       text)        // Expression
{
   pub::debugging::debugf("%4d: Error: MUST_NOT(%s)\n", line, text);
   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ne_error
//
// Purpose-
//       Verify expected value obtained
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   ne_error(                        // Assert equal
     int               line,        // Calling line number
     const char*       expr,        // Expression
     long              have,        // Actual value
     long              want)        // Desired value
{
   int result= 0;

   if( want != have )
   {
     result= 1;
     pub::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%ld) want(%ld)\n",
                            line, expr, have, want);
   }

   return result;
}

static inline int                   // Error count
   ne_error(                        // Assert equal std::thread::id
     int               line,        // Calling line number
     const char*       expr,        // Expression
     std::thread::id&  have,        // Actual value
     std::thread::id&  want)        // Desired value
{
   int result= 0;

   if( want != have )
   {
     result= 1;
     pub::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%s) want(%s)\n",
         line, expr, to_string(have).c_str(), to_string(want).c_str());
   }

   return result;
}

static inline int                   // Error count
   ne_error(                        // Assert equal std::thread::id
     int               line,        // Calling line number
     const char*       expr,        // Expression
     volatile std::thread::id&
                       have,        // Actual value
     std::thread::id&  want)        // Desired value
{  return ne_error(line, expr, const_cast<std::thread::id&>(have), want);
}

//----------------------------------------------------------------------------
//
// Class-
//       Thing1
//
// Purpose-
//       A class which keeps track of the number of instantiations.
//
//----------------------------------------------------------------------------
class Thing1 {                      // The Thread Object
//----------------------------------------------------------------------------
// Thing1::Attributes
//----------------------------------------------------------------------------
public:
enum                                // Counter indexes
{  IX_OBJS                          // Object count
,  IX_NEWS                          // Constructor count
,  IX_OLDS                          // Destructor count
,  IX_UNUSED                        // Unused
};
static unsigned        counter[4];  // Objects, Constructs, Destructs, unused

//----------------------------------------------------------------------------
// Thing1::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   ~Thing1( void )
{  if( HCDM ) { printf("%4d Thing1(%p)::~Thing1\n", __LINE__, this); }
   counter[IX_OLDS]++;
   counter[IX_OBJS]--;
}

   Thing1( void )
{  if( HCDM ) { printf("%4d Thing1(%p)::Thing1\n", __LINE__, this); }
   counter[IX_NEWS]++;
   counter[IX_OBJS]++;
}
}; // class Thing1

//----------------------------------------------------------------------------
//
// Class-
//       Thing2
//
// Purpose-
//       Like Thing1, but with a virtual destructor.
//
//----------------------------------------------------------------------------
class Thing2 {                      // The Thread Object
//----------------------------------------------------------------------------
// Thing2::Attributes
//----------------------------------------------------------------------------
public:
enum                                // Counter indexes
{  IX_OBJS                          // Object count
,  IX_NEWS                          // Constructor count
,  IX_OLDS                          // Destructor count
,  IX_UNUSED                        // Unused
};
static unsigned        counter[4];  // Objects, Constructs, Destructs, unused

//----------------------------------------------------------------------------
// Thing2::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thing2( void )
{  if( HCDM ) { printf("%4d Thing2(%p)::~Thing2\n", __LINE__, this); }
   counter[IX_OLDS]++;
   counter[IX_OBJS]--;
}

   Thing2( void )
{  if( HCDM ) { printf("%4d Thing2(%p)::Thing2\n", __LINE__, this); }
   counter[IX_NEWS]++;
   counter[IX_OBJS]++;
}
}; // class Thing2

//----------------------------------------------------------------------------
//
// Class-
//       Things
//
// Purpose-
//       A Thing1 and Thing2 container.
//
// Implementation note-
//       As configured, used to verify that Thing1 and Thing2 constructor and
//       destructor are properly called.
//
//----------------------------------------------------------------------------
class Things {                      // A Thing constructor verifier object
//----------------------------------------------------------------------------
// Things::Attributes
//----------------------------------------------------------------------------
public:
Thing1                 thing1;
Thing2                 thing2;

//----------------------------------------------------------------------------
// Things::Constructors/Destructors
//----------------------------------------------------------------------------
public:
#if 0 // Omit wrapping destructor
   ~Things( void )
{  if( HCDM ) { printf("%4d Things(%p)::~Things\n", __LINE__, this); }
}
#endif

#if 0 // Omit wrapping constructor
   Things( void )
{  if( HCDM ) { printf("%4d Things(%p)::Things\n", __LINE__, this); }
}
#endif
}; // class Things
#endif // _VERIFY_H_INCLUDED
