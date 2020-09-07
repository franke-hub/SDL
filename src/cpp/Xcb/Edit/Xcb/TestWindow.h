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
//       2020/09/06
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
{  HCDM= true                       // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// xcb::TestWindow::Attributes
//----------------------------------------------------------------------------
public:
::pub::List<Line>       list;       // The Line list

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
{
   if( use_debug() )
     debugh("TestWindow(%p)::TestWindow\n", this);

   // Set dimensionality
   col_size= 16;
   row_size=  8;

   // Create the data
   char buffer[128];                // Line buffer
   for(int i= 0; i<16; i++) {
     sprintf(buffer, "%.4d Line abcdefghijklmnopqrstuv", i);
     Line* line= new Line(strdup(buffer));
     list.fifo(line);
   }

   this->line= list.get_head();
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

   // Delete the lines
   for(;;) {
     Line* line= list.remq();
     if( line == nullptr )
       break;

     free(line);
   }

   // Wait for deletion operations to complete
   flush();

   if( use_debug() )
     debugh("TestWindow(%p)::...~TestWindow()\n", this);
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
