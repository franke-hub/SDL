//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
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
//       2020/10/12
//
//----------------------------------------------------------------------------
#ifndef EDTEXT_H_INCLUDED
#define EDTEXT_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include <string>                   // For std::string
#include <pub/List.h>               // For pub::List

#include "Xcb/Active.h"             // For xcb::Active
#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/TextWindow.h"         // For xcb::TextWindow

#include "EdFile.h"                 // For EdFile objects
#include "EdFind.h"                 // For EdFind
#include "Editor.h"                 // For Editor
#include "EdTabs.h"                 // For EdTabs

//----------------------------------------------------------------------------
//
// Class-
//       EdText
//
// Purpose-
//       TextWindow viewport
//
//----------------------------------------------------------------------------
class EdText : public xcb::TextWindow { // Editor TextWindow viewport
//----------------------------------------------------------------------------
// EdText::Attributes
//----------------------------------------------------------------------------
public:
xcb::Active            active;        // The Active buffer
EdFile*                file= nullptr; // The current File object
EdFind*                find= nullptr; // The Search control window

//----------------------------------------------------------------------------
// EdText::Constructor/Destructor
//----------------------------------------------------------------------------
   EdText(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr)
:  TextWindow(parent, name ? name : "EdText"), active()
{
   if( opt_hcdm )
     debugh("EdText(%p)::EdText\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdText( void )                  // Destructor
{
   if( opt_hcdm )
    debugh("EdText(%p)::~EdText\n", this);

   // Delete the FindWindow
   if( find ) {
     delete find;
     find= nullptr;
   }

   // Detach the EdFile
   set_file(nullptr);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::commit
//
// Purpose-
//       Commit the Active line
//
// Implementation notes-
//       TODO: Add REDO/UNDO logic
//
//----------------------------------------------------------------------------
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
// Method-
//       EdText::cursor_*
//
// Purpose-
//       Cursor movement control
//
//----------------------------------------------------------------------------
size_t                              // The current cursor column
   cursor_S(bool set= true)         // Set cursor (with set == true)
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
     putxy(flipGC, getxy(col, row), buffer);
   else                             // If cursor clear
     putxy(getxy(col, row), buffer);
   flush();

   return column;
}

size_t                              // The current cursor column
   cursor_C( void )                 // Clear cursor
{  return cursor_S(false); }        // (Un-set the cursor)

int                                 // Return code, 0 if draw performed
   cursor_H(size_t column)          // Move cursor to column
{
   int rc= 1;                       // Default, draw not performed

   size_t current= cursor_C();      // Clear the cursor, setting current column
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

   // Set (invert) the current cursor character
   cursor_S();

   return rc;
}

int                                 // Return code, 0 if draw performed
   cursor_V(int64_t n= 1)           // Move cursor vertically (Down positive)
{
   cursor_C();

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
           rc= 0;
           this->line= line;
           row_used--;
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
     cursor_S();
   }

   return rc;
}

virtual const char*                 // The cursor line text
   cursor_text(                     // Get cursor line text
     const xcb::Line*  line) const  // For this (cursor) line
{  (void)line; return active.get_buffer(); }

//----------------------------------------------------------------------------
//
// Method-
//       EdText::draw
//
// Purpose-
//       (Re)draw the screen and set the cursor
//
//----------------------------------------------------------------------------
virtual void
   draw( void )                     // Redraw the Window
{
   TextWindow::draw();
   cursor_S();

   show();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::set_active
//
// Purpose-
//       Set the Active (cursor) line
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
   cursor_S();                      // Set the cursor
}

void
   set_file(
     EdFile*           file)
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

     strcpy(buffer, "Editor: ");
     if( L > 55 ) {
       memcpy(buffer + 8, C, 26);
       memcpy(buffer + 34, "...", 3);
       strcpy(buffer + 37, C + L - 26);
     } else {
       memcpy(buffer + 8, C, L+1);
     }
     set_main_name(buffer);

     // Redraw
     set_active();
     draw();
     cursor_S();
   }
}

