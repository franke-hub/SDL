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
//       Testtrm2.cpp
//
// Purpose-
//       Test Keyboard/Screen functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/Handler.h>
#include <com/Color.h>
#include <com/Debug.h>
#include "com/Keyboard.h"
#include "com/TextScreen.h"
#include "com/Terminal.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TESTTRM2" // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ESC                      27 // ASCII code for Escape

//----------------------------------------------------------------------------
// Internal classes
//----------------------------------------------------------------------------
class MyHandler : public Handler {
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
static Keyboard        keyboard;    // Keyboard object
static TextScreen      screen;      // Screen object

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
static inline void
   pattern(void)                    // Fill the screen with a pattern
{
   char            buff[256];       // Line pattern

   unsigned int    row;             // Current row
   int             i;

   screen.clearScreen();
   for(row=0; row<screen.getYSize(); row++)
   {
     strcpy(buff, "line");
     for(i=4; i<sizeof(buff)-2; i+=2)
       sprintf(&buff[i],"%.2d",row);
     screen.wr(row, buff, strlen(buff));
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
   // Open the keyboard and screen
   //-------------------------------------------------------------------------
   keyboard.setHandler(&handler);
   screen.setHandler(&handler);
   screen.setAttribute(VGAColor::White, VGAColor::Blue);
   screen.clearScreen();

   //-------------------------------------------------------------------------
   // Initial screen write/read
   //-------------------------------------------------------------------------
   tracef("Initial screen write\n");
   screen.wr(0, "Test: Keyboard/Screen");
   keyboard.rd();

   //-------------------------------------------------------------------------
   // Cursor mode test
   //-------------------------------------------------------------------------
   screen.clearScreen();
   tracef("Cursor mode test\n");
   p= '\0';
   c= '\0';
   count= 0;
   for(;;)
   {
     if (keyboard.ifInsertKey())
       screen.setCursorMode(TextScreen::INSERT);
     else
       screen.setCursorMode(TextScreen::REPLACE);

     screen.physicalXY(0,0);
     screen.logicalXY(0,0);
     screen.printf("Insert: %s",
                    keyboard.ifInsertKey() ? "LOCKED  " : "unlocked");
     screen.logicalXY(0,1);
     screen.printf("Scroll: %s",
                    keyboard.ifScrollKey() ? "LOCKED  " : "unlocked");

     z= keyboard.poll();
     if (z)
     {
       count= 0;
       p= c;
       c= keyboard.rd();
       if (c == ESC && p == ESC)
         break;
     }
     count++;
     screen.logicalXY(0,2);
     screen.printf(" poll: %.4X (%4ld)\n", z, count);
     screen.printf("   rd: %.4X  %.4X\n", c, p);
   }

   //-------------------------------------------------------------------------
   // Attribute (color) test
   //-------------------------------------------------------------------------
   tracef("Attribute test\n");
   for(bg=0; bg<=VGAColor::MAXVGA; bg++) // Background color
   {
     screen.setAttribute((Color::VGA)0, bg);
     screen.clearScreen();          // Clear the screen
     for(fg=0; fg<=VGAColor::MAXVGA; fg++) // Foreground color
     {
       screen.logicalXY(0, fg);
       screen.setAttribute(fg, bg);
       screen.printf("%3d=BG(%s) %3d=FG(%s)",
                       bg, color[bg], fg, color[fg]);
     }

     while(keyboard.poll())         // Drain the keyboard queue
       keyboard.rd();               // Ignoring anything there
     c= keyboard.rd();
     if (c == ESC)
       break;
   }

   //-------------------------------------------------------------------------
   // Position test
   //-------------------------------------------------------------------------
   tracef("Position test\n");
   fg= VGAColor::White;
   bg= VGAColor::Blue;
   screen.setAttribute(fg, bg);
   screen.clearScreen();
   screen.printf("Position test");

   col= 1;
   row= 1;
   for(;;)                          // Character test
   {
     c= keyboard.rd();              // Read next character
     if (col >= screen.getXSize())
     {
       col= 0;
       row++;
     }
     if (row >= screen.getYSize())
     {
       screen.clearScreen();
       row= 0;
     }
     screen.physicalXY(col, row);
     screen.logicalXY(col, row);
     screen.printf("Position[%4d,%4d] test", col, row);
     if (c == ESC)
       break;
     col++;
     row++;
   }

   //-------------------------------------------------------------------------
   // deleteRow test
   //-------------------------------------------------------------------------
   screen.physicalXY(0,0);
   tracef("deleteRow test\n");
   pattern();
   screen.printf("Delete row[0]");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.deleteRow(0);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Delete last row");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.deleteRow(screen.getYSize()-1);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Delete row[%d]", screen.getYSize()/2);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.deleteRow(screen.getYSize()/2);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Delete row(2,23)");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.deleteRow(2, 23);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   //-------------------------------------------------------------------------
   // insertRow test
   //-------------------------------------------------------------------------
   tracef("insertRow test\n");
   pattern();
   screen.printf("Insert row[0]");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.insertRow(0);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Insert last row");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.insertRow(screen.getYSize()-1);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Insert row[%d]", screen.getYSize()/2);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.insertRow(screen.getYSize()/2);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   pattern();
   screen.printf("Insert row(2,23)");
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;
   screen.insertRow(2, 23);
   screen.logicalXY(0,0);
   screen.wr("Done!", 5);
   c= keyboard.rd();
   if( c == ESC )
     goto test_error_indicator;

   //-------------------------------------------------------------------------
   // Error indicator tests
   //-------------------------------------------------------------------------
test_error_indicator:
   tracef("Screen error indicator test\n");
   screen.error((Terminal::Error)9999);

   tracef("Keyboard error indicator test\n");
   keyboard.error((Terminal::Error)9998);

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   screen.clearScreen();
   tracef("Done!\n");
   printf("Testing complete\n");

   return 0;
}

