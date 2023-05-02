//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.cpp
//
// Purpose-
//       Sample usage of GUI Objects.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <exception>
#include <com/Thread.h>             // For Thread::sleep
#include <com/ThreadLogger.h>

#include "Constant.h"
#include "WormBuffer.hpp"

#include "gui/Types.h"
#include "gui/Action.h"
#include "gui/Bounds.h"
#include "gui/Buffer.h"
#include "gui/Event.h"
#include "gui/Filler.h"
#include "gui/Font.h"
#include "gui/Line.h"
#include "gui/Object.h"
#include "gui/Offset.h"
#include "gui/Text.h"
#include "gui/Window.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef DEFAULT_DELAY
#define DEFAULT_DELAY           100 // Default delay (milliseconds)
#endif

#ifndef DIM_WORM
#define DIM_WORM                 25 // The number of worm objects
#endif

#ifndef MINIMUM_DELAY
#define MINIMUM_DELAY             0 // Minimum delay (milliseconds)
#endif

#ifndef RUN_WORM
#define RUN_WORM               2000 // The number of worm iterations
#endif

#ifndef OBJECT_SIZE
#define OBJECT_SIZE             256 // Size of Object
#endif

#ifndef WINDOW_SIZE
#define WINDOW_SIZE             600 // Size of Window
#endif

#ifndef USE_TESTOBJECT
#define USE_TESTOBJECT 0            // Use Object test?
#endif

#ifndef USE_TESTBRINGUP
#define USE_TESTBRINGUP 1           // Use bringup test?
#endif

#ifndef USE_TESTBRINGUP_WAIT        // Overrides USE_TESTBRINGUP_DELAY
#define USE_TESTBRINGUP_WAIT 0      // Use bringup wait test?
#endif

#ifndef USE_TESTBRINGUP_DELAY
#define USE_TESTBRINGUP_DELAY 5000  // If defined, ending delay (milliseconds)
#endif

#ifndef USE_TESTWINDOW
#define USE_TESTWINDOW 1            // Use TESTWINDOW test?
#endif

#ifndef USE_TESTWINDOW_COLOR
#define USE_TESTWINDOW_COLOR 1      // Use TESTWINDOW color change test?
#endif

#ifndef USE_TESTWINDOW_LINE
#define USE_TESTWINDOW_LINE 1       // Use TESTWINDOW line test?
#endif

#ifndef USE_TESTWINDOW_MOVE
#define USE_TESTWINDOW_MOVE 1       // Use TESTWINDOW movement test?
#endif

#ifndef USE_TESTWINDOW_TREE
#define USE_TESTWINDOW_TREE 1       // Use TESTWINDOW raise/lower test?
#endif

#ifndef USE_TESTWINDOW_TWO
#define USE_TESTWINDOW_TWO 1        // Use TESTWINDOW second window?
#endif

#ifndef USE_TESTWINDOW_WAIT         // Overrides USE_TESTWINDOW_DELAY
#define USE_TESTWINDOW_WAIT 0       // Use TESTWINDOW wait (at end)?
#endif

#ifndef USE_TESTWINDOW_DELAY
#define USE_TESTWINDOW_DELAY 5000  // If defined, ending delay (milliseconds)
#endif

#ifndef USE_WORMWINDOW
#define USE_WORMWINDOW 1            // Use WORMWINDOW test?
#endif