//----------------------------------------------------------------------------
// EdText::Event handlers
//----------------------------------------------------------------------------
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

   switch( E.detail ) {             // Handle mouse wheel
     case xcb::WT_PUSH:
       cursor_V(-3);
       break;

     case xcb::WT_PULL:
       cursor_V(3);
       break;

     default:
       break;
   }
}

virtual int                         // Return code, 0 if handled
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

virtual int                         // Return code, 0 if handled
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

//----------------------------------------------------------------------------
//
// Method-
//       EdText::key_input
//
// Purpose-
//       Handle keypress event
//
//----------------------------------------------------------------------------
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
       if( cursor_H(column + 1) ) {
         const char* buffer= active.get_buffer();
         putxy(getxy(col - 1, row), buffer + active.index(column));
       }
     } else {
       active.replace_char(column, key);
       if( cursor_H(column + 1) ) {
         char buffer[2];
         buffer[0]= char(key);
         buffer[1]= '\0';
         putxy(getxy(col - 1, row), buffer);
       }
     }
     cursor_S();
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
       cursor_C();                  // Clear the cursor
       if( column > 0 )
         column--;
       active.remove_char(column);
       int rc= cursor_H(column);
       if( rc ) {
         active.append_text(" ");   // (Clear removed character)
         const char* buffer= active.get_buffer(column - col_zero);
         putxy(getxy(col, row), buffer);
         cursor_S();
         flush();
       }
       break;
     }
     case 0x007f:                  // (Should not occur)
     case XK_Delete: {
       active.remove_char(column);
       active.append_text(" ");
       const char* buffer= active.get_buffer(column);
       putxy(getxy(col, row), buffer);
       cursor_S();
       flush();
       break;
     }
     case XK_Escape:                // Escape: UNDO
       active.undo();
       active.index(col_zero+col_size); // Blank fill
       putxy(getxy(0, row), active.get_buffer(col_zero));
       cursor_S();
       break;

     case XK_Insert: {              // Insert key
       xcb::keystate ^= xcb::KS_INS; // Invert the insert state
       break;
     }
     case XK_Return:
       cursor_V(+1);
       cursor_H(0);
       break;

     case XK_Tab: {                 // TODO: PRELIMINARY
       enum { TAB= 8 };             // TAB Column BRINGUP (( 2 ** N ))
       column += TAB;
       column &= ~(TAB - 1);
       cursor_H(column);
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
         cursor_H(column);
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
         set_file(file);
       break;
     }
     case XK_F8: {                  // Next file
       commit();
       EdFile* file= this->file->get_next();
       if( file == nullptr )
         file= Editor::editor->ring.get_head();
       if( file != this->file )
         set_file(file);
       break;
     }
     case XK_F12: {                 // Bringup test
       Editor::editor->do_test();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case XK_Home: {                // Home key
       cursor_C();
       col= 0;
       if( col_zero ) {
         col_zero= 0;
         draw();
       }
       cursor_S();
       break;
     }
     case XK_Left: {                // Left arrow
       if( column > 0 )
         cursor_H(column - 1);
       break;
     }
     case XK_Up: {                  // Up arrow
       cursor_V(-1);
       break;
     }
     case XK_Right: {               // Right arrow
       cursor_H(column + 1);
       break;
     }
     case XK_Down: {                // Down arrow
       cursor_V(1);
       break;
     }
     case XK_Page_Up: {             // Page_Up key
       cursor_C();
       commit();
       unsigned count= row_size;
       if( line->get_prev() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)line->get_prev();
           if( up == nullptr )
             break;

           line= up;
         }
         set_active();
         draw();
       }
       cursor_S();
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       cursor_C();
       commit();
       if( line != last ) {
         row_zero += row_used;
         line= last;
         set_active();
         draw();
       }
       cursor_S();
       break;
     }
     case XK_End: {                 // End key
       cursor_H(active.get_cols());
       break;
     }

     default:
       if( opt_hcdm || opt_verbose >= 0 ) debugf("0x%.6x Key ignored\n", key);
       rc= 1;
   }

   return rc;
}
}; // class EdText
#endif // EDTEXT_H_INCLUDED
