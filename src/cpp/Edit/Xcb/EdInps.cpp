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
//       EdInps.cpp
//
// Purpose-
//       Editor: Implement EdText.h keyboard and mouse event handlers.
//
// Last change date-
//       2021/08/08
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Keysym.h>             // For X11/keysymdef
#include <gui/Types.h>              // For gui::DEV_EVENT_MASK
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  KP_MAX= 0xffbf                   // Keypad maximum key value
,  KP_MIN= 0xff80                   // Keypad minimum key value
,  TAB= 8                           // TAB spacing (2**N)
,  USE_BRINGUP= false               // Use bringup debugging?
}; // Compilation controls

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
static char buffer[16];
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
//       line_protected
//
// Purpose-
//       Disallow keypress event for a protected line.
//
//----------------------------------------------------------------------------
static int                          // Return code, TRUE if error message
   line_protected(                  // Handle this protected line
     xcb_keysym_t      key,         // Input key
     int               state)       // Alt/Ctl/Shift state mask
{
   if( (key >= 0x0020 && key < 0x007F) || key == '\t' ) { // If text key
     int mask= state & (gui::KS_ALT | gui::KS_CTRL);

     if( mask ) {
       key= toupper(key);
       if( mask == gui::KS_ALT ) {
         switch(key) {                // Allowed keys:
           case 'C':                  // COPY MARK
           case 'D':                  // DELETE MARK
           case 'I':                  // INSERT
           case 'M':                  // MOVE MARK
           case 'Q':                  // QUIT
           case 'U':                  // UNDO MARK
             return false;

           default:
             break;
         }
       } else if( mask == gui::KS_CTRL ) {
         switch(key) {                // Allowed keys:
           case 'C':                  // COPY MARK
           case 'V':                  // PASTE COPY
           case 'X':                  // CUT MARK
             return false;

           default:
             break;
         }
       }
     }
   } else {
     switch( key ) {                // Check for disallowed key
       case 0x007F:                 // (DEL KEY, should not occur)
       case XK_BackSpace:
       case XK_Delete:
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
//       EdText::key_alt
//
// Purpose-
//       Handle alt-key event
//
//----------------------------------------------------------------------------
void
   EdText::key_alt(                 // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // ALT-
     case 'B': {                    // Mark block
       editor::put_message(mark->mark(file, data->cursor, data->get_column()));
       draw();
       break;
     }
     case 'C': {                    // Copy the mark
       const char* error= mark->verify_copy(data->cursor);
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->copy();
       mark->paste(file, data->cursor, data->get_column());
       draw();
       break;
     }
     case 'D': {                    // Delete the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     case 'J': {                    // Join lines
       editor::put_message( editor::do_join() ); // Join current/next lines
       break;
     }
     case 'I': {                    // Insert line
       editor::put_message( editor::do_insert() ); // Insert line after cursor
       break;
     }
     case 'L': {                    // Mark line
       editor::put_message( mark->mark(file, data->cursor) );
       draw();
       break;
     }
     case 'M': {                    // Move mark (Uses cut/paste)
       const char* error= mark->verify_move(data->cursor);
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->cut();
       mark->paste(file, data->cursor, data->get_column());
       draw();
       break;
     }
     case 'Q': {                    // (Safe) quit
       data->commit();
       editor::put_message( editor::do_quit() );
       break;
     }
     case 'S': {                    // Split line
       editor::put_message( editor::do_split() ); // Split the current line
       break;
     }
     case 'U': {                    // Undo mark
       EdFile* mark_file= mark->mark_file;
       mark->undo();
       if( file == mark_file )
         draw();
       else
         draw_info();               // (Remove "No Mark" message)
       break;
     }
     default:
       editor::put_message("Invalid key");
       draw_info();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::key_ctl
//
// Purpose-
//       Handle ctl-key event
//
//----------------------------------------------------------------------------
void
   EdText::key_ctl(                 // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // CTRL-
     case 'C': {                    // Copy the mark
       editor::put_message( mark->copy() );
       break;
     }
     case 'V': {                    // Paste (from copy/cut)
       data->commit();
       const char* error= mark->paste(file, data->cursor, data->get_column());
       if( error )
         editor::put_message(error);
       else
         draw();
       break;
     }
     case 'X': {                    // Cut the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     default:
       editor::put_message("Invalid key");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::key_input
//
// Purpose-
//       Handle keypress event
//
//----------------------------------------------------------------------------
void
   EdText::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdView* const view= editor::view;

   // Diagnostics
   const char* key_name= key_to_name(key);
   Config::trace(".KEY", (state<<16) | (key & 0x0000ffff), key_name);
   if( opt_hcdm ) {
     debugh("EdText(%p)::key_input(0x%.4x,%.4x) '%s'\n", this
           , key, state, key_name);
   }

   // Convert Keypad keys to standard keys
   if( key >= KP_MIN && key <= KP_MAX ) {
     unsigned short* kp_tab= kp_off;
     if( state & gui::KS_NUML )
       kp_tab= kp_num;
     key= kp_tab[key - KP_MIN];
   }

   // Convert Ctrl-TAB to standard TAB
   if( key == XK_Tab && (state & gui::KS_CTRL) ) {
     key= '\t';
     state= 0;
   }

   // Handle protected line
   if( view == data ) {             // Only applies to data view
     if( data->cursor->flags & EdLine::F_PROT // If protected line
         && line_protected(key, state) ) // And modification key
       return;                      // (Disallowed)
   }

   // Handle input key
   file->rem_message_type();        // Remove informational message
   if( file->mess_list.get_head() ) { // If any message remains
     draw_info();
     return;
   }

   size_t column= view->get_column(); // The cursor column
   if( (key >= 0x0020 && key < 0x007F) || key == '\t' ) { // If text key
     int mask= state & (gui::KS_ALT | gui::KS_CTRL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case gui::KS_ALT:
           key_alt(key);
           break;

         case gui::KS_CTRL:
           key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTRL)
           editor::put_message("Invalid key");
       }
       return;
     }

     if( editor::data_protected() )
       return;

     if( keystate & KS_INS ) {      // If Insert state
       view->active.insert_char(column, key);
       if( move_cursor_H(column + 1) )
         view->draw_active();
     } else {
       view->active.replace_char(column, key);
       move_cursor_H(column + 1);
     }
     draw_info();
     draw_cursor();
     flush();

     return;
   }

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
       ;;                           // (Silently Ignore)
       break;

     case XK_BackSpace: {
       if( editor::data_protected() )
         return;

       if( column > 0 )
         column--;
       view->active.remove_char(column);
       move_cursor_H(column);
       view->active.append_text(" ");
       view->draw_active();
       break;
     }
     case XK_Break:
     case XK_Pause: {
       if( state & gui::KS_ALT ) { // Alt-Pause, diagnostic dump
         if( editor::diagnostic ) {
           editor::diagnostic= false;
           Config::errorf("Diagnostic mode exit\n");
           pub::Trace* trace= pub::Trace::trace;
           if( trace )
             trace->flag[pub::Trace::X_HALT]= false;
         } else {
           Editor::alertf("*DEBUG*"); // (Sets editor::diagnostic, stops trace)
         }
       }
       break;
     }
     case 0x007F:                  // (This encoding should not occur)
     case XK_Delete: {             // (This encoding is expected instead)
       if( editor::data_protected() )
         return;

       view->active.remove_char(column);
       view->active.append_text(" ");
       view->draw_active();
       draw_info();
       break;
     }
     case XK_Escape: {              // Escape: Invert the view
       editor::do_view();
       break;
     }
     case XK_Insert: {              // Insert key
       keystate ^= KS_INS;          // Invert the insert state
       draw_info();
       break;
     }
     case XK_Return: {
       move_cursor_H(0);
       view->enter_key();
       break;
     }
     case XK_Tab: {
       column += TAB;
       column &= ~(TAB - 1);
       move_cursor_H(column);
       break;
     }
     case XK_ISO_Left_Tab:
       if( column ) {               // If not already at column[0]
         if( column <= TAB )
           column= 0;
         else {
           if( (column % TAB) == 0 )
             column--;
           column &= ~(TAB - 1);
         }
         move_cursor_H(column);
       }
       break;

     //-----------------------------------------------------------------------
     // Function keys
     case XK_F1: {
       printf(" F1: This help message\n"
              " F2: NOP\n"
              " F3: Quit File\n"
              " F4: Test changed\n"
              " F5: Locate\n"
              " F6: Change\n"
              " F7: Previous File\n"
              " F8: Next File\n"
              " F9: NOP\n"
              "F10: Line to top\n"
              "F11: Undo\n"
              "F12: Redo\n"
       );
       break;
     }
     case XK_F2: {                  // NOT ASSIGNED
       break;
     }
     case XK_F3: {                  // (Safe) quit
       data->commit();
       editor::put_message( editor::do_quit() );
       break;
     }
     case XK_F4: {                  // Test changed
       if( editor::un_changed() )
         editor::put_message("No files changed");
       break;
     }
     case XK_F5: {
       editor::put_message( editor::do_locate() );
       break;
     }
     case XK_F6: {
       editor::put_message( editor::do_change() );
       break;
     }
     case XK_F7: {                  // Prior file
       data->commit();
       EdFile* file= editor::file->get_prev();
       if( file == nullptr )
         file= editor::file_list.get_tail();
       if( file != editor::file ) {
         activate(file);
         draw();
       }
       break;
     }
     case XK_F8: {                  // Next file
       data->commit();
       EdFile* file= editor::file->get_next();
       if( file == nullptr )
         file= editor::file_list.get_head();
       if( file != editor::file ) {
         activate(file);
         draw();
       }
       break;
     }
     case XK_F9: {                  // NOT ASSIGNED
       break;
     }
     case XK_F10: {                 // Line to top
       head= data->cursor;
       data->row_zero += (data->row - USER_TOP);
       data->row= USER_TOP;
       draw();
       break;
     }
     case XK_F11: {                 // Undo
       if( data->active.undo() ) {
         data->draw_active();
         draw_info();
       } else
         file->undo();
       break;
     }
     case XK_F12: {                 // Redo
       file->redo();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case XK_Home: {                // Home key
       undo_cursor();
       view->col= 0;
       if( view->col_zero ) {
         view->col_zero= 0;
         draw();
       } else
         draw_info();

       draw_cursor();
       flush();
       break;
     }
     case XK_Left: {                // Left arrow
       if( column > 0 )
         move_cursor_H(column - 1);
       break;
     }
     case XK_Up: {                  // Up arrow
       view->move_cursor_V(-1);
       break;
     }
     case XK_Right: {               // Right arrow
       move_cursor_H(column + 1);
       break;
     }
     case XK_Down: {                // Down arrow
       view->move_cursor_V(1);
       break;
     }
     case XK_Page_Up: {             // Page_Up key
       int rows= row_size - (USER_TOP + USER_BOT + 1);
       move_screen_V(-rows);
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       int rows= row_size - (USER_TOP + USER_BOT + 1);
       move_screen_V(+rows);
       break;
     }
     case XK_End: {                 // End key
       move_cursor_H(view->active.get_cols());
       break;
     }
     default:
       editor::put_message("Invalid key");
   }
}

//============================================================================
// EdText::Event handlers
//============================================================================
void
   EdText::button_press(            // Handle this
     xcb_button_press_event_t* event) // Button press event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   // Use E.detail and gui::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm )
     debugh("button:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t current_col= view->get_column(); // The current column number
   unsigned button_row= get_row(E.event_y); // (Absolute) button row

   switch( E.detail ) {
     case gui::BT_LEFT: {           // Left button
       if( button_row < USER_TOP ) { // If on command/history line
         if( !file->rem_message() ) { // If not message removed
           if( view == hist )       // If history active
             move_cursor_H(hist->col_zero + get_col(E.event_x)); // Update column
           else
             hist->activate();
         }
         draw_info();
         break;
       }

       // Button press is on data screen
       if( view == hist ) {         // If history active
         data->activate();
         draw_info();
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
       if( button_row < USER_TOP ) {
         if( file->rem_message() ) { // If message removed
           draw_info();
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
   EdText::client_message(          // Handle this
     xcb_client_message_event_t* E) // Client message event
{
   if( opt_hcdm )
     debugh("message: type(%u) data(%u)\n", E->type, E->data.data32[0]);

   if( E->type == protocol && E->data.data32[0] == wm_close )
     device->operational= false;
}

void
   EdText::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("EdText(%p)::configure_notify(%d,%d)\n", this
           , E->width, E->height);

   resize(E->width, E->height);
}

void
   EdText::expose(                  // Handle this
     xcb_expose_event_t* E)         // Expose event
{
   if( opt_hcdm )
     debugh("EdText(%p)::expose(%d) %d [%d,%d,%d,%d]\n", this
           , E->window, E->count, E->x, E->y, E->width, E->height);

   draw();
}

void
   EdText::focus_in(                // Handle this
     xcb_focus_in_event_t* E)       // Focus-in event
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("gain focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   draw_cursor();
   flush();
   focus= true;
}

void
   EdText::focus_out(               // Handle this
     xcb_focus_out_event_t* E)      // Focus-out event
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("lost focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   undo_cursor();
   focus= false;
}

void
   EdText::motion_notify(           // Handle this
     xcb_motion_notify_event_t* E)  // Motion notify event
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("motion: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   // printf("."); fflush(stdout);  // See when called
   if( E->event_x != motion.x || E->event_y != motion.y ) {
     show_mouse();
   } else {
     if( (E->time - motion.time) < 1000 ) // If less than 1 second idle
       return;                      // Ignore
     { if( config::USE_MOUSE_HIDE ) hide_mouse(); }
   }

   motion.time= E->time;
   motion.x= E->event_x;
   motion.y= E->event_y;
}
