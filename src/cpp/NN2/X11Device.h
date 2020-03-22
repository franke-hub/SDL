//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       X11Device.h
//
// Purpose-
//       X11 / ImageMagick device.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef X11DEVICE_H_INCLUDED
#define X11DEVICE_H_INCLUDED

#include <list>
#include <string>

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <Magick++.h>

//----------------------------------------------------------------------------
// Attributes (compiler dependent, no effect if not supported)
//----------------------------------------------------------------------------
#ifdef __GNUC__
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm) \
               __attribute__ ((format (printf, fmt_parm, arg_parm)))

   #define _ATTRIBUTE_NORETURN __attribute__ ((noreturn))
#else
   #define _ATTRIBUTE_PRINTF(fmt_parm, arg_parm)
   #define _ATTRIBUTE_NORETURN
#endif

//----------------------------------------------------------------------------
//
// Class-
//       X11DeviceAttr
//
// Purpose-
//       X11 / ImageMagick device attributes
//
//----------------------------------------------------------------------------
class X11DeviceAttr {               // X11 / ImageMagick device attributes
//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
public:                             // PUBLIC access
enum
{  DEBUG= false                     // Debugging active?
,  MAX_LENGTH= 16777216             // Maximum Window size
,  MIN_LENGTH= 32                   // Minimum Window size
};

//----------------------------------------------------------------------------
// Attributes
//----------------------------------------------------------------------------
public:                             // PUBLIC access
// X11 objects
Display*               disp;        // The X11 display
GC                     xgcx;        // The X11 graphics context
int                    xscr;        // The X11 screen

Window                 xwin;        // The X11 window
XEvent                 xevt;        // The X11 Event
XImage*                ximg;        // The X11 pixel image
Visual*                xvis;        // The X11 Visual

unsigned               x_length;    // The X (row) length
unsigned               y_length;    // The Y (column) length

//----------------------------------------------------------------------------
// Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~X11DeviceAttr( void )           // Destructor
{
   // Close the display
// debugf("~X11DeviceAttr..\n");
// debugf("xvis(%p)\n", xvis);
// debugf("ximg(%p)\n", ximg);
   if( ximg )
     XDestroyImage(ximg);
// debugf("xgcx(%p)\n", &xgcx);
   XFreeGC(disp, xgcx);
// debugf("xwin(%p)\n", &xwin);
   XDestroyWindow(disp, xwin);
// debugf("disp(%p)\n", disp);
   if( disp )
     XCloseDisplay(disp);
// debugf("..~X11DeviceAttr\n");
}

   X11DeviceAttr( void )            // Constructor
:  disp(nullptr), ximg(nullptr), xvis(nullptr)
,  xgcx(), xscr(0), xwin(), xevt()
{
// debugf("X11DeviceAttr\n");

   // Open the display
   disp= XOpenDisplay(getenv("DISPLAY")); // Open the default display
   x11check(disp != nullptr, "XOpenDisplay");
   xscr= DefaultScreen(disp);       // Get the default display
   x11debug(xscr, "DefaultScreen");

   // Locate an acceptable Visual
   int count= VisualDepthMask   | VisualBitsPerRGBMask
            | VisualRedMaskMask | VisualGreenMaskMask | VisualBlueMaskMask;
   XVisualInfo xinp;
   xinp.depth= 24;
   xinp.red_mask=   0x00ff0000;
   xinp.green_mask= 0x0000ff00;
   xinp.blue_mask=  0x000000ff;
   xinp.bits_per_rgb= 8;
   XVisualInfo* xout= XGetVisualInfo(disp, count, &xinp, &count);
   assert( count != 0 );            // ERROR: Cannot create XVisual
   xvis= xout[0].visual;
   XFree(xout);
}

//----------------------------------------------------------------------------
// Internal methods
//----------------------------------------------------------------------------
public:
static inline void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(1, 2)
{
   if( DEBUG )
   {
     va_list argptr;                // Argument list pointer

     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stderr, fmt, argptr);
     va_end(argptr);                // Close va_ functions
   }
}

static inline unsigned              // Resultant
   min_max(                         // Minimum/Maximum length
     unsigned          inp)         // The source
{
   if( inp < MIN_LENGTH )
     inp= MIN_LENGTH;
   else if( inp > MAX_LENGTH )
     inp= MAX_LENGTH;
   return inp;
}

static inline unsigned              // Resultant
   toRange(                         // Convert value to QuantumRange value
     unsigned          inp)         // The source value, range 0..255
{
   using namespace Magick;
   assert( inp < 256 );
   if( QuantumRange == 65535 )
     return inp << 8;

   return inp;
}

