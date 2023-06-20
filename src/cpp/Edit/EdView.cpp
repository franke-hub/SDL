//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdView.cpp
//
// Purpose-
//       EdView object methods.
//
// Last change date-
//       2023/06/19 (Version 2, Release 2)
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/istring.h>
#include <com/syslib.h>

#include "Editor.h"

#include "Active.h"
#include "EdLine.h"
#include "EdMark.h"
#include "EdRing.h"
#include "Status.h"

#include "EdView.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef MOVE_HCDM
#undef  MOVE_HCDM                   // Move: Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// toPrint translation table
//----------------------------------------------------------------------------
static const char      toPrint[]=   // Convert to printable
{   '~',  '~',  '~',  '~',  '~',  '~',  '~',  '~'  // 00 - 07
,   '~',  '~',  '~',  '~',  '~',  '~',  '~',  '~'  // 08 - 0f
,   '~',  '~',  '~',  '~',  '~',  '~',  '~',  '~'  // 10 - 17
,   '~',  '~',  '~',  '~',  '~',  '~',  '~',  '~'  // 18 - 1f
,  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27  // 20 - 27
,  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f  // 20 - 2f
,  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37  // 30 - 37
,  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f  // 30 - 3f
,  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47  // 40 - 47
,  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f  // 40 - 4f
,  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57  // 50 - 57
,  0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f  // 50 - 5f
,  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67  // 60 - 67
,  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f  // 60 - 6f
,  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77  // 70 - 77
,  0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e,  '.'  // 70 - 7f
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // 80 - 87
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // 88 - 8f
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // 90 - 97
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // 98 - 9f
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // a0 - a7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // a8 - af
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // b0 - b7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // b8 - bf
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // c0 - c7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // c8 - cf
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // d0 - d7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // d8 - df
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // e0 - e7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // e8 - ef
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // f0 - f7
,   '.',  '.',  '.',  '.',  '.',  '.',  '.',  '.'  // f8 - ff
};

