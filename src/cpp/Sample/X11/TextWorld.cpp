//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TextWorld.cpp
//
// Purpose-
//       Sample program that writes text on a Window and/or a Pixmap.
//
// Last change date-
//       2010/01/01
//
// Implementation notes-
//       Used to determine a mechanism for text display with specified font.
//       Now extended to use keypress.
//
// Implementation notes-
//       The XImage associated with a Pixmap appears to change every time
//       the Pixmap is modified. It must be allocated/release for each use.
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Interval.h>
#include <com/Thread.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#define USE_X11ERROR FALSE          // TRUE to use x11error subroutine
#define USE_X11FATAL FALSE          // TRUE to use x11fatal subroutine
#define USE_ISO8859  FALSE          // TRUE to use ISO8859 example
#define USE_WINDOW   TRUE           // TRUE to write to Window and Pixmap

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define CHECKSTOP(name) checkstop(__LINE__, name)
#define X11CHECK(cc, name) x11check(__LINE__, int(cc), name)

#ifdef HCDM
#define X11DEBUG(rc, name) x11debug(__LINE__, int(rc), name)
#else
#define X11DEBUG(rc, name)
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Display*        disp;        // The X server connection
static int             xscr;        // The default screen
static Window          xwin;        // Our Window
static int             xwinHeight= 64; // Window height
static int             xwinWidth=  768; // Window width
static GC              wgco;        // Our Graphics Context (Window)
static XImage*         ximg= NULL;  // Our XImage
static Pixmap          xmap;        // Our Pixmap
static GC              mgco;        // Our Graphics Context (Map)

static XFontStruct*    font= NULL;  // Our font
static int             fontWidth;   // The Font width
static int             fontHeight;  // The Font height

