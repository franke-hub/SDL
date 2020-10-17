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
//       EdText.h
//
// Purpose-
//       Editor: TextWindow screen
//
// Last change date-
//       2020/10/16
//
// Implementation notes-
//       Cygwin X-server does not support xcb_xfixes_hide_cursor, even though
//       it reports Xfixes 5.0 supported. Auto-hide of cursor not available.
//
// Implementation TODOs-
//       Command line input NOT CODED YET.
//
//----------------------------------------------------------------------------
#ifndef EDTEXT_H_INCLUDED
#define EDTEXT_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include <string>                   // For std::string
#include <sys/types.h>              // For
#include <pub/List.h>               // For pub::List

#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include "Xcb/Active.h"             // For xcb::Active
#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/TextWindow.h"         // For xcb::TextWindow
#include "Xcb/Types.h"              // For xcb::DEV_EVENT_MASK

#include "EdFile.h"                 // For EdFile objects
#include "Editor.h"                 // For Editor
#include "EdTabs.h"                 // For EdTabs

//----------------------------------------------------------------------------
//
// Class-
//       EdText
//
// Purpose-
//       TextWindow keyboard, mouse, and screen controller.
//
//----------------------------------------------------------------------------
class EdText : public xcb::TextWindow { // Editor TextWindow viewport
//----------------------------------------------------------------------------
// EdText::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
enum                                // System cursor state
{  CS_RESET= 0                      // Reset (initial state)
,  CS_HIDDEN                        // Hidden
,  CS_VISIBLE                       // Visible
};

struct Motion {                     // System motion controls
int                    state;       // System cursor state
xcb_timestamp_t        time;        // Last movement timestamp
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

//----------------------------------------------------------------------------
// EdText::Attributes
//----------------------------------------------------------------------------
xcb::Active            active;      // The Active buffer
EdFile*                file= nullptr; // The current File object

int                    command= false; // TRUE if in command mode
xcb_gcontext_t         gc_chg= 0;   // Graphic context: status, changed file
xcb_gcontext_t         gc_cmd= 0;   // Graphic context: command line
xcb_gcontext_t         gc_msg= 0;   // Graphic context: message line
xcb_gcontext_t         gc_sts= 0;   // Graphic context: status, default
Motion                 motion= {0, 0, 0, 0}; // System motion controls

//----------------------------------------------------------------------------
// EdText::Constructor
//----------------------------------------------------------------------------
public:
   EdText(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr)
:  TextWindow(parent, name ? name : "EdText"), active()
{
   if( opt_hcdm )
     debugh("EdText(%p)::EdText\n", this);

   // Reserve TOP line
   USER_TOP= 1;                     // Message/Status/Command line

   // Set window event mask
   emask= XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_MOTION
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
        | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_PROPERTY_CHANGE
        ;
}

//----------------------------------------------------------------------------
// EdText::Destructor
//----------------------------------------------------------------------------
void                                // In .cpp, remove from .h file (window parm)
   free_gc(                         // Free
     xcb_gcontext_t&   gc)          // This graphic context
{
   if( gc ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc) );
     gc= 0;
   }
}

virtual
   ~EdText( void )                  // Destructor
{
   if( opt_hcdm )
    debugh("EdText(%p)::~EdText\n", this);

   free_gc(gc_chg);
   free_gc(gc_cmd);
   free_gc(gc_msg);
   free_gc(gc_sts);

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       text= nullptr) const // Associated text
{
   debugf("EdText(%p)::debug(%s) Named(%s)\n", this, text ? text : ""
         , get_name().c_str());

   TextWindow::debug(text);

   debugf("..command(%d) gc_chg(%u) gc_cmd(%u) gc_msg(%u) gc_sts(%u)\n"
         , command, gc_chg, gc_cmd, gc_msg, gc_sts);
   debugf("..motion[%d,%u,%d,%d]\n"
         , motion.state, motion.time, motion.x, motion.y);
   active.debug(text);
}