//----------------------------------------------------------------------------
//
// Method-
//       EdView::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   EdView::check( void ) const      // Debugging check
{
#ifdef HCDM
   if( active != NULL )
   {
     if( curRing != active->getRing() )
     {
       tracef("%4d EdView(%p)::check, "
              "curRing(%p) %p= active(%p)->getRing())\n"
              , __LINE__, this
              , curRing, active->getRing(), active);
       throw "InternalLogicError";
     }

     if( curLine != active->getLine() )
     {
       tracef("%4d EdView(%p)::check, "
              "curLine(%p) %p= active(%p)->getLine())\n"
              , __LINE__, this
              , curLine, active->getLine(), active);
       throw "InternalLogicError";
     }
   }

   if( curRow > rowMax-rowMin )
   {
     tracef("%4d EdView(%p)::check, curRow(%d) > max(%d)-min(%d)\n"
            , __LINE__, this, curRow, rowMax, rowMin);
     throw "InternalLogicError";
   }

   if( vid >= edit->viewCount )
   {
     tracef("%4d EdView(%p)::check, vid(%d) > edit->viewCount(%d)\n"
            , __LINE__, this, vid, edit->viewCount);
     throw "InternalLogicError";
   }
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdView::debug(                   // Debugging display
     const char*       message) const // Display message
{
   tracef("%4d EdView(%p)::debug(%s) %s vid(%d of %d)\n"
          "    firstLine(%p) firstRow(%d) curRow(%d) rows(%d,%d:%d)\n"
          "    firstCol(%d) curCol(%d) cols(%d) defer: row(%d) buf(%d)\n"
          "    curRing(%p) curLine(%p) '%s'\n"
          , __LINE__, this, message
          , isHistView() ? "Hist" : "Data"
          , vid, edit->viewCount
          , firstLine, firstRow, curRow, rows, rowMin, rowMax
          , firstCol, curCol, cols, deferRow, deferBuf
          , curRing, curLine, curLine->getText()
          );

#if FALSE
   const EdLine* line= firstLine;
   for(unsigned row= rowMin; row<=rowMax; row++)
   {
//   if( !isHistView() )
//     break;

     if( line == NULL )
       break;

     if( active != NULL && line == active->getLine() )
       tracef("%p [%2d] ****(%s)\n", line, row, active->getText());
     else
       tracef("%p [%2d] Line(%s)\n", line, row, line->getText());

     line= (EdLine*)line->getNext();
   }
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::~EdView
//
// Purpose-
//       EdView destructor
//
//----------------------------------------------------------------------------
   EdView::~EdView( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d EdView(%p)::~EdView()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::EdView
//
// Purpose-
//       EdView constructor
//
//----------------------------------------------------------------------------
   EdView::EdView(                  // Constructor
     Editor*           parent,      // Editor object
     Active*           active)      // Initial Active object
:  List<EdView>::Link(), EdDraw(parent->getTerminal())
,  edit(parent)
,  vid(parent->viewCount)
,  zcol(0)
,  zrow(2)
,  cols(0)
,  rows(0)
,  rowMin(0)
,  rowMax(0)
,  active(active)
,  firstCol(0)
,  firstRow(0)
,  firstLine(NULL)
,  curCol(0)
,  curRow(0)
,  curLine(NULL)
,  curRing(NULL)
,  deferRow(0)
,  deferBuf(FALSE)
{
   #ifdef SCDM
     tracef("%4d EdView(%p)::EdView(%p)\n", __LINE__, this, parent);
   #endif

   //-------------------------------------------------------------------------
   // Initialize the screen colors
   //-------------------------------------------------------------------------
   bufNorm[0]= VGAColor::Grey;      // File: grey  on blue
   bufNorm[1]= VGAColor::Blue;

   bufMark[0]= VGAColor::Blue;      // Mark: blue  on grey
   bufMark[1]= VGAColor::Grey;

   if( isHistView() )
   {
     bufNorm[0]= VGAColor::White;   // File: white on magenta
     bufNorm[1]= VGAColor::Magenta;

     bufMark[0]= VGAColor::Magenta; // Mark: magenta on white
     bufMark[1]= VGAColor::White;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::activate
//
// Purpose-
//       Activate the associated EdLine
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::activate(                // Activate Line
     EdLine*           edLine)      // Using this EdLine
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::activate(EdLine %p)\n", __LINE__, this, edLine);
   #endif

   curLine= edLine;
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::activate
//
// Purpose-
//       Activate the associated EdRing
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::activate(                // Activate Ring
     EdRing*           edRing)      // Using this EdRing
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::activate(EdRing %p)\n", __LINE__, this, edRing);
   #endif

   // Synchronize out
   synchStore();

   // Synchronize in
   return synchFetch(edRing);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::column
//
// Purpose-
//       Set column
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdView::column(                  // Set column
     int               left,        // Left column
     int               right)       // Right column
{
   unsigned            oldFirst= firstCol;

   #ifdef HCDM
     tracef("%4d EdView(%p)::column(%d,%d)\n", __LINE__, this, left, right);
   #endif

   // Position right
   if( right >= 0 )
   {
     if( firstCol > unsigned(right) )
     {
       firstCol= 0;
       if( unsigned(right) > cols )
         firstCol= right - cols + 1;
     }

     if( firstCol+cols <= unsigned(right) )
       firstCol= right - cols + 1;

     curCol= right - firstCol;
   }

   // Position left
   if( left >= 0 )
   {
     if( firstCol > unsigned(left) )
     {
       firstCol= left;
       curCol= 0;
     }

     if( firstCol+cols <= unsigned(left) )
       firstCol= left - cols + 1;

     curCol= left - firstCol;
   }

   if( oldFirst != firstCol )
     defer(RESHOW_BUF);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::defer(unsigned)
//
// Purpose-
//       Defer row display
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::defer(                   // Deferred row display
     unsigned int      row)         // This row
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::defer(row %d)\n", __LINE__, this, row);
   #endif

   if( deferRow == 0 )
     deferRow= row+1;
   else if( deferRow != row+1 )
     deferBuf= TRUE;

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::defer(ReshowType)
//
// Purpose-
//       Deferred item display
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::defer(                   // Deferred item display
     ReshowType        type)        // This item
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::defer(type %d)\n", __LINE__, this, type);
   #endif

   switch(type)                     // Process defer
   {
     case RESHOW_CSR:               // If reshow cursor
       terminal->physicalXY(curCol, rowMin + curRow);
       break;

     case RESHOW_ALL:               // If reshow ALL
     case RESHOW_BUF:               // If reshow BUF
       deferBuf= TRUE;
       break;

     default:
       break;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::display
//
// Purpose-
//       Physical reshow of one data row
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::display(                 // Physical reshow of one data row
     unsigned          row,         // Row number
     const EdLine*     edLine) const// -> EdLine
{
   const Color::VGA*   attrNorm= bufNorm; // Normal attribute
   const Color::VGA*   attrMark= bufMark; // Marked attribute
   EdMark*             mark= edit->mark; // The Mark

   Color::Char         buffer[512]; // Screen line buffer
   int                 marked;      // TRUE iff line is marked
   unsigned            size;        // strlen(text)
   const char*         text;        // -> Display text

   const Color::VGA*   attr;        // Attribute character
   unsigned int        i, j, k;

   #ifdef HCDM
     tracef("%4d EdView(%p)::display(%d,%p=%s)\n", __LINE__, this,
            row, edLine, edLine ? edLine->getText() : "NULL");
   #endif

   //-------------------------------------------------------------------------
   // Address the text and find it's length
   //-------------------------------------------------------------------------
   text= "";                        // Default, null text
   size= 0;
   marked= FALSE;
   if( edLine != NULL )
   {
     if( edLine->ctrl.marked )      // If this line is marked
       marked= TRUE;

     text= edLine->getText();
     if( active != NULL && edLine == active->getLine() )
       text= active->getText();

     size= strlen(text);
     if( firstCol < size )
     {
       text= text + firstCol;
       size -= firstCol;
     }
     else
     {
       text= "";
       size= 0;
     }
   }

   #if FALSE
     tracef("%4d EdView(%p)::display(%2d,%p) '%s'\n",
            __LINE__, this, row, edLine, text);
   #endif

   //-------------------------------------------------------------------------
   // Set screen data characters
   //-------------------------------------------------------------------------
   if( size > cols )
     size= cols;

   for(i=0; i < size ; i++)
     buffer[i].data= toPrint[(unsigned char)text[i]];

   for(i=size; i < cols ; i++)
     buffer[i].data= ' ';

   //-------------------------------------------------------------------------
   // Set screen attributes
   //-------------------------------------------------------------------------
   if( isDataView() && vid > 0 && row == rowMin )
   {
     attrNorm= bufMark;
     attrMark= bufNorm;
   }

   if( mark->state == EdMark::FSM_BLOCK // If blocks are marked
       && marked )                  // and this line is marked
   {
     j= 0;
     k= 0;
     if( mark->left > firstCol )
     {
       k= mark->left - firstCol;
       if( k > cols )
         k= cols;
     }
     for(i=j; i<k; i++)
       buffer[i].setAttribute(attrNorm[0], attrNorm[1]);

     j= k;
     if( mark->right >= firstCol )
     {
       k= mark->right - firstCol + 1;
       if( k > cols )
         k= cols;
     }
     for(i=j; i<k; i++)
       buffer[i].setAttribute(attrMark[0], attrMark[1]);

     j= k;
     k= cols;
     for(i=j; i<k; i++)
       buffer[i].setAttribute(attrNorm[0], attrNorm[1]);
   }
   else                             // If not a block mark
   {
     attr= attrNorm;                // Default, normal line
     if( marked )                   // If this line is marked
       attr= attrMark;              // Use the marked attribute
     for(i=0; i < cols; i++)
       buffer[i].setAttribute(attr[0], attr[1]);
   }

   //-------------------------------------------------------------------------
   // Display the line
   //-------------------------------------------------------------------------
   terminal->wr(row, buffer, cols); // Write the line
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::display
//
// Purpose-
//       Physical display
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::display( void )          // Physical display
{
   EdLine*             line;        // -> Line
   unsigned            row;         // Working row

   #ifdef HCDM
     tracef("%4d EdView(%p)::display()\n" , __LINE__, this);
     debug("display");
   #endif

   //-------------------------------------------------------------------------
   // Physical display
   //-------------------------------------------------------------------------
   synch();                         // Internal synchronization
   if( deferBuf )                   // If reshow BUF
   {
     terminal->setAttribute(bufNorm[0], bufNorm[1]);
     line= firstLine;
     for(row= rowMin; row <= rowMax; row++)
     {
       display(row, line);
       if( line != NULL )
         line= (EdLine*)line->getNext();
     }

     deferRow= 0;
     deferBuf= FALSE;
   }

   if( deferRow != 0 )             // If reshow ROW
   {
     deferRow += (rowMin - 1);
     line= firstLine;
     for(row= rowMin; row <= rowMax; row++)
     {
       if( row == deferRow )
       {
         display(row, line);
         break;
       }

       if( line != NULL )
         line= (EdLine*)line->getNext();
     }

     deferRow= 0;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveDown
//
// Purpose-
//       Move the view down one row.
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::moveDown( void )         // Move the view down one row
{
   EdLine*             line;        // -> Line

   #ifdef HCDM
     tracef("%4d EdView(%p)::moveDown()\n", __LINE__, this);
     #ifdef MOVE_HCDM
       debug("before moveDown");
     #endif
   #endif

   line= curLine;                   // Working line
   if( rowMin+curRow == rowMax      // If at bottom of screen (BOS)
       ||line->getNext() == NULL )  // or at end of file (EOF)
   {
     if( isHistView() )
     {
       line= (EdLine*)line->getNext();
       if( line == NULL || line->ctrl.readonly )
         line= (EdLine*)curRing->lineList.getHead()->getNext();

       activate(line);
     }
     else if( terminal->ifScrollKey() ) // If scroll locked
     {
       while( curRow > 0 )          // Move view to Top Of Screen
       {
         line= (EdLine*)line->getPrev();
         assert( line != NULL );
         curRow--;
       }
       activate(line);
     }
     else                           // If BOS, scroll unlocked
     {
       if( line->getNext() == NULL )// If EOF
       {
         if( curRow == 0 )
         {
           edit->status->message(MSG_WARN, "At end of file");
           goto exit;
         }
         firstLine= (EdLine*)firstLine->getNext();
         firstRow++;
         curRow--;
       }
       else                         // If BOS, scroll normal, !(EOF)
       {
         firstLine= (EdLine*)firstLine->getNext();
         firstRow++;
         activate((EdLine*)line->getNext());
       }

       defer(RESHOW_BUF);
     }
   }
   else                             // If !(BOS || EOF)
   {
     activate((EdLine*)line->getNext());
     curRow++;
   }

exit:
   #ifdef MOVE_HCDM
     debug("after moveDown");
   #endif
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveFirst
//
// Purpose-
//       Move view to top of ring
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::moveFirst( void )        // Move view to top of ring
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::moveFirst()\n", __LINE__, this);
   #endif

   firstLine= (EdLine*)curRing->lineList.getHead();
   activate(firstLine);
   curRow= firstRow= 0;
   defer(RESHOW_ALL);
   return firstLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveLast
//
// Purpose-
//       Move view to bottom of ring
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::moveLast( void )         // Move view to bottom of ring
{
   EdLine*             result;      // Resultant

   #ifdef HCDM
     tracef("%4d EdView(%p)::moveLast()\n", __LINE__, this);
   #endif

   firstLine= (EdLine*)curRing->lineList.getTail();
   activate(firstLine);
   curRow= 0;
   firstRow= curRing->rows;
   result= screenUp();
   defer(RESHOW_ALL);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveLeft
//
// Purpose-
//       Move the view left one column
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdView::moveLeft( void )         // Move the view left one column
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::moveLeft()\n", __LINE__, this);
     #ifdef MOVE_HCDM
       debug("before moveLeft");
     #endif
   #endif

   if( curCol == 0 )                // If at left of screen (LOS)
   {
     if( terminal->ifScrollKey() )  // If scroll locked
       curCol= cols - 1;
     else
     {
       if( firstCol > 0 )
       {
         firstCol--;
         defer(RESHOW_BUF);
       }
     }
   }
   else                             // If !(LOS)
     curCol--;

   #ifdef MOVE_HCDM
     debug("after moveLeft");
   #endif

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveRight
//
// Purpose-
//       Move the view right one column
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdView::moveRight( void )        // Move the view right one column
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::moveRight()\n", __LINE__, this);
     #ifdef MOVE_HCDM
       debug("before moveRight");
     #endif
   #endif

   curCol++;
   if( curCol == cols )             // If at right of screen
   {
     if( terminal->ifScrollKey() )  // If scroll locked
       curCol= 0;
     else
     {
       curCol--;
       firstCol++;
       defer(RESHOW_BUF);
     }
   }

   #ifdef MOVE_HCDM
     debug("after moveRight");
   #endif

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::moveUp
//
// Purpose-
//       Move the view up one row.
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::moveUp( void )           // Move the view up one row
{
   EdLine*             line;        // -> Line

   #ifdef HCDM
     tracef("%4d EdView(%p)::moveUp()\n", __LINE__, this);
     #ifdef MOVE_HCDM
       debug("before moveUp");
     #endif
   #endif

   line= curLine;                   // Working line
   if( curRow == 0 )                // If at top of screen (TOS)
   {
     if( isHistView() )
     {
       line= (EdLine*)line->getPrev();
       if( line == NULL || line->ctrl.readonly )
         line= (EdLine*)curRing->lineList.getTail()->getPrev();

       activate(line);
     }
     else if( terminal->ifScrollKey() ) // If scroll locked
     {
       while( curRow < rowMax-rowMin ) // Move view to Bottom Of Screen
       {
         if( line->getNext() == NULL )
           break;

         line= (EdLine*)line->getNext();
         curRow++;
       }
       activate(line);
     }
     else                           // If TOS, scroll unlocked
     {
       if( line->getPrev() == NULL )// If at top of file
       {
         edit->status->message(MSG_WARN, "At top of file");
         goto exit;
       }
       else                         // If TOS, scroll normal, !(top of file)
       {
         firstLine= (EdLine*)firstLine->getPrev();
         firstRow--;
         activate((EdLine*)line->getPrev());
       }

       defer(RESHOW_BUF);
     }
   }
   else                             // If !(TOS)
   {
     activate((EdLine*)line->getPrev());
     curRow--;
   }

exit:
   #ifdef MOVE_HCDM
     debug("after moveUp");
   #endif

   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::resize
//
// Purpose-
//       Screen resize
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdView::resize(                  // Resize screen
     unsigned          cols,        // Number of columns
     unsigned          rows)        // Number of rows
{
   unsigned            totalRows;   // The total number of buffer rows
   unsigned            perScreen;   // The number of rows per screen
   unsigned            maxScreen;   // The highest screen index
   unsigned            firstSize;   // The number of rows on the first screen

   #ifdef HCDM
     tracef("%4d EdView(%p)::resize(%d,%d)\n", __LINE__, this, cols, rows);
   #endif

   //-------------------------------------------------------------------------
   // Initialize the screen size
   //-------------------------------------------------------------------------
   this->cols= cols;
   this->rows= rows;

   //-------------------------------------------------------------------------
   // Initialize the screen format
   //-------------------------------------------------------------------------
   if( isHistView() )               // If history view
   {
     rowMin= 1;
     rowMax= 1;
   }
   else
   {
     maxScreen= edit->viewCount - 1;// The highest screen index
     totalRows= rows - zrow - 1;    // The total number of usable rows
     perScreen= (totalRows - maxScreen) / edit->viewCount;
     firstSize= totalRows - (perScreen*maxScreen);
     if( vid == 0 )
     {
       rowMin= zrow;
       rowMax= rowMin + firstSize - 1;
     }
     else
     {
       rowMin= zrow + firstSize + (vid-1)*perScreen;
       rowMax= rowMin + perScreen - 1;
     }

     #ifdef HCDM
       tracef("%4d EdView(%p) vid(%d of %d) rowMin(%d) rowMax(%d)\n"
              "    totalRows(%d) perScreen(%d) firstSize(%d)\n",
              __LINE__, this, vid, maxScreen, rowMin, rowMax,
              totalRows, perScreen, firstSize);
     #endif

     if( cols < 40 )
       return "Need more columns";
     if( perScreen < 4 )
       return "Need more rows";
   }
   return defer(RESHOW_ALL);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::screenDown
//
// Purpose-
//       Move the view down one screen
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::screenDown( void )       // Move the view down one screen
{
   EdLine*             line;        // -> Line
   unsigned            row;         // Working row

   #ifdef MOVE_HCDM
     debug("before screenDown");
   #endif

   line= firstLine;
   for(row= rowMin; row<rowMax; row++)
   {
     if( line->getNext() == NULL )
       break;

     firstRow++;
     line= (EdLine*)line->getNext();
   }
   firstLine= line;

   for(row= 0; row<curRow; row++)
   {
     if( line->getNext() == NULL )
       break;

     line= (EdLine*)line->getNext();
   }
   curRow= row;

   if( curLine != line )
     activate(line);

   defer(RESHOW_BUF);

   #ifdef MOVE_HCDM
     debug("after screenDown");
   #endif

   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::screenTop
//
// Purpose-
//       Move the current row to the top of the screen
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::screenTop( void )        // Move the current row to the top
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::screenTop()\n", __LINE__, this);
   #endif

   if( curRow == 0 )
     return curLine;

   firstLine= curLine;
   firstRow= firstRow + curRow;
   curRow= 0;
   defer(RESHOW_BUF);
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::screenUp
//
// Purpose-
//       Move the view up one screen
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::screenUp( void )         // Move the view up one screen
{
   EdLine*             line;        // -> Line
   unsigned            row;         // Working row

   #ifdef MOVE_HCDM
     debug("before screenUp");
   #endif

   line= firstLine;
   for(row= rowMin; row<rowMax; row++)
   {
     if( line->getPrev() == NULL )
       break;

     firstRow--;
     line= (EdLine*)line->getPrev();
   }
   firstLine= line;

   for(row= 0; row<curRow; row++)
   {
     if( line->getNext() == NULL )
       break;

     line= (EdLine*)line->getNext();
   }
   curRow= row;

   if( curLine != line )
     activate(line);

   defer(RESHOW_BUF);

   #ifdef MOVE_HCDM
     debug("after screenUp");
   #endif
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::scrollDown
//
// Purpose-
//       Scroll the data view down one row. The top line is set to next.
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::scrollDown( void )       // Scroll the data view down one row
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::scrollDown()\n", __LINE__, this);
   #endif

   if( firstLine->getNext() == NULL ) // If at end of file
   {
     edit->status->message(MSG_WARN, "At end of file");
     return curLine;
   }

   firstLine= firstLine->getNext(); // Update firstLine
   ++firstRow;

   if( curLine->getNext() )         // If moving curLine
     activate(curLine->getNext());

   defer(RESHOW_BUF);
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::scrollUp
//
// Purpose-
//       Scroll the data view up one row. The top line is set to previous.
//
//----------------------------------------------------------------------------
EdLine*                             // The new current Line
   EdView::scrollUp( void )         // Scroll the data view up one row
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::scrollUp()\n", __LINE__, this);
   #endif

   if( firstLine->getPrev() == NULL ) // If at top of file
   {
     edit->status->message(MSG_WARN, "At top of file");
     return curLine;
   }

   firstLine= firstLine->getPrev(); // Update firstLine
   --firstRow;

   if( curLine->getPrev() )         // If moving curLine
     activate(curLine->getPrev());

   defer(RESHOW_BUF);
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::synch
//
// Purpose-
//       Synchronize the current line internally and with the active object.
//
// Implementation notes-
//       Make the current line visible while minimizing jitter:
//         If adjustments are required
//           1) Keep firstLine stable and, failing that,
//           2) Make curRow= rowMax/2
//
//----------------------------------------------------------------------------
void
   EdView::synch( void )            // Synchronize
{
   EdLine*             line;        // -> Line
   unsigned            row;         // Working row

   #ifdef HCDM
     tracef("%4d EdView(%p)::synch()\n", __LINE__, this);
     debug();
   #endif

   //-------------------------------------------------------------------------
   // Synchronize with the Active object
   //-------------------------------------------------------------------------
   if( active != NULL )
   {
     #ifdef HCDM
       if( curRing != active->getRing() )
       {
         tracef("%4d EdView(%p)::synch, "
                "curRing(%p) %p= active(%p)->getRing())\n"
                , __LINE__, this
                , curRing, active->getRing(), active);
         throw "InternalLogicError";
       }
     #endif

     curLine= active->getLine();
   }

   //-------------------------------------------------------------------------
   // See if the current line is visible.
   //-------------------------------------------------------------------------
   line= firstLine;
   for(row= rowMin; row < rowMax; row++)
   {
     if( line == curLine || line == NULL )
       break;

     line= (EdLine*)line->getNext();
   }

   //-------------------------------------------------------------------------
   // If not visible, make it visible.
   //-------------------------------------------------------------------------
   if( line == curLine )            // If it is visible
     curRow= row - rowMin;          // Synchronize curRow with curLine
   else                             // If not visible
   {
     deferBuf= TRUE;
     curRow= (rowMax-rowMin)/2;
     firstLine= curLine;
     for(row= curRow; row > 0; row--)
     {
       if( firstLine->getPrev() == NULL )
       {
         curRow -= row;
         break;
       }

       firstLine= (EdLine*)firstLine->getPrev();
     }
   }

   //-------------------------------------------------------------------------
   // Synchronize firstRow with firstLine
   //-------------------------------------------------------------------------
   firstRow= curRing->rowNumber(firstLine);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::synchFetch
//
// Purpose-
//       Restore the current settings from the Ring
//
//----------------------------------------------------------------------------
EdLine*                             // The new current line
   EdView::synchFetch(              // Restore settings
     EdRing*           edRing)      // Using this EdRing
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::synchFetch(%p)\n", __LINE__, this, edRing);
   #endif

   curRing= edRing;
   if( curRing == NULL )
     return NULL;

   firstLine= curRing->firstLine;
   firstCol= curRing->firstCol;
   curLine= curRing->curLine;
   curCol= curRing->curCol;
   curRow= curRing->curRow;
   deferBuf= TRUE;
   return curLine;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::synchStore
//
// Purpose-
//       Save the current settings into the Ring
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::synchStore( void )       // Save settings
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::synchStore()\n", __LINE__, this);
   #endif

   if( curRing != NULL )
   {
     curRing->firstLine= firstLine;
     curRing->firstCol= firstCol;
     curRing->curLine= curLine;
     curRing->curCol= curCol;
     curRing->curRow= curRow;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::viewChange
//
// Purpose-
//       Update view after change.
//
// Implementation note-
//       This function determines whether reshow is required and, later,
//       synch() corrects firstLine and curLine when called from display().
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   EdView::viewChange(              // Update view after change
     const EdRing*     edRing,      // For this ring
     const EdLine*     edLine)      // For this line
{
   EdLine*             line;        // Working line

   #ifdef HCDM
     tracef("%4d EdView(%p)::viewChange(%p,%p)\n", __LINE__, this,
            edRing, edLine);
   #endif

   if( edRing == curRing )
   {
     line= firstLine;
     for(unsigned row= rowMin; row<=rowMax; row++)
     {
       if( edLine == line )
       {
         defer(row-rowMin);
         break;
       }

       line= (EdLine*)line->getNext();
       if( line == NULL )
         break;
     }
   }

   return NULL;
}

const char*                         // Return message (NULL)
   EdView::viewChange(              // Update view after change
     const EdRing*     edRing,      // For this ring
     const EdLine*     edLine,      // For this line
     const unsigned    column)      // For this column
{
   #ifdef HCDM
     tracef("%4d EdView(%p)::viewChange(%p,%p,%d)\n", __LINE__, this,
            edRing, edLine, column);
   #else                            // column ignored without HCDM
     (void)column;
   #endif

   return viewChange(edRing, edLine);
}

const char*                         // Return message (NULL)
   EdView::viewChange(              // Update view after change
     const EdRing*     edRing,      // For this ring
     const EdLine*     head,        // First changed line
     const EdLine*     tail)        // Last changed line
{
   EdLine*             line;        // Working line

   #ifdef HCDM
     tracef("%4d EdView(%p)::viewChange(%p,%p,%p)\n", __LINE__, this,
            edRing, head, tail);
   #endif

   if( edRing == curRing )
   {
     if( firstLine->between(head, tail) )
       return defer(RESHOW_BUF);

     line= firstLine;
     for(unsigned row= rowMin; row<=rowMax; row++)
     {
       if( head == line )
       {
         if( row == rowMax )
           defer(row-rowMin);
         else
           defer(RESHOW_BUF);
         break;
       }

       if( line == NULL )
         break;
       line= (EdLine*)line->getNext();
     }
   }

   return NULL;
}