static inline unsigned              // Resultant
   toRGB(                           // Convert Color to RGB pixel
     Magick::Color&    color)       // The source Color
{
   using namespace Magick;
   unsigned R= color.redQuantum();
   unsigned G= color.greenQuantum();
   unsigned B= color.blueQuantum();

   if( QuantumRange == 65535 )
   {
     R >>= 8;
     G >>= 8;
     B >>= 8;
   }
// debugf("%x,%x,%x\n", R, G, B);

   assert( R < 256 );
   assert( G < 256 );
   assert( B < 256 );
   return R << 16 | G << 8 | B;
}

static inline void
   x11check(                        // Assert X11 result
     int               cc,          // Condition code
     const char*       name)        // Function name
{
   if( cc )
     return;

   fprintf(stderr, "FAILED: %s()\n", name);
   exit(EXIT_FAILURE);
}

static inline void
   x11debug(                        // Debug X11 result
     int               rc,          // Return code
     const char*       name)        // Function name
{
   debugf("%d= %s()\n", rc, name);
}
}; // X11DeviceAttr

//----------------------------------------------------------------------------
//
// Class-
//       X11Device
//
// Purpose-
//       X11 / ImageMagick device.
//
//----------------------------------------------------------------------------
class X11Device : public X11DeviceAttr { // X11 / ImageMagick device
//----------------------------------------------------------------------------
// Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~X11Device( void )               // Destructor
{
// debugf("~X11Device\n");
}

   X11Device( void )                // Default constructor
:  X11DeviceAttr()
{
// debugf("X11Device()\n");
   config(MIN_LENGTH, MIN_LENGTH);
}

