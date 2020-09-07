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
//       Xcb/Widget.cpp
//
// Purpose-
//       Implement Widget.h
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#include <exception>                // For std::invalid_argument
#include <mutex>                    // For mutex, std::lock_guard

#include <pub/Debug.h>              // For Debug object

#include "Xcb/Global.h"             // For xcb::opt_hcdm, xcb::opt_verbose
#include "Xcb/Widget.h"             // Implementation class

using namespace pub::debugging;     // For debugging subroutines

namespace xcb {
//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::recursive_mutex
                       mutex;       // Global (recursive) mutex

//----------------------------------------------------------------------------
// duplicate_insert: Attempt to insert a widget that already has a parent
//----------------------------------------------------------------------------
static void
   duplicate_insert(                // Handle duplicate insert
     const Widget*     widget,      // The current Widget
     const Widget*     insert)      // The insert Widget
{
   Widget* parent= insert->get_parent();
   debugf("Widget(%p,%s)::insert(%p,%s) with parent(%p,%s)\n"
         , widget, widget->get_name().c_str()
         , insert, insert->get_name().c_str()
         , parent, parent->get_name().c_str() );
   debug_flush();
   throw std::invalid_argument("Widget::insert, but widget has parent");
}

//----------------------------------------------------------------------------
// not_a_child: Attempt to remove a widget that isn't a child
//----------------------------------------------------------------------------
static void
   not_a_child(                     // Handle improper remove
     const Widget*     widget,      // The current Widget
     const Widget*     remove)      // The remove Widget
{
   Widget* parent= remove->get_parent();
   debugf("Widget(%p,%s)::remove(%p,%s) with parent(%p,%s)\n"
         , widget, widget->get_name().c_str()
         , remove, remove->get_name().c_str()
         , parent, parent ? parent->get_name().c_str() : "<nullptr>");
   debug_flush();
   throw std::invalid_argument("Widget::remove, but widget not a child");
}

//----------------------------------------------------------------------------
// nullptr_argument: widget parameter == nullptr
//----------------------------------------------------------------------------
static void
   nullptr_argument(                // Handle nullptr argument
     const Widget*     widget,      // The current Widget
     const char*       op)          // Operation
{
   debugf("Widget(%p,%s)::%s(<nullptr>)\n"
         , widget, widget->get_name().c_str(), op);
   debug_flush();
   throw std::invalid_argument("<nullptr>");
}

//----------------------------------------------------------------------------
//
// Method-
//       Widget::Widget
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Widget::Widget(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  pub::List<Widget>::Link(), Named(name ? name : "*unnamed*")
,  parent(nullptr), w_list()
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("Widget(%p)::Widget Named(%s)\n", this, get_name().c_str());

   if( parent )
     parent->insert(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Widget::~Widget
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Widget::~Widget( void )          // Destructor
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("Widget(%p)::~Widget\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( parent )
     parent->remove(this);

   for(Widget* child= w_list.remq(); child; child= w_list.remq() ) {
     child->parent= nullptr;
   }
}

//----------------------------------------------------------------------------
// Widget::GLOBAL lock
//----------------------------------------------------------------------------
void
   Widget::lock( void )
{  mutex.lock(); }

void
   Widget::unlock( void )
{  mutex.unlock(); }

//----------------------------------------------------------------------------
// Widget::List control methods
//----------------------------------------------------------------------------
void
   Widget::fifo(                    // Insert Widget onto w_list
     Widget*           widget)      // The Widget to insert, FIFO ordering
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( widget == nullptr )
     nullptr_argument(this, "fifo");

   if( widget->parent )
     duplicate_insert(this, widget);

   w_list.fifo(widget);
   widget->parent= this;
}

int                                 // TRUE if Widget on w_list
   Widget::is_on_list(              // Is widget on w_list?
     Widget*           widget) const // The Widget to check
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( widget == nullptr )
     nullptr_argument(this, "is_on_list");

   return w_list.is_on_list(widget);
}

void
   Widget::lifo(                    // Insert Widget onto w_list
     Widget*           widget)      // The Widget to insert, LIFO ordering
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( widget == nullptr )
     nullptr_argument(this, "lifo");

   if( widget->parent )
     duplicate_insert(this, widget);

   w_list.lifo(widget);
   widget->parent= this;
}

Widget*                             // The removed Widget
   Widget::remove(                  // Remove Widget from w_list
     Widget*           widget)      // The Widget to remove
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( widget ) {
     if( widget->parent != this )
       not_a_child(this, widget);

     w_list.remove(widget, widget);
     widget->parent= nullptr;
   } else {
     widget= w_list.remq();
     if( widget)
       widget->parent= nullptr;
   }

   return widget;
}
}  // namespace xcb