//----------------------------------------------------------------------------
//
// Protected method-
//       EdText::commit
//
// Purpose-
//       Commit the Active line
//
// Implementation notes-
//       TODO: Add REDO/UNDO logic
//
//----------------------------------------------------------------------------
protected:
void
   commit( void )                   // Commit the Active line
{
   const char* buffer= active.get_changed();
   if( opt_hcdm )
    debugh("EdText(%p)::commit buffer(%s) cursor(%p)[%u,%u]\n", this, buffer
          , cursor, col, row);

   if( buffer && cursor ) {         // If actually changed (and changeable)
     if( ((EdLine*)cursor)->flags.rdonly ) return; // TODO: INSURE NEEDED
     file->changed= true;           // The file has changed

     size_t length= active.get_used();
     if( length == 0 )
       line->text= Editor::NO_STRING;
     else {
       char* revise= Editor::editor->get_text(length + 1);
       strcpy(revise, buffer);
       cursor->text= revise;
     }
   }
}

//----------------------------------------------------------------------------
//
// Protected method-
//       EdText::hide_cursor
//
// Purpose-
//       Get pixel [x,y] position from [col,row], also hiding the cursor
//
//----------------------------------------------------------------------------
xcb_point_t                         // The offset in Pixels
   hide_cursor(                     // Get offset in Pixels
     unsigned          col,         // And this column
     unsigned          row)         // For this row
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;

     flush();                       // Should not be needed: putxy imminent
   }

   return TextWindow::get_xy(col, row);
}

//----------------------------------------------------------------------------
//
// Protected method-
//       EdText::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column)      // The (absolute) column number
{
   int rc= 1;                       // Default, draw not performed

   clr_cursor();                    // Clear the current cursor
   size_t current= col_zero + col;  // Set current column
   unsigned col_move= col_size / 8; // MINIMUM shift size
   if( col_move == 0 ) col_move= 1;
   if( column < current ) {         // If moving cursor left
     if( column < col_zero ) {      // If shifting left
       rc= 0;                       // Redraw required
       if( column <= (col_size - col_move) )
         col_zero= 0;
       else
         col_zero= column - col_move;
     }
   } else if( column > current ) {  // If moving right
     if( column >= (col_zero + col_size ) ) { // If shifting right
       rc= 0;                       // Redraw required
       col_zero= column - col_size + col_move;
     }
   }
   col= unsigned(column - col_zero);

   if( rc == 0 )                    // If redraw needed
     draw();
   else
     draw_status();

   // Set (invert) the current cursor character
   set_cursor();

   return rc;
}

//----------------------------------------------------------------------------
//
// Protected method-
//       EdText::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   move_cursor_V(                   // Move cursor vertically
     int             n= 1)          // The relative column (Down positive)
{
   clr_cursor();

   int rc= 1;                       // Default, no draw
   EdLine* cursor= (EdLine*)this->cursor; // The cursor line
   if( cursor->get_next() && cursor->get_prev() ) // If not a static line
     commit();                      // Commit the active line

   if( n > 0 ) {                    // Move down
     while( n-- ) {
       if( row_used > (row + 1) )
         row++;
       else {
         EdLine* line= (EdLine*)this->line->get_next();
         if( line ) {
           this->line= line;
           row_used--;
           row_zero++;
           rc= 0;
         } else {
           xcb::trace(".BOT", 0);
           break;
         }
         if( last->get_next() == nullptr )
           row--;
       }
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( row > 0 )
         row--;
       else {
         EdLine* line= (EdLine*)this->line->get_prev();
         if( line ) {
           this->line= line;
           row_zero--;
           rc= 0;
         } else {
           xcb::trace(".TOP", 0);
           break;
         }
       }
     }
   }

   set_active();
   if( rc == 0 ) {
     draw();
     set_cursor();
   } else
     draw_status();

   return rc;
}

