//----------------------------------------------------------------------------
//
//       Copyright (C) 2024 Frank Eskesen.
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
//       2024/05/11
//
// Implementation notes-
// Term: TODO: On Fedora, UTF8 characters display as separate characters, so
//             lines spill over into next line. **UNUSABLE**
//             (But cat UTF8.html everything OK but Amharic language)
// Term: TODO: Utf8 combining characters: characters combine, cursor wrong
// Xcb:  TODO: Utf8 combining characters: does not combine
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types

#include <ncurses.h>                // For ncurses (== curses.h)
#include <term.h>                   // For ncurses terminal
#  undef set_clock                  // (MACRO in term.h)

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Utf.h>                // For pub::Utf::UNI_REPLACEMENT
#include <pub/utility.h>            // For pub::utility::visify

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps, implemented
#include "EdMark.h"                 // For EdMark
#include "EdOuts.h"                 // For EdInps, derived class
#include "EdType.h"                 // For GC_t

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace
using pub::utility::visify;         // For pub::utility::visify
using std::string;                  // Using std::string

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Controls
// IO_TRACE is used to optimize out opt_hcdm checks
,  IO_TRACE= true                   // I/O trace mode?

// MAX_COLOR= the ncurses color saturation value (Determined experimentally)
,  MAX_COLOR= 1000                  // Maximum  color value

,  USE_UTF8= false                  // Use (TODO) UTF-8 code?
}; // Compilation controls

enum // Key definitions
{  KEY_ESC= '\x1B'                  // ESCape key
,  KEY_TAB= '\t'                    // TAB key
}; // Key definitions

// Conditionally defined keys. (If not implemented, we'll have unused code)
#ifndef KEY_MOUSE
#define KEY_MOUSE  0631
#endif

#ifndef KEY_RESIZE
#define KEY_RESIZE 0632
#endif

enum MOUSE_BUTTONS                  // NCURSES mouse buttons
{  MB_0=               0x00000000   // (Dummy entry)

// Mouse buttons
,  MB_LEFT=            0x00000006   // Left mouse button
,  MB_LEFT_CLICK=      0x00000004   // Left mouse button   (click)
,  MB_LEFT_PRESS=      0x00000002   // Left mouse button   (press)
,  MB_LEFT_RELEASE=    0x00000001   // Left mouse button   (release)
,  MB_CENTER=          0x000000C0   // Center mouse button
,  MB_CENTER_CLICK=    0x00000080   // Center mouse button (click)
,  MB_CENTER_PRESS=    0x00000040   // Center mouse button (press)
,  MB_CENTER_RELEASE=  0x00000020   // Center mouse button (release)
,  MB_RIGHT=           0x00001800   // Right mouse button
,  MB_RIGHT_CLICK=     0x00001000   // Right mouse button  (click)
,  MB_RIGHT_PRESS=     0x00000800   // Right mouse button  (press)
,  MB_RIGHT_RELEASE=   0x00000400   // Right mouse button  (release)
,  MB_ANY_BUTTON=      0x00001CE7   // Any button, any action

// Mouse wheel
,  MB_PULL=            0x00200000   // Mouse wheel PULL
,  MB_PUSH=            0x00010000   // Mouse wheel PUSH
}; // enum MOUSE_BUTTONS

//----------------------------------------------------------------------------
// Imports
//----------------------------------------------------------------------------
enum // Imported
{  KS_ALT= EdUnit::KS_ALT
,  KS_CTL= EdUnit::KS_CTL
}; // Imported

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Control keys G..M (encoded as 0x07..0x0d) are not passed to application
                                      // 123456789abcdef0123456789a
static constexpr const char* alt_table= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static constexpr const char* ctl_table= "ABCDEF*******NOPQRSTUVWXYZ";

// EdInps::at_exit controls
static int             nc_active= false; // Is curses active?

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const char*            EdUnit::EDITOR= "editerm";
const char*            EdUnit::DEFAULT_CONFIG=
   "[Program]\n"
   "URL=https://github.com/franke-hub/SDL/tree/trunk/src/cpp/Edit/Term\n"
   "Exec=Edit ; Edit in read-write mode\n"
   "Exec=View ; Edit in read-only mode\n"
   "Purpose=NCURSES based text editor\n"
   "Version=1.1.0\n"
   "\n"
   "[Options]\n"
   ";; (Defaulted) See sample: ~/src/cpp/Edit/Term/.SAMPLE/Edit.conf\n"
   ;

