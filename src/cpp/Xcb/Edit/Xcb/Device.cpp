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
//       Xcb/Device.cpp
//
// Purpose-
//       XCB device driver
//
// Last change date-
//       2020/12/08
//
//----------------------------------------------------------------------------
#include <limits.h>                 // For UINT_MAX
#include <mutex>                    // For mutex, std::lock_guard
#include <string.h>                 // For strlen, ...
#include <unistd.h>                 // For close, ftruncate
#include <X11/Xlib.h>               // For X11 Display type
#include <X11/XKBlib.h>             // For XkbKeycodeToKeysym

#include <pub/Debug.h>              // For Debug object
#include <pub/Trace.h>              // For Trace object
#include <pub/utility.h>            // For pub::utility::clock

#include "Xcb/Device.h"             // Implementation class
#include "Xcb/Global.h"             // For Global data areas and utilities
#include "Xcb/Layout.h"             // For Layout
#include "Xcb/Widget.h"             // For Widget
#include "Xcb/Types.h"              // For enum KEY_STATE, DEV_EVENT_MASK

#include "Xcb/Window.h"             // For Window (Base class)

using pub::Debug;                   // For Debug object
using pub::Trace;                   // For Trace object
using namespace pub::debugging;     // For debugging subroutines
using namespace pub::utility;       // For utility subroutines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
}; // Compilation controls

