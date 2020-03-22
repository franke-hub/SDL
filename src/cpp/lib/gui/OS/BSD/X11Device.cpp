//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/X11Device.cpp
//
// Purpose-
//       Graphical User Interface: X11Device implementation.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
// Included only from OS/BSD/Device.cpp

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define VERSION_ID "X11Device/1.001"

#define USE_X11THREAD TRUE          // Use X11Thread?

#define MIN_WINDOW_X            128 // Minimum window width
#define MIN_WINDOW_Y            128 // Minimum window height

#define KB_SHIFT_L       0x00800000 // Left shift key depressed
#define KB_SHIFT_R       0x00400000 // Right shift key depressed
#define KB_META_L        0x00008000 // Left alt key depressed
#define KB_META_R        0x00004000 // Right alt key depressed
#define KB_CTRL_L        0x00000080 // Left alt key depressed
#define KB_CTRL_R        0x00000040 // Right altkey depressed

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define CHECKSTOP(name) checkstop(__LINE__, name)
#define X11CHECK(cc, name) x11check(__LINE__, int(cc), name)

#ifdef HCDM
#define X11DEBUG(rc, name) x11debug(__LINE__, int(rc), name)
#else
#define X11DEBUG(rc, name) (void)(rc)
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYOffset  zeroOffset= {0, 0}; // Offset zero
static const XYLength  unitLength= {1, 1}; // Length one