//----------------------------------------------------------------------------
// EdInps.hpp: Included ONLY for compilation check, otherwise unused
//----------------------------------------------------------------------------
#include "EdInps.hpp"               // Contains UNUSED struct key_definitions

//----------------------------------------------------------------------------
//
// Exception-
//       curses_err
//
// Purpose-
//       Indicate curses error
//
//----------------------------------------------------------------------------
struct curses_err : public std::runtime_error {
   using std::runtime_error::runtime_error;
};

//----------------------------------------------------------------------------
//
// Struct-
//       Color
//
// Purpose-
//       RGB color type
//
//----------------------------------------------------------------------------
struct Color {                      // RGB color
uint32_t               rgb;         // The RGB color

   Color(uint32_t color= 0)         // Constructor
:  rgb(color) {}

Color
   operator=(uint32_t color)        // Assignment operator
{  rgb= color; return *this; }

short                               // The RED color component
   red( void )
{  return (rgb >> 16) & 0x00FF; }

short                               // The GREEN color component
   green( void )
{  return (rgb >>  8) & 0x00FF; }

short                               // The BLUE color component
   blue( void )
{  return (rgb >>  0) & 0x00FF; }
}; // struct Color

//----------------------------------------------------------------------------
//
// Struct-
//       putcr_record
//
// Purpose-
//       putcr trace record
//
//----------------------------------------------------------------------------
struct putcr_record {
enum { DATA_SIZE= 32 };             // The output data length
char                   ident[4];    // The trace type identifier     ".PUT"
char                   unit[4];     // The trace data sub-identifier "data"
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
uint32_t               col;         // The screen (X) column
uint32_t               row;         // The screen (Y) row
uint32_t               _0018;       // Reserved/unused
uint32_t               length;      // The output data length
char                   data[DATA_SIZE]; // The output data
}; // struct putcr_record

