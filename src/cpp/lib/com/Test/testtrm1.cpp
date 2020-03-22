//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Testtrm1.cpp
//
// Purpose-
//       Test Terminal functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef HCDM                        // INLINE HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <stdio.h>
#include <string.h>

#include <com/Color.h>
#include <com/Debug.h>
#include "com/Handler.h"
#include "com/Terminal.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TESTTRM1" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ESC                      27 // ASCII code for Escape

//----------------------------------------------------------------------------
// Internal classes
//----------------------------------------------------------------------------
class MyHandler : public Handler {
public:
virtual
   ~MyHandler( void )
{
   #ifdef HCDM
     tracef("%8s= MyHandler(%p)::~MyHandler()\n", "", this);
   #endif
}

   MyHandler( void )
:  Handler()
{
   #ifdef HCDM
     tracef("%8s= MyHandler(%p)::MyHandler()\n", "", this);
   #endif
}

protected:
virtual void
   handleError(void)                // Error handler
{
   tracef( "Error(%d) handled\n", getIdent() );
}
}; // class MyHandler

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static MyHandler       handler;     // Handler  object
static Terminal        terminal;    // Terminal object

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*          color[]= { // Color names
   "Black        ",                 //  0
   "Blue         ",                 //  1
   "Green        ",                 //  2
   "Cyan         ",                 //  3
   "Red          ",                 //  4
   "Magenta      ",                 //  5
   "Brown        ",                 //  6
   "Light grey   ",                 //  7
   "Dark grey    ",                 //  8
   "Light blue   ",                 //  9
   "Light Green  ",                 // 10
   "Light Cyan   ",                 // 11
   "Light Red    ",                 // 12
   "Light Magenta",                 // 13
   "Yellow       ",                 // 14
   "White        "                  // 15
   };

