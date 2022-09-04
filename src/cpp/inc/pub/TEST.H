//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
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
//       2022/09/02
//
// Implementation note-
//       Uses the default namespace, not `pub`
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TEST_H_INCLUDED
#define _LIBPUB_TEST_H_INCLUDED

#include <thread>                   // For std::thread::id

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/utility.h>            // For pub::utility::to_string

#define _PUB _LIBPUB_NAMESPACE      // (Undefined at end)

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
     _PUB::debugging::debugf("%4d: Error: VERIFY(%s)\n", line, expr);
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
   _PUB::debugging::debugf("%4d: Error: MUST_NOT(%s)\n", line, text);
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
     _PUB::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%ld) want(%ld)\n"
                            , line, expr, have, want);
   }

   return result;
}

static inline int                   // Error count
   ne_error(                        // Assert equal std::thread::id
     int               line,        // Calling line number
     const char*       expr,        // Expression
     const std::thread::id&
                       have,        // Actual value
     const std::thread::id&
                       want)        // Desired value
{
   int result= 0;

   if( want != have ) {
     result= 1;
     _PUB::debugging::debugf("%4d: Error: MUST_EQ(%s) have(%s) want(%s)\n"
         , line, expr, _PUB::utility::to_string(have).c_str()
         , _PUB::utility::to_string(want).c_str());
   }

   return result;
}
#undef _PUB                         // (Temporary)
#endif // _LIBPUB_TEST_H_INCLUDED