static int             shift= 0;    // Shift key state

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const char      message[]=   // Initial message
{  'H', 'e', 'l', 'l', 'o', ' '
,  't', 'e', 'x', 't', ' '
,  'w', 'o', 'r', 'l', 'd'
#if( USE_ISO8859 )
,  ',', ' '
,  'W', 'a', 't', 'e', 'r', '(', 0xe6, 0xb0, 0xb4,  ')',  ',',  ' '
,  'G', '-', 'c', 'l', 'e', 'f',  '(', 0xf0, 0x9d, 0x84, 0x9e,  ')'
#endif
,  '\0'
};

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
   debugf("%4d CHECKSTOP(%s)\n", line, name);
   Debug::get()->flush();
   throw "checkstop()";
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
   tracef("%4d 0x%x= %s()\n", line, rc, name);
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
     int               cc,          // Assertion code (must be TRUE)
     const char*       name)        // Function name
{
   #ifdef HCDM
     x11debug(line, cc, name);
     if( cc == 0 )
       checkstop(line, "x11check");
   #else
     if( cc == 0 )
     {
       x11debug(line, cc, name);
       checkstop(line, "x11check");
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
#if( USE_X11ERROR )
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

   throw "X11ERROR";
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       x11fatal
//
// Purpose-
//       Handle an X11 fatal error.
//
//----------------------------------------------------------------------------
#if( USE_X11FATAL )
static int
   x11fatal(                        // Handle X11 error
     Display*          disp)        // -> Display
{
   fprintf(stderr, "_X fatal error, goodbye.\n");
   return 1;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       initWindow
//
// Purpose-
//       Do everything needed to connect to the X server and open a Window.
//
//----------------------------------------------------------------------------
static void
   initWindow(                      // Open the Window
     int               argc,        // Argument count (For XSetWMProperties)
     char**            argv)        // Argument array (For XSetWMProperties)
{
   int                 rc;

   // Connect to the X server
   disp= XOpenDisplay(getenv("DISPLAY"));
   X11CHECK(disp!=NULL, "XOpenDisplay");

   // Set the defaults
   xscr= DefaultScreen(disp);
   #if( USE_X11ERROR )
     XSetErrorHandler(x11error);
   #endif
   #if( USE_X11FATAL )
     XSetIOErrorHandler(x11fatal);
   #endif

   // Create the window
   #if( USE_WINDOW )
     int atFlags= CWBackPixel;        // Background pixel
     XSetWindowAttributes atValue;
     atValue.background_pixel= 0x00ffffff; // (White background)
     xwin= XCreateWindow(             // Create the window
               disp,                  // On this display
               DefaultRootWindow(disp), // With this parent
               0,                     // (Use default x offset)
               0,                     // (Use default y offset)
               xwinWidth,             // Width
               xwinHeight,            // Height
               0,                     // Border width
               24,                    // Depth
               InputOutput,           // Class
               CopyFromParent,        // Visual
               atFlags,               // Attribute flags
               &atValue);             // Attribute values
     X11DEBUG(xwin, "XCreateWindow");
   #else
     xwin= DefaultRootWindow(disp);   // Use default root window
     X11DEBUG(xwin, "DefaultRootWindow");
   #endif

   // Select input events
   #if( USE_WINDOW )
     int mask= ExposureMask | ButtonPressMask | StructureNotifyMask
             | KeyPressMask | KeyReleaseMask
             ;
     XSelectInput(disp, xwin, mask);
   #endif

   // Pick a font
   if( 0 )
   {
     char** list= XListFonts(disp, "*", 4096, &rc);
     X11DEBUG(rc, "XListFonts");
     debugf("%d Available fonts (at least)\n", rc);
     for(int i= 0; i < rc; i++)
       tracef("[%4d] '%s'\n", i, list[i]);

     rc= XFreeFontNames(list);
     X11DEBUG(rc, "XFreeFontNames");
   }

   font= XLoadQueryFont(disp, "9x15");
// font= XLoadQueryFont(disp, "variable");
// font= XLoadQueryFont(disp, "*-helvetica-medium-r-*--*-180-*");
// font= XLoadQueryFont(disp, "*-new century schoolbook-medium-r-*--*-180-*-iso8859-9");
   X11CHECK(font!=NULL, "XLoadQueryFont");

   fontWidth= font->max_bounds.width;
   fontHeight= font->ascent + font->descent;
   tracef("Font: width(%u) ascent(%u) descent(%u)\n",
          font->max_bounds.width, font->ascent, font->descent);

   // Create a graphics context for the window
   wgco= XCreateGC(disp, xwin, 0, NULL);
   X11CHECK(wgco!=NULL, "XCreateGC");

   rc= XSetFont(disp, wgco, font->fid);
   X11DEBUG(rc, "XSetFont");
   rc= XSetForeground(disp, wgco, 0x00000000); // Black
   X11DEBUG(rc, "XSetForeground");
   rc= XSetBackground(disp, wgco, 0x00ffffff); // White
   X11DEBUG(rc, "XSetBackground");

   // Create the Pixmap
   xmap= XCreatePixmap(disp, xwin, xwinWidth, xwinHeight, 24);
   X11DEBUG(xmap, "XCreatePixmap");

   // Create a graphics context for the Pixmap
   mgco= XCreateGC(disp, xmap, 0, NULL);
   X11CHECK(mgco!=NULL, "XCreateGC");

   // Set the context attributes
   rc= XSetFont(disp, mgco, font->fid);
   X11DEBUG(rc, "XSetFont");
   rc= XSetForeground(disp, mgco, 0x00000000); // Black
   X11DEBUG(rc, "XSetForeground");
   rc= XSetBackground(disp, mgco, 0x00ffffff); // White
   X11DEBUG(rc, "XSetBackground");

   // Show the window
   #if( USE_WINDOW )
     rc= XMapWindow(disp, xwin);
     X11DEBUG(rc, "XMapWindow");
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       termWindow
//
// Purpose-
//       Do everything needed to clean up and disconnect from the X server.
//
//----------------------------------------------------------------------------
static void
   termWindow( void )               // Close the Window
{
   int                 rc;

   rc= XFreeGC(disp, wgco);
   X11DEBUG(rc, "XFreeGC");

   rc= XFreeGC(disp, mgco);
   X11DEBUG(rc, "XFreeGC");

   rc= XFreePixmap(disp, xmap);
   X11DEBUG(xmap, "XFreePixmap");

   rc= XFreeFont(disp, font);
   X11DEBUG(rc, "XFreeFont");

   rc= XCloseDisplay(disp);
   X11DEBUG(rc, "XCloseDisplay");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       repaint
//
// Purpose-
//       This redraws the window whenever it is uncovered.
//
//----------------------------------------------------------------------------
static void
   repaint(                         // Redraw the window
     const char*       text)        // Using this text
{
   const unsigned      length= strlen(text);

   int                 rc;

   // Draw in Window
   #if( USE_WINDOW )
     rc= XSetBackground(disp, wgco, 0x00ffffff);
     X11DEBUG(rc, "XSetForeground");
     rc= XSetForeground(disp, wgco, 0x00000000);
     X11DEBUG(rc, "XSetForeground");
     rc= XSetFont(disp, wgco, font->fid);
     X11DEBUG(rc, "XSetFont");

     rc= XDrawImageString(disp, xwin, wgco, 30, 30, text, length);
     X11DEBUG(rc, "XDrawImageString");
   #endif

   // Set the Pixmap context attributes
   if( 0 )
   {
     rc= XSetBackground(disp, mgco, 0x00ffffff);
     X11DEBUG(rc, "XSetForeground");
     rc= XSetForeground(disp, mgco, 0x00000000);
     X11DEBUG(rc, "XSetForeground");
     rc= XSetFont(disp, mgco, font->fid);
     X11DEBUG(rc, "XSetFont");
   }

   int direction, ascent, descent;
   XCharStruct xcs;
   rc= XQueryTextExtents(disp, font->fid, text, length,
                         &direction, &ascent, &descent, &xcs);
   X11DEBUG(rc, "XQueryTextExtents");
   tracef("..direction(%d) ascent(%d) descent(%d)\n", direction, ascent, descent);
   tracef("..lbearing(%d) rbearing(%d) width(%d) ascent(%d) descent(%d)"
          " attributes(%x)\n",
          xcs.lbearing, xcs.rbearing, xcs.width, xcs.ascent, xcs.descent,
          xcs.attributes);

// rc= XDrawImageString(disp, xmap, mgco, xcs.lbearing, ascent, text, length);
   rc= XDrawImageString(disp, xmap, mgco, 0, ascent, text, length);
   X11DEBUG(rc, "XDrawImageString");

   if( 0 )
   {
     unsigned maxY= xwinHeight;
     unsigned maxX= xwinWidth;
     if( ximg == NULL )
     {
       ximg= XGetImage(disp, xmap, 0, 0, maxX, maxY, 0x00ffffff, ZPixmap);
       // X11DEBUG(ximg, "XGetImage");
       debugf("Data(%p)\n", ximg->data);
     }

     maxY= ascent + descent;
     maxX= xcs.lbearing + xcs.rbearing;

     debugf("%4d BufferDump HCDM..\n", __LINE__);
     for(unsigned y= 0; y < maxY; y++)
     {
       tracef("|");
       for(unsigned x= 0; x < maxX; x++)
       {
         long cc= XGetPixel(ximg, x, y);
         tracef("%s", cc == 0 ? "*" : " ");
       }
       tracef("|\n");
     }
     debugf("%4d ..BufferDump HCDM\n", __LINE__);
     Debug::get()->flush();

     if( 1 )
     {
       rc= XDestroyImage(ximg);
       X11DEBUG(rc, "XDestroyImage");
       ximg= NULL;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       eventLoop
//
// Purpose-
//       Process events (forever)
//
//----------------------------------------------------------------------------
static inline void
   eventLoop( void )                // Process events
{
   Interval interval;
   for(;;)
   {
     int rc= XPending(disp);
     X11DEBUG(rc, "XPending");
     if( rc == 0 )
     {
       double delay= interval.stop();
       if( delay < 0.1 )
         delay= 0.1;
       else if( delay > 2.0 )
         delay= 2.0;

       Thread::sleep(delay);
       continue;
     }

     interval.start();              // Reset the delay interval
     XEvent event;
     rc= XNextEvent(disp, &event);
     X11DEBUG(rc, "XNextEvent");

     char buffer[64];
     const char* string= NULL;
     KeySym sym;
     unsigned code, key, width, height;
     switch(event.type)
     {
       case Expose:
         if( event.xexpose.count != 0 ) // If more in queue, wait
           break;

         repaint(message);
         break;

       case ConfigureNotify:
         width= event.xconfigure.width;
         height= event.xconfigure.height;
         if( (width != xwinWidth) || (height != xwinHeight) )
         {
           xwinWidth= width;
           xwinHeight= height;
         }
         break;

     case KeyPress:
     case KeyRelease:
       string= "KeyPress";
       if( event.type == KeyRelease )
         string= "KeyRelease";
       tracef("Event(%s)\n"
              "..time(%ld)\n"
              "..x(%d) y(%d)\n"
              "..x_root(%d) y_root(%d)\n"
              "..state(%d) keycode(%d) same_screen(%d)\n"
              , string
              , long(event.xkey.time)
              , event.xkey.x, event.xkey.y
              , event.xkey.x_root, event.xkey.y_root
              , event.xkey.state, event.xkey.keycode, event.xkey.same_screen
              );

       #if 1
         memset(buffer, 0, sizeof(buffer)); // (No length returned)
         key= XLookupString((XKeyEvent*)&event, buffer, sizeof(buffer),
                            &sym, NULL);
         X11DEBUG(key, "XLookupString");
         tracef("buffer(%s)\n", buffer);

         code= unsigned(event.xkey.keycode);
         key= unsigned(sym);
         sprintf(buffer, "Code(0x%.4x,%3d) Key(0x%.6x,%c) %10s %u %d",
                         code, code, key, key, string,
                         unsigned(event.xkey.time), shift != 0);
         tracef("%s\n", buffer);
       #else
         code= unsigned(event.xkey.keycode);
         key= XLookupKeysym((XKeyEvent*)&event, shift != 0);
         X11DEBUG(key, "XLookupKeysym");
         sprintf(buffer, "Code(0x%.4x,%3d) Key(0x%.6x,%c) %10s %u %d",
                         code, code, key, key, string,
                         unsigned(event.xkey.time), shift != 0);
         tracef("%s\n", buffer);
         if( event.type == KeyPress )
         {
           if( key == XK_Shift_L )
             shift |= 0x00000008;
           else if( key == XK_Shift_R )
             shift |= 0x00000004;
           else if( key == XK_Caps_Lock )
             shift |= 0x00000002;
           else if( key == XK_Shift_Lock )
             shift |= 0x00000001;
         }
         else
         {
           if( key == XK_Shift_L )
             shift &= 0xfffffff7;
           else if( key == XK_Shift_R )
             shift &= 0xfffffffb;
           else if( key == XK_Caps_Lock )
             shift &= 0xfffffffd;
           else if( key == XK_Shift_Lock )
             shift &= 0xfffffffe;
         }
       #endif
       repaint(buffer);
       break;

       case ButtonPress:
         break;

       default:
         break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char**            argv)        // Argument array
{
   debugSetIntensiveMode();
   debugf("TextWindow started\n");

   try {
     initWindow(argc, argv);
     #if( USE_WINDOW )
       eventLoop();
     #else
       repaint(message);
       repaint("abcdefghijklmnopqrstuvwxyz");
       repaint("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
       repaint("0123456789 )!@#$%^&*(");
       repaint("<>?,./:\";'{}|[]\\-=_+");
       repaint("a\b:-\t-\r-\n-");
       repaint("alles in ordnung");
     #endif
   } catch(const char* X) {
     debugf("Exception: const char*(%s)\n", X);
     return 1;
   } catch(...) {
     debugf("Exception: ...\n");
     return 1;
   }

   // Close the X server connection
   termWindow();
   return 0;
}

