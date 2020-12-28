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
//       EdText.cpp
//
// Purpose-
//       Editor: Implement EdText.h
//
// Last change date-
//       2020/12/26
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For fprintf TODO: REMOVE
#include <sys/types.h>              // For
#include <pub/List.h>               // For pub::List

#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Trace.h>              // For pub::Trace

#include "Xcb/Active.h"             // For xcb::Active
#include "Xcb/Global.h"             // For xcb::trace
#include "Xcb/TextWindow.h"         // For xcb::TextWindow
#include "Xcb/Types.h"              // For xcb::DEV_EVENT_MASK

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
,  USE_BRINGUP= false               // Extra bringup diagnostics?
,  USE_HIDDEN= true                 // Use mouse hide/show? TODO: REMOVE
// USE_HIDDEN= false                // Use mouse hide/show? TODO: REMOVE
}; // Compilation controls

static struct EdText_initializer {  // TODO: REMOVE
   EdText_initializer( void )       // (DO NOT USE DEBUGF)
{  fprintf(stderr, "%4d EdText: Cursor hiding(%s)\n", __LINE__
                 , USE_HIDDEN ? "ENABLED" : "DISABLED");
}
}  static_initializer;

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const char      zero8[8]= {'\0','\0','\0','\0', '\0','\0','\0','\0'};

//----------------------------------------------------------------------------
// EdText::Constructor
//----------------------------------------------------------------------------
   EdText::EdText(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  TextWindow(parent, name ? name : "EdText")
{
   if( opt_hcdm )
     debugh("EdText(%p)::EdText\n", this);

   // Configure text colors
   bg= 0x00FFFFF0;                  // Ivory, pale yellow
   fg= 0x00000000;                  // Black

   // Reserve TOP line              // Message/Status/History line
   USER_TOP= 1;

   // Set window event mask
   emask= XCB_EVENT_MASK_KEY_PRESS
//      | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_MOTION
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
//      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
//      | XCB_EVENT_MASK_PROPERTY_CHANGE
        ;
}

//----------------------------------------------------------------------------
// EdText::Destructor
//----------------------------------------------------------------------------
   EdText::~EdText( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdText(%p)::~EdText\n", this);

   if( gc_chg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_chg) );
   if( gc_cmd ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_cmd) );
   if( gc_msg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_msg) );
   if( gc_sts ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_sts) );
   if( markGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, markGC) );
   markGC= 0;                       // (Prevents dupicate xcb_free_gc)

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
void
   EdText::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdText(%p)::debug(%s) Named(%s)\n", this, info ? info : ""
         , get_name().c_str());

   debugf("..gc_chg(%u) gc_cmd(%u) gc_msg(%u) gc_sts(%u) markGC(%u)\n"
         , gc_chg, gc_cmd, gc_msg, gc_sts, markGC);
   debugf("..motion[%d,%u,%d,%d]\n"
         , motion.state, motion.time, motion.x, motion.y);

   TextWindow::debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::activate
//
// Purpose-
//       Set the current file
//       Set the current line
//
//----------------------------------------------------------------------------
void
   EdText::activate(                // Activate
     EdFile*           act_file)    // This file
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   if( opt_hcdm )
     debugh("EdText(%p)::activate(%s)\n", this, act_file->get_name().c_str());

   // Trace activation
   Config::trace(".ACT", "file", act_file, file);

   // Out with the old
   if( file ) {
     data->commit();

     file->top_line= (EdLine*)this->line;
     file->col_zero= data->col_zero;
     file->row_zero= data->row_zero;
     file->col= data->col;
     file->row= data->row;
   }

   // In with the new
   editor::file= act_file;
   this->line= nullptr;
   if( act_file ) {
     this->last= this->line= act_file->top_line;
     data->col_zero= act_file->col_zero;
     data->row_zero= act_file->row_zero;
     data->col=  act_file->col;
     data->row=  act_file->row;
     if( data->row < USER_TOP )
       data->row= USER_TOP;

     // Update window title, omitting middle of file name if necessary
     char buffer[64];
     const char* C= act_file->name.c_str();
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

     // Synchronize
     synch_active();
   }
}

