//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/Keyboard.cpp
//
// Purpose-
//       Keyboard control.
//
// Last change date-
//       2016/01/01 Mouse wheel support.
//
//----------------------------------------------------------------------------
#include <stdlib.h>                 // For setenv()
#include <curses.h>                 // Uses CURSES

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Controls
//----------------------------------------------------------------------------
#ifndef USE_CBREAK
#define USE_CBREAK FALSE            // Use cbreak()?
#endif

#ifndef USE_KEYPAD
#define USE_KEYPAD TRUE             // Use keypad()?
#endif

#ifndef USE_MOUSE
#define USE_MOUSE  TRUE             // Use mouse events? (Requires USE_KEYPAD)
#endif

#ifndef USE_RAW
#define USE_RAW    TRUE             // Use raw()?
#endif

#ifndef USE_WGETCH
#define USE_WGETCH TRUE             // Use wgetch()?
#endif

#ifndef SET_META
#define SET_META   FALSE            // If defined, use meta(window*,SET_META)
#endif

#if( USE_CBREAK == FALSE && USE_RAW == FALSE )
  #error "Neither USE_CBREAK or USE_RAW"
#endif

#if( USE_CBREAK == TRUE && USE_RAW == TRUE )
  #error "USE_CBREAK and USE_RAW conflict"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Attr