#ifdef HCDM
static const char*     eventName[]= // Event name array
{  "Invalid(0)-ERROR"               // 0
,  "Invalid(1)-REPLY"               // 1
,  "KeyPress"                       // 2
,  "KeyRelease"                     // 3
,  "ButtonPress"                    // 4
,  "ButtonRelease"                  // 5
,  "MotionNotify"                   // 6
,  "EnterNotify"                    // 7
,  "LeaveNotify"                    // 8
,  "FocusIn"                        // 9
,  "FocusOut"                       // 10
,  "KeymapNotify"                   // 11
,  "Expose"                         // 12
,  "GraphicsExpose"                 // 13
,  "NoExpose"                       // 14
,  "VisibilityNotify"               // 15
,  "CreateNotify"                   // 16
,  "DestroyNotify"                  // 17
,  "UnmapNotify"                    // 18
,  "MapNotify"                      // 19
,  "MapRequest"                     // 20
,  "ReparentNotify"                 // 21
,  "ConfigureNotify"                // 22
,  "ConfigureRequest"               // 23
,  "GravityNotify"                  // 24
,  "ResizeRequest"                  // 25
,  "CirculateNotify"                // 26
,  "CirculateRequest"               // 27
,  "PropertyNotify"                 // 28
,  "SelectionClear"                 // 29
,  "SelectionRequest"               // 30
,  "SelectionNotify"                // 31
,  "ColormapNotify"                 // 32
,  "ClientMessage"                  // 33
,  "MappingNotify"                  // 34
,  "GenericEvent"                   // 35
,  "Invalid(36)"                    // 36
,  "Invalid(37)"                    // 37
,  "Invalid(38)"                    // 38
,  "Invalid(39)"                    // 39
}; // eventName
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//
// Purpose-
//       Handle checkstop condition.
//
//----------------------------------------------------------------------------
static void
   checkstop(                       // Check stop
     int               line,        // Line number
     const char*       name)        // Function name
{
   fprintf(stderr, "%4d CHECKSTOP(%s)\n", line, name);
   Logger::log("%4d CHECKSTOP(%s)\n", line, name);
   Logger::get()->flush();
   throw "Device::checkstop()";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       x11debug
//
// Purpose-
//       Display an X11 result.
//
//----------------------------------------------------------------------------
static void
   x11debug(                        // Log X11 function result
     int               line,        // Line number
     int               rc,          // Return code
     const char*       name)        // Function name
{
   Logger::log("%4d %d= %s()\n", line, rc, name);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       x11check
//
// Purpose-
//       Validate an X11 result.
//
//----------------------------------------------------------------------------
static void
   x11check(                        // Log X11 function result
     int               line,        // Line number
     int               cc,          // Condition code
     const char*       name)        // Function name
{
   #ifdef HCDM
     x11debug(line, cc, name);
     if( cc == 0 )
       checkstop(line, "X11Device::x11check");
   #else
     if( cc == 0 )
     {
       fprintf(stderr, "%4d %d= %s()\n", line, cc, name);
       Logger::log("%4d %d= %s()\n", line, cc, name);
       checkstop(line, "X11Device::x11check");
     }
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       x11error
//
// Purpose-
//       Handle an X11 error.
//
//----------------------------------------------------------------------------
static int
   x11error(                        // Handle X11 error
     Display*          disp,        // -> Display
     XErrorEvent*      code)        // -> XErrorEvent
{
   char buffer[1024];
   XGetErrorText(disp, code->error_code, buffer, sizeof(buffer));
   debugf("_X Error of failed request: %s\n"
          "_  Major opcode of failed request: %d\n"
          "_  Serial number of failed request: %ld\n"
          , buffer, code->request_code, code->serial);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       x11fatal
//
// Purpose-
//       Handle an X11 fatal error.
//
//----------------------------------------------------------------------------
static int
   x11fatal(                        // Handle X11 error
     Display*          disp)        // -> Display
{
   fprintf(stderr, "_X fatal error, goodbye.\n");
   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       eventDump
//
// Purpose-
//       Dump event data
//
//----------------------------------------------------------------------------
#ifdef HCDM
static void
   eventDump(                       // Dump event data
     const void*       rdda,        // Buffer address
     unsigned          size)        // Buffer length
{
   char                buffer[128]; // Working buffer
   unsigned            O= 0;        // Working buffer offset

   const char* addr= (const char*)rdda;
   for(int i= 0; i<size; i++)
   {
     int C= addr[i] & 0x000000ff;
     if( i != 0 && (i % 4) == 0 )
       buffer[O++]= ' ';

     O += sprintf(buffer+O, "%.2x", C);
   }

   O += sprintf(buffer+O, " *");
   for(int i= 0; i<size; i++)
   {
     int C= addr[i] & 0x000000ff;
     if( !isprint(C) )
       C= '~';
     buffer[O++]= C;
   }
   buffer[O]= '\0';

   tracef("..%s*\n", buffer);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       eventDebug
//
// Purpose-
//       Debug event data
//
//----------------------------------------------------------------------------
#ifdef HCDM
static void
   eventDebug(                      // Dump event data
     XEvent&           e)           // (OUTPUT) Event
{
   //-------------------------------------------------------------------------
   // Diagnostic display
   Logger::log("%4d X11Device XNextEvent(%d) %s\n"
          "..serial(%lu)\n"
          "..send_event(%d)\n"
          "..display(%p)\n"
          "..window(%d)\n"
          , __LINE__
          , e.type, (e.type > 0 && e.type < 36) ? eventName[e.type] : "<Unknown>"
          , e.xany.serial
          , e.xany.send_event
          , e.xany.display
          , int(e.xany.window)
          );

   const char* text;
   switch(e.type)
   {
     case ClientMessage:
       tracef("..message_type(%d) format(%d)\n"
              , int(e.xclient.message_type), e.xclient.format
              );
       eventDump(e.xclient.data.b, sizeof(e.xclient.data.b));
       break;

     case ConfigureNotify:
       tracef("..x(%d) y(%d)\n"
              "..width(%d) height(%d)\n"
              "..border_width(%d) above(%d) override_redirect(%d)\n"
              , e.xconfigure.x, e.xconfigure.y
              , e.xconfigure.width, e.xconfigure.height
              , e.xconfigure.border_width
              , int(e.xconfigure.above)
              , int(e.xconfigure.override_redirect)
              );
       break;

     case EnterNotify:
     case LeaveNotify:
       tracef("..root(%d) subwindow(%d)\n"
              "..time(%ld)\n"
              "..x(%d) y(%d)\n"
              "..x_root(%d) y_root(%d)\n"
              "..mode(%d) detail(%d)\n"
              "..same_screen(%d) focus(%d) state(%d)\n"
              , int(e.xcrossing.root), int(e.xcrossing.subwindow)
              , long(e.xcrossing.time)
              , e.xcrossing.x, e.xcrossing.y
              , e.xcrossing.x_root, e.xcrossing.y_root
              , e.xcrossing.mode, e.xcrossing.detail
              , e.xcrossing.same_screen, e.xcrossing.focus, e.xcrossing.state
              );
       break;

     case Expose:
       tracef("..x(%d) y(%d)\n"
              "..width(%d) height(%d)\n"
              "..count(%d)\n"
              , e.xexpose.x, e.xexpose.y
              , e.xexpose.width, e.xexpose.height
              , e.xexpose.count
              );
       break;

     case FocusIn:
     case FocusOut:
       tracef("..mode(%d) detail(%d)\n"
              , e.xfocus.mode, e.xfocus.detail
              );
       break;

     case GraphicsExpose:
       tracef("..x(%d) y(%d)\n"
              "..width(%d) height(%d)\n"
              "..count(%d)\n"
              "..major(%d) minor(%d)\n"
              , e.xgraphicsexpose.x, e.xgraphicsexpose.y
              , e.xgraphicsexpose.width, e.xgraphicsexpose.height
              , e.xgraphicsexpose.count
              , e.xgraphicsexpose.major_code
              , e.xgraphicsexpose.minor_code
              );
       break;

     case KeymapNotify:
       eventDump(e.xkeymap.key_vector, sizeof(e.xkeymap.key_vector));
       break;

     case KeyPress:
     case KeyRelease:
       tracef("..time(%ld)\n"
              "..x(%d) y(%d)\n"
              "..x_root(%d) y_root(%d)\n"
              "..state(%d) keycode(%d) same_screen(%d)\n"
              , long(e.xkey.time)
              , e.xkey.x, e.xkey.y
              , e.xkey.x_root, e.xkey.y_root
              , e.xkey.state, e.xkey.keycode, e.xkey.same_screen
              );
       break;

     case MapNotify:
       tracef("..event(%d) window(%d) override_redirect(%d)\n"
              , int(e.xmap.event), int(e.xmap.window)
              , int(e.xmap.override_redirect)
              );
       break;

#if( 0 ) // Too much work/event if we're really interested
     case MotionNotify:
       tracef("..root(%d) subwindow(%d)\n"
              "..time(%ld)\n"
              "..x(%d) y(%d)\n"
              "..x_root(%d) y_root(%d)\n"
              "..state(%d) is_hint(%d) same_screen(%d)\n"
              , int(e.xmotion.root), int(e.xmotion.subwindow)
              , long(e.xmotion.time)
              , e.xmotion.x, e.xmotion.y
              , e.xmotion.x_root, e.xmotion.y_root
              , e.xmotion.state, e.xmotion.is_hint, e.xmotion.same_screen
              );
       break;
#endif

     case PropertyNotify:
       tracef("..time(%d) atom(%d) state(%d)\n"
              , int(e.xproperty.time), int(e.xproperty.atom)
              , e.xproperty.state
              );
       break;

     case ResizeRequest:
       tracef("..width(%d) height(%d)\n"
              , e.xresizerequest.width, e.xresizerequest.height
              );
       break;

     case VisibilityNotify:
       text= "<< INVALID STATE >>";
       if( e.xvisibility.state == VisibilityUnobscured )
         text= "Unobscured";
       else if( e.xvisibility.state == VisibilityPartiallyObscured )
         text= "PartiallyObscured";
       else if( e.xvisibility.state == VisibilityFullyObscured )
         text= "FullyObscured";
       tracef("..state(%d) Visiblity%s\n", e.xvisibility.state, text);
       break;

     default:
       break;
   }
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::~X11Device
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   X11Device::~X11Device( void )    // Destructor
{
   IFHCDM( Logger::log("%4d X11Device(%p)::~X11Device()\n", __LINE__, this); )

   int                 rc;

   operational= FALSE;

   // Kill the server thread
   #if( USE_X11THREAD )
     thread.notify(0);
     thread.wait();
   #endif

   // Release all X11 resources
   if( ximg != NULL )
   {
     rc= XDestroyImage(ximg);
     X11DEBUG(rc, "XDestroyImage");
     ximg= NULL;
   }

   if( disp != NULL )
   {
     rc= XFreeGC(disp, xgco);
     X11DEBUG(rc, "XFreeGC");
     rc= XDestroyWindow(disp, xwin);
     X11DEBUG(rc, "XDestroyWindow");
//   rc= XSetCloseDownMode(disp, DestroyAll);
//   X11DEBUG(rc, "XSetCloseDownMode");
     rc= XCloseDisplay(disp);
     X11DEBUG(rc, "XCloseDisplay");
     disp= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::X11Device
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   X11Device::X11Device(            // Constructor
     GUI::Window*      window)      // Source Window
:  Device(window)
,  operational(FALSE)
,  thread(this)
,  unitMutex()
,  kb_state(0)
,  wm_delete(0)
,  disp(NULL)
,  xscr(0)
,  xvis(NULL)
,  xwin(0)
,  xgco(0)
,  ximg(NULL)
{
   IFHCDM(
     Logger::log("%4d X11Device(%p)::X11Device(%p)\n", __LINE__, this, window);
     Logger::log("%4d USE_X11THREAD=%d\n", __LINE__, USE_X11THREAD);
     )

   // Initialize the attributes
// debugf("%4d DEVICE HCDM\n", __LINE__);
   device.offset= zeroOffset;
   device.length= window->getLength();
   device.length.x= max(device.length.x, MIN_WINDOW_X);
   device.length.y= max(device.length.y, MIN_WINDOW_Y);

   // Open the display
   disp= XOpenDisplay(getenv("DISPLAY"));
   X11CHECK(disp!=NULL, "XOpenDisplay");
   xscr= DefaultScreen(disp);       // We use the default screen
   XSetErrorHandler(x11error);
   XSetIOErrorHandler(x11fatal);

   // Locate an acceptable Visual
   int count= VisualDepthMask   | VisualBitsPerRGBMask
            | VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask;
   XVisualInfo temp;
   temp.depth= 24;
   temp.red_mask=   0x00ff0000;
   temp.green_mask= 0x0000ff00;
   temp.blue_mask=  0x000000ff;
   temp.bits_per_rgb= 8;
   XVisualInfo* info= XGetVisualInfo(disp, count, &temp, &count);
   X11CHECK(info!=NULL, "XGetVisualInfo");
   if( count == 0 )
   {
     debugf("X11Device NotCapableException\n");
     throw "X11DeviceNotCapableException";
   }

   xvis= info[0].visual;
   XFree(info);

   // Configure the Device
   config(device.length);

   // Go operational
   operational= TRUE;

   // Start the client Thread (Requires operational)
   #if( USE_X11THREAD )
     thread.start();
   #endif

   // Initialize the backing store (Requires operational)
   change(zeroOffset, device.length);
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::setAttribute
//
// Purpose-
//       Change attribute.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   X11Device::setAttribute(         // Set Attribute
     int               attribute,   // Attribute identifier
     int               value)       // Attribute value
{
   int                 rc;

   IFHCDM( Logger::log("%4d X11Device(%p)::setAttribute(%d,%d)\n",
                       __LINE__, this, attribute, value); )

   int state= (value != 0);
   switch( attribute )
   {
     case VISIBLE:
       if( getAttribute(value) == state )
         break;

       {{{{
         AutoMutex lock(unitMutex);

         if( value )
         {
           rc= XMapWindow(disp, xwin);
           X11DEBUG(rc, "XMapWindow");
         }
         else
         {
           rc= XUnmapWindow(disp, xwin);
           X11DEBUG(rc, "XUnmapWindow");
         }
       }}}}

       #if( USE_X11THREAD == FALSE )
         if( state )
           expose(zeroOffset, window->getLength());
         flush();
       #endif
       break;

     default:
       break;
   }

   Attributes::setAttribute(attribute, value);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::change
//
// Purpose-
//       Device change.
//
//----------------------------------------------------------------------------
void
   X11Device::change(               // Device changed
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   IFHCDM( Logger::log("%4d X11Device(%p)::change({%d,%d},{%d,%d})\n",
                       __LINE__, this,
                       offset.x, offset.y, length.x, length.y); )

   Pixel*              pixel;       // Working -> Pixel
   XOffset_t           oX;          // Working XOffset
   YOffset_t           oY;          // Working YOffset

   // State check
   if( !operational )
   {
     x11debug(__LINE__, 1, "NonOperational"); // (Force x11debug reference)
     CHECKSTOP("X11Device::change");
   }

   // Handle any pending events
   #if( USE_X11THREAD == FALSE )
     flush();
   #endif

   // Make the change (whether or not visible)
   {{{{                             // While holding the unitMutex
     AutoMutex lock(unitMutex);

     const XYLength& winlen= window->getLength();
     unsigned maxX= min(winlen.x, offset.x + length.x);
     maxX= min(maxX, device.length.x);
     unsigned maxY= min(winlen.y, offset.y + length.y);
     maxY= min(maxY, device.length.y);
     for(oY= offset.y; oY<maxY; oY++)
     {
       pixel= window->getPixel(offset.x, oY);
       for(oX= offset.x; oX<maxX; oX++)
       {
         XPutPixel(ximg, oX, oY, pixel->color);
         pixel++;
       }
     }
   }}}}

   // Expose the change
   if( getAttribute(VISIBLE) )
     expose(offset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::config
//
// Purpose-
//       Configure the device.
//
// Implementation notes-
//       Locking is the responsibility of the caller.
//
//----------------------------------------------------------------------------
void
   X11Device::config(               // Configure the device
     const XYLength&   length)      // Length (of backing store)
{
   IFHCDM( Logger::log("%4d X11Device(%p)::config({%d,%d})\n",
                       __LINE__, this, length.x, length.y); )

   // Create the window
   int atFlags= CWBackPixel;        // Background pixel
   XSetWindowAttributes atValue;
   atValue.background_pixel= 0;     // (Black background)
   xwin= XCreateWindow(             // Create the window
             disp,                  // On this display
             DefaultRootWindow(disp), // With this parent
             0,                     // (Use default x offset)
             0,                     // (Use default y offset)
             device.length.x,       // Width
             device.length.y,       // Height
             0,                     // Border width
             24,                    // Depth
             InputOutput,           // Class
             xvis,                  // Visual
             atFlags,               // Attribute flags
             &atValue);             // Attribute values
   X11DEBUG(xwin, "XCreateWindow");

   // Set the title
   int rc= XStoreName(disp, xwin, VERSION_ID);
   X11DEBUG(rc, "XStoreName");

   // Extract the window attributes
   XWindowAttributes xwa;
   rc= XGetWindowAttributes(disp, DefaultRootWindow(disp), &xwa);
   X11DEBUG(rc, "XGetWindowAttributes");
   screen.length.x= xwa.width;
   screen.length.y= xwa.height;

   rc= XGetWindowAttributes(disp, xwin, &xwa);
   X11DEBUG(rc, "XGetWindowAttributes");
   device.offset.x= xwa.x;
   device.offset.y= xwa.y;

   // We want WM_DELETE_WINDOW messages to be received using a ClientMessage
   // event when the user clicks the WM close button.
   wm_delete= XInternAtom(disp, "WM_DELETE_WINDOW", 0);
   X11DEBUG(wm_delete, "XInternalAtom(WM_DELETE_WINDOW)");
   rc= XSetWMProtocols(disp, xwin, &wm_delete, 1);
   X11DEBUG(rc, "XSetWMProtocols");

   //-------------------------------------------------------------------------
   // Register for Window Events
   // (google: xlib mask event correspondence)
   rc= KeyPressMask | KeyReleaseMask       // Required: KeyPress/Release
     | ButtonPressMask | ButtonReleaseMask // Required: ButtonPress/Release
//   | EnterWindowMask | LeaveWindowMask   // Required: Enter/LeaveNotify
     | PointerMotionMask                   // Required: MotionNotify
//   | PointerMotionHintMask               // Not useful: (No XEvent)
//   | Button1MotionMask
//   | Button2MotionMask
//   | Button3MotionMask
//   | Button4MotionMask
//   | Button5MotionMask
//   | ButtonMotionMask                    // Required: MotionNotify
//   | KeymapStateMask                     // Not useful
     | ExposureMask                        // Required: Expose
//   | VisibilityChangeMask                // Not useful: (Pointer in window)
     | StructureNotifyMask                 // Required: ConfigureNotify
//   | ResizeRedirectMask                  // Not useful, use StructureNotifyMask
//   | SubstructureNotifyMask              // Not useful
//   | SubstructureRedirectMask            // Not useful
//   | FocusChangeMask
//   | PropertyChangeMask
//   | ColormapChangeMask
//   | OwnerGrabButtonMask
     ;
   rc= XSelectInput(disp, xwin, rc);
   X11DEBUG(rc, "XSelectInput");

   // Write debugging information
   IFHCDM(
     Logger::log("Display(%p) Screen(%d,{%lu,%lu})\n",
                 disp, xscr, long(screen.length.x) ,long(screen.length.y));
     Logger::log("Window(%ld,{%lu,%lu},{%lu,%lu})\n",
                 long(xwin), long(device.offset.x), long(device.offset.y),
                 long(device.length.x), long(device.length.y));
   )

   // Create the Context
   int gcFlags= 0;
   XGCValues gcValues;
   xgco= XCreateGC(disp, xwin, gcFlags, &gcValues);
   X11CHECK(xgco!=NULL, "XCreateGC");

   // Create the Image
   unsigned size= length.x * length.y * sizeof(long);
   char* data= (char*)Unconditional::malloc(size);
   ximg= XCreateImage(disp, xvis, 24, ZPixmap, 0, data,
                      length.x, length.y, 32, 0);
   X11CHECK(ximg!=NULL, "XCreateImage");

   // Map the Window
   if( getAttribute(VISIBLE) )
   {
     rc= XMapWindow(disp, xwin);
     X11DEBUG(rc, "XMapWindow");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::expose
//
// Purpose-
//       Expose change.
//
//----------------------------------------------------------------------------
void
   X11Device::expose(               // Expose Buffer changed
     const XYOffset&   offset,      // Offset
     const XYLength&   length)      // Length
{
   IFHCDM( Logger::log("%4d X11Device(%p)::expose({%d,%d},{%d,%d})\n",
                       __LINE__, this,
                       offset.x, offset.y, length.x, length.y); )

   // Expose the change
   {{{{                             // While holding the unitMutex
     AutoMutex lock(unitMutex);

     int rc;
     rc= XPutImage(disp, xwin, xgco, ximg, offset.x, offset.y,
                   offset.x, offset.y, length.x, length.y);
     X11DEBUG(rc, "XPutImage");
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::flush
//
// Purpose-
//       Wait for all pending XEvents
//
// Implementation note-
//       Locking considerations: See X11Device::nextEvent
//
//----------------------------------------------------------------------------
void
   X11Device::flush( void )         // Wait for all pending XEvents
{
   for(;;)
   {
     int rc= XPending(disp);
     X11DEBUG(rc, "XPending");
     if( rc == 0 )
       break;

     XEvent e;
     nextEvent(e);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::move
//
// Purpose-
//       Position the Device (in the parent screen)
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   X11Device::move(                 // Position the Device
     const XYOffset&   offset)      // Offset
{
   IFHCDM( Logger::log("%4d X11Device(%p)::move({%d,%d})\n",
                       __LINE__, this, offset.x, offset.y); )

// device.offset= offset;           // NOT CODED YET

   return "NOT CODED YET";
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::nextEvent
//
// Purpose-
//       Select, read next event
//
// Implementation note-
//       device->nextEvent() can be called either ONLY by the device
//       (when USE_X11THREAD == FALSE,) or ONLY by the thread. We don't
//       want both the thread and the device sharing X calls.
//
//       We MUST be called without the unitMutex lock held. If an X operation
//       is required, we obtain the unitMutex. When calling callback, the
//       unitMutex lock must NOT be held, or we can deadlock.
//
//----------------------------------------------------------------------------
void
   X11Device::nextEvent(            // Select, read next event
     XEvent&           e)           // (OUTPUT) Event

{
   int                 rc;

   IFHCDM( Logger::log("%4d X11Device(%p)::nextEvent()\n", __LINE__, this); )

   //-------------------------------------------------------------------------
   // Wait for the next XEvent
   memset(&e, 0, sizeof(e));
   rc= XNextEvent(disp, &e);
   X11DEBUG(rc, "XNextEvent");

   //-------------------------------------------------------------------------
   // Handle the event
   IFHCDM(eventDebug(e);)           // Diagnostic debug

   char                buffer[32];  // Working buffer
   unsigned            code;        // The key code
   Event::EC           ec;          // Event code
   int                 ed;          // Event data
   XYLength            length;      // Length
   XYOffset            offset;      // Offset
   KeySym              sym;         // Keycode symbol
   XWindowAttributes   xwa;         // XWindowAttributes
   switch(e.type)
   {
     case ClientMessage:
       if( int(e.xclient.data.l[0]) == int(wm_delete) )
       {
         IFHCDM(Logger::log("WM_DELETE_WINDOW event=====================\n");)

         operational= false;
         #if( USE_X11THREAD )
           thread.notify(0);
         #endif

         callback(Event::EC_TERMINATE, 0, zeroOffset, unitLength);
       }
       break;

     case ConfigureNotify:
       length.x= e.xconfigure.width;
       length.y= e.xconfigure.height;
       if( length.x == device.length.x && length.y == device.length.y )
         break;

       {{{{                         // While holding the unitMutex
         AutoMutex lock(unitMutex);

         rc= XGetWindowAttributes(disp, xwin, &xwa);
         X11DEBUG(rc, "XGetWindowAttributes");
         if( xwa.width == length.x && xwa.height == length.y )
         {
           //-----------------------------------------------------------------
           // At long, long last, we are resized. In the end, so simple.
           //
           // We create a second Image, zero it, then copy the current Image
           // on top of it. X will generate the appropriate expose events
           unsigned size= length.x * length.y * sizeof(long);
           char* data= (char*)Unconditional::malloc(size);
           memset(data, 0, size);
           XImage* yimg= XCreateImage(disp, xvis, 24, ZPixmap, 0, data,
                                      length.x, length.y, 32, 0);
           X11CHECK(yimg!=NULL, "XCreateImage");

           if( ximg != NULL )       // Pretty much always true
           {
             XLength_t maxX= min(device.length.x, length.x);
             YLength_t maxY= min(device.length.y, length.y);
             for(YOffset_t y= 0; y<maxY; y++)
             {
               for(XOffset_t x= 0; x<maxX; x++)
                 XPutPixel(yimg, x, y, XGetPixel(ximg, x, y));
             }

             rc= XDestroyImage(ximg);
             X11DEBUG(rc, "XDestroyImage");
           }

           ximg= yimg;
           device.length.x= length.x;
           device.length.y= length.y;
         }}}}
       }

       callback(Event::EC_RESIZE, 0, zeroOffset, length);
       break;

     case Expose:
       offset.x= e.xexpose.x;
       offset.y= e.xexpose.y;
       length.x= e.xexpose.width;
       length.y= e.xexpose.height;
       expose(offset, length);
       break;

     case KeyPress:
     case KeyRelease:
       offset.x= e.xkey.x;
       offset.y= e.xkey.y;
////   memset(buffer, 0, sizeof(buffer)); // (No length returned)
       {{{{                         // While holding the unitMutex
         AutoMutex lock(unitMutex);
         rc= XLookupString((XKeyEvent*)&e, buffer, sizeof(buffer), &sym, NULL);
         X11DEBUG(rc, "XLookupString");
       }}}}

       code= unsigned(sym);
       if( e.xkey.type == KeyRelease )
       {
         switch(code)
         {
           case XK_Shift_L:
             kb_state &= ~(KB_SHIFT_L);
             break;

           case XK_Shift_R:
             kb_state &= ~(KB_SHIFT_R);
             break;

           case XK_Alt_L:
           case XK_Meta_L:
             kb_state &= ~(KB_META_L);
             break;

           case XK_Alt_R:
           case XK_Meta_R:
             kb_state &= ~(KB_META_R);
             break;

           case XK_Control_L:
             kb_state &= ~(KB_CTRL_L);
             break;

           case XK_Control_R:
             kb_state &= ~(KB_CTRL_R);
             break;

           default:
             break;
         }

         return;
       }

       // Keypress. Note: No distinction made between normal and keypad codes.
       switch(code)
       {
         case XK_Shift_L:           // We manage shift keys
           kb_state |= KB_SHIFT_L;
           return;

         case XK_Shift_R:
           kb_state |= KB_SHIFT_R;
           return;

         case XK_Alt_L:
         case XK_Meta_L:
           kb_state |= KB_META_L;
           return;

         case XK_Alt_R:
         case XK_Meta_R:
           kb_state |= KB_META_R;
           return;

         case XK_Control_L:
           kb_state |= KB_CTRL_L;
           return;

         case XK_Control_R:
           kb_state |= KB_CTRL_R;
           return;

         case XK_Caps_Lock:         // Xlib manages these lock keys
         case XK_Num_Lock:
         case XK_Shift_Lock:
           return;

         case XK_Scroll_Lock:       // Translations to private UNICODE space
           code= GUI::KeyCode::ScrollLock;
           break;

         case XK_Pause:
           code= GUI::KeyCode::Pause;
           break;

         case XK_Print:
           code= GUI::KeyCode::Print;
           break;

         case XK_BackSpace:
           code= GUI::KeyCode::BS;
           break;

         case XK_Tab:
           code= GUI::KeyCode::TAB;
           break;

         case XK_ISO_Left_Tab:
           code= GUI::KeyCode::TAB | GUI::KeyCode::_SHIFT;
           break;

         case XK_Return:
         case XK_KP_Enter:
           code= GUI::KeyCode::ENTER;
           break;

         case XK_Escape:
           code= GUI::KeyCode::ESC;
           break;

         case XK_Up:
         case XK_KP_Up:
           code= GUI::KeyCode::Up;
           break;

         case XK_Down:
         case XK_KP_Down:
           code= GUI::KeyCode::Down;
           break;

         case XK_Left:
         case XK_KP_Left:
           code= GUI::KeyCode::Left;
           break;

         case XK_Right:
         case XK_KP_Right:
           code= GUI::KeyCode::Right;
           break;

         case XK_KP_Begin:
           code= GUI::KeyCode::Center;
           break;

         case XK_Home:
         case XK_KP_Home:
           code= GUI::KeyCode::Home;
           break;

         case XK_End:
         case XK_KP_End:
           code= GUI::KeyCode::End;
           break;

         case XK_Page_Up:
         case XK_KP_Page_Up:
           code= GUI::KeyCode::PageUp;
           break;

         case XK_Page_Down:
         case XK_KP_Page_Down:
           code= GUI::KeyCode::PageDown;
           break;

         case XK_Insert:
         case XK_KP_Insert:
           code= GUI::KeyCode::Insert;
           break;

         case XK_Delete:
         case XK_KP_Delete:
           code= GUI::KeyCode::Delete;
           break;

         case XK_KP_Separator:
           code= GUI::KeyCode::Comma;
           break;

         case XK_KP_Decimal:
           code= GUI::KeyCode::Period;
           break;

         case XK_KP_Add:
           code= GUI::KeyCode::PlusSign;
           break;

         case XK_KP_Subtract:
           code= GUI::KeyCode::Hyphen;
           break;

         case XK_KP_Multiply:
           code= GUI::KeyCode::Asterisk;
           break;

         case XK_KP_Divide:
           code= GUI::KeyCode::RightSlash;
           break;

         case XK_KP_0:
         case XK_KP_1:
         case XK_KP_2:
         case XK_KP_3:
         case XK_KP_4:
         case XK_KP_5:
         case XK_KP_6:
         case XK_KP_7:
         case XK_KP_8:
         case XK_KP_9:
           code= GUI::KeyCode::_0_ + (code - XK_KP_0);
           break;

         case XK_F1:
         case XK_F2:
         case XK_F3:
         case XK_F4:
         case XK_F5:
         case XK_F6:
         case XK_F7:
         case XK_F8:
         case XK_F9:
         case XK_F10:
         case XK_F11:
         case XK_F12:
         case XK_F13:
         case XK_F14:
         case XK_F15:
         case XK_F16:
         case XK_F17:
         case XK_F18:
         case XK_F19:
         case XK_F20:
         case XK_F21:
         case XK_F22:
         case XK_F23:
         case XK_F24:
         case XK_F25:
         case XK_F26:
         case XK_F27:
         case XK_F28:
         case XK_F29:
         case XK_F30:
         case XK_F31:
         case XK_F32:
         case XK_F33:
         case XK_F34:
         case XK_F35:
           code= GUI::KeyCode::F01 + (code - XK_F1);
           break;

         default:
           if( code >= 0x000000ff )
             code= GUI::KeyCode::_CODE | e.xkey.keycode;
           break;
       }

       if( (kb_state & (KB_SHIFT_L | KB_SHIFT_R)) != 0 )
       {
         if( (code >= 0x0000f800 && code <= 0x0000f8ff)
             || code <= 0x0000001f || (code & GUI::KeyCode::_CODE) != 0 )
           code |= GUI::KeyCode::_SHIFT;
       }

       if( (kb_state & (KB_META_L | KB_META_R)) != 0 )
         code |= GUI::KeyCode::_ALT;

       if( (kb_state & (KB_CTRL_L | KB_CTRL_R)) != 0 )
         code |= GUI::KeyCode::_CTRL;

       IFHCDM( Logger::log("%4d out(0x%.8x) inp(0x%.8x) state(0x%.4x)\n",
                       __LINE__, code, e.xkey.keycode, kb_state); )
       ec= Event::EC_KEYDOWN;
       callback(ec, code, offset, unitLength);
       break;

     case MotionNotify:
       offset.x= e.xmotion.x;
       offset.y= e.xmotion.y;
       callback(Event::EC_MOUSEOVER, 0, offset, unitLength);
       break;

     case EnterNotify:
     case LeaveNotify:
       offset.x= e.xcrossing.x;
       offset.y= e.xcrossing.y;
       ed= Event::MO_ENTER;
       if( e.type == LeaveNotify )
         ed= Event::MO_EXIT;
       callback(Event::EC_MOUSEOVER, ed, offset, unitLength);
       break;

     default:
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::resize
//
// Purpose-
//       Resize the Device
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   X11Device::resize(               // Resize the Device
     const XYLength&   length)      // Length
{
   IFHCDM( Logger::log("%4d X11Device(%p)::resize({%d,%d})\n",
                       __LINE__, this, length.x, length.y); )

   // If required, Reconfigure
   if( length.x != device.length.x || length.y != device.length.y )
   {
     device.length.x= max(length.x, MIN_WINDOW_X);
     device.length.y= max(length.y, MIN_WINDOW_Y);

     {{{{                           // While holding the unitMutex
       AutoMutex lock(unitMutex);

       // Destroy the Window
       int rc= XDestroyWindow(disp, xwin);
       X11DEBUG(rc, "XDestroyWindow");

       // Delete the graphics context
       rc= XFreeGC(disp, xgco);
       X11DEBUG(rc, "XFreeGC");

       // Delete the image
       // TODO: As in nextEvent?
       if( ximg != NULL )
       {
         rc= XDestroyImage(ximg);
         X11DEBUG(rc, "XDestroyImage");
         ximg= NULL;
       }

       // Create the new backing store
       config(device.length);
     }}}}

     // Initialize the new backing store
     change(zeroOffset, device.length);
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::wait
//
// Purpose-
//       Wait while operational
//
//----------------------------------------------------------------------------
long                                // (UNUSED) Return code
   X11Device::wait( void )          // Wait for Device termination
{
   IFHCDM( Logger::log("%4d X11Device(%p)::wait()\n", __LINE__, this); )

   #if( USE_X11THREAD )
     thread.wait();
   #else
     for(;;)
     {
       if( !operational )
         break;

       XEvent e;
       nextEvent(e);
     }
   #endif

   return 0;
}