//----------------------------------------------------------------------------
//
// Subroutine-
//       patttern
//
// Purpose-
//       Fill the screen with a known pattern.
//
//----------------------------------------------------------------------------
static void
   pattern(void)                    // Fill the screen with a pattern
{
   char            buff[256];       // Line pattern

   unsigned int    row;             // Current row
   int             i;

   terminal.clearScreen();
   for(row=0; row<terminal.getYSize(); row++)
   {
     strcpy(buff, "line");
     for(i=4; i<sizeof(buff)-2; i+=2)
       sprintf(&buff[i],"%.2d",row);
     terminal.wr(row, buff, strlen(buff));
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
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Color::VGA        bg, fg;        // BackGround color, ForeGround color
   unsigned int      col;           // Current column
   unsigned int      row;           // Current row

   long              count;         // Counter
   int               c;             // Last character read
   int               p;             // Next to last character read
   int               z;             // Temporary

   //-------------------------------------------------------------------------
   // Initialize debugging
   //-------------------------------------------------------------------------
   debugSetIntensiveMode();

   //-------------------------------------------------------------------------
   // Open the terminal
   //-------------------------------------------------------------------------
   terminal.setHandler(&handler);
   terminal.setAttribute(VGAColor::White, VGAColor::Blue);
   terminal.clearScreen();

   //-------------------------------------------------------------------------
   // Initial screen write/read
   //-------------------------------------------------------------------------
   tracef("Initial screen write\n");
   terminal.wr(0, "Test: Terminal");
   terminal.rd();

   //-------------------------------------------------------------------------
   // Windows bringup test
   //-------------------------------------------------------------------------
#if 0
{{{{
   INPUT_RECORD      inpRecord[16];
   DWORD             eventCount;
   unsigned int      keyCode;
   unsigned int      scanCode;
   DWORD             keyState;
   HANDLE            keyH;
   int               c;             // Last character read
   int               p;             // Next to last character read

   terminal.clearScreen();
   keyH= GetStdHandle(STD_INPUT_HANDLE);
   p= '\0';
   c= '\0';
   for(;;)
   {
     ReadConsoleInput(
         keyH,                      // input buffer handle
         inpRecord,                 // buffer to read into
         1,                         // size of read buffer
         &eventCount);              // number of records read

     if (inpRecord[0].EventType == KEY_EVENT)
     {
       keyCode=  inpRecord[0].Event.KeyEvent.uChar.UnicodeChar;
       scanCode= inpRecord[0].Event.KeyEvent.wVirtualScanCode;
       keyState= inpRecord[0].Event.KeyEvent.dwControlKeyState;

       if (inpRecord[0].Event.KeyEvent.bKeyDown)
       {
         p= c;
         c= scanCode;
       }

       terminal.physicalXY(0,0);
       terminal.printf("KC(%.4x) SC(%.4x) RC(%2d) ",
                       keyCode, scanCode,
                       inpRecord[0].Event.KeyEvent.wRepeatCount);
       terminal.printf("%s ",
                       inpRecord[0].Event.KeyEvent.bKeyDown ? "Down" : " Up ");
       terminal.printf("STATE(%.8lx)\n",
                       (long)keyState);

     }
     if (c == ESC && p == ESC)
       break;
   }
}}}}
#endif

   //-------------------------------------------------------------------------
   // Cursor mode test
   //-------------------------------------------------------------------------
   terminal.clearScreen();
   tracef("Cursor mode test\n");
   p= '\0';
   c= '\0';
   count= 0;
   for(;;)
   {
     if (terminal.ifInsertKey())
       terminal.setCursorMode(Terminal::INSERT);
     else
       terminal.setCursorMode(Terminal::REPLACE);

     terminal.physicalXY(0,0);
     terminal.logicalXY(0,0);
     terminal.printf("Insert: %s",
                     terminal.ifInsertKey() ? "LOCKED  " : "unlocked");
     terminal.logicalXY(0,1);
     terminal.printf("Scroll: %s",
                     terminal.ifScrollKey() ? "LOCKED  " : "unlocked");

     z= terminal.poll();
     if (z)
     {
       count= 0;
       p= c;
       c= terminal.rd();
       if (c == ESC && p == ESC)
         break;
     }
     count++;
     terminal.logicalXY(0,2);
     terminal.printf(" poll: %.4X (%4ld)\n", z, count);
     terminal.printf("   rd: %.4X  %.4X\n", c, p);
   }

   //-------------------------------------------------------------------------
   // Attribute test
   //-------------------------------------------------------------------------
   tracef("Attribute test\n");
   for(bg=0; bg<=VGAColor::MAXVGA; bg++) // Background color
   {
     terminal.setAttribute((Color::VGA)0, bg);
     terminal.clearScreen();        // Clear the screen
     for(fg=0; fg<=VGAColor::MAXVGA; fg++) // Foreground color
     {
       terminal.logicalXY(0, fg);
       terminal.setAttribute(fg, bg);
       terminal.printf("%3d=BG(%s) %3d=FG(%s)",
                       bg, color[bg], fg, color[fg]);
     }

     while(terminal.poll())         // Drain the keyboard queue
       terminal.rd();               // Ignoring pending characters
     c= terminal.rd();
     if (c == ESC)
       break;
   }

   //-------------------------------------------------------------------------
   // Position test
   //-------------------------------------------------------------------------
   tracef("Position test\n");
   fg= VGAColor::White;
   bg= VGAColor::Blue;
   terminal.setAttribute(fg, bg);
   terminal.clearScreen();
   terminal.printf("Position test");

   col= 1;
   row= 1;
   for(;;)                          // Character test
   {
     c= terminal.rd();              // Read next character
     if (col >= terminal.getXSize())
     {
       col= 0;
       row++;
     }
     if (row >= terminal.getYSize())
     {
       terminal.clearScreen();
       row= 0;
     }
     terminal.physicalXY(col, row);
     terminal.logicalXY(col, row);
     terminal.printf("Position[%4d,%4d] test", col, row);
     if (c == ESC)
       break;
     col++;
     row++;
   }

   //-------------------------------------------------------------------------
   // deleteRow test
   //-------------------------------------------------------------------------
   terminal.physicalXY(0,0);
   tracef("deleteRow test\n");
   pattern();
   terminal.printf("Delete row[0]");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.deleteRow(0);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Delete last row");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.deleteRow(terminal.getYSize()-1);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Delete row[%d]", terminal.getYSize()/2);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.deleteRow(terminal.getYSize()/2);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Delete row(2,23)");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.deleteRow(2, 23);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   //-------------------------------------------------------------------------
   // insertRow test
   //-------------------------------------------------------------------------
   tracef("insertRow test\n");
   pattern();
   terminal.printf("Insert row[0]");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.insertRow(0);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Insert last row");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.insertRow(terminal.getYSize()-1);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Insert row[%d]", terminal.getYSize()/2);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.insertRow(terminal.getYSize()/2);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   terminal.printf("Insert row(2,23)");
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;
   terminal.insertRow(2, 23);
   terminal.logicalXY(0,0);
   terminal.wr("Done!", 5);
   c= terminal.rd();
   if( c == ESC )
     goto test_error_indicator;

   //-------------------------------------------------------------------------
   // Error indicator tests
   //-------------------------------------------------------------------------
test_error_indicator:
   tracef("terminal error indicator test\n");
   terminal.error(9999);

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   terminal.clearScreen();
   tracef("Done!\n");
   printf("Testing complete\n");

   return 0;
}

