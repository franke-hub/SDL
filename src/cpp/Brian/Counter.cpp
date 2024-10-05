//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Counter.cpp
//
// Purpose-
//       Debugging object reference Counter.
//
// Last change date-
//       2024/10/01
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging

#include "Counter.h"                // For Counter, implemented

using pub::Debug;
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Counter::counter_t     Counter::c_count= 0; //  Constructor count
Counter::counter_t     Counter::d_count= 0; //  Destructor count

//----------------------------------------------------------------------------
// Subroutine plural: returns "" if argument is 1, otherwise "s"
//----------------------------------------------------------------------------
static const char*                  // "" or "s"
   plural(size_t arg)
{  return arg == 1 ? "" : "s"; }

//----------------------------------------------------------------------------
// Static_global: Static initialization/termination
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct Static_global {
   Static_global( void )
{  if( HCDM ) debugf("Counter::Static_global!\n"); }

   ~Static_global( void )
{  if( HCDM ) debugf("Counter::Static_global~\n");

   if( Counter::c_count != Counter::d_count ) {
     debugf("Counter constructors != destructors\n");
     debugf("%8zd constructor%s\n", Counter::c_count.load()
           , plural(Counter::c_count.load()));
     debugf("%8zd destructor%s\n",  Counter::d_count.load()
           , plural(Counter::d_count.load()));
   } else if( VERBOSE ) {
     debugf("%8zd constructor%s\n", Counter::c_count.load()
           , plural(Counter::c_count.load()));
     debugf("%8zd destructor%s\n",  Counter::d_count.load()
           , plural(Counter::d_count.load()));
   }
}
}  Static_global;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       Counter::Counter
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Counter::Counter( void )           // Default constructor
{  if( HCDM ) debugf("Counter(%p)!\n", this);

   ++c_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Counter::~Counter
//
// Purpose-
//       Destructor.
//
// Notes-
//       All Threads have completed or we wouldn't be here.
//
//----------------------------------------------------------------------------
   Counter::~Counter( void )          // Destructor
{  if( HCDM ) debugf("Counter(%p)~\n", this);

   ++d_count;
}