//----------------------------------------------------------------------------
//
// Subroutine-
//       init_program_modes
//
// Purpose-
//       Initialize ncurses program modes.
//
// Implementation notes-
//       Should use cbreak() or raw(), but not both.
//
//----------------------------------------------------------------------------
static inline void
   init_program_modes(              // Initialize the ncurses program modes
     WINDOW*           win)         // (For this WINDOW)
{
   raw();                           // (Don't use key translation)
   // cbreak();                     // (Don't wait for '\n', conflicts w/ raw)

   keypad(win, true);               // (Use keypad translation?)
   meta(win, true);                 // (Use 8-bit input key characters)
   mousemask(ALL_MOUSE_EVENTS, nullptr); // Report mouse events (not position)
   scrollok(win, false);            // (Use automatic scrolling?)

   // idlok(win, false);            // (Do not use hardware insert/delete line?)
   curs_set(1);                     // (Use visible cursor)
   intrflush(win, false);           // (Do not flush on interrupt)
   noecho();                        // (Do not echo)
   nonl();                          // (Do not convert '\r' to '\n')
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_text_key
//
// Purpose-
//       Is the key a text key?
//
// Implementation notes-
//       DOES NOT check KS_ALT or KS_CTL modifiers
//
//----------------------------------------------------------------------------
static bool
   is_text_key(                     // Is key a text key?
     uint32_t          key,         // Input key
     int               state)       // ESC + Alt/Ctl/Shift state mask
{
   if( state & EdUnit::KS_ESC ) {   // If escape mode
     if( key == '\b' || key == '\t' || key == KEY_ESC )
       return true;
   }

   if( key >= 0x0020 && key < 0x007F ) // If standard ASCII text key
     return true;

   return false;                    // (Key_0x7f treated as BACKSPACE)
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
     uint32_t          key,         // Input key
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
       case '\b':                   // (BACKSPACE)
//     KEY_BASKSPACE:               // (BACKSPACE, already translated)
       case 0x007F:                 // (DELete encoding)
       case KEY_DC:                 // (Delete)
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
// Subroutine-
//       key_to_name
//
// Purpose-
//       Convert key to its name.
//
//----------------------------------------------------------------------------
static const char*                  // The name of the key
   key_to_name(int key)             // Convert key to name
{
   static char buffer[16];          // (Static) return buffer
   static const char* F_KEY= "123456789ABCDEF";

   if( key >= 0x0020 && key <= 0x007f ) { // If text key (but not TAB or ESC)
     buffer[0]= char(key);
     buffer[1]= '\0';
     return buffer;
   }

   if( key >= KEY_F(1) && key <= KEY_F(15) ) { // If function key
     buffer[0]= 'F';
     buffer[1]= F_KEY[key - KEY_F(1)];
     buffer[2]= '\0';
     return buffer;
   }

   switch( key ) {                  // Handle control key
     case '\b':
       return "\\b";

     case KEY_BACKSPACE:
       return "(Encoded) \\b";

     case KEY_ESC:
       return "\\e (KEY_ESC)";      // ('\e' doesn't exist)

     case '\n':
       return "\\n";

     case KEY_ENTER:
       return "(Encoded) Enter";

     case '\r':
       return "\\r";

     case '\t':
       return "\\t (KEY_TAB)";

     case '\x7F':
       return "DEL char";

     case KEY_DOWN:
       return "Down arrow";

     case KEY_UP:
       return "Up arrow";

     case KEY_LEFT:
       return "Left arrow";

     case KEY_RIGHT:
       return "Right arrow";

     case KEY_BTAB:
       return "Back tab";

     case KEY_DC:
       return "Delete (key)";

     case KEY_DL:
       return "Delete-line";

     case KEY_END:
       return "End";

     case KEY_HOME:
       return "Home";

     case KEY_IC:
       return "Insert";

     case KEY_IL:
       return "Insert-line";

     case KEY_MOUSE:
       return "Mouse button";

     case KEY_NPAGE:
       return "Page down";

     case KEY_PPAGE:
       return "Page up";

     case KEY_RESIZE:
       return "Resize event";

     default:
       break;
   }

   sprintf(buffer, "\\x%.2x", key);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_init_color
//
// Purpose-
//       Initialize a color
//
//----------------------------------------------------------------------------
static inline void
   nc_init_color(                   // Initialize a color
     int               ix,          // The color index
     Color             rgb)         // The Color
{  if( IO_TRACE && opt_hcdm )
     traceh("init_color(%d, 0x%.6X) {%d,%d,%d}\n", ix, rgb.rgb
           , rgb.red(), rgb.green(), rgb.blue() );

   int r= rgb.red()   * MAX_COLOR / 255;
   int g= rgb.green() * MAX_COLOR / 255;
   int b= rgb.blue()  * MAX_COLOR / 255;
   int cc= init_extended_color(ix, r, g, b);

   if( IO_TRACE && opt_hcdm )
     traceh("%d= init_extended_color(%d,%d,%d,%d)\n", cc, ix, r, g, b);
   if( cc == ERR )
     throw curses_err("init_color");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_init_pair
//
// Purpose-
//       Initialize a color pair
//
// Implemenation notes-
//       COLOR_PAIR and COLOR numbers are pre-assigned
//       GC: The COLOR_PAIR number
//       GC+0: The foreground COLOR number
//       GC+1: The background COLOR number
//
//----------------------------------------------------------------------------
static inline void
   nc_init_pair(                    // Initialize a color pair
     GC_t              GC,          // The graphic context
     Color             fg,          // The foreground Color
     Color             bg)          // The background Color
{  if( IO_TRACE && opt_hcdm )
     traceh("nc_init_pair(%d,0x%.6X,0x%.6X)\n", GC, fg.rgb, bg.rgb);

   nc_init_color(GC+0, fg);
   nc_init_color(GC+1, bg);

   int cc= init_pair(short(GC), short(GC+0), short(GC+1));

   if( IO_TRACE && opt_hcdm )
     traceh("%d= init_pair(%d,%d,%d)\n", cc, GC, GC+0, GC+1);
   if( cc == ERR )
     throw curses_err("init_pair");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trace_keystroke
//       trace_every_keystroke
//
// Purpose-
//       Diagnostic: trace keystroke
//
//----------------------------------------------------------------------------
static inline void
   trace_keystroke(                 // Trace actual read operations
     int               key,         // Input key
     uint32_t          state)       // Input key state
{
   traceh("'%s'= '%c'= 0:%.4o= 0x%.4x= read()%s%s\n", key_to_name(key)
         , key < 256 && isprint(key) ? key : '~', key, key
         , state & KS_ALT ? "-ALT" : ""
         , state & KS_CTL ? "-CTL" : ""
   );
}

static inline void
   trace_every_keystroke(           // Trace actual read operations
     int               key,         // Input key
     uint32_t          state)       // Input key state
{  if( IO_TRACE && opt_hcdm && opt_verbose > 1 ) trace_keystroke(key, state); }

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::Init::initialize
//       EdUnit::Init::terminate
//       EdUnit::Init::at_exit
//
// Purpose-
//       Initialize the EdUnit
//       Terminate  the EdUnit
//       Abnormal termination handler
//
//----------------------------------------------------------------------------
EdUnit*                             // The EdUnit
   EdUnit::Init::initialize( void ) // Initialize an EdUnit
{
   return new EdOuts();             // The associated EdUnit
}

void
   EdUnit::Init::terminate(         // Terminate
     EdUnit*           unit)        // This EdUnit
{
   delete unit;                     // Delete the EdUnit
}

void
   EdUnit::Init::at_exit( void )    // (Idempotent) termination handler
{  if( opt_hcdm )
     traceh("EdUnit::Init::at_exit(%s)\n", nc_active ? "true" : "false");

   if( nc_active ) {                // Is ncurses active?
     resetty();                     // Reset the keyboard
     endwin();                      // Terminate the NCURSES window

     nc_active= false;
   }
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
   EdInps::EdInps( void )           // Constructor
{  if( opt_hcdm )
     traceh("EdInps(%p)::EdInps\n", this);

   atexit(Init::at_exit);           // Set termination handler
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::~EdInps
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdInps::~EdInps( void )          // Destructor
{  if( opt_hcdm )
     traceh("EdInps(%p)::~EdInps\n", this);

   Init::at_exit();                // Terminate ncurses

   delete editor::data;            // Delete the views and the mark
   delete editor::hist;
   delete editor::mark;

   // Note: editor::view equals editor::data, editor::view, or nullptr
   editor::data= nullptr;
   editor::hist= nullptr;
   editor::mark= nullptr;
   editor::view= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::init
//
// Purpose-
//       Initialize
//
// Implementation notes-
//       We cannot initialize until *after* Config::parser invocation because
//       the parser sets the variables we need to initialize.
//       Our constructor is invoked *before* Config::parser invocation.
//
// Implementation notes-
//       We set the TERM environment variable to "xterm-256color".
//       This works OK (at least for now) and avoids having to implement for
//       multiple environments.
//
//----------------------------------------------------------------------------
void
   EdInps::init( void )             // Initializer
{  if( opt_hcdm )
     traceh("EdInps(%p)::init\n", this);

   if( HCDM || opt_hcdm ) {
     debug_set_mode(pub::Debug::MODE_INTENSIVE);
     traceh("%s %s %s Hard Core Debug Mode\n", __FILE__, __DATE__, __TIME__);
   }

   // Must be done before initscr()
   // xterm256-color: has_colors()==true, can_change_color()==true
   setenv("TERM", "xterm-256color", 1);
   if( false  )
     slk_init(false);               // Not using special line keys
   setlocale(LC_CTYPE, "");         // (Support UTF-8)

   // Initialize NCURSES
   win= initscr();                  // Initialize NCURSES window
   nc_active= true;                 // NCURSES active

   start_color();                   // Use color features
   init_program_modes(win);         // Initialize settings
   def_prog_mode();                 // (Save modes as "program" modes)

   getmaxyx(win, row_size, col_size); // Set screen size
   wsetscrreg(win, 0, row_size-1);  // Set scrolling region

   if( !has_colors() ) {            // If monochrome screen
     fprintf(stderr, "Terminal color support is required\n");
     return;
   }

   if( !can_change_color() ) {      // If monochrome screen
     fprintf(stderr, "Terminal color change support is required\n");
     return;
   }

   nc_init_color(bg_chg, config::change_bg); // bg_chg is a COLOR, not a GC_t
   nc_init_color(bg_sts, config::status_bg); // bg_sts is a COLOR, not a GC_t

   nc_init_pair(gc_font, config::text_fg, config::text_bg);
   int fg= gc_font+0;               // (Per nc_init_pair convention)
   int bg= gc_font+1;

   nc_init_pair(gc_flip, config::text_bg,   config::text_fg);
   nc_init_pair(gc_mark, config::mark_fg,   config::mark_bg);
   nc_init_pair(gc_chg, config::change_fg,  config::change_bg);
   nc_init_pair(gc_msg, config::message_fg, config::message_bg);
   nc_init_pair(gc_sts, config::status_fg,  config::status_bg);

   assume_default_colors(fg, bg);
   bkgdset(' ');                    // Set the background
   set_escdelay(50);                // MINIMAL escape character delay

   // Initialize views
   editor::data= new EdData();      // Data view
   editor::hist= new EdHist();      // History view
   editor::mark= new EdMark();      // Mark handler
   editor::view= editor::hist;      // (Initial view)

   // Copy the predefined values into the Views
   editor::data->gc_flip= gc_flip;
   editor::data->gc_font= gc_font;
   editor::data->gc_mark= gc_mark;

   editor::hist->gc_chg=  gc_chg;
   editor::hist->gc_sts=  gc_sts;

   // We are ready to rumble
   operational= true;
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
   tracef("EdInps(%p)::debug(%s)\n", this, info ? info : "");

   tracef("..head(%p) tail(%p) col_size(%u) row_size(%u) row_used(%u)\n"
         , head, tail, col_size, row_size, row_used);
   tracef("..key_state(0x%.8X)%s%s\n", key_state
         , key_state & KS_INS ? "-INS" : "", key_state & KS_ESC ? "-ESC" : ""
   );
   tracef("..motion(%d,%d,%d)\n", motion.state, motion.x, motion.y);
   tracef("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n", gc_font, gc_flip, gc_mark);
   tracef("..bg_chg(%u)  bg_sts(%u)\n", bg_chg, bg_sts);
   tracef("..gc_chg(%u)  gc_msg(%u)  gc_sts(%u)\n", gc_chg, gc_msg, gc_sts);
   tracef("..operational(%d) poll_char(0x%.4X)\n", operational, poll_char);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::flush
//
// Purpose-
//       Complete an operation
//
// Implementation notes-
//       Not normally required: Next poll automatically flushes.
//
//----------------------------------------------------------------------------
void
   EdInps::flush( void )            // Complete an operation
{  if( opt_hcdm )
     traceh("EdInps(%p)::flush()\n", this);

   wrefresh(win);
}

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
     uint32_t          key)         // Alt_Key input event
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
       key_state |= KS_ESC;         // Next key is escaped
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
     uint32_t          key)         // Control key input event
{
   switch(key) {                    // CTRL-
     case 'C': {                    // Copy the mark into the stash
       op_mark_stash();
       break;
     }
     case 'Q': {                    // Quit (safely)
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
//       EdInps::key_input
//
// Purpose-
//       Input key handler
//
//----------------------------------------------------------------------------
void
   EdInps::key_input(               // Handle this
     uint32_t          key,         // The input key
     uint32_t          state)       // The Alt/Ctl/Shift state mask
{  if( opt_hcdm && opt_verbose > 0 )
     traceh("EdInps(%p)::key_input(0x%.4X,0x%.8X) '%s%s%s'\n", this
           , key, state
           , (state & KS_ALT) ? "ALT-" : ""
           , (state & KS_CTL) ? "CTL-" : ""
           , key_to_name(key)
     );

   EdData* const data= editor::data;
   EdFile* const file= editor::file;
   EdView* const view= editor::view;

   // Diagnostics
   const char* key_name= key_to_name(key);
   Trace::trace(".KEY", (state | key), key_name);

   // Key translations
   if( key == KEY_BACKSPACE         // (On xterm)
       || key == 0x007F )           // (On Cygwin terminal)
     key= '\b';

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
     } else {
       view->active.replace_char(column, key);
     }
     move_cursor_H(column + 1);
     draw_top();
     show_cursor();

     // Escape complete; "No File Changed" message complete
     key_state &= ~(KS_ESC | KS_NFC);
     return;
   }

   // Handle action key
   switch( key ) {                  // Handle data key
//   case 0x007F:                   // (DEL char, already translated)
//   case KEY_BACKSPACE:            // (BACKSPACE, already translated)
     case '\b': {                   // (Backspace)
       op_key_backspace();
       break;
     }
     // While defined, KEY_Break and KEY_Pause are inactive in curses
     ///case KEY_Break:             // (Break) or
     ///case KEY_Pause: {           // (Pause)
     ///  if( state & KS_ALT ) {
     ///    op_debug();
     ///   break;
     ///}
     case KEY_SDC:                  // (Shift Delete Character)
     case KEY_DC: {                 // (Delete Character)
       op_key_delete();
       break;
     }
     case KEY_ENTER:                // (Enter) Does not occur. Kept anyway.
     case '\n':                     // (Enter) Does not occur. Kept anyway.
     case '\r': {                   // (Enter)
       if( state & KS_CTL )         // Does not occur. Kept anyway
         op_insert_line();
       else
         op_key_enter();
       break;
     }
     case KEY_ESC: {                // (Escape)
       op_swap_view();
       break;
     }
     case KEY_SIC:                  // (Shift Insert Character)
     case KEY_IC: {                 // (Insert Character)
       op_key_insert();
       break;
     }
     case KEY_TAB: {                // (TAB)
       op_key_tab_forward();
       break;
     }
     case KEY_BTAB: {               // (Backwards TAB)
       op_key_tab_reverse();
       break;
     }

     //-----------------------------------------------------------------------
     // Function keys
     case KEY_F(1): {
       op_help();
       break;
     }
     case KEY_F(2): {
       op_key_idle();
       break;
     }
     case KEY_F(3): {
       op_safe_quit();
       break;
     }
     case KEY_F(4): {
       op_goto_changed();
       return;
     }
     case KEY_F(5): {
       op_repeat_locate();
       break;
     }
     case KEY_F(6): {
       op_repeat_change();
       break;
     }
     case KEY_F(7): {
       op_goto_prev_file();
       break;
     }
     case KEY_F(8): {
       op_goto_next_file();
       break;
     }
     case KEY_F(9): {
       if( state & KS_CTL )
         op_copy_cursor_to_hist();
       else
         op_copy_file_name_to_hist();
       break;
     }
     case KEY_F(10): {
       op_line_to_top();
       break;
     }
     case KEY_F(11): {
       op_undo();
       break;
     }
     case KEY_F(12): {
       op_redo();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case KEY_HOME: {               // (Home)
       op_key_home();
       break;
     }
     case KEY_DOWN: {               // (Down arrow)
       op_key_arrow_down();
       break;
     }
     case KEY_LEFT: {               // (Left arrow)
       op_key_arrow_left();
       break;
     }
     case KEY_RIGHT: {              // (Right arrow)
       op_key_arrow_right();
       break;
     }
     case KEY_UP: {                 // (Up arrow)
       op_key_arrow_up();
       break;
     }
     case KEY_NPAGE: {              // (Next Page)
       op_key_page_down();
       break;
     }
     case KEY_PPAGE: {              // (Previous Page)
       op_key_page_up();
       break;
     }
     case KEY_END: {                // (End)
       op_key_end();
       break;
     }

     //-----------------------------------------------------------------------
     // Mouse buttons
     case KEY_MOUSE: {
       MEVENT mevent;
       int cc= getmouse(&mevent);

       if( IO_TRACE && opt_hcdm )
         traceh("KEY_MOUSE: %d= getmouse mevent(%2d,%2d,%d,0x%.8X)\n", cc
               , mevent.x, mevent.y, mevent.z, mevent.bstate);

       uint32_t button= mevent.bstate;
       unsigned col= (unsigned)mevent.x;
       unsigned row= (unsigned)mevent.y;

       if( button & MB_LEFT ) {     // If LEFT button
         if( row < USER_TOP ) {     // If on top of screen
           if( !file->rem_message() ) { // If no message removed or remains
             if( view == editor::hist ) // If history active
               move_cursor_H(editor::hist->col_zero + col); // Update column
             else
               editor::hist->activate();
           }
           draw_top();
           break;
         }

         // Button press is on data screen
         if( view == editor::hist ) { // If history active
           data->activate();
           draw_top();
         }

         if( row != data->row ) {   // If row changed
           if( row > row_used )     // (Button should not cause scroll up)
             row= row_used;
           data->move_cursor_V(row - data->row); // Set new row
         }
         move_cursor_H(data->col_zero + col); // Set new column
         break;
       }
       if( button & MB_RIGHT ) {    // Right button
         if( row < USER_TOP ) {     // If on top of screen
           if( file->rem_message() ) { // If message removed
             draw_top();
             break;
           }

           // Invert the view
           editor::do_view();
         }
         break;
       }
       if( button & MB_PULL ) {     // Mouse wheel pull (toward)
         move_screen_V(+3);
         break;
       }
       if( button & MB_PUSH ) {     // Mouse wheel push (away)
         move_screen_V(-3);
         break;
       }
       break;
     }

     //-----------------------------------------------------------------------
     // Resize event
     case KEY_RESIZE: {
       if( IO_TRACE && opt_hcdm )
         traceh("KEY_RESIZE: col_size(%d=>%d) row_size(%d=>%d)\n"
               , col_size, COLS, row_size, LINES);

       col_size= COLS;
       row_size= LINES;
       clear();
       draw();
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

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::poll
//       EdInps::read
//
// Purpose-
//       Is character available?
//       read next character (waiting until it's available)
//
//----------------------------------------------------------------------------
bool                                // TRUE if a  character is available
   EdInps::poll(                    // Is a character available?
     int               delay)       // Delay in milliseconds
{  if( IO_TRACE && opt_hcdm )
     traceh("EdInps(%p)::poll(%d) poll_char(%.4X)\n", this, delay, poll_char);

   EdView* view= editor::view;

   if( poll_char <= 0 ) {
//   Trace::trace(".INP", "move", (void*)(uint64_t(view->col)<<32|view->row) );
     wtimeout(win, delay);
     poll_char= mvwgetch(win, view->row, view->col);
     if( poll_char <= 0 ) {
       poll_char= 0;
       return false;
     }
   }

   Trace::trace(".INP", " key"
               , (void*)(uint64_t(view->col)<<32|view->row)
               , (void*)(uint64_t(poll_char)) );
   return true;
}

uint32_t                            // The next character with modifiers
   EdInps::read( void )             // Get the next character
{  if( IO_TRACE && opt_hcdm )
     traceh("EdInps(%p)::read() poll_char(%.4X)\n", this, poll_char);

   key_state &= KS_LOGIC;           // Preserve logical state
   while( poll_char <= 0 )
     poll(125);

   int pc= poll_char;
   poll_char= 0;
   trace_every_keystroke(pc, key_state);

   if( pc == KEY_ESC ) {            // If ESC key
     if( poll(0) ) {                // If alt-key
       key_state |= KS_ALT;         // Indicate alt key
       pc= poll_char;               // (Get the alt key)
       poll_char= 0;
       trace_every_keystroke(pc, key_state);
       if( pc == '[' ) {            // If terminal escape sequence
         while( poll(0) ) {         // Drain the escape sequence
           pc= poll_char;
           poll_char= 0;
           trace_every_keystroke(pc, key_state);
         }
         key_state &= ~(KS_ALT | KS_CTL);
         pc= pub::Utf8::UNI_REPLACEMENT; // Unicode error replacement character
         trace_every_keystroke(pc, key_state);
       }
     }
   }

   if( pc >= 0x01 && pc <= 0x1A ) { // If control key
     if( ctl_table[pc - 1] != '*' ) { // If not also a keyboard key
       key_state |= KS_CTL;         // Indicate control key
       pc= ctl_table[pc - 1];       // Get the associated key code
     }
   }

   if( IO_TRACE && opt_hcdm )
     trace_keystroke(pc, key_state);

   return pc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::putch
//       EdInps::putcr
//
// Purpose-
//       Draw char at column, row
//       Draw text at column, row
//
// Implementation note-
//       Putch was only used to hide or show the cursor. It's now unused.
//
//----------------------------------------------------------------------------
void
   EdInps::putch(                   // Draw text
     GC_t              GC,          // The graphic context
     unsigned          col,         // The (X) column
     unsigned          row,         // The (Y) row
     int               code)        // The character
{  // if( IO_TRACE && opt_hcdm && opt_verbose > 0 )
     traceh("EdOuts(%p)::putch(%u,[%d,%d],0x%.4X) '%s'\n", this, GC, col, row
     , code, visify(code).c_str());

   if( code == 0 )                  // Disallow '\0', convert to ' '
     code= ' ';

   if( USE_UTF8 && code > 0x007f ) {   // Handle UTF-8 encoding
traceh("%4d Outs UTF8(0x%.6X)\n", __LINE__, code); // NOT EXPECTED
     char buffer[8];                // The cursor encoding buffer
     pub::Utf8::encode(code, (utf8_t*)buffer);
     buffer[pub::Utf8::length(code)]= '\0';
     putcr(GC, col, row, buffer);
     return;
   }

   color_set(short(GC), nullptr);   // Set the color
   mvwaddch(win, row, col, code);
   Trace::trace(".PCH", code, visify(code).c_str());
}

void
   EdInps::putcr(                   // Draw text
     GC_t              GC,          // The graphic context
     unsigned          col,         // The (X) column
     unsigned          row,         // The (Y) row
     const char*       text)        // Using this text
{  if( IO_TRACE && opt_hcdm && opt_verbose > 0 ) {
     char buffer[24];
     if( strlen(text) < 17 )
       strcpy(buffer, text);
     else {
       memcpy(buffer, text, 16);
       strcpy(buffer+16, "...");
     }
     traceh("EdOuts(%p)::putcr(%u,[%d,%d],'%s'.%zd)\n", this, GC, col, row
           , visify(buffer).c_str(), strlen(text));
   }

   // Compute output length (in glyphs)
   size_t COL= col_size - col;      // Number of characters left on line
   size_t OUT= COL;                 // Number of characters to write
   size_t LEN= strlen(text);        // Number of characters in string
   OUT= LEN > COL ? COL : LEN;      // Don't use more than lines on string

   // Get the output buffer, adjusting size if UTF-8 characters are present
   Active* active= editor::altact;
   active->reset(text);
   const char* output= active->get_buffer();
   size_t UTF= pub::Utf8::index(output, OUT);
   if( UTF > OUT )
     OUT= UTF;
   output= active->resize(OUT);

   // The curses addstr methods provide '\b' and '\t' special handling.
   // This botches our screen handling, so we prevent that.
   // (TODO: Replace '\b' and '\t' with UNI_REPLACEMENT character)
   char*
   C= (char*)strchr(output, '\b');  // Remove '\b' characters
   while( C ) {
     *C= '~';
     C= (char*)strchr(output, '\b');
   }

   C= (char*)strchr(output, '\t');  // Remove '\t' characters
   while( C ) {
     *C= '~';
     C= (char*)strchr(output, '\t');
   }

   color_set(short(GC), nullptr);   // Set the color
   mvwaddstr(win, row, col, output); // Write the string
   putcr_record* R= (putcr_record*)Trace::storage_if(sizeof(putcr_record));
   if( R ) {
     R->col= htonl(col);
     R->row= htonl(row);
     R->_0018= 0;
     R->length= htonl(uint32_t(OUT));
     Trace::Buffer<putcr_record::DATA_SIZE> buff(output);
     memcpy(R->data, buff.temp, putcr_record::DATA_SIZE);
     memcpy(R->unit, "data", 4);
     ((Trace::Record*)R)->trace(".OUT"); // Trace::trace(".OUT", "data")
   }
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
//----------------------------------------------------------------------------
void
   EdInps::start( void )
{
   init();                          // Initialize, setting graphic contexts

   // Set initial file
   activate(editor::file_list.get_head());
   draw();

   // The main polling loop
   while( operational ) {
//   flush();                       // (Not needed: poll flushes automatically)
     int key= (int)poll(15'000);    // (When data present, immediate response)
     if( key > 0 ) {
       key= read();
       key_input(key, key_state);
     }
   }
}

void
   EdInps::stop( void )             // Stop the editor
{  operational= false; }

void
   EdInps::join( void )             // Wait for the editor completion
{  }