//----------------------------------------------------------------------------
//
// Protected method-
//       EdText::set_active
//
// Purpose-
//       After a vertical column move, set the Active (cursor) line
//
//----------------------------------------------------------------------------
virtual void
   set_active( void )               // Set the Active (cursor) line
{
   cursor= nullptr;                 // Default, no cursor
   EdLine* line= (EdLine*)this->line; // Get the first line
   if( line == nullptr ) {          // If Empty
     if( opt_hcdm || opt_verbose >= 0 ) // TODO: REMOVE
       xcb::user_debug("%4d HCDM EdText\n", __LINE__);
     return;
   }

   for(unsigned r= 0; ; r++) {      // Set the Active line
     if( r == row ) {
       cursor= line;
       break;
     }

     EdLine* next= line->get_next();
     if( next == nullptr ) {
       if( opt_hcdm || opt_verbose >= 0 ) // TODO: REMOVE
         xcb::user_debug("%4d HCDM EdText r(%3d) row(%3d) row_size(%3d)\n"
                        , __LINE__, r, row, row_size );
       row= r;
       cursor= line;
       break;
     }

     if( (r + 1) >= row_size ) {
       if( opt_hcdm || opt_verbose >= 0 ) // TODO: REMOVE
         xcb::user_debug("%4d HCDM EdText r(%3d) row(%3d) row_size(%3d)\n"
                        , __LINE__, r, row, row_size );
       row= r;
       cursor= line;
       break;
     }

     line= next;
   }

   active.reset(cursor->text);
   set_cursor();                    // Set the cursor
}

//----------------------------------------------------------------------------
//
// Protected methods-
//       EdText::set_cursor
//       EdText::clr_cursor
//
// Purpose-
//       Set or clear the cursor
//
//----------------------------------------------------------------------------
void
   set_cursor(bool set= true)       // Set cursor (with set == true)
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("EdText(%p)::cursor_%s cursor[%u,%u]\n", this, set ? "S" : "C"
           , col, row);

   size_t column= col_zero + col;   // The current column
   char buffer[8];                  // Encoder buffer
   pub::UTF8::Encoder encoder(buffer, sizeof(buffer));
   pub::UTF8::Decoder decoder(active.get_buffer(column));
   int code= decoder.decode();
   if( code <= 0 )
     code= ' ';
   encoder.encode(code);
   buffer[encoder.get_used()]= '\0';

   if( set )                        // If cursor set
     putxy(flipGC, hide_cursor(col, row), buffer);
   else                             // If cursor clear
     putxy(hide_cursor(col, row), buffer);
   flush();
}

void
   clr_cursor( void )               // Clear cursor
{  return set_cursor(false); }      // (Un-set the cursor)

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
public:
virtual void
   configure( void )                // Configure the Window
{
   if( opt_hcdm )
     debugh("EdText(%p)::configure\n", this);

   TextWindow::configure();         // Create the Window

   // Create the graphic contexts
   gc_chg= font.makeGC(0x00B22222, 0x00C0C0C0); // FireBrick red over grey
   gc_cmd= font.makeGC(0x00FFFFFF, 0x00FF00FF); // White over light purple
   gc_msg= font.makeGC(0x00B22222, 0x00FFFF00); // FireBrick red over yellow
   gc_sts= font.makeGC(0x00000000, 0x00C0C0C0); // Black over grey
}

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::cursor_text
//
// Purpose-
//       Return the current cursor line text (may differ from commit version)
//
//----------------------------------------------------------------------------
virtual const char*                 // The cursor line text
   cursor_text(                     // Get cursor line text
     const xcb::Line*  line) const  // For this (cursor) line
{
   if( cursor )                     // If the cursor is active
     return active.get_buffer();    // Return the active buffer

   return line->text;
}

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::draw
//
// Purpose-
//       (Re)draw the screen and set the cursor
//
//----------------------------------------------------------------------------
static void                         // TODO: Local static
   format6(
     size_t            value,
     char*             buffer)
{
   if( value > 10000000 )
     sprintf(buffer, "*%.6u", unsigned(value % 1000000));
   else
     sprintf(buffer, "%7zu", value);
}

static void                         // TODO: Local static
   format8(
     size_t            value,
     char*             buffer)
{
   if( value > 1000000000 )
     sprintf(buffer, "*%.8u", unsigned(value % 100000000));
   else
     sprintf(buffer, "%9zu", value);
}

virtual bool                        // Return code, TRUE if handled
   draw_command( void )             // Redraw the command line
{
   if( command ) {
     char buffer[256];              // Status line buffer
     memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
     buffer[sizeof(buffer)-1]= '\0';
     putxy(gc_cmd, 0, 0, buffer);
     flush();
     return true;
   }

   return false;
}

