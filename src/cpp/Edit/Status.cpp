//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Status.cpp
//
// Purpose-
//       Status object methods.
//
// Last change date-
//       2020/10/03 (Version 2, Release 1) - Extra compiler warnings
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/istring.h>
#include <com/syslib.h>

#include "Active.h"
#include "EdLine.h"
#include "EdMark.h"
#include "EdRing.h"
#include "EdView.h"

#include "Status.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*       msgnone=
    "F1=NOP1  2=Save  3=Quit  4=Buff  5=Find "
    " 6=Chng  7=Prev  8=Next  9=Name 10=Undo ";

//----------------------------------------------------------------------------
//
// Method-
//       Status::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   Status::check( void ) const      // Debugging check
{
#ifdef HCDM
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Status::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   EdRing*             ring= edit->dataActive->getRing();
   EdView*             view= edit->dataView;

   tracef("%4d Status(%p)::debug(%s) fsm(%d) defer: msg(%d) sts(%d)\n"
          , __LINE__, this, message
          , msgState, deferMsg, deferSts);

   tracef("    cols(%d) rowSts(%d) rowMsg(%d)\n"
          , cols, rowSts, rowMsg);

   tracef("    view(%p) ring(%p->%p) col(%d->%d) row(%d->%d)\n"
          , edit->dataView
          , this->ring, ring
          , column, view->getColumn(), row, view->getRow());

   if( ring != NULL )
     tracef("    rows(%d->%d) mode(%d->%d) chg(%d->%d) err(%d->%d) type(%d)"
            , rows, ring->rows, mode, ring->mode
            , changed, ring->changed, damaged, ring->damaged
            , ring->type);
   tracef("\n");

#else                               // Parameter ignored without HCDM
   (void)message;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::~Status
//
// Purpose-
//       Status destructor
//
//----------------------------------------------------------------------------
   Status::~Status( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d Status(%p)::~Status()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::Status
//
// Purpose-
//       Status constructor
//
//----------------------------------------------------------------------------
   Status::Status(                  // Constructor
     Editor*           parent)      // Editor object
:  EdDraw(parent->getTerminal())
,  edit(parent)
,  cols(0)
,  rowSts(0)
,  rowMsg(0)
,  ring(NULL)
,  column(0)
,  row(0)
,  mode(0)
,  rows(0)
,  changed(FALSE)
,  damaged(FALSE)
,  insertKey(FALSE)
,  deferMsg(FALSE)
,  deferSts(FALSE)
,  msgState(MSG_NONE)
{
   #ifdef SCDM
     tracef("%4d Status(%p)::Status(%p)\n", __LINE__, this, terminal);
   #endif

   //-------------------------------------------------------------------------
   // Initialize the screen colors
   //-------------------------------------------------------------------------
   stsNorm[0]= VGAColor::Grey;      // Status:  grey  on black
   stsNorm[1]= VGAColor::Black;

   stsChng[0]= VGAColor::Red;       // Status:  red   on black
   stsChng[1]= VGAColor::Black;

   stsErrs[0]= VGAColor::Black;     // Status:  black on red
   stsErrs[1]= VGAColor::Red;

   msgDisp[0]= VGAColor::LightCyan; // Display: lt cyan on black
   msgDisp[1]= VGAColor::Black;

   msgInfo[0]= VGAColor::Green;     // Message: green   on black
   msgInfo[1]= VGAColor::Black;

   msgWarn[0]= VGAColor::Yellow;    // Message: yellow  on black
   msgWarn[1]= VGAColor::Black;

   msgErrs[0]= VGAColor::LightRed;  // Message: lt red  on black
   msgErrs[1]= VGAColor::Black;
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::defer(ReshowType)
//
// Purpose-
//       Deferred item reshow
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Status::defer(                   // Deferred item reshow
     ReshowType        type)        // This item
{
   #ifdef HCDM
     tracef("%4d Status(%p)::defer(%d)\n", __LINE__, this, type);
   #endif

   switch(type)                     // Process reshow
   {
     case RESHOW_ALL:               // If defer ALL
       deferMsg= TRUE;
       deferSts= TRUE;
       break;

     case RESHOW_CSR:               // If display cursor
       terminal->physicalXY(0, rowMsg);
       break;

     default:
       break;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::display(void)
//
// Purpose-
//       Physical display
//
/* Status line-
01234567890123456789012345678901234567890123456789012345678901234567890123456789
C[nnnn] L[nnnnnnnn,nnnnnnnn] [INS] [UNIX] EDIT V4.2, Filename.ext
*/
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Status::display( void )          // Physical display
{
   EdRing*             ring= edit->dataActive->getRing();
   EdView*             view= edit->dataView;

   Color::VGA*         attr;        // Attribute
   Color::Char         buffer[600]; // Screen line buffer
   int                 deferCol;    // Column number changed
   int                 deferRow;    // Row number changed
   unsigned            L;           // Working length
   char                string[2048];// Working string
   const char*         text;        // -> Text

   #ifdef HCDM
     tracef("%4d Status(%p)::display()\n", __LINE__, this);
     debug("display");
   #endif

   //-------------------------------------------------------------------------
   // Handle files damaged in various ways
   //-------------------------------------------------------------------------
   if( ring->type == EdRing::FT_UNUSABLE )
     message(MSG_ERROR, "Unusable");

   //-------------------------------------------------------------------------
   // Status display
   //-------------------------------------------------------------------------
   deferCol= FALSE;
   deferRow= FALSE;
   if( this->ring != ring
       || insertKey != terminal->ifInsertKey()
       || rows != ring->rows
       || mode != ring->mode
       || changed != ring->changed
       || damaged != ring->damaged )
     deferSts= TRUE;

   if( !deferSts )
   {
     if( column != view->getColumn() )
       deferCol= TRUE;

     if( row != view->getRow() )
       deferRow= TRUE;
   }
   if( deferSts || deferCol || deferRow )
   {
     L= sizeof(buffer);
     if( L > cols )
       L= cols;

     text= " BIN";
     if( ring->mode == EdRing::FM_UNIX )
       text= "UNIX";
     else if( ring->mode == EdRing::FM_DOS )
       text= " DOS";
     else if( ring->mode == EdRing::FM_MIXED )
       text= " MIX";
     else if( ring->mode == EdRing::FM_RESET )
     {
       #ifdef _OS_WIN
         text= " DOS";
         ring->mode= EdRing::FM_DOS;
       #else
         text= "UNIX";
         ring->mode= EdRing::FM_UNIX;
       #endif
     }

     memset(string, ' ', L);
     size_t x= snprintf(string, sizeof(string)-1,
                 "C[%4d] L[%8d,%8d] [%s] [%s] %s, %s"
                 , (view->getColumn() + 1) % 10000
                 , (view->getRow()) % 100000000
                 , (ring->rows - 2) % 100000000
                 , terminal->ifInsertKey() ? "INS" : "REP"
                 , text
                 , EDIT_VERSION
                 , ring->fileName
                 );
     if( x >= sizeof(string) )
       x= sizeof(string) - 1;
     string[x]= ' ';

     for(unsigned i= 0; i<L; i++)
     {
       buffer[i].setAttribute(stsNorm[0], stsNorm[1]);
       buffer[i].data= string[i] & 0x00ff;
     }

     #ifdef _OS_WIN
       if( ring->mode != EdRing::FM_DOS )
       {
         for(i= 36; i<40; i++)
           buffer[i].setAttribute(stsChng[0], stsChng[1]);
       }
     #else
       if( ring->mode != EdRing::FM_UNIX )
       {
         for(unsigned i= 36; i<40; i++)
           buffer[i].setAttribute(stsChng[0], stsChng[1]);
       }
     #endif

     attr= stsNorm;                 // Default, normal status
     if( ring->damaged )
       attr= stsErrs;
     else if( ring->changed )
       attr= stsChng;
     for(unsigned i= 53; i<L; i++)
       buffer[i].setAttribute(attr[0], attr[1]);

     if( deferSts )
       terminal->wr(rowSts, buffer, L);

     if( deferCol )
     {
       terminal->logicalXY(2, rowSts);
       terminal->wr(buffer+2, 4);
     }

     if( deferRow )
     {
       terminal->logicalXY(10, rowSts);
       terminal->wr(buffer+10, 8);
     }

     deferSts= FALSE;
   }

   //-------------------------------------------------------------------------
   // Message display
   //-------------------------------------------------------------------------
   if( deferMsg )                   // If reshow MSG
   {
     if( msgState == MSG_NONE )
     {
       terminal->setAttribute(msgDisp[0], msgDisp[1]);
       terminal->wr(rowMsg, msgnone, strlen(msgnone));
       deferMsg= FALSE;
     }
     else
     {
       if( msgState == MSG_INFO )
         attr= msgInfo;
       else if( msgState == MSG_WARN )
         attr= msgWarn;
       else
         attr= msgErrs;
       terminal->setAttribute(attr[0], attr[1]);
       terminal->wr(rowMsg, msgLine, strlen(msgLine));
       msgState= MSG_NONE;
     }
   }

   this->ring= ring;
   column=     view->getColumn();
   row=        view->getRow();
   rows=       ring->rows;
   mode=       ring->mode;
   changed=    ring->changed;
   damaged=    ring->damaged;
   insertKey=  terminal->ifInsertKey();

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::message
//
// Purpose-
//       Display message
//
//----------------------------------------------------------------------------
int                                 // Reply response character
   Status::message(                 // Put message on status line
     MsgFsm            level,       // Message level
     const char*       string)      // The message string
{
   int                 result;      // Resultant

   #ifdef HCDM
     tracef("%4d Status(%p)::message(%d,%s)\n", __LINE__, this, level, string);
   #endif

   result= 0;                       // Default, no reply character
   if( level > MSG_ERROR            // If reply is required
       ||level > msgState )         // or this message is more important
   {
     if( strlen(string) >= sizeof(msgLine) )
     {
       memcpy(msgLine, string, sizeof(msgLine));
       msgLine[sizeof(msgLine)-1]= '\0';
     }
     else
       strcpy(msgLine, string);

     deferMsg= TRUE;                // Deferred message pending
     msgState= level;               // Set new message level
     if( level > MSG_ERROR )        // If reply required
     {
       msgState= MSG_NONE;
       terminal->setAttribute(msgErrs[0], msgErrs[1]);
       terminal->wr(rowMsg, msgLine, strlen(msgLine));
       if( level > MSG_REPLY )
         terminal->alarm();
       while( terminal->poll() )
         terminal->rd();
       result= terminal->rd();
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::resize
//
// Purpose-
//       Handle resize event
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Status::resize(                  // Handle resize event
     unsigned          cols,        // Number of columns
     unsigned          rows)        // Number of rows
{
   const char*         result;      // Resultant

   #ifdef HCDM
     tracef("%4d Status(%p)::resize(%d,%d)\n", __LINE__, this, cols, rows);
   #endif

   //-------------------------------------------------------------------------
   // Initialize the screen format
   //-------------------------------------------------------------------------
   this->cols= cols;
   rowSts= 0;
   rowMsg= rows-1;

   result= NULL;
   if( cols < 80 )
     result= "Not enough columns";
   else if( rows < 4 )
     result= "Not enough rows";

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Status::warning
//
// Purpose-
//       Display warning
//
//----------------------------------------------------------------------------
const char*                         // (The input format string)
   Status::warning(                 // Display warning message
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer
   char                string[2048];// Intermediate string

   #if FALSE
     tracef("%4d Status(%p)::warning(%s)\n", __LINE__, this, fmt);
   #endif

   if( fmt != NULL )
   {
     va_start(argptr, fmt);         // Initialize va_ functions
     int L= vsnprintf(string, sizeof(string)-1, fmt, argptr); // Create string in buffer
     va_end(argptr);                // Close va_ functions
     if( size_t(L) >= sizeof(string) )
       string[sizeof(string)-1]= '\0';

     message(MSG_WARN, string);
   }

   return fmt;
}

