//----------------------------------------------------------------------------
//
//       Copyright (C) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Context.h
//
// Purpose-
//       Context descriptor.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include <com/Debug.h>
#include <obj/List.h>

//----------------------------------------------------------------------------
//
// Class-
//       Context
//
// Purpose-
//       Context descriptor.
//
//----------------------------------------------------------------------------
class Context : public obj::List<Context>::Link { // Context descriptor
//----------------------------------------------------------------------------
// Context::Attributes
//----------------------------------------------------------------------------
protected:
Context*               parent;      // Parent Context
obj::List<Context>     child;       // Child List

//----------------------------------------------------------------------------
// Context::Constructors
//----------------------------------------------------------------------------
public:
   Context(                         // Constructor
     Context*          parent= nullptr) // Parent Context
:  parent(parent), child()
{  if( false ) debugf("Context(%p)::Context(%p)\n", this, parent);
   if( parent )
     parent->child.lifo(this);
}

virtual
   ~Context( void )                 // Destructor
{  if( false ) debugf("Context(%p)::~Context()\n", this);
   for(Context* context= child.remq(); context; context= child.remq())
   {
     delete context;
   }
}

//----------------------------------------------------------------------------
// Context::debug
//
// Context debugging display
//----------------------------------------------------------------------------
void
   debug( void ) const              // Debugging display
{  debugf("Context(%p).debug() parent(%p)\n", this, parent);

   for(Context* context= child.get_head(); context; context= context->get_next())
   {
     debugf("..child(%p) prev(%p) next(%p) parent(%p)\n", context
            , context->get_prev(), context->get_next(), context->parent);
   }

   debugf("----------------\n\n");
   for(Context* context= child.get_head(); context; context= context->get_next())
     context->debug();
}

static void
   debug_static( void )             // Debugging display
{  debugf("Context::debug_static() NOT CODED YET\n"); }

//----------------------------------------------------------------------------
// Context::do_that
// Context::do_this
//
// Placeholder
//----------------------------------------------------------------------------
void
   do_that( void ) {}               // Do something

void
   do_this( void ) {}               // Do something else
}; // class Context

#endif // CONTEXT_H_INCLUDED
