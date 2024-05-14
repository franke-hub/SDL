//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdInps.cpp
//
// Purpose-
//       Editor: Implement EdInps.h: Terminal keyboard and mouse handlers.
//
// Last change date-
//       2024/05/07
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Global.h>             // For gui::opt_* controls
#include <gui/Keysym.h>             // For X11/keysymdef
#include <gui/Types.h>              // For gui::DEV_EVENT_MASK
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Utf.h>                // For pub::Utf classes

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps, implemented
#include "EdMark.h"                 // For EdMark
#include "EdOuts.h"                 // For EdOuts, subclass

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

typedef pub::Utf::utf8_t   utf8_t;  // Import utf8_t
typedef pub::Utf::utf16_t  utf16_t; // Import utf16_t
typedef pub::Utf::utf32_t  utf32_t; // Import utf32_t

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  KP_MAX= 0xffbf                   // Keypad maximum key value
,  KP_MIN= 0xff80                   // Keypad minimum key value

,  USE_GRAB_MOUSE= true             // When starting, position the mouse
}; // Compilation controls

enum // Key definitions
{  KEY_ESC= '\x1B'                  // ESCape key
,  KEY_TAB= '\t'                    // TAB key
}; // Key definitions

//----------------------------------------------------------------------------
// Imports
//----------------------------------------------------------------------------
enum // Imported
{  KS_ALT= EdUnit::KS_ALT
,  KS_CTL= EdUnit::KS_CTL
}; // Imported

//----------------------------------------------------------------------------
// Keypad conversion table (Dependent upon /usr/include/X11/keysymdef.h)
//----------------------------------------------------------------------------
static unsigned short  kp_num[64]=  // Keypad conversion table, numlock on
{     ' ', 0xff81, 0xff82, 0xff83, 0xff84, 0xff85, 0xff86, 0xff87 // 0xff80 ..
,  0xff88, 0xff89, 0xff8a, 0xff8b, 0xff8c, 0xff0d, 0xff8e, 0xff8f
,  0xff90, 0xff91, 0xff92, 0xff93, 0xff94,    '7',    '4',    '8'
,     '6',    '2',    '9',    '3',    '1',    '5',    '0',    '.' // 0xff98 ..
,  0xffa0, 0xffa1, 0xffa2, 0xffa3, 0xffa4, 0xffa5, 0xffa6, 0xffa7
,  0xffa8, 0xffa9,    '*',    '+', 0xffac,    '-', 0xffae,    '/'
,     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7' // 0xffb0 ..
,     '8',    '9', 0xffba, 0xffbb, 0xffbc,    '=', 0xffbe, 0xffbf // 0xffb8 ..
}; // kp_num

static unsigned short  kp_off[64]=  // Keypad conversion table, numlock off
{  0xff80, 0xff81, 0xff82, 0xff83, 0xff84, 0xff85, 0xff86, 0xff87 // 0xff80 ..
,  0xff88, 0xff89, 0xff8a, 0xff8b, 0xff8c, 0xff0d, 0xff8e, 0xff8f
,  0xff90, 0xff91, 0xff92, 0xff93, 0xff94, 0xff50, 0xff51, 0xff52
,  0xff53, 0xff54, 0xff55, 0xff56, 0xff57, 0xff58, 0xff63, 0xffff // 0xff98 ..
,  0xffa0, 0xffa1, 0xffa2, 0xffa3, 0xffa4, 0xffa5, 0xffa6, 0xffa7
,  0xffa8, 0xffa9,    '*',    '+', 0xffac,    '-', 0xffae,    '/'
,     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7' // 0xffb0 ..
,     '8',    '9', 0xffba, 0xffbb, 0xffbc,    '=', 0xffbe, 0xffbf // 0xffb8 ..
}; // kp_off