//----------------------------------------------------------------------------
//
// Class-
//       ActionChange
//
// Purpose-
//       Test Action object.
//
//----------------------------------------------------------------------------
class ActionChange : public Action
{
protected:
Color_t                color;       // Our color
const char*            name;        // Our name

public:
inline
   ~ActionChange( void ) {};

inline
   ActionChange(
     Object*           parent= NULL)
:  Action(parent), color(0), name(NULL) {}

inline Color_t
   getColor( void ) const
{  return color;
}

inline void
   setColor(
     Color_t           color)
{  this->color= color;
}

inline void
   setName(
     const char*       name)
{  this->name= name;
}

public:
virtual void
   callback(                        // Event handler
     const Event&      event)       // For this Event
{
   #ifdef HCDM
     Logger::log("ActionChange::callback() %s\n", name);
   #endif

   if( event.getCode() == Event::EC_KEYDOWN )
     throw "ActionChange.TERMINATE";

   if( event.getCode() == Event::EC_MOUSEOVER
       && (event.getData() == Event::MO_ENTER
           || event.getData() == Event::MO_EXIT) )
   {
     // Swap colors with the parent, redraw if different
     Object* parent= getParent();
     if( parent != NULL )
     {
       Color_t color= parent->getColor();
       if( color != getColor() )
       {
         parent->setColor(getColor());
         setColor(color);
         parent->redraw();
       }
     }
   }
}
}; // class ActionChange

//----------------------------------------------------------------------------
//
// Class-
//       DebugVisitor
//
// Purpose-
//       Debugging display visitor.
//
//----------------------------------------------------------------------------
class DebugVisitor : public ObjectVisitor // DebugVisior
{
public:
virtual Object*                     // Always returns object
   visit(                           // Visit an Object
     Object*           object);     // The Object to visit
}; // class DebugVisitor

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             errorCount= 0; // Error count
static XYLength        objectLength= {OBJECT_SIZE, OBJECT_SIZE};
static XYLength        windowLength= {WINDOW_SIZE, WINDOW_SIZE};

