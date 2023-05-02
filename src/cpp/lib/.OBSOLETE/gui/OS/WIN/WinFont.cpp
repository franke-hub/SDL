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
//       OS/WIN/WinFont.cpp
//
// Purpose-
//       Graphical User Interface: _SystemFont, WinFont implementations
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
// Included only from Font.cpp
#pragma comment(lib,"Gdi32.lib")    // Add Windows libraries
#pragma comment(lib,"User32.lib")

#include <windows.h>
#include <gdiplus.h>
#include <com/AutoPointer.h>
#include <gui/Justification.h>
#include <gui/Window.h>

#undef TRANSPARENT                  // Windows::SetBkMode parameter
#include "Device.h"                 // For utilities, e.g. WINDEBUG
#include "WinFont.h"                // Implementation class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef USE_GETFONT
#undef  USE_GETFONT                 // If defined, use getFont subroutine
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

#ifdef HCDM
  #define DEBUG() debug(__LINE__)
#else
  #define DEBUG()
#endif

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Struct-
//       FontEnumerator
//
// Purpose-
//       Font enumeration control object
//
//----------------------------------------------------------------------------
struct FontEnumerator {
   LOGFONT             inp;         // The input LOGFONT

   int                 isValid;     // TRUE if a valid Font is found
   LOGFONT             out;         // The best resultant LOGFONT
}; // struct FontEnumerator

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::fontFamilyEnumerator
//
// Purpose-
//       EnumFontFamilies callback routine
//
//----------------------------------------------------------------------------
#ifdef USE_GETFONT
static int CALLBACK                 // Non-zero to continue enumeration
   fontFamilyEnumerator(            // EnumFontFamilies callback
     CONST LOGFONT*     lf,         // The LOGFONTA enumeration
     CONST TEXTMETRIC*  tm,         // The NEWTEXTMETRIC enumeration
     DWORD              type,       // The Font type
     LPARAM             lParam)     // (Pass-through parameter)
{
   IFHCDM( Logger::log("%4d fontFamilyEnum(%p,%p,0x%.4x,%p)\n", __LINE__,
                       lf, tm, type, (void*)lParam);
   )

#if defined(HCDM) && TRUE           // Extended debugging
   // Extended debugging information
   ENUMLOGFONTEX*  ex= (ENUMLOGFONTEX*)lf; // Extended information
   Logger::log("..%s-%s-%s-%ld\n", ex->elfFullName, ex->elfStyle,
               ex->elfScript, lf->lfHeight);
#endif

   // Verify the LOGFONT
   int isValid= TRUE;               // Default, valid

   FontEnumerator* fe= (FontEnumerator*)lParam;
   if( fe->inp.lfHeight == 0        // If height defaulted
       && lf->lfHeight < 8 )        // and height < 8
     isValid= FALSE;                // Do not accept

   // Check for better match (currently only height)
   if( isValid )                    // If acceptable at all
   {
     if( fe->isValid == FALSE )     // If this is the first valid font
     {
       IFHCDM( Logger::log("..First acceptable\n"); )
       fe->isValid= TRUE;           // We have a winner
       memcpy(&fe->out, lf, sizeof(fe->out)); // Use it as the comparitor
     }

     if( lf->lfHeight >= fe->inp.lfHeight
         && lf->lfHeight < fe->out.lfHeight ) // If better fit
     {
       IFHCDM( Logger::log("..Better fit\n"); )
       memcpy(&fe->out, lf, sizeof(fe->out)); // Replace the winner
     }
   }

   return TRUE;                     // Continue enumerating
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       GUI::getFont
//
// Purpose-
//       Find a suitable Font
//
//----------------------------------------------------------------------------
#ifdef USE_GETFONT
static int                          // Return code, TRUE iff valid
   getFont(                         // Get Font
     LOGFONT*           lf)         // *INP/OUT* Resultant
{
   FontEnumerator       fe;         // Working FontEnumerator
   int                  rc;

   // Initialize the parameter area
   memcpy(&fe.inp, lf, sizeof(fe.inp)); // Copy the input parameter
   fe.isValid= FALSE;               // None found yet

   // Drive the font family enumerator
   SetLastError(0);
   HDC hDC= GetWindowDC(NULL);
   WINDEBUG(hDC, "GetWindowDC");

   rc= EnumFontFamiliesEx(hDC, lf, fontFamilyEnumerator, (LPARAM)&fe, 0);
   WINCHECK(rc, "EnumFontFamiliesEx");

   rc= DeleteDC(hDC);
   WINCHECK(rc, "DeleteDC");

   // Return resultant LOGFONT
   if( fe.isValid )
     memcpy(lf, &fe.out, sizeof(*lf));

   return fe.isValid;
}
#endif

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
   return WinFont::isValidDescriptor(desc);
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
   return WinFont::getValidDescriptor(desc, length, result);
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
   IFHCDM( Logger::log("%4d: SystemFont(*)::make(%s)\n", __LINE__, desc); )
   return WinFont::make(desc);
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::~WinFont
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   WinFont::~WinFont( void )        // Destructor
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinFont(*)::~WinFont(%s)\n", __LINE__); )

   // Delete windows objects
   if( hFont != NULL )
   {
     rc= DeleteObject(hFont);
     WINCHECK(rc, "DeleteObject");
     hFont= NULL;
   }

   if( hDC != NULL )
   {
     rc= DeleteDC(hDC);
     WINCHECK(rc, "DeleteDC");
     hDC= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::WinFont
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   WinFont::WinFont(                // Constructor
     LOGFONT&          lf)          // For this Font descriptor
:  _SystemFont()
,  hFont(NULL)
{
   IFHCDM(
     Logger::log("%4d: WinFont(*)::WinFont(%s)\n", __LINE__, lf.lfFaceName);
   )

   // Save the font descriptor
   lFont= lf;

   // Create the font object
   hFont= CreateFontIndirect(&lf);
   WINCHECK(hFont, "CreateFontIndirect");

   // Create working memory device context
   hDC= CreateCompatibleDC(NULL);   // Create memory device context
   WINCHECK(hDC, "CreateCompatibleDC");
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::isValidDescriptor
//
// Purpose-
//       Test descriptor validitiy.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   WinFont::isValidDescriptor(      // Test Font descriptor validity
     const char*       desc)        // For this descriptor (wildcards OK)
{
   return NULL; // NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::getValidDescriptor
//
// Purpose-
//       Get valid descriptor.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   WinFont::getValidDescriptor(     // Get first valid Font descriptor
     const char*       desc,        // For this descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result)      // OUTPUT: Result string
{
   return NULL; // NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::make
//
// Purpose-
//       Return a System Dependent Implementation
//
//----------------------------------------------------------------------------
_SystemFont*                        // -> _SystemFont
   WinFont::make(                   // Generate _SystemFont
     const char*       desc)        // For this Font descriptor
{
   int                 rc;

   IFHCDM( Logger::log("%4d: WinFont(*)::make(%s)\n", __LINE__, desc); )

   LOGFONT             lf;          // Our LOGFONT
   memset(&lf, 0, sizeof(lf));      // Initialized zeros

   // Minimal initialization
   if( desc == NULL )
     desc= "System";
   else if( strlen(desc) >= LF_FACESIZE )
     desc= "<INVALID>";
   strcpy(lf.lfFaceName, desc);
   lf.lfHeight= 8;                  // Default height (BRINGUP)

   WinFont* result= NULL;

#if defined(USE_GETFONT)
   // Validate the face
   rc= getFont(&lf);
   WINDEBUG(rc, "getFont");

   // Create the WinFont
   if( rc )
     result= new WinFont(lf);
#else
   result= new WinFont(lf);
#endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::extent
//
// Purpose-
//       Determine text extent
//
//----------------------------------------------------------------------------
void
   WinFont::extent(                 // Determine text extent
     const std::string&text,        // For this text
     XYLength&         outlen)      // Returning this pixel length
{
   IFHCDM(
     Logger::log("%4d: WinFont(%p)::extent(%s)\n", __LINE__, this,
                 text.c_str());
   )

   SetLastError(0);
   WINCHECK(hDC, "CreateCompatibleDC");
   SelectObject(hDC, hFont);        // Select the font

   SIZE size;                       // GetTextExtentPoint resultant
   int rc= GetTextExtentPoint(hDC, text.c_str(), text.length(), &size);
   WINCHECK(rc, "GetTextExtentPoint");

   outlen.x= size.cx;
   outlen.y= size.cy;
}

//----------------------------------------------------------------------------
//
// Method-
//       WinFont::render
//
// Purpose-
//       Render text using this Font
//
//----------------------------------------------------------------------------
void
   WinFont::render(                 // Render text using this Font
     Buffer*           buffer,      // Into this Buffer
     const XYOffset&   inpoff,      // At this pixel offset
     const XYLength&   inplen,      // For this pixel length
     const std::string&text,        // Using this text
     int               inpmode)     // And this justification mode
{
   int                 rc;

   IFHCDM(
     Logger::log("%4d: WinFont(%p)::render(%p=%s,O{%u,%u},L{%u,%u},%s,%x)\n",
                 __LINE__, this, buffer, buffer->getName(),
                 inpoff.x, inpoff.y, inplen.x, inplen.y, text.c_str(), inpmode);
   )

   if( buffer != NULL && hFont != NULL && inplen.x > 0 && inplen.y > 0 )
   {
     // Initialize parameters
     RECT rect;
     rect.left= rect.top= 0;
     rect.right=  inplen.x;
     rect.bottom= inplen.y;
     logRect(__LINE__, rect);

     int format= DT_SINGLELINE;
     switch(inpmode&Justification::TB_MASK)
     {
       case Justification::TB_TOP:
         format |= DT_TOP;
         break;

       case Justification::TB_BOTTOM:
         format |= DT_BOTTOM;
         break;

       case Justification::TB_CENTER:
         format |= DT_VCENTER;
         break;

       default:
         break;
     }

     switch(inpmode&Justification::LR_MASK)
     {
       case Justification::LR_LEFT:
         format |= DT_LEFT;
         break;

       case Justification::LR_RIGHT:
         format |= DT_RIGHT;
         break;

       case Justification::LR_CENTER:
         format |= DT_CENTER;
         break;

       default:
         break;
     }

     // Create a temporary bitmap
     SetLastError(0);
     WINCHECK(hDC, "CreateCompatibleDC");

     HBITMAP hMap= CreateCompatibleBitmap(hDC, inplen.x, inplen.y);
     WINCHECK(hMap, "CreateCompatibleBitmap");
     SelectObject(hDC, hMap);       // Select the bitmap
     SelectObject(hDC, hFont);      // Select the font

     // Paint the rectangle, then draw the text (into the memory DC)
     HBRUSH brush= CreateSolidBrush(RGB(0,0,0));
     rc= FillRect(hDC, &rect, brush);

     COLORREF cr= SetTextColor(hDC, RGB(255,255,255));
     WINCHECK(cr, "SetTextColor");
     cr= SetBkColor(hDC, RGB(0,0,0));
     WINCHECK(cr, "SetBkColor");
     rc= SetBkMode(hDC, OPAQUE);
     WINCHECK(cr, "SetBkMode");

     IFHCDM(
       Logger::log("%4d: DrawText(%p,%s,%d,LTRB{%d,%d,%d,%d},0x%x)\n", __LINE__,
                   hDC, text.c_str(), -1,
                   rect.left, rect.top, rect.right, rect.bottom, format);
     )
     rc= DrawText(hDC, text.c_str(), -1, &rect, format);
     WINCHECK(rc, "DrawText");
     logRect(__LINE__, rect);

     for(YOffset_t y= 0; y<inplen.y; y++)
     {
       Pixel* pixel= buffer->getPixel(inpoff.x, inpoff.y + y);
       for(XOffset_t x= 0; x<inplen.x; x++)
       {
         COLORREF cref= GetPixel(hDC, x, y);
         if( int(cref) > 0 )
           pixel->setColor(color);

         pixel++;
       }
     }

     // Delete the temporary objects
     DeleteObject(hMap);
   }
}
#include "gui/namespace.end"

