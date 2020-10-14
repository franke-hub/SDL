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
//       Xcb/TestWindow.h
//
// Purpose-
//       Test window: For experimentation
//
// Last change date-
//       2020/10/13
//
//----------------------------------------------------------------------------
#ifndef TESTWINDOW_H_INCLUDED
#define TESTWINDOW_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include <string>                   // For std::string
#include <pub/utility.h>            // TODO: REMOVE. for to_string

#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xfixes.h>             // For XCB xfixes
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Font.h"               // For xcb::Font
#include "Xcb/Device.h"             // For xcb::Device
#include "Xcb/Global.h"             // For xcb::ENQUEUE macro
#include "Xcb/Keysym.h"             // For X11/keysymdef.h
#include "Xcb/Types.h"              // For xcb::Line
#include "Xcb/TextWindow.h"         // For xcb::TextWindow base class

namespace xcb {
//----------------------------------------------------------------------------
//
// Class-
//       xcb::TestWindow
//
// Purpose-
//       xcb::Window containing text.
//
//----------------------------------------------------------------------------
class TestWindow : public TextWindow { // Test Window
//----------------------------------------------------------------------------
// xcb::TestWindow: Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// xcb::TestWindow::Attributes
//----------------------------------------------------------------------------
public:
::pub::List<Line>       list;       // The Line list
Line*                   data= nullptr; // The (only) line

//----------------------------------------------------------------------------
// use_debug: Test for debugging active
//----------------------------------------------------------------------------
static inline bool use_debug( void ) { return HCDM || USE_BRINGUP || opt_hcdm; }

//----------------------------------------------------------------------------
// xcb::TestWindow: Constructor
//----------------------------------------------------------------------------
public:
   TestWindow(                      // Constructor
     Widget*           parent= nullptr, // Our parent Widget
     const char*       name= nullptr) // This Window's name
:  TextWindow(parent, name ? name : "TestWindow")
,  list(), data(new Line("This is the test line"))
{
   if( use_debug() )
     debugh("TestWindow(%p)::TestWindow(%p,%s)\n", this, parent
           , parent ? parent->get_name().c_str() : "?");

   // Set dimensionality
   col_size= 80;
   row_size=  1;

   // Initialize the List
   line= data;
   list.fifo(data);
}

//----------------------------------------------------------------------------
// xcb::TestWindow: Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~TestWindow( void )              // Destructor
{
   if( use_debug() )
     debugh("TestWindow(%p)::~TestWindow()...\n", this);

   delete data;
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TestWindow::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       text= nullptr) const // Associated text
{
   debugf("TestWindow(%p)::debug(%s) Named(%s)\n", this, text ? text : ""
         , get_name().c_str());

   TextWindow::debug(text);
}

void
   configure( void )                // Create the Window and dependent objects
{
   if( opt_hcdm )
     debugh("TestWindow(%p)::configure\n", this);

   TextWindow::configure();

   draw();
   show();
}
}; // class xcb::TestWindow
}  // namespace xcb
#endif // TESTWINDOW_H_INCLUDED
