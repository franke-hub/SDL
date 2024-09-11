//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdRedo.cpp
//
// Purpose-
//       Implement EdRedo.h
//
// Last change date-
//       2024/08/28
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, fopen, fclose, ...
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For unlink
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Trace.h>              // For pub::Trace
#include <pub/List.h>               // For pub::List

#include "Config.h"                 // For namespace config
#include "EdRedo.h"                 // For EdRedo - implemented

using namespace config;             // For config::opt_*
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_OBJECT_COUNT= true           // Use object counting?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       EdRedo::EdRedo
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdRedo::EdRedo( void )           // Constructor
:  ::pub::List<EdRedo>::Link()
{  if( HCDM || opt_hcdm )
     traceh("EdRedo(%p)::EdRedo\n", this);

   Trace::trace(".NEW", "redo", this);
}

   EdRedo::~EdRedo( void )          // Destructor
{  if( HCDM || opt_hcdm )
     traceh("EdRedo(%p)::~EdRedo\n", this);

   Trace::trace(".DEL", "redo", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRedo::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdRedo::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  traceh("EdRedo(%p)::debug(%s)\n", this, info ? info : "");

   traceh("  COL [%3zd:%3zd]\n", lh_col, rh_col);

   traceh("  INS [");
   if( head_insert ) tracef("%p<-", head_insert->get_prev());
   tracef("%p,%p", head_insert, tail_insert);
   if( tail_insert ) tracef("->%p", tail_insert->get_next());
   tracef("],\n");

   for(EdLine* line= head_insert; line; line=line->get_next() ) {
     traceh("    "); line->debug();
     if( line == tail_insert )
       break;
   }

   traceh("  REM [");
   if( head_remove ) tracef("%p<-", head_remove->get_prev());
   tracef("%p,%p", head_remove, tail_remove);
   if( tail_remove ) tracef("->%p", tail_remove->get_next());
   tracef("]\n");

   for(EdLine* line= head_remove; line; line=line->get_next() ) {
     traceh("    "); line->debug();
     if( line == tail_remove )
       break;
   }
}