void
   EdText::activate(                // Activate
     EdLine*           act_line)    // This line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace activation
   Config::trace(".ACT", "line", data->cursor, act_line);

   // Activate
   undo_cursor();                   // Clear the character cursor
   data->commit();                  // Commit any active line
   data->active.reset(act_line->text); // Activate the new line
   data->cursor= act_line;          // "
   data->activate();                // Insure data view

   // Locate line on-screen
   EdLine* line= (EdLine*)this->line;
   for(unsigned r= USER_TOP; (r+1) < row_size; r++) { // Set the Active line
     if( line == act_line ) {
//     printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
       data->row= r;
       draw_cursor();
       draw_info();
       return;
     }

     EdLine* next= line->get_next();
     if( next == nullptr )
       break;

     line= next;
   }

   // Line off-screen. Locate line in file
   data->row_zero= 0;
   for( line= file->line_list.get_head(); line; line= line->get_next() ) {
     if( line == act_line ) {       // If line found
//     printf(" %4drow_zero(%zd) rows(%zd) row_size(%u) USER_TOP(%u)\n", __LINE__
//           , data->row_zero, file->rows, row_size, USER_TOP);

       // If near top of file
       if( data->row_zero < (row_size - USER_TOP) ) {
//       printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
         this->line= file->line_list.get_head();
         data->row= (unsigned)data->row_zero + USER_TOP;
         data->row_zero= 0;
         draw();
         return;
       }

       // If near end of file
       if( data->row_zero > (file->rows + 1 + USER_TOP - row_size ) ) {
//       printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
         data->row_zero= file->rows + 2 + USER_TOP - row_size;
         data->row= USER_TOP;
         unsigned r= row_size - 1;
         line= file->line_list.get_tail(); // "** END OF FILE **", rows + 1
         while( r > USER_TOP ) {
           if( line == act_line )
             data->row= r;
           line= line->get_prev();
           r--;
         }
         this->line= line;
         draw();
         return;
       }

       // Not near top nor end of file
//     printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
       unsigned r= row_size / 2;
       data->row= r;
       data->row_zero -= r - USER_TOP;
       while( r > USER_TOP ) {
         line= line->get_prev();
         r--;
       }
       this->line= line;
       draw();
       return;
     }

     data->row_zero++;
   }

   // Line is not in file (SHOULD NOT OCCUR)
   Config::alertf("%4d HCDM EdText file(%p) line(%p)", __LINE__
                 , file, act_line); // TODO: REMOVE
   data->cursor= line= file->line_list.get_head();
   data->col_zero= data->col= 0;
   data->row_zero= 0;
   data->row= USER_TOP;
   draw();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdText::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("EdText(%p)::configure\n", this);

   TextWindow::configure();         // Create the Window

   // Create the graphic contexts               // FG,       BG
   gc_chg= font.makeGC(0x00000000, 0x00F08080); // Black,    Light Pink
// gc_cmd= font.makeGC(0x00000000, 0x00FFC020); // Black,    Orange
   gc_cmd= font.makeGC(0x00000000, 0x0000FFFF); // Black,    Cyan
   gc_msg= font.makeGC(0x00900000, 0x00FFFF00); // Dark Red, Yellow
   gc_sts= font.makeGC(0x00000000, 0x0080F080); // Black,    Light Green
   markGC= font.makeGC(0x00000000, 0x00C0F0FF); // Black,    Light Blue

   // Configure views
   EdView* const data= editor::data;
   data->gc_flip= flipGC;
   data->gc_font= fontGC;
   data->gc_mark= markGC;
   EdHist* const hist= editor::hist;
   hist->gc_flip= flipGC;
   hist->gc_font= gc_cmd;
   hist->gc_mark= gc_cmd;

   // Set up WM_DELETE_WINDOW protocol handler
   protocol= name_to_atom("WM_PROTOCOLS", true);
   wm_close= name_to_atom("WM_DELETE_WINDOW");
   enqueue(__LINE__, "xcb_change_property"
          , xcb_change_property_checked
          ( c, XCB_PROP_MODE_REPLACE, widget_id
          , protocol, 4, 32, 1, &wm_close) );
   if( opt_hcdm )
     debugf("atom PROTOCOL(%d)\natom WM_CLOSE(%d)\n", protocol, wm_close);

