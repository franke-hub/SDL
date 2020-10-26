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
//       TextWindow.cpp
//
// Purpose-
//       Implement TextWindow.h
//
// Last change date-
//       2020/10/16
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Debug.h>              // For pub::Debug
#include <pub/Trace.h>              // For pub::Trace
#include <pub/UTF8.h>               // For pub::UTF8

#include "Xcb/Global.h"             // For xcb globals
#include "Xcb/Types.h"              // For xcb types
#include "Xcb/TextWindow.h"         // For xcb::TextWindow

using pub::Debug;                   // For Debug object
using pub::Trace;                   // For Trace object
using namespace pub::debugging;     // For debugging subroutines
using namespace xcb;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
}; // Compilation controls

namespace xcb {
//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::TextWindow
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   TextWindow::TextWindow(          // Constructor
     Widget*           parent,      // Our parent Widget
     const char*       name)        // This Window's name
:  Window(parent, name ? name : "TextWindow"), font(this), font_name("7x13")
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::TextWindow(%p,%s)\n", this, parent
           , parent ? parent->get_name().c_str() : "?");

   bg= 0x00FFFFF0;                  // (Pale Yellow Background)
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::~TextWindow
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TextWindow::~TextWindow( void )  // Destructor
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::~TextWindow()...\n", this);

   if( flipGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, flipGC) );
     flipGC= 0;
   }

   if( fontGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, fontGC) );
     fontGC= 0;
   }

   flush();

   if( opt_hcdm )
     debugh("TextWindow(%p)::...~TextWindow()\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::configure
//
// Purpose-
//       Complete the configuration
//
//----------------------------------------------------------------------------
void
   TextWindow::configure(           // Configure the Layout
     Layout::config_t& config)      // Using this configurator
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::configure(config_t)\n", this);

   // Get the Font dimensions
   int rc= set_font(font_name.c_str());
   if( rc )                         // If failed
     set_font();                    // Use default font

   Layout::configure(config);
}

