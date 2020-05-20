//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/WinDevice.cpp
//
// Purpose-
//       Graphical User Interface: Device implementation (Windows).
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
// Included only from OS/WIN/Device.cpp
#include <com/Barrier.h>

#include "WinDevice.h"              // Implementation class

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Protects WinDevice singleton
static WinDevice*      common= NULL;// The WinDevice singleton

//----------------------------------------------------------------------------
//       Trying to get resize callback to work.
//       ...
//       The callback works and the buffer is properly built.
//       The window size shrinks after the last resize so the right
//       and bottom parts of the buffer are not visible.

//----------------------------------------------------------------------------
//
// Subroutine-
//       aboutDialog
//
// Purpose-
//       This callback routine handles the ABOUT diaglog.
//       NeedsWork: No resource section defined
//
//----------------------------------------------------------------------------
static LRESULT CALLBACK             // Resultant
   aboutDialog(                     // ABOUT dialog processor
     HWND              hwnd,        // Window handle
     UINT              event,       // Event
     WPARAM            wParam,      // First message
     LPARAM            lParam)      // Second message
{
   LRESULT             result= 0;   // Resultant
   const char*         status= "Handled";

   int                 rc;

   switch (event)
   {
     case WM_INITDIALOG:
       {
         SetDlgItemText(hwnd, ID_ABOUT_BOX, "about");
         result= TRUE;
         break;
       }

     case WM_CLOSE:
       EndDialog(hwnd, TRUE);
       result= TRUE;
       break;

     case WM_COMMAND:
       if( LOWORD(wParam) == IDOK )
       {
         EndDialog(hwnd, TRUE);
       }
       break;

     default:
       status= "IGNORED";
   }

   IFHCDM(
     Logger::log("%4d: %d= aboutDialog(%p,%.4x) %s %s\n",
                 __LINE__, result,
                 hwnd, event, status, code2name(event));
   )

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       winCallback
//
// Purpose-
//       This is the WINDOWS callback routine, called on each event
//       It is logically an extension of the WinDevice object.
//
// Notes-
//       WM_* defines are in WINUSER.H
//
//----------------------------------------------------------------------------
static LRESULT CALLBACK             // Resultant
   winCallback(                     // Windows callback routine
     HWND              hwnd,        // Window handle
     UINT              event,       // Event
     WPARAM            wParam,      // First message
     LPARAM            lParam)      // Second message
{
   LRESULT             result= 0;   // Resultant
   WinDevice*          device= (WinDevice*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

   #ifdef HCDM
     const char*       status= "Started";
   #endif

   IFHCDM(
     Logger::log("%4d: %2d= WinDevice(%p)::callback(%p,%.4x) %s %s\n",
                 __LINE__, result,
                 device, hwnd, event, status, code2name(event));
   )

   if( device == NULL )
   {
     switch(event)
     {
       case WM_CREATE:
       case WM_DESTROY:
         IFHCDM( status= "Ignored"; )
         break;

       default:
         IFHCDM( status= "Default"; )
         result= DefWindowProc(hwnd, event, wParam, lParam);
         break;
     }
   }
   else
   {
     IFHCDM( status= "*DEVICE"; )
     result= device->callback(hwnd, event, wParam, lParam);
   }

   #if defined(HCDM) && (VERBOSE > 2)
     Logger::log("%4d: %2d= WinDevice(%p)::callback(%p,%.4x) %s %s\n",
                 __LINE__, result,
                 device, hwnd, event, status, code2name(event));
   #endif

   return result;
}

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       WinDevice_StaticDestructor (SINGLETON)
//
// Purpose-
//       Delete the WinDevice singleton
//
//----------------------------------------------------------------------------
static class WinDevice_StaticDestructor  {
public:
   WinDevice_StaticDestructor::~WinDevice_StaticDestructor( void )
{
   AutoBarrier lock(barrier);

   if( common != NULL )
   {
     delete common;
     common= NULL;
   }
}
} WinDevice_StaticDestructor;

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::~WinDevice
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   WinDevice::~WinDevice( void )    // Destructor
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinDevice(%p)::~WinDevice()\n", __LINE__, this); )

   #if( VERBOSE > 1 )
     DEBUG();
   #endif

   if( operating )
   {
     rc= PostMessage(hwnd, WM_CLOSE, 0, 0);
     WINDEBUG(rc, "PostMessage");

     wait();
   }

   DEBUG();
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::WinDevice
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   WinDevice::WinDevice(            // Constructor
     Window*           window)      // Source Window
:  Device(window), Thread()         // Base classes
,  started(NULL)
,  operational(FALSE)
,  operating(FALSE)
,  eventsMutex()
,  statusMutex()
,  bitmapMutex()
,  hdc(NULL)
,  hinst(NULL)
,  hmap(NULL)
,  hpal(NULL)
,  hwnd(NULL)
{
   int                 i;
   int                 rc;

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::WinDevice(%p)\n",
                 __LINE__, this, window);
     Logger::log("%4d: Version %s, %s %s\n", __LINE__,
                 VERSION_ID, __DATE__, __TIME__);
   )

   AutoMutex statusLock(statusMutex);

   for(i= 0; i<sizeof(prefix); i++)
     prefix[i]= 'p';

   for(i= 0; i<sizeof(suffix); i++)
     suffix[i]= 's';

   // Initialize all offsets and lengths
   client.bitmap.x= client.bitmap.y= 0;
   if( window == NULL )
   {
     client.offset= zeroOffset;
     client.length.x= MIN_WINDOW_X;
     client.length.y= MIN_WINDOW_X;
   }
   else
   {
     client.offset= window->getOffset();
     client.length= window->getLength();
   }

   client.window= client.length;

   device.dpi.x= device.dpi.y= 0;
   device.offset.x= device.offset.y= 0;
   device.length.x= device.length.y= 0;
   device.window.x= device.window.y= 0;

   scroll.active= FALSE;
   scroll.offset.x= scroll.offset.y= 0;
   scroll.length= client.length;
   scroll.window= client.window;

   // Start our thread, which owns the window
   started= new Status();           // Something to wait for
   start();                         // Start (this) Thread
   started->wait();                 // Wait for initialization to complete
   delete started;                  // (Not needed any more)
   started= NULL;                   // (For neatness)

   // Initialization complete
   resize(client.length);
   DEBUG();
// throw "Debugging Checkstop";
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::adjust
//
// Purpose-
//       Adjust the Device offset and length
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   WinDevice::adjust(               // Adjust the offset and length
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   const char*         result;      // Exception message

   RECT                rect;        // Working RECT
   TempDC              tdc(hwnd);   // Temporary HDC
   unsigned            x;           // Working X length
   unsigned            y;           // Working Y length

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::adjust({%d,%d},{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y, length.x, length.y);
   )

   // State check
   AutoMutex statusLock(statusMutex);
   DEBUG();

   if( !operational )
   {
     if( window != NULL )
       window->setAttribute(Window::VISIBLE, FALSE);
     Logger::log("%4d: WinDevice(%p) NonOperationalException\n",
                 __LINE__, this);
     throw "DeviceNonOperationalException";
   }

   // Save the offset and length
   client.offset= offset;           // TODO: + device.offset
   client.offset.x= min(client.offset.x, device.window.x-1);
   client.offset.y= min(client.offset.y, device.window.y-1);

   // Once moved, the client cannot be moved again programatically
   if( device.offset.x != 0 || device.offset.y != 0 )
     client.offset= device.offset;

   client.length= length;
   client.window= length;

   scroll.offset.x= scroll.offset.y= 0;
   scroll.length= client.length;
   scroll.window= client.window;

   // If required, update the frame size
   if( length.x > client.bitmap.x
       || length.y > client.bitmap.y )
     resizeBitmap(length);

   int rc= TRUE;
   if( rc )
   {
     x= max(MIN_WINDOW_X, length.x);
     y= max(MIN_WINDOW_Y, length.y);
     SetRect(&rect, client.offset.x, client.offset.y,
             client.offset.x + x, client.offset.y + y);
     logRect(__LINE__, rect);

     result= "AdjustWindowRectEx";
     rc= AdjustWindowRectEx(
         &rect,
         GetWindowLong(hwnd, GWL_STYLE) & ~WS_HSCROLL & ~WS_VSCROLL,
         FALSE,
         GetWindowLong(hwnd, GWL_EXSTYLE));
     WINDEBUG(rc, result);
     logRect(__LINE__, rect);

     client.window.x= rect.right - rect.left;
     client.window.y= rect.bottom - rect.top;
     scroll.window= client.window;
   }

   if( rc )
   {
     result= "SetWindowPos";
     rc= SetWindowPos(
         hwnd,
         HWND_TOP,
         client.offset.x,
         client.offset.y,
         client.window.x,
         client.window.y,
         SWP_SHOWWINDOW);
     WINDEBUG(rc, result);
   }

   SetRect(&rect, 0, 0, client.window.x, client.window.y);
   updateScrolling(rect);

   if( rc )
     result= NULL;

   DEBUG();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::callback
//
// Purpose-
//       Called from the WINDOWS callback routine for each event
//
// Notes-
//       WM_* defines are in WINUSER.H
//
//----------------------------------------------------------------------------
LRESULT                             // Resultant
   WinDevice::callback(             // Windows callback routine
     HWND              hwnd,        // Window handle
     UINT              event,       // Event
     WPARAM            wParam,      // First message
     LPARAM            lParam)      // Second message
{
   LRESULT             result= 0;   // Resultant
   const char*         action= "Handled"; // Possibly "Default"
   RECT*               ptrRECT;     // Working -> RECT
   RECT                rect;        // Working RECT
   int                 x;           // Working offset/length
   int                 y;           // Working offset/length

   int                 rc;

   switch(event)
   {
     case WM_CREATE:
     case WM_DESTROY:
       break;

     case WM_SYSCOMMAND:
       {
         switch(LOWORD(wParam))
         {
           case ID_MENU_ABOUT:
             rc= DialogBox(
                     hinst,
                     MAKEINTRESOURCE(ID_MENU_ABOUT),
                     NULL,
                     (DLGPROC)aboutDialog);
             WINDEBUG(rc, "DialogBox");
             break;

           default:
             result= DefWindowProc(hwnd, event, wParam, lParam);
             action= "Default";
             break;
         }

         break;
       }

     case WM_CLOSE:
     case WM_QUIT:
       {
         rc= PostMessage(hwnd, WM_USER+1, 0, 0);
         WINDEBUG(rc, "PostMessage");
      // terminate();
       }
       break;

     case WM_PAINT:
       render();
       break;

     case WM_SIZE:
       SetRect(&rect, 0, 0, LOWORD(lParam), HIWORD(lParam));
       logRect(__LINE__, rect);

       switch(wParam)
       {
         case SIZE_MAXHIDE:
         case SIZE_MINIMIZED:
         case SIZE_MAXIMIZED:
         case SIZE_RESTORED:
           break;

         default:
           WINDEBUG(wParam, "WM_SIZE.wParam");
           break;
       }

#if 0 //////////////////////////////// Infinite loop
       DEBUG();
       if( (rect.right-rect.left) != client.length.x
           || (rect.bottom-rect.top) != client.length.y )
         handleEvent(Event::EC_RESIZE, 0, rect);
#endif
       break;

     case WM_SIZING:
       ptrRECT= (RECT*)lParam;
       logRect(__LINE__, *ptrRECT);

       updateScrolling(*ptrRECT);

//////////////////////////////////////////////////////////////////////////////
// The problem here is that the next window paint misses the change
//     handleEvent(Event::EC_RESIZE, 0, *ptrRECT);

       result= DefWindowProc(hwnd, event, wParam, lParam);
       action= "Default";
       break;

     case WM_HSCROLL:
       {
         int dx= 0;
         int pos= HIWORD(wParam);
         switch (LOWORD(wParam))
         {
           case SB_LINEUP:
             dx= -2;
             break;

           case SB_LINEDOWN:
             dx= 2;
             break;

           case SB_PAGEUP:
             dx= scroll.length.x * -1/4;
             break;

           case SB_PAGEDOWN:
             dx= scroll.length.x * 1/4;
             break;

           case SB_THUMBPOSITION:
           case SB_THUMBTRACK:
             dx= pos - scroll.offset.x;
             break;
         }

         updateScrollOffset(dx, 0);
         break;
       }

     case WM_VSCROLL:
       {
         int dy= 0;
         int pos= HIWORD(wParam);
         switch (LOWORD(wParam))
         {
           case SB_LINEUP:
             dy= -2;
             break;

           case SB_LINEDOWN:
             dy= 2;
             break;

           case SB_PAGEUP:
             dy= scroll.length.y * -1/4;
             break;

           case SB_PAGEDOWN:
             dy= scroll.length.y * 1/4;
             break;

           case SB_THUMBPOSITION:
           case SB_THUMBTRACK:
             dy= pos - scroll.offset.y;
             break;
         }

         updateScrollOffset(0, dy);
         break;
       }

     case WM_MOUSEWHEEL:
       {
         int dy= (short)HIWORD(wParam);

         updateScrollOffset(0, -dy);
         break;
       }

     case WM_USER+1:
//     PostQuitMessage(0);
       break;

//   case WM_REGIONUPDATED:
//   case WM_TIMER:
//   case WM_LBUTTONDOWN:
//   case WM_LBUTTONUP:
//   case WM_MBUTTONDOWN:
//   case WM_MBUTTONUP:
//   case WM_RBUTTONDOWN:
//   case WM_RBUTTONUP:
//   case WM_MOUSEMOVE:
//   case WM_KEYDOWN:
//   case WM_KEYUP:
//   case WM_SYSKEYDOWN:
//   case WM_SYSKEYUP:
//   case WM_CHAR:
//   case WM_SYSCHAR:
//   case WM_DEADCHAR:
//   case WM_SYSDEADCHAR:
//   case WM_SETFOCUS:
//   case WM_KILLFOCUS:
//   case WM_QUERYNEWPALETTE:
//   case WM_PALETTECHANGED:
//   case WM_SETCURSOR:
//   case WM_DRAWCLIPBOARD:
//   case WM_CHANGECBCHAIN:
//   case WM_WINDOWPOSCHANGED:
     default:
       result= DefWindowProc(hwnd, event, wParam, lParam);
       action= "Default";
       break;
   }

   IFHCDM(
     Logger::log("%4d: %2d= WinDevice(%p)::callback(%p,%.4x) %s %s\n",
                 __LINE__, result,
                 this, hwnd, event, action, code2name(event));
   )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::change
//
// Purpose-
//       Reflect a change in the Window
//
//----------------------------------------------------------------------------
void
   WinDevice::change(               // Reflect buffer change
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   Pixel*              pixel;       // Working -> Pixel
   XOffset_t           oX;          // Working XOffset
   YOffset_t           oY;          // Working YOffset
   RECT                rect;        // Working RECTangle

   int                 rc;

   if( window == NULL )
     checkstop(__LINE__, "WinDevice::change", -1);

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::change({%d,%d},{%d,%d}) {%d,%d}\n",
                 __LINE__, this, offset.x, offset.y, length.x, length.y,
                 window->getLength().x, window->getLength().y);
   )

   // State check
   AutoMutex eventsLock(eventsMutex);
   AutoMutex bitmapLock(bitmapMutex);

   DEBUG();
   if( !operational )
   {
     window->setAttribute(Window::VISIBLE, FALSE);
     Logger::log("%4d: WinDevice(%p) NonOperationalException\n",
                 __LINE__, this);
     throw "DeviceNonOperationalException";
   }

   // Make the change
   ObjectSelector  objsel(hdc, hmap);
   PaletteSelector palsel(hdc, hpal);
   WINCHECK(0, "changeSelector");

   SetRect(&rect, offset.x, offset.y, offset.x+length.x, offset.y+length.y);
   rect.right= min(rect.right, client.length.x);
   rect.bottom= min(rect.bottom, client.length.y);
   logRect(__LINE__, rect);

   for(oY= rect.top; oY<rect.bottom; oY++)
   {
     pixel= window->getPixel(rect.left, oY);
     for(oX= rect.left; oX<rect.right; oX++)
     {
       // Why we built a palette, who knows?
       Color_t color= (pixel->getBlue()  << 16)
                    | (pixel->getGreen() << 8)
                    | (pixel->getRed()   << 0);
       SetPixel(hdc, oX, oY, color);
       pixel++;
     }
   }
   WINCHECK(hdc, "SetPixel");

   // Expose the change
   rect.left   -= scroll.offset.x;
   rect.right  -= scroll.offset.x;
   rect.top    -= scroll.offset.y;
   rect.bottom -= scroll.offset.y;
   logRect(__LINE__, rect);

   rc= InvalidateRect(hwnd, &rect, FALSE);
   WINCHECK(rc, "InvalidateRect");

   // Experimental test
   #if 0
     rect.top= 0;
     rect.left= 0;
     rect.bottom= 10;
     rect.right= 40;
     rc= DrawText(hdc, "Hi There!", -1, &rect, DT_LEFT + DT_TOP);
     WINCHECK(rc, "DrawText");
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::debug
//
// Purpose-
//       Diagnostic log
//
//----------------------------------------------------------------------------
#undef DEBUGADDR
#define DEBUGADDR(f) \
   Logger::log("..%16s: %p\n", #f, f);

#undef DEBUGDECI
#define DEBUGDECI(f) \
   Logger::log("..%16s: %d\n", #f, f);

#undef DEBUGHEXI
#define DEBUGHEXI(f) \
   Logger::log("..%16s: 0x%.8x\n", #f, f);

#undef DEBUGSIZE
#define DEBUGSIZE(f) \
   Logger::log("..%16s: {%d,%d}\n", #f, f.x, f.y);

void
   WinDevice::debug(                // Diagnostic log
     int               line,        // Caller's line number
     const char*       message)     // Associated message
{
   char                string[128]; // Working string
   int                 i;

   if( message == NULL )
     message= "";
   Logger::log("%4d: WinDevice(%p)::debug(%s)\n", line, this, message);

   DEBUGADDR(started);
   DEBUGDECI(operational);
   DEBUGDECI(operating);

   DEBUGSIZE(device.dpi);
   DEBUGSIZE(device.offset);
   DEBUGSIZE(device.length);
   DEBUGSIZE(device.window);

   DEBUGSIZE(client.bitmap);
   DEBUGSIZE(client.offset);
   DEBUGSIZE(client.length);
   DEBUGSIZE(client.window);

   DEBUGDECI(scroll.active);
   DEBUGSIZE(scroll.offset);
   DEBUGSIZE(scroll.length);
   DEBUGSIZE(scroll.window);

   DEBUGADDR(hdc);
   DEBUGADDR(hinst);
   DEBUGADDR(hmap);
   DEBUGADDR(hpal);
   DEBUGADDR(hwnd);

   for(i= 0; i<sizeof(prefix); i++)
   {
     if( prefix[i] != 'p' )
       if( operational )
       {
         operational= FALSE;
         checkstop(__LINE__, "PrefixFault", i);
       }
   }

   for(i= 0; i<sizeof(suffix); i++)
   {
     if( suffix[i] != 's' )
       if( operational )
       {
         operational= FALSE;
         checkstop(__LINE__, "SuffixFault", i);
       }
   }

   Logger::log("................................\n");
}

void
   WinDevice::debug( void )          // Debugging display
{
   Logger::log("%4d: WinDevice(%p)::debug()\n", __LINE__, this);
   debug(__LINE__, "debug");
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::get
//
// Purpose-
//       Retrieve the WinDevice singleton
//
//----------------------------------------------------------------------------
WinDevice*                          // The WinDevice singleton
   WinDevice::get( void )           // Get WinDevice singleton
{
   if( common == NULL )
   {
     AutoBarrier lock(barrier);

     if( common == NULL )
       common= new WinDevice(NULL);
   }

   return common;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::handleEvent
//
// Purpose-
//       Handle an Event.
//
//----------------------------------------------------------------------------
void
   WinDevice::handleEvent(          // Handle an Event
     Event::EC         code,        // Event code
     unsigned          data,        // Event data
     const RECT&       rect)        // Location
{
#if 0
   EventVisitor        visitor;     // The EventVisitor
   Event               event;       // The Event
   XYLength            length;      // The Length
   XYOffset            offset;      // The Offset

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::handleEvent(%d,%d,LTRB{%d,%d,%d,%d})\n",
                 __LINE__, this, code, data,
                 rect.left, rect.top, rect.right, rect.bottom);
   )

   // Format the Event
   length.x= rect.right - rect.left;
   length.y= rect.bottom- rect.top;
   offset.x= rect.left;
   offset.y= rect.top;

   event.setEvent(code, data);      // Set the code and data
   event.setLength(length);         // Set the length
   event.setOffset(offset);         // Set the offset

   // Determine which Object should handle the Event
   visitor.object= window;          // Set the default object
// window->visit(visitor, zeroOffset, window->getLength());

   // Handle the Event
// AutoMutex eventsLock(eventsMutex);
// visitor.object->handleEvent(event);
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::initiaize
//
// Purpose-
//       When using WinThread, must be called from there for Window ownership.
//
//----------------------------------------------------------------------------
void
   WinDevice::initialize( void )    // Initialize the Device instance
{
   XYLength            length;      // Bitmap length
   RECT                rect;        // Working rectangle
   WNDCLASS            wndclass;    // Windows CLASS object

   int                 rc;

   IFHCDM( Logger::log("%4d: WinDevice(%p)::initialize()\n", __LINE__, this); )

   //-------------------------------------------------------------------------
   // Initialize Windows objects
   SetLastError(0);                 // For WINCHECK/WINDEBUG
// hinst= GetModuleHandle(NULL);    // The Windows application instance
   hinst= GetModuleHandle("WinDevice.cpp");
   WINDEBUG(hinst, "GetModuleHandle");

   // Initialize the WNDCLASS object
   wndclass.style= 0;               // No special style
   wndclass.lpfnWndProc= ::winCallback; // Callback routine
   wndclass.cbClsExtra= 0;          // No extra data at end of WNDCLASS
   wndclass.cbWndExtra= 0;          // No extra data at end of WINDOW
   wndclass.hInstance= hinst;
   wndclass.hIcon= ::LoadIcon(hinst, MAKEINTRESOURCE(IDI_APPLICATION));
   wndclass.hCursor= ::LoadCursor(NULL, IDC_ARROW);
   wndclass.hbrBackground= (HBRUSH)GetStockObject(BLACK_BRUSH);
   wndclass.lpszMenuName= (const TCHAR *) NULL;
   wndclass.lpszClassName= "WinDevice";

   // Register the class
   rc= RegisterClass(&wndclass);
   WINDEBUG(rc, "RegisterClass");

   // Create the associated physical window
   const DWORD style= WS_OVERLAPPED  | WS_CAPTION     | WS_SYSMENU
                    | WS_MINIMIZEBOX | WS_THICKFRAME;

   hwnd= CreateWindow("WinDevice",     // Registered class name
                      "WinDevice",     // Title
                      style,           // Style
                      CW_USEDEFAULT,   // X offset
                      CW_USEDEFAULT,   // Y offset
                   // CW_USEDEFAULT,   // X length
                   // CW_USEDEFAULT,   // Y length
                      client.window.x, // X length
                      client.window.y, // Y length
                      NULL,            // Parent handle
                      NULL,            // Menu handle
                      hinst,           // Application instance
                      NULL);           // Window creation data
   WINCHECK(hwnd, "CreateWindow");

   rc= ShowWindow(hwnd, SW_HIDE);   // Begin hidden
   WINCHECK(rc, "ShowWindow");

   // Associate device with Window
   SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

   // Create a memory DC for the local framebuffer
   hdc= CreateCompatibleDC(NULL);
   WINCHECK(hdc, "CreateCompatibleDC");

   // Determine device characteristics
   HDC rootDC= GetDC(NULL);
   WINCHECK(rootDC, "GetDC");
   device.dpi.x= GetDeviceCaps(rootDC, LOGPIXELSX);
   device.dpi.y= GetDeviceCaps(rootDC, LOGPIXELSY);

   device.window.x= GetDeviceCaps(rootDC, HORZRES);
   device.window.y= GetDeviceCaps(rootDC, VERTRES);
   WINCHECK(rootDC, "GetDeviceCaps");

   rc= SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
   WINDEBUG(rc, "SystemParametersInfo");
   logRect(__LINE__, rect);
   device.length.x= rect.right - rect.left;
   device.length.y= rect.bottom - rect.top;

   // Create a Bitmap
   length.x= max(device.window.x, client.length.x);
   length.y= max(device.window.y, client.length.y);
   resizeBitmap(length);

   // Set up a suitable palette
   if( (GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) != 0 )
   {
     Logger::log("%4d: SIZEPALETTE(%d) NUMRESERVED(%d)\n", __LINE__,
                 GetDeviceCaps(hdc, SIZEPALETTE),
                 GetDeviceCaps(hdc, NUMRESERVED));
     BYTE        buffer[sizeof(LOGPALETTE)+(6*6*6)*sizeof(PALETTEENTRY)];
     LOGPALETTE* ptrPAL= (LOGPALETTE*)buffer;
     int index= 0;
     for (int r= 5; r >= 0; r--)
     {
       for (int g= 5; g >= 0; g--)
       {
         for (int b= 5; b >= 0; b--)
         {
           ptrPAL->palPalEntry[index].peRed=   r * 255 / 5;
           ptrPAL->palPalEntry[index].peGreen= g * 255 / 5;
           ptrPAL->palPalEntry[index].peBlue=  b * 255 / 5;
           ptrPAL->palPalEntry[index].peFlags= NULL;
           index++;
         }
       }
     }
     ptrPAL->palVersion= 0x300;     // Per documentation
     ptrPAL->palNumEntries= (6*6*6);
     hpal= CreatePalette(ptrPAL);
     WINCHECK(hpal, "CreatePalette");
   }

   // Add stuff to System menu
   HMENU hmenu= GetSystemMenu(hwnd, FALSE);
   WINCHECK(hmenu, "GetSystemMenu");
   AppendMenu(hmenu, MF_SEPARATOR, NULL, NULL);
   AppendMenu(hmenu, MF_STRING, ID_MENU_ABOUT, "&About WinDevice...");
   rc= DrawMenuBar(hwnd);
   WINCHECK(rc, "DrawMenuBar");

   // We are now operational
   operational= TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::move
//
// Purpose-
//       Move the Device
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   WinDevice::move(                 // Move the Device
     const XYOffset&   offset)      // Offset
{
   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::move({%d,%d})\n",
                 __LINE__, this, offset.x, offset.y);
   )

   return adjust(offset, client.length);
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::poll
//
// Purpose-
//       Poll and drive single event
//
//----------------------------------------------------------------------------
int                                 // The event code
   WinDevice::poll( void )          // Poll and drive single event
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinDevice(%p)::poll()\n", __LINE__, this); )

   MSG                 msg;         // The input message

   rc= GetMessage(&msg, hwnd, 0, 0);
   WINDEBUG(rc, "GetMessage");
   if( rc == (-1) )
     checkstop(__LINE__, "GetMessage", rc);

   if( rc == 0 || rc == (-1) ||  msg.message == (WM_USER+1) )
     return WM_CLOSE;

   rc= TranslateMessage(&msg);
   WINDEBUG(rc, "TranslateMessage");
   rc= DispatchMessage(&msg);
   WINDEBUG(rc, "DispatchMessage");

   return msg.message;              // Return message code
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::render
//
// Purpose-
//       Render (paint) the window
//
//----------------------------------------------------------------------------
void
   WinDevice::render( void )        // Render (paint) the Window
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinDevice(%p)::render()\n", __LINE__, this); )
   #if( VERBOSE > 5 )
     DEBUG();
   #endif

   // Obtain the bitmap lock
   AutoMutex bitmapLock(bitmapMutex);
   if( hmap == NULL || !operational )
     return;

   // Prepare to paint
   PAINTSTRUCT ps;
   HDC paintDC= BeginPaint(hwnd, &ps);
   WINCHECK(paintDC, "BeginPaint");

   PaletteSelector palsel(paintDC, hpal);
   ObjectSelector  objsel(hdc, hmap);
   WINCHECK(0, "renderSelector");

   // Paint
   IFHCDM(
      Logger::log("%4d: BitBlt(%p,{%d,%d,%d,%d},%p,{%d,%d},%x)\n", __LINE__,
           paintDC,
           ps.rcPaint.left,
           ps.rcPaint.top ,
           ps.rcPaint.right-ps.rcPaint.left,
           ps.rcPaint.bottom-ps.rcPaint.top,
           hdc,
           ps.rcPaint.left + scroll.offset.x,
           ps.rcPaint.top  + scroll.offset.y,
           SRCCOPY);
   )
   rc= BitBlt(
           paintDC,
           ps.rcPaint.left,
           ps.rcPaint.top ,
           ps.rcPaint.right-ps.rcPaint.left,
           ps.rcPaint.bottom-ps.rcPaint.top,
           hdc,
           ps.rcPaint.left + scroll.offset.x,
           ps.rcPaint.top  + scroll.offset.y,
           SRCCOPY);
   WINCHECK(rc, "BitBlt");

   EndPaint(hwnd, &ps);
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::resize
//
// Purpose-
//       Resize the Device
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   WinDevice::resize(               // Resize the Device
     const XYLength&   length)      // Length
{
   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::resize({%d,%d})\n",
                 __LINE__, this, length.x, length.y);
   )

   adjust(client.offset, length);
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::resizeBitmap
//
// Purpose-
//       Resize the bitmap.
//
//----------------------------------------------------------------------------
void
   WinDevice::resizeBitmap(         // Update the bitmap size
     const XYLength&   length)      // To this length
{
   int                 result;      // Resultant
   HBITMAP             hmap;        // Working BITMAP handle

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::resizeBitmap(%d,%d)\n",
                 __LINE__, this, length.x, length.y);
   )

   // Obtain the bitmap latch
   AutoMutex bitmapLock(bitmapMutex);

   // Create an updated bitmap
   TempDC tdc(hwnd);
   SetLastError(0);
   hmap= CreateCompatibleBitmap(
             (HDC)tdc,
             length.x,
             length.y);
   WINCHECK(hmap, "CreateCompatibleBitmap");

   // Update the bitmap
   if( hmap != NULL )
   {
     client.bitmap= length;         // Update the bitmap length

     // Replace any existing bitmap
     if( this->hmap != NULL)
       DeleteObject(this->hmap);

     this->hmap= hmap;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::run
//
// Purpose-
//       Run under thread control
//
//----------------------------------------------------------------------------
long                                // Thread completion code
   WinDevice::run( void )           // Work processor
{
   IFHCDM( Logger::log("%4d: WinDevice(%p)::run()\n", __LINE__, this); )

   //-------------------------------------------------------------------------
   // Initialize. Called from within Thread for window ownership
   operating= TRUE;
   initialize();
   started->post(0);

   // Processing loop
   while( operational )
   {
     int ec= poll();                // Process an event
     IFHCDM(
       Logger::log("%4d 0x%.4lx= WinDevice(%p)::poll()\n", __LINE__, ec, this);
     )
     if( ec == WM_CLOSE )           // If CLOSE event
       break;
   }

   // Termination sequencing
   if( operational )
     terminate();

   // Thread complete
   operating= FALSE;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::setAttribute
//
// Purpose-
//       Change attribute.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   WinDevice::setAttribute(         // Set Attribute
     int               attribute,   // Attribute identifier
     int               value)       // Attribute value
{
   int                 rc;

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::setAttribute(%d,%d)\n", __LINE__, this,
                 attribute, value);
   )

   switch( attribute )
   {
     case Object::VISIBLE:
       if( value )
       {
         rc= ShowWindow(hwnd, SW_NORMAL);
         WINDEBUG(rc, "ShowWindow");
       }
       else
       {
         rc= ShowWindow(hwnd, SW_HIDE);
         WINDEBUG(rc, "ShowWindow");
       }
       break;

     default:
       break;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::terminate
//
// Purpose-
//       Terminate device processing
//
//----------------------------------------------------------------------------
void
   WinDevice::terminate( void )     // Terminate the Device instance
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinDevice(%p)::terminate()\n", __LINE__, this); )

   // First, go non-operational
   AutoMutex statusLock(statusMutex);

   if( !operational )               // Only required once
     return;

   operational= FALSE;

   // Delete the Palette
   if( hpal != NULL )
   {
     rc= DeleteObject(hpal);
     WINCHECK(rc, "DeleteObject");
     hpal= NULL;
   }

   // Delete the Bitmap
   AutoMutex bitmapLock(bitmapMutex); // Holding statusMutex

   if( hmap != NULL)
   {
     DeleteObject(hmap);
     hmap= NULL;
   }

   // Delete the Device Context
   if( hdc != NULL )
   {
     rc= DeleteDC(hdc);
     WINCHECK(rc, "DeleteDC");

     rc= DeleteObject(hdc);
     WINCHECK(rc, "DeleteObject");
     hdc= NULL;
   }

   // Delete the Window
   if( hwnd != NULL )
   {
     rc= DestroyWindow(hwnd);
//// WINCHECK(rc, "DestroyWindow"); // ERROR_MOD_NOT_FOUND if window modified
     WINDEBUG(rc, "DestroyWindow");
     hwnd= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::updateScrolling
//
// Purpose-
//       Update the scrolling controls
//
//----------------------------------------------------------------------------
void
   WinDevice::updateScrolling(      // Update the scroll bars
     const RECT&       rect)        // Updated scroll client window
{
   BOOL                oldActive= scroll.active;
   BOOL                newActive;
   RECT                work;

   int                 rc;

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::updateScrolling(LTRB{%d,%d,%d,%d})\n",
                 __LINE__, this, rect.left, rect.top, rect.right, rect.bottom);
   )

   //-------------------------------------------------------------------------
   // Update the scroll client dimensions
   //-------------------------------------------------------------------------
   scroll.length= client.length;
   scroll.window.x= min(device.length.x, rect.right - rect.left);
   scroll.window.y= min(device.length.y, rect.bottom - rect.top);

   //-------------------------------------------------------------------------
   // Determine the new scrolling state
   //-------------------------------------------------------------------------
   newActive= client.window.x > scroll.window.x
           || client.window.y > scroll.window.y;

   #if ( VERBOSE > 0 ) && defined(HCDM)
     Logger::log("%4d: oldActive(%d) newActive(%d)\n", __LINE__,
                 oldActive, newActive);
   #endif

   if( oldActive != newActive )
   {
     rc= ShowScrollBar(hwnd, SB_BOTH, newActive);
     WINDEBUG(rc, "ShowScrollBar");

     scroll.offset.x= scroll.offset.y= 0;
     scroll.active= newActive;

     rc= UpdateWindow(hwnd);
     WINDEBUG(rc, "UpdateWindow");
   }

   //-------------------------------------------------------------------------
   // If scrolling is active, adjust scroll.length and the scroll offset
   //-------------------------------------------------------------------------
   if( scroll.active )
   {
     rc= GetClientRect(hwnd, &work);
     WINDEBUG(rc, "GetClientRect");
     logRect(__LINE__, work);

     scroll.length.x= work.right;
     scroll.length.y= work.bottom;

     updateScrollOffset(0, 0);
   }

   //-------------------------------------------------------------------------
   // If going from scroll active to inactive, refresh the window
   //-------------------------------------------------------------------------
   if( !newActive && oldActive )
     InvalidateRect(hwnd, NULL, TRUE);

   #if( VERBOSE > 2 )
     DEBUG();
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::updateScrollOffset
//
// Purpose-
//       Update the scroll offset.
//
//----------------------------------------------------------------------------
void
   WinDevice::updateScrollOffset(   // Update the scroll offsets
     int               dx,          // X (width) adjustment
     int               dy)          // Y (height) adjustment
{
   int                 adjust;      // Scroll adjustment
   SCROLLINFO          horz;        // Horizontal SCROLLINFO struct
   SCROLLINFO          vert;        // Vertical SCROLLINFO struct
   int                 rc;

   IFHCDM(
     Logger::log("%4d: WinDevice(%p)::updateScrollOffset(%d,%d)\n",
                 __LINE__, this, dx, dy);
   )

   if( scroll.active )
   {
     //-----------------------------------------------------------------------
     // Compute the scroll offset
     //-----------------------------------------------------------------------
     #if( VERBOSE > 5 )
       DEBUG();
     #endif
     dx= min(dx, client.length.x - (scroll.length.x - scroll.offset.x));
     dx= max(dx, -scroll.offset.x);
     dy= min(dy, client.length.y - (scroll.length.y - scroll.offset.y));
     dy= max(dy, -scroll.offset.y);
     scroll.offset.x += dx;
     scroll.offset.y += dy;

     //-----------------------------------------------------------------------
     // If the window is being expanded, adjust scroll.offset, dx and dy
     //-----------------------------------------------------------------------
     adjust= scroll.offset.x + scroll.length.x - client.length.x;
     adjust= min(adjust, scroll.offset.x);
     if( adjust > 0 )
     {
       dx -= adjust;
       scroll.offset.x -= adjust;
     }

     adjust= scroll.offset.y + scroll.length.y - client.length.y;
     adjust= min(adjust, scroll.offset.y);
     if( adjust > 0 )
     {
       dy -= adjust;
       scroll.offset.y -= adjust;
     }

     //-----------------------------------------------------------------------
     // Set the scroll controls
     //-----------------------------------------------------------------------
     horz.cbSize= sizeof(horz);
     horz.fMask= SIF_ALL | SIF_DISABLENOSCROLL;
     horz.nMin= 0;
     horz.nMax= client.length.x;
     horz.nPage= scroll.length.x;
     horz.nPos= scroll.offset.x;
     vert.cbSize= sizeof(vert);
     vert.fMask= SIF_ALL | SIF_DISABLENOSCROLL;
     vert.nMin= 0;
     vert.nMax= client.length.y;
     vert.nPage= scroll.length.y;
     vert.nPos= scroll.offset.y;

     #if ( VERBOSE > 2 ) && defined(HCDM)
       Logger::log("%4d: H(%3d): cbSize(%d) fMask(%x) nMin(%d) "
                   "nMax(%d) nPage(%d) nPos(%d)\n", __LINE__,
                   dx, horz.cbSize, horz.fMask, horz.nMin,
                   horz.nMax, horz.nPage, horz.nPos);

       Logger::log("%4d: V(%3d): cbSize(%d) fMask(%x) nMin(%d) "
                   "nMax(%d) nPage(%d) nPos(%d)\n", __LINE__,
                   dy, vert.cbSize, vert.fMask, vert.nMin,
                   vert.nMax, vert.nPage, vert.nPos);
     #endif

     if( dx != 0  || dy != 0 )
     {
       rc= ScrollWindow(hwnd, -dx, -dy, NULL, NULL);
       WINCHECK(rc, "ScrollWindow");
     }

     rc= SetScrollInfo(hwnd, SB_HORZ, &horz, TRUE);
     WINDEBUG(rc, "SetScrollInfo-H");

     rc= SetScrollInfo(hwnd, SB_VERT, &vert, TRUE);
     WINDEBUG(rc, "SetScrollInfo-V");

     #if( VERBOSE > 5 )
       DEBUG();
     #endif
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WinDevice::wait
//
// Purpose-
//       Wait while operational.
//
//----------------------------------------------------------------------------
long                                // The thread return code
   WinDevice::wait( void )          // Wait while operational
{
   IFHCDM( Logger::log("%4d: WinDevice(%p)::wait()\n", __LINE__, this); )

   return Thread::wait();
}
#include "gui/namespace.end"

