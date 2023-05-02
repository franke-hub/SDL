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
//       Keyboard.cpp
//
// Purpose-
//       Keyboard control.
//
// Last change date-
//       2016/01/01 Mouse wheel support.
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // Used in debug traces
#include <unistd.h>

#include <com/Debug.h>
#include <com/KeyCode.h>
#include <com/syslib.h>

#include "com/Keyboard.h"
#include "com/Terminal.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define Attr KeyboardAttr           // Qualify local class
#define Base KeyboardAttrBase       // Qualify local class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define STS_ENHANCED         0x0100 // Enhanced keystroke
#define STS_CAPLOCK          0x0080 // CAPS LOCK   key is toggled
#define STS_SCRLOCK          0x0040 // SCROLL LOCK key is toggled
#define STS_NUMLOCK          0x0020 // NUM LOCK    key is toggled
#define STS_SHIFT            0x0010 // SHIFT key is depressed
#define STS_CONTROL          0x000C // Any CONTROL key is depressed
#define STS_LCONTROL         0x0008 // Left CONTROL key is depressed
#define STS_RCONTROL         0x0004 // Right CONTROL key is depressed
#define STS_ALT              0x0003 // Any ALT key is depressed
#define STS_LALT             0x0002 // Left ALT key is depressed
#define STS_RALT             0x0001 // Right ALT key is depressed

//----------------------------------------------------------------------------
// Enumerations and Typedefs
//----------------------------------------------------------------------------
enum State                          // Keyboard state
{  SYSREQ=                   0x8000 // SysReq key
,  CAPKEY=                   0x4000 // Capslock key
,  NUMKEY=                   0x2000 // Numlock key
,  SCRKEY=                   0x1000 // Scroll key
,  RALT=                     0x0800 // Right Alt
,  RCTL=                     0x0400 // Right Ctl
,  LALT=                     0x0200 // Left Alt
,  LCTL=                     0x0100 // Left Ctl
,  INSLOCK=                  0x0080 // Ins lock
,  CAPLOCK=                  0x0040 // Cap lock
,  NUMLOCK=                  0x0020 // Num lock
,  SCRLOCK=                  0x0010 // Scroll lock
,  ALT=                      0x0008 // Either Alt
,  CTL=                      0x0004 // Either Ctl
,  SHIFT=                    0x0003 // Either shift
,  LSHIFT=                   0x0002 // Left  shift
,  RSHIFT=                   0x0001 // Right shift
}; // enum State

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const short   altCode[26]= {
   KeyCode::ALT_A,
   KeyCode::ALT_B,
   KeyCode::ALT_C,
   KeyCode::ALT_D,
   KeyCode::ALT_E,
   KeyCode::ALT_F,
   KeyCode::ALT_G,
   KeyCode::ALT_H,
   KeyCode::ALT_I,
   KeyCode::ALT_J,
   KeyCode::ALT_K,
   KeyCode::ALT_L,
   KeyCode::ALT_M,
   KeyCode::ALT_N,
   KeyCode::ALT_O,
   KeyCode::ALT_P,
   KeyCode::ALT_Q,
   KeyCode::ALT_R,
   KeyCode::ALT_S,
   KeyCode::ALT_T,
   KeyCode::ALT_U,
   KeyCode::ALT_V,
   KeyCode::ALT_W,
   KeyCode::ALT_X,
   KeyCode::ALT_Y,
   KeyCode::ALT_Z
};

//----------------------------------------------------------------------------
//
// Class-
//       Base
//
// Purpose-
//       Define hidden keyboard attributes base class.
//
//----------------------------------------------------------------------------
class Base {                        // Hidden attributes base class
//----------------------------------------------------------------------------
// Base: Attributes
//----------------------------------------------------------------------------
public:
   Keyboard&           keyboard;    // Associated Keyboard
   unsigned int        rptCount;    // Repeat counter (0 or 1)
   int                 rptChar;     // Repeat character
   unsigned int        keyState;    // Keyboard state
   unsigned int        mouseRow;    // Current mouse row
   unsigned int        mouseCol;    // Current mouse column

//----------------------------------------------------------------------------
// Base::Constructors
//----------------------------------------------------------------------------
public:
inline virtual
   ~Base( void ) { }                // Destructor

inline
   Base(
     Keyboard&         keyboard)    // Constructor
:  keyboard(keyboard), rptCount(0), rptChar('\0'), keyState(0)
,  mouseRow(0), mouseCol(0)
{}

private:                            // Bitwise copy prohibited
   Base(const Base&);               // Disallowed copy constructor
   Base& operator=(const Base&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// Base: Methods
//----------------------------------------------------------------------------
public:
inline State                        // The (combination of) keyboard states
   getState( void )                 // Get the keyboard state
{  return State(keyState); }
}; // class Base

//----------------------------------------------------------------------------
//
// Class-
//       Attr
//
// Purpose-
//       OS dependent keyboard operation.
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN)
#include "OS/WIN/Keyboard.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/Keyboard.cpp"

#else
#error "Invalid OS"
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::~Keyboard
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Keyboard::~Keyboard( void )      // Destructor
{
   #ifdef HCDM
     tracef("%8s= Keyboard(%p)::~Keyboard()\n", "", this);
   #endif

   if( attr != NULL )               // If attributes exist
   {
     delete (Attr*)attr;            // Delete them
     attr= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::Keyboard
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Keyboard::Keyboard( void )       // Constructor
:  Handler()
{
   #ifdef HCDM
     tracef("%8s= Keyboard(%p)::Keyboard()\n", "", this);
   #endif

   attr= (void*)new Attr(*this);    // Create associated attributes
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::ifInsertKey
//
// Purpose-
//       Get insert mode.
//
//----------------------------------------------------------------------------
int                                 // TRUE if insert mode
   Keyboard::ifInsertKey( void )    // Is insert key locked?
{
   int                 result;      // Resultant
   Attr&               attr= *(Attr*)this->attr;

   result= FALSE;
   if( attr.getState() & INSLOCK)
     result= TRUE;

   #ifdef HCDM
     tracef("%8d= Keyboard(%p)::ifInsertKey()\n", result, this);
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::ifScrollKey
//
// Purpose-
//       Get scroll mode.
//
//----------------------------------------------------------------------------
int                                 // TRUE if scroll mode
   Keyboard::ifScrollKey( void )    // Is scroll key locked?
{
   int                 result;      // Resultant
   Attr&               attr= *(Attr*)this->attr;

   result= FALSE;
   if( attr.getState() & SCRLOCK )
     result= TRUE;

   #ifdef HCDM
     tracef("%8d= Keyboard(%p)::ifScrollKey()\n", result, this);
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::poll
//
// Purpose-
//       Determine whether a keypress is available.
//
//----------------------------------------------------------------------------
int                                 // TRUE if keypress available
   Keyboard::poll(                  // Is keypress present?
     unsigned          delay)       // Delay in milliseconds
{
   int                 result;      // Resultant
   Attr&               attr= *(Attr*)this->attr;

   result= attr.poll(delay);

   #ifdef HCDM
     tracef("%8d= Keyboard(%p)::poll(%u)\n", result, this, delay);
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Keyboard::rd
//
// Purpose-
//       Read character from keyboard.
//
//----------------------------------------------------------------------------
int                                 // Next available character
   Keyboard::rd( void )             // Read next character from keyboard
{
   int                 result;      // Resultant
   Attr&               attr= *(Attr*)this->attr;

   result= attr.rd();

   #ifdef HCDM
     tracef("%7xx= Keyboard(%p)::rd()\n", result, this);
   #endif
   return result;
}