virtual void
   draw_info( void )                // Redraw the information line
{
   if( draw_command() ) return;
   if( draw_message() ) return;
   draw_status();
}

virtual bool                        // Return code, TRUE if handled
   draw_message( void )             // Message line
{
   EdMess* mess= file->messages.get_head();
   if( mess == nullptr ) return false;

   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
   strcpy(buffer, mess->mess.c_str());
   buffer[strlen(buffer)]= ' ';
   putxy(gc_msg, 0, 0, buffer);
   flush();
   return true;
}

virtual void
   draw_status( void )              // Redraw the status line
{
   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
   // Offset:      012345678901234567890123456789012345678901234567890123456
   strcpy(buffer, "[REP] [UNIX] L[*********,*********] C[*******] EDIT V3.0");
   buffer[56]= ' ';                 // Full size (255) length buffer
   if( xcb::keystate & xcb::KS_INS ) // Set insert mode (if not REP)
     memcpy(buffer+1, "INS", 3);
   if( file->mode != EdFile::M_UNIX ) {
     if( file->mode == EdFile::M_DOS )
       memcpy(buffer+7, "!DOS", 4);
     else if( file->mode == EdFile::M_MIX )
       memcpy(buffer+7, "!MIX", 4);
     else if( file->mode == EdFile::M_BIN ) // Set file mode (if not UNIX)
       memcpy(buffer+7, "!BIN", 4);
   }

   char number[16];                 // Numeric buffer
   format8(row_zero + row, number); memcpy(buffer+15, number, 9);
   format8(file->rows,     number); memcpy(buffer+25, number, 9);
   format6(col_zero + col, number); memcpy(buffer+38, number, 7);

   xcb_gcontext_t gc= gc_sts;       // Set background/foreground GC
   if( file->changed || file->damaged || active.get_changed() )
     gc= gc_chg;
   putxy(gc, 0, 0, buffer);
   flush();
}

virtual void
   draw( void )                     // Redraw the Window
{
   TextWindow::draw();              // Draw the text screen
   draw_info();                     // Draw the information line

   set_cursor();
   show();
}

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::activate
//
// Purpose-
//       Set the current file
//
//----------------------------------------------------------------------------
void
   activate(                        // Activate
     EdFile*           file)        // This file
{
   // Out with the old
   if( this->file ) {
     commit();

     this->file->top_line= (EdLine*)this->line;
     this->file->col_zero= this->col_zero;
     this->file->row_zero= this->row_zero;
     this->file->col=  this->col;
     this->file->row=  this->row;
   }

   // In with the new
   this->file= file;
   this->line= nullptr;
   if( file ) {
     // Get the file attributes
     this->line= file->top_line;
     this->last= this->line;
     this->col_zero= file->col_zero;
     this->row_zero= file->row_zero;
     this->col=  file->col;
     this->row=  file->row;

     // Update window title, omitting middle of file name if necessary
     char buffer[64];
     const char* C= file->name.c_str();
     size_t      L= strlen(C);

     strcpy(buffer, "Edit: ");
     if( L > 57 ) {
       memcpy(buffer + 6, C, 27);
       memcpy(buffer + 33, "...", 3);
       strcpy(buffer + 36, C + L - 27);
     } else {
       memcpy(buffer + 6, C, L+1);
     }
     set_main_name(buffer);

     // Redraw
     set_active();
     draw();
     set_cursor();
   }
}

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::key_input
//
// Purpose-
//       Handle keypress event
//
//----------------------------------------------------------------------------
protected: // (Only used by key_input)
int                                 // Return code, 0 if handled
   key_alt(                         // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   int rc= 0;                       // Return code, default handled
   switch(key) {
     default:
       fprintf(stderr, "Alt(%c) ignored\n", key);
       rc= 1;
   }

   return rc;
}

int                                 // Return code, 0 if handled
   key_ctl(                         // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   int rc= 0;                       // Return code, default handled
   switch(key) {
     case 'Q':                       // ctrl-Q (Quit)
       Editor::editor->do_done();
       break;

     default:
       fprintf(stderr, "Ctrl(%c) ignored\n", key);
       rc= 1;
   }

   return rc;
}

