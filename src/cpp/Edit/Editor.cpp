//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Editor.cpp
//
// Purpose-
//       Editor object methods.
//
// Last change date-
//       2018/01/01 (Version 2, Release 1) - Mouse wheel support
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/Clock.h>
#include <com/istring.h>
#include <com/KeyCode.h>
#include <com/Parser.h>

#include "Active.h"
#include "EdDraw.h"
#include "EdHand.h"
#include "EdLine.h"
#include "EdMark.h"
#include "EdPool.h"
#include "EdRing.h"
#include "EdView.h"
#include "Status.h"

#include "Editor.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef DEFER_DISPLAY
#define DEFER_DISPLAY TRUE          // Do we defer display? (combine keypress)
#endif

// DEC_POL_DELAY <= MIN_POLL_DELAY <= MAX_POLL_DELAY
#define DEC_POLL_DELAY 6            // Polling decrement adjustment
#define INC_POLL_DELAY 9            // Polling increment adjustment
#define MIN_POLL_DELAY 10           // Minimum polling delay (milliseconds)
#define MAX_POLL_DELAY 75           // Maximum polling delay (milliseconds)

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>            // Must follow define/undef HCDM

#define ENUM_RING(ring) \
   ring= (EdRing*)ringList.getHead(); \
   ring != NULL; \
   ring= (EdRing*)ring->getNext()

#define ENUM_VIEW(view) \
   view= (EdView*)viewList.getHead(); \
   view != NULL;                      \
   view= (EdView*)view->getNext()

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const char*     const deadkey= "Invalid key";

//----------------------------------------------------------------------------
// ALT key transforms
//----------------------------------------------------------------------------
#define KALT_A KeyCode::ALT_A
#define KALT_B KeyCode::ALT_B
#define KALT_C KeyCode::ALT_C
#define KALT_D KeyCode::ALT_D
#define KALT_E KeyCode::ALT_E
#define KALT_F KeyCode::ALT_F
#define KALT_G KeyCode::ALT_G
#define KALT_H KeyCode::ALT_H
#define KALT_I KeyCode::ALT_I
#define KALT_J KeyCode::ALT_J
#define KALT_K KeyCode::ALT_K
#define KALT_L KeyCode::ALT_L
#define KALT_M KeyCode::ALT_M
#define KALT_N KeyCode::ALT_N
#define KALT_O KeyCode::ALT_O
#define KALT_P KeyCode::ALT_P
#define KALT_Q KeyCode::ALT_Q
#define KALT_R KeyCode::ALT_R
#define KALT_S KeyCode::ALT_S
#define KALT_T KeyCode::ALT_T
#define KALT_U KeyCode::ALT_U
#define KALT_V KeyCode::ALT_V
#define KALT_W KeyCode::ALT_W
#define KALT_X KeyCode::ALT_X
#define KALT_Y KeyCode::ALT_Y
#define KALT_Z KeyCode::ALT_Z

