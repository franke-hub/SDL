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
//       TEST.H
//
// Purpose-
//       Testcase error checking.
//
// Last change date-
//       2021/11/19
//
// Implementation note-
//       Contained subroutines are in the default namespace, not pub::
//
//----------------------------------------------------------------------------
#ifndef TEST_H_INCLUDED
#define TEST_H_INCLUDED

// #include <sstream>                  // Used by id_string
#include <pub/Debug.h>                 // For namespace pub::Debug::debugging
#include <pub/utility.h>               // For pub::utility::to_string

//----------------------------------------------------------------------------
//
// Subroutine-
//       id_string
//
// Purpose-
//       Get id string of std::thread::id
//
//----------------------------------------------------------------------------
// using _PUB_NAMESPACE::utility::to_string;

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

   if( !valid ) {
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

   if( want != have ) {
     result= 1;
     pub::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%ld) want(%ld)\n"
                           , line, expr, have, want);
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

   if( want != have ) {
     result= 1;
     pub::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%s) want(%s)\n"
         , line, expr, pub::utility::to_string(have).c_str()
         , pub::utility::to_string(want).c_str());
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
{  return ne_error(line, expr, const_cast<std::thread::id&>(have), want); }
#endif // TEST_H_INCLUDED
