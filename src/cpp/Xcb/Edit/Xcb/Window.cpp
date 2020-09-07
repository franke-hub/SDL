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
//       Xcb/Window.cpp
//
// Purpose-
//       Implement Xcb/Window.h
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard

#include <xcb/xcb.h>                // For XCB types
#include <xcb/xfixes.h>             // For XCB fixes
#include <xcb/xproto.h>             // For XCB interfaces

#include <pub/Debug.h>              // For Debug object
#include <pub/utility.h>            // For pub::utility::clock

#include "Xcb/Device.h"             // For Device
#include "Xcb/Global.h"             // For Global data areas and utilities
#include "Xcb/Types.h"              // For Type definitions
#include "Xcb/Window.h"             // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Use bringup options?
}; // Compilation controls

namespace xcb {
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
:  Layout(parent, name ? name : "Window"), device(nullptr)
{
   if( opt_hcdm )
     debugh("Window(%p)::Window(%p,%s) Named(%s)\n", this, parent
           , parent ? parent->get_name().c_str() : "?", get_name().c_str());
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
   if( window_id ) {
     ENQUEUE("xcb_destroy_window", xcb_destroy_window_checked
            (connection, window_id));
     window_id= 0;
     flush();
   }
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
   Window::configure(               // Configure this Layout using
     Device*           device,      // This parent Device and
     Window*           window)      // This parent Window
{
   if( opt_hcdm )
     debugh("Layout(%p)::configure(%p,%p)\n", this, device, window);

   // Get Device, connection, and screen information
   this->device= device;           // Set (root) Device
   this->window= window;           // Set parent Window
   connection= device->connection; // Set Device connection
   screen= device->screen;         // Set Device screen
}

void
   Window::configure( void )        // Configure (create) Window
{
   if( opt_hcdm )
     debugh("Window(%p)::configure [%d,%d,%u,%u]\n", this
           , rect.x, rect.y, rect.width, rect.height);

   // Create the Window
   if( window_id != 0 ) {           // If already created
     debugf("%4d Window: Nothing to do when window created\n", __LINE__);
     return;
   }

   if( this != device )             // (Device initialized in constructor)
     parent_id= window->window_id;
   xcb_connection_t* const C= connection;
   xcb_screen_t*     const S= screen;
   xcb_window_t      const P= parent_id;
   xcb_window_t      const W=
   window_id= xcb_generate_id(C);   // Our XCB Window

   uint32_t mask= XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
   uint32_t parm[2];
   parm[0]= bg_pixel;               // Background Pixel
   parm[1]= DEV_EVENT_MASK          // (For device support)
          | XCB_EVENT_MASK_KEY_PRESS
          | XCB_EVENT_MASK_KEY_RELEASE
          | XCB_EVENT_MASK_BUTTON_PRESS
//        | XCB_EVENT_MASK_EXPOSURE        // (In DEV_EVENT_MASK)
          | XCB_EVENT_MASK_STRUCTURE_NOTIFY // (Drives configure_notify)
//        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
//        | XCB_EVENT_MASK_PROPERTY_CHANGE // (In DEV_EVENT_MASK)
          ;

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
          ( C, S->root_depth, W, P
          , rect.x, rect.y, rect.width, rect.height // Default position and size
          , XCB_WINDOW_CLASS_COPY_FROM_PARENT, XCB_WINDOW_CLASS_INPUT_OUTPUT
          , S->root_visual, mask, parm
          ) );
   if( opt_hcdm )
     debugh("Window(%p) created(%u) parent(%u)\n", this, window_id, parent_id);

   // Configure XFIXES library, enabling xcb_xfixes_hide_cursor
   xcb_xfixes_query_version_cookie_t cookie=
   xcb_xfixes_query_version(C, XCB_XFIXES_MAJOR_VERSION
                          , XCB_XFIXES_MINOR_VERSION);
   xcb_xfixes_query_version_reply_t* reply=
   xcb_xfixes_query_version_reply(C, cookie, nullptr);
   if( reply ) { // (Reply ignored)
     free(reply);
   }

   show();                          // TODO: REMOVE???? (EdText needs show!)
   flush();
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
     const char*       text) const  // Associated text
{
   if( text == nullptr ) text= "";
   debugf("Window(%p)::debug(%s) state(0x%.8x)\n", this, text
         , *((uint32_t*)&state) );
   debugf("..device(%p), window(%p)\n", device, window);
   debugf("..connection(%p)\n",    connection);
   debugf("..screen(%p)\n",        screen);
   debugf("..parent_id(%d)\n",     parent_id);
   debugf("..window_id(%d)\n",     window_id);
   debugf("..rect(%d,%d,%u,%u)\n", rect.x, rect.y, rect.width, rect.height);
   debugf("..penduse(%d)\n", penduse);
   for(int i= 0; i<penduse; i++) {
     const Pending& p= pending[i];
     debugf("..[%2d] %4d: (%6u) %s\n", i, p.opline, p.op.sequence, p.opname);
   }

   Layout::debug(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::get_size
//
// Purpose-
//       Get current width and height
//
//----------------------------------------------------------------------------
WH_size_t                           // The current window size
   Window::get_size(                // Get current window size
     int               line)        // Caller's line number
{
   xcb_get_geometry_cookie_t c= xcb_get_geometry(connection, window_id);
   xcb_get_geometry_reply_t* r= xcb_get_geometry_reply(connection, c, nullptr);
   WH_size_t size= {rect.width, rect.height};
   if( r ) {
     size= {r->width, r->height};
     free(r);
   } else
     debugf("%4d Window xcb_get_geometry error\n", __LINE__);

   if( opt_hcdm ) {
     if( line > 0 )
       debugf("%4d [%d x %d]= get_size\n", line, size.width, size.height);
     else
       debugf("[%u x %u]= get_size\n", size.width, size.height);
   }

   return size;
}

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
     int               y,           // New height
     int               line)        // Caller's line number
{
   int32_t mask= XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT;
   int32_t parm[2]= { x, y };
   synchronously(__LINE__
                , "xcb_configure_window", xcb_configure_window_checked
                ( connection, window_id, mask, parm ) );
   if( opt_hcdm )
     debugf("%4d set_size(%d,%d)\n", line, x, y);
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
       cookie= xcb_get_atom_name(connection, atom);
   xcb_get_atom_name_reply_t*
       reply= xcb_get_atom_name_reply(connection, cookie, nullptr);
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
       cookie= xcb_intern_atom(connection, only, strlen(name), name);
   xcb_intern_atom_reply_t*
       reply= xcb_intern_atom_reply(connection, cookie, nullptr);
   xcb_atom_t result= reply->atom;
   free(reply);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::enqueue
//
// Purpose-
//       Add operation to pending queue
//
//----------------------------------------------------------------------------
void
   Window::enqueue(                 // Add operation to pending queue
     int               line,        // Source line number
     const char*       name,        // Operation name
     xcb_void_cookie_t op)          // Operation cookie
{
   if( opt_hcdm && opt_verbose > 0 )
     traceh("Window(%p)::enqueue(%s)\n", this, name);

   if( penduse >= DIM_PENDING ) {
     debugf("%4d HCDM Window.cpp UNEXPECTED QUEUE FULL EVENT\n", __LINE__);
     flush();
   }

   Pending& pending= this->pending[penduse++];
   pending.opname= name;
   pending.opline= line;
   pending.op=     op;
}

// noqueue does nothing. The response is handled in the reply loop.
void                                // Response handled in reply loop
   Window::noqueue(                 // Drive operation
     int               line,        // Source line number
     const char*       name,        // Operation name
     xcb_void_cookie_t op)          // Operation cookie
{
   if( opt_hcdm && opt_verbose > 0 )
     traceh("Window(%p)::noqueue(%d,%s)\n", this, line, name);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::flush
//
// Purpose-
//       Complete outstanding operations
//
//----------------------------------------------------------------------------
void
   Window::flush( void )            // Flush outstanding operations
{
   if( opt_hcdm )
     debugh("Window(%p)::flush()\n", this);

   for(int i= 0; i<penduse; i++) {  // Complete pending operations
     Pending& pending= this->pending[i];
     synchronously(pending.opline, pending.opname, pending.op);
   }

   penduse= 0;
   if( connection )                 // (Window may be uninitialized)
     xcb_flush(connection);
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

   if( state.visible ) {
     ENQUEUE("xcb_unmap_window", xcb_unmap_window_checked(connection, window_id));
     state.visible= false;
   }
}

void
   Window::show( void )             // Show the Window
{
   if( opt_hcdm )
     debugh("Window(%p)::show Named(%s)\n", this, get_name().c_str());

   if( !state.visible ) {
     ENQUEUE("xcb_map_window", xcb_map_window_checked(connection, window_id));
     state.visible= true;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::synchronously
//
// Purpose-
//       Synchronously handle an XCB operation.
//
//----------------------------------------------------------------------------
void
   Window::synchronously(           // Synchronous XCB operation
     int               line,        // Source line number
     const char*       name,        // The operation name
     xcb_void_cookie_t op)          // The synchronous operation (cookie)
{  xcbcheck(line, name, xcb_request_check(connection, op)); }

void
   Window::synchronously(           // Synchronous XCB operation
     xcb_void_cookie_t op)          // The synchronous operation (cookie)
{  xcbcheck(__LINE__, "synchronously", xcb_request_check(connection, op)); }

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
          "Since STRUCTURE_NOTIFY logic is working, debugging RESIZE_REDIRECT "
          "isn't a high priority.\n"
          );
}
#endif //-- DOCUMENTATION ONLY -----------------------------------------------
}  // namespace xcb
