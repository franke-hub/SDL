//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdMark.cpp
//
// Purpose-
//       EdMark object methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/syslib.h>

#include "Active.h"
#include "Editor.h"
#include "EdLine.h"
#include "EdRing.h"

#include "EdMark.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "EDMARK  " // Source file

#if defined(HCDM) && FALSE
  #define HCDM_COPY(x) x
#else
  #define HCDM_COPY(x) ((void)0)
#endif

#ifndef BRINGUP_FORMAT
#undef  BRINGUP_FORMAT
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetchActive
//
// Purpose-
//       Fetch the active line since we may have changed it.
//
//----------------------------------------------------------------------------
static void
   fetchActive(                     // Fetch the active Line
     Editor*           editor)      // Editor object
{
   Active*             active= editor->dataActive;
   EdLine*             line=   active->getLine();

   active->fetch(line);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wordLength
//
// Purpose-
//       Determine the length of a text word.
//
//----------------------------------------------------------------------------
static int                          // Number of characters in word
   wordLength(                      // Determine word length
     const char*       text)        // Pointer to text
{
   int                 L;

   for(L=0; text[L] != ' ' && text[L] != '\0'; L++)
     ;

   return(L);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   EdMark::check( void ) const      // Debugging check
{
#ifdef HCDM
   EdLine*             last;        // Last marked line
   EdLine*             line;        // Working line
   EdRing*             ring;        // Working ring

   if( state == FSM_RESET )
     return;

   if( left > touchCol || right < touchCol )
   {
     tracef("%4d EdMark(%p)::check, !cols(%d<=%d<=%d)\n", __LINE__, this,
            left, touchCol, right);
     throw "InternalLogicError";
   }

   line= (EdLine*)this->ring->lineList.getHead();
   for(;;)
   {
     if( line == NULL )
     {
       tracef("%4d EdMark(%p)::check, no marked lines in ring\n", __LINE__,
              this);
       throw "InternalLogicError";
     }

     if( line->ctrl.marked )
       break;

     line= (EdLine*)line->getNext();
   }

   if( line != first )
   {
     tracef("%4d EdMark(%p)::check, first(%p) inconsistent(%p)\n", __LINE__,
            this, first, line);
     throw "InternalLogicError";
   }

   last= line;
   for(;;)
   {
     if( line == NULL )
     {
       tracef("%4d EdMark(%p)::check, mark does not end\n", __LINE__, this);
       throw "InternalLogicError";
     }

     if( !line->ctrl.marked )
       break;

     last= line;
     line= (EdLine*)line->getNext();
   }

   if( touchLine != first && touchLine != last )
   {
     tracef("%4d EdMark(%p)::check, touchLine(%p) first(%p) last(%p)\n",
            __LINE__, this, touchLine, first, last);
     throw "InternalLogicError";
   }

   for(;;)
   {
     if( line == NULL )
       break;

     if( line->ctrl.marked )
     {
       tracef("%4d EdMark(%p)::check, multiple marks\n", __LINE__, this);
       throw "InternalLogicError";
     }

     line= (EdLine*)line->getNext();
   }

   ring= (EdRing*)edit->ringList.getHead();
   while( ring != NULL )
   {
     if( ring != this->ring )
     {
       line= (EdLine*)ring->lineList.getHead();
       for(;;)
       {
         if( line == NULL )
           break;

         if( line->ctrl.marked )
         {
           tracef("%4d EdMark(%p)::check, mark offring(%p)\n",
                  __LINE__, this, ring);
           throw "InternalLogicError";
         }

         line= (EdLine*)line->getNext();
       }
     }

     ring= (EdRing*)ring->getNext();
   }
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdMark::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   tracef("%4d EdMark(%p)::debug(%s) "
          "state(%d) ring(%p) first(%p) last(%p) cols(%d<=%d<=%d)\n",
          __LINE__, this, message,
          state, ring, first, touchLine, left, touchCol, right);

#if FALSE
   const EdLine* line= first;
   while( line != NULL && line->ctrl.marked )
   {
     tracef("%p Line(%s)\n", line, line->text);

     line= (EdLine*)line->getNext();
   }
#endif
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::~EdMark
//
// Purpose-
//       EdMark constructor
//
//----------------------------------------------------------------------------
   EdMark::~EdMark( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d EdMark(%p)::~EdMark()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::EdMark
//
// Purpose-
//       EdMark constructor
//
//----------------------------------------------------------------------------
   EdMark::EdMark(                  // Constructor
     Editor*           editor)      // Editor object
:  edit(editor)
,  state(FSM_RESET)
,  ring(NULL)
,  first(NULL)
,  left(0)
,  right(0)
,  touchLine(NULL)
,  touchCol(0)
{
   #ifdef SCDM
     tracef("%4d EdMark(%p)::EdMark(%p)\n", __LINE__, this, editor);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::copy
//
// Purpose-
//       Copy a mark
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdMark::copy(                    // Copy the mark
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After this line
     unsigned          column)      // Using this column
{
   const char*         result;      // Resultant

   EdRing*             const ring= this->ring; // Source ring
   int                 const size= right-left+1; // Number of bytes in block
   EdLine*             from;        // Working Line
   EdLine*             into;        // Working Line
   EdLine*             last;        // Working Line
   EdLine*             line;        // Working Line
   unsigned            newSize;     // sizeof(newText)
   char*               newText;     // Allocated text
   unsigned            oldSize;     // sizeof(oldText)
   const char*         oldText;     // Original text

   #ifdef HCDM
     tracef("%4d EdMark(%p)::copy(%p,%p,%d)\n", __LINE__, this,
            edRing, edLine, column);
     check();
     edRing->check(__SOURCE__, __LINE__, edLine, edLine);
   #endif

   //-------------------------------------------------------------------------
   // Verify parameters
   //-------------------------------------------------------------------------
   result= verifyCopy(edRing, edLine, column);
   if( result != NULL )
     return result;

   //-------------------------------------------------------------------------
   // Unmark the source lines
   //-------------------------------------------------------------------------
   last= first;
   for(line= first; line->ctrl.marked; line= (EdLine*)line->getNext())
   {
     last= line;
     line->ctrl.marked= FALSE;
   }
   edit->viewChange(ring, first, last); // The source ring has changed

   from= first;
   into= edLine;
   first= NULL;
   touchLine= NULL;

   //-------------------------------------------------------------------------
   // Copy lines
   //-------------------------------------------------------------------------
   if( state == FSM_LINES )
   {
     state= FSM_RESET;              // No mark exists
     for(;;)                        // Copy the range
     {
       line= edRing->insertLine(into); // Insert a new line
       if( line == NULL )
       {
         result= "No storage";
         break;
       }

       line->ctrl.marked= TRUE;
       if( edRing->mode == EdRing::FM_BINARY )
         line->ctrl.delim= from->ctrl.delim;
       if( state == FSM_RESET )
       {
         state= FSM_LINES;
         first= line;
         this->ring= edRing;
       }
       touchLine= line;

       if( from->getSize() > 0 )
       {
         newText= edRing->allocateText(from->getSize()+1);
         if( newText == NULL )
         {
           result= "No storage";
           break;
         }

         strcpy(newText, from->getText());
         line->setText(newText);
       }

       if( from == last )
         break;

       into= line;
       from= (EdLine*)from->getNext();
     }
   }

   //-------------------------------------------------------------------------
   // Copy block
   //-------------------------------------------------------------------------
   else
   {
     HCDM_COPY(tracef("%4d copy block\n", __LINE__));
     HCDM_COPY(edRing->textPool.debug("EdMark::copy"));
     state= FSM_RESET;              // No mark exists
     for(;;)                        // Copy the range
     {
       oldSize= into->getSize();
       oldText= into->getText();
       if( oldSize > column || from->getSize() > left )
       {
         HCDM_COPY(tracef("%4d from: %3d:%3d '%s'\n", __LINE__,
                          left, right, from->getText()));
         HCDM_COPY(tracef("%4d into: %3d '%s'\n", __LINE__,
                          column, into->getText()));
         newSize= oldSize;
         if( newSize < column )
           newSize= column;

         newSize += size;
         newText= edRing->allocateText(newSize + 1);
         if( newText == NULL )
         {
           result= "No storage";
           break;
         }
         memset(newText, ' ', newSize);
         newText[newSize]= '\0';

         // Copy old text up to column
         if( column < oldSize )
           memcpy(newText, oldText, column);
         else
           memcpy(newText, oldText, oldSize);
         HCDM_COPY(tracef("%4d into: %3d '%s'\n", __LINE__, column, newText));

         // Copy new text from left to right columns
         if( from->getSize() > left )
         {
           if( from->getSize() <= right )
             memcpy(newText+column, from->getText()+left,
                    from->getSize()-left);
           else
             memcpy(newText+column, from->getText()+left, size);
         }
         HCDM_COPY(tracef("%4d into: %3d '%s'\n", __LINE__,
                          column+size, newText));

         // Copy old text from column to end
         if( oldSize > column )
           memcpy(newText+column+size, oldText+column, oldSize-column);
         HCDM_COPY(tracef("%4d into: %3d '%s'\n", __LINE__, newSize, newText));

         // Remove trailing blanks
         while( newSize > 0 && newText[newSize-1] == ' ' )
         {
           newSize--;
           newText[newSize]= '\0';
         }
         HCDM_COPY(tracef("%4d into: %3d %p '%s'\n", __LINE__,
                          newSize, newText, newText));
         HCDM_COPY(edRing->textPool.debug("EdMark::copy"));

         // Replace the text
         if( oldSize > 0 )
           edRing->releaseText((char*)into->getText());
         into->setText(newText);
         edRing->changed= TRUE;
         HCDM_COPY(into->debug("EdMark::copy"));
       }

       if( state == FSM_RESET )
       {
         state= FSM_BLOCK;
         first= into;
         this->ring= edRing;
       }
       touchLine= into;
       into->ctrl.marked= TRUE;

       if( from == last )
         break;

       into= (EdLine*)into->getNext();
       from= (EdLine*)from->getNext();
     }

     right= column + (right-left);
     left= column;
     touchCol= column;

     // We may have changed (and therefore deleted) the active line
     // without informing the Active object.
     // Fetching it again corrects the Active object and never hurts.
     fetchActive(edit);
   }

   #ifdef HCDM
     check();
   #endif
   if( touchLine != NULL )          // (If not "No Storage")
     edit->viewChange(edRing, first, touchLine); // The target ring has changed
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::format
//
// Purpose-
//       Format the mark
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdMark::format( void )           // Format the mark
{
   const char*         result;      // Resultant

   unsigned            const marginLeft= edit->marginLeft;
   unsigned            const marginRight= edit->marginRight;

   Active*             active= edit->workActive;
   EdLine*             from;        // Working Line
   EdLine*             into;        // Working Line
   EdLine*             last;        // Working Line
   EdLine*             line;        // Working Line
   const char*         text;        // Working Line text
   EdLine*             work;        // Working Line
   unsigned            workCol;     // Column number of work line
   unsigned            wordLen;     // Length of next word

   #ifdef HCDM
     tracef("%4d EdMark(%p)::format()\n", __LINE__, this);
   #endif

   //-------------------------------------------------------------------------
   // Verify parameters
   //-------------------------------------------------------------------------
   #ifdef BRINGUP_FORMAT
     tracef("%4d: left(%d) right(%d)\n", __LINE__, marginLeft, marginRight);
     check();
   #endif

   if( state == FSM_RESET )
     return "No mark";

   if( state != FSM_LINES )
     return "Improper mark";

   if( marginRight >= 256 )
     return "Invalid margins";

   //-------------------------------------------------------------------------
   // Find the last marked line, removing the mark along the way
   //-------------------------------------------------------------------------
   from= last= first;
   for(line= first; line->ctrl.marked; line= (EdLine*)line->getNext())
   {
     line->ctrl.marked= FALSE;
     last= line;
   }

   //-------------------------------------------------------------------------
   // Format the mark
   //-------------------------------------------------------------------------
   result= NULL;                    // No error yet
   state= FSM_RESET;                // No mark exists
   into= last;                      // Insert after last line
   work= from;                      // Begin at the beginning
   workCol= 0;                      // Copy from column
   for(;;)                          // Format the range
   {
     line= ring->insertLine(into);  // Insert a new line
     if( line == NULL )
     {
       result= "No storage";
       break;
     }
     #ifdef BRINGUP_FORMAT
       tracef("%4d: Allocated(%p)\n", __LINE__, line);
     #endif

     line->ctrl.marked= TRUE;
     if( state == FSM_RESET )
     {
       state= FSM_LINES;
       first= line;
     }
     touchLine= line;
     active->fetch(ring, line);
     if( marginLeft > 0 )
       active->expand(marginLeft - 1);

     for(;;)
     {
       #ifdef BRINGUP_FORMAT
         tracef("%4d: %d %d '%s'\n", __LINE__,
                workCol, work->getSize(), work->getText());
       #endif
       if( workCol >= work->getSize() )
       {
         if( work == last )
           goto formatComplete;

         work= (EdLine*)work->getNext();
         workCol= 0;
         continue;
       }

       text= work->getText();
       #ifdef BRINGUP_FORMAT
         tracef("%4d: '%s'\n", __LINE__, &text[workCol]);
       #endif
       while( text[workCol] == ' ' )
         workCol++;

       wordLen= wordLength(&text[workCol]);
       #ifdef BRINGUP_FORMAT
         tracef("%4d: %d= wordLength(%s)\n", __LINE__,
                wordLen, &text[workCol]);
       #endif
       if( wordLen == 0 )
         continue;

       if( (wordLen + active->getUsed()) >= marginRight
           &&active->getUsed() > marginLeft )
         break;

       #ifdef BRINGUP_FORMAT
         active->debug("before append");
       #endif
       if( active->getUsed() > marginLeft )
         active->expand(active->getUsed());
       if( active->appendString(&text[workCol], wordLen) != 0 )
       {
         result= "Format error";
         break;
       }
       #ifdef BRINGUP_FORMAT
         active->debug(" after append");
       #endif

       workCol += wordLen;
       #ifdef BRINGUP_FORMAT
         tracef("%4d: '%s'\n", __LINE__, &text[workCol]);
       #endif
     }

     if( active->store() != 0 )
     {
       active->reset();
       result= "No storage";
       break;
     }
     into= line;
   }

   //-------------------------------------------------------------------------
   // Remove the old mark
   //-------------------------------------------------------------------------
formatComplete:
   if( result == NULL )
     result= active->store();

   if( result == NULL )
   {
     removePrior(ring, from, last, 0, 0);

     edit->activate(first);
     if( active->getUsed() == marginLeft )
     {
       touchLine= (EdLine*)line->getPrev();
       edit->removeLine(ring, line, line);
       if( line == first )
         state= FSM_RESET;
     }
   }

   //-------------------------------------------------------------------------
   // Format complete
   //-------------------------------------------------------------------------
   #ifdef HCDM
     check();
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::mark
//
// Purpose-
//       Create/expand/contract block mark
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdMark::mark(                    // Create/expand/contract block mark
     EdRing*           edRing,      // In this ring
     EdLine*           edLine,      // Using this line
     int               column)      // Using this column
{
   EdLine*             head;        // First changed line
   EdLine*             tail;        // Final changed line
   EdLine*             line;        // Working line

   #ifdef HCDM
     tracef("%4d EdMark(%p)::mark(%p,%p,%d)\n", __LINE__, this,
            edRing, edLine, column);
   #endif

   if( edLine->ctrl.readonly )
     return "Protected";

   if( state == FSM_RESET )
   {
     ring= edRing;
     first= touchLine= edLine;
     edLine->ctrl.marked= TRUE;
     state= FSM_LINES;
     left= right= touchCol= 0;
     if( column >= 0 )
     {
       state= FSM_BLOCK;
       left= right= touchCol= column;
     }
     return edit->viewChange(edRing, edLine);
   }

   if( ring != edRing )
     return "Mark offscreen";

   head= tail= NULL;
   if( edLine->ctrl.marked )        // If contract
   {
     head= tail= (EdLine*)touchLine->getNext();
     if( tail->ctrl.marked )        // First line is also touch line
     {
       // Remove marks from the next line to the last marked line
       tail= (EdLine*)edLine->getNext();
       while( tail->ctrl.marked )
       {
         tail->ctrl.marked= FALSE;
         tail= (EdLine*)tail->getNext();
       }
     }
     else                           // Last line is the touch line
     {
       // Remove marks from first up to (but not including) edLine
       head= tail= first;           // Begin with the first mark
       for(;;)
       {
         if( tail == edLine )
           break;

         tail->ctrl.marked= FALSE;
         tail= (EdLine*)tail->getNext();
         assert( tail != NULL );
       }
       first= edLine;
     }
   }
   else                             // If expand
   {
     head= tail= edLine;            // First look from edLine on down
     while( !tail->ctrl.marked )
     {
       tail= (EdLine*)tail->getNext();
       if( tail == NULL )
         break;
     }
     if( tail == NULL )             // Didn't find a mark
     {
       tail= edLine;                // Look from the edLine on up
       while( !head->ctrl.marked )
       {
         head= (EdLine*)head->getPrev();
         if( head == NULL )
           return "SNO: No mark found!";
       }
     }
     else
       first= head;                 // We have a new first line

     for(line= head;;line= (EdLine*)line->getNext())
     {
       line->ctrl.marked= TRUE;
       if( line == tail )
         break;
     }
   }
   touchLine= edLine;

   state= FSM_LINES;
   if( column >= 0 )                // If column operation
   {
     state= FSM_BLOCK;
     if( column < left )
       left= column;
     else if( column > right )
       right= column;
     else if( left == touchCol )
       right= column;
     else
       left= column;
     touchCol= column;
   }

   edit->viewChange(edRing, head, tail); // Indicate lines changed

   #ifdef HCDM
     check();
   #endif
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::move
//
// Purpose-
//       Move a mark
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdMark::move(                    // Move the mark
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // Using this line
     unsigned          column)      // Using this column
{
   const char*         result;      // Resultant

   EdRing*             const ring= this->ring; // Source ring
   EdLine*             from;        // Working Line
   EdLine*             last;        // Working Line
   EdLine*             line;        // Working Line
   unsigned            length;      // Remove length
   unsigned            oldLeft;     // Remove from left column
   unsigned            oldRight;    // Remove from right column

   #ifdef HCDM
     tracef("%4d EdMark(%p)::move(%p,%p,%d)\n", __LINE__, this,
            edRing, edLine, column);
     check();
     edRing->check(__SOURCE__, __LINE__, edLine, edLine);
   #endif

   //-------------------------------------------------------------------------
   // Verify parameters
   //-------------------------------------------------------------------------
   result= verifyMove(edRing, edLine, column);
   if( result != NULL )
     return result;

   //-------------------------------------------------------------------------
   // Find the last marked line
   //-------------------------------------------------------------------------
   from= last= first;
   for(line= first; line->ctrl.marked; line= (EdLine*)line->getNext())
     last= line;

   //-------------------------------------------------------------------------
   // Move lines
   //-------------------------------------------------------------------------
   if( state == FSM_LINES )
   {
     if( ring == edRing )           // If intra-ring move
     {
       // Remove the block from the ring
       from->getPrev()->setNext(last->getNext());
       last->getNext()->setPrev(from->getPrev());

       // Insert the block into the ring
       line= (EdLine*)edLine->getNext();
       last->setNext(line);
       line->setPrev(last);

       edLine->setNext(from);
       from->setPrev(edLine);

       edRing->changed= TRUE;
       edRing->resetCache();
       edit->viewChange(edRing, from, last);
     }
     else                           // If inter-ring move
     {
       result= copy(edRing, edLine, column); // First copy the lines
       if( result == NULL )         // If copy OK, delete the source lines
         removePrior(ring, from, last, 0, 0);
     }
   }

   //-------------------------------------------------------------------------
   // Move block
   //-------------------------------------------------------------------------
   else
   {
     oldLeft= left;
     oldRight= right;
     length= oldRight - oldLeft + 1;

     result= copy(edRing, edLine, column); // First copy the block
     if( result == NULL )           // If copy OK, delete the source columns
     {
       if( from == edLine           // If overlap move
           &&column < oldRight )    // and to the left
       {
         oldLeft  += length;
         oldRight += length;
       }

       removePrior(ring, from, last, oldLeft, oldRight);

       if( from == edLine           // If overlap move
           &&column >= oldRight )   // and to the right
       {
         left  -= length;
         right -= length;
         touchCol= right;
       }
     }
   }

   #ifdef HCDM
     check();
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::remove
//
// Purpose-
//       Remove (delete) the mark
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdMark::remove( void )           // Remove the mark
{
   EdLine*             last;        // Working Line
   EdLine*             line;        // Working Line

   #ifdef HCDM
     tracef("%4d EdMark(%p)::remove()\n", __LINE__, this);
   #endif

   //-------------------------------------------------------------------------
   // Verify parameters
   //-------------------------------------------------------------------------
   #ifdef HCDM
     check();
   #endif

   if( state == FSM_RESET )
     return "No mark";

   //-------------------------------------------------------------------------
   // Find the last marked line, removing the mark along the way
   //-------------------------------------------------------------------------
   last= first;
   for(line= first; line->ctrl.marked; line= (EdLine*)line->getNext())
   {
     line->ctrl.marked= FALSE;
     last= line;
   }

   //-------------------------------------------------------------------------
   // Delete the mark
   //-------------------------------------------------------------------------
   removePrior(ring, first, last, left, right);

   state= FSM_RESET;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::removeLine
//
// Purpose-
//       Prepare to remove a line.
//
//----------------------------------------------------------------------------
void
   EdMark::removeLine(              // Prepare removal of a marked line
     const EdRing*     edRing,      // The Ring containing the lines
     const EdLine*     head,        // The first line to remove
     const EdLine*     tail)        // The final line to remove
{
   EdLine*             line;        // Working Line

   #ifdef HCDM
     tracef("%4d EdMark(%p)::removeLine(%p,%p,%p)\n", __LINE__, this,
            edRing, head, tail);
   #endif

   if( first->between(head, tail) ) // Removing first line
   {
     first= (EdLine*)tail->getNext();
     if( !first->ctrl.marked )
       state= FSM_RESET;
   }

   if( touchLine->between(head, tail) ) // Removing last line touched
   {
     line= (EdLine*)tail->getNext();
     if( line->ctrl.marked )
       touchLine= line;
     else
       touchLine= (EdLine*)head->getPrev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::removeRing
//
// Purpose-
//       Prepare to remove a ring
//
//----------------------------------------------------------------------------
void
   EdMark::removeRing(              // Prepare removal of a ring
     EdRing*           edRing)      // Using this ring
{
   #ifdef HCDM
     tracef("%4d EdMark(%p)::removeRing(%p)\n", __LINE__, this, edRing);
   #endif

   if( ring == edRing )             // If removing the marked ring
     state= FSM_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::removePrior
//
// Purpose-
//       Remove (delete) the prior mark
//
//----------------------------------------------------------------------------
void
   EdMark::removePrior(             // Physically delete the mark
     EdRing*           edRing,      // The ring that was marked
     EdLine*           head,        // The first line that was marked
     EdLine*           tail,        // The last line that was marked
     unsigned          left,        // Old left column
     unsigned          right)       // Old right column
{
   unsigned            length;      // Number of bytes in old block
   EdLine*             line;        // Working Line
   unsigned            size;        // Working Line size

   #ifdef HCDM
     tracef("%4d EdMark(%p)::removePrior(%p,%p,%p,%d,%d)\n", __LINE__, this,
            edRing, head, tail, left, right);
     edRing->check(__SOURCE__, __LINE__, head, tail);
   #endif

   //-------------------------------------------------------------------------
   // Remove lines
   //-------------------------------------------------------------------------
   if( state == FSM_LINES )
   {
     edit->removeLine(edRing, head, tail); // Remove the source lines
     return;
   }

   //-------------------------------------------------------------------------
   // Remove the prior block
   //-------------------------------------------------------------------------
   if( state == FSM_BLOCK )
   {
     line= head;
     length= right-left+1;          // Number of columns deleted
     for(;;)                        // Update the source lines
     {
       size= line->getSize();       // Get the line length
       if( size > left )            // If there is something to remove
       {
         if( size < (left + length) )
           memset(line->text + left, 0, size-left);
         else
         {
           memmove(line->text + left, line->text + left + length, size-left);
           memset(line->text + size - length, 0, length);
         }
       }

       // Remove trailing blanks
       size= line->getSize();
       while( size > 0 && line->text[size-1] == ' ' )
       {
         size--;
         line->text[size]= '\0';
       }

       if( line == tail )
         break;

       line= (EdLine*)line->getNext();
     }

     edit->viewChange(ring, head, tail);
     edRing->changed= TRUE;

     // We may have also changed the active line without informing the
     // Active object.
     // Fetching it again corrects the Active object and never hurts.
     fetchActive(edit);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::reset
//
// Purpose-
//       Reset (undo) the mark
//
//----------------------------------------------------------------------------
void
   EdMark::reset( void )            // Reset (undo) the mark
{
   EdLine*             head;        // First changed line
   EdLine*             tail;        // Final changed line
   EdLine*             line;        // Working line

   #ifdef HCDM
     tracef("%4d EdMark(%p)::reset()\n", __LINE__, this);
     check();
   #endif

   if( state != FSM_RESET )         // If a mark exists
   {
     head= tail= line= first;       // Remove marks from ring
     while( line->ctrl.marked )
     {
       tail= line;
       line->ctrl.marked= FALSE;
       line= (EdLine*)line->getNext();
     }
     state= FSM_RESET;          

     edit->viewChange(ring, head, tail); // Indicate lines changed
   }

   #ifdef HCDM
     check();
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::verifyCopy
//
// Purpose-
//       Verify that a copy operation can be performed.
//
//----------------------------------------------------------------------------
const char*                         // Error message (NULL OK)
   EdMark::verifyCopy(              // Verify copy parameters
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After or into this line
     unsigned          column)      // Using this column
{
   EdLine*             from;        // Working Line
   EdLine*             into;        // Working Line
   EdLine*             next;        // Working Line

   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   if( state == FSM_RESET )         // If no mark exists
     return "No mark";

   //-------------------------------------------------------------------------
   // Line mark checks
   //-------------------------------------------------------------------------
   if( state == FSM_LINES )
   {
     if( edLine->getNext() == NULL )
       return "Protected";

     if( edLine->ctrl.marked )      // If the current line is marked
     {
       if( ((EdLine*)edLine->getNext())->ctrl.marked )
         return "Block conflict";
     }

     return NULL;
   }

   //-------------------------------------------------------------------------
   // Block mark checks
   //-------------------------------------------------------------------------
   if( edLine->ctrl.readonly )
     return "Protected";

   //-------------------------------------------------------------------------
   // Check for allowed overlap within same ring
   //-------------------------------------------------------------------------
   if( edRing == ring )             // If same ring
   {
     from= first;
     into= edLine;
     if( from == into )             // If same line
     {
       if( column <= left || column > right )
         return NULL;

       return "Block conflict";
     }

     while( from->ctrl.marked )
     {
       if( into->ctrl.marked )
         return "Block conflict";

       from= (EdLine*)from->getNext();
       into= (EdLine*)into->getNext();
       if( into == NULL )
         break;
     }
   }

   //-------------------------------------------------------------------------
   // If required, expand the target ring
   //-------------------------------------------------------------------------
   from= (EdLine*)first->getNext();
   into= edLine;
   next= (EdLine*)into->getNext();
   while( from->ctrl.marked )
   {
     if( next->ctrl.readonly )
     {
       into= edRing->insertLine(into);
       if( into == NULL )
         return "No storage";
     }
     else
     {
       into= next;
       next= (EdLine*)next->getNext();
     }

     from= (EdLine*)from->getNext();
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::verifyMove
//
// Purpose-
//       Verify that a move operation can be performed.
//
//----------------------------------------------------------------------------
const char*                         // Error message (NULL OK)
   EdMark::verifyMove(              // Verify copy parameters
     EdRing*           edRing,      // To this ring
     EdLine*           edLine,      // After this line
     unsigned          column)      // Using this column
{
   //-------------------------------------------------------------------------
   // Line move is not allowed if the target line is marked
   //-------------------------------------------------------------------------
   if( state == FSM_LINES )
   {
     if( edLine->ctrl.marked )
       return "Block conflict";
   }

   //-------------------------------------------------------------------------
   // Move is otherwise the same as copy
   //-------------------------------------------------------------------------
   return verifyCopy(edRing, edLine, column);
}