// flush();                         // Not needed: draw scheduled
}

//----------------------------------------------------------------------------
//
// Methods-
//       EdText::draw_cursor
//
// Purpose-
//       Set or clear the screen cursor character
//
//----------------------------------------------------------------------------
void
   EdText::draw_cursor(bool set)    // Set the character cursor
{
   EdView* const view= editor::view;

   if( opt_hcdm && opt_verbose > 1 )
     debugh("EdText(%p)::cursor_%s cursor[%u,%u]\n", this
           , set ? "S" : "C", view->col, view->row);

   size_t column= view->col_zero + view->col; // The current column
   char buffer[8];                  // Encoder buffer
   pub::UTF8::Encoder encoder(buffer, sizeof(buffer));
   pub::UTF8::Decoder decoder(view->active.get_buffer(column));
   int code= decoder.decode();
   if( code <= 0 )
     code= ' ';
   encoder.encode(code);
   buffer[encoder.get_used()]= '\0';

   xcb_gcontext_t gc= set ? view->gc_flip : view->get_gc();
   putxy(gc, get_xy(view->col, view->row), buffer);

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::draw
//       EdText::draw_info
//       EdText::draw_message
//       EdText::draw_history
//       EdText::draw_status
//
// Purpose-
//       Draw the entire screen, data and info
//       Draw the information line: draw_message, draw_history, or draw_status
//         Draw the message line
//         Draw the history line
//         Draw the status line
//
//----------------------------------------------------------------------------
static void
   format6(
     size_t            value,
     char*             buffer)
{
   if( value > 10000000 )
     sprintf(buffer, "*%.6u", unsigned(value % 1000000));
   else
     sprintf(buffer, "%7zu", value);
}

static void
   format8(
     size_t            value,
     char*             buffer)
{
   if( value > 1000000000 )
     sprintf(buffer, "*%.8u", unsigned(value % 100000000));
   else
     sprintf(buffer, "%9zu", value);
}

bool                                // Return code, TRUE if handled
   EdText::draw_history( void )     // Redraw the history line
{
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( view != hist )               // If history not active
     return false;

   const char* buffer= hist->get_active();
   putxy(gc_cmd, 1, 1, buffer);
   draw_cursor();
   flush();
   return true;
}

void
   EdText::draw_info( void )        // Redraw the information line
{
   if( draw_message() ) return;
   if( draw_history() ) return;
   draw_status();
}

bool                                // Return code, TRUE if handled
   EdText::draw_message( void )     // Message line
{
   EdMess* mess= editor::file->mess_list.get_head();
   if( mess == nullptr ) return false;

   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
   strcpy(buffer, mess->mess.c_str());
   buffer[strlen(buffer)]= ' ';
   putxy(gc_msg, 1, 1, buffer);
   flush();
   return true;
}

void
   EdText::draw_status( void )      // Redraw the status line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
   // Offset:      012345678901234567890123456789012345678901234567890123456
   strcpy(buffer, "C[*******] L[*********,*********] [REP] [UNIX] EDIT V3.0");
   buffer[56]= ' ';                 // Full size (255) length buffer
   char number[16];                 // Numeric buffer
   format6(data->col_zero + data->col + 1, number);
   memcpy(buffer+2, number, 7);
   format8(data->row_zero + data->row - USER_TOP, number);
   memcpy(buffer+13, number, 9);
   format8(file->rows,     number);
   memcpy(buffer+23, number, 9);

   if( xcb::keystate & xcb::KS_INS ) // Set insert mode (if not REP)
     memcpy(buffer+35, "INS", 3);
   if( file->mode != EdFile::M_UNIX ) {
     if( file->mode == EdFile::M_DOS )
       memcpy(buffer+41, "=DOS", 4);
     else if( file->mode == EdFile::M_MIX )
       memcpy(buffer+41, "=MIX", 4);
     else if( file->mode == EdFile::M_BIN ) // Set file mode (if not UNIX)
       memcpy(buffer+41, "=BIN", 4);
   }

   xcb_gcontext_t gc= gc_sts;       // Set background/foreground GC
   if( file->changed || file->damaged || data->active.get_changed() )
     gc= gc_chg;
   putxy(gc, 1, 1, buffer);
   flush();
}

