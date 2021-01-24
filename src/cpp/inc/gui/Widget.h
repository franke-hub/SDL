//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Widget.h
//
// Purpose-
//       GUI Widget descriptor
//
// Last change date-
//       2021/01/22
//
//----------------------------------------------------------------------------
#ifndef GUI_WIDGET_H_INCLUDED
#define GUI_WIDGET_H_INCLUDED

#include <pub/List.h>               // For List
#include <pub/Named.h>              // For Named

#include "gui/Types.h"              // For gui type definitions

namespace gui {
//----------------------------------------------------------------------------
//
// Class-
//       gui::Widget
//
// Purpose-
//       GUI Widget descriptor.
//
// Implementation notes-
//       FIFO (First In, First Out) ordering: Add to end of List
//       LIFO (Last In, First Out) ordering: Add to beginning of List
//       The child List is always searched starting from the beginning.
//
//       The destructor removes this Widget's w_list children, setting each
//       child Widget's parent= nullptr.  The child Widgets are NOT deleted.
//       Currently, the destoyed Widget IS removed from the Parent's w_list.
//       (This is subject to change.)
//
//----------------------------------------------------------------------------
class Widget : public pub::List<Widget>::Link, public pub::Named {
//----------------------------------------------------------------------------
// Widget::Private attributes
//----------------------------------------------------------------------------
private:
Widget*                parent;      // Our parent Widget
pub::List<Widget>      w_list;      // Our child Widget List

//----------------------------------------------------------------------------
// Widget::Constructors/Destructors/Operators
//----------------------------------------------------------------------------
public:
virtual
   ~Widget( void );                 // Destructor
   Widget(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr); // Widget name

//----------------------------------------------------------------------------
// gui::Widget::GLOBAL basic-lockable
//----------------------------------------------------------------------------
public:
static void
   lock( void );

static void
   unlock( void );

//----------------------------------------------------------------------------
// Widget::List control methods
//----------------------------------------------------------------------------
public:
Widget*                             // The first child Widget
   get_first( void ) const          // Get first child Widget
{  return w_list.get_head(); }

Widget*                             // The parent Widget
   get_parent( void ) const         // Get parent Widget
{  return parent; }

void
   fifo(                            // Insert Widget onto w_list
     Widget*           widget);     // The Widget to insert, FIFO ordering

void
   insert(                          // Insert Widget onto w_list
     Widget*           widget)      // The Widget to insert
{  fifo(widget); }                  // Default, FIFO ordering

int                                 // TRUE if Widget on w_list
   is_on_list(                      // Is Widget on w_list?
     Widget*           widget) const; // The Widget to check

void
   lifo(                            // Insert Widget onto w_list
     Widget*           widget);     // The Widget to insert, LIFO ordering

Widget*                             // The removed Widget
   remove(                          // Remove Widget from w_list
     Widget*           widget= nullptr); // The Widget to remove

//----------------------------------------------------------------------------
// Widget::set_parent(), For possible use when a derived class isn't a child.
//----------------------------------------------------------------------------
protected:
virtual void
   set_parent(                      // Set parent
     Widget*           parent)      // (But don't add this to child list)
{  this->parent= parent; }

//----------------------------------------------------------------------------
// Widget::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // (Configure phase III)
   configure( void )                // Configure the Widget (create object)
{  }                                // (Ignored unless overridden)

virtual void
   debug(                           // Debugging display
     const char*       info= nullptr) const // Associated info
{  (void)info; }                    // (Ignored unless overridden)

virtual void
   draw( void )                     // (Re)draw
{  }                                // (Ignored unless overridden)
}; // class Widget
}  // namespace gui
#endif // GUI_WIDGET_H_INCLUDED