namespace xcb {
//----------------------------------------------------------------------------
// get_name: Get widget name (or "<nullptr>")
//----------------------------------------------------------------------------
static const char*                  // The widget name or "<nullptr>"
   get_name(                        // Get widget name or "<nullptr>"
     Widget*           widget)      // For this (possible) Widget
{  return widget ? widget->get_name().c_str() : "<nullptr>"; }

//----------------------------------------------------------------------------
//
// Method-
//       Device::Device
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Device::Device( void )           // Constructor
:  Window(nullptr, "Device"), signal("DeviceSignal")
{
   if( opt_hcdm )
     debugh("Device(%p)::Device()\n", this);

   // Connect to the XCB server
   display= XOpenDisplay(nullptr); // For X11 Display
   if( display == nullptr ) {
     const char* disp= getenv("DISPLAY");
     if( disp == nullptr )
       disp= "";
     user_debug("Cannot open DISPLAY(%s)\n", disp);
     exit(EXIT_FAILURE);
   }

   int rc;                          // Working return code
   c= xcb_connect(nullptr, &rc);
   if( xcb_connection_has_error(c) )
     checkstop(__LINE__, "xcb_connect");

   // Get the current screen
   const xcb_setup_t* setup= xcb_get_setup(c);
   xcb_screen_iterator_t iter= xcb_setup_roots_iterator(setup);
   for(; iter.rem; --rc, xcb_screen_next(&iter) ) {
     if( rc == 0 ) {
       s= iter.data;
       break;
     }
   }
   XCBCHECK(s, "xcb_get_screen");

   // Bringup: List atoms (only) =============================================
   if( false ) {                                                            //
     debugf("\nATOMS:\n");                                                  //
     for(xcb_atom_t atom= 1; atom<512; atom++) {                            //
       debugf("[%3d] '%s'\n", atom, atom_to_name(atom).c_str());            //
     }                                                                      //
     operational= false;                                                    //
   } //=======================================================================

   // Pixmap configuration
   device= this;
   window= this;
   parent_id= s->root;
   widget_id= s->root;

   // Initialize Layout geometry
   geom.x= 0;                       // The screen geometry
   geom.y= 0;
   geom.width=  s->width_in_pixels;
   geom.height= s->height_in_pixels;
// rect= geom;                      // The Layout rectangle (Set in configure)
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::~Device
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Device::~Device( void )          // Destructor
{
   if( opt_hcdm )
     debugh("Device(%p)::~Device()\n", this);

   // Clean up
   if( display ) XCloseDisplay(display);
   if( c) xcb_disconnect(c);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Device::configure
//
// Purpose-
//       Configure everything
//
//----------------------------------------------------------------------------
static void
   configure_pixmap(                // Recursively configure Window tree
     Widget*           widget,      // For this Widget
     Device*           device,      // Using this Parent Device and
     Window*           parent)      // Using this Parent Window
{
   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     Pixmap* pixmap= dynamic_cast<Pixmap*>(child);
     if( pixmap ) {
       if( opt_hcdm && opt_verbose > 1 )
         debugh("%4d Device: %s(%p)->configure(%s,%s)\n", __LINE__
               , get_name(pixmap), pixmap, get_name(device), get_name(parent));

       pixmap->configure(device, parent);
       Window* window= dynamic_cast<Window*>(child);
       if( window )
         parent= window;
     }

     configure_pixmap(child, device, parent);
   }
}

static void
   configure_widget(                // Recursively configure Widget tree
     Widget*           widget)      // Below this Widget
{
   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     if( opt_hcdm && opt_verbose > 1 )
       debugf("%4d Device %s(%p)->configure()\n", __LINE__
             , get_name(child), child );
     child->configure();

     configure_widget(child);
   }
}

//----------------------------------------------------------------------------
void
   Device::configure( void )        // Recursively configure everything
{
   if( opt_hcdm )
     debugh("Device(%p)::configure\n", this);

   // Window: Device, Parent, Child configurator
   configure_pixmap(this, this, this);

   // Layout configuration, depth and breadth recursion controlled by Layout.
   Layout::config_t config;
// memset((char*)&config, 0, sizeof(config)); // (Already initialized)
   rect= geom;                      // Initialize rectangle
   Layout::configure(config);
   rect= {20, 20, config.max_size.width, config.max_size.height};

   if( opt_hcdm )
     debug_tree("Device::configure(config_t&)");

   // Widget: configuration
   configure_widget(this);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Device::debug_tree
//
// Purpose-
//       Debug the Device tree
//
//----------------------------------------------------------------------------
static void
   debug_widget_tree(               // Recursively debug Widget tree
     Widget*           widget)      // Starting with and including this Widget
{
   Widget* parent= widget->get_parent();
   Layout* layout= dynamic_cast<Layout*>(widget);
   if( layout ) {
     xcb_rectangle_t& rect= layout->rect;
     debugf("[%4d,%4d,%4u,%4u] ", rect.x, rect.y, rect.width, rect.height);
   } else
     debugf("[----,----,----,----] ");

   debugf("Widget(%.10zx,%6s) Parent(%p,%s)\n"
         , uintptr_t(widget), get_name(widget)
         , parent, get_name(parent) );

   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     debug_widget_tree(child);
   }
}

void
   Device::debug_tree(              // Display the Device tree
     const char*       info)        // Caller information
{
   debugf("Device(%p)::debug_tree(%s)\n", this, info ? info : "");

   debug_widget_tree(this);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Device::draw
//
// Purpose-
//       Draw everything
//
//----------------------------------------------------------------------------
static void
   draw_widget(                     // Recursively draw Widget tree
     Widget*           widget)      // Starting (but not including) this Widget
{
   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     child->draw();
     draw_widget(child);
   }
}

void
   Device::draw( void )          // Recursively draw everything
{
   if( opt_hcdm )
     debugh("Device(%p)::draw\n", this);

   draw_widget(this);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       (::xcb::Device::locate::)locate_pixmap
//       (::xcb::Device::locate::)locate_window
//
// Purpose-
//       Locate associated Pixmap
//       Locate associated Window
//
//----------------------------------------------------------------------------
static Pixmap*                      // Resultant Pixmap/Window
   locate_pixmap(                   // Lock wrapper
     xcb_drawable_t    target,      // This XCB pixmap/window
     Widget*           widget)      // Below this widget
{
   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     Pixmap* pixmap= dynamic_cast<Pixmap*>(child);
     if( pixmap && pixmap->widget_id == target )
       return pixmap;
   }

   for(Widget* child= widget->get_first(); child; child= child->get_next()) {
     Pixmap* pixmap= locate_pixmap(target, child);
     if( pixmap )
       return pixmap;
   }

   return nullptr;
}

static Window*                      // Resultant Window
   locate_window(                   // Locate Window
     xcb_drawable_t    target,      // This XCB pixmap/window
     Device*           device)      // For this Device
{  std::lock_guard<decltype(*device)> lock(*device);

   if( target == device->widget_id ) // If looking for the Device drawable
     return device;                 // We found it

   Pixmap* pixmap= locate_pixmap(target, device);
   Window* window= dynamic_cast<Window*>(pixmap);
   if( window )
     return window;

   debugf("%4d Device: No Window(%u)\n", __LINE__, target);
   throw "Device/Window mismatch";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Device::locate
//
// Purpose-
//       Locate Pixmap/Window (using breadth-first search)
//
//----------------------------------------------------------------------------
Pixmap*                             // The Pixmap/Window, if located
   Device::locate(                  // Recursively locate
     xcb_drawable_t    target)      // This XCB window/pixmap identifier
{
   if( opt_hcdm && opt_verbose > 3 )
     debugh("Device::locate(%u)\n", target);

   if( target == widget_id )        // If looking for the Device drawable
     return this;                   // We found it

   std::lock_guard<decltype(*device)> lock(*device);
   return locate_pixmap(target, device);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Device::to_keysym
//
// Purpose-
//       Extract xcb_keysym_t from keypress event
//
//----------------------------------------------------------------------------
xcb_keysym_t                        // The associated keyboard symbol
   Device::to_keysym(               // Convert to xcb_keysym_t
     const xcb_key_press_event_t*
                       event) const // This key press (or release) event
{
   int state= event->state;         // Working state
   if( state & KS_LOCK )            // If caps lock in effect
     state ^= KS_SHIFT;             // Invert the shift state
   state &= KS_SHIFT;               // Only interested in key::KS_SHIFT state

// // Convert defined ASCII symbols in the range 0x00..0x1f
// // No keysym values for ff00-ff07,ff0c,ff0e-ff0f
// if( (key >= 0xff00 && key <= 0xff0f) || key == 0xff1b )
//   key &= 0x001f;

   return (xcb_keysym_t)
          XkbKeycodeToKeysym(display, event->detail, 0, bool(state) );
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::run
//
// Purpose-
//       Handle events
//
//----------------------------------------------------------------------------
void
   Device::run( void )              // Handle window events
{
   typedef ::pub::Trace::Record Record;
   int run_hcdm= opt_hcdm | HCDM;
// run_hcdm= true;

   while( operational ) {
     xcb_generic_event_t* e= xcb_wait_for_event(c);
     if( e ) {
       // Trace XCB event
       if( opt_verbose > -2 ) {     // (If not forbidden)
         Record* record= (Record*)Trace::storage_if(sizeof(Record));
         if( record ) {
           record->unit= *((uint32_t*)e); // type(8), extension(8), sequence(16)
           memcpy(record->value, (char*)e + 4, sizeof(record->value));
           record->trace(".XCB");
         }
       }

       switch(e->response_type & 0x7f)
       {
         case 0: {                  // If X11 error
           // X11 error codes are defined in /usr/include/X11/X.h
           xcb_generic_error_t* et= (xcb_generic_error_t*)e;
           xcberror(et);
           break;
         }
         // case 1:                 // X11 reply (handled by default:)
         case XCB_BUTTON_PRESS: {
           xcb_button_press_event_t* et= (xcb_button_press_event_t*)e;
           if( run_hcdm ) debugf("DEV.BUTTON_PRESS\n");
           locate_window(et->event, this)->button_press(et);
           break;
         }
         case XCB_BUTTON_RELEASE: {
           xcb_button_release_event_t* et= (xcb_button_release_event_t*)e;
           if( run_hcdm ) debugf("DEV.BUTTON_RELEASE\n");
           locate_window(et->event, this)->button_release(et);
           break;
         }
         case XCB_CIRCULATE_NOTIFY: {
           xcb_circulate_notify_event_t* et= (xcb_circulate_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.CIRCULATE_NOTIFY\n");
           locate_window(et->event, this)->circulate_notify(et);
           break;
         }
         case XCB_CIRCULATE_REQUEST: {
           xcb_circulate_request_event_t* et= (xcb_circulate_request_event_t*)e;
           if( run_hcdm ) debugf("DEV.CIRCULATE_REQUEST\n");
           locate_window(et->event, this)->circulate_request(et);
           break;
         }
         case XCB_CLIENT_MESSAGE: {
           xcb_client_message_event_t* et= (xcb_client_message_event_t*)e;
           if( run_hcdm ) debugf("DEV.CLIENT_MESSAGE type(%d) data(%d)\n"
                                , et->type, et->data.data32[0]);
           locate_window(et->window, this)->client_message(et);
           break;
         }
         case XCB_COLORMAP_NOTIFY: {
           xcb_colormap_notify_event_t* et= (xcb_colormap_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.COLORMAP_NOTIFY\n");
           locate_window(et->window, this)->colormap_notify(et);
           break;
         }
         case XCB_CONFIGURE_NOTIFY: {
           xcb_configure_notify_event_t* et= (xcb_configure_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.CONFIGURE_NOTIFY\n");
           locate_window(et->event, this)->configure_notify(et);
           break;
         }
         case XCB_CONFIGURE_REQUEST: {
           xcb_configure_request_event_t* et= (xcb_configure_request_event_t*)e;
           if( run_hcdm ) debugf("DEV.CONFIGURE_REQUEST\n");
           locate_window(et->window, this)->configure_request(et);
           break;
         }
         case XCB_CREATE_NOTIFY: {
           xcb_create_notify_event_t* et= (xcb_create_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.CREATE_NOTIFY\n");
           locate_window(et->window, this)->create_notify(et);
           break;
         }
         case XCB_DESTROY_NOTIFY: {
           xcb_destroy_notify_event_t* et= (xcb_destroy_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.DESTROY_NOTIFY\n");
           locate_window(et->window, this)->destroy_notify(et);
           break;
         }
         case XCB_ENTER_NOTIFY: {
           xcb_enter_notify_event_t* et= (xcb_enter_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.ENTER_NOTIFY\n");
           locate_window(et->event, this)->enter_notify(et);
           break;
         }
         case XCB_EXPOSE: {
           xcb_expose_event_t* et= (xcb_expose_event_t*)e;
           if( run_hcdm ) debugf("DEV.EXPOSE %d [%d,%d,%u,%u]\n", et->window
                                , et->x, et->y, et->width, et->height);
           locate_window(et->window, this)->expose(et);
           break;
         }
         case XCB_FOCUS_IN: {
           xcb_focus_in_event_t* et= (xcb_focus_in_event_t*)e;
           if( run_hcdm ) debugf("DEV.FOCUS_IN\n");
           locate_window(et->event, this)->focus_in(et);
           break;
         }
         case XCB_FOCUS_OUT: {
           xcb_focus_out_event_t* et= (xcb_focus_out_event_t*)e;
           if( run_hcdm ) debugf("DEV.FOCUS_OUT\n");
           locate_window(et->event, this)->focus_out(et);
           break;
         }
         case XCB_GE_GENERIC: {
           xcb_ge_generic_event_t* et= (xcb_ge_generic_event_t*)e;
           if( run_hcdm ) debugf("DEV.GE_GENERIC %d\n", et->event_type);
           DeviceEvent event(e);
           signal.signal(event);
           break;
         }
         case XCB_GRAPHICS_EXPOSURE: {
           xcb_graphics_exposure_event_t* et= (xcb_graphics_exposure_event_t*)e;
           if( run_hcdm )
             debugf("DEV.GRAPHICS_EXPOSURE %d [%d,%d,%u,%u]\n", et->drawable
                   , et->x, et->y, et->width, et->height);
           locate_pixmap(et->drawable, this)->graphics_exposure(et);
           break;
         }
         case XCB_GRAVITY_NOTIFY: {
           xcb_gravity_notify_event_t* et= (xcb_gravity_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.GRAVITY_NOTIFY\n");
           locate_window(et->event, this)->gravity_notify(et);
           break;
         }
         case XCB_KEY_PRESS: {
           xcb_key_press_event_t* et= (xcb_key_press_event_t*)e;
           if( run_hcdm ) debugf("DEV.KEY_PRESS\n");
           locate_window(et->event, this)->key_press(et);
           break;
         }
         case XCB_KEY_RELEASE: {
           xcb_key_release_event_t* et= (xcb_key_release_event_t*)e;
           if( run_hcdm ) debugf("DEV.KEY_RELEASE\n");
           locate_window(et->event, this)->key_release(et);
           break;
         }
         case XCB_KEYMAP_NOTIFY: {
           xcb_keymap_notify_event_t* et= (xcb_keymap_notify_event_t*)e;
           if( run_hcdm ) {
             debugf("DEV.KEYMAP_NOTIFY\n");
             if( opt_verbose > 4 )
               dump(et, sizeof(xcb_keymap_notify_event_t));
           }
           DeviceEvent event(e);
           signal.signal(event);
           break;
         }
         case XCB_LEAVE_NOTIFY: {
           xcb_leave_notify_event_t* et= (xcb_leave_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.LEAVE_NOTIFY\n");
           locate_window(et->event, this)->leave_notify(et);
           break;
         }
         case XCB_MAP_NOTIFY: {
           xcb_map_notify_event_t* et= (xcb_map_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.MAP_NOTIFY event(%u) window(%u) %d\n"
                                , et->event, et->window, et->override_redirect );
           locate_window(et->event, this)->map_notify(et);
           break;
         }
         case XCB_MAP_REQUEST: {
           xcb_map_request_event_t* et= (xcb_map_request_event_t*)e;
           if( run_hcdm ) debugf("DEV.MAP_REQUEST\n");
           locate_window(et->window, this)->map_request(et);
           break;
         }
         case XCB_MAPPING_NOTIFY: {
////       xcb_mapping_notify_event_t* et= (xcb_mapping_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.MAPPING_NOTIFY\n");
           DeviceEvent event(e);
           signal.signal(event);
           break;
         }
         case XCB_MOTION_NOTIFY: {
           xcb_motion_notify_event_t* et= (xcb_motion_notify_event_t*)e;
           if( run_hcdm && opt_verbose >= 0) debugf("DEV.MOTION_NOTIFY\n");
           locate_window(et->event, this)->motion_notify(et);
           break;
         }
         case XCB_NO_EXPOSURE: {
           xcb_no_exposure_event_t* et= (xcb_no_exposure_event_t*)e;
           if( run_hcdm ) debugf("DEV.NO_EXPOSURE(%d,%d) DEV\n"
                                , et->major_opcode, et->minor_opcode);
           locate_pixmap(et->drawable, this)->no_exposure(et);
           break;
         }
         case XCB_PROPERTY_NOTIFY: {
           xcb_property_notify_event_t* et= (xcb_property_notify_event_t*)e;
           Window* window= locate_window(et->window, this);
           if( run_hcdm ) {
             debugf("DEV.PROPERTY_NOTIFY atom(%3d) time(%d) state(0x%.2x) '%s'\n"
                   , et->atom, et->time, et->state
                   , window->atom_to_name(et->atom).c_str());
             if( opt_verbose > 4 )
               pub::utility::dump(et, sizeof(*et));
           }
           window->property_notify(et);
           break;
         }
         case XCB_REPARENT_NOTIFY: {
           xcb_reparent_notify_event_t* et= (xcb_reparent_notify_event_t*)e;
           if( run_hcdm )
             debugf("DEV.REPARENT_NOTIFY event(%u) window(%u) parent(%u) %d\n"
                   , et->event, et->window, et->parent, et->override_redirect);
           locate_window(et->event, this)->reparent_notify(et);
           break;
         }
         case XCB_RESIZE_REQUEST: {
           xcb_resize_request_event_t* et= (xcb_resize_request_event_t*)e;
           if( run_hcdm ) debugf("DEV.RESIZE_REQUEST\n");
           locate_window(et->window, this)->resize_request(et);
           break;
         }
         case XCB_SELECTION_CLEAR: {
           xcb_selection_clear_event_t* et= (xcb_selection_clear_event_t*)e;
           if( run_hcdm ) debugf("DEV.SELECTION_CLEAR\n");
           locate_window(et->owner, this)->selection_clear(et);
           break;
         }
         case XCB_SELECTION_NOTIFY: {
           xcb_selection_notify_event_t* et= (xcb_selection_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.SELECTION_NOTIFY\n");
           locate_window(et->requestor, this)->selection_notify(et);
           break;
         }
         case XCB_SELECTION_REQUEST: {
           xcb_selection_request_event_t* et= (xcb_selection_request_event_t*)e;
           if( run_hcdm ) debugf("DEV.SELECTION_REQUEST\n");
           locate_window(et->owner, this)->selection_request(et);
           break;
         }
         case XCB_UNMAP_NOTIFY: {
           xcb_unmap_notify_event_t* et= (xcb_unmap_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.UNMAP_NOTIFY event(%u) window(%u) %d\n"
                                , et->event, et->window, et->from_configure);
           locate_window(et->event, this)->unmap_notify(et);
           break;
         }
         case XCB_VISIBILITY_NOTIFY: {
           xcb_visibility_notify_event_t* et= (xcb_visibility_notify_event_t*)e;
           if( run_hcdm ) debugf("DEV.VISIBILITY_NOTIFY 0x%.2x\n", et->state);
           locate_window(et->window, this)->visibility_notify(et);
           break;
         }
         default:
           // case 1: xcb_generic_reply NOT EXPECTED but handled here.
           debugh("Event(%.2u) NOT HANDLED\n", e->response_type & 0x007f);
           dump(e, sizeof(xcb_generic_event_t));
           break;
       }

       free(e);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::join
//
// Purpose-
//       Wait for Device Thread completion
//
//----------------------------------------------------------------------------
void
   Device::join( void )             // Join the (non-existent) Thread
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Device::start
//
// Purpose-
//       Drive the Device Thread
//
//----------------------------------------------------------------------------
void
   Device::start( void )            // Start the (non-existent) Thread
{  run(); }
}  // namespace xcb