   X11Device(                       // Default constructor
     unsigned          x_length,    // X (row) length
     unsigned          y_length)    // Y (column) length
:  X11DeviceAttr()
{
// debugf("X11Device(%d,%d)\n", x_length, y_length);
   config(x_length, y_length);
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::config
//
// Purpose-
//       Configure the device
//
//----------------------------------------------------------------------------
inline void
   config(                          // Configure the device
     unsigned          x_length,    // X (row) length
     unsigned          y_length)    // Y (column) length
{
   int                 rc;          // Generic return code

   x_length= min_max(x_length);
   y_length= min_max(y_length);
   this->x_length= x_length;
   this->y_length= y_length;

   xwin= XCreateSimpleWindow(       // Create a window
             disp, DefaultRootWindow(disp), // Display, parent window
             0, 0,                  // XY Offset
             x_length, y_length,    // XY Length
             0, BlackPixel(disp, xscr), // Border: size, color
             WhitePixel(disp, xscr)); // Background
   x11debug(xwin, "XCreateSimpleWindow");
   rc= XSetStandardProperties(disp, xwin, "X11Device", "X11",
       None,                      // Icon pixmap
       nullptr, 0,                // Arguments, count
       nullptr);                  // Hints
   x11debug(rc, "XSetStandardProperties");

   // Create the Graphics Context
   xgcx= XCreateGC(disp, xwin, 0, nullptr); // Create graphics context
   x11check(xgcx != nullptr, "XCreateGC");
   rc= XSetBackground(disp, xgcx, WhitePixel(disp, xscr));
   x11debug(rc, "XSetBackground");
   rc= XSetForeground(disp, xgcx, BlackPixel(disp, xscr));
   x11debug(rc, "XSetForeground");

   // Create the Image
   unsigned size= x_length * y_length * sizeof(uint32_t);
   char* data= (char*)malloc(size);
   memset(data, 0, size);
   x11check(data != nullptr, "malloc");
   ximg= XCreateImage(disp, xvis, 24, ZPixmap, 0, data,
                      x_length, y_length, 32, 0);
   x11check(ximg != nullptr, "XCreateImage");

   // Map the Window
   rc= XMapWindow(disp, xwin);
   x11debug(rc, "XMapWindow");

   // Select inputs
   rc= 0                            // Input selection mask
     | ButtonPressMask | ButtonReleaseMask // ButtonPress/Release
     | StructureNotifyMask                 // ConfigureNotify
     | ExposureMask                        // Expose
     ;
   XSelectInput(disp, xwin, rc);
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::expose
//
// Purpose-
//       Display the updated image
//
//----------------------------------------------------------------------------
inline void
   expose( void )                   // Display the updated image
{
   int rc= XPutImage(disp, xwin, xgcx, ximg,
                     0, 0,          // Offset from image
                     0, 0, x_length, y_length); // Bounding box
   x11debug(rc, "XPutImage");
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::flush
//
// Purpose-
//       Wait for all pending XEvents
//
//----------------------------------------------------------------------------
inline void
   flush( void )                    // Wait for all pending XEvents
{
   while( true )
   {
     int rc= XPending(disp);
     x11debug(rc, "XPending");
     if( rc == 0 )
       break;

     XNextEvent(disp, &xevt);
     x11debug(xevt.type, "XNextEvent");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::from/intoMagickImage
//
// Purpose-
//       Convert from or into Magick::Image
//
//----------------------------------------------------------------------------
inline void
   fromMagickImage(                 // Convert from MagickImage
     const Magick::Image&
                       image)       // INPUT: Magick::Image
{
   using namespace Magick;

   resize(image.rows(), image.columns());

   Pixels cache((Image&)image);     // We only read the image
   PixelPacket* packet= cache.get(0, 0, x_length, y_length);
   for(int y= 0; y<y_length; y++)
   {
     for(int x= 0; x<x_length; x++)
     {
       Color color(*packet);
       uint32_t pixel= toRGB(color);
       XPutPixel(ximg, x, y, pixel);

       packet++;
     }
   }

   // Note: Caller invokes expose(), if required
}

inline void
   intoMagickImage(                 // Convert into MagickImage
     Magick::Image&    image) const // OUTPUT: Magick::Image
{
   using namespace Magick;
   Geometry geometry(x_length, y_length);
   image.resize(geometry);

   for(int y= 0; y<y_length; y++)
   {
     for(int x= 0; x<x_length; x++)
     {
       uint32_t pixel= XGetPixel(ximg, x, y);
       unsigned R= (pixel >> 16) & 0x000000ff;
       unsigned G= (pixel >>  8) & 0x000000ff;
       unsigned B= (pixel >>  0) & 0x000000ff;

       Color color(toRange(R), toRange(G), toRange(B), 0);
       image.pixelColor(x, y, color);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::pixel
//
// Purpose-
//       Get/set the pixel at X, Y
//
//----------------------------------------------------------------------------
inline uint32_t                     // The pixel
   pixel(                           // Get the pixel
     unsigned          x,           // X (row) offet
     unsigned          y)           // Y (column) offet
{
   return XGetPixel(ximg, x, y);
}

inline void
   pixel(                           // Set the pixel
     unsigned          x,           // X (row) offet
     unsigned          y,           // Y (column) offet
     uint32_t          pixel)       // The pixel to set
{
   XPutPixel(ximg, x, y, pixel);
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::resize
//
// Purpose-
//       Resize the window, destroying its content.
//
// Implementation note-
//       ** NOT TESTED **
//
//----------------------------------------------------------------------------
inline void
   resize(                          // Resize the Window
     unsigned          x_length,    // X (row) length
     unsigned          y_length)    // Y (column) length
{
   int                 rc;          // Generic return code

// debugf("resize(%d,%d)<=(%d,%d)\n", x_length, y_length,
//        this->x_length, this->y_length);

   if( x_length != this->x_length || y_length != this->y_length )
   {
     // Remove the old XImage
     rc= XDestroyWindow(disp, xwin);
     x11debug(rc, "XDestroyWindow");

     rc= XFreeGC(disp, xgcx);
     x11debug(rc, "XFreeGC");

     rc= XDestroyImage(ximg);
     x11debug(rc, "XDestroyImage");

     // Reconfigure
     config(x_length, y_length);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::title
//
// Purpose-
//       Set window title
//
//----------------------------------------------------------------------------
inline void
   title(                           // Set the window title
     const char*       name)        // To this name
{
   int rc= XStoreName(disp, xwin, name);
   x11debug(rc, "XStoreName");
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Device::zoom
//
// Purpose-
//       Resize the window, using ImageMagick to interpolate content
//
// Implementation note-
//       ** NOT TESTED **
//
//----------------------------------------------------------------------------
inline void
   zoom(                            // Resize the Window, keeping content
     unsigned          x_length,    // X (row) length
     unsigned          y_length)    // Y (column) length
{
// debugf("zoom(%d,%d)<=(%d,%d)\n", x_length, y_length,
//        this->x_length, this->y_length);

   if( x_length != this->x_length || y_length != this->y_length )
   {
     using namespace Magick;

     // Save the current image
     Geometry iGeom(this->x_length, this->y_length);
     Image    image(iGeom, "white"); // The display image
     intoMagickImage(image);

     // Resize, zoom
     resize(x_length, y_length);
     Geometry oGeom(this->x_length, this->y_length);
     image.zoom(oGeom);

     // Load the resized Magick::Image
     fromMagickImage(image);
     expose();
   }
}
}; // class X11Device
#endif // X11DEVICE_H_INCLUDED
