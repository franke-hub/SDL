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
//       EdInps.hpp
//
// Purpose-
//       EdInps.cpp subroutines
//
// Last change date-
//       2024/08/30
//
//----------------------------------------------------------------------------
#ifndef EDINPS_HPP_INCLUDED
#define EDINPS_HPP_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       key_defs
//
// Purpose-
//       Key definitions
//
// Implementation notes-
//       *PRAGMATIC* Windows test results (US Keyboard only)
//       Acronyms: ALT_: Alt key; CTL_: Ctrl key; SFT_: Shift key
//
//----------------------------------------------------------------------------
struct key_defs {
enum
{  DEAD=               0            // Dead keys have no application effect
                                    // but may cause system interaction
,  PRINT_SCREEN=       DEAD         // (Windows action?)
,  SCROLL_LOCK=        DEAD         // (Windows action?)
,  PAUSE_BREAK=        DEAD         // (Windows action?)

,  F1=                 00411        // 0x0109 KEY_F( 1)
,  F2=                 00412        // 0x010A KEY_F( 2)
,  F3=                 00413        // 0x010B KEY_F( 3)
,  F4=                 00414        // 0x010C KEY_F( 4)
,  F5=                 00415        // 0x010D KEY_F( 5)
,  F6=                 00416        // 0x010E KEY_F( 6)
,  F7=                 00417        // 0x010F KEY_F( 7)
,  F8=                 00420        // 0x0110 KEY_F( 8)
,  F9=                 00421        // 0x0111 KEY_F( 9)
,  F10=                00422        // 0x0112 KEY_F(10)
,  F11=                00423        // 0x0113 KEY_F(11)
,  F12=                00424        // 0x0114 KEY_F(12)

,  SFT_F1=             00425        // 0x0115 + 0:0014 0x000C (12)
,  SFT_F2=             00426        // 0x0116
,  SFT_F3=             00427        // 0x0117
,  SFT_F4=             00430        // 0x0118
,  SFT_F5=             00431        // 0x0119
,  SFT_F6=             00432        // 0x011A
,  SFT_F7=             00433        // 0x011B
,  SFT_F8=             00434        // 0x011C
,  SFT_F9=             00435        // 0x011D
,  SFT_F10=            00436        // 0x011E
,  SFT_F11=            00437        // 0x011F
,  SFT_F12=            00440        // 0x0120

,  CTL_F1=             00441        // 0x0121 + 0:0030 0x0018 (24)
,  CTL_F2=             00442        // 0x0122
,  CTL_F3=             00443        // 0x0123
,  CTL_F4=             00444        // 0x0124
,  CTL_F5=             00445        // 0x0125
,  CTL_F6=             00446        // 0x0126
,  CTL_F7=             00447        // 0x0127
,  CTL_F8=             00450        // 0x0128
,  CTL_F9=             00451        // 0x0129
,  CTL_F10=            00452        // 0x012A
,  CTL_F11=            00453        // 0x012B
,  CTL_F12=            00454        // 0x012C

,  CTL_SFT_F1=         00455        // 0x012D + 0:0044 0x0024 (36)
,  CTL_SFT_F2=         00456        // 0x012E
,  CTL_SFT_F3=         00457        // 0x012F
,  CTL_SFT_F4=         00460        // 0x0130
,  CTL_SFT_F5=         00461        // 0x0131
,  CTL_SFT_F6=         00462        // 0x0132
,  CTL_SFT_F7=         00463        // 0x0133
,  CTL_SFT_F8=         00464        // 0x0134
,  CTL_SFT_F9=         00465        // 0x0135
,  CTL_SFT_F10=        00465        // 0x0136
,  CTL_SFT_F11=        00467        // 0x0137
,  CTL_SFT_F12=        00470        // 0x0138

,  ALT_F1=             00471        // 0x0139 + 0:0060 0x0030 (48)
,  ALT_F2=             00472        // 0x013A
,  ALT_F3=             00473        // 0x013B
,  ALT_F4=             00474        // 0x013C (Window manager: Close Window)
,  ALT_F5=             00475        // 0x013D
,  ALT_F6=             00476        // 0x013E
,  ALT_F7=             00477        // 0x013F
,  ALT_F8=             00500        // 0x0140
,  ALT_F9=             00501        // 0x0141
,  ALT_F10=            00502        // 0x0142
,  ALT_F11=            00503        // 0x0143
,  ALT_F12=            00504        // 0x0144
,  ALT_CTL_Fnn=        DEAD         // ALT-CTL-KEY_F(*) are all dead keys

// ALT-SFT-F4 is treated as ALT-F4, a "Close Window" demand sequence.
// ALT-SFT-F5..F12, if the sequence were to be continued, already have other
//   KEY_ definitions. (CURSES returns an escape sequence instead.)
,  ALT_SFT_F1=         00505        // 0x0145 + 0:0074 0x003C (60)
,  ALT_SFT_F2=         00506        // 0x0146
,  ALT_SFT_F3=         00507        // 0x0147
// ALT_SFT_F4=         00510        // 0x0148 // KEY_DL (Delete line)
// ALT_SFT_F5=         00511        // 0x0149 // KEY_IL
// ALT_SFT_F6=         00512        // 0x014A // KEY_DC
// ALT_SFT_F7=         00513        // 0x014B // KEY_IC
// ALT_SFT_F8=         00514        // 0x014C // KEY_EIC
// ALT_SFT_F9=         00515        // 0x014D // KEY_CLEAR
// ALT_SFT_F10=        00516        // 0x014E // KEY_EOS
// ALT_SFT_F11=        00517        // 0x014F // KEY_EOL
// ALT_SFT_F12=        00520        // 0x0150 // KEY_SF

,  DELete=             00512        // 0x014A KEY_DC
,  ALT_delete=         01016        // 0x020E + 0:0304 0x00C4
,  CTL_delete=         01020        // 0x0210 + 0:0306 0x00C6
,  ALT_CTL_delete=     00000        // 0x0000 (Windows task manager sequence)

,  insert=             00513        // 0x014B KEY_IC
,  ALT_insert=         01043        // 0x0223 + 0:0330 0x00D8
,  CTL_insert=         01045        // 0x0225 + 0:0332 0x00DA
,  ALT_CTL_insert=     01047        // 0x0227 + 0:0334 0x00DC

,  home=               00406        // 0x0106 KEY_HOME
,  ALT_home=           01036        // 0x021E + 0:0118 0x0430
,  CTL_home=           01040        // 0x0220 + 0:011A 0x0432
,  ALT_CTL_home=       01042        // 0x0222 + 0:011C 0x0434

,  end=                00550        // 0x0168 KEY_END
,  ALT_end=            01031        // 0x0219 + 0:0261 0x00B1
,  CTL_end=            01033        // 0x021B + 0:0263 0x00B3
,  ALT_CTL_end=        01035        // 0x021D + 0:0265 0x00B5

,  page_down=          00522        // 0x0152 KEY_NPAGE
,  ALT_page_down=      01055        // 0x022D + 0:0333 0x00DB
,  CTL_page_down=      01057        // 0x022F + 0:0335 0x00DD
,  ALT_CTL_page_down=  01061        // 0x0231 + 0:0337 0x00DF

,  page_up=            00523        // 0x0153 KEY_PPAGE
,  ALT_page_up=        01062        // 0x0232 + 0:0337 0x00DF
,  CTL_page_up=        01064        // 0x0234 + 0:0341 0x00E1
,  ALT_CTL_page_up=    01066        // 0x0236 + 0:0343 0x00E3

// Arrow keys- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
,  arrow_down=         00402        // 0x0102 KEY_DOWN
,  ALT_arrow_down=     01024        // 0x0214 + 0:0422 0x0112
,  CTL_arrow_down=     01026        // 0x0216 + 0:0424 0x0114
,  ALT_CTL_arrow_down= 01030        // 0x0218 + 0:0426 0x0116

,  arrow_left=         00404        // 0x0104 KEY_LEFT
,  ALT_arrow_left=     01050        // 0x0228 + 0:0444 0x0124
,  CTL_arrow_left=     01052        // 0x022A + 0:0446 0x0126
,  ALT_CTL_arrow_left= 01054        // 0x022C + 0:0450 0x0128

,  arrow_right=        00405        // 0x0105 KEY_RIGHT
,  ALT_arrow_right=    01067        // 0x0237 + 0:0462 0x0132
,  CTL_arrow_right=    01071        // 0x0239 + 0:0464 0x0134
,  ALT_CTL_arrow_right=01073        // 0x023B + 0:0466 0x0136

,  arrow_up=           00403        // 0x0103 KEY_UP
,  ALT_arrow_up=       01075        // 0x023D + 0:0472 0x013A
,  CTL_arrow_up=       01077        // 0x023F + 0:0474 0x013C
,  ALT_CTL_arrow_up=   01101        // 0x0241 + 0:0476 0x013E
}; // (Generic enum)
}; // key_defs

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
// Subroutine-
//       translate_irregular_keys
//
// Purpose-
//       *MINIMAL* Key control modifications
//
// Implementation notes-
//       ONLY modified key values that are used are updated.
//       (Currently Ctrl-F2)
//
//----------------------------------------------------------------------------
static inline void
   translate_irregular_keys(        // Translate irregular key values
     uint32_t&         key,         // IN/OUT The input key
     uint32_t&         state)       // IN/OUT The Alt/Ctl/Shift state mask
{
   switch( key ) {                  // Handle irregular key values
     case key_defs::CTL_F2:         // CTL-F2
       state |= KS_CTL;
       key= key_defs::F2;
       break;
     default:                       // No translation provided
       break;
   }
}
#endif // EDINPS_HPP_INCLUDED