// (Statically) verify kp_num and kp_off definitions
static_assert(0xff80 == XK_KP_Space     && 0xffbf == XK_F2
           && 0xff8d == XK_KP_Enter     && 0xff0d == XK_Return
           && 0xff95 == XK_KP_Home      && 0xff50 == XK_Home
           && 0xff96 == XK_KP_Left      && 0xff51 == XK_Left
           && 0xff97 == XK_KP_Up        && 0xff52 == XK_Up
           && 0xff98 == XK_KP_Right     && 0xff53 == XK_Right
           && 0xff99 == XK_KP_Down      && 0xff54 == XK_Down
           && 0xff9a == XK_KP_Page_Up   && 0xff55 == XK_Page_Up
           && 0xff9b == XK_KP_Page_Down && 0xff56 == XK_Page_Down
           && 0xff9c == XK_KP_End       && 0xff57 == XK_End
           && 0xff9d == XK_KP_Begin     && 0xff58 == XK_Begin
           && 0xff9e == XK_KP_Insert    && 0xff63 == XK_Insert
           && 0xff9f == XK_KP_Delete    && 0xffff == XK_Delete
             , "/usr/include/X11/keysymdefs mismatch");

//----------------------------------------------------------------------------
//
// Subroutine-
//       key_to_name
//
// Purpose-
//       Convert xcb_keysym_t to its name.
//
//----------------------------------------------------------------------------
static const char*                  // The name of the key
   key_to_name(xcb_keysym_t key)    // Convert xcb_keysym_t to name
{
   static char buffer[16];          // (Static) return buffer
   static const char* F_KEY= "123456789ABCDEF";

   if( key >= 0x0020 && key <= 0x007f ) { // If text key
     buffer[0]= char(key);
     buffer[1]= '\0';
     return buffer;
   }

   if( key >= XK_F1 && key <= XK_F12 ) { // If function key
     buffer[0]= 'F';
     buffer[1]= F_KEY[key - XK_F1];
     buffer[2]= '\0';
     return buffer;
   }

   switch( key ) {                  // Handle control key
     case XK_ISO_Left_Tab:
       return "Left tab";

     case XK_BackSpace:
       return "BackSpace";

     case XK_Tab:
       return "Tab";

     case XK_Return:
       return "Return";

     case XK_Pause:
       return "Pause";

     case XK_Scroll_Lock:
       return "Scroll lock";

     case XK_Escape:
       return "Escape";

     case XK_Delete:
       return "Delete";

     case XK_Insert:
       return "Insert";

     case XK_Num_Lock:
       return "Num lock";

     case XK_Home:
       return "Home";

     case XK_End:
       return "End";

     case XK_Menu:
       return "Menu";

     case XK_Break:
       return "Break";

     case XK_Left:
       return "Left arrow";

     case XK_Up:
       return "Up arrow";

     case XK_Right:
       return "Right arrow";

     case XK_Down:
       return "Down arrow";

     case XK_Page_Up:
       return "Page up";

     case XK_Page_Down:
       return "Page down";

     case XK_Shift_L:
     case XK_Shift_R:
       return "Shift";

     case XK_Alt_L:
     case XK_Alt_R:
       return "Alt";

     case XK_Control_L:
     case XK_Control_R:
       return "Ctrl";

     default:
       break;
   }

   sprintf(buffer, "0x%.2x", key);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_text_key
//
// Purpose-
//       Is the key a text key?
//
//----------------------------------------------------------------------------
static bool
   is_text_key(                     // Is key a text key?
     xcb_keysym_t      key,         // Input key
     int               state)       // ESC + Alt/Ctl/Shift state mask
{
   if( state & EdUnit::KS_ESC ) {   // If escape mode
     if( key == '\b' || key == '\t' || key == KEY_ESC )
       return true;
   }

   if( key >= 0x0020 && key < 0x007F ) // If standard text key
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_protected_key
//
// Purpose-
//       Determine whether a keypress is allowed for a protected line.
//
// Implementation notes-
//       Copy and move operations have additional protections
//
//----------------------------------------------------------------------------
static int                          // Return code, TRUE if error message
   is_protected_key(                // Is keypress protected
     xcb_keysym_t      key,         // Input key
     int               state)       // ESC + Alt/Ctl/Shift state mask
{
   if( is_text_key(key, state) ) {  // If text key
     int mask= state & (KS_ALT | KS_CTL);

     if( mask ) {
       key= toupper(key);
       if( mask == KS_ALT ) {
         switch(key) {              // Allowed keys:
           case 'C':                // COPY MARK
           case 'D':                // DELETE MARK
           case 'I':                // INSERT
           case 'M':                // MOVE MARK
           case 'Q':                // QUIT (safe)
           case 'U':                // UNDO MARK
             return false;

           default:
             break;
         }
       } else if( mask == KS_CTL ) {
         switch(key) {              // Allowed keys:
           case 'C':                // STASH MARK
           case 'Q':                // QUIT (safe)
           case 'S':                // SAVE
           case 'V':                // PASTE STASH
           case 'X':                // CUT and STASH MARK
           case 'Y':                // REDO
           case 'Z':                // UNDO
             return false;

           default:
             break;
         }
       }
     }
   } else {                         // If action key
     switch( key ) {                // Check for disallowed keys
//     case '\b':                   // (Backspace, does not occur
       case XK_BackSpace:           // (Backspace)
       case 0x007F:                 // (DELete encoding)
       case XK_Delete:              // (Delete)
         break;

       default:                     // All others allowed
         return false;
     }
   }

   editor::put_message("Protected");
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::EdInps
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdInps::EdInps(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  EdUnit(), Window(parent, name ? name : "EdInps")
{
   if( opt_hcdm )
     debugh("EdInps(%p)::EdInps\n", this);

   // Set GUI (static) debugging control options
   gui::opt_hcdm= opt_hcdm;
   gui::opt_verbose= opt_verbose;

   // Allocate GUI units
   gui::Device* device= static_cast<gui::Device*>(get_parent());
   font=   new gui::Font(device);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::~EdInps
//
// Purpose-
//       Destructor
//
// Implementation notes-
//       Graphic contexts, editor::data/hist/mark created in method start
//
//----------------------------------------------------------------------------
   EdInps::~EdInps( void )          // Destructor
{  if( opt_hcdm ) debugh("EdInps(%p)::~EdInps\n", this);

   // Delete Graphic Contexts
   if( gc_flip ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_flip) );
   if( gc_font ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_font) );
   if( gc_mark ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_mark) );
   if( bg_chg  ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, bg_chg ) );
   if( bg_sts  ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, bg_sts ) );
   if( gc_chg  ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_chg ) );
   if( gc_msg  ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_msg ) );
   if( gc_sts  ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_sts ) );
   flush();

   // Delete controlled objects
   delete editor::data;
   delete editor::hist;
   delete editor::mark;

   editor::data= nullptr;
   editor::hist= nullptr;
   editor::mark= nullptr;
   editor::view= nullptr;

   // Delete GUI objects
   // This (EdUnit derived) object must be deleted before device deletion.
   // (That's why static methods EdUnit::Init::initialize/terminate exist.)
   // EdUnit::Init::terminate deletes this object, then the device.
   delete font;                     // (No reason not to delete the font now)
   font= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdInps::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdInps(%p)::debug(%s) Named(%s)\n", this, info ? info : ""
         , get_name().c_str());

   debugf("..head(%p) tail(%p) col_size(%u) row_size(%u) row_used(%u)\n"
         , head, tail, col_size, row_size, row_used);
   debugf("..motion(%d,%d,%d,%d)\n", motion.state, motion.time
         , motion.x, motion.y);
   debugf("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n", gc_font, gc_flip, gc_mark);
   debugf("..gc_chg(%u) gc_msg(%u) gc_sts(%u)\n"
         , gc_chg, gc_msg, gc_sts);
   debugf("..protocol(%u) wm_close(%u)\n", protocol, wm_close);
   Window::debug(info);
   debugf("\n..font:\n");
   font->debug(info);
}