void
   TextWindow::configure( void )    // Create the Window and dependent objects
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::configure\n", this);

   Window::configure();             // Create the Window

   // Create the graphic contexts
   fontGC= font.makeGC(fg, bg);     // (The default)
   flipGC= font.makeGC(bg, fg);     // (Inverted)

   // Default: show the window
   show();
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   TextWindow::debug(               // Debugging display
     const char*       text) const  // Associated text
{
   debugf("TextWindow(%p)::debug(%s) Named(%s)\n", this, text ? text : ""
         , get_name().c_str());
   Window::debug(text);

   debugf("..font_name(%s) flipGC(%u) fontGC(%u)\n"
         , font_name.c_str(), flipGC, fontGC);
   debugf("..col_zero(%zd), row_zero(%zd)\n", col_zero, row_zero);
   debugf("..col(%u) row(%u)\n", col, row);

   if( opt_hcdm || opt_verbose >= 0 ) {
     font.debug(text);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::draw
//
// Purpose-
//       Redraw the window
//
//----------------------------------------------------------------------------
void
   TextWindow::draw( void )         // Redraw the Window
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::draw()\n", this);

   // Clear the drawable window
   WH_size_t size= get_size(__LINE__); // TODO: ??? Why is this needed ???
   rect.width=  size.width;
   rect.height= size.height;
   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   // Display the text (if any)
   if( this->line ) {
     Line* line= this->line;
     last= line;

     row_used= 0;
     unsigned y= get_y(row_used);
     unsigned font_height= font.length.height;
     unsigned last_height= rect.height - USER_BOT * font_height;
     while( (y + font_height) <= last_height ) {
       if( line == nullptr )
         break;
       row_used++;                  // Update used row count
       last= line;                  // Update last line displayed
       const char* text= line->text; // Default, column[0]
       if( line == cursor )         // If cursor line
         text= cursor_text(cursor); // Get cursor line text

       if( col_zero )               // If offset
         text= text + pub::UTF8::index((const unsigned char*)text, col_zero);
       putxy(1, y, text);
       y += font_height;
       line= line->get_next();
     }
     if( opt_hcdm ) debugf("%4d LAST xy(%d,%d)\n", __LINE__, 0, y);

     if( USE_BRINGUP ) {
       // BRINGUP: Draw diagonal line (to see where boundaries are)
       if( opt_hcdm && opt_verbose > 2 ) { // (This is still optional)
//       debug(pub::utility::to_string("%4d TextWindow diagonal", __LINE__).c_str());
         xcb_point_t points[2]= { {0,                0}
                                , {PT_t(rect.width), PT_t(rect.height)}
                                };
         NOQUEUE("xcb_poly_line", xcb_poly_line(c
                , XCB_COORD_MODE_ORIGIN, widget_id, font.fontGC, 2, points));
         if( opt_verbose > 2 )
           debugf("%4d POLY {0,{%d,%d}}\n", __LINE__, rect.width, rect.height);
       }
     }
   }

   // Redraw complete
   flush();
}

//----------------------------------------------------------------------------
//
// Protected method-
//       xcb::TextWindow::get_col
//
// Purpose-
//       Convert pixel x position to (screen) column
//
//----------------------------------------------------------------------------
unsigned                            // The column
   xcb::TextWindow::get_col(        // Get column
     int               x)           // For this x pixel position
{  return unsigned(x+1)/font.length.width; }

//----------------------------------------------------------------------------
//
// Protected method-
//       xcb::TextWindow::get_row
//
// Purpose-
//       Convert pixel y position to (screen) row
//
//----------------------------------------------------------------------------
unsigned                            // The row
   xcb::TextWindow::get_row(        // Get row
     int               y)           // For this y pixel position
{  return (y+1)/int(font.length.height); }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_x
//
// Purpose-
//       Get pixel position for column.
//
//----------------------------------------------------------------------------
unsigned                            // The offset in Pixels
   xcb::TextWindow::get_x(          // Get offset in Pixels
     unsigned          col)         // For this column
{  return col * font.length.width + 1; }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_y
//
// Purpose-
//       Get pixel position for row.
//
//----------------------------------------------------------------------------
unsigned                            // The offset in Pixels
   xcb::TextWindow::get_y(          // Get offset in Pixels
     unsigned          row)         // For this row
{  return (row + USER_TOP) * font.length.height + 1; }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_xy
//
// Purpose-
//       Get [col,row] pixel position.
//
//----------------------------------------------------------------------------
xcb_point_t                         // The offset in Pixels
   TextWindow::get_xy(              // Get offset in Pixels
     unsigned          col,         // And this column
     unsigned          row)         // For this row
{  return {PT_t(get_x(col)), PT_t(get_y(row))}; }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::resize
//
// Purpose-
//       Resize the window
//
//----------------------------------------------------------------------------
void
   TextWindow::resize(              // Resize the Window
     int               x,           // New width
     int               y)           // New height
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::resize(%d,%d)\n", this, x, y);

   if( x < min_size.width )  x= min_size.width;
   if( y < min_size.height ) y= min_size.height;
   if( false ) {                    // This is required. ??? WHY ???
     x += 7; x &= 0xfffffff8;
     y += 7; y &= 0xfffffff8;
   }

   // If size unchanged, do nothing
   WH_size_t size= get_size(__LINE__);
   if( size.width == x && size.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure the window
   set_size(x, y, __LINE__);
   col_size= x / font.length.width;
   row_size= y / font.length.height;

   // Diagnostics
   if( opt_hcdm ) {
     WH_size_t size= get_size();
     debugf("%4d [%d x %d]= chg_size <= [%d x %d]\n",  __LINE__
           , size.width, size.height, rect.width, rect.height);
     rect.width=  size.width;
     rect.height= size.height;
   }

// draw(); // Not required: Expose events generated by set_size()
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::set_font
//
// Purpose-
//       Set the font name. If active, replace the font.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   TextWindow::set_font(            // Set the Font
     const char*       name)        // To this name
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::set_font(%s) conn(%p)\n", this, name, c);

   if( c == nullptr ) {            // If not connected
     this->font_name= name;
     return 0;
   }

   int rc= font.open(name);         // Open the font
   if( rc == 0 ) {                  // Update the Layout
     if( col_size == 0 ) col_size= COLS_W;
     if( row_size == 0 ) row_size= ROWS_H;
     unsigned mini_c= MINI_C;
     unsigned mini_r= MINI_R;
     if( mini_c > col_size ) mini_c= col_size;
     if( mini_r > row_size ) mini_r= row_size;
     min_size= { WH_t(mini_c   * font.length.width  + 1)
               , WH_t(mini_r   * font.length.height + 2) };
     use_size= { WH_t(col_size * font.length.width  + 1)
               , WH_t(row_size * font.length.height + 2) };
     use_unit= { WH_t(font.length.width), WH_t(font.length.height) };

     if( widget_id ) {              // TODO: NEEDS WORK, Layout::configure
       draw();                      // Maybe resize?
     }
   }

   return rc;
}

//----------------------------------------------------------------------------
// xcb::TextWindow: Event handlers
//----------------------------------------------------------------------------
void
   TextWindow::configure_notify(    // Handle this
     xcb_configure_notify_event_t* event) // Configure notify event
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::configure_notify(%d,%d)\n", this
           , event->width, event->height);

   resize(event->width, event->height);
}

void
   TextWindow::expose(              // Handle this
     xcb_expose_event_t* event)     // Expose event
{
   if( opt_hcdm )
     debugh("TextWindow(%p)::expose(%d) %d [%d,%d,%d,%d]\n", this
             , event->window, event->count
             , event->x, event->y, event->width, event->height);

   draw();
}

//----------------------------------------------------------------------------
// See the (UNUSED) resize_request in Window.cpp for a discussion of why
// configure_notify is used instead of this method.
void
   TextWindow::resize_request(      // Handle this
     xcb_resize_request_event_t* event) // Resize request event
{
// if( opt_hcdm )
     debugh("TextWindow(%p)::resize_request(%d,%d)\n", this
           , event->width, event->height);

   resize(event->width, event->height);
}
}  // namespace xcb
