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
//       OS/BSD/X11Font.cpp
//
// Purpose-
//       Graphical User Interface: _SystemFont, X11Font implementations
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
// Included only from Font.cpp
#include <com/Barrier.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

using GUI::Buffer;
using GUI::Color_t;
using GUI::Justification;
using GUI::Pixel;
using GUI::Text;
using GUI::UniCode;
using GUI::UTF8_t;
using GUI::UTF16_t;
using GUI::XYLength;
using GUI::XYOffset;
using GUI::XOffset_t;
using GUI::YOffset_t;
using GUI::_SystemFont;

#define X11Font GUI ## _X11Font
#include "X11Font.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

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
// Internal data areas (protected by Barrier)
//----------------------------------------------------------------------------
Barrier                X11Font::barrier= BARRIER_INIT; // Static Barrier
unsigned               X11Font::count= 0;    // Reference count

Display*               X11Font::disp= NULL;  // Display handle
int                    X11Font::xscr= 0;     // Display screen
GC                     X11Font::xgco= 0;     // Graphic context
Pixmap                 X11Font::xmap= 0;     // Pixmap handle
XYLength               X11Font::xmapLength= {0,0}; // Pixmap length

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
   Logger::log("%4d CHECKSTOP(%s)\n", line, name);
   Logger::get()->flush();
   throw "X11Font::checkstop()";
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
       checkstop(line, "X11Font::x11check");
   #else
     if( cc == 0 )
     {
       x11debug(line, cc, name);
       checkstop(line, "X11Font::x11check");
     }
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::isValidDescriptor
//
// Purpose-
//       Test descriptor validitiy.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   _SystemFont::isValidDescriptor(  // Test Font descriptor validity
     const char*       desc)        // For this descriptor (wildcards OK)
{
   return X11Font::isValidDescriptor(desc);
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::getValidDescriptor
//
// Purpose-
//       Get valid descriptor.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   _SystemFont::getValidDescriptor( // Get first valid Font descriptor
     const char*       desc,        // For this descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result)      // OUTPUT: Result string
{
   return X11Font::getValidDescriptor(desc, length, result);
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::make
//
// Purpose-
//       Return a System Dependent Implementation
//
//----------------------------------------------------------------------------
_SystemFont*                        // -> _SystemFont
   _SystemFont::make(               // Generate _SystemFont
     const char*       desc)        // For this Font descriptor
{
   return X11Font::make(desc);
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::~X11Font
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   X11Font::~X11Font( void )        // Destructor
{
   int                 rc;

   #ifdef HCDM
     Logger::log("%4d: X11Font(%p)::~X11Font() %s\n", __LINE__,
                 this, desc);
   #endif

   if( desc != NULL )
   {
     free(desc);
     desc= NULL;
   }

   if( font != NULL )
   {
     rc= XFreeFont(disp, font);
     X11DEBUG(rc, "XFreeFont");
     font= NULL;
   }

   AutoBarrier lock(barrier);
   count--;
   if( count == 0 )
     term();
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::X11Font
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   X11Font::X11Font(                // Constructor
     char*             name,        // The font name (strdup in make)
     XFontStruct*      font)        // Associated Font
:  _SystemFont(), font(font)
{
   #ifdef HCDM
     Logger::log("%4d: X11Font(%p)::X11Font(%p)\n", __LINE__,
                 this, font);
   #endif

   desc= name;
   if( font != NULL )
   {
     length.x= font->max_bounds.width;
     length.y= font->ascent + font->descent;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::make
//
// Purpose-
//       Return a System Dependent Implementation
//
//----------------------------------------------------------------------------
_SystemFont*                        // -> _SystemFont
   X11Font::make(                   // Generate _SystemFont
     const char*       desc)        // For this Font descriptor
{
   char                buffer[256]; // Descriptor buffer

   #if defined(HCDM) && FALSE
     debugSetIntensiveMode();
   #endif

   // Handle default font name
   if( desc == NULL )
     desc= "9x15";

   // Initialize
   AutoBarrier lock(barrier);       // Remainder under lock protection
   if( count == 0 )
   {
     xmapLength.x= 1024;
     xmapLength.y= 64;
     init(xmapLength);
   }

   // Get the actual font name
   int items;
   char** list= XListFonts(disp, desc, 1, &items);
   X11DEBUG(items, "XListFonts");
   if( list != NULL )
   {
     if( strlen(list[0]) < sizeof(buffer) )
     {
       strcpy(buffer, list[0]);
       desc= buffer;
     }

     XFreeFontNames(list);
   }

   // Duplicate the name
   char* name= strdup(desc);
   X11CHECK(name!=NULL, "strdup");

   // Load the Font
   XFontStruct* font= XLoadQueryFont(disp, name);
   if( font == NULL && count == 0 )
   {
     term();
     fprintf(stderr, "%4d: X11Font::make() invalidFont(%s)\n", __LINE__, name);
   }
   X11CHECK(font!=NULL, "XLoadQueryFont");

   count++;
   X11Font* result= new X11Font(name, font); // (Convienient gdb break point)
   return result;                   // (Exposes X11Font* in gdb)
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::isValidDescriptor
//
// Purpose-
//       Test descriptor validitiy.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   X11Font::isValidDescriptor(      // Test Font descriptor validity
     const char*       desc)        // For this descriptor (wildcards OK)
{
   const char*         result= "NoSuchFont"; // Default resultant

   #ifdef HCDM
     Logger::log("%4d: X11Font(*)::isValidDescriptor(%s)\n", __LINE__,
                 desc);
   #endif

   AutoBarrier lock(barrier);
   if( count == 0 )
   {
     xmapLength.x= 1024;
     xmapLength.y= 64;
     init(xmapLength);
   }

   int items;
   char** list= XListFonts(disp, desc, 1, &items);
   X11DEBUG(items, "XListFonts");
   if( list != NULL )
   {
     result= NULL;
     XFreeFontNames(list);
   }

   if( count == 0 )
     term();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::getValidDescriptor
//
// Purpose-
//       Get valid descriptor.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   X11Font::getValidDescriptor(     // Get first valid Font descriptor
     const char*       desc,        // For this descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             output)      // OUTPUT: Result string
{
   const char*         result= "NoSuchFont"; // Default resultant

   #ifdef HCDM
     Logger::log("%4d: X11Font(*)::getValidDescriptor(%s)\n", __LINE__,
                 desc);
   #endif

   AutoBarrier lock(barrier);
   if( count == 0 )
   {
     xmapLength.x= 1024;
     xmapLength.y= 64;
     init(xmapLength);
   }

   int items;
   char** list= XListFonts(disp, desc, 128, &items);
   X11DEBUG(items, "XListFonts");
   if( list != NULL )
   {
     result= NULL;
     if( strlen(list[0]) >= length )
       result= "InvalidLength";
     else
       strcpy(output, list[0]);

     #if defined(HCDM) && TRUE
       for(int i= 0; i<items; i++)
         tracef("[%2d] %s\n", i, list[i]);
     #endif

     XFreeFontNames(list);
   }

   if( count == 0 )
     term();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::init
//
// Purpose-
//       Initialize static data
//
// Usage notes-
//       The mutex must be held when calling this method.
//
//----------------------------------------------------------------------------
void
   X11Font::init(                   // Initialize static data
     const XYLength&   length)      // For this size Pixmap
{
   // Open the display
   disp= XOpenDisplay(getenv("DISPLAY"));
   X11CHECK(disp!=NULL, "XOpenDisplay");
   xscr= DefaultScreen(disp);

   // Create the Pixmap
   Window root= DefaultRootWindow(disp);
   X11DEBUG(root, "DefaultRootWindow");
   xmap= XCreatePixmap(disp, root, length.x, length.y, 24);
   X11DEBUG(xmap, "XCreatePixmap");


   // Create the Context
   int gcFlags= 0;
   XGCValues gcValues;
   xgco= XCreateGC(disp, xmap, gcFlags, &gcValues);
   X11CHECK(xgco!=NULL, "XCreateGC");

   // Set foreground and background colors
   int rc= XSetForeground(disp, xgco, 0x00ffffff); // (Non-zero)
   X11DEBUG(rc, "XSetForeground");
   rc= XSetBackground(disp, xgco, 0x00000000);
   X11DEBUG(rc, "XSetBackground");
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::term
//
// Purpose-
//       Terminate static data, releasing static resources
//
// Usage notes-
//       The mutex must be held when calling this method.
//
//----------------------------------------------------------------------------
void
   X11Font::term( void )            // Terminate static data
{
   int                 rc;

   if( disp != NULL )
   {
     rc= XFreeGC(disp, xgco);
     X11DEBUG(rc, "XFreeGC");
     rc= XFreePixmap(disp, xmap);
     X11DEBUG(rc, "XFreePixmap");
     rc= XCloseDisplay(disp);
     X11DEBUG(rc, "XCloseDisplay");
     disp= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::reconfig
//
// Purpose-
//       Configure the internal Pixmap
//
// Usage notes-
//       The mutex must be held when calling this method.
//
//----------------------------------------------------------------------------
void
   X11Font::reconfig(               // Reconfigure the internal Pixmap
     const XYLength&   length)
{
   int                 rc;

   if( length.x > xmapLength.x || length.y > xmapLength.y )
   {
     if( length.x > xmapLength.x )  // We never shrink the Pixmap extent
       xmapLength.x= length.x;
     if( length.y > xmapLength.y )
       xmapLength.y= length.y;

     if( font != NULL )
     {
       rc= XFreeFont(disp, font);
       X11DEBUG(rc, "XFreeFont");
       font= NULL;
     }

     term();
     init(xmapLength);

     // Reload the Font
     font= XLoadQueryFont(disp, desc);
     X11CHECK(font!=NULL, "XLoadQueryFont");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::extent
//
// Purpose-
//       Determine text extent.
//
//----------------------------------------------------------------------------
void
   X11Font::extent(                 // Determine text extent for this Font and
     const std::string&text,        // This text
     XYLength&         length)      // (OUTPUT) Pixel length
{
   int                 rc;

   #ifdef HCDM
     Logger::log("%4d: X11Font(%p)::extent(%s)\n", __LINE__, this,
                 text.c_str());
   #endif

   length.x= length.y= 0;
   AutoBarrier lock(barrier);       // Protect static attributes
   if( font != NULL )
   {
     rc= XSetFont(disp, xgco, font->fid);
     X11DEBUG(rc, "XSetFont");

     int direction, ascent, descent;
     XCharStruct xcs;
     rc= XTextExtents(font, text.c_str(), text.length(),
                      &direction, &ascent, &descent, &xcs);
     X11DEBUG(rc, "XTextExtents");

     length.x= xcs.lbearing + xcs.rbearing;
     length.y= ascent + descent;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       X11Font::render
//
// Purpose-
//       Render text using this Font
//
//----------------------------------------------------------------------------
void
   X11Font::render(                 // Render text using this Font into
     Buffer*           buffer,      // This Buffer at
     const XYOffset&   inpoff,      // This pixel offset for
     const XYLength&   inplen,      // This pixel length using
     const std::string&text,        // This text and
     int               inpmode)     // This Justification mode
{
   XYOffset            offset;      // Working offset
   XYLength            length;      // Working length
   int                 rc;

   #ifdef HCDM
     Logger::log("%4d: X11Font(%p)::render(%p=%s,O{%u,%u},L{%u,%u},%s,%x)\n",
                 __LINE__, this, buffer, buffer->getName(),
                 inpoff.x, inpoff.y, inplen.x, inplen.y, text.c_str(), inpmode);
   #endif

   length= inplen;
   if( this->length.y > length.y )
     length.y= this->length.y;
   AutoBarrier lock(barrier);       // Protect static attributes
   reconfig(length);

   if( buffer != NULL && font != NULL
       && inplen.x > 0 && inplen.y > 0 )
   {
     rc= XSetFont(disp, xgco, font->fid);
     X11DEBUG(rc, "XSetFont");

     int direction, ascent, descent;
     XCharStruct xcs;
     rc= XTextExtents(font, text.c_str(), text.length(),
                      &direction, &ascent, &descent, &xcs);
     X11DEBUG(rc, "XTextExtents");

     int rc= XDrawImageString(disp, xmap, xgco, 0, ascent,
                              text.c_str(), text.length());
     X11DEBUG(rc, "XDrawImageString");

     unsigned minX= 0;
     unsigned minY= 0;
     unsigned maxX= xcs.lbearing + xcs.rbearing;
     unsigned maxY= ascent + descent;

     // Justify the text
     offset= inpoff;
     length= inplen;
     if( inpmode != 0 )
     {
       unsigned mode= inpmode & Justification::LR_MASK;
       switch( mode )
       {
         case Justification::LR_TEXT:
           if( length.x < maxX )
             minX= maxX - length.x;
           break;

         case Justification::LR_RIGHT:
           if( length.x >= maxX )
             offset.x += (length.x - maxX);
           else
             minX= maxX - length.x;
           break;

         case Justification::LR_CENTER:
           if( length.x >= maxX )
             offset.x += (length.x - maxX) / 2;
           else
             minX= (maxX - length.x) / 2;
           break;

         default:
           break;
       }

       mode= inpmode & Justification::TB_MASK;
       switch( mode )
       {
         case Justification::TB_BOTTOM:
           if( length.y >= maxY )
             offset.y += (length.y - maxY);
           else
             minY= maxY - length.y;
           break;

         case Justification::TB_CENTER:
           if( length.y >= maxY )
             offset.y += (length.y - maxY) / 2;
           else
             minY= (maxY - length.y) / 2;
           break;

         default:
           break;
       }
     }

     if( (maxX - minX) > length.x )
       maxX= length.x + minX;
     if( (maxY - minY) > length.y )
       maxY= length.y + minY;

     // For some reason, the XImage is not persistent.
     // We need to allocate and free an XImage each time we draw a string.
     XImage* ximg= XGetImage(disp, xmap, 0, 0, maxX, maxY, 0x00ffffff, ZPixmap);
     X11DEBUG(ximg!=NULL, "XGetImage");
     for(YOffset_t y= minY; y < maxY; y++)
     {
       Pixel* pixel= buffer->getPixel(offset.x, offset.y + y);
       for(XOffset_t x= minX; x < maxX; x++)
       {
         Color_t cc= XGetPixel(ximg, x, y);
         if( cc != 0 )
           pixel->setColor(color);
         pixel++;
       }
     }

     rc= XDestroyImage(ximg);
     X11DEBUG(rc, "XDestroyImage");
   }
}