//----------------------------------------------------------------------------
//
// Pseudo-thread methods-
//       EdInps::start
//       EdInps::stop
//       EdInps::join
//
// Purpose-
//       Start the editor
//       Stop the editor
//       Wait for editor completion
//
// Purpose-
//       Thread simulation methods
//
//----------------------------------------------------------------------------
void
   EdInps::start( void )
{  if( opt_hcdm ) debugh("EdInps(%p)::start\n", this);

   // Initialize the configuration
   gui::Device* device= static_cast<gui::Device*>(get_parent());
   // device->insert(this);
   device->configure();             // Configure the gui::Window

   // Create the graphic contexts
   gc_font= font->makeGC(fg, bg);   // (The default)
   gc_flip= font->makeGC(bg, fg);   // (Inverted)
   gc_mark= font->makeGC(mark_fg,    mark_bg);
   bg_chg=  font->makeGC(change_bg,  change_bg);
   bg_sts=  font->makeGC(status_bg,  status_bg);
   gc_chg=  font->makeGC(change_fg,  change_bg);
   gc_msg=  font->makeGC(message_fg, message_bg);
   gc_sts=  font->makeGC(status_fg,  status_bg);

   // EdData and EdHist require initialized graphic contexts.
   // That doesn't happen until the gui::Window is configured.
   EdData* data= new EdData();      // Data view
   EdHist* hist= new EdHist();      // History view
   EdMark* mark= new EdMark();      // Mark handler

   editor::data= data;
   editor::hist= hist;
   editor::mark= mark;
   editor::view= hist;              // (Initial view)

   // Configure views
   data->gc_flip= gc_flip;
   data->gc_font= gc_font;
   data->gc_mark= gc_mark;

   hist->gc_chg=  gc_chg;
   hist->gc_sts=  gc_sts;

   // Set initial file
   activate(editor::file_list.get_head());

   // Start the Device
   device->draw();
   show();                          // (move_window fails unless visible)
   if( geom.x || geom.y )           // If position specified
     move_window(geom.x, geom.y);

#ifdef _OS_CYGWIN // - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Grabbing the mouse is not a recommeded practice, but when using Cygwin
   // leaving the mouse outside the window and hitting escape multiple times
   // (because it's not working) locks the terminal. It only unlocks after
   // control-C (or multiple control-C's) and about a 5 second delay.
   // This has become quite annoying, grabbing the mouse helps (but doesn't
   // eliminate) the problem.
   if( USE_GRAB_MOUSE )             // (Optionally)
     grab_mouse();                  // Position the mouse inside the window
#endif //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

   flush();
   device->run();                   // Run the polling loop
}