//----------------------------------------------------------------------------
//
// Subroutine-
//       toColor
//
// Purpose-
//       Used (only by) bufferDebug to return a pseudo-color
//
//----------------------------------------------------------------------------
static inline int                   // Pseudo color
   toColor(                         // Convert to pseudo color
     Pixel*            pixel)       // -> Pixel
{
   int                 result= '?'; // Default resultant
   Color_t             color;       // Pixel color

   color= pixel->getColor();
   if( color == RGB::Black )
     result= ' ';

   else if( color == RGB::White )
     result= 'W';

   else if( color == RGB::Grey )
     result= 'm';

   else if( color == RGB::Red )
     result= 'R';

   else if( color == RGB::Green )
     result= 'G';

   else if( color == RGB::Blue )
     result= 'B';

   else if( color == RGB::LightRed )
     result= 'r';

   else if( color == RGB::LightGreen )
     result= 'g';

   else if( color == RGB::LightBlue )
     result= 'b';

   else if( color == RGB::Cyan )
     result= 'C';

   else if( color == RGB::Magenta )
     result= 'M';

   else if( color == RGB::Yellow )
     result= 'Y';

   else if( color == RGB::Brown )
     result= 'N';

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugBuffer
//
// Purpose-
//       Debug the Buffer object.
//
//----------------------------------------------------------------------------
static void
   debugBuffer(                     // Debug
     Buffer&           buffer)      // This Buffer object
{
   debugSetStandardMode();
   tracef("bufferDebug(%p) %s\n", &buffer, buffer.getName());

   XYOffset offset= buffer.getOffset();
   XYLength length= buffer.getLength();
   tracef("offset(%u,%u) length(%u,%u)\n\n",
          offset.x, offset.y, length.x, length.y);

   Offset_t x;                      // (Separate for Windows)
   for(x= 0; x<length.x; x++)
     tracef("*");
   tracef("**\n");

   Pixel* pixel= buffer.getPixel(0,0);
   for(Offset_t y= 0; y<length.y; y++)
   {
     tracef("*");
     for(Offset_t x= 0; x<length.x; x++)
     {
       tracef("%c", toColor(pixel));
       pixel++;
     }
     tracef("*\n");
   }

   for(x= 0; x<length.x; x++)
     tracef("*");
   tracef("**\n");
   Debug::get()->flush();
   debugSetIntensiveMode();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       delay
//
// Purpose-
//       Delay between updates.
//
//----------------------------------------------------------------------------
static void
   delay(                           // Delay
     int               msec= DEFAULT_DELAY)  // Time in milliseconds
{
   #ifdef HCDM
     Logger::log("%4d: delay(%d)\n",  __LINE__, msec);
   #endif

   if( msec < MINIMUM_DELAY )
     msec= MINIMUM_DELAY;

   if( msec == 0 )
     return;

   Thread::sleep((double)msec/1000.0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       msgerr
//
// Purpose-
//       Indicate error.
//
//----------------------------------------------------------------------------
static void
   msgerr(                          // Error log
     const char*       format,      // Format string
                       ...)         // Remaining arguments
{
   va_list             argptr;      // Argument list pointer

   errorCount++;

   va_start(argptr, format);        // Initialize va_ functions
   Logger::get()->vlogf(format, argptr);
   va_end(argptr);                  // Close va_ functions

   va_start(argptr, format);        // Initialize va_ functions
   vfprintf(stderr, format, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       visit
//
// Purpose-
//       Run debugging visitor on a subtree.
//
//----------------------------------------------------------------------------
static inline void
   visit(                           // Debug Object layout
     Object&           object)      // The object to debug
{
   Logger::log("\n-------------------------------------------------------\n");
   Logger::log("Visit subtree\n");
   DebugVisitor visitor;
   object.visit(visitor);
   Logger::log("\n-------------------------------------------------------\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       DebugVisitor::visit
//
// Purpose-
//       Debugging display of an Object
//
//----------------------------------------------------------------------------
Object*                             // Always returns object
   DebugVisitor::visit(             // Object debugging display
     Object*           object)      // The Object to visit
{
   XYOffset            offset;      // Visible offset (in Window)
   XYLength            length;      // Visible length
   Buffer*             buffer;      // Buffer Object
   Object*             parent;      // Parent Object
   const char*         bufferName;  // Object's Buffer's name
   const char*         objectName;  // Object's name
   const char*         parentName;  // Object's Parent's name

   buffer= object->getBuffer();
   parent= (Object*)object->getParent();
   bufferName= "NONE";
   objectName= object->getName();
   parentName= "NONE";
   if( buffer != NULL )
     bufferName= buffer->getName();
   if( parent != NULL )
     parentName= parent->getName();

   Logger::log("DebugVisitor(%p)::visit(%p) %s %.8x\n", this,
                object, objectName, object->getColor());

   Logger::log("%s=Parent(%p), %s=Buffer(%p)\n",
                parentName, parent, bufferName, buffer);

   Bounds* bounds= dynamic_cast<Bounds*>(object);
   if( bounds == NULL )
     Logger::log("Object (Unbounded) %s\n", object->getName());
   else
     Logger::log("Object Offset(%d,%d) Length(%d,%d)\n",
                  bounds->getOffset().x, bounds->getOffset().y,
                  bounds->getLength().x, bounds->getLength().y);

   object->range(offset, length);
   Logger::log("Buffer Offset(%d,%d) Length(%d,%d)\n",
                offset.x, offset.y, length.x, length.y);
   Logger::log("\n");

   return object;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testObject
//
// Purpose-
//       Basic Object function test.
//
//----------------------------------------------------------------------------
static void
   testObject( void )               // Verify static Object operation
{
   #ifdef HCDM
     Logger::log("\n");
     debugf("%4d: testObject()..\n", __LINE__);
   #endif

   Object              object(NULL);// A simple stand-alone Object

   #ifdef HCDM
     Logger::log("%4d: ..testObject()\n",  __LINE__);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testBringup
//
// Purpose-
//       Insure Windows operate correctly
//
//----------------------------------------------------------------------------
static void
   testBringup( void )              // Verify that Windows work properly
{
   #ifdef HCDM
     Logger::log("\n");
     debugf("%4d: testBringup()..\n", __LINE__);
   #endif

   Window              window(windowLength);
   Line                line(&window);

   //-------------------------------------------------------------------------
   // Set the line color
   line.setColor(RGB::White);

   //-------------------------------------------------------------------------
   // Make window visible
   //-------------------------------------------------------------------------
   #ifdef HCDM
     Logger::log("\n-------------------------------------------------------\n");
   #endif
   window.redraw();
   window.setAttribute(Object::VISIBLE, TRUE);
   #ifdef HCDM
     Logger::log("\n-------------------------------------------------------\n");
   #endif

   //-------------------------------------------------------------------------
   // Wait test
   //-------------------------------------------------------------------------
   if( USE_TESTBRINGUP_WAIT )
   {
     printf("%4d: waiting\n", __LINE__);
     window.wait();
   }
   else
     delay(USE_TESTBRINGUP_DELAY);

   // For destructors, avoid visiblity
   window.setAttribute(Object::VISIBLE, FALSE);
   #ifdef HCDM
     Logger::log("%4d: ..testBringup()\n",  __LINE__);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLine
//
// Purpose-
//       Test a Line
//
//----------------------------------------------------------------------------
static void
   testLine(                        // Test a Line
     Line&             line,        // The Line
     XOffset_t         left,
     YOffset_t         top,
     XOffset_t         right,
     YOffset_t         bottom)
{
     XYOffset origin= {left, top};
     XYOffset ending= {right, bottom};
     line.line(origin, ending);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testWindow
//
// Purpose-
//       Insure Windows operate correctly
//
//----------------------------------------------------------------------------
static void
   testWindow( void )               // Verify that Windows work properly
{
   int                 deltaT;      // Delay time
   XLength_t           lX;          // Length (in Pixels)
   YLength_t           lY;          // Length (in Pixels)
   XYOffset            offset;      // XY offset
   XYLength            length;      // XY length

   int                 i;

   #ifdef HCDM
     Logger::log("\n");
     debugf("%4d: testWindow()..\n", __LINE__);
   #endif

   offset.x= offset.y= 64;
   Window              window(offset, windowLength);
   ActionChange        terminator(&window);
   terminator.setName("Terminator");

   Offset              xy20(&window);                 xy20.setName("xy20");
   Filler              fo24(&xy20, objectLength);     fo24.setName("fo24");
   Filler              fo23(&xy20, objectLength);     fo23.setName("fo23");
   Filler              fo22(&xy20, objectLength);     fo22.setName("fo22");
   Filler              fo21(&xy20, objectLength);     fo21.setName("fo21");
   Filler              fo20(&xy20, objectLength);     fo20.setName("fo20");
   ActionChange        so20(&fo20);                   so20.setName("so20");
   Filler              fo2a(&fo20);                   fo2a.setName("fo2a");
   Buffer              bo2b(&fo20, objectLength);     bo2b.setName("bo2b");
   Filler              fo2b(&bo2b, objectLength);     fo2b.setName("fo2b");

   Object              xy10(&window);                 xy10.setName("xy10");
   Filler              fo14(&xy10, objectLength);     fo14.setName("fo14");
   Filler              fo13(&xy10, objectLength);     fo13.setName("fo13");
   Filler              fo12(&xy10, objectLength);     fo12.setName("fo12");
   Filler              fo11(&xy10, objectLength);     fo11.setName("fo11");
   Filler              fo10(&xy10, objectLength);     fo10.setName("fo10");
   Line                line(&fo11);                   line.setName("Line");

   Text                text(&window);                 text.setName("text");
   Font                font(NULL);  // Default font

   line.setColor(RGB::Blue);
   fo10.setColor(RGB::Red);
   fo11.setColor(RGB::White);
   fo12.setColor(RGB::Blue);
   fo13.setColor(RGB::Green);
   fo14.setColor(RGB::Brown);

   fo20.setColor(RGB::LightRed);
   so20.setColor(RGB::Green);
   fo21.setColor(RGB::Grey);
   fo22.setColor(RGB::LightBlue);
   fo23.setColor(RGB::LightGreen);
   fo24.setColor(RGB::Yellow);
   fo2a.setColor(RGB::Yellow);
   fo2b.setColor(RGB::Yellow);
   lX= fo10.getLength().x;
   lY= fo10.getLength().y;

   offset.x= lX*1; offset.y= lY*1; fo11.setOffset(offset);
   offset.x= lX*2; offset.y= lY*2; fo12.setOffset(offset);
   offset.x= lX*3; offset.y= lY*3; fo13.setOffset(offset);
   offset.x= lX*4; offset.y= lY*4; fo14.setOffset(offset);
   length.x= lX*5; length.y= lY*5;

   offset.x= lX*1; offset.y= lY*1; fo21.setOffset(offset);
   offset.x= lX*2; offset.y= lY*2; fo22.setOffset(offset);
   offset.x= lX*3; offset.y= lY*3; fo23.setOffset(offset);
   offset.x= lX*4; offset.y= lY*4; fo24.setOffset(offset);
   length.x= lX*5; length.y= lY*5;

   offset.x= lX/2; offset.y= lY/2; xy20.setOffset(offset);
   offset.x= lX-8; offset.y= 0;    bo2b.setOffset(offset);
   length.x= 8;    length.y= 8;    bo2b.setLength(length);

   offset.x= 0;
   offset.y= lY-8;
   length.x= length.y= 8;
   fo2a.setOffset(offset);
   fo2a.setLength(length);
   #ifdef HCDM
     visit(window);
   #endif

   offset.x= windowLength.x / 2;
   offset.y= 0;
   length.x= windowLength.x - offset.x;
   length.y= 32;
   font.setColor(RGB::Red);
   text.setOffset(offset); text.setLength(length);
   text.setFont(&font);
   text.setText("Hello, text world!");
   text.setJustification(Justification::LR_CENTER + Justification::TB_CENTER);

   if( 0 ) { xy10.raise(&xy20); xy10.redraw(); }

   //-------------------------------------------------------------------------
   // Make window visible
   //-------------------------------------------------------------------------
   #ifdef HCDM
     Logger::log("\n-------------------------------------------------------\n");
   #endif
   window.setAttribute(Object::VISIBLE, TRUE);
   window.redraw();
   #ifdef HCDM
     Logger::log("\n-------------------------------------------------------\n");
   #endif

   if( 0 )                          // Debug initial window?
   {
     printf("%4d: waiting\n", __LINE__);
     debugBuffer(window);
     window.wait();
   }

   //-------------------------------------------------------------------------
   // Movement test
   //-------------------------------------------------------------------------
// debugf("%4d: HCDM Sample\n", __LINE__);
   if( USE_TESTWINDOW_MOVE )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
       debugf("%4d: testWindow() window.move() test\n", __LINE__);
     #endif

     deltaT= DEFAULT_DELAY;
     if( 1 )
       deltaT= 0;

     for(i= 0; i<4; i++)            // Clockwise
     {
       offset.x= 32; offset.y= 32; window.move(offset); delay(deltaT);
       offset.x= 64; offset.y= 32; window.move(offset); delay(deltaT);
       offset.x= 64; offset.y= 64; window.move(offset); delay(deltaT);
       offset.x= 32; offset.y= 64; window.move(offset); delay(deltaT);
     }

     for(i= 0; i<4; i++)            // Counter clockwise
     {
       offset.x= 32; offset.y= 32; window.move(offset); delay(deltaT);
       offset.x= 32; offset.y= 64; window.move(offset); delay(deltaT);
       offset.x= 64; offset.y= 64; window.move(offset); delay(deltaT);
       offset.x= 64; offset.y= 32; window.move(offset); delay(deltaT);
     }

     offset.x= offset.y= 64; window.move(offset);
     delay(MINIMUM_DELAY);
   }

   //-------------------------------------------------------------------------
   // Background change test
   //-------------------------------------------------------------------------
// debugf("%4d: HCDM Sample\n", __LINE__);
   if( USE_TESTWINDOW_COLOR )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
       debugf("%4d: testWindow() color change test\n", __LINE__);
     #endif
     deltaT= DEFAULT_DELAY;
     if( 0 )
       deltaT= 0;

     for(i= 0; i<2; i++)
     {
       fo20.setColor(RGB::Magenta);    fo20.redraw(); delay(deltaT);
       fo21.setColor(RGB::Magenta);    fo21.redraw(); delay(deltaT);
       fo22.setColor(RGB::Magenta);    fo22.redraw(); delay(deltaT);
       fo23.setColor(RGB::Magenta);    fo23.redraw(); delay(deltaT);
       fo24.setColor(RGB::Magenta);    fo24.redraw(); delay(deltaT);
       fo24.setColor(RGB::Magenta);    fo24.redraw(); delay(deltaT);

       fo20.setColor(RGB::LightRed);   fo20.redraw(); delay(deltaT);
       fo21.setColor(RGB::Grey);       fo21.redraw(); delay(deltaT);
       fo22.setColor(RGB::LightBlue);  fo22.redraw(); delay(deltaT);
       fo23.setColor(RGB::LightGreen); fo23.redraw(); delay(deltaT);
       fo24.setColor(RGB::Yellow);     fo24.redraw(); delay(deltaT);
       fo2a.setColor(RGB::Yellow);     fo2a.redraw(); delay(deltaT);
     }
   }

   //-------------------------------------------------------------------------
   // Raise/lower test
   //-------------------------------------------------------------------------
// debugf("%4d: HCDM Sample\n", __LINE__);
   if( USE_TESTWINDOW_TREE )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
       debugf("%4d: testWindow() raise/lower test\n", __LINE__);
     #endif
     deltaT= DEFAULT_DELAY;
     if( 0 )
       deltaT= 0;

     for(i= 0; i<2; i++)
     {
       xy10.lower(&xy20); xy10.redraw(); delay(deltaT);
       xy20.lower(&xy10); xy20.redraw(); delay(deltaT);
     }

     for(i= 0; i<2; i++)
     {
       xy20.raise(&xy10); xy20.redraw(); delay(deltaT);
       xy10.raise(&xy20); xy10.redraw(); delay(deltaT);
     }
   }

   //-------------------------------------------------------------------------
   // Secondary window test
   //-------------------------------------------------------------------------
   if( USE_TESTWINDOW_TWO )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
       debugf("%4d: testWindow() secondary window test\n", __LINE__);
     #endif

     Window window(windowLength);
     Filler fill(&window, objectLength); fill.setName("fill");
     fill.setColor(RGB::LightRed);
     window.setAttribute(Object::VISIBLE, TRUE);
     window.redraw();
     delay(3000);
   }

   //-------------------------------------------------------------------------
   // Line test
   //-------------------------------------------------------------------------
   if( USE_TESTWINDOW_LINE )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
       debugf("%4d: testWindow() testLine sequence\n", __LINE__);
     #endif
     deltaT= DEFAULT_DELAY;
     if( 0 )
       deltaT= 0;

     testLine(line,   0,   0, 256, 256);
     for(i=0; i<8; i++)
     {
       line.setColor(RGB::Red);
       testLine(line,  64,   0, 192, 256); delay(deltaT);
       testLine(line, 128,   0, 128, 256); delay(deltaT);
       testLine(line, 192,   0,  64, 256); delay(deltaT);
       testLine(line, 256,   0,   0, 256); delay(deltaT);

       testLine(line, 256,  64,   0, 192); delay(deltaT);
       testLine(line, 256, 128,   0, 128); delay(deltaT);
       testLine(line, 256, 192,   0,  64); delay(deltaT);
       testLine(line, 256, 256,   0,   0); delay(deltaT);

       line.setColor(RGB::Blue);
       testLine(line, 192, 256,  64,   0); delay(deltaT);
       testLine(line, 128, 256, 128,   0); delay(deltaT);
       testLine(line,  64, 256, 192,   0); delay(deltaT);
       testLine(line,   0, 256, 256,   0); delay(deltaT);

       testLine(line,   0, 192, 256,  64); delay(deltaT);
       testLine(line,   0, 128, 256, 128); delay(deltaT);
       testLine(line,   0,  64, 256, 192); delay(deltaT);
       testLine(line,   0,   0, 256, 256); delay(deltaT);
     }
   }

   //-------------------------------------------------------------------------
   // Wait test
   //-------------------------------------------------------------------------
   if( USE_TESTWINDOW_WAIT )
   {
     #ifdef HCDM
       Logger::log("\n-------------------------------------------------------\n");
     #endif
     printf("%4d: waiting\n", __LINE__);
//   xy10.raise(); xy10.redraw(); // On top for ActionChange
//   xy20.raise(); xy20.redraw(); // On top for ActionChange
     window.wait();
   }
   else
     delay(USE_TESTWINDOW_DELAY);

   // For destructors, avoid visiblity
   window.setAttribute(Object::VISIBLE, FALSE);
   #ifdef HCDM
     Logger::log("%4d: ..testWindow()\n",  __LINE__);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testWormBuffer
//
// Purpose-
//       Test the WormBuffer dohickey
//
//----------------------------------------------------------------------------
static void
   testWormBuffer( void )           // Test the WormBuffer
{
   int                 deltaT;      // Delay time
   Worm                worm[DIM_WORM]; // Worm array

   int                 i;

   #ifdef HCDM
     Logger::log("\n");
     debugf("%4d: testWormBuffer()..\n", __LINE__);
   #endif

   Window              window(windowLength);
   WormBuffer          buffer(&window);
   buffer.setName("WormBuffer");

   //-------------------------------------------------------------------------
   // Add worms to the WormBuffer
   //-------------------------------------------------------------------------
   for(i= 0; i<DIM_WORM; i++)
   {
     buffer.append(worm[i]);
     worm[i].reset(&buffer);
   }

   //-------------------------------------------------------------------------
   // Make window visible
   //-------------------------------------------------------------------------
// Logger::log("%4d: HCDM Sample\n",  __LINE__);
   window.setAttribute(Object::VISIBLE, TRUE);

   //-------------------------------------------------------------------------
   // Run the WormBuffer
   //-------------------------------------------------------------------------
   deltaT= DEFAULT_DELAY;
   if( 1 )
     deltaT= 5;
   for(i= 0; i<RUN_WORM; i++)
   {
     buffer.toggle();
     delay(deltaT);
   }

   // For destructors, avoid visiblity
   window.setAttribute(Object::VISIBLE, FALSE);
   #ifdef HCDM
     Logger::log("%4d: ..testWindow()\n",  __LINE__);
   #endif
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
     char*             argv[])      // Argument array
{
   #ifdef HCDM
     const char* name= "sample.log";
     if( argc > 1 )
       name= argv[1];
     Debug::set(new ThreadLogger(name));

     debugf("%4d: Sample Started\n", __LINE__);
//   debugSetIntensiveMode();
   #endif

   #ifdef HCDM
     Logger::log("\n");
     Logger::log("%4ld= sizeof(Object)\n", (long)sizeof(Object));
     Logger::log("%4ld= sizeof(Bounds)\n", (long)sizeof(Bounds));
     Logger::log("%4ld= sizeof(Buffer)\n", (long)sizeof(Buffer));

     Logger::log("%4ld= sizeof(Window)\n", (long)sizeof(Window));
   #endif

   try {
     // Basic test group -----------------------------------------------------
     if( USE_TESTOBJECT ) testObject();
     if( USE_TESTBRINGUP ) testBringup();
     if( USE_TESTWINDOW ) testWindow();
     if( USE_WORMWINDOW ) testWormBuffer();
     if( 0 ) delay(30000);

   } catch(const char* e) {
     msgerr("%4d: Exception const char*(%s) !!NOT EXPECTED!!\n", __LINE__, e);

   } catch(std::exception& e) {
     msgerr("%4d: Exception exception&(%s) !!NOT EXPECTED!!\n", __LINE__, e.what());

   } catch( ... ) {
     msgerr("%4d: Exception ... !!NOT EXPECTED!! ... UNKNOWN\n", __LINE__);
   }

   printf("%4d: Sample errorCount(%d)\n", __LINE__, errorCount);
   return errorCount;
}