public:
virtual int                         // Return code, 0 if handled
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{
   if( opt_hcdm ) {
     char B[2]; B[0]= '\0'; B[1]= '\0'; const char* K= B;
     if( key >= 0x0020 && key < 0x007f ) B[0]= char(key);
     else K= Editor::editor->key_to_name(key);
     debugh("EdText(%p)::key_input(0x%.4x,%.4x) '%s'\n", this, key, state, K);
   }

   const char* name= Editor::editor->key_to_name(key);
   xcb::trace(".KEY", (state<<16) | (key & 0x0000ffff), name);

   int rc= 0;                       // Return code, default handled
   size_t column= col_zero + col;   // The cursor column
   if( key >= 0x0020 && key < 0x007f ) { // If text key
     int mask= state & (xcb::KS_ALT | xcb::KS_CTRL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case xcb::KS_ALT:
           rc= key_alt(key);
           break;

         case xcb::KS_CTRL:
           rc= key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTRL)
           // BRINGUP TEST. TODO:REMOVE
           debugf("Alt+Ctrl(0x%.4x,%c) ignored\n", key, key);
           rc= 1;
       }
       return rc;
     }

     // If data key
     if( ((EdLine*)cursor)->flags.rdonly ) // If read-only line
       return 0;                    // TODO: Warning?

     if( xcb::keystate & xcb::KS_INS ) { // If Insert state
       active.insert_char(column, key);
       if( move_cursor_H(column + 1) ) {
         const char* buffer= active.get_buffer();
         putxy(hide_cursor(col - 1, row), buffer + active.index(column));
       }
     } else {
       active.replace_char(column, key);
       if( move_cursor_H(column + 1) ) {
         char buffer[2];
         buffer[0]= char(key);
         buffer[1]= '\0';
         putxy(hide_cursor(col - 1, row), buffer);
       }
     }
     set_cursor();
     flush();

     return 0;
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
       ;;                           // (Silently Ignore)
       break;

     case XK_BackSpace: {
       clr_cursor();                // Clear the cursor
       if( column > 0 )
         column--;
       active.remove_char(column);
       int rc= move_cursor_H(column);
       if( rc ) {
         active.append_text(" ");   // (Clear removed character)
         const char* buffer= active.get_buffer(column - col_zero);
         putxy(hide_cursor(col, row), buffer);
         set_cursor();
         flush();
       }
       break;
     }
     case 0x007f:                  // (Should not occur)
     case XK_Delete: {
       active.remove_char(column);
       active.append_text(" ");
       const char* buffer= active.get_buffer(column);
       putxy(hide_cursor(col, row), buffer);
       set_cursor();
       flush();
       break;
     }
     case XK_Escape:                // Escape: UNDO
       active.undo();
       active.index(col_zero+col_size); // Blank fill
       putxy(hide_cursor(0, row), active.get_buffer(col_zero));
       draw_status();
       set_cursor();
       break;

     case XK_Insert: {              // Insert key
       xcb::keystate ^= xcb::KS_INS; // Invert the insert state
       draw_status();
       break;
     }
     case XK_Return:
       move_cursor_V(+1);
       move_cursor_H(0);
       break;

     case XK_Tab: {                 // TODO: PRELIMINARY
       enum { TAB= 8 };             // TAB Column BRINGUP (( 2 ** N ))
       column += TAB;
       column &= ~(TAB - 1);
       move_cursor_H(column);
       break;
     }
     case XK_ISO_Left_Tab:          // TODO: PRELIMINARY
       if( column ) {               // If not already at column[0]
         enum { TAB= 8 };           // TAB Column BRINGUP (( 2 ** N ))
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
       Editor::editor->do_help();
       break;
     }
     case XK_F3: {
       commit();
       Editor::editor->do_exit(this->file);
       break;
     }
     case XK_F7: {                  // Prior file
       commit();
       EdFile* file= this->file->get_prev();
       if( file == nullptr )
         file= Editor::editor->ring.get_tail();
       if( file != this->file )
         activate(file);
       break;
     }
     case XK_F8: {                  // Next file
       commit();
       EdFile* file= this->file->get_next();
       if( file == nullptr )
         file= Editor::editor->ring.get_head();
       if( file != this->file )
         activate(file);
       break;
     }
     case XK_F12: {                 // Bringup test
       Editor::editor->do_test();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case XK_Home: {                // Home key
       clr_cursor();
       col= 0;
       if( col_zero ) {
         col_zero= 0;
         draw();
       } else
         draw_status();
       set_cursor();
       break;
     }
     case XK_Left: {                // Left arrow
       if( column > 0 )
         move_cursor_H(column - 1);
       break;
     }
     case XK_Up: {                  // Up arrow
       move_cursor_V(-1);
       break;
     }
     case XK_Right: {               // Right arrow
       move_cursor_H(column + 1);
       break;
     }
     case XK_Down: {                // Down arrow
       move_cursor_V(1);
       break;
     }
     case XK_Page_Up: {             // Page_Up key
       clr_cursor();
       commit();
       unsigned count= row_size;
       if( line->get_prev() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)line->get_prev();
           if( up == nullptr )
             break;

           row_zero--;
           line= up;
         }
         set_active();
         draw();
       }
       set_cursor();
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       clr_cursor();
       commit();
       if( line != last ) {
         row_zero += (row_used - USER_TOP);
         line= last;
         set_active();
         draw();
       }
       set_cursor();
       break;
     }
     case XK_End: {                 // End key
       move_cursor_H(active.get_cols());
       break;
     }

     default:
       if( opt_hcdm || opt_verbose >= 0 ) debugf("0x%.6x Key ignored\n", key);
       rc= 1;
   }

   return rc;
}