static const int       altKeys[256]=
{  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007  // Identity
,  0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F
,  0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017
,  0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F
,  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027
,  0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F
,  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037
,  0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F
,  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047
,  0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F
,  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057
,  0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F
,  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067
,  0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F
,  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077
,  0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F

,  0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087  // Transform
,  0x0088, 0x0089, 0x008A, 0x008B, 0x008C, 0x008D, 0x008E, 0x008F
,  0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097
,  0x0098, 0x0099, 0x009A, 0x009B, 0x009C, 0x009D, 0x009E, 0x009F
,  0x00A0, 0x00A1, 0x00A2, 0x00A3, 0x00A4, 0x00A5, 0x00A6, 0x00A7
,  0x00A8, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF
,  0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x00B6, 0x00B7
,  0x00B8, 0x00B9, 0x00BA, 0x00BB, 0x00BC, 0x00BD, 0x00BE, 0x00BF
,  0x00C0, KALT_A, KALT_B, KALT_C, KALT_D, KALT_E, KALT_F, KALT_G
,  KALT_H, KALT_I, KALT_J, KALT_K, KALT_L, KALT_M, KALT_N, KALT_O
,  KALT_P, KALT_Q, KALT_R, KALT_S, KALT_T, KALT_U, KALT_V, KALT_W
,  KALT_X, KALT_Y, KALT_Z, 0x00DB, 0x00DC, 0x00DD, 0x00DE, 0x00DF
,  0x00E0, KALT_A, KALT_B, KALT_C, KALT_D, KALT_E, KALT_F, KALT_G
,  KALT_H, KALT_I, KALT_J, KALT_K, KALT_L, KALT_M, KALT_N, KALT_O
,  KALT_P, KALT_Q, KALT_R, KALT_S, KALT_T, KALT_U, KALT_V, KALT_W
,  KALT_X, KALT_Y, KALT_Z, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       nameToHist
//
// Purpose-
//       Copy the file name to the history line.
//
// Notes-
//       This function seems too obscure to make it visible but it should
//       be in a function because the fileName string is temporary.
//
//----------------------------------------------------------------------------
static void
   nameToHist(                      // Copy file name to history View
     Editor*           editor)      // Editor
{
   EdRing*             dataRing= editor->dataActive->getRing();
   char                string[FILENAME_MAX+1+FILENAME_MAX+1];

   string[0]= '\0';
   if( dataRing != NULL )
     FileName::concat(string, sizeof(string), dataRing->pathName, dataRing->fileName);
   editor->histActive->replaceLine(string);
   editor->histView->defer(EdView::RESHOW_BUF);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Find next non-whitespace in string
//
//----------------------------------------------------------------------------
static const char*                  // Next non-whitespace character
   skipBlank(                       // Find next non-whitespace character
     const char*       text)        // In this string
{
   Parser parser(text);
   return parser.skipSpace();
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   Editor::check( void ) const      // Debugging check
{
#ifdef HCDM
   EdLine*             line;
   EdRing*             ring;
   EdView*             view;

   for(ENUM_RING(ring))
     ring->check();
   histRing->check();
   utilRing->check();

   for(ENUM_VIEW(view))
   {
     view->synch();
     view->check();
   }

   mark->check();
   status->check();

   // Cross-object coherency checks: dataActive vs. dataView
   line= dataActive->getLine();
   ring= dataActive->getRing();
   view= dataView;

   if( view->getRing() != ring )
   {
     tracef("%4d Editor(%p)::check, ring: view(%p) active(%p)\n", __LINE__,
            this, view->getRing(), ring);
     throw "InternalLogicError";
   }

   if( view->getLine() != line )
   {
     tracef("%4d Editor(%p)::check, line: view(%p) active(%p)\n", __LINE__,
            this, view->getLine(), line);
     throw "InternalLogicError";
   }
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Editor::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   EdRing*             ring;
   EdView*             view;

   tracef("%4d Editor(%p)::debug(%s)\n"
          "  workView(%p) dataView(%p) histView(%p)\n"
          "  workRing(%p) utilRing(%p) histRing(%p)\n"
          , __LINE__, this, message
          , workView, dataView, histView
          , workView->getActive()->getRing(), utilRing, histRing);

   for(ENUM_VIEW(view))
     view->debug("Editor");

   for(ENUM_RING(ring))
     ring->debug("Editor");
   histRing->debug("Editor");
   utilRing->debug("Editor");

   dataActive->debug("Editor");
   histActive->debug("Editor");

   mark->debug("Editor");
   status->debug("Editor");
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::~Editor
//
// Purpose-
//       Editor constructor
//
//----------------------------------------------------------------------------
   Editor::~Editor( void )          // Destructor
{
   EdRing*             ring;        // Working Ring
   EdView*             view;        // Working View

   #ifdef SCDM
     tracef("%4d Editor(%p)::~Editor()\n", __LINE__);
   #endif

   check();

   // Delete all Views
   for(;;)
   {
     view= (EdView*)viewList.remq();
     if( view == NULL )
       break;

     delete view;
   }

   // Delete all Rings
   for(;;)
   {
     ring= (EdRing*)ringList.remq();
     if( ring == NULL )
       break;

     delete ring;
   }
   delete histRing;
   delete utilRing;

   // Delete all Actives
   delete dataActive;
   delete histActive;
   delete workActive;

   // Delete helper Objects
   delete handler;
   delete mark;
   delete status;
   delete terminal;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::Editor
//
// Purpose-
//       Editor constructor
//
//----------------------------------------------------------------------------
   Editor::Editor( void )           // Constructor
:  EdDraw(new Terminal())
,  dataActive(NULL)
,  histActive(NULL)
,  workActive(NULL)
,  ringList()
,  histRing(NULL)
,  utilRing(NULL)
,  viewList()
,  dataView(NULL)
,  histView(NULL)
,  workView(NULL)
,  viewCount(0)
,  handler(NULL)
,  mark(NULL)
,  status(NULL)
,  changeLength(0)
,  locateLength(0)
,  online(FALSE)
,  marginLeft(0)
,  marginRight(78)
,  tabUsed(0)
{
   EdLine*             line;        // Working line

   #ifdef SCDM
     tracef("%4d Editor(%p)::Editor()\n", __LINE__, this);
   #endif

   // Internal Editor objects
   handler= new EdHand(terminal);
   terminal->setHandler(handler);
   mark= new EdMark(this);
   changeString[0]= '\0';
   locateString[0]= '\0';

   // Create the Active objects
   dataActive= new Active(MAX_ACTIVE);
   histActive= new Active(MAX_ACTIVE);
   workActive= new Active(MAX_ACTIVE);

   // Create the history ring
   histRing= new EdRing();
   histRing->mode= EdRing::FM_UNIX;
   histRing->type= EdRing::FT_PROTECTED;
   strcpy(histRing->fileName, "**History**");
   line= (EdLine*)histRing->lineList.getHead();
   histRing->curLine=
   histRing->firstLine= histRing->insertLine(line);
   if( histRing->curLine == NULL )
     throw "NoStorageException";

   // Create the utility ring
   utilRing= new EdRing();
   utilRing->mode= EdRing::FM_UNIX;
   utilRing->type= EdRing::FT_PROTECTED;
   strcpy(utilRing->fileName, "**Buffer**");

   // Create the Views
   workView= histView= new EdView(this, histActive);
   dataView= new EdView(this, dataActive);
   viewList.fifo(histView);
   viewList.fifo(dataView);
   status= new Status(this);
   viewCount= 1;

   // Initialization
   terminal->setAttribute(VGAColor::Grey, VGAColor::Black);
   terminal->clearScreen();         // Clear the screen
   dataActive->fetch(utilRing, dataView->activate(utilRing));
   histActive->fetch(histRing, histView->activate(histRing));
   memset(changeString, 0, sizeof(changeString));
   memset(locateString, 0, sizeof(locateString));
   memset(tabStop, 0, sizeof(tabStop));
   resize();
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::activate
//
// Purpose-
//       Activate the associated EdLine
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::activate(                // Activate Line
     EdLine*           edLine)      // Using this EdLine
{
   IFHCDM(
     tracef("%4d Editor(%p)::activate(EdLine %p)\n", __LINE__, this, edLine);
   )

   return dataActive->fetch(dataView->activate(edLine));
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::activate
//
// Purpose-
//       Activate the associated EdRing
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::activate(                // Activate ring
     EdRing*           edRing)      // Using this EdRing
{
   IFHCDM(
     tracef("%4d Editor(%p)::activate(EdRing %p)\n", __LINE__, this, edRing);
   )

   commit();
   dataView->synchStore();
   return dataActive->fetch(edRing, dataView->synchFetch(edRing));
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::change
//
// Purpose-
//       Change string
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::change( void )           // Locate, change
{
   const char*         result;      // Resultant
   unsigned            col;         // Working column number
   EdLine*             line;        // Working Line
   EdRing*             ring;        // Working Ring

   result= locate(TRUE);            // Locate (same OK)
   if( result == NULL )
   {
     col= dataView->getColumn();
     result= dataActive->replaceString(col, locateLength, changeString);
   }

   if( result == NULL )
   {
     line= dataActive->getLine();
     ring= dataActive->getRing();
     result= viewChange(ring, line);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::commit
//
// Purpose-
//       Commit data updates.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::commit( void )           // Commit data updates
{
   const char*         result;      // Resultant

   IFHCDM( tracef("%4d Editor(%p)::commit()\n", __LINE__, this); )

   result= dataActive->store();
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::defer
//
// Purpose-
//       Deferred redraw for all views.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Editor::defer(                   // Deferred reshow
     ReshowType        type)        // Reshow type
{
   EdView*             view;        // Working View

   IFHCDM( tracef("%4d Editor(%p)::defer(%d)\n", __LINE__, this, type); )

// TODO: Remove or document...
// if( type == RESHOW_ALL )
//   dataActive->fetch(dataActive->getLine());

   for(ENUM_VIEW(view))
     view->defer(type);
   return status->defer(type);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::display
//
// Purpose-
//       Physical display of deferred items.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Editor::display( void )          // Display
{
   EdView*             view;        // Working View

   IFHCDM( tracef("%4d Editor(%p)::display()\n", __LINE__, this); )

   for(ENUM_VIEW(view))
     view->display();

   return status->display();        // Must be last
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::focus
//
// Purpose-
//       Set focus.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Editor::focus(                   // Set focus
     EdView*           edView)      // Using this View
{
   EdLine*             line;        // Working Line

   IFHCDM( tracef("%4d Editor(%p)::focus(%p)\n", __LINE__, this, edView); )

   if( workView == histView
       || edView == histView )
   {
     line= (EdLine*)histRing->lineList.getHead()->getNext();
     histActive->fetch(histView->activate(line));
     histView->column(0);
     histView->defer(RESHOW_BUF);
   }

   workView= edView;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::histInsert
//
// Purpose-
//       Insert a line into the history ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::histInsert( void )       // Insert a history line
{
   const char*         result;      // Resultant
   EdLine*             line;        // Working line
   const char*         text;        // Working text pointer

   IFHCDM(
     tracef("%4d Editor(%p)::histInsert(%s)\n", __LINE__, this,
            histActive->getText());
   )

   //-------------------------------------------------------------------------
   // Exit if comment
   //-------------------------------------------------------------------------
   histActive->strip();             // Strip leading and trailing blanks
   text= histActive->getText();
   if( *text == '\0' )              // If empty command
     return NULL;                   // Return, done

   //-------------------------------------------------------------------------
   // Look for duplicate line
   //-------------------------------------------------------------------------
   line= (EdLine*)histRing->lineList.getHead();
   while( line != NULL )
   {
     if( !line->ctrl.readonly )
     {
       if( strcmp(text, line->getText()) == 0 )
       {
         histRing->lineList.remove(line, line);
         histRing->lineList.insert(histRing->lineList.getTail()->getPrev(),
                                   line, line);
         histRing->resetCache();

         histActive->fetch(histRing, line);
         return NULL;
       }
     }

     line= (EdLine*)line->getNext();
   }

   //-------------------------------------------------------------------------
   // Add command to end of history ring
   //-------------------------------------------------------------------------
   line= (EdLine*)histRing->lineList.getTail()->getPrev();
   line= histRing->insertLine(line);
   if( line == NULL )
     return "No storage";

   result= histActive->setLine(histRing, line);
   if( result == NULL )
     result= histActive->store();

   if( result != NULL )
   {
     histRing->removeLine(line);
     return result;
   }

   return histActive->fetch(histView->activate(line));
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::insertLine
//
// Purpose-
//       Account for lines inserted into a ring.
//
//----------------------------------------------------------------------------
#if 0
const char*                         // Return message (NULL OK)
   Editor::insertLine(              // Insert lines into Ring
     EdRing*           edRing,      // The Ring containing the lines
     EdLine*           head,        // The first line to insert
     EdLine*           tail)        // The final line to insert
{
   EdView*             view;        // Working View

   IFHCDM(
     tracef("%4d Editor(%p)::insertLine(%p,%p,%p)\n", __LINE__, this,
            edRing, head, tail);
   )

   for(ENUM_VIEW(view))
     view->viewChange(edRing, head, tail);

   return NULL;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Editor::insertLine
//
// Purpose-
//       Do all the work required to insert a Line into the Editor.
//
//----------------------------------------------------------------------------
EdLine*                             // The new Line
   Editor::insertLine( void )       // Insert a new, empty Line
{
   EdLine*             result;      // The newly inserted Line
   EdLine*             line= dataActive->getLine(); // Working Line
   EdRing*             ring= dataActive->getRing(); // Working Ring
   EdView*             view= dataView; // Working View

   IFHCDM( tracef("%4d Editor(%p)::insertLine()\n", __LINE__, this); )

   commit();
   result= ring->insertLine(line);
   if( result == NULL )
   {
     if( line->getNext() == NULL )
       status->warning("Protected");
     else
       status->warning("No storage!");
   }
   else
   {
     view->column(0);
     dataActive->fetch(view->moveDown());
     viewChange(ring, result, result);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::insertRing
//
// Purpose-
//       Do all the work required to insert a set of Rings into the Editor.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::insertRing(              // Insert (or activate) Ring
     const char*       fileName)    // The associated file name
{
   const char*         result;      // Resultant

   EdRing*             curRing;     // Current Ring
   char                currentPath[FILENAME_MAX+1]; // Path (only)
   char                currentName[FILENAME_MAX+1]; // Name (only)
   char                currentFull[FILENAME_MAX+1]; // Path/Name
   EdRing*             ring;        // Working Ring
   const char*         workingName; // Working file name (only)

   IFHCDM(
     tracef("%4d Editor(%p)::insertRing(%s)\n", __LINE__, this, fileName);
   )

   commit();

   workingName= FileName::resolve(currentFull, fileName);
   if( workingName != NULL )
   {
     warning("Error(%s) in(%s)", workingName, fileName);
     return "Invalid name";
   }

   FileInfo fileInfo(currentFull);
   if( fileInfo.isPath() )
   {
     warning("Folder(%s)", currentFull);
     return "Folder";
   }

   //-------------------------------------------------------------------------
   // (Note: Both currentPath and currentName are subsets of currentFull)
   FileName::getPathOnly(currentPath, currentFull);
   FileName::getNamePart(currentName, currentFull);

   //-------------------------------------------------------------------------
   // For existing files, this sequence updates case in workingName
   FileList fileList(currentPath, currentName);
   workingName= fileList.getCurrent();
   if( workingName == NULL )
     workingName= currentName;

   //-------------------------------------------------------------------------
   // Extract filenames (workingName may contain wildcard characters)
   //-------------------------------------------------------------------------
   while(workingName != NULL)
   {
     //-----------------------------------------------------------------------
     // Skip if path
     fileInfo.reset(currentPath, workingName);
     if( fileInfo.isPath() )
     {
       workingName= fileList.getNext();
       continue;
     }

     //-----------------------------------------------------------------------
     // Skip (but activate) if file is already loaded
     FileName::concat(currentFull, currentPath, workingName);
     for(ring= (EdRing*)ringList.getHead();
         ring != NULL;
         ring= (EdRing*)ring->getNext())
     {
       if( ring->contains(currentFull) )
       {
         activate(ring);
         break;
       }
     }

     if( ring == NULL )
     {
       //---------------------------------------------------------------------
       // The file is not loaded. Create the ring and load it.
       try {
         ring= new EdRing(workingName);
       } catch(...) {
         return warning("No storage");
       }

       curRing= dataView->getActive()->getRing();
       if( curRing == utilRing )
         ringList.fifo(ring);
       else
         ringList.insert(curRing, ring, ring);
       activate(ring);
       warning("Loading");
       defer(RESHOW_ALL);
       display();
       status->defer(RESHOW_CSR);
       dataView->defer(RESHOW_ALL);

       //---------------------------------------------------------------------
       // Load the file. Note that "Nonexistent" occurs only without wildchars
       result= warning(ring->read(currentFull));
       if( result != NULL )
         break;
     }

     workingName= fileList.getNext();
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::lineJoin
//
// Purpose-
//       Join lines.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::lineJoin( void )         // Join lines
{
   const char*         result;      // Resultant

   EdLine*             newLine;     // Working line
   EdLine*             oldLine;     // Working line
   EdRing*             oldRing;     // Working ring

   IFHCDM( tracef("%4d Editor(%p)::lineJoin()\n", __LINE__, this); )

   commit();
   oldRing= dataActive->getRing();
   oldLine= dataActive->getLine();
   newLine= (EdLine*)oldLine->getNext();
   result= NULL;
   if( oldLine->ctrl.readonly
       || newLine == NULL
       || newLine->ctrl.readonly )
     result= "Protected";

   if( result == NULL )
     result= dataActive->shrink();

   if( result == NULL )
     result= dataActive->expand(dataActive->getUsed());

   if( result == NULL )
     result= dataActive->appendString(skipBlank(newLine->getText()));

   if( result == NULL )
   {
     result= dataActive->store();
     if( result == NULL )
       removeLine(oldRing, newLine, newLine);

     viewChange(oldRing, oldLine, newLine); // Update views
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::lineSplit
//
// Purpose-
//       Split lines.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::lineSplit( void )        // Split lines
{
   const char*         result;      // Resultant

   unsigned            length;      // Working length
   EdLine*             newLine;     // Working line
   EdLine*             oldLine;     // Working line
   EdRing*             oldRing;     // Working ring
   const char*         string;      // Working string

   unsigned            column= dataView->getColumn();

   IFHCDM( tracef("%4d Editor(%p)::lineSplit()\n", __LINE__, this); )

   commit();
   result= NULL;
   oldRing= dataActive->getRing();
   oldLine= dataActive->getLine();

   // Allocate a new line
   newLine= oldRing->insertLine(oldLine);
   if( newLine == NULL )
   {
     result= "Protected";
     if( oldLine->getNext() != NULL )
       result= "No storage!";
   }
   else
   {
     // Fetch the new line
     dataActive->fetch(oldRing, newLine);

     // Format the new line
     string= oldLine->getText();
     for(length= 0; string[length] == ' '; length++)
       ;

     if( length > 0 )
       dataActive->expand(length-1);// Add leading blanks

     if( oldLine->getSize() > column )
       dataActive->appendString(oldLine->getText()+column);

     result= dataActive->store();
     if( result == NULL )
     {
       dataActive->fetch(oldRing, oldLine);
       dataActive->clear(column);
       result= dataActive->store();
     }

     viewChange(oldRing, oldLine, newLine); // Update views
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::locate
//
// Purpose-
//       Locate string in data ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::locate(                  // Locate string
     int               change)      // Non-zero to locate for change
{
   const char*         C;           // -> Character
   unsigned            col=    dataView->getColumn();
   unsigned            length= locateLength;
   EdLine*             line=   dataActive->getLine();

   commit();                        // Commit the active line

   //-------------------------------------------------------------------------
   // Search the active line
   //-------------------------------------------------------------------------
   if( change == 0 )                // If locate next (default)
     col++;                         // Skip current column

   if( col < line->getSize() && line->ctrl.readonly == FALSE )
   {
     C= stristr(line->text+col, locateString);
     if( C != NULL )
     {
       col= C - line->text;
       dataView->column(col, col+length);
       return focus(dataView);
     }
   }

   //-------------------------------------------------------------------------
   // Search remainder of file
   //-------------------------------------------------------------------------
   for(;;)
   {
     line= (EdLine*)line->getNext();
     if( line == NULL )
       return "Not found";

     if( line->text != NULL && line->ctrl.readonly == FALSE )
     {
       C= stristr(line->text, locateString);
       if( C != NULL )
         break;
     }
   }

   col= C - line->text;
   dataActive->fetch(dataView->activate(line));
   dataView->column(col, col+length);
   return focus(dataView);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::removeLine
//
// Purpose-
//       Remove lines from a Ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::removeLine(              // Remove lines from Ring
     EdRing*           edRing,      // The Ring containing the lines
     EdLine*           head,        // The first line to remove
     EdLine*           tail)        // The final line to remove
{
   EdView*             view;        // Working View

   IFHCDM(
     tracef("%4d Editor(%p)::removeLine(%p,%p,%p)\n", __LINE__, this,
            edRing, head, tail);
   )

   if( head->ctrl.readonly || tail->ctrl.readonly )
     return "Protected";

   if( dataActive->getLine()->between(head,tail) )
   {
     dataActive->reset();
     dataActive->fetch((EdLine*)tail->getNext());
   }

   if( edRing->firstLine->between(head,tail) )
     edRing->firstLine= (EdLine*)tail->getNext();

   if( edRing->curLine->between(head,tail) )
     edRing->curLine= (EdLine*)tail->getNext();

   for(ENUM_VIEW(view))
     view->viewChange(edRing, head, tail);

   mark->removeLine(edRing, head, tail);
   edRing->removeUndo(head, tail);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::removeLine
//
// Purpose-
//       Do all the work required to remove the dataActive Line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::removeLine( void )       // Remove the dataActive Line
{
   EdLine*             line= dataActive->getLine(); // Working Line
   EdRing*             ring= dataActive->getRing(); // Working Ring

   IFHCDM( tracef("%4d Editor(%p)::removeLine()\n", __LINE__, this); )

   return removeLine(ring, line, line);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::removeRing
//
// Purpose-
//       Do all the work required to remove a Ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::removeRing(              // Remove a Ring
     EdRing*           edRing)      // The Ring to remove
{
   EdRing*             ring;        // Working ring
   EdView*             view;        // Working view

   IFHCDM( tracef("%4d Editor(%p)::removeRing(%p)\n", __LINE__, this, edRing); )

   if( edRing->type == EdRing::FT_PROTECTED )
     return "Protected";

   ring= (EdRing*)edRing->getPrev();
   if( ring == NULL )
   {
     ring= (EdRing*)ringList.getTail();
     if( ring == edRing )
     {
       online= FALSE;
       return NULL;
     }
   }

   // Remove Ring from all Views
   for(ENUM_VIEW(view))
   {
     if( view->getRing() == edRing )
       view->activate(ring);
   }

   // Synchronize dataActive with dataView
   dataActive->fetch(dataView->getRing(), dataView->getLine());

   // Delete Ring from Mark
   mark->removeRing(edRing);

   // Remove Ring from Editor
   ringList.remove(edRing, edRing);

   // Delete the Ring
   delete edRing;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::resize
//
// Purpose-
//       Handle resize event.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::resize(                  // Handle resize event
     unsigned          cols,        // Number of columns
     unsigned          rows)        // Number of rows
{
   const char*         result;      // Resultant
   const char*         string;      // Working string
   EdView*             view;

   IFHCDM(
     tracef("%4d Editor(%p)::resize(%d,%d)\n", __LINE__, this, cols, rows);
   )

   result= NULL;
   for(ENUM_VIEW(view))
   {
     string= view->resize(cols, rows);
     if( string != NULL )
       result= string;
   }

   string= status->resize(cols, rows);
   if( string != NULL )
     result= string;

   return result;
}

void
   Editor::resize( void )           // Handle resize event
{
   unsigned            cols;        // Number of columns
   unsigned            rows;        // Number of columns
   const char*         string;      // Working string

   cols= terminal->getXSize();
   rows= terminal->getYSize();
   string= resize(cols, rows);
   if( string != NULL )
   {
     online= FALSE;
     warning(string);
   }
   defer(RESHOW_ALL);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::safeExit
//
// Purpose-
//       Safely remove ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::safeExit(                // Safely remove a ring
     EdRing*           edRing)      // The ring to remove
{
   int                 rc;

   if( edRing->type == EdRing::FT_PROTECTED )
     return activate((EdRing*)ringList.getHead());

   if( edRing->changed == TRUE
       &&!edRing->damaged )
   {
     display();
     rc= status->message(MSG_REPLY, "Throw away changes?");
     if( rc != 'y' && rc != 'Y' )
       return "Kept";
   }

   return removeRing(edRing);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::safeSave
//
// Purpose-
//       Safely write ring.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::safeSave(                // Safely write a ring
     EdRing*           edRing)      // The ring to remove
{
   const char*         string;      // Message string

   int                 rc;

   if( edRing->damaged )
     string= "File Damaged. Confirm save?";
   else
     string= "Confirm save?";
   rc= status->message(MSG_REPLY, string);
   if( rc != 'y' && rc != 'Y' )
     return "Kept";

   return edRing->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::tabLeft
//
// Purpose-
//       Find next tab stop.
//
//----------------------------------------------------------------------------
unsigned                            // Next tab stop
   Editor::tabLeft(                 // Tab left
     unsigned          column)      // From this column
{
   int                 i;

   if( tabUsed == 0 || column > (tabStop[tabUsed-1]+8) )
   {
     column--;
     column &= ~(7);
     return column;
   }

   for(i= tabUsed; i>0; i--)
   {
     if( tabStop[i-1] < column )
       return tabStop[i-1];
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::tabRight
//
// Purpose-
//       Find next tab stop.
//
//----------------------------------------------------------------------------
unsigned                            // Next tab stop
   Editor::tabRight(                // Tab right
     unsigned          column)      // From this column
{
   int                 i;

   for(i= 0; i<tabUsed; i++)
   {
     if( tabStop[i] > column )
       break;
   }

   column= ((column+8)/8*8);        // Next default tab stop
   if( i < tabUsed )
     column= tabStop[i];

   return column;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::undo
//
// Purpose-
//       Undo last delete.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Editor::undo( void )             // Undo last delete
{
   const char*         result;      // Resultant

   EdLine*             head;        // First undo line
   EdLine*             line;        // Working line
   EdRing*             ring= dataView->getRing(); // Current Ring
   EdLine*             tail;        // Final undo line

   if( dataActive->getState() != Active::FSM_RESET )
   {
     line= dataActive->getLine();
     dataActive->fetch(line);
     return viewChange(ring, line, line);
   }

   result= ring->undo(head,tail);
   if( result == NULL )
     result= viewChange(ring, head, tail);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::viewChange
//
// Purpose-
//       Update views for changed lines.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Editor::viewChange(              // Update view after change
     const EdRing*     edRing,      // For this ring
     const EdLine*     edLine)      // For this line
{
   EdView*             view;        // Working View

   IFHCDM(
     tracef("%4d Editor(%p)::viewChange(%p,%p)\n", __LINE__, this,
            edRing, edLine);
   )

   for(ENUM_VIEW(view))
     view->viewChange(edRing, edLine);

   return NULL;
}

const char*                         // Return message (NULL)
   Editor::viewChange(              // Update view after change
     const EdRing*     edRing,      // For this ring
     const EdLine*     edLine,      // For this line
     const unsigned    column)      // For this column
{
   EdView*             view;        // Working View

   IFHCDM(
     tracef("%4d Editor(%p)::viewChange(%p,%p,%d)\n", __LINE__, this,
            edRing, edLine, column);
   )

   for(ENUM_VIEW(view))
     view->viewChange(edRing, edLine, column);

   return NULL;
}

const char*                         // Return message (NULL)
   Editor::viewChange(              // Update views after change
     EdRing*           edRing,      // For this ring
     EdLine*           head,        // First change line
     EdLine*           tail)        // Last change line
{
   EdView*             view;        // Working View

   IFHCDM(
     tracef("%4d Editor(%p)::viewChange(%p,%p,%p)\n", __LINE__, this,
            edRing, head, tail);
   )

   for(ENUM_VIEW(view))
     view->viewChange(edRing, head, tail);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::warning
//
// Purpose-
//       Display warning
//
//----------------------------------------------------------------------------
const char*                         // (The input format string)
   Editor::warning(                 // Display warning message
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer
   char                string[2048];// Intermediate string

   if( fmt != NULL )
   {
     va_start(argptr, fmt);         // Initialize va_ functions
     int L= vsnprintf(string, sizeof(string)-1, fmt, argptr); // Create string in buffer
     va_end(argptr);                // Close va_ functions
     if( L >= sizeof(string) )
       string[sizeof(string) - 1]= '\0';

     status->message(MSG_WARN, string);
   }

   return fmt;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::run
//
// Purpose-
//       Run the editor in interactive mode.
//
//----------------------------------------------------------------------------
void
   Editor::run( void )              // Operate the Editor
{
   int                 incode;      // Keyboard input character
   Active*             active;      // Working Active
   unsigned            column;      // Working column
   int                 delay= MIN_POLL_DELAY; // Current polling delay
   EdLine*             newLine;     // Working Line
   EdRing*             newRing;     // Working Ring
   EdLine*             oldLine;     // Working Line
   EdRing*             oldRing;     // Working Ring
   const double        reset= MAX_POLL_DELAY/500.0;
   const char*         string;      // -> String

   IFHCDM( tracef("%4d Editor(%p)::run()\n", __LINE__, this); )
   IFHCDM( debug("run"); )

   //-------------------------------------------------------------------------
   // Insure that there is something to edit
   //-------------------------------------------------------------------------
   oldRing= (EdRing*)ringList.getHead(); // Get first ring
   if( oldRing == NULL )            // If no ring
   {
     status->message(MSG_INFO, "Nothing to edit!");
     status->defer(RESHOW_CSR);
     display();
     terminal->rd();
     return;
   }
   dataActive->fetch(oldRing, dataView->activate(oldRing));

   //-------------------------------------------------------------------------
   // Process keystrokes
   //-------------------------------------------------------------------------
   online= TRUE;
   while( online )                  // Process keystrokes
   {
     //-----------------------------------------------------------------------
     // Get the current status
     //-----------------------------------------------------------------------
     active= workView->getActive();
     column= workView->getColumn();
     oldRing= active->getRing();
     oldLine= active->getLine();

     //-----------------------------------------------------------------------
     // Process deferred reshow
     //-----------------------------------------------------------------------
     int poll= terminal->poll((unsigned)delay);
     if( !(DEFER_DISPLAY) || poll == FALSE ) // If no keypress
     {
       if( terminal->ifInsertKey() )
         terminal->setCursorMode(Terminal::INSERT);
       else
         terminal->setCursorMode(Terminal::REPLACE);

       display();                   // Display
       check();                     // Internal consistency check
     }
     workView->defer(RESHOW_CSR);   // Position the cursor

     if( poll )                     // If character present
     {
       delay -= DEC_POLL_DELAY;
       if( delay < MIN_POLL_DELAY )
         delay= MIN_POLL_DELAY;
     } else {                       // No character present
       delay += INC_POLL_DELAY;
       if( delay > MAX_POLL_DELAY )
         delay= MAX_POLL_DELAY;
     }

     // Debugging trace hook
     // Debug::get()->logf("delay(%d) poll(%d)\n", delay, poll);

     //-----------------------------------------------------------------------
     // Read the next character
     //-----------------------------------------------------------------------
     double now= Clock::current();  // The current time
     incode= terminal->rd();        // Get next input from keyboard
     if( (Clock::current() - now) > reset )
       delay= MIN_POLL_DELAY;       // Become responsive after delay

     IFHCDM(
       tracef("----------------------------------------------------------\n");
       tracef("%4d Editor: Keystroke(%3d,%3d='%c') Work(%p) Ring(%p) '%s'\n",
              __LINE__, incode < 256 ? altKeys[incode] : incode,
              incode, (incode < ' ' || incode > '~') ? '~' : incode,
              workView, oldRing, oldRing->fileName);
     )

     if( incode < 256 )
       incode= altKeys[incode];
     if( incode < 256 )             // If not extended code
     {
       switch (incode)              // Standard keystroke
       {
         case '\b':                 // Backspace
           if( column > 0 )
             column--;
           warning(active->removeChar(column));
           viewChange(oldRing, oldLine);
           workView->moveLeft();
           break;

         case '\t':                 // Tab
           warning(workView->column(tabRight(column)));
           break;

         case '\n':                 // Ctrl-enter
           insertLine();
           break;

         case '\r':                 // Enter
           commit();                // Commands require commit first
           workView->column(0);
           if( workView == histView )
           {
             histInsert();
             string= execute();
             if( warning(string) == NULL )
             {
               newLine= (EdLine*)histRing->lineList.getHead()->getNext();
               histActive->fetch(histView->activate(newLine));
               histView->defer(RESHOW_BUF);
             }
             break;
           }

           warning(active->fetch(workView->moveDown()));
           break;

         case KeyCode::ESC:              // Escape
           if( workView != histView )
             focus(histView);
           else
             focus(dataView);
           break;

         case 127:                  // Ctrl-backspace
           warning(removeLine());
           break;

         default:                   // Any other standard keystoke
           if( !isprint(incode) )   // If not printable character
           {
             warning(deadkey);
             break;
           }

           if( terminal->ifInsertKey() )
           {
             warning(active->insertChar(column, incode));
             viewChange(oldRing, oldLine);
           }
           else
           {
             warning(active->replaceChar(column, incode));
             viewChange(oldRing, oldLine, column);
           }
           workView->moveRight();
           break;
       }                            // Standard keystroke
     }
     else                           // Extended codes
     {
       switch (incode)              // Extended code input key
       {
         case KeyCode::Insert:      // Insert
           break;

         case KeyCode::Delete:      // Delete
           warning(active->removeChar(column));
           viewChange(oldRing, oldLine);
           break;

         case KeyCode::Home:        // Home
           warning(workView->column(0));
           break;

         case KeyCode::End:         // End
           warning(workView->column(-1, active->getUsed()));
           break;

         case KeyCode::PageUp:      // Page up
           commit();
           warning(dataActive->fetch(dataView->screenUp()));
           break;

         case KeyCode::PageDown:    // Page down
           commit();
           warning(dataActive->fetch(dataView->screenDown()));
           break;

         case KeyCode::CursorDown:  // Down arrow
           if( workView == dataView )
             commit();
           warning(active->fetch(workView->moveDown()));
           break;

         case KeyCode::CursorLeft:  // Left arrow
           warning(workView->moveLeft());
           break;

         case KeyCode::CursorRight: // Right arrow
           warning(workView->moveRight());
           break;

         case KeyCode::CursorUp:    // Up arrow
           if( workView == dataView )
             commit();
           warning(active->fetch(workView->moveUp()));
           break;

//       case KeyCode::F01:         // F1
//         warning(deadkey);
//         break;
//
         case KeyCode::F02:         // F2
           commit();
           warning(safeSave(dataView->getRing()));
           break;

         case KeyCode::F03:         // F3
           commit();
           warning(safeExit(dataView->getRing()));
           break;

         case KeyCode::F04:         // F4
           commit();
           warning(activate(utilRing));
           break;

         case KeyCode::F05:         // F5
/////////  commit();                // Locate does this
           warning(locate());
           break;

         case KeyCode::F06:         // F6
/////////  commit();                // Locate does this
           warning(change());
           break;

         case KeyCode::F07:         // F7 (Prev)
           commit();
           oldRing= dataView->getRing();
           newRing= (EdRing*)oldRing->getPrev();
           if( newRing == NULL )
             newRing= (EdRing*)ringList.getTail();
           warning(activate(newRing));
           break;

         case KeyCode::F08:         // F8 (Next)
           commit();
           oldRing= dataView->getRing();
           newRing= (EdRing*)oldRing->getNext();
           if( newRing == NULL )
             newRing= (EdRing*)ringList.getHead();
           warning(activate(newRing));
           break;

         case KeyCode::F09:         // F9
           nameToHist(this);
           break;

         case KeyCode::F10:         // FA
           warning(undo());
           break;

         case KeyCode::F11:         // FB
           dataView->screenTop();
           break;

         case KeyCode::F12:         // FC (Next screen)
           commit();
           dataView->setActive(NULL);
           dataView= (EdView*)dataView->getNext();
           if( dataView == NULL )
             dataView= (EdView*)histView->getNext();

           // Resynchronize
           dataView->setActive(dataActive);
           dataActive->fetch(dataView->getRing(), dataView->getLine());
           if( workView != histView )
             workView= dataView;
           break;

         case KeyCode::ALT_B:       // Alt-b
           string= mark->mark(dataView->getRing(),
                              dataView->getLine(),
                              dataView->getColumn());
           warning(string);
           break;

         case KeyCode::ALT_C:       // Alt-c
           commit();
           string= mark->copy(dataView->getRing(),
                              dataView->getLine(),
                              dataView->getColumn());
           warning(string);
           break;

         case KeyCode::ALT_D:       // Alt-d
           commit();
           warning(mark->remove());
           break;

         case KeyCode::ALT_I:       // Alt-i
           insertLine();
           break;

         case KeyCode::ALT_J:       // Alt-j
           warning(lineJoin());
           break;

         case KeyCode::ALT_L:       // Alt-l
           warning(mark->mark(dataView->getRing(), dataView->getLine()));
           break;

         case KeyCode::ALT_M:       // Alt-m
           commit();
           string= mark->move(dataView->getRing(),
                              dataView->getLine(),
                              dataView->getColumn());
           warning(string);
           break;

         case KeyCode::ALT_P:       // Alt-p
           commit();
           warning(mark->format());
           break;

         case KeyCode::ALT_R:       // Alt-r
           warning(removeLine());
           break;

         case KeyCode::ALT_S:       // Alt-s
           warning(lineSplit());
           break;

         case KeyCode::ALT_U:       // Alt-u
           mark->reset();
           break;

         case KeyCode::CTL_Home:    // Ctrl-Home
           commit();
           warning(dataActive->fetch(dataView->moveFirst()));
           break;

         case KeyCode::CTL_End:     // Ctrl-End
           warning(active->clear(column));
           viewChange(oldRing, oldLine);
           break;

         case KeyCode::CTL_PageUp:  // Ctrl-Page up
           commit();
           warning(dataActive->fetch(dataView->moveFirst()));
           break;

         case KeyCode::CTL_PageDown:// Ctrl-Page down
           commit();
           warning(dataActive->fetch(dataView->moveLast()));
           break;

         case KeyCode::BACKTAB:     // Shft-tab
           warning(workView->column(tabLeft(column)));
           break;

         case KeyCode::MOUSE_1:
         case KeyCode::MOUSE_2:
         case KeyCode::MOUSE_3:
           break;

         case KeyCode::MOUSE_WHEEL_DOWN:
           commit();
           for(int i=0; i<3; i++)
             warning(dataActive->fetch(dataView->moveDown()));
           break;

         case KeyCode::MOUSE_WHEEL_UP:
           commit();
           for(int i=0; i<3; i++)
             warning(dataActive->fetch(dataView->moveUp()));
           break;

         case KeyCode::MOUSE_WHEEL_LEFT:
           for(int i=0; i<3; i++)
             warning(dataView->moveLeft());
           break;

         case KeyCode::MOUSE_WHEEL_RIGHT:
           for(int i=0; i<3; i++)
             warning(dataView->moveRight());
           break;

         default:
           warning(deadkey);
           break;
       }
     }
   }
}

