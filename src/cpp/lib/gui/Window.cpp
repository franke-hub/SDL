//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Window.cpp
//
// Purpose-
//       Implement gui/Window.h and gui/Pixmap.h
//
// Last change date-
//       2024/03/31
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard

#include <xcb/xcb.h>                // For XCB types
#include <xcb/xfixes.h>             // For XCB fixes
#include <xcb/xproto.h>             // For XCB interfaces

#include <pub/Debug.h>              // For Debug object
#include <pub/utility.h>            // For pub::utility::clock

#include "gui/Device.h"             // For Device
#include "gui/Global.h"             // For opt_* definitions
#include "gui/Pixmap.h"             // For Pixmap, base class
#include "gui/Types.h"              // For Type definitions
#include "gui/Window.h"             // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Use bringup options?
}; // Compilation controls

namespace gui {
//----------------------------------------------------------------------------
//
// Subroutine-
//       xcbcheck
//
// Purpose-
//       Validate an XCB result.
//
//----------------------------------------------------------------------------
static void
   xcbcheck(                        // Verify XCB function result
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // Function name
     xcb_generic_error_t* xc)       // Generic error
{
   if( xc ) {
     debugh("%4d %s EC(%d)= %s()\n", line, file, xc->error_code, name);
     xcberror(xc);

     debugh("%4d %s::%s CHECKSTOP\n", line, file, name);
     debug_flush();
     exit(2);
   } else if( opt_hcdm && opt_verbose > 1 ) {
     debugh("%4d %s::%s()\n", line, file, name);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::Pixmap
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Pixmap::Pixmap(                  // Constructor
     Widget*           parent,      // Our parent Widget
     const char*       name)        // The Pixmap's name
:  Layout(parent, name ? name : "Pixmap"), device(nullptr)
{
   if( opt_hcdm )
     debugh("Pixmap(%p)::Pixmap(%p,%s)\n", this, parent
           , parent ? parent->get_name().c_str() : "?");
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::~Pixmap
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Pixmap::~Pixmap( void )          // Destructor
{
   if( opt_hcdm )
     debugh("Pixmap(%p)::~Pixmap()\n", this);

   // Free the pixmap
   if( widget_id ) {
     ENQUEUE("xcb_free_pixmap", xcb_free_pixmap_checked(c, widget_id));
     widget_id= 0;
     flush();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::configure
//
// Purpose-
//       Configure this Pixmap
//
//----------------------------------------------------------------------------
void
   Pixmap::configure(               // Configure this Layout using
     Device*           device,      // This parent Device and
     Window*           window)      // This parent Window
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("Pixmap(%p)::configure(%p,%p)\n", this, device, window);

   // Get Device, connection, and screen information
   this->device= device;           // Set (root) Device
   this->window= window;           // Set parent Window
   c= device->c;                   // Set Device connection
   s= device->s;                   // Set Device screen
}

void
   Pixmap::configure( void )        // Configure (create) Pixmap
{
   if( opt_hcdm )
     debugh("Pixmap(%p)::configure [%u,%u]\n", this, rect.width, rect.height);

   // Set the parent_id. (The parent Window has been configured)
   parent_id= window->widget_id;

   // Create the Pixmap
   if( widget_id != 0 ) {           // If already created
     debugf("%4d Pixmap: Nothing to do when pixmap created\n", __LINE__);
     return;
   }

   widget_id= xcb_generate_id(c);   // Our XCB Pixmap ID
   ENQUEUE("xcb_create_pixmap", xcb_create_pixmap_checked
          ( c, s->root_depth, widget_id, parent_id
          , rect.width, rect.height // Default position and size
          ) );
   if( opt_hcdm )
     debugh("Pixmap(%p) created(%u) parent(%u)\n", this, widget_id, parent_id);
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Pixmap::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   if( info == nullptr ) info= "";
   debugf("Pixmap(%p)::debug(%s)\n", this, info);
   debugf("..device(%p), window(%p)\n", device, window);
   debugf("..c(%p)\n",   c);
   debugf("..s(%p)\n",   s);
   debugf("..parent_id(%d)\n", parent_id);
   debugf("..widget_id(%d)\n", widget_id);
   debugf("..rect(%d,%d,%u,%u)\n", rect.x, rect.y, rect.width, rect.height);
   debugf("..penduse(%d)\n", penduse);
   for(unsigned i= 0; i<penduse; i++) {
     const Pending& p= pending[i];
     debugf("..[%2d] %4d: %s(%6u) %s\n", i, p.opline, p.opfile
           , p.op.sequence, p.opname);
   }

   Layout::debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::get_size
//
// Purpose-
//       Get current width and height
//
//----------------------------------------------------------------------------
WH_size_t                           // The current Pixmap size
   Pixmap::get_size( void )         // Get current Piamap size
{
   xcb_get_geometry_cookie_t cookie= xcb_get_geometry(c, widget_id);
   xcb_get_geometry_reply_t* r= xcb_get_geometry_reply(c, cookie, nullptr);
   WH_size_t size= {rect.width, rect.height};
   if( r ) {
     size= {r->width, r->height};
     free(r);
   } else
     debugf("%4d Pixmap xcb_get_geometry error\n", __LINE__);

   if( opt_hcdm )
     debugh("[%u x %u]= get_size\n", size.width, size.height);

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::set_size
//
// Purpose-
//       Set current width and height
//
// Implementation notes-
//       Window::set_size overrides this method.
//
//----------------------------------------------------------------------------
void
   Pixmap::set_size(                // Set Pixmap size
     int               x,           // New width
     int               y)           // New height
{  if( opt_hcdm )
     traceh("Pixmap::set_size(%d,%d)\n", x, y);

   rect.width=  uint16_t(x);
   rect.height= uint16_t(y);

   // Free any existing Pixmap
   if( widget_id != 0 )             // If already created, replace it
     ENQUEUE("xcb_free_pixmap", xcb_free_pixmap_checked(c, widget_id));

   // Create the (new) Pixmap
   const xcb_window_t widget_id= xcb_generate_id(c);
   ENQUEUE("xcb_create_pixmap", xcb_create_pixmap_checked
          ( c, s->root_depth, parent_id, widget_id, rect.width, rect.height ) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::enqueue
//       Pixmap::noqueue
//
// Purpose-
//       Add operation cookie to pending queue
//       Ignore operation cookie. Handle (error) response in the reply loop.
//
//----------------------------------------------------------------------------
void
   Pixmap::enqueue(                 // Add operation to pending queue
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // Operation name
     xcb_void_cookie_t op)          // Operation cookie
{
   if( opt_hcdm && opt_verbose > 0 )
     traceh("Pixmap(%p)::enqueue(%s)\n", this, name);

   if( penduse >= DIM_PENDING ) {
     debugf("%4d HCDM Window.cpp UNEXPECTED QUEUE FULL EVENT\n", __LINE__);
     flush();
   }

   Pending& pending= this->pending[penduse++];
   pending.opname= name;
   pending.opname= file;
   pending.opline= line;
   pending.op=     op;
}

// Ignore operation cookie, handling (error) responses in the reply loop.
void                                // Response handled in reply loop
   Pixmap::noqueue(                 // Drive operation
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // Operation name
     xcb_void_cookie_t op)          // Operation cookie
{
   (void)op;                        // Unused parameter
   if( opt_hcdm && opt_verbose > 0 )
     traceh("Pixmap(%p)::noqueue %4d %s(%p)\n", this, line, file, name);
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::flush
//
// Purpose-
//       Complete outstanding operations
//
//----------------------------------------------------------------------------
void
   Pixmap::flush( void )            // Flush outstanding operations
{
   if( opt_hcdm && opt_verbose > 0 )
     debugh("Pixmap(%p)::flush(%u)\n", this, penduse);

   for(unsigned i= 0; i<penduse; i++) {  // Complete pending operations
     Pending& pending= this->pending[i];
     synchronously(pending.opline, pending.opfile, pending.opname, pending.op);
   }

   penduse= 0;
   if( c )                          // (Pixmap may be uninitialized)
     xcb_flush(c);
}

//----------------------------------------------------------------------------
//
// Method-
//       Pixmap::synchronously
//
// Purpose-
//       Synchronously handle an XCB operation.
//
//----------------------------------------------------------------------------
void
   Pixmap::synchronously(           // Synchronous XCB operation
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // The operation name
     xcb_void_cookie_t op)          // The synchronous operation (cookie)
{  xcbcheck(line, file, name, xcb_request_check(c, op)); }

void
   Pixmap::synchronously(           // Synchronous XCB operation
     xcb_void_cookie_t op)          // The synchronous operation (cookie)
{  xcbcheck(__LINE__, __FILE__, "synchronously", xcb_request_check(c, op)); }

//----------------------------------------------------------------------------
//
// Method-
//       Window::Window
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Window::Window(                  // Constructor
     Widget*           parent,      // Our parent Widget
     const char*       name)        // The Window's name
:  Pixmap(parent, name ? name : "Window")
{
   if( opt_hcdm )
     debugh("Window(%p)::Window(%p,%s)\n", this, parent
           , parent ? parent->get_name().c_str() : "?");
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::~Window
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Window::~Window( void )          // Destructor
{
   if( opt_hcdm )
     debugh("Window(%p)::~Window()\n", this);

   // Destroy the window
   if( widget_id ) {
     ENQUEUE("xcb_destroy_window", xcb_destroy_window_checked(c, widget_id));
     widget_id= 0;
     flush();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::atom_to_name
//       Window::name_to_atom
//
// Purpose-
//       Extract name from xcb_atom_t
//       Extract xcb_atom_t from name
//
//----------------------------------------------------------------------------
std::string                         // The associated name
   Window::atom_to_name(            // Get associated name
     xcb_atom_t        atom)        // For this atom
{
   xcb_get_atom_name_cookie_t
       cookie= xcb_get_atom_name(c, atom);
   xcb_get_atom_name_reply_t*
       reply= xcb_get_atom_name_reply(c, cookie, nullptr);
   if( reply == nullptr ) return "<null>";
   int size= xcb_get_atom_name_name_length(reply);
   const char* name= xcb_get_atom_name_name(reply);
   std::string result;
   if( name ) {
     std::string s(name, size);
     result= s;
   }
   free(reply);

   return result;
}

xcb_atom_t                          // The associated xcb_atom_t
   Window::name_to_atom(            // Get xcb_atom_t
     const char*       name,        // For this name
     int               only)        // (Do not create atom indicator)
{
   xcb_intern_atom_cookie_t
       cookie= xcb_intern_atom(c, bool(only), uint16_t(strlen(name)), name);
   xcb_intern_atom_reply_t*
       reply= xcb_intern_atom_reply(c, cookie, nullptr);
   xcb_atom_t result= reply->atom;
   free(reply);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::configure
//
// Purpose-
//       Configure this Window
//
//----------------------------------------------------------------------------
void
   Window::configure( void )        // Configure (create) Window
{
   if( opt_hcdm )
     debugh("Window(%p)::configure [%d,%d,%u,%u]\n", this
           , rect.x, rect.y, rect.width, rect.height);

   // Set the parent_id. (The parent Window has been configured)
   parent_id= window->widget_id;

   // Create the Window
   if( widget_id != 0 ) {           // If already created
     debugf("%4d Window: Nothing to do when window created\n", __LINE__);
     return;
   }

   if( emask == 0 )
     emask= XCB_EVENT_MASK_KEY_PRESS
//        | XCB_EVENT_MASK_KEY_RELEASE
          | XCB_EVENT_MASK_BUTTON_PRESS
//        | XCB_EVENT_MASK_EXPOSURE        // (In DEV_EVENT_MASK)
          | XCB_EVENT_MASK_STRUCTURE_NOTIFY // (Drives configure_notify)
//        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
//        | XCB_EVENT_MASK_PROPERTY_CHANGE // (In DEV_EVENT_MASK)
          ;
   emask |= DEV_EVENT_MASK;         // (For Device support)

   widget_id= xcb_generate_id(c);   // Our XCB Window
   uint32_t mask= XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
   uint32_t parm[2];
   parm[0]= bg;                     // Background Pixel
   parm[1]= emask;

   if( USE_BRINGUP ) {
     // For bringup, we enable all events to see what they do.
     // When opt_hcdm is specified, Device.cpp displays each event.
     parm[1] |= (XCB_EVENT_MASK_NO_EVENT
          | XCB_EVENT_MASK_BUTTON_RELEASE
          | XCB_EVENT_MASK_ENTER_WINDOW
          | XCB_EVENT_MASK_LEAVE_WINDOW
          | XCB_EVENT_MASK_POINTER_MOTION
          | XCB_EVENT_MASK_POINTER_MOTION_HINT
          | XCB_EVENT_MASK_BUTTON_1_MOTION
          | XCB_EVENT_MASK_BUTTON_2_MOTION
          | XCB_EVENT_MASK_BUTTON_3_MOTION
          | XCB_EVENT_MASK_BUTTON_4_MOTION
          | XCB_EVENT_MASK_BUTTON_5_MOTION
          | XCB_EVENT_MASK_BUTTON_MOTION
          | XCB_EVENT_MASK_KEYMAP_STATE
          | XCB_EVENT_MASK_VISIBILITY_CHANGE
//        | XCB_EVENT_MASK_RESIZE_REDIRECT  // Conflicts with STRUCTURE_NOTIFY
          | XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
          | XCB_EVENT_MASK_FOCUS_CHANGE
          | XCB_EVENT_MASK_PROPERTY_CHANGE
          | XCB_EVENT_MASK_COLOR_MAP_CHANGE
          | XCB_EVENT_MASK_OWNER_GRAB_BUTTON
          );
     if( opt_hcdm )
       debugf("%4d Window EventMask 0x%.8x\n", __LINE__, parm[1]);
   }

   ENQUEUE("xcb_create_window", xcb_create_window_checked
          ( c, s->root_depth, widget_id, parent_id
          , rect.x, rect.y, rect.width, rect.height // Default position and size
          , XCB_WINDOW_CLASS_COPY_FROM_PARENT, XCB_WINDOW_CLASS_INPUT_OUTPUT
          , s->root_visual, mask, parm
          ) );
   if( opt_hcdm )
     debugh("Window(%p) created(%u) parent(%u)\n", this, widget_id, parent_id);

   // Configure XFIXES library, enabling xcb_xfixes_hide_cursor
   xcb_xfixes_query_version_cookie_t cookie=
   xcb_xfixes_query_version(c, XCB_XFIXES_MAJOR_VERSION
                          , XCB_XFIXES_MINOR_VERSION);
   xcb_xfixes_query_version_reply_t* reply=
   xcb_xfixes_query_version_reply(c, cookie, nullptr);
   if( reply ) {                    // Reply mostly ignored
     if( opt_hcdm )
       debugh("query_xfixes reply: major(%d) minor(%d)\n"
             , reply->major_version, reply->minor_version);
     free(reply);
   }

// flush();                         // (NOT NEEDED)
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Window::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  Pixmap::debug(info); }

//----------------------------------------------------------------------------
//
// Method-
//       Window::set_size
//
// Purpose-
//       Set current width and height
//
//----------------------------------------------------------------------------
void
   Window::set_size(                // Set window size
     int               x,           // New width
     int               y)           // New height
{  if( opt_hcdm )
     debugh("Window::set_size(%d,%d)\n", x, y);

   rect.width=  uint16_t(x);
   rect.height= uint16_t(y);

   int16_t mask= XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
   int32_t parm[2]= { x, y };
   synchronously(__LINE__, __FILE__
                , "xcb_configure_window", xcb_configure_window_checked
                ( c, widget_id, mask, parm ) );
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::hide
//       Window::show
//
// Purpose-
//       Hide the Window
//       Show the Window
//
//----------------------------------------------------------------------------
void
   Window::hide( void )             // Hide the Window
{
   if( opt_hcdm )
     debugh("Window(%p)::hide Named(%s)\n", this, get_name().c_str());

   if( state & WS_VISIBLE ) {
     ENQUEUE("xcb_unmap_window", xcb_unmap_window_checked(c, widget_id));
     state &= ~(WS_VISIBLE);
   }
}

void
   Window::show( void )             // Show the Window
{
   if( opt_hcdm )
     debugh("Window(%p)::show Named(%s)\n", this, get_name().c_str());

   if( (state & WS_VISIBLE) == 0 ) {
     ENQUEUE("xcb_map_window", xcb_map_window_checked(c, widget_id));
     state |= WS_VISIBLE;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::key_press
//
// Purpose-
//       Event handler: xcb_key_press_event_t
//
//----------------------------------------------------------------------------
void
   Window::key_press(               // Handle this
     xcb_key_press_event_t* event)  // Key press event
{
   xcb_keysym_t key= device->to_keysym(event); // Convert to xcb_keysym_t
   if( opt_hcdm )
     debugh("Window(%p)::key_press(0x%.6x)\n", this, key);

   key_input(key, event->state);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::resize_request
//
// Purpose-
//       Document unexplained problem found during bringup testing.
//
//----------------------------------------------------------------------------
#if false //-- DOCUMENTATION ONLY --------------------------------------------
virtual void
   resize_request(                  // Handle this
     xcb_resize_request_event_t* event) // Resize request event
{
   debugh("TextWindow(%p)::resize_request(%d,%d)\n", this
         , event->width, event->height);

   // Obviously this author found out something he can't explain properly.
   // This warning explains one thing you shouldn't do that's hard to debug.
   debugf("WARNING: When XCB_EVENT_MASK_RESIZE_REDIRECT "
          "and XCB_EVENT_MASK_STRUCTURE_NOTIFY are set,\n"
          "resize_request is not invoked but configure_notify is driven "
          "each time xcb_configure_window is invoked,\n"
          "resulting in an infinite loop.\n"
          "When RESIZE_REDIRECT is used instead of STRUCTURE_NOTIFY,"
          "the window background is not filled in.\n"
          "Since STRUCTURE_NOTIFY logic is working, debugging this anomaly "
          "isn't a high priority.\n"
          );
}
#endif //-- DOCUMENTATION ONLY -----------------------------------------------
}  // namespace gui
