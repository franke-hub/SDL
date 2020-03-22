//----------------------------------------------------------------------------
//
//       Copyright (C) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Prt.cpp
//
// Purpose-
//       Print a file on the printer.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For tolower
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>                 // For atoi
#include <string.h>
#include <windows.h>

#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "PRT.NEW " // Source file name

#define NAMESIZE                768 // Sizeof(fileName)

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define extentof(a) (sizeof(a)/sizeof((a)[0])) // Number of entries in array

#ifdef HCDM
#define ISHCDM 1
#define TRACEF(x) \
   (tracef("%s %4d: ", __SOURCE__, __LINE__), tracef x)

#else
#define ISHCDM 0
#define TRACEF(x) \
   (swVerbose > 8)                                            \
     ? (tracef("%s %4d: ", __SOURCE__, __LINE__), tracef x)   \
     : ((void)0)
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       PrtContext
//
// Purpose-
//       Describe the printer context.
//
//----------------------------------------------------------------------------
struct PrtContext
{
   int               flags;         // Selection flags

   HDC               ghdc;          // Printer device context handle

   // selectFont resultant
   int               ft;            // Font type
   LOGFONT           lf;            // Font descriptor
};

//----------------------------------------------------------------------------
//
// Struct-
//       FontEntry
//
// Purpose-
//       Describe a font selection entry.
//
//----------------------------------------------------------------------------
struct FontEntry
{
   char*             name;          // The name of the font
   LPARAM            flags;         // Associated control flags
#define rasterOK    (1)             // Raster font is acceptable
#define variableOK  (2)             // Variable font is acceptable
#define apologize   (4)             // Apologize if error
};

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int           swDisplay= 0;  // Print on display
static int           swVerbose= 0;  // Verbosity
static DWORD         gdwTextColor= 0x000000; // current text color
static int           ptSize= 11;    // Desired font size, in points
static struct FontEntry
                     fontList[]=    // The default font list
   { {"Courier", 0},
     {"Courier New", 0},
     {NULL, variableOK|apologize},
     {NULL, rasterOK|variableOK|apologize}
   };

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugNAME
//
// Purpose-
//       Display a name, which may not be printable.
//
//----------------------------------------------------------------------------
void
   debugNAME(                       // Debugging: Display NAME
     const char*       named,       // The name of the name
     const char*       name)        // The name itself
{
   int                 i;
   int                 c;

   if( !ISHCDM || swVerbose <= 8 )
     return;

   tracef("%s %4d: %s(", __SOURCE__, __LINE__, named);
   for(i= 0; name[i] != '\0'; i++)
   {
     c= name[i] & 0xff;

     if( isprint(c) )
       tracef("%c", c);
     else
       tracef(".");
   }

   tracef(") *");
   for(i= 0; name[i] != '\0'; i++)
   {
     c= name[i] & 0xff;

     if( i > 0 && (i%4) == 0 )
       tracef(" ");
     tracef("%.2x", c);
   }
   tracef("*\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugHDC
//
// Purpose-
//       Display a HDC.
//
//----------------------------------------------------------------------------
void
   debugHDC(                        // Debugging: Display HDC
     HDC               ghdc)        // Device context handle
{
   TRACEF(("\n"));
   TRACEF(("Device capabilities:\n"));
   TRACEF((" %10u= HORZSIZE\n",        GetDeviceCaps(ghdc, HORZSIZE) ));
   TRACEF((" %10u= VERTSIZE\n",        GetDeviceCaps(ghdc, VERTSIZE) ));
   TRACEF((" %10u= HORZRES\n",         GetDeviceCaps(ghdc, HORZRES) ));
   TRACEF((" %10u= VERTRES\n",         GetDeviceCaps(ghdc, VERTRES) ));
   TRACEF((" %10u= LOGPIXELSX\n",      GetDeviceCaps(ghdc, LOGPIXELSX) ));
   TRACEF((" %10u= LOGPIXELSY\n",      GetDeviceCaps(ghdc, LOGPIXELSY) ));
   TRACEF((" %10u= PHYSICALWIDTH\n",   GetDeviceCaps(ghdc, PHYSICALWIDTH) ));
   TRACEF((" %10u= PHYSICALHEIGHT\n",  GetDeviceCaps(ghdc, PHYSICALHEIGHT) ));
   TRACEF((" %10u= PHYSICALOFFSETX\n", GetDeviceCaps(ghdc, PHYSICALOFFSETX) ));
   TRACEF((" %10u= PHYSICALOFFSETY\n", GetDeviceCaps(ghdc, PHYSICALOFFSETY) ));
   TRACEF((" 0x%.8X= RASTERCAPS\n",    GetDeviceCaps(ghdc, RASTERCAPS) ));
   TRACEF((" 0x%.8X= CURVECAPS\n",     GetDeviceCaps(ghdc, CURVECAPS) ));
   TRACEF((" 0x%.8X= LINECAPS\n",      GetDeviceCaps(ghdc, LINECAPS) ));
   TRACEF((" 0x%.8X= POLYGONALCAPS\n", GetDeviceCaps(ghdc, POLYGONALCAPS) ));
   TRACEF((" 0x%.8X= TEXTCAPS\n",      GetDeviceCaps(ghdc, TEXTCAPS) ));
   TRACEF((" 0x%.8X= TEXTALIGN\n",     GetTextAlign(ghdc) ));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugLOGFONT
//
// Purpose-
//       Display an LOGFONT.
//
//----------------------------------------------------------------------------
void
   debugLOGFONT(                    // Debugging: Display LOGFONT
     LOGFONT*          lf)          // -> LOGFONT
{
   TRACEF(("\n"));
   TRACEF(("debugLOGFONT(%p)\n", lf));

   debugNAME(" Face", lf->lfFaceName);
   TRACEF((" %10d= lfHeight\n",         lf->lfHeight));
   TRACEF((" %10d= lfWidth\n",          lf->lfWidth));
   TRACEF((" %10d= lfEscapement\n",     lf->lfEscapement));
   TRACEF((" %10d= lfOrientation\n",    lf->lfOrientation));
   TRACEF((" %10d= lfWeight\n",         lf->lfWeight));
   TRACEF((" %10d= lfItalic\n",         lf->lfItalic));
   TRACEF((" %10d= lfUnderline\n",      lf->lfUnderline));
   TRACEF((" %10d= lfStrikeOut\n",      lf->lfStrikeOut));
   TRACEF((" %10d= lfCharSet\n",        lf->lfCharSet));
   TRACEF((" 0x%.8x= lfOutPrecision\n",   lf->lfOutPrecision));
   TRACEF((" 0x%.8x= lfClipPrecision\n",  lf->lfClipPrecision));
   TRACEF((" 0x%.8x= lfQuality\n",        lf->lfQuality));
   TRACEF((" 0x%.8x= lfPitchAndFamily\n", lf->lfPitchAndFamily));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugTEXTMETRIC
//
// Purpose-
//       Display a TEXTMETRIC
//
//----------------------------------------------------------------------------
void
   debugTEXTMETRIC(                 // Debugging: Display TEXTMETRIC
     TEXTMETRIC*       tm)          // -> TEXTMETRIC
{
   TRACEF(("\n"));
   TRACEF(("debugTEXTMETRIC(%p)\n", tm));

   TRACEF((" %10d= tmHeight\n",         tm->tmHeight));
   TRACEF((" %10d= tmAscent\n",         tm->tmAscent));
   TRACEF((" %10d= tmDescent\n",        tm->tmDescent));
   TRACEF((" %10d= tmInternalLeading\n",tm->tmInternalLeading));
   TRACEF((" %10d= tmExternalLeading\n",tm->tmExternalLeading));
   TRACEF((" %10d= tmAveCharWidth\n",   tm->tmAveCharWidth));
   TRACEF((" %10d= tmMaxCharWidth\n",   tm->tmMaxCharWidth));
   TRACEF((" %10d= tmWeight\n",         tm->tmWeight));
   TRACEF((" %10d= tmOverhang\n",       tm->tmOverhang));
   TRACEF((" %10d= tmDigitizedAspectX\n", tm->tmDigitizedAspectX));
   TRACEF((" %10d= tmDigitizedAspectY\n", tm->tmDigitizedAspectY));
   TRACEF((" %10d= tmFirstChar\n",      tm->tmFirstChar));
   TRACEF((" %10d= tmLastChar\n",       tm->tmLastChar));
   TRACEF((" %10d= tmDefaultChar\n",    tm->tmDefaultChar));
   TRACEF((" %10d= tmBreakChar\n",      tm->tmBreakChar));
   TRACEF((" %10d= tmItalic\n",         tm->tmItalic));
   TRACEF((" %10d= tmUnderlined\n",     tm->tmUnderlined));
   TRACEF((" %10d= tmStruckOut\n",      tm->tmStruckOut));
   TRACEF((" %10d= tmPitchAndFamily\n", tm->tmPitchAndFamily));
   TRACEF((" %10d= tmCharSet\n",        tm->tmCharSet));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       strcpylower
//
// Purpose-
//       Convert a string to lower case.
//
//----------------------------------------------------------------------------
static char*                        // Resultant (output) string
   strcpylower(                     // Convert string to lower case
     char*             out,         // Resultant (output) string
     const char*       inp)         // Input string
{
   char*               const resultant= out;

   for(;;)
   {
     *out= tolower(*inp);
     if( *inp == 0 )
       break;

     inp++;
     out++;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       specFamProc
//
// Purpose-
//       Called by EnumFontFamilies for each font in the specified family.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if accepted)
   CALLBACK specFamProc(            // EnumFontFamilies callback routine
     ENUMLOGFONT FAR*
                     elf,           // -> ENUMLOGFONT
     NEWTEXTMETRIC FAR*
                     ntm,           // -> NEWTEXTMETRIC
     int             ft,            // Font type
     LPARAM          param)         // -> PrtContext
{
   PrtContext*       const context= (PrtContext*)param; // ->PrtContext
   int               const flags= context->flags; // Control flags

   int               returncd= 1;   // This routine's return code

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   TRACEF( ("specFamProc(%p)\n", context) );
   debugNAME(">>> Font", (char*)elf->elfFullName);
   debugNAME(">>>Style", (char*)elf->elfStyle);
   debugNAME(">>> Face", (char*)elf->elfLogFont.lfFaceName);
   TRACEF( (">>> Type(%.8x)\n", ft) );
   TRACEF( (">>>Flags(%.8x)\n", context->flags) );

   //-------------------------------------------------------------------------
   // Accept or reject the font
   //-------------------------------------------------------------------------
   if( (ft&RASTER_FONTTYPE) != 0    // If this is a raster font
       &&(flags&rasterOK) == 0 )    // and a raster font is unacceptable
   {
     TRACEF((">>>RASTER\n"));
     goto exit;                     // Reject the font
   }

   if( ((ntm->tmPitchAndFamily&0x01) != 0) // If variable pitch
         &&(flags&variableOK) == 0 ) // and variable pitch is unacceptable
   {
     TRACEF((">>>VARIABLE(%.2x)\n", ntm->tmPitchAndFamily));
     goto exit;                     // Reject the font
   }

   if( ntm->tmWeight < FW_NORMAL || ntm->tmWeight > FW_SEMIBOLD )
   {
     TRACEF((">>>WEIGHT(%d)\n", ntm->tmWeight));
     goto exit;                     // Reject the font
   }

   if( ntm->tmItalic != 0 )         // If italic font
   {
     TRACEF((">>>ITALIC(%d)\n", ntm->tmItalic));
     goto exit;                     // Reject the font
   }

   if( ntm->tmUnderlined != 0 )     // If underlined font
   {
     TRACEF((">>>UNDERLINED(%d)\n", ntm->tmUnderlined));
     goto exit;                     // Reject the font
   }

   if( ntm->tmStruckOut != 0 )      // If italic font
   {
     TRACEF((">>>STRUCKOUT(%d)\n", ntm->tmStruckOut));
     goto exit;                     // Reject the font
   }

   if( ntm->tmCharSet != ANSI_CHARSET ) // If not an ANSI chararacter set font
   {
     TRACEF((">>>CHARSET(%d)\n", ntm->tmCharSet));
     goto exit;                     // Reject the font
   }

   //-------------------------------------------------------------------------
   // Font accepted, save its characteristics
   //-------------------------------------------------------------------------
   returncd= 0;                     // Indicate accepted

   context->ft= ft;
   memcpy(&context->lf, &elf->elfLogFont, sizeof LOGFONT);

   if( context->flags&apologize )
     printf("Font(%s) selected.\n", elf->elfFullName);

   context->lf.lfHeight=
       (-MulDiv(ptSize, GetDeviceCaps(context->ghdc, LOGPIXELSY), 72));
   context->lf.lfWidth= 0;

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
exit:
   TRACEF((">>>RC(%d)\n", returncd));

   return returncd;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nullFamProc
//
// Purpose-
//       Called by EnumFontFamilies for each font family.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if accepted)
   CALLBACK nullFamProc(            // EnumFontFamilies callback routine
     ENUMLOGFONT FAR*
                     elf,           // -> ENUMLOGFONT
     NEWTEXTMETRIC FAR*
                     ntm,           // -> NEWTEXTMETRIC
     int             ft,            // Font type
     LPARAM          param)         // -> PrtContext
{
   PrtContext*       const context= (PrtContext*)param; // ->PrtContext
   char              name[NAMESIZE];// Work area for name
   char*             end;           // -> Style in font name

   int               rc;            // Return code

   TRACEF(("nullFamProc()\n"));
   strcpy(name, (char*)elf->elfFullName);
   if( elf->elfStyle[0] != 0 )       // There is a style name
   {
     end= strstr(name, (char*)elf->elfStyle);
     if( end != NULL )
       *(end-1)= '\0';              // Throw out style in name
   }
   rc= EnumFontFamilies(context->ghdc,
                        name,
                        (FONTENUMPROC)specFamProc,
                        param);

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       selectPrinter
//
// Purpose-
//       Find a printer whose name matches a partial printer name.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 if selected
   selectPrinter(                   // Get printer definition
     char*           name,          // -> Name to match for printer
     char*           fullName,      // -> Where to save printer's full name
     char*           driver,        // -> Where to save driver name
     char*           port)          // -> Where to save port name
{
   DWORD             const dwFlags= PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL;
   int               returncd= 1;   // This routine's return code

   LPPRINTER_INFO_2  pPrinters;
   DWORD             cbPrinters;
   DWORD             cReturned;

   char              lowName[NAMESIZE]; // Name to match in lower case
   char              lowPrinter[NAMESIZE]; // Printer name in lower case

   int               rc;
   int               i;

   //-------------------------------------------------------------------------
   // Special case - "display"
   //-------------------------------------------------------------------------
   strcpylower(lowPrinter, name);
   if( strcmp(lowPrinter, "display") == 0 )
   {
     swDisplay= TRUE;
     return 0;
   }

   //-------------------------------------------------------------------------
   // Allocate the printer enumeration array
   //-------------------------------------------------------------------------
   EnumPrinters(dwFlags, NULL, 2, NULL, 0, &cbPrinters, &cReturned);
   pPrinters= (LPPRINTER_INFO_2)LocalAlloc(LPTR, cbPrinters+4);
   if( !pPrinters )
   {
     printf("LocalAlloc failed\n");
     goto done_refreshing;
   }

   //-------------------------------------------------------------------------
   // Enumerate the printers
   //-------------------------------------------------------------------------
   rc= EnumPrinters(dwFlags, NULL, 2, (LPBYTE)pPrinters,
                      cbPrinters, &cbPrinters, &cReturned);
   if( !rc )
   {
     printf("EnumPrinters failed\n");
     goto done_refreshing;
   }

   if( cReturned > 0 )
   {
     strcpylower(lowName, name);
     for(i= 0; (unsigned)i < cReturned; i++)
     {
       //---------------------------------------------------------------------
       // for each printer in the PRINTER_INFO_2 array: look for match
       //---------------------------------------------------------------------
       strcpylower(lowPrinter, (pPrinters+i)->pPrinterName);
       if( strncmp(lowPrinter, lowName, strlen(lowName)) == 0 ) // Found match
       {
         if( returncd == 0 )        // Had already found a match
         {
           printf("Printer(%s) ambiguously(%s,%s)\n",
                  name, fullName, (pPrinters+i)->pPrinterName);
           returncd= 1;
           goto done_refreshing;
         }
         strcpy(fullName, (pPrinters + i)->pPrinterName);
         strcpy(port, (pPrinters + i)->pPortName);
         strcat(driver, (pPrinters + i)->pDriverName);
         returncd= 0;
       }
     }

     if( returncd != 0 )
     {
       printf("Printer(%s) not found.\n", name);
     }
   }
   else
     printf("No printers listed\n");

done_refreshing:
   LocalFree(LocalHandle(pPrinters));

   TRACEF(("%d= selectPrinter()\n", rc));
   return returncd;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       selectFont
//
// Purpose-
//       Print a file using specified printer, font and driver.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 if OK
   selectFont(                      // Select a font
     char*           preferredFont, // -> Font user especially wants
     PrtContext*     context)       // Resultant
{
   int               rc;            // Return code
   int               i;

   TRACEF(("selectFont(%p)\n", context));
   if( preferredFont != NULL )      // If a preferred font exists
   {
     context->flags= rasterOK|variableOK;

     TRACEF((">>flags(%.8x) name(%s)\n", context->flags, preferredFont));
     rc= EnumFontFamilies(context->ghdc,
                          preferredFont,
                          (FONTENUMPROC)specFamProc,
                          (LPARAM)context);
     TRACEF((">>RC(%d)\n", rc));
     if( rc != 0 )
       fprintf(stderr, "Font(%s) not available\n", preferredFont);
   }

   rc= 1;
   for(i= 0; i<extentof(fontList); i++)
   {
     context->flags= fontList[i].flags;

     TRACEF((">>flags(%.8x) name(%s)\n", context->flags, fontList[i].name));
     if( fontList[i].name == NULL )
       rc= EnumFontFamilies(context->ghdc,
                            NULL,
                            (FONTENUMPROC)nullFamProc,
                            (LPARAM)context);
     else
       rc= EnumFontFamilies(context->ghdc,
                            fontList[i].name,
                            (FONTENUMPROC)specFamProc,
                            (LPARAM)context);
     TRACEF((">>RC(%d)\n", rc));
     if( rc == 0 )                  // Found font
       break;
   }

   TRACEF(("%d= selectFont()\n", rc));
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       printFile
//
// Purpose-
//       Print a file using specified printer context.
//
//----------------------------------------------------------------------------
static void
   printFile(                       // Print a file
     char*           file,          // -> Name of file to print
     char*           preferredFont, // -> Font user especially wants
     PrtContext*     context)       // -> Printer context
{
   char              line[2000];    // Line of text to print
   DOCINFO           di;            // Document info
   HDC               const ghdc= context->ghdc; // Printer device context handle
   HFONT             hUserFont= NULL; // Indirect font
   HFONT             hSaveFont;     // Selected font
   FILE*             inp;           // -> File to be printed
   long              pixHeight;     // Height of chars, in pixels
   RECT              rect;          // Rectangle descriptor
   TEXTMETRIC        tm;            // Metrics of current font
   long              totHeight;     // Height of chars + spacing, in pixels

   char*             c;             // -> Data read from file
   int               partPage;      // Have started a page
   int               pageNumber;    // Current page number
   int               xText;         // X position to draw text
   int               yText;         // Y position to draw text

   inp= fopen(file, "rb");
   if( inp == NULL )
   {
     printf("File(%s): ", file);
     perror("open failure");
     return;
   }

   TRACEF(("File(%s) selected\n", file));

   SetMapMode(ghdc, MM_TEXT);
   SetTextColor(ghdc, gdwTextColor);
// SetTextAlign(ghdc, TA_TOP | TA_LEFT | TA_NOUPDATECP);

   di.cbSize=      sizeof(DOCINFO);
   di.lpszDocName= file;
   di.lpszOutput=  NULL;
   StartDoc(ghdc, &di);

   hUserFont= CreateFontIndirect(&context->lf);
   if( hUserFont == NULL )
   {
     fprintf(stderr, "Font(%s) CreateFontIndirect() failure\n",
                     context->lf.lfFaceName);
     goto exit;
   }
   hSaveFont= (HFONT)SelectObject(ghdc, hUserFont);
   GetTextMetrics(ghdc, &tm);       // Get data on current font
   pixHeight= tm.tmHeight;
   totHeight= pixHeight + tm.tmInternalLeading;
                                    // Include the space at top of box

   rect.top=       2;
   rect.left=      2;
   rect.bottom=    GetDeviceCaps(ghdc, VERTRES) - rect.top  - rect.top;
   rect.right=     GetDeviceCaps(ghdc, HORZRES) - rect.left - rect.left;

   TRACEF(("\n"));
   TRACEF(("Context(%p):\n", context));
   TRACEF((" %10u= ghdc\n", context->ghdc));
   TRACEF((" %10u= pixHeight\n", pixHeight));
   TRACEF((" %10u= totHeight\n", totHeight));
   TRACEF((" %10u= top\n", rect.top));
   TRACEF((" %10u= left\n", rect.left));
   TRACEF((" %10u= bottom\n", rect.bottom));
   TRACEF((" %10u= right\n", rect.right));
   TRACEF((" 0x%.8x= ft\n", context->ft));

   debugHDC(ghdc);
   debugLOGFONT(&context->lf);
   debugTEXTMETRIC((TEXTMETRIC*)&tm);

   partPage= 0;
   pageNumber= 0;
   xText= rect.left;                // Set left margin
   for(;;)
   {
     c= fgets(line, sizeof line, inp); // Read a line to print
     if( c == NULL )                // Didn't read anything
       break;

     if( !partPage )                // No page to print on
     {
       partPage= 1;
       pageNumber++;
       TRACEF(("Page(%4d)\n", pageNumber));
       StartPage(ghdc);
       yText= rect.top;             // Start in upper left corner
     }
     line[strlen(line)-1]= '\0';    // Throw away \n at end of line
     TextOut(ghdc, xText, yText, line, strlen(line));
     TRACEF(("X(%4d) Y(%4d) '%s'\n", xText, yText, line));

     yText += totHeight;
     if( yText+pixHeight > (int)rect.bottom )
     {
       partPage= 0;
       EndPage(ghdc);

       if( swDisplay )
       {
         printf("Page?: ");
         getchar();
       }
     }
   }

exit:
   if( partPage )                   // Partial page in production
      EndPage(ghdc);
   if( hUserFont != NULL)
     DeleteObject(hUserFont);
   EndDoc(ghdc);
   fclose(inp);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Print usage information.
//
//----------------------------------------------------------------------------
static void
   info(                            // Print usage information
     char*             argv[])      // Argument array
{
   printf("Usage: %s {options} filename ...\n", argv[0]);
   printf("  -F<font name>    (default Courier)\n");
   printf("  -P<printer name> (display for onScreen)\n");
   printf("  -S<point size>   (default 11)\n");
   printf("  -V<verbosity>    (default 0)\n");
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   WIN32_FIND_DATA   findBuffer;    // Result of looking up names
   PrtContext        context;       // Printer context
   HDC               ghdc= 0;       // Printer device context handle
   HANDLE            handle;        // For looking up names

   char              file[NAMESIZE];// Name of file to print
   char              fullName[NAMESIZE]; // Printer's name in full
   char              driver[NAMESIZE]; // Printer driver name
   char              port[NAMESIZE];// Printer port name
   char              printer[NAMESIZE]; // Printer name

   char*             e;             // End of path in file name
   char*             p;             // -> Stuff on command line
   char*             preferredFont= NULL; // -> Font user wants

   int               gotName;       // 1 iff got file name from cmdline
   char              profile[NAMESIZE]; // Result from GetProfileString

   int               rc;
   int               argx;

   if( argc <= 1 )
   {
     info(argv);
     exit(1);
   }

   printer[0]= '\0';
   argx= 1;
   gotName= 0;
   for(;;)
   {
     if( argv[argx][0] == '-' )
     {
       switch(argv[argx][1])
       {
         case 'F':
         case 'f':
           if( argv[argx][2] == 0 )
           {
             argx++;
             preferredFont= argv[argx];
           }
           else
             preferredFont= argv[argx]+2;
           break;

         case 'P':
         case 'p':
           if( argv[argx][2] == 0)
           {
             argx++;
             p= argv[argx];
           }
           else
             p= argv[argx]+2;
           strcpy(printer, p);
           break;

         case 'S':
         case 's':
           if( argv[argx][2] == 0 )
           {
             argx++;
             p= argv[argx];
           }
           else
             p= argv[argx]+2;
           ptSize= atoi(p);
           break;

         case 'V':
         case 'v':
           if( argv[argx][2] == 0)
           {
             argx++;
             p= argv[argx];
           }
           else
             p= argv[argx]+2;
           swVerbose= atoi(p);
           break;

         default:
           info(argv);
           exit(1);
       }
     }
     argx++;
     if( argx >= argc )
       break;
   }

   if( printer[0] == 0 )            // No printer specified
   {
     rc= GetEnvironmentVariable("PRINTER", &printer[0], sizeof printer);
     if( rc == 0 )                  // No environment variable
     {
       GetProfileString("windows", "device", "", profile, sizeof profile);
                                    // Get name of default printer
       if( profile[0] == 0 )        // No default printer
       {
         printf("No default printer.\n");
         exit(1);
       }
       *(strchr(profile, ','))= '\0';// Terminate after printer name
       strcpy(printer, profile);

       TRACEF(("Printer(%s) selected\n", printer));
     }
   }

   //-------------------------------------------------------------------------
   // Select a printer
   //-------------------------------------------------------------------------
   rc= selectPrinter(printer, &fullName[0], &driver[0], &port[0]);
   if( rc != 0 )
     return 1;
   TRACEF(("Driver(%s) selected\n", driver));

   if( swDisplay )
     ghdc= CreateDC("DISPLAY", NULL, NULL, NULL); // Open the display
   else
     ghdc= CreateDC(driver, printer, NULL, NULL); // Open the printer
   if( ghdc == 0 )
   {
     perror("CreateDC() failed");
     return 1;
   }
   context.ghdc= ghdc;

   //-------------------------------------------------------------------------
   // Select a font
   //-------------------------------------------------------------------------
   rc= selectFont(preferredFont, &context);
   if( rc != 0 )
   {
     fprintf(stderr, "Printer(%s) has no available font\n", fullName);
     goto exit;
   }
   TRACEF(("Font(%s) selected\n", context.lf.lfFaceName));

   //-------------------------------------------------------------------------
   // Print the list of files
   //-------------------------------------------------------------------------
   for(argx= 1; argx<argc; argx++)
   {
     if( argv[argx][0] == '-' )
     {
       if( argv[argx][2] == 0 )     // There is space between flag and value
         argx++;                    // Skip value, too
     }
     else
     {
       handle= FindFirstFile(argv[argx], &findBuffer);
       if( handle == INVALID_HANDLE_VALUE )
       {
         printf("File(%s) not found\n", argv[argx]);
         continue;
       }

       // find end of path in string
       strcpy(file, argv[argx]);
       e= strrchr(file, '\\');
       if( e != 0 )
         e++;
       else
       {
         e= strrchr(file, ':');
         if( e != 0 )
           e++;
         else
           e= file;
       }

       // add the file name to the path
       strcpy(e, findBuffer.cFileName);

       // print the file
       printFile(file, preferredFont, &context);

       // while file names match
       for(;;)
       {
         rc= FindNextFile(handle, &findBuffer);
         if( rc == FALSE )
           break;

         // add the file name to the path
         strcpy(e, findBuffer.cFileName);

         // print the file
         printFile(file, preferredFont, &context);
       }
     }
   }
   rc= 0;

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
exit:
   if( ghdc != 0 )
     DeleteDC(ghdc);
   return rc;
}

