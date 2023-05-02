//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/Device.cpp
//
// Purpose-
//       Graphical User Interface: Device implementation (Windows).
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "Device.h"
#include "WinDevice.h"

#pragma comment(lib,"Gdi32.lib")
#pragma comment(lib,"User32.lib")

using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define MIN_WINDOW_X            128 // Minimum window width
#define MIN_WINDOW_Y            128 // Minimum window height

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ID_MENU_ABOUT           101 // ABOUT menu item
#define ID_ABOUT_BOX           1001 // ABOUT dialog item

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

#define CALLCODE(x) {x, #x}

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

#ifdef HCDM
  #define DEBUG() debug(__LINE__)
#else
  #define DEBUG()
#endif

//----------------------------------------------------------------------------
// struct CallCode
//----------------------------------------------------------------------------
struct CallCode {
   int                 code;
   const char*         name;
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const RECT      unitRect= {0,0,1,1}; // Unit rectangle
static const RECT      zeroRect= {0,0,0,0}; // Empty rectangle
static const XYOffset  zeroOffset= {0,0};   // Zero offset

static const CallCode  codename[]=
{  CALLCODE(WM_NULL)
,  CALLCODE(WM_CREATE)
,  CALLCODE(WM_DESTROY)
,  CALLCODE(WM_MOVE)
,  CALLCODE(WM_SIZE)
,  CALLCODE(WM_ACTIVATE)
,  CALLCODE(WM_SETFOCUS)
,  CALLCODE(WM_KILLFOCUS)
,  CALLCODE(WM_ENABLE)
,  CALLCODE(WM_SETREDRAW)
,  CALLCODE(WM_SETTEXT)
,  CALLCODE(WM_GETTEXT)
,  CALLCODE(WM_GETTEXTLENGTH)
,  CALLCODE(WM_PAINT)
,  CALLCODE(WM_CLOSE)
,  CALLCODE(WM_QUERYENDSESSION)
,  CALLCODE(WM_QUIT)
,  CALLCODE(WM_QUERYOPEN)
,  CALLCODE(WM_ERASEBKGND)
,  CALLCODE(WM_SYSCOLORCHANGE)
,  CALLCODE(WM_ENDSESSION)
,  CALLCODE(WM_SHOWWINDOW)
,  CALLCODE(WM_SETTINGCHANGE)
,  CALLCODE(WM_DEVMODECHANGE)
,  CALLCODE(WM_ACTIVATEAPP)
,  CALLCODE(WM_FONTCHANGE)
,  CALLCODE(WM_TIMECHANGE)
,  CALLCODE(WM_CANCELMODE)
,  CALLCODE(WM_SETCURSOR)
,  CALLCODE(WM_MOUSEACTIVATE)
,  CALLCODE(WM_CHILDACTIVATE)
,  CALLCODE(WM_QUEUESYNC)
,  CALLCODE(WM_GETMINMAXINFO)
,  CALLCODE(WM_PAINTICON)
,  CALLCODE(WM_ICONERASEBKGND)
,  CALLCODE(WM_NEXTDLGCTL)
,  CALLCODE(WM_SPOOLERSTATUS)
,  CALLCODE(WM_DRAWITEM)
,  CALLCODE(WM_MEASUREITEM)
,  CALLCODE(WM_DELETEITEM)
,  CALLCODE(WM_VKEYTOITEM)
,  CALLCODE(WM_CHARTOITEM)
,  CALLCODE(WM_SETFONT)
,  CALLCODE(WM_GETFONT)
,  CALLCODE(WM_SETHOTKEY)
,  CALLCODE(WM_GETHOTKEY)
,  CALLCODE(WM_QUERYDRAGICON)
,  CALLCODE(WM_COMPAREITEM)
,  CALLCODE(WM_COMPACTING)
,  CALLCODE(WM_WINDOWPOSCHANGING)
,  CALLCODE(WM_WINDOWPOSCHANGED)
,  CALLCODE(WM_POWER)
,  CALLCODE(WM_CANCELJOURNAL)
,  CALLCODE(WM_NOTIFY)
,  CALLCODE(WM_INPUTLANGCHANGEREQUEST)
,  CALLCODE(WM_INPUTLANGCHANGE)
,  CALLCODE(WM_TCARD)
,  CALLCODE(WM_HELP)
,  CALLCODE(WM_USERCHANGED)
,  CALLCODE(WM_NOTIFYFORMAT)
,  CALLCODE(WM_CONTEXTMENU)
,  CALLCODE(WM_STYLECHANGING)
,  CALLCODE(WM_STYLECHANGED)
,  CALLCODE(WM_DISPLAYCHANGE)
,  CALLCODE(WM_GETICON)
,  CALLCODE(WM_SETICON)
,  CALLCODE(WM_NCCREATE)
,  CALLCODE(WM_NCDESTROY)
,  CALLCODE(WM_NCCALCSIZE)
,  CALLCODE(WM_NCHITTEST)
,  CALLCODE(WM_NCPAINT)
,  CALLCODE(WM_NCACTIVATE)
,  CALLCODE(WM_GETDLGCODE)
,  CALLCODE(WM_NCMOUSEMOVE)
,  CALLCODE(WM_NCLBUTTONDOWN)
,  CALLCODE(WM_NCLBUTTONUP)
,  CALLCODE(WM_NCLBUTTONDBLCLK)
,  CALLCODE(WM_NCRBUTTONDOWN)
,  CALLCODE(WM_NCRBUTTONUP)
,  CALLCODE(WM_NCRBUTTONDBLCLK)
,  CALLCODE(WM_NCMBUTTONDOWN)
,  CALLCODE(WM_NCMBUTTONUP)
,  CALLCODE(WM_NCMBUTTONDBLCLK)
,  CALLCODE(WM_KEYFIRST)
,  CALLCODE(WM_KEYDOWN)
,  CALLCODE(WM_KEYUP)
,  CALLCODE(WM_CHAR)
,  CALLCODE(WM_DEADCHAR)
,  CALLCODE(WM_SYSKEYDOWN)
,  CALLCODE(WM_SYSKEYUP)
,  CALLCODE(WM_SYSCHAR)
,  CALLCODE(WM_SYSDEADCHAR)
,  CALLCODE(WM_KEYLAST)
,  CALLCODE(WM_IME_STARTCOMPOSITION)
,  CALLCODE(WM_IME_ENDCOMPOSITION)
,  CALLCODE(WM_IME_COMPOSITION)
,  CALLCODE(WM_IME_KEYLAST)
,  CALLCODE(WM_INITDIALOG)
,  CALLCODE(WM_COMMAND)
,  CALLCODE(WM_SYSCOMMAND)
,  CALLCODE(WM_TIMER)
,  CALLCODE(WM_HSCROLL)
,  CALLCODE(WM_VSCROLL)
,  CALLCODE(WM_INITMENU)
,  CALLCODE(WM_INITMENUPOPUP)
,  CALLCODE(WM_MENUSELECT)
,  CALLCODE(WM_MENUCHAR)
,  CALLCODE(WM_ENTERIDLE)
,  CALLCODE(WM_CTLCOLORMSGBOX)
,  CALLCODE(WM_CTLCOLOREDIT)
,  CALLCODE(WM_CTLCOLORLISTBOX)
,  CALLCODE(WM_CTLCOLORBTN)
,  CALLCODE(WM_CTLCOLORDLG)
,  CALLCODE(WM_CTLCOLORSCROLLBAR)
,  CALLCODE(WM_CTLCOLORSTATIC)
,  CALLCODE(WM_MOUSEFIRST)
,  CALLCODE(WM_MOUSEMOVE)
,  CALLCODE(WM_LBUTTONDOWN)
,  CALLCODE(WM_LBUTTONUP)
,  CALLCODE(WM_LBUTTONDBLCLK)
,  CALLCODE(WM_RBUTTONDOWN)
,  CALLCODE(WM_RBUTTONUP)
,  CALLCODE(WM_RBUTTONDBLCLK)
,  CALLCODE(WM_MBUTTONDOWN)
,  CALLCODE(WM_MBUTTONUP)
,  CALLCODE(WM_MBUTTONDBLCLK)
,  CALLCODE(WM_MOUSEWHEEL)
,  CALLCODE(WM_MOUSELAST)
,  CALLCODE(WM_PARENTNOTIFY)
,  CALLCODE(WM_ENTERMENULOOP)
,  CALLCODE(WM_EXITMENULOOP)
,  CALLCODE(WM_NEXTMENU)
,  CALLCODE(WM_SIZING)
,  CALLCODE(WM_CAPTURECHANGED)
,  CALLCODE(WM_MOVING)
,  CALLCODE(WM_POWERBROADCAST)
,  CALLCODE(WM_DEVICECHANGE)
,  CALLCODE(WM_IME_SETCONTEXT)
,  CALLCODE(WM_IME_NOTIFY)
,  CALLCODE(WM_IME_CONTROL)
,  CALLCODE(WM_IME_COMPOSITIONFULL)
,  CALLCODE(WM_IME_SELECT)
,  CALLCODE(WM_IME_CHAR)
,  CALLCODE(WM_IME_KEYDOWN)
,  CALLCODE(WM_IME_KEYUP)
,  CALLCODE(WM_MDICREATE)
,  CALLCODE(WM_MDIDESTROY)
,  CALLCODE(WM_MDIACTIVATE)
,  CALLCODE(WM_MDIRESTORE)
,  CALLCODE(WM_MDINEXT)
,  CALLCODE(WM_MDIMAXIMIZE)
,  CALLCODE(WM_MDITILE)
,  CALLCODE(WM_MDICASCADE)
,  CALLCODE(WM_MDIICONARRANGE)
,  CALLCODE(WM_MDIGETACTIVE)
,  CALLCODE(WM_MDISETMENU)
,  CALLCODE(WM_ENTERSIZEMOVE)
,  CALLCODE(WM_EXITSIZEMOVE)
,  CALLCODE(WM_DROPFILES)
,  CALLCODE(WM_MDIREFRESHMENU)
,  CALLCODE(WM_CUT)
,  CALLCODE(WM_COPY)
,  CALLCODE(WM_PASTE)
,  CALLCODE(WM_CLEAR)
,  CALLCODE(WM_UNDO)
,  CALLCODE(WM_RENDERFORMAT)
,  CALLCODE(WM_RENDERALLFORMATS)
,  CALLCODE(WM_DESTROYCLIPBOARD)
,  CALLCODE(WM_DRAWCLIPBOARD)
,  CALLCODE(WM_PAINTCLIPBOARD)
,  CALLCODE(WM_VSCROLLCLIPBOARD)
,  CALLCODE(WM_SIZECLIPBOARD)
,  CALLCODE(WM_ASKCBFORMATNAME)
,  CALLCODE(WM_CHANGECBCHAIN)
,  CALLCODE(WM_HSCROLLCLIPBOARD)
,  CALLCODE(WM_QUERYNEWPALETTE)
,  CALLCODE(WM_PALETTEISCHANGING)
,  CALLCODE(WM_PALETTECHANGED)
,  CALLCODE(WM_HOTKEY)
,  CALLCODE(WM_PRINT)
,  CALLCODE(WM_PRINTCLIENT)
,  CALLCODE(WM_HANDHELDFIRST)
,  CALLCODE(WM_HANDHELDLAST)
,  CALLCODE(WM_AFXFIRST)
,  CALLCODE(WM_AFXLAST)
,  CALLCODE(WM_PENWINFIRST)
,  CALLCODE(WM_PENWINLAST)
,  CALLCODE(WM_USER)
,  CALLCODE(WM_USER+1)
,  CALLCODE(WM_USER+2)
,  CALLCODE(WM_USER+3)
,  {0, NULL}
   };


#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//
// Purpose-
//       Handle checkstop condition.
//
//----------------------------------------------------------------------------
void
   checkstop(                       // Check stop
     int               line,        // Line number
     const char*       name,        // Function name
     int               code)        // Error code
{
   Logger::log("%4d: CHECKSTOP(%s,%d)\n", line, name, code);
   Logger::get()->flush();
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       code2name
//
// Purpose-
//       Convert callback code to callback name.
//
//----------------------------------------------------------------------------
const char*                         // Resultant (codename)
   code2name(                       // Get name
     int               code)        // For this code
{
   const char*         result;

   int                 i;

   result= "WM_UNKNOWN";
   for(i= 0; codename[i].name != NULL; i++)
   {
     if( codename[i].code == code )
     {
       result= codename[i].name;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wincheck
//
// Purpose-
//       Check the status of a windows function.
//
//----------------------------------------------------------------------------
void
   wincheck(                        // Check windows function
     int               line,        // Line number
     void*             rc,          // Return code
     const char*       name)        // Function name
{
   int ec= windebug(line, rc, name);

   #if 1                            // WIN98 glitch
     if( ec == 87 )                 // For WIN98, some functions return error
       ec= 0;                       // when they don't really mean it.
   #endif

   if( ec != 0 )
   {
     #ifndef HCDM                   // If not already logged
         Logger::log("%4d: %p= %s() EC(%d)\n", line, rc, name, ec);
     #endif
     fprintf(stderr, "%4d: %p= %s() EC(%d)\n", line, rc, name, ec);

     errno= ec;
     perror("Checkstop: perror");
     checkstop(line, name, ec);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       windebug
//
// Purpose-
//       Get last window function status code, then set it to 0.
//
// Notes-
//       Error codes are defined in WINERROR.H
//
//----------------------------------------------------------------------------
int                                 // The error code on input
   windebug(                        // Check windows function
     int               line,        // Line number
     void*             rc,          // Return code
     const char*       name)        // Function name
{
   int                 ec= GetLastError();

   IFHCDM( Logger::log("%4d: %p= %s() EC(%d)\n", line, rc, name, ec); )

   SetLastError(0);
   return ec;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logRect
//
// Purpose-
//       Log a rectangle.
//
//----------------------------------------------------------------------------
void
   logRect(                         // Log a rectangle
     int               line,        // Caller's line number
     RECT&             rect)        // The rectangle
{
   IFHCDM(
     Logger::log("%4d: RECT(%p) LTRB{%d,%d,%d,%d}\n", line, &rect,
                 rect.left, rect.top, rect.right, rect.bottom);
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       Device::make
//
// Purpose-
//       Return a System Dependent Implementation
//
//----------------------------------------------------------------------------
Device*                             // -> Device
   Device::make(                    // Return System Dependent Implementation
     Window*           window)      // -> Window
{
   IFHCDM( Logger::log("%4d: Device(*)::make(%p)\n", __LINE__, window); )

   return new WinDevice(window);
}
#include "gui/namespace.end"

//----------------------------------------------------------------------------
//
// Now the device extentions are included.
// They are included rather than separately compiled so that they will be
// part of the library include dependency list.
//
//----------------------------------------------------------------------------
#include "WinDevice.cpp"