void
   EdInps::stop( void )
{  if( opt_hcdm ) debugh("EdInps(%p)::stop\n", this);

   gui::Device* device= static_cast<gui::Device*>(get_parent());
   device->operational= false;
}

void
   EdInps::join( void )
{  if( opt_hcdm ) debugh("EdInps(%p)::join\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::key_alt
//
// Purpose-
//       Handle alt-key event
//
//----------------------------------------------------------------------------
void
   EdInps::key_alt(                 // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   switch(key) {                    // ALT-
     case 'B': {                    // Mark block
       op_mark_block();
       break;
     }
     case 'C': {                    // Copy the mark
       op_mark_copy();
       break;
     }
     case 'D': {                    // Delete the mark
       op_mark_delete();
       break;
     }
     case 'J': {                    // Join lines
       op_join_line();
       break;
     }
     case 'I': {                    // Insert line
       op_insert_line();
       break;
     }
     case 'L': {                    // Mark line
       op_mark_line();
       break;
     }
     case 'M': {                    // Move mark (Uses cut/paste)
       op_mark_move();
       break;
     }
     case 'P': {                    // Format paragraph
       op_mark_format();
       break;
     }
     case 'S': {                    // Split line
       op_split_line();
       break;
     }
     case 'U': {                    // Undo mark
       op_mark_undo();
       break;
     }
     case '\\': {                   // Escape
       key_state |= KS_ESC;
       break;
     }
     default:
       editor::put_message("Invalid key");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::key_ctl
//
// Purpose-
//       Handle ctl-key event
//
//----------------------------------------------------------------------------
void
   EdInps::key_ctl(                 // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   switch(key) {                    // CTRL-
     case 'C': {                    // Copy the mark into the stash
       op_mark_stash();
       break;
     }
     case 'Q': {                    // Quit
       op_safe_quit();
       break;
     }
     case 'S': {                    // Save
       op_save();
       break;
     }
     case 'V': {                    // Paste (the stash)
       op_mark_paste();
       break;
     }
     case 'X': {                    // Cut the mark
       op_mark_cut();
       break;
     }
     case 'Y': {                    // REDO
       op_redo();
       break;
     }
     case 'Z': {                    // UNDO
       op_undo();
       break;
     }
     default:
       op_key_dead();
   }
}


//----------------------------------------------------------------------------
//
// Method-
//       Window event handler methods
//
// Purpose-
//       Window *_event_t handlers
//
//----------------------------------------------------------------------------
void
   EdInps::button_press(            // Handle this
     xcb_button_press_event_t* event) // Button press event
{
   EdData* const data= editor::data;
   EdFile* const file= editor::file;
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   // Use E.detail and gui::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm && opt_verbose > 0 )
     debugh("button:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t current_col= view->get_column(); // The current column number
   unsigned button_row= get_row(E.event_y); // (Absolute) button row

   switch( E.detail ) {
     case gui::BT_LEFT: {           // Left button
       if( button_row < USER_TOP ) { // If on top of screen
         if( !file->rem_message() ) { // If no message removed or remains
           if( view == hist )       // If history active
             move_cursor_H(hist->col_zero + get_col(E.event_x)); // Update column
           else
             hist->activate();
         }
         draw_top();
         break;
       }

       // Button press is on data screen
       if( view == hist ) {         // If history active
         data->activate();
         draw_top();
       }

       if( button_row != data->row ) { // If row changed
         if( button_row > row_used ) // (Button should not cause scroll up)
           button_row= row_used;
         data->move_cursor_V(button_row - data->row); // Set new row
       }
       move_cursor_H(data->col_zero + get_col(E.event_x)); // Set new column
       break;
     }
     case gui::BT_RIGHT: {          // Right button
       if( button_row < USER_TOP ) { // If on top of screen
         if( file->rem_message() ) { // If message removed
           draw_top();
           break;
         }

         // Invert the view
         editor::do_view();
       }
       break;
     }
     case gui::WT_PUSH:             // Mouse wheel push (away)
       move_screen_V(-3);
       break;

     case gui::WT_PULL:             // Mouse wheel pull (toward)
       move_screen_V(+3);
       break;

     case gui::WT_LEFT:             // Mouse wheel left
       move_cursor_H(current_col > 3 ? current_col - 3 : 0);
       break;

     case gui::WT_RIGHT:            // Mouse wheel right
       move_cursor_H(current_col + 3);
       break;

     case gui::BT_CNTR:             // Middle button (ignored)
     default:                       // (Buttons 6 and 7 undefined)
       break;
   }
}

void
   EdInps::client_message(          // Handle this
     xcb_client_message_event_t* E) // Client message event
{
   if( opt_hcdm )
     debugh("message: type(%u) data(%u)\n", E->type, E->data.data32[0]);

   if( E->type == protocol && E->data.data32[0] == wm_close )
     stop();                        // Unconditional terminate
}

void
   EdInps::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("configure_notify(%d,%d) window(%x)\n"
           , E->width, E->height, E->window);

   // (Ignore anything other than a window size change, e.g. window movement)
   if( rect.width != E->width || rect.height != E->height )
     resized(E->width, E->height);
}

void
   EdInps::expose(                  // Handle this
     xcb_expose_event_t* E)         // Expose event
{
   if( opt_hcdm )
     debugh("expose(%x) %d [%d,%d,%d,%d]\n"
           , E->window, E->count, E->x, E->y, E->width, E->height);

   draw();
}

void
   EdInps::focus_in(                // Handle this
     xcb_focus_in_event_t* E)       // Focus-in event
{  using namespace editor;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("gain focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   if( !(view == hist && file->mess_list.get_head()) ) {
     show_cursor();
     flush();
   }
}

void
   EdInps::focus_out(               // Handle this
     xcb_focus_out_event_t* E)      // Focus-out event
{  using namespace editor;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("lost focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   if( !(view == hist && file->mess_list.get_head()) ) {
     hide_cursor();
     flush();
   }
}

void
   EdInps::motion_notify(           // Handle this
     xcb_motion_notify_event_t* E)  // Motion notify event
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("motion: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   if( E->event_x != motion.x || E->event_y != motion.y ) {
     show_mouse();
   } else {
     if( (E->time - motion.time) < 1000 ) // If less than 1 second idle
       return;                      // Ignore
     if( config::USE_MOUSE_HIDE )
       hide_mouse();
   }

   motion.time= E->time;
   motion.x= E->event_x;
   motion.y= E->event_y;
}

// EdInps::property_notify does nothing except write a debugging message.
void
   EdInps::property_notify(         // Handle this
     xcb_property_notify_event_t* E) // Property notify event
{
   if( opt_hcdm )
     debugh("property_notify: window(%x) atom(%x,%s) state(%d)\n"
           , E->window, E->atom, atom_to_name(E->atom).c_str(), state);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::key_input
//
// Purpose-
//       Input key handler
//
//----------------------------------------------------------------------------
void
   EdInps::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               gui_state)   // GUI state mask
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("EdInps(%p)::key_input(0x%.4X,0x%.8X) '%s%s%s'\n", this
           , key, state
           , (gui_state & gui::KS_ALT) ? "ALT-" : ""
           , (gui_state & gui::KS_CTRL) ? "CTL-" : ""
           , key_to_name(key)
     );

   EdData* const data= editor::data;
   EdFile* const file= editor::file;
   EdView* const view= editor::view;

   // Diagnostics
   const char* key_name= key_to_name(key);
   Trace::trace(".KEY", (state | key), key_name);

   // Translate gui_state
   state &= ~(KS_LOGIC | KS_ALT | KS_CTL);
   if( gui_state & gui::KS_ALT )
     state |= KS_ALT;
   if( gui_state & gui::KS_CTRL )
     state |= KS_CTL;

   if( key >= KP_MIN && key <= KP_MAX ) { // Convert Keypads to standard keys
     unsigned short* kp_tab= kp_off;
     if( gui_state & gui::KS_NUML )
       kp_tab= kp_num;
     key= kp_tab[key - KP_MIN];
   }

   if( state & KS_ESC ) {           // Escaped characters
     if( key == XK_BackSpace || key == XK_Tab || key == XK_Escape )
       key &= 0x00FF;               // Keys "cleverly chosen to map to ASCII"
   }

   // Handle protected line
   if( view == data ) {             // Protection only applies to data view
     if( data->cursor->flags & EdLine::F_PROT // If protected line
         && is_protected_key(key, state) ) // And modification key
       return;                      // (Disallowed)
   }

   // Handle message completion, removing informational messages.
   file->rem_message_type();        // Remove current informational message
   if( draw_message() )             // If another message is present
     return;                        // (Return, ignoring the current key)

   if( key_state & (KS_MSG | KS_NFC) ) { // If a message completed
     key_state &= ~(KS_MSG);        // (KS_NFC removed later)
     draw_history();
   }

   // Handle input key
   size_t column= view->get_column(); // The cursor column
   if( is_text_key(key, state) ) {  // If text key
     int mask= state & (KS_ALT | KS_CTL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case KS_ALT:
           key_alt(key);
           break;

         case KS_CTL:
           key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTL)
           op_key_dead();
       }
       return;
     }

     if( editor::data_protected() )
       return;

     if( key_state & KS_INS ) {     // If Insert state
       view->active.insert_char(column, key);
       if( move_cursor_H(column + 1) )
         view->draw_active();
     } else {
       view->active.replace_char(column, key);
       move_cursor_H(column + 1);
     }
     draw_top();
     show_cursor();
     flush();

     // Escape complete; "No File Changed" message complete
     key_state &= ~(KS_ESC | KS_NFC);
     return;
   }

   // Handle action key
   switch( key ) {                  // Handle data key
     case XK_Shift_L:               // Left Shift key
     case XK_Shift_R:               // Right Shift key
     case XK_Control_L:             // Left Control key
     case XK_Control_R:             // Right Control key
     case XK_Caps_Lock:             // Caps Lock key
     case XK_Shift_Lock:            // Shift Lock key
     case XK_Meta_L:                // Left Meta key
     case XK_Meta_R:                // Right Meta key
     case XK_Alt_L:                 // Left Alt key
     case XK_Alt_R:                 // Right Alt key
     case XK_Super_L:               // Left Super key
     case XK_Super_R:               // Right Super key
     case XK_Hyper_L:               // Left Hyper key
     case XK_Hyper_R:               // Right Hyper key
     case XK_Num_Lock:              // Num Lock key
       ;;                           // (Silently Ignored)
       break;

     case XK_BackSpace: {           // (Backspace)
       op_key_backspace();
       break;
     }
     case XK_Break:                 // (Break) or
     case XK_Pause: {               // (Pause)
       if( state & KS_ALT )
         op_debug();
       break;
     }
     case 0x007F:                   // (DEL)
     case XK_Delete: {              // (Delete)
       op_key_delete();
       break;
     }
     case XK_Escape:                // (Escape)
     case KEY_ESC: {                // (Escape)
       op_swap_view();
       break;
     }
     case XK_Insert: {              // (Insert)
       op_key_insert();
       break;
     }
     case XK_Return: {              // (Enter)
       if( state & KS_CTL )
         op_insert_line();
       else
         op_key_enter();
       break;
     }
     case KEY_TAB:                  // (TAB)
     case XK_Tab: {                 // (TAB)
       op_key_tab_forward();
       break;
     }
     case XK_ISO_Left_Tab: {        // (Shift-TAB)
       op_key_tab_reverse();
       break;
     }

     //-----------------------------------------------------------------------
     // Function keys
     case XK_F1: {
       op_help();
       break;
     }
     case XK_F2: {
       op_key_idle();
       break;
     }
     case XK_F3: {
       op_safe_quit();
       break;
     }
     case XK_F4: {
       op_goto_changed();
       return;
     }
     case XK_F5: {
       op_repeat_locate();
       break;
     }
     case XK_F6: {
       op_repeat_change();
       break;
     }
     case XK_F7: {
       op_goto_prev_file();
       break;
     }
     case XK_F8: {
       op_goto_next_file();
       break;
     }
     case XK_F9: {
       if( state & KS_CTL )
         op_copy_cursor_to_hist();
       else
         op_copy_file_name_to_hist();
       break;
     }
     case XK_F10: {
       op_line_to_top();
       break;
     }
     case XK_F11: {
       op_undo();
       break;
     }
     case XK_F12: {
       op_redo();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case XK_Home: {                // (Home)
       op_key_home();
       break;
     }
     case XK_Down: {                // (Down arrow)
       op_key_arrow_down();
       break;
     }
     case XK_Left: {                // (Left arrow)
       op_key_arrow_left();
       break;
     }
     case XK_Right: {               // (Right arrow)
       op_key_arrow_right();
       break;
     }
     case XK_Up: {                  // (Up arrow)
       op_key_arrow_up();
       break;
     }
     case XK_Page_Up: {             // (Page_Up key)
       op_key_page_up();
       break;
     }
     case XK_Page_Down: {           // (Page_Down key)
       op_key_page_down();
       break;
     }
     case XK_End: {                 // (End key)
       op_key_end();
       break;
     }

     //-----------------------------------------------------------------------
     // Key not assigned
     default:
       op_key_dead();
       break;
   }

   key_state &= ~(KS_ESC | KS_NFC);
}
