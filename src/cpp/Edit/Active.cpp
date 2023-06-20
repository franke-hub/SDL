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
//       Active.cpp
//
// Purpose-
//       Active object methods.
//
// Last change date-
//       2020/10/03 (Version 2, Release 1) - Extra compiler warnings
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/syslib.h>

#include "EdLine.h"
#include "EdRing.h"
#include "Active.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#if defined(HCDM) && TRUE
  #define DEBUG(x) x
#else
  #define DEBUG(x) ((void)0)
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Active::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   Active::check( void ) const      // Debugging check
{
#ifdef HCDM
   if( text == NULL )
   {
     if( textUsed != 0 )
     {
       tracef("%4d Active(%p)::check, text(NULL) textUsed(%d)\n", __LINE__,
              this, textUsed);
       throw "InternalLogicError";
     }
   }
   else if( strlen(text) != textUsed )
   {
     tracef("%4d Active(%p)::check, strlen(%zd) textUsed(%d)\n", __LINE__,
            this, strlen(text), textUsed);
     throw "InternalLogicError";
   }
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Active::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   tracef("%4d Active(%p)::debug(%s) "
          "state(%d) ring(%p) line(%p) base(%p) size(%d) used(%d)\n"
          "    %p '%s'\n", __LINE__,
          this, message,
          state, ring, line, base, textSize, textUsed, text, text);

   if( text != NULL && textSize > 0 )
     dump(text, textSize);
#else                               // Parameter unused with HCDM
   (void)message;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::~Active
//
// Purpose-
//       Editor: Active destructor
//
//----------------------------------------------------------------------------
   Active::~Active( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d Active(%p)::~Active()\n", __LINE__, this);
   #endif

   reset();

   if( base != NULL )
   {
     free(base);
     base= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::Active
//
// Purpose-
//       Editor: Active constructor
//
//----------------------------------------------------------------------------
   Active::Active(                  // Constructor
     unsigned          size)        // Working line size
:  state(FSM_RESET)
,  base(NULL)
,  baseSize(size)
,  text(NULL)
,  textSize(0)
,  textUsed(0)
,  ring(NULL)
,  line(NULL)
{
   #ifdef SCDM
     tracef("%4d Active(%p)::Active(%d)\n", __LINE__, this, size);
   #endif

   base= (char*)malloc(baseSize);
   if( base == NULL )
     throw "NoStorageException";

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::setState
//
// Purpose-
//       Update the current State
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::setState(                // Set the current State
     State             state)       // To this State
{
   const char*         result;      // Resultant

   result= NULL;
   switch(state)
   {
     case FSM_RESET:
       reset();
       break;

     default:
       result= expand(textUsed);
       break;
   }

   if( result == NULL )
     this->state= state;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::appendString
//
// Purpose-
//       Concatenate string
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::appendString(            // Concatenate string
     const char*       string,      // The join string
     unsigned          length)      // The join string length
{
   const char*         result;      // Resultant
   unsigned            textUsed= this->textUsed;

   #ifdef SCDM
     tracef("%4d Active(%p)::appendString(%s,%d)\n", __LINE__, this,
            string, length);
   #endif

   result= NULL;
   if( length > 0 )                 // (Can append NULL string)
   {
     assert( strlen(string) >= length ); // (Can append partial string)
     result= expand(textUsed + length - 1); // (Updates this->textUsed)
     if( result == NULL )
       memcpy(text+textUsed, string, length); // Concatenate the strings
   }
   return result;
}

const char*                         // Return message (NULL OK)
   Active::appendString(            // Concatenate string
     const char*       string)      // The join string
{
   return appendString(string, strlen(string));
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::clear
//
// Purpose-
//       Clear to end of line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::clear(                   // Clear to end of line
     unsigned          column)      // From this column (0 origin)
{
   const char*         result;      // Resultant

   #ifdef SCDM
     tracef("%4d Active(%p)::clear(%d)\n", __LINE__, this, column);
   #endif

   result= expand(column);
   if( result != NULL )
     return result;

   textUsed= column;
   text[column]= '\0';
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::expand
//
// Purpose-
//       Expand the active line, enabling it for change.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::expand(                  // Expand the active line
     unsigned          column)      // So that this column is valid
{
   char*               work;        // Working text
   unsigned            size;        // Working text length

   #ifdef SCDM
     tracef("%4d Active(%p)::expand(%d)\n", __LINE__, this, column);
   #endif

   if( line->ctrl.readonly )        // If protected
     return "Protected";

   DEBUG(debug("expand.."));

   column++;
   if( state == FSM_RESET )         // If first change
   {
     if( baseSize > textUsed && baseSize > column )
     {
       DEBUG(tracef("%4d text(%p=>%p) base\n", __LINE__, text, base));
       text= base;
       textSize= baseSize;
     }
     else
     {
       size= textUsed;
       if( column > size )
         size= column;
       size++;

       work= ring->allocateText(size);
       if( work == NULL )
         return "No storage";

       DEBUG(tracef("%4d text(%p) allocated\n", __LINE__, work));
       DEBUG(tracef("%4d text(%p=>%p)\n", __LINE__, text, work));
       text= work;
       textSize= size;
     }

     strcpy(text, line->getText());
     textUsed= line->getSize();
     state= FSM_CHANGE;
   }

   if( column > textUsed )          // If not already expanded
   {
     if( column >= textSize )       // If no room
     {
       size= column + 16;
       work= ring->allocateText(size);
       if( work == NULL )
         return "No storage";

       strcpy(work, text);
       DEBUG(tracef("%4d text(%p) allocated\n", __LINE__, work));
       if( text != base )
       {
         DEBUG(tracef("%4d text(%p) released\n", __LINE__, text));
         memset(text+textUsed, 0, textSize-textUsed);
         ring->releaseText(text);
       }

       DEBUG(tracef("%4d text(%p=>%p)\n", __LINE__, text, work));
       text= work;
       textSize= size;
     }

     memset(text+textUsed, ' ', textSize-textUsed); // Prevent GC
     text[column]= '\0';
     textUsed= column;
   }

   DEBUG(debug("..expand"));

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::fetch
//
// Purpose-
//       Activate a line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::fetch(                   // Activate a line
     EdRing*           edRing,      // -> ring to activate
     EdLine*           edLine)      // -> line to activate
{
   char*               text;        // -> Text
   unsigned            size;        // strlen(text)

   #ifdef SCDM
     tracef("%4d Active(%p)::fetch(%p,%p) '%s'\n", __LINE__, this,
            edRing, edLine, edLine->text);
   #endif

   reset();
   ring= edRing;
   line= edLine;
   text= edLine->text;
   size= edLine->getSize();

   // We start off with the unmodified line (which may be NULL)
   this->text= text;
   textSize= size;
   textUsed= size;
   return NULL;
}

const char*                         // Return message (NULL OK)
   Active::fetch(                   // Activate a line
     EdLine*           edLine)      // -> line to activate
{
   return fetch(ring, edLine);
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::insertChar
//
// Purpose-
//       Insert character.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::insertChar(              // Insert character
     unsigned          column,      // At this column (0 origin)
     int               code)        // The insert character
{
   const char*         result;      // Resultant
   int                 L;           // Move length

   #ifdef SCDM
     tracef("%4d Active(%p)::insertChar(%d,%c)\n", __LINE__, this,
            column, code);
   #endif

   //-------------------------------------------------------------------------
   // Expand the line
   //-------------------------------------------------------------------------
   if( column > textUsed )
     result= expand(column);
   else
     result= expand(textUsed);
   if( result != NULL )
     return result;

   L= textUsed - column - 1;
   memmove(text+column+1, text+column, L);

   //-------------------------------------------------------------------------
   // Insert the character
   //-------------------------------------------------------------------------
   text[column]= code;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::removeChar
//
// Purpose-
//       Remove character.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::removeChar(              // Remove the character
     unsigned          column)      // At this column (0 origin)
{
   const char*         result;      // Resultant
   int                 L;           // Move length

   #ifdef SCDM
     tracef("%4d Active(%p)::removeChar(%d)\n", __LINE__, this, column);
   #endif

   if( column >= textUsed )
     return NULL;

   result= expand(column);
   if( result != NULL )
     return result;

   L= textUsed - column;
   memmove(text+column, text+column+1, L+1);
   text[textUsed]= 0xff;
   textUsed--;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::replaceChar
//
// Purpose-
//       Replace character.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::replaceChar(             // Replace character
     unsigned          column,      // At this column (0 origin)
     int               code)        // The replacement character
{
   const char*         result;      // Resultant

   #ifdef SCDM
     tracef("%4d Active(%p)::replaceChar(%d,'%c')\n", __LINE__, this,
            column, code);
   #endif

   result= expand(column);
   if( result != NULL )
     return result;

   text[column]= code;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::replaceLine
//
// Purpose-
//       Replace an entire Line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::replaceLine(             // Replace an entire Line
     const char*       text)        // Replacement text
{
   const char*         result;      // Resultant
   int                 L;           // Working length

   #ifdef SCDM
     tracef("%4d Active(%p)::replaceLine(%s)\n", __LINE__, this, text);
   #endif

   L= strlen(text);
   result= expand(L);
   if( result == NULL )
   {
     strcpy(this->text, text);
     textUsed--;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::replaceString
//
// Purpose-
//       Replace string
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::replaceString(           // Replace string
     unsigned          column,      // The replacement column
     unsigned          length,      // The replacement (delete) length
     const char*       string)      // The replacement (insert) string
{
   const char*         result;      // Resultant
   unsigned            L= strlen(string); // Replacement length
   unsigned            textUsed= this->textUsed; // Strlen(text) on entry

   #ifdef SCDM
     tracef("%4d Active(%p)::replaceString(%d,%d,%s)\n", __LINE__, this,
            column, length, string);
   #endif

   result= expand(column+L);
   if( result != NULL )
     return result;

   if( L == length )                // Replacement lengths are identical
     memcpy(text+column, string, L);

   else if( L > length )            // Replacement string is larger
   {
     result= expand(textUsed + L - length - 1);
     if( result != NULL )
       return result;

     memmove(text+column+L, text+column+length, textUsed-(column+length));
     memcpy(text+column, string, L);
   }
   else                             // Replacement string is smaller
   {
     memmove(text+column+L, text+column+length, textUsed-(column+length));
     memcpy(text+column, string, L);
     textUsed -= (length-L);
     text[textUsed]= '\0';
     this->textUsed= textUsed;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::reset
//
// Purpose-
//       Discard the active line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL)
   Active::reset( void )            // Discard the active line
{
   #ifdef SCDM
     tracef("%4d Active(%p)::reset()\n", __LINE__, this);
   #endif

   if( state != FSM_RESET )         // If we have a changed line
   {
     if( text != base )             // If the line is allocated
       memset(text, 0, textSize);   // Zero the string (releasing it)

     state= FSM_RESET;
   }

   text= base;                      // Restore the base state
   textSize= baseSize;

   text[0]= '\0';
   textUsed= 0;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::shrink
//
// Purpose-
//       Remove trailing blanks
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::shrink( void )           // Remove trailing blanks
{
   const char*         result;      // Resultant

   if( textUsed == 0 )
     return NULL;

   result= expand(textUsed-1);
   if( result != NULL )
     return result;

   while(textUsed > 0 && text[textUsed-1] == ' ') // Remove trailing blanks
     --textUsed;

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::store
//
// Purpose-
//       Store (replace) the active line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::store( void )            // Store (replace) the active line
{
   char*             string;        // Working string

   #ifdef SCDM
     tracef("%4d Active(%p)::store() state(%d)\n"
            "    '%s'\n"
            , __LINE__, this, state, text);
     check();
     debug("store");
   #endif

   if( state == FSM_RESET )         // If unchanged
     return NULL;

   if( state == FSM_CHANGE )        // If changed
   {
     ring->changed= TRUE;           // Indicate changed
     shrink();                      // Remove trailing blanks

     // Allocate (or clean up) a new editor line
     memset(text+textUsed, 0, textSize-textUsed);

     if( textUsed == 0 )
       text= NULL;
     else if( base == text )
     {
       string= ring->allocateText(textUsed + 1);
       if( string == NULL )
         return "No storage";

       strcpy(string, text);
       text= string;
     }

     // Remove the old editor line
     string= line->text;
     if( string != NULL )
       ring->releaseText(string);

     // Replace the line text
     line->setText(text);

     // The line is reset
     state= FSM_RESET;
     return fetch(ring, line);
   }

   return "No storage";
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::strip
//
// Purpose-
//       Remove leading and trailing blanks
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::strip( void )            // Remove leading and trailing blanks
{
   const char*         result;      // Resultant
   int                 L;           // Number of leading blanks

   if( textUsed == 0 )
     return NULL;

   result= expand(textUsed-1);
   if( result != NULL )
     return result;

   L= 0;
   while( text[L] == ' ' )
     L++;

   if( L != 0 )
   {
     memmove(text, text+L, textUsed-L+1);
     textUsed -= L;
   }

   return shrink();
}

//----------------------------------------------------------------------------
//
// Method-
//       Active::undo
//
// Purpose-
//       Undo any action on the active line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Active::undo( void )             // Undo any action on the active line
{
   #ifdef SCDM
     tracef("%4d Active(%p)::undo() state(%d)\n", __LINE__, this, state);
   #endif

   return fetch(ring, line);
}