//============================================================================
// EdText::Event handlers
//============================================================================
public:
virtual void
   button_press(                    // Handle this
     xcb_button_press_event_t* event) // Button press event
{
   // Use E.detail and xcb::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm && opt_verbose > 1 )
     debugf("button_press:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t column= col_zero + col;   // The current column number
   switch( E.detail ) {
     case xcb::BT_LEFT: {           // Left button
       int chg_row= int(get_row(E.event_y)) - int(row); // Row change
       if( chg_row != 0 )         // If row changed
         move_cursor_V(chg_row - USER_TOP); // Set new row
       move_cursor_H(col_zero + get_col(E.event_x)); // Set new column
       break;
     }
     case xcb::BT_RIGHT: {          // Right button
       unsigned abs_row= get_row(E.event_y+1);
       if( abs_row == 0 ) {
         if( file->messages.remq() ) { // If message handled
           draw_info();
           break;
         }

         if( command ) {            // If in command mode
           command= false;
           set_cursor();
         } else {
           commit();
           command= true;
         }
         draw_info();
       }
       break;
     }
     case xcb::WT_PUSH:             // Mouse wheel push (away)
       move_cursor_V(-3);
       break;

     case xcb::WT_PULL:             // Mouse wheel pull (toward)
       move_cursor_V(+3);
       break;

     case xcb::WT_LEFT:             // Mouse wheel left
       move_cursor_H(column > 3 ? column - 3 : 0);
       break;

     case xcb::WT_RIGHT:            // Mouse wheel right
       move_cursor_H(column + 3);
       break;

     case xcb::BT_CNTR:             // Middle button (ignored)
     default:                       // (Buttons 6 and 7 undefined)
       break;
   }
}

virtual void
   motion_notify(                   // Handle this
     xcb_motion_notify_event_t*     // Motion notify event
                       E)
{
   if( opt_hcdm )
     debugh("motion_notify: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   // printf("."); fflush(stdout);  // See when called
   if( E->event_x != motion.x || E->event_y != motion.y ) {
     if( motion.state != CS_VISIBLE ) { // If not already visible
       NOQUEUE("xcb_show_cursor", xcb_xfixes_show_cursor(c, widget_id) );
       motion.state= CS_VISIBLE;
       flush();
     }

     motion.time= E->time;
     motion.x= E->event_x;
     motion.y= E->event_y;
   }
}
}; // class EdText
#endif // EDTEXT_H_INCLUDED
