//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       pub/diag-counter.cpp
//
// Purpose-
//       Implement pub/diag-counter.h
//
// Last change date-
//       2025/01/15
//
//----------------------------------------------------------------------------
#include "pub/diag-counter.h"       // For pub::diag::counter, implemented

#include <pub/Debug.h>              // For namespace pub::debugging

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;     // For debugging subroutines

namespace _LIBPUB_NAMESPACE {
namespace diag {
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
int                    Counter::hcdm= false; // Hard Core Debug Mode?

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
{  if( HCDM ) debugf("diag::Counter Static_global!\n"); }

   ~Static_global( void )
{  if( HCDM ) debugf("diag::Counter Static_global~\n");

   if( Counter::c_count != Counter::d_count ) {
     debugf("\n");
     Counter::debug("constructors != destructors");
   } else if( VERBOSE ) {
     Counter::debug("VERBOSE");
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
   Counter::Counter( void )         // Default constructor
{  if( HCDM || hcdm ) debugf("pub::diag::Counter(%p)!\n", this);

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
//----------------------------------------------------------------------------
   Counter::~Counter( void )          // Destructor
{  if( HCDM || hcdm ) debugf("pub::diag::Counter(%p)~\n", this);

   ++d_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Counter::debug
//
// Purpose-
//       Write debugging message.
//
//----------------------------------------------------------------------------
void
   Counter::debug(                  // Debugging display
     const char*       info)        // Caller information
{
   debugf("pub::diag::Counter::debug(%s)\n", info);

   debugf("%8zd constructor%s\n", Counter::c_count.load()
         , plural(Counter::c_count.load()));
   debugf("%8zd destructor%s\n",  Counter::d_count.load()
         , plural(Counter::d_count.load()));
}

//----------------------------------------------------------------------------
//
// Method-
//       Counter::set_hcdm
//
// Purpose-
//       Set/clear Hard Core Debug Mode
//
//----------------------------------------------------------------------------
void
   Counter::set_hcdm(               // Set/clear Hard Core Debug Mode
     bool              _mode)       // New Hard Core Debug Mode
{  hcdm= _mode; }
}  // namespace diag
}  // namespace _LIBPUB_NAMESPACE