void
   EdText::draw( void )             // Redraw the Window
{
   if( opt_hcdm )
     debugh("EdText(%p)::draw\n", this);

   TextWindow::draw(editor::data->col_zero); // Draw the text screen
   draw_info();                     // Draw the information line

   draw_cursor();
   show();
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::get_text
//
// Purpose-
//       Return the line text, which differs for the cursor line
//
//----------------------------------------------------------------------------
const char*                         // The text
   EdText::get_text(                // Get text
     xcb::Line*        line)        // For this Line
{
   const char* text= line->text;    // Default, use line text
   if( line == editor::data->cursor ) // If this is the cursor line
     text= editor::data->active.get_buffer(); // Return the active buffer
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::grab_mouse
//       EdText::hide_mouse
//       EdText::show_mouse
//
// Purpose-
//       Grab the mouse cursor
//       Hide the mouse cursor
//       Show the mouse cursor
//
//----------------------------------------------------------------------------
void
   EdText::grab_mouse( void )       // Grab the mouse cursor
{
// uint16_t mask= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
// uint32_t parm[2];
// parm[0]= 0;
// parm[1]= 0;
// NOQUEUE("xcb_configure_windowr", xcb_configure_window
//        (c, widget_id, mask, parm) );

   NOQUEUE("xcb_warp_pointer", xcb_warp_pointer
          (c, XCB_NONE, widget_id, 0,0,0,0, rect.width/2, rect.height/2) );
   flush();
}

void
   EdText::hide_mouse( void )       // Hide the mouse cursor
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;
     flush();
   }
}

