//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Action.cpp
//
// Purpose-
//       Graphical User Interface: Action implementation
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gui/Object.h"
#include "gui/Action.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Action::~Action
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Action::~Action( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Action(%p)::~Action()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Action::Action
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Action::Action(                  // Constructor
     Object*           parent)      // -> Parent Object
:  parent(NULL), next(NULL)
{
   #ifdef HCDM
     Logger::log("%4d: Action(%p)::Action(%p)\n", __LINE__, this, parent);
   #endif

   if( parent != NULL )
     parent->addAction(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Action::callback
//
// Purpose-
//       Pure virtual method placeholder, does nothing.
//
//----------------------------------------------------------------------------
void
   Action::callback(                // Handle callback
     const Event&      e)           // For this Event
{
   #if defined(HCDM)
     Logger::log("%4d: Action(%p)::callback()\n",
                 __LINE__, this);
   #endif

   (void)e;                         // (Unused)
}

