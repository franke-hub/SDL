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
//       2024/07/27
//
//----------------------------------------------------------------------------
#define _XOPEN_SOURCE_EXTENDED 1

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

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps, implemented
#include "EdMark.h"                 // For EdMark
#include "EdOpts.h"                 // For EdOpts
#include "EdType.h"                 // For GC_t

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace
using std::string;                  // Using std::string

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Controls
,  IO_TRACE= true                   // I/O trace mode?

// The color saturation value (Determined experimentally)
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
static int             have_extended_colors= true; // DEFAULT, extended colors

//----------------------------------------------------------------------------
// Static internal data areas
//----------------------------------------------------------------------------
// Control keys G..M (encoded as 0x07..0x0d) are not passed to application
                                      // 123456789abcdef0123456789a
static constexpr const char* alt_table= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static constexpr const char* ctl_table= "ABCDEF*******NOPQRSTUVWXYZ";

//----------------------------------------------------------------------------
// EdInps.hpp: Extended key management
//----------------------------------------------------------------------------
#include <EdInps.hpp>

//----------------------------------------------------------------------------
//
// Exception-
//       curses_error
//
// Purpose-
//       Indicate curses error
//
//----------------------------------------------------------------------------
struct curses_error : public std::runtime_error {
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
// Subroutine-
//       init_program_modes
//
// Purpose-
//       Initialize ncurses program modes.
//
// Implementation notes-
//       Implementations should use cbreak() or raw(), but not both.
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
// curs_set(1);                     // (Use visible cursor)
   curs_set(0);                     // (Don't use cursor)
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

   return false;                    // (Key_0x7F treated as BACKSPACE)
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
       case '\b':                   // (Backspace char)
       case KEY_BACKSPACE:          // (Backspace key)
       case 0x007F:                 // (DELete char encoding)
       case KEY_DC:                 // (Delete key)
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

