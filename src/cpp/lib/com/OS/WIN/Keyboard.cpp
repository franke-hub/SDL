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
//       OS/WIN/Keyboard.cpp
//
// Purpose-
//       Keyboard control.
//
// Last change date-
//       2016/01/01 Mouse wheel support (preparation.)
//
//----------------------------------------------------------------------------
#include <windows.h>                // Uses Windows

#include <com/ScanCode.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

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
   HANDLE              keyH;        // Standard keyboard handle

   int                 RC;          // Return code, only used in debugging
   int                 x_size;      // Last X resize
   int                 y_size;      // Last Y resize

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
     tracef("%8s= KeyboardAttr(%p)::~KeyboardAttr()\n", "", this);
   #endif
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
:  Base(keyboard), keyH(NULL), x_size(-1), y_size(-1)
{
   IFHCDM( tracef("%8s= KeyboardAttr(%p)::KeyboardAttr(%p)\n",
                  "", this, &keyboard); )

   // Get keyboard handle
   keyH= GetStdHandle(STD_INPUT_HANDLE);
   IFHCDM( tracef("%4d %p= GetStdHandle(STD_INPUT..)\n", __LINE__, keyH); )

   // Initialize the keyboard
   RC= SetConsoleMode(
       keyH,
       ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT);
   IFHCDM( tracef("%4d %p= SetConsoleMode(%p, ENABLE_..)\n", __LINE__, keyH); )
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
   DWORD               eventCount;
   INPUT_RECORD        inpRecord;
   unsigned int        inpCode;
   DWORD               inpState;
   unsigned int        inpScan;

   //-------------------------------------------------------------------------
   // Wait for an event
   //-------------------------------------------------------------------------
   WaitForSingleObject(             // Wait for input
       keyH,                        // input buffer handle
       INFINITE);                   // Timeout value

   //-------------------------------------------------------------------------
   // Handle the event
   //-------------------------------------------------------------------------
   result= KeyCode::NUL;
   RC= ReadConsoleInput(
       keyH,                        // input buffer handle
       &inpRecord,                  // buffer to read into
       1,                           // size of read buffer
       &eventCount);                // number of records read
   IFHCDM( tracef("%d= KeyBoard::ReadConsoleInput count(%d) type(%d)\n",
                  RC, eventCount, inpRecord.EventType); )
   if( eventCount > 0 )
   {
     switch(inpRecord.EventType)
     {
       case KEY_EVENT:                // keyboard input
         inpCode=  inpRecord.Event.KeyEvent.uChar.AsciiChar;
         inpScan=  inpRecord.Event.KeyEvent.wVirtualScanCode;
         inpState= inpRecord.Event.KeyEvent.dwControlKeyState;

         #ifdef __DEBUGKEY__
           tracef("%s: KeyEvent: ", __FILE__);
           tracef("%c ", isprint(inpCode)? inpCode : '.');
           tracef("EC(%.1d) KC(%.4x) SC(%.4x) RC(%2d) ",
                  eventCount, inpCode, inpScan,
                  inpRecord[0].Event.KeyEvent.wRepeatCount);
           tracef("STATE(%.8lx) ",
                  (long)inpState);
           if( inpRecord[0].Event.KeyEvent.bKeyDown == TRUE )
             tracef("DOWN ");
           else if( inpRecord[0].Event.KeyEvent.bKeyDown == FALSE )
             tracef("UP   ");
           else
             tracef("%.8lX ", (long)inpRecord[0].Event.KeyEvent.bKeyDown);
           tracef("\n");
         #endif

         // Map the auxiliary key state
         if( (inpState & STS_SHIFT) != 0 )
           keyState |= SHIFT;
         else
           keyState &= ~SHIFT;
         if( (inpState & STS_CONTROL) != 0 )
           keyState |= CTL;
         else
           keyState &= ~CTL;
         if( (inpState & STS_ALT) != 0 )
           keyState |= ALT;
         else
           keyState &= ~ALT;
         if( (inpState & STS_SCRLOCK) != 0 )
           keyState |= SCRLOCK;
         else
           keyState &= ~SCRLOCK;
         if( (inpState & STS_NUMLOCK) != 0 )
           keyState |= NUMLOCK;
         else
           keyState &= ~NUMLOCK;
         if( (inpState & STS_CAPLOCK) != 0 )
           keyState |= CAPLOCK;
         else
           keyState &= ~CAPLOCK;

         if( inpRecord.Event.KeyEvent.bKeyDown == FALSE ) // If KeyUp event
           break;

         if( inpScan == ScanCode::Insert )
           keyState ^= INSLOCK;

         if( (keyState & CTL) != 0 )
         {
           switch(inpScan)
           {
             case ScanCode::CursorLeft:
               result= KeyCode::CTL_CursorLeft;  break;
             case ScanCode::CursorRight:
               result= KeyCode::CTL_CursorRight; break;
             case ScanCode::CursorUp:  result= KeyCode::CTL_CursorUp; break;
             case ScanCode::CursorDown:
               result= KeyCode::CTL_CursorDown;  break;
             case ScanCode::PageUp:    result= KeyCode::CTL_PageUp;   break;
             case ScanCode::PageDown:  result= KeyCode::CTL_PageDown; break;
             case ScanCode::End:       result= KeyCode::CTL_End;      break;
             case ScanCode::Home:      result= KeyCode::CTL_Home;     break;
             case ScanCode::Insert:    result= KeyCode::CTL_Insert;   break;
             case ScanCode::Delete:    result= KeyCode::CTL_Delete;   break;
             case ScanCode::F11:       result= KeyCode::CTL_F11;      break;
             case ScanCode::F12:       result= KeyCode::CTL_F12;      break;

             default:
               if( inpScan >= ScanCode::F01 && inpScan <= ScanCode::F10 )
                 result= KeyCode::CTL_F01 + (inpScan-ScanCode::F01);
               break;
           }

           if( result != KeyCode::NUL )
             break;
         }

         if( (keyState & ALT) != 0 )
         {
           switch(inpScan)
           {
             case ScanCode::Backspace: result= KeyCode::ALT_BS;      break;
             case ScanCode::F11:       result= KeyCode::ALT_F11;     break;
             case ScanCode::F12:       result= KeyCode::ALT_F12;     break;

             default:
               if( inpCode >= 'A' && inpCode <= 'Z' )
                 result= altCode[inpCode - 'A'];
               else if( inpCode >= 'a' && inpCode <= 'z' )
                 result= altCode[inpCode - 'a'];
               else if( inpScan >= ScanCode::F01 && inpScan <= ScanCode::F10 )
                 result= KeyCode::ALT_F01 + (inpScan-ScanCode::F01);
               break;
           }

           if( result != KeyCode::NUL )
             break;
         }

         if( (keyState & SHIFT) != 0 )
         {
           if( inpCode == KeyCode::TAB )
           {
             result= KeyCode::BACKTAB;
             break;
           }
         }

         // Neither CTL, AUX nor SHIFT
         switch(inpScan)
         {
           case ScanCode::Home:        result= KeyCode::Home;        break;
           case ScanCode::CursorUp:    result= KeyCode::CursorUp;    break;
           case ScanCode::PageUp:      result= KeyCode::PageUp;      break;
           case ScanCode::CursorLeft:  result= KeyCode::CursorLeft;  break;
           case ScanCode::CursorRight: result= KeyCode::CursorRight; break;
           case ScanCode::End:         result= KeyCode::End;         break;
           case ScanCode::CursorDown:  result= KeyCode::CursorDown;  break;
           case ScanCode::PageDown:    result= KeyCode::PageDown;    break;
           case ScanCode::Insert:      result= KeyCode::Insert;      break;
           case ScanCode::Delete:      result= KeyCode::Delete;      break;
           case ScanCode::F11:         result= KeyCode::F11;         break;
           case ScanCode::F12:         result= KeyCode::F12;         break;

           default:
             result= inpCode;
             if( inpScan >= ScanCode::F01 && inpScan <= ScanCode::F10 )
               result= KeyCode::F01 + (inpScan-ScanCode::F01);
             break;
         }
         break;

       case WINDOW_BUFFER_SIZE_EVENT:
         {{{{
         // This code prevents WIN10 recursive resize event loop
         int x= inpRecord.Event.WindowBufferSizeEvent.dwSize.X;
         int y= inpRecord.Event.WindowBufferSizeEvent.dwSize.Y;
         IFHCDM(
           tracef("%4d Keyboard::Attr *RESIZE* (%d,%d)\n", __LINE__, x, y);
         )

         if( x_size > 0 && (x != x_size || y != y_size) )
           keyboard.event(Terminal::EventResize);

         x_size= x;
         y_size= y;
         }}}}
         break;

       case MOUSE_EVENT:
         switch( (long)inpRecord.Event.MouseEvent.dwEventFlags )
         {
           case MOUSE_HWHEELED:
             if( (long)inpRecord.Event.MouseEvent.dwButtonState > 0 )
               result= KeyCode::MOUSE_WHEEL_RIGHT;
             else
               result= KeyCode::MOUSE_WHEEL_LEFT;
             break;

           case MOUSE_WHEELED:
             if( (long)inpRecord.Event.MouseEvent.dwButtonState > 0 )
               result= KeyCode::MOUSE_WHEEL_UP;
             else
               result= KeyCode::MOUSE_WHEEL_DOWN;
             break;
         }
         break;

       case FOCUS_EVENT:
       case MENU_EVENT:
       default:
         break;
     }
   }

   #ifdef HCDM
     tracef("%8d= KeyboardAttr(%p)::keyPress()\n",
            result, this);
   #endif
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
   DWORD               eventCount;  // Number of pending events

   result= FALSE;                   // Default, no character available
   if( rptCount != 0 )              // If a character is already present
     result= TRUE;                  // Indicate character available
   else
   {
     for(;;)
     {
       eventCount= 0;               // Default in case of error
       GetNumberOfConsoleInputEvents( // Check for pending event
         keyH,                      // Keyboard handle
         &eventCount);              // Number of events

       if( eventCount == 0 )
         break;

       rptChar= keyPress();         // Read the next character
       if( rptChar != KeyCode::NUL )
       {
         rptCount= 1;
         result= TRUE;
         break;
       }
     }
   }

   #ifdef HCDM
     tracef("%8d= KeyboardAttr(%p)::poll(%u)\n", result, this, delay);
   #endif
   return result;
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
   // Remove repeat character
   //-------------------------------------------------------------------------
   if( rptCount > 0 )
   {
     rptCount--;
     keyCode= rptChar;
   }

   //-------------------------------------------------------------------------
   // Read next character, repeating if KeyCode::NUL
   //-------------------------------------------------------------------------
   while( keyCode == KeyCode::NUL )
   {
     keyCode= keyPress();
   }

   #ifdef HCDM
     tracef("%8d= KeyboardAttr(%p)::rd())\n", keyCode, this);
   #endif
   return keyCode;
}

