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
//       EdLine.cpp
//
// Purpose-
//       Implement EdLine.h
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
#include "EdLine.h"                 // For EdLine - implemented

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
//       EdLine::EdLine
//
// Purpose-
//       Constructor/Destructor
//
// Implementation notes-
//       Implicit copy constructor used only in EdView for a temporary EdLine.
//       (This copy constructor DOES NOT increment object_count.)
//
//----------------------------------------------------------------------------
   EdLine::EdLine(                  // Constructor
     const char*       text)        // (Immutable) text
:  ::pub::List<EdLine>::Link(), text(text ? text : "")
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::EdLine\n", this);

   Trace::trace(".NEW", "line", this);

   if( USE_OBJECT_COUNT )
     ++object_count;
}

   EdLine::~EdLine( void )          // Destructor
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::~EdLine\n", this);

   if( flags & F_AUTO )             // If this is a temporary line
     return;                        // Delete means nothing

   Trace::trace(".DEL", "line", this);

   if( USE_OBJECT_COUNT )
     --object_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::debug
//
// Purpose-
//       (Minimal) debugging display
//
//----------------------------------------------------------------------------
void
   EdLine::debug( void ) const      // Minimal debugging display
{
   char buffer[42]; buffer[41]= '\0';
   strncpy(buffer, text, 41);
   tracef("%p F(%.4x) D(%.2x,%.2x) '%s'\n", this, flags
         , delim[0], delim[1], buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::is_within
//
// Purpose-
//       Is this line within range head..tail (inclusive)?
//
//----------------------------------------------------------------------------
bool
   EdLine::is_within(               // Is this line within range head..tail?
     const EdLine*     head,        // First line in range
     const EdLine*     tail) const  // Final line in range
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::is_within(%p,%p)\n", this, head, tail);

   for(const EdLine* line= head; line; line= line->get_next() ) {
     if( line == this )
       return true;
     if( line == tail )
       return false;
   }

   /// We get here because line == nullptr, which should not occur.
   /// The associated list segment is corrupt, and code needs fixing.
   if( head || tail )               // If the range is not empty
     traceh("%4d EdLine(%p).is_within(%p..%p) invalid range\n", __LINE__
           , this, head, tail);
   return false;
}