   if( key >= 0x0020 && key <= 0x007F ) { // If ASCII text key
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
//       nc_64k
//
// Purpose-
//       Convert a 64M color into a 64K color
//
// Implementation notes-
//       ASSUMING: rrrrrr ggggg bbbbb, NOT TESTED
//
//----------------------------------------------------------------------------
static inline int                   // The 64K color
   nc_64k(                          // Convert 64M color into 64K color
     uint32_t          color)       // The 64M color
{
   Color rgb(color);
   int r= (rgb.red()    * 63) / 252;
   int g= (rgb.green()  * 31) / 248;
   int b= (rgb.blue()   * 31) / 248;

   int c= (r << 10 | g << 4 | b);
   if( c < 16 && c > 0 )
     c |= 0x00000420;
   return c;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_256
//
// Purpose-
//       Convert a 64M color into a 256 color
//
// Implementation notes-
//       ASSUMING: rrr ggg bb, NOT TESTED
//
//----------------------------------------------------------------------------
static inline int                   // The 256 color
   nc_256(                          // Convert 64M color into 256 color
     uint32_t          color)       // The 64M color
{
   Color rgb(color);
   int r= (rgb.red()    * 7) / 223;
   int g= (rgb.green()  * 7) / 223;
   int b= (rgb.blue()   * 3) / 192;

   int c= (r << 5 | g << 2 | b);
   if( c < 8 && c > 0 )
     c |= 0x00000020;
   return c;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_8
//
// Purpose-
//       Convert a 64M color into an 8 color
//
// Implementation notes-
//       The built-in colors are mapped BLUE, GREEN, RED; i.e. BGR *not* RGB
//
//       (Color)           BGR
//       COLOR_BLACK   0 B:000 ---
//       COLOR_RED     1 B:001 --R
//       COLOR_GREEN   2 B:010 -G-
//       COLOR_YELLOW  3 B:011 -GR (Minus B)
//       COLOR_BLUE    4 B:100 B--
//       COLOR_MAGENTA 5 B:101 B-R (Minus G)
//       COLOR_CYAN    6 B:110 BG- (Minus R)
//       COLOR_WHITE   7 B:111 BGR
//
//       While this function works, translating the 64M default color
//       definitions doesn't work well enough to be useful.
//       (This subroutine isn't used.)
//
//----------------------------------------------------------------------------
static inline int                   // The 8 color
   nc_8(                            // Convert 64M color into an 8 color
     uint32_t          color)       // The 64M color
{
   Color rgb(color);
   int r= (rgb.red()    / 248);
   int g= (rgb.green()  / 248);
   int b= (rgb.blue()   / 248);

   return (b << 2 | g << 1 | r);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_set_color
//
// Purpose-
//       Initialize a color, returning a replacement color index
//
//----------------------------------------------------------------------------
static inline int                   // The replacement color index
   nc_set_color(                    // Set a color
     uint32_t          color)       // The 64M color
{
static int ix= 16;                  // The current color index

   Color rgb(color);
   int r= rgb.red()   * MAX_COLOR / 255;
   int g= rgb.green() * MAX_COLOR / 255;
   int b= rgb.blue()  * MAX_COLOR / 255;
   int cc= init_extended_color(ix, r, g, b);
   if( cc == ERR ) {
     traceh("%d= init_extended_color(%d,%d,%d,%d)\n", cc, ix, r, g, b);
     throw curses_error("init_extended_color");
   }

   if( IO_TRACE && opt_hcdm )
     traceh("%d= init_extended_color(%d,%d,%d,%d)\n", cc, ix, r, g, b);

   return ix++;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nc_set_pair
//
// Purpose-
//       Initialize a basic color pair
//
//----------------------------------------------------------------------------
static inline void
   nc_set_pair(                     // Initialize a color pair
     GC_t              GC,          // The graphic context
     int               fg,          // The foreground Color or index
     int               bg)          // The background Color or index
{
   if( have_extended_colors ) {     // If using extended colors
     int cc= init_extended_pair(GC, fg, bg);
     if( cc == ERR ) {
       traceh("%d= init_extended_pair(%d,0x%.8x,0x%.8x)\n", cc, GC, fg, bg);
       throw curses_error("init_extended_pair");
     }

     if( IO_TRACE && opt_hcdm )
       traceh("%d= init_extended_pair(%d,0x%.8x,0x%.8x)\n", cc, GC, fg, bg);
   } else {
     int cc= init_pair(short(GC), short(fg), short(bg));
     if( cc == ERR ) {
       traceh("%d= init_pair(%d,0x%.8x,0x%.8x)\n", cc, GC, fg, bg);
       throw curses_error("init_pair");
     }

     if( IO_TRACE && opt_hcdm )
       traceh("%d= init_pair(%d,0x%.8x,0x%.8x)\n", cc, GC, fg, bg);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
//----------------------------------------------------------------------------
static inline void
   term( void )                     // Terminate
{  EdOpts::at_exit(); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       tf
//
// Purpose-
//       Convert boolean to "true" or "false"
//
//----------------------------------------------------------------------------
static inline const char*           // "true" or "false"
   tf(bool cc)                      // Convert boolean cc to "true" or "false"
{  return cc ? "true" : "false"; }

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
//       EdInps::EdInps
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------

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

   delete editor::data;             // Delete the views and the mark
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
//       This constructor is invoked *before* Config::parser invocation.
//
//----------------------------------------------------------------------------
void
   EdInps::init( void )             // Initializer
{  if( opt_hcdm )
     traceh("EdInps(%p)::init\n", this);

   // opt_hcdm= true;               // (Bringup?)

   if( HCDM || opt_hcdm ) {
     debug_set_mode(pub::Debug::MODE_INTENSIVE);
     traceh("%s %s %s Hard Core Debug Mode\n", __FILE__, __DATE__, __TIME__);
   }

   // Get default colors
   int fg=      config::text_fg;
   int bg=      config::text_bg;

   int fg_mark= config::mark_fg;
   int bg_mark= config::mark_bg;
   int fg_chg=  config::change_fg;
       bg_chg=  config::change_bg;
   int fg_msg=  config::message_fg;
   int bg_msg=  config::message_bg;
   int fg_sts=  config::status_fg;
       bg_sts=  config::status_bg;

   // Cygwin consoles, when $TERM is changed from `xterm` to `xterm-256color`
   // actually support 256 COLORS and can_change_color().
   const char* CYGWIN= getenv("CYGWIN");
   const char* DISPLAY= getenv("DISPLAY");
   if( CYGWIN && DISPLAY == nullptr ) {
     setenv("TERM", "xterm-256color", true);
   }

   try {                            // Activate NCURSES
     win= initscr();                // Open the WINDOW
     start_color();

     // Initialize colors
     if( !has_colors() )
       throw curses_error("No color support!");

     int has_COLORS= COLORS;
     int has_PAIRS=  COLOR_PAIRS;

     if( has_COLORS < 8 || has_PAIRS < 8 )
       throw curses_error("Not enough color support!");

     // Would you believe it?
     // Some terminals lie about their can_change_color capability.
     bool has_change_color= can_change_color();
     if( has_change_color && COLORS >= 32 ) {
       int cc= init_extended_color(18, 752, 941, 1000); // Try to change color
       if( cc == ERR ) {            // LIE detected
         if( opt_hcdm )
           traceh("%4d EdInps FAILED init_extended_color test\n", __LINE__);
         has_change_color= false;   // Fall back to minimal color set
         has_COLORS= 8;
       }
     }

     if( opt_hcdm ) {
       const char* TERM= getenv("TERM");
       traceh("$TERM(%s) COLORS(%d) COLOR_PAIRS(%d)\n"
             , TERM, COLORS, COLOR_PAIRS);
       traceh("can_change_color(%s) has_change_color(%s)\n"
             , tf(can_change_color()), tf(has_change_color));
     }

     if( has_change_color && has_COLORS >= 32 ) { // If can_change_color()
       if( opt_hcdm )
         traceh("%4d EdInps: SET colors\n", __LINE__);

       fg=      nc_set_color(fg     );
       bg=      nc_set_color(bg     );
       fg_mark= nc_set_color(fg_mark);
       bg_mark= nc_set_color(bg_mark);
       fg_chg=  nc_set_color(fg_chg );
       bg_chg=  nc_set_color(bg_chg );
       fg_msg=  nc_set_color(fg_msg );
       bg_msg=  nc_set_color(bg_msg );
       fg_sts=  nc_set_color(fg_sts );
       bg_sts=  nc_set_color(bg_sts );
     } else if( COLORS == 0x01000000 ) { // If 16M colors supported
       if( opt_hcdm )
         traceh("%4d EdInps: 16M colors\n", __LINE__);

     } else if( has_COLORS == 0x00010000 ) { // If 64K colors supported
       if( opt_hcdm )
         traceh("%4d EdInps: 64K colors\n", __LINE__);

       fg=      nc_64k(fg     );
       bg=      nc_64k(bg     );
       fg_mark= nc_64k(fg_mark);
       bg_mark= nc_64k(bg_mark);
       fg_chg=  nc_64k(fg_chg );
       bg_chg=  nc_64k(bg_chg );
       fg_msg=  nc_64k(fg_msg );
       bg_msg=  nc_64k(bg_msg );
       fg_sts=  nc_64k(fg_sts );
       bg_sts=  nc_64k(bg_sts );
     } else if( has_COLORS == 0x00000100 ) { // If 256 colors supported
       if( opt_hcdm )
         traceh("%4d EdInps: 256 colors\n", __LINE__);

       fg=      nc_256(fg     );
       bg=      nc_256(bg     );
       fg_mark= nc_256(fg_mark);
       bg_mark= nc_256(bg_mark);
       fg_chg=  nc_256(fg_chg );
       bg_chg=  nc_256(bg_chg );
       fg_msg=  nc_256(fg_msg );
       bg_msg=  nc_256(bg_msg );
       fg_sts=  nc_256(fg_sts );
       bg_sts=  nc_256(bg_sts );
     } else {                       // MINIMAL (8) color support
       if( opt_hcdm )
         traceh("%4d EdInps: 8 colors\n", __LINE__);

       have_extended_colors= false; //
       fg=      COLOR_WHITE;        // (Hand modified)
       bg=      COLOR_BLUE;
       fg_mark= COLOR_BLACK;
       bg_mark= COLOR_CYAN;
       fg_chg=  COLOR_WHITE;
       bg_chg=  COLOR_RED;
       fg_msg=  COLOR_BLACK;
       bg_msg=  COLOR_YELLOW;
       fg_sts=  COLOR_BLACK;
       bg_sts=  COLOR_GREEN;
     }

     // Set the color pairs (Graphic Contexts)
     nc_set_pair(gc_font, fg,      bg);
     nc_set_pair(gc_flip, bg,      fg);
     nc_set_pair(gc_mark, fg_mark, bg_mark);
     nc_set_pair(gc_chg,  fg_chg,  bg_chg);
     nc_set_pair(gc_msg,  fg_msg,  bg_msg);
     nc_set_pair(gc_sts,  fg_sts,  bg_sts);
   } catch( std::exception& X ) {
     traceh("%4d EdInps Exception.what(%s)\n", __LINE__, X.what());
     term();
     Config::failure("Initialization failed: %s", X.what());
   }

   // Colors initialized. Set screen size, etc.
   init_program_modes(win);         // Initialize settings
   def_prog_mode();                 // (Save modes as "program" modes)

   getmaxyx(win, row_size, col_size); // Set screen size
   wsetscrreg(win, 0, row_size-1);  // Set scrolling region
   assume_default_colors(fg, bg);   // Set default colors (for clear screen)
   bkgdset(' ');                    // Set the background character
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
   tracef("..mouse_cursor(%d,%d,%d)\n"
         , mouse_cursor.state, mouse_cursor.x, mouse_cursor.y);
   tracef("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n", gc_font, gc_flip, gc_mark);
   tracef("..bg_chg(%u)  bg_sts(%u)\n", bg_chg, bg_sts);
   tracef("..gc_chg(%u)  gc_msg(%u)  gc_sts(%u)\n", gc_chg, gc_msg, gc_sts);
   tracef("..operational(%d) poll_char(0x%.4X)\n", operational, poll_char);
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
{  translate_irregular_keys(key, state); // Translate (used) irregular keys

   if( opt_hcdm && opt_verbose > 0 )
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
       if( move_cursor_H(column + 1) )
         view->draw_active();
     } else {
       view->active.replace_char(column, key);
       move_cursor_H(column + 1);
     }
     draw_top();
     show_cursor();

     // Escape complete; "No File Changed" message complete
     key_state &= ~(KS_ESC | KS_NFC);
     return;
   }

   // Handle action key
   switch( key ) {                  // Handle data key
     case 0x007F:                   // (DEL char, treated as backspace)
     case KEY_BACKSPACE:            // (Backspace)
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
       if( state & KS_CTL )
         op_copy_cursor_to_hist();
       else
         op_copy_file_name_to_hist();
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
       op_line_to_bot();
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