void
   EdText::show_mouse( void )       // Show the mouse cursor
{
   if( motion.state != CS_VISIBLE ) { // If not already visible
     NOQUEUE("xcb_show_cursor", xcb_xfixes_show_cursor(c, widget_id) );
     motion.state= CS_VISIBLE;
     flush();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   EdText::move_cursor_H(           // Move cursor horizontally
     size_t            column)      // The (absolute) column number
{
   int rc= 1;                       // Default, draw not performed

   undo_cursor();                   // Clear the current cursor

   EdView* const view= editor::view;
   size_t current= view->col_zero + view->col; // Set current column
   unsigned col_move= col_size / 8; // MINIMUM shift size
   if( col_move == 0 ) col_move= 1;
   if( column < current ) {         // If moving cursor left
     if( column < view->col_zero ) { // If shifting left
       rc= 0;                       // Redraw required
       if( column <= (col_size - col_move) )
         view->col_zero= 0;
       else
         view->col_zero= column - col_move;
     }
   } else if( column > current ) {  // If moving right
     if( column >= (view->col_zero + col_size ) ) { // If shifting right
       rc= 0;                       // Redraw required
       view->col_zero= column - col_size + col_move;
     }
   }
   view->col= unsigned(column - view->col_zero);

   if( rc ) {                       // If full redraw not needed
     draw_cursor();                 // Just set cursor
     draw_info();                   // Update status line
   } else {                         // If full redraw needed
     if( view == editor::data )     // If data view (draw_info included)
       draw();
     else
       draw_info();
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::synch_active
//
// Purpose-
//       Set the Active (cursor) line to the current row.
//
// Inputs-
//       this->line= top screen line
//       data->row   screen row
//
//----------------------------------------------------------------------------
void
   EdText::synch_active( void )     // Set the Active (cursor) line
{
   EdView* const data= editor::data;

   data->cursor= nullptr;           // Default, no cursor
   EdLine* line= (EdLine*)this->line; // Get the first line
   if( line == nullptr ) {          // If Empty      // TODO: SHOULD NOT OCCUR
     Config::alertf("%4d HCDM EdText\n", __LINE__); // TODO: REMOVE
     return;
   }

   if( data->row < USER_TOP )
     data->row= USER_TOP;

   for(unsigned r= USER_TOP; ; r++) { // Set the Active line
     if( r == data->row ) {
       data->cursor= line;
       break;
     }

     EdLine* next= line->get_next();
     if( next == nullptr ) {        // (Can occur if window grows)
       data->row= r;
       data->cursor= line;
       break;
     }

     if( (r + 1) >= row_size ) {    // (Can occur if window shrinks)
       data->row= r;
       data->cursor= line;
       break;
     }

     line= next;
   }

   data->active.reset(data->cursor->text);
   draw_cursor();                   // Set the cursor
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
   EdText::key_alt(                 // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // ALT-
     case 'C': {                    // Copy
       const char* error= mark->copy();
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->paste(file, data->cursor);
       draw();
       break;
     }
     case 'D': {                    // Delete the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     case 'I': {                    // Insert
       data->commit();
       EdLine* cursor= data->cursor; // The current cursor line
       if( cursor->get_next() == nullptr ) // If it's the last line
         cursor= cursor->get_prev(); // Use the prior line instead

       EdRedo* redo= new EdRedo();
       EdLine* tail;
       EdLine* head= tail= file->new_line(); // Get new, empty insert line
       if( memcmp(cursor->delim, zero8, 2) == 0 ) { // If after no delimiter
         // The no delimiter line can only be the last data line of a file.
         pub::List<EdLine> list;    // (Used to connect head and tail)
         head= file->new_line(cursor->text); // Get cursor line replacement
         list.fifo(head);
         list.fifo(tail);

         // Remove the cursor from the file, updating the REDO
         file->remove(cursor);
         redo->head_remove= redo->tail_remove= cursor;
         cursor= cursor->get_prev(); // (cursor->get_prev() valid after remove)
       }

       data->col_zero= data->col= 0;
       file->insert(cursor, head, tail);
       redo->head_insert= head;
       redo->tail_insert= tail;
       file->insert_undo(redo);
       file->activate(tail);        // (Activate the newly inserted line)
       draw();                      // And redraw
       break;
     }
     case 'L':                      // Mark line
       editor::put_message( mark->mark(file, data->cursor) );
       draw();
       break;

     case 'M': {                    // Move (Uses cut/paste)
       const char* error= mark->cut();
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->paste(file, data->cursor);
       draw();
       break;
     }
     case 'Q':                      // (Safe) quit
       if( editor::un_changed() )
         editor::exit();
       break;

     case 'U': {                    // Undo mark
       EdFile* mark_file= mark->file;
       mark->undo();
       if( file == mark_file )
         draw();
       break;
     }
     default:
       editor::put_message("Invalid key");
       draw_info();
   }
}

void
   EdText::key_ctl(                 // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // CTRL-
     case 'C': {                    // Copy
       editor::put_message( mark->copy() );
       break;
     }
     case 'V': {                    // Paste
       const char* error= mark->paste(file, data->cursor);
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
       draw_info();
   }
}

// Handle protected line. Disallow keys which modify text.
int                                 // Return code, TRUE if error message
   EdText::key_protected(           // Handle this protected line
     xcb_keysym_t      key,         // Input key
     int               state)       // Alt/Ctl/Shift state mask
{
   if( key >= 0x0020 && key < 0x007F ) { // If text key
     int mask= state & (xcb::KS_ALT | xcb::KS_CTRL);

     if( mask ) {
       if( mask == xcb::KS_ALT ) {
         key= toupper(key);
         switch(key) {
           case 'I':                  // INSERT allowed
           case 'Q':                  // QUIT allowed
           case 'U':                  // EdMark::reset allowed
             return false;
             break;

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
         break;
     }
   }

   editor::put_message("Protected line");
   return true;
}

void
   EdText::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( opt_hcdm ) {
     char B[2]; B[0]= '\0'; B[1]= '\0'; const char* K= B;
     if( key >= 0x0020 && key < 0x007F ) B[0]= char(key);
     else K= editor::key_to_name(key);
     debugh("EdText(%p)::key_input(0x%.4x,%.4x) '%s'\n", this
           , key, state, K);
   }

   const char* name= editor::key_to_name(key);
   Config::trace(".KEY", (state<<16) | (key & 0x0000ffff), name);

   // Handle protected line
   if( view == data ) {             // Only applies to data view
     if( data->cursor->flags & EdLine::F_PROT // If protected line
         && key_protected(key, state) ) // And modification key
       return;                      // (Disallowed)
   }

   // Handle input key
   file->rem_message_type();        // Remove informational message
   if( file->mess_list.get_head() ) { // If any message remains
     draw_info();
     return;
   }

   size_t column= view->col_zero + view->col; // The cursor column
   if( key >= 0x0020 && key < 0x007F ) { // If text key
     int mask= state & (xcb::KS_ALT | xcb::KS_CTRL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case xcb::KS_ALT:
           key_alt(key);
           break;

         case xcb::KS_CTRL:
           key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTRL)
           editor::put_message("Invalid key");
       }
       return;
     }

     if( xcb::keystate & xcb::KS_INS ) { // If Insert state
       view->active.insert_char(column, key);
       if( move_cursor_H(column + 1) ) {
         const char* buffer= view->active.get_buffer();
         putxy(view->get_gc(), get_xy(view->col - 1, view->row), buffer + view->active.index(column));
       }
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
       ;;                           // (Silently Ignore)
       break;

     case XK_BackSpace: {
       undo_cursor();               // Clear the cursor
       if( column > 0 )
         column--;
       view->active.remove_char(column);
       int rc= move_cursor_H(column);
       if( rc ) {
         view->active.append_text(" "); // (Clear removed character)
         const char* buffer= view->active.get_buffer(column - view->col_zero);
         putxy(view->get_gc(), get_xy(view->col, view->row), buffer);
         draw_cursor();
         flush();
       }
       break;
     }
     case 0x007F:                  // (Should not occur)
     case XK_Delete: {
       view->active.remove_char(column);
       view->active.append_text(" ");
       const char* buffer= view->active.get_buffer(column);
       putxy(view->get_gc(), get_xy(view->col, view->row), buffer);
       draw_cursor();
       flush();
       break;
     }
     case XK_Escape:                // Escape: Invert history view
       editor::do_history();
       break;

     case XK_Insert: {              // Insert key
       xcb::keystate ^= xcb::KS_INS; // Invert the insert state
       draw_info();
       break;
     }
     case XK_Return: {
       move_cursor_H(0);
       if( view == data )
         data->move_cursor_V(+1);
       else
         hist->commit();
       break;
     }
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
       printf(" F1: This help message\n"
              " F2: Bringup test\n"
              " F3: Quit File\n"
              " F4: Test changed\n"
              " F5: Locate\n"
              " F6: Change\n"
              " F7: Previous File\n"
              " F8: Next File\n"
              " F9: Quick debug\n"
              "F10: Line to top\n"
              "F11: Undo\n"
              "F12: Redo\n"
              "A-I: Insert\n"
              "A-Q: Quit\n"
       );
       break;
     }
     case XK_F2: {                  // Bringup test
       editor::do_test();
       break;
     }
     case XK_F3: {                  // (Safe) quit
       data->commit();
       editor::do_exit();
       break;
     }
     case XK_F4: {                  // Test changed
       if( editor::un_changed() )
         editor::put_message("No files changed");
       break;
     }
     case XK_F5: {
       editor::put_message(editor::do_locate());
       break;
     }
     case XK_F6: {
       editor::put_message(editor::do_change());
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
     case XK_F9: {                  // Quick debug. TODO: REMOVE/REPLACE
       pub::Trace* trace= pub::Trace::trace;
       if( trace ) {
         if( trace->flag[pub::Trace::X_HALT] ) {
           Config::errorf("Tracing resumed\n");
           trace->flag[pub::Trace::X_HALT]= false;
           return;
         }

         trace->flag[pub::Trace::X_HALT]= true;
       } // else Config::errorf("Tracing inactive\n");

       Config::alertf("F9");
       break;
     }
     case XK_F10: {                 // Line to top
       line= data->cursor;
       data->row_zero += (data->row - USER_TOP);
       data->row= USER_TOP;
       draw();
       break;
     }
     case XK_F11: {                 // Undo
       if( view->active.undo() ) {
         view->active.index(view->col_zero+col_size); // Blank fill
         putxy(view->get_gc(), get_xy(0, view->row), view->active.get_buffer(view->col_zero));
         draw_info();
         draw_cursor();
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
       undo_cursor();
       data->commit();
       unsigned count= row_size - (USER_TOP + USER_BOT);
       if( line->get_prev() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)line->get_prev();
           if( up == nullptr )
             break;

           data->row_zero--;
           line= up;
         }
         synch_active();
         draw();
       }
       draw_cursor();
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       undo_cursor();
       data->commit();
//     if( line != last ) {
//       data->row_zero += row_used - USER_TOP;
//       line= last;
//       synch_active();
//       draw();
//     }
       unsigned count= row_size - (USER_TOP + USER_BOT);
       if( line->get_next() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)line->get_next();
           if( up == nullptr )
             break;

           data->row_zero++;
           line= up;
         }
         synch_active();
         draw();
       }
       draw_cursor();
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

   // Use E.detail and xcb::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm )
     debugh("button:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t current_col= view->col_zero + view->col; // The current column number
   unsigned button_row= get_row(E.event_y); // (Absolute) button row
   switch( E.detail ) {
     case xcb::BT_LEFT: {           // Left button
       if( button_row < USER_TOP ) { // If on command/history line
         if( file->rem_message() ) { // If message removed
           draw_info();
           break;
         }

         if( view == hist )         // If history active
           move_cursor_H(hist->col_zero + get_col(E.event_x)); // Update column
         else
           hist->activate();
         draw_info();
         break;
       }

       if( view == hist ) {         // If history active
         data->activate();
         draw_info();
       }

       if( button_row != view->row ) // If row changed
         data->move_cursor_V(button_row - view->row); // Set new row
       move_cursor_H(view->col_zero + get_col(E.event_x)); // Set new column
       break;
     }
     case xcb::BT_RIGHT: {          // Right button
       if( button_row < USER_TOP ) {
         if( file->rem_message() ) { // If message removed
           draw_info();
           break;
         }

         // Invert history command line mode
         editor::do_history();
       }
       break;
     }
     case xcb::WT_PUSH:             // Mouse wheel push (away)
       view->move_cursor_V(-3);
       break;

     case xcb::WT_PULL:             // Mouse wheel pull (toward)
       view->move_cursor_V(+3);
       break;

     case xcb::WT_LEFT:             // Mouse wheel left
       move_cursor_H(current_col > 3 ? current_col - 3 : 0);
       break;

     case xcb::WT_RIGHT:            // Mouse wheel right
       move_cursor_H(current_col + 3);
       break;

     case xcb::BT_CNTR:             // Middle button (ignored)
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
   EdText::motion_notify(           // Handle this
     xcb_motion_notify_event_t* E)  // Motion notify event
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("motion: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   // printf("."); fflush(stdout);  // See when called
   if( E->event_x != motion.x || E->event_y != motion.y )
     { if( USE_HIDDEN ) show_mouse(); } // TODO: REMOVE USE_HIDDEN
   else {
     if( (E->time - motion.time) < 1000 ) // If less than 1 second idle
       return;                      // Ignore
     { if( USE_HIDDEN ) hide_mouse(); } // TODO: REMOVE USE_HIDDEN
   }

   motion.time= E->time;
   motion.x= E->event_x;
   motion.y= E->event_y;
}