//
// Purpose-
//       Define hidden keyboard attributes.
//
//----------------------------------------------------------------------------
class Attr : public Base {          // Hidden attributes
//----------------------------------------------------------------------------
// Attr: Attributes
//----------------------------------------------------------------------------
public:
   WINDOW*             keyH;        // Standard keyboard handle

//----------------------------------------------------------------------------
// Attr::Constructors
//----------------------------------------------------------------------------
public:
inline virtual
   ~Attr( void );                   // Destructor

inline
   Attr(
     Keyboard&         keyboard);   // Constructor

private:                            // Bitwise copy prohibited
   Attr(const Attr&);               // Disallowed copy constructor
   Attr& operator=(const Attr&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// Attr: Methods
//----------------------------------------------------------------------------
protected:
inline int                          // KeyPress ( may be KeyCode::NUL )
   keyPress( void );                // Read character from keyboard

public:
inline int                          // TRUE if character available
   poll(                            // Is keypress present?
     unsigned          delay= 0);   // Delay in milliseconds

inline int                          // Keyboard character
   rd( void );                      // Read character from keyboard
}; // class Attr

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::~Attr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Attr::~Attr( void )              // Destructor
{
   #ifdef HCDM
     tracef("%8s= KeyboardAttr(%p)::~KeyboardAttr()\n",
            "", this);
   #endif

   resetty();                       // Reset the Keyboard
   endwin();                        // Terminate the Keyboard
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::Attr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Attr::Attr(                      // Constructor
     Keyboard&         keyboard)
:  Base(keyboard), keyH(NULL)
{
   #ifdef HCDM
     tracef("%8s= KeyboardAttr(%p)::KeyboardAttr(%p)\n",
            "", this, &keyboard);
     tracef("%8s= USE_KEYPAD\n", USE_KEYPAD ? "TRUE" : "FALSE");
     tracef("%8s= USE_WGETCH\n", USE_WGETCH ? "TRUE" : "FALSE");
     tracef("%8s= USE_CBREAK\n", USE_CBREAK ? "TRUE" : "FALSE");
     tracef("%8s= USE_RAW\n",    USE_RAW    ? "TRUE" : "FALSE");
     #ifdef SET_META
       tracef("%8s= SET_META\n", SET_META   ? "TRUE" : "FALSE");
     #endif
   #endif

   // Get keyboard handle
   setenv("ESCDELAY", "100", 0);    // Our default ESCDELAY (no override)
   keyH= initscr();                 // Initialize the Keyboard

   // Initialize the keyboard
   #if( USE_CBREAK )
     cbreak();                      // Set cbreak mode
   #endif

   #if( USE_MOUSE )
     mousemask(ALL_MOUSE_EVENTS, NULL); // Enable mouse events
   #endif

   #if( USE_RAW )
     raw();                         // Set raw mode (no translation)
   #endif

   #if( USE_KEYPAD )
     keypad(keyH, TRUE);            // Perform keypad translation
   #else
     keypad(keyH, FALSE);           // DO NOT Perform keypad translation
   #endif

   #ifdef SET_META
     meta(keyH, SET_META);          // Explicitly set META control
   #endif

   intrflush(keyH, FALSE);          // Do not flush on interrupt
   noecho();                        // Set echo OFF (ALWAYS)
   nonl();                          // No '\r' to '\n' conversion, detect '\n'
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::keyPress
//
// Purpose-
//       (Blocking) read character from keyboard, or NULL
//
//----------------------------------------------------------------------------
int                                 // Next KeyPress, or KeyCode::NUL
   Attr::keyPress( void )           // Read character from keyboard
{
   int                 result;      // Resultant

   //-------------------------------------------------------------------------
   // Read next character
   //-------------------------------------------------------------------------
   if( rptCount != 0 )              // If repeated key available
   {
     result= rptChar;               // Use the repeat character
     rptCount= 0;
   }
   else
   {
     //-----------------------------------------------------------------------
     // Read from keyboard
     //-----------------------------------------------------------------------
     #if( USE_WGETCH )
       wtimeout(keyH, (-1));        // Infinite delay
       result= wgetch(keyH);        // Get the next character
     #else
       result= getchar();           // Get the next character
     #endif
   }

   #ifdef HCDM
     //-----------------------------------------------------------------------
     // We trace here, then again later after translation
     tracef("%7xx= KeyboardAttr(%p)::keyPress() RAW\n",
            result, this);
   #endif

   //-------------------------------------------------------------------------
   // Handle the event
   //-------------------------------------------------------------------------
   switch(result)
   {
     case KeyCode::ESC:             // If ESCape
       if( !poll() )                // If no additional keypress present
         break;                     // Return the ESCape
       if( (rptChar >= 'a' && rptChar <= 'z' ) )
       {
         result= altCode[rptChar - 'a'];
         rptCount= 0;
         break;
       }
       if( (rptChar >= 'A' && rptChar <= 'Z' ) )
       {
         result= altCode[rptChar - 'A'];
         rptCount= 0;
         break;
       }
       break;                       // Return the ESCape

     case KEY_MOUSE:                // If mouse event
       result= KeyCode::NUL;
       MEVENT event;
       if( getmouse(&event) == OK )
       {
         if( event.bstate & BUTTON1_PRESSED )
           result= KeyCode::MOUSE_1;
         else if( event.bstate & BUTTON2_PRESSED )
           result= KeyCode::MOUSE_2;
         else if( event.bstate & BUTTON3_PRESSED )
           result= KeyCode::MOUSE_3;
         else if( event.bstate & BUTTON4_PRESSED )
           result= KeyCode::MOUSE_WHEEL_UP;
#if NCURSES_MOUSE_VERSION > 1
         else if( event.bstate & BUTTON5_PRESSED )
           result= KeyCode::MOUSE_WHEEL_DOWN;
#endif

         if( result != KeyCode::NUL )
         {
           mouseCol= event.x;
           mouseRow= event.y;
         }
       }
       break;

     case KEY_RESIZE:               // If resize event
       keyboard.event(Terminal::EventResize);
       result= KeyCode::NUL;
       break;

     case KEY_BTAB:
       result= KeyCode::BACKTAB;
       break;

     case KEY_F(1):
       result= KeyCode::F01;
       break;

     case KEY_F(2):
       result= KeyCode::F02;
       break;

     case KEY_F(3):
       result= KeyCode::F03;
       break;

     case KEY_F(4):
       result= KeyCode::F04;
       break;

     case KEY_F(5):
       result= KeyCode::F05;
       break;

     case KEY_F(6):
       result= KeyCode::F06;
       break;

     case KEY_F(7):
       result= KeyCode::F07;
       break;

     case KEY_F(8):
       result= KeyCode::F08;
       break;

     case KEY_F(9):
       result= KeyCode::F09;
       break;

     case KEY_F(10):
       result= KeyCode::F10;
       break;

     case KEY_HOME:
     case KEY_SHOME:
       result= KeyCode::Home;
       break;

     case KEY_UP:
       result= KeyCode::CursorUp;
       break;

     case KEY_PPAGE:
       result= KeyCode::PageUp;
       break;

     case KEY_LEFT:
     case KEY_SLEFT:
       result= KeyCode::CursorLeft;
       break;

     case KEY_B2:
       result= KeyCode::Center;
       break;

     case KEY_RIGHT:
     case KEY_SRIGHT:
       result= KeyCode::CursorRight;
       break;

     case KEY_END:
     case KEY_SEND:
       result= KeyCode::End;
       break;

     case KEY_DOWN:
       result= KeyCode::CursorDown;
       break;

     case KEY_NPAGE:
       result= KeyCode::PageDown;
       break;

     case KEY_IC:
     case KEY_SIC:
       keyState ^= INSLOCK;
       result= KeyCode::Insert;
       break;

     case KEY_DC:
     case KEY_SDC:
       result= KeyCode::Delete;
       break;

     case KEY_F(11):
       result= KeyCode::F11;
       break;

     case KEY_F(12):
       result= KeyCode::F12;
       break;

     case KEY_BACKSPACE:
     case 0x7F:
       result= '\b';
       break;

     default:
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::poll
//
// Purpose-
//       Determine whether a keypress is available.
//
//----------------------------------------------------------------------------
int                                 // TRUE if keypress available
   Attr::poll(                      // Is keypress present?
     unsigned          delay)       // Delay in milliseconds
{
   int                 result;      // Resultant
   int                 ch;          // Character

   result= FALSE;                   // Default, no character available
   if( rptCount != 0 )              // If a character is already present
     result= TRUE;                  // Indicate character available
   else
   {
     #if( USE_WGETCH )
       wtimeout(keyH, delay);       // Small delay for repeat characters
       ch= wgetch(keyH);            // Get the next character
     #else
       int             arg;         // Argument
       int             fn;          // File number

       fn= fileno(stdin);           // Get STDIN's file number
       arg= 1; ioctl(fn, FIONBIO, &arg); // Enable non-blocking I/O
       ch= getchar();               // Get the next character
       arg= 0; ioctl(fn, FIONBIO, &arg); // Disable non-blocking I/O
     #endif

     if( ch > 0 )                   // If a character available
     {
       result= TRUE;
       rptCount= 1;
       rptChar= ch;
     }
   }

   #ifdef HCDM
     tracef("%8d= KeyboardAttr(%p)::poll(%u)\n", result, this, delay);
   #endif

   return result;                   // Return, data available
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Attr::rd
//
// Purpose-
//       Read character from keyboard.
//
//----------------------------------------------------------------------------
int                                 // Next available character
   Attr::rd( void )                 // Read next character from keyboard
{
   unsigned int        keyCode= KeyCode::NUL;

   //-------------------------------------------------------------------------
   // Read next character, repeating if KeyCode::NUL
   //-------------------------------------------------------------------------
   while( keyCode == KeyCode::NUL )
   {
     keyCode= keyPress();
   }

   #ifdef HCDM
     tracef("%7xx= KeyboardAttr(%p)::rd())\n", keyCode, this);
   #endif
   return keyCode;
}

