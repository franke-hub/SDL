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
//       EdMess.cpp
//
// Purpose-
//       Implement EdMess.h
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
#include "EdMess.h"                 // For EdMess - implemented

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
//       EdMess::EdMess
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdMess::EdMess(                  // Constructor
     std::string       mess_,       // Message text
     int               type_)       // Message type
:  ::pub::List<EdMess>::Link(), mess(mess_), type(type_)
{  if( HCDM || opt_hcdm )
     traceh("EdMess(%p)::EdMess(%s,%d)\n", this, mess_.c_str(), type_);
}

   EdMess::~EdMess( void )          // Destructor
{  if( HCDM || opt_hcdm )
     traceh("EdMess(%p)::~EdMess\n", this);
}
