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
//       EdRing.cpp
//
// Purpose-
//       EdRing object methods.
//
// Last change date-
//       2020/10/03 (Version 2, Release 1) - Extra compiler warnings
//
//----------------------------------------------------------------------------
#include <new>
#include <assert.h>
#include <inttypes.h>               // For PRI*64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <com/Media.h>
#include <com/nativeio.h>
#include <com/syslib.h>

#include "EdRing.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define __SOURCE__       "EDRING  " // Source file

#include <com/ifmacro.h>            // After #define HCDM

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
static const char*       CRCRLF = "\r\r\n"; // For delimiter testing
static const char*       NULLS4 = "\0\0\0"; // For delimiter testing

//----------------------------------------------------------------------------
//
// Subroutine-
//       delimiter
//
// Purpose-
//       Get default delimiter for mode
//
//----------------------------------------------------------------------------
static inline int                   // The default delimiter
   delimiter(                       // Get default delimiter
     int               mode)        // For this mode
{
   int                 result;      // Resultant

   if( mode == EdRing::FM_UNIX )
     result= EdLine::DT_LF;
   else if( mode == EdRing::FM_DOS )
     result= EdLine::DT_CRLF;
   else
   {
     #ifdef _OS_WIN
       result= EdLine::DT_CRLF;
     #else
       result= EdLine::DT_LF;
     #endif
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   EdRing::check( void ) const      // Debugging check
{
#ifdef HCDM
   EdLine*             curLine;     // Detected firstLine
   EdLine*             firstLine;   // Detected firstLine
   EdLine*             line;        // Working Line
   EdLine*             prev;        // Working Line
   int                 rowx;        // Row number

   int                 i;

   curLine= NULL;
   firstLine= NULL;
   prev= NULL;
   line= (EdLine*)lineList.getHead();
   for(rowx= 0; ; rowx++)
   {
     if( line == this->curLine )
       curLine= this->curLine;

     if( line == this->firstLine )
       firstLine= this->firstLine;

     if( line == NULL )
     {
       if( rows == rowx && rows > cacheRow )
         break;

       if( rows != rowx )
         tracef("%4d EdRing(%p)::check, rows(%d) rowx(%d)\n",
                __LINE__, this, rows, rowx);

       if( rows <= cacheRow )
         tracef("%4d EdRing(%p)::check, rows(%d) cacheRow(%d) cacheLine(%p)\n",
                __LINE__, this, rows, cacheRow, cacheLine);

       debug();
       throw "InternalLogicError";
     }

     if( line == cacheLine )
     {
       if( rowx != cacheRow )
       {
         tracef("%4d EdRing(%p)::check, cacheRow(%d) cacheLine(%p) rowx(%d)\n",
                __LINE__, this, cacheRow, cacheLine, rowx);
         throw "InternalLogicError";
       }
     }
     else if( rowx == cacheRow )
     {
       tracef("%4d EdRing(%p)::check, cacheRow(%d) cacheLine(%p) line(%p)\n",
              __LINE__, this, cacheRow, cacheLine, line);
       throw "InternalLogicError";
     }

     if( prev != line->getPrev() )
     {
       tracef("%4d EdRing(%p)::check, rowx(%d) line(%p) prev(%p) getPrev(%p)\n",
              __LINE__, this, rowx, line, prev, line->getPrev());
       throw "InternalLogicError";
     }

     prev= line;
     line= (EdLine*)line->getNext();
   }

   if( curLine == NULL || firstLine == NULL )
   {
     if( curLine == NULL )
       tracef("%4d EdRing(%p)::check, curLine(%p) not found\n",
              __LINE__, this, this->curLine);

     if( firstLine == NULL )
       tracef("%4d EdRing(%p)::check, firstLine(%p) not found\n",
              __LINE__, this, this->firstLine);

     throw "InternalLogicError";
   }

   if( undoCount > MAX_UNDO )
   {
     tracef("%4d EdRing(%p)::check(), undoCount(%d)\n",
            __LINE__, this, undoCount);
     debug();
     throw "InternalLogicError";
   }

   for(i= 0; i<undoCount; i++)
   {
     line= undoArray[i];
     if( line == NULL )
     {
       tracef("%4d EdRing(%p)::check(), undoCount(%d) undo[%d] NULL\n",
              __LINE__, this, undoCount, i);
       debug();
       throw "InternalLogicError";
     }
   }
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       EdRing::check
//
// Purpose-
//       Verify the consistency of an EdLine list
//       (The range must reside within the list.)
//
//----------------------------------------------------------------------------
#ifdef HCDM
void
   EdRing::check(
     const char*       filenm,      // Verify that Lines are contained
     int               lineno,      // Caller's file name
     const EdLine*     head,        // First Line of range
     const EdLine*     tail) const  // Final Line of range
{
   EdLine*             line;        // Working Line

   for(line= (EdLine*)lineList.getHead();
       line != NULL;
       line= (EdLine*)line->getNext())
   {
     if( line == head )
       break;

     if( line == tail )
     {
       tracef("%4d EdRing(%p)::check(%s,%d), tail(%p) before head(%p)\n",
              __LINE__, this, filenm, lineno, tail, head);
       debug("check");
       throw "InternalLogicError";
     }
   }

   if( line == NULL )
   {
     tracef("%4d EdRing(%p)::check(%s,%d), head(%p) missing\n",
            __LINE__, this, filenm, lineno, head);
     debug("check");
     throw "InternalLogicError";
   }

   for(; line != NULL; line= (EdLine*)line->getNext())
   {
     if( line == tail )
       break;
   }

   if( line == NULL )
   {
     tracef("%4d EdRing(%p)::check(%s,%d), tail(%p) missing\n",
            __LINE__, this, filenm, lineno, tail);
     debug("check");
     throw "InternalLogicError";
   }
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdRing::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   tracef("%4d EdRing(%p)::debug(%s) "
          "rows(%d) undo(%d) mode(%d) type(%d) chg(%d) err(%d)\n"
          "    '%s/%s'\n"
          , __LINE__, this, message
          , rows, undoCount, mode, type, changed, damaged
          , pathName, fileName);

#if FALSE // Display the file
   const EdLine* line= (EdLine*)lineList.getHead();
   int row= 0;
   while( line != NULL )
   {
     if( curLine == line )
       tracef("%p [%2d] ****(%s)\n", line, row, line->text);
     else
       tracef("%p [%2d] Line(%s)\n", line, row, line->text);

     line= (EdLine*)line->getNext();
     row++;
   }
#endif

#if FALSE // Display the undo array
   for(int undoIndex= undoCount-1; undoIndex>=0; undoIndex--)
   {
     EdLine* undoL= undoArray[undoIndex];
     tracef("Undo array[%d] %p\n", undoIndex, undoL);
     while(undoL != NULL)
     {
       tracef(": %.8lx %.8lx %.8lx %s\n", long(undoL),
              long(undoL->getPrev()), long(undoL->getNext()),  undoL->text);
       undoL= undoL->getNext();
     }
     tracef("*\n");
   }
#endif

#else                               // Parameter ignored without HCDM
   (void)message;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::~EdRing
//
// Purpose-
//       EdRing destructor
//
//----------------------------------------------------------------------------
   EdRing::~EdRing( void )          // Destructor
{
   IFHCDM( tracef("%4d EdRing(%p)::~EdRing()\n", __LINE__, this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::EdRing
//
// Purpose-
//       EdRing constructor
//
//----------------------------------------------------------------------------
   EdRing::EdRing( void )           // Constructor
:  List<EdRing>::Link()
,  linePool()
,  textPool()
,  lineList()
,  topOfFile()
,  botOfFile()
{
   IFHCDM( tracef("%4d EdRing(%p)::EdRing()\n", __LINE__, this); )

   next= prev= NULL;
   reset();
}

   EdRing::EdRing(                  // Constructor
     const char*       fileName)    // File name
:  List<EdRing>::Link()
,  linePool()
,  textPool()
,  lineList()
,  topOfFile()
,  botOfFile()
{
   IFHCDM( tracef("%4d EdRing(%p)::EdRing(%s)\n", __LINE__, this, fileName); )

   reset();
   if( strlen(fileName) < sizeof(this->fileName) )
     strcpy(this->fileName, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::allocateText
//
// Purpose-
//       Allocate text string from pool
//
//----------------------------------------------------------------------------
char*                               // -> Allocated text string
   EdRing::allocateText(            // Allocate text string
     unsigned          size)        // Of this size
{
   return textPool.allocate(size);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::append
//
// Purpose-
//       Append file after specified line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdRing::append(                  // Append
     const char*       fileName,    // This file
     EdLine*           edLine)      // After this line
{
   FileMedia           file;        // The FileMedia object
   EdLine*             line;        // Working line
   EdLine*             next;        // Working line

   unsigned            L;           // Number of bytes
   unsigned            size;        // Data length
   char*               text;        // Data area
   char*               textStart;   // Data area

   int                 rc;

   IFHCDM(
     tracef("%4d EdRing::append(%s,%s)\n", __LINE__,
            fileName, edLine->getText());
   )

   FileInfo info(fileName);         // File Info
   if( !info.exists() )             // If non-existent
     return "Non-existent";

   if( info.isPath() )              // If directory
     return "Folder";

   if( edLine->getNext() == NULL )
     return "Protected";

   size= info.getFileSize();
   if( size != info.getFileSize() ) // If size > 32 bits
   {
     damaged= TRUE;
     IFHCDM(
       tracef("%4d EdRing::append() size(%d) info(%" PRId64 ")\n", __LINE__,
              size, info.getFileSize());
     )
     return "File too large";
   }

   if( size == 0 )                  // If empty file
     return NULL;

   text= textPool.allocate(size+1, 4096);
   if( text == NULL )
   {
     IFHCDM( tracef("%4d EdRing::append() size(%d)\n", __LINE__, size); )
     damaged= TRUE;
     return "No storage";
   }

   rc= file.open(fileName, file.MODE_READ); // Open the file
   if( rc != 0 )
   {
     damaged= TRUE;
     memset(text, 0, size);
     textPool.release(text);
     return "Open failure";
   }

   L= file.read(text, size);
   if( L != size )
   {
     damaged= TRUE;
     file.close();
     memset(text, 0, size);
     textPool.release(text);
     return "Read failure";
   }

   //-------------------------------------------------------------------------
   // Parse the text into lines
   // Performance critical path
   // :
   text[size]= '\0';
   while( size > 0 )
   {
     // Allocate a new line
     line= insertLine(edLine);
     if( line == NULL )
     {
       memset(text,0,size);
       damaged= TRUE;
       return "No storage";
     }
     edLine= line;

     // Set the line
     textStart= text;
     while( size > 0 )
     {
       if( *text == '\n' )
       {
         *text= '\0';
         line->ctrl.delim= EdLine::DT_LF;
         if( mode == FM_DOS )
           mode= FM_MIXED;
         break;
       }

       if( *text == '\r' )
       {
         *text= '\0';
         if( size >= 2 && *(text+1) == '\n' )
         {
           line->ctrl.delim= EdLine::DT_CRLF;
           if( mode == FM_UNIX )
             mode= FM_MIXED;
           text++;
           size--;
         }
         else if( size >= 3 && memcmp(text+1, CRCRLF+1, 2) == 0 )
         {
           line->ctrl.delim= EdLine::DT_CRCRLF;
           mode= FM_BINARY;
           text += 2;
           size -= 2;
         }
         else if( size >= 4 && memcmp(text+1, CRCRLF, 3) == 0 )
         {
           line->ctrl.delim= EdLine::DT_CRCRCRLF;
           mode= FM_BINARY;
           text += 3;
           size -= 3;
         }
         else
         {
           line->ctrl.delim= EdLine::DT_CR;
           mode= FM_BINARY;
         }

         break;
       }

       if( *text == '\0' )
       {
         if( size >= 4 && memcmp(text+1, NULLS4, 3) == 0 )
         {
           line->ctrl.delim= EdLine::DT_NUL4;
           text += 3;
           size -= 3;
         }
         else if( size >= 3 && memcmp(text+1, NULLS4, 2) == 0 )
         {
           line->ctrl.delim= EdLine::DT_NUL3;
           text += 2;
           size -= 2;
         }
         else if( size >= 2 && *(text+1) == '\0' )
         {
           line->ctrl.delim= EdLine::DT_NUL2;
           text++;
           size--;
         }
         else
           line->ctrl.delim= EdLine::DT_NULL;

         mode= FM_BINARY;
         break;
       }

       text++;
       size--;
     }

     if( *textStart != '\0' )
       line->text= textStart;
     if( size == 0 )
     {
       mode= FM_BINARY;
       line->ctrl.delim= EdLine::DT_NONE;
       return "Last line incomplete";
     }

     text++;
     size--;
   }

   // RESET mode implies only DOS or UNIX mode lines detected.
   if( mode == FM_RESET )
   {
     line= (EdLine*)lineList.getHead(); // Top of file line
     next= (EdLine*)line->getNext(); // First actual file line
     if( next->getNext() != NULL )  // If some data line present
     {
       if( next->ctrl.delim == EdLine::DT_CRLF )
       {
         mode= FM_DOS;
         for(;;)
         {
           line= next;
           next= (EdLine*)line->getNext();
           if( next == NULL )
             break;

           if( line->ctrl.delim != EdLine::DT_CRLF )
           {
             mode= FM_MIXED;
             break;
           }
         }
       }
       else
       {
         mode= FM_UNIX;
         for(;;)
         {
           line= next;
           next= (EdLine*)line->getNext();
           if( next == NULL )
             break;

           if( line->ctrl.delim != EdLine::DT_LF )
           {
             mode= FM_MIXED;
             break;
           }
         }
       }
     }
   }
   // :
   // Performance critical path
   //-------------------------------------------------------------------------

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::contains
//
// Purpose-
//       Determine whether EdRing contains the specified file
//
//----------------------------------------------------------------------------
int                                 // Return code (TRUE || FALSE)
   EdRing::contains(                // Does this EdRing
     const char*       fileDesc) const // Contain this file?
{
   int                 result= 0;
   char                workPath[FILENAME_MAX+1]; // Resolved path
   char                workName[FILENAME_MAX+1]; // Resolved name

   if( FileName::resolve(workName,fileDesc) != NULL )
     result= 1;

   else if( FileName::getPathOnly(workPath,workName) == NULL )
     result= 2;

   else if( FileName::getNamePart(workName,fileDesc) == NULL )
     result= 3;

   else if( FileName::compare(this->pathName,workPath) != 0 )
     result= 4;

   else if( FileName::compare(this->fileName,workName) != 0 )
     result= 5;

   #if 0
     tracef("HCDM %d= ring:contains(%s)\n", result, fileDesc);
     tracef("HCDM thisPath(%s) thisName(%s)\n"
            "HCDM thatPath(%s) thatName(%s)\n\n",
            this->pathName, this->fileName, workPath, workName);
   #endif

   return (result == 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::deleteList
//
// Purpose-
//       Delete a list of EdLines that have been removed.
//
//----------------------------------------------------------------------------
void
   EdRing::deleteList(              // Delete EdLine list
     EdLine*           edLine)      // List of lines, NULL terminated
{
   EdLine*             next;        // Working line pointer
   EdLine*             line;        // Working line pointer

   IFHCDM( tracef("%4d EdRing(%p)::deleteList(%p)\n", __LINE__, this, edLine); )

   //-------------------------------------------------------------------------
   // Delete the list of lines
   //-------------------------------------------------------------------------
   line= edLine;
   for(;;)
   {
     next= (EdLine*)line->getNext();

     if( line->text != NULL )
     {
       textPool.release(line->text);
       line->text= NULL;
     }

     line->~EdLine();
     linePool.release(line, sizeof(*line));

     if( next == NULL )
       break;
     line= next;
   }

   resetCache();        
   changed= TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::insertLine
//
// Purpose-
//       Allocate and insert new, empty EdLine
//
//----------------------------------------------------------------------------
EdLine*                             // -> New, empty EdLine
   EdRing::insertLine(              // Allocate and insert
     EdLine*           edLine)      // After this line
{
   EdLine*             line;        // -> EdLine
   void*               ptrVoid;     // -> EdLine storage

   #if defined(HCDM) && FALSE       // Performance sensitive test
     check(__SOURCE__, __LINE__, edLine, edLine);
   #endif

   if( edLine->getNext() == NULL )  // If this is the last line
     return NULL;                   // Cannot insert after it

   ptrVoid= linePool.allocate(sizeof(EdLine));
   if( ptrVoid == NULL )
     return NULL;

   line= new(ptrVoid) EdLine();
   lineList.insert(edLine, line, line);
   rows++;

   if( ((EdLine*)line->getPrev())->ctrl.marked
       &&((EdLine*)line->getNext())->ctrl.marked )
     line->ctrl.marked= TRUE;

   if( ((EdLine*)line->getPrev())->ctrl.hidden
       &&((EdLine*)line->getNext())->ctrl.hidden )
     line->ctrl.hidden= TRUE;

   line->ctrl.delim= delimiter(mode);

   resetCache();                
   changed= TRUE;
   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::read
//
// Purpose-
//       Load the ring
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdRing::read(                    // Load the ring
     const char*       fileName)    // Using this filename
{
   const char*         result;      // Return message

   char                workName[FILENAME_MAX+1]; // Resolved name

   reset();                         // Reset the ring
   result= FileName::resolve(workName, fileName);
   FileInfo fileInfo(workName);
   if( result != NULL || fileInfo.isPath() )
   {
     damaged= TRUE;
     type= FT_UNUSABLE;
     strcpy(this->pathName, workName);
     strcpy(this->fileName, (result != NULL) ? result : "<NONAME>");

     result= "Cannot read";
     if( fileInfo.isPath() )
       result= "Folder";

     return result;
   }

   FileName::getPathOnly(this->pathName, workName);
   FileName::getNamePart(this->fileName, workName);
   result= append(workName, (EdLine*)lineList.getHead());
   if( !damaged )
     changed= FALSE;

   type= FT_DATA;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::releaseText
//
// Purpose-
//       Release text string into pool
//
//----------------------------------------------------------------------------
void
   EdRing::releaseText(             // Release text string
     char*             addr)        // At this address
{
   textPool.release(addr);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::removeLine
//
// Purpose-
//       Remove EdLines from the EdRing and delete them
//
//----------------------------------------------------------------------------
void
   EdRing::removeLine(              // Remove and delete
     EdLine*           head,        // From this line
     EdLine*           tail)        // To this line
{
   EdLine*             line;        // Working line pointer

   IFHCDM(
     tracef("%4d EdRing(%p)::removeLine(%p,%p)\n", __LINE__, this, head, tail);
     check(__SOURCE__, __LINE__, head, tail);
   )

   lineList.remove(head, tail);     // Remove the lines from the ring
   tail->setNext(NULL);             // Make sure the chain is ended

   for(line= head; line != NULL; line=(EdLine*)line->getNext())
     rows--;

   deleteList(head);                // Delete the list
}

void
   EdRing::removeLine(              // Remove and delete
     EdLine*           edLine)      // This line
{
   removeLine(edLine, edLine);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::removeUndo
//
// Purpose-
//       Remove lines with undo capability.
//
//----------------------------------------------------------------------------
void
   EdRing::removeUndo(              // Insert lines onto the undo list
     EdLine*           head,        // From this line
     EdLine*           tail)        // To this line
{
   EdLine*             line;        // Working line pointer

   int                 i;

   IFHCDM(
     tracef("%4d EdRing(%p)::removeUndo(%p,%p)\n", __LINE__, this, head, tail);
     check(__SOURCE__, __LINE__, head, tail);
     debug("removeUndo-Entry");
   )

   //-------------------------------------------------------------------------
   // If the undo array is full, empty the oldest element
   //-------------------------------------------------------------------------
   if( undoCount == MAX_UNDO )
   {
     line= undoArray[0];
     deleteList(line);

     for(i= 0; i<(MAX_UNDO-1); i++)
       undoArray[i]= undoArray[i+1];
     undoCount--;
   }

   //-------------------------------------------------------------------------
   // Remove the list from the ring
   //-------------------------------------------------------------------------
   lineList.remove(head, tail);     // This does not change head->getPrev()
   tail->setNext(NULL);             // Make sure the chain is ended

   line= head;
   while( line != NULL )            // Decrement the row count
   {
     rows--;
     line= (EdLine*)line->getNext();
   }

   //-------------------------------------------------------------------------
   // Add the list to the undo array
   //-------------------------------------------------------------------------
   undoArray[undoCount++]= head;

   resetCache();
   changed= TRUE;

   IFHCDM( debug("removeUndo-Exit"); check(); )
}

void
   EdRing::removeUndo(              // Insert lines onto the undo list
     EdLine*           edLine)      // Using this line
{
   removeUndo(edLine, edLine);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::reset
//
// Purpose-
//       Reset (empty) the ring.
//
//----------------------------------------------------------------------------
void
   EdRing::reset( void )            // Reset (empty) the ring
{
   // Reset variables and objects
   memset(pathName, 0, sizeof(pathName));
   memset(fileName, 0, sizeof(fileName));
   memset(autoName, 0, sizeof(autoName));

   mode= FM_RESET;
   type= FT_RESET;
   changed= damaged= FALSE;

   linePool.reset();
   textPool.reset();
// rows= 0;
   lineList.reset();

   undoCount= 0;
   resetCache();

   // Construct the Lines
   topOfFile.ctrl.readonly= TRUE;
   topOfFile.text= (char*)"* * * * Top of file * * * *";
   botOfFile.ctrl.readonly= TRUE;
   botOfFile.text= (char*)"* * * * End of file * * * *";

   // Initialize the Ring
   lineList.fifo(&topOfFile);
   lineList.fifo(&botOfFile);
   rows= 2;

   // Reset the View
   curLine= firstLine= &topOfFile;
   curRow= curCol= firstCol= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::resetCache
//
// Purpose-
//       Reset the rownumber cache.
//
//----------------------------------------------------------------------------
void
   EdRing::resetCache( void )       // Reset  the row number cache
{
   cacheRow= 0;
   cacheLine= &topOfFile;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::resetUndo
//
// Purpose-
//       Reset the UNDO array, releasing any saved lines.
//
//----------------------------------------------------------------------------
void
   EdRing::resetUndo( void )        // Reset the UNDO array
{
   EdLine*             line;        // Working line pointer

   IFHCDM(
     tracef("%4d EdRing(%p)::resetUndo()\n", __LINE__, this);
     check();
     debug("resetUndo");
   )

   //-------------------------------------------------------------------------
   // Reset the undo array
   //-------------------------------------------------------------------------
   for(unsigned i= 0; i<undoCount; i++)
   {
     line= undoArray[i];
     deleteList(line);
   }
   undoCount= 0;

   IFHCDM( debug("resetUndo-HCDM"); check(); )
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::rowNumber
//
// Purpose-
//       Determine whether EdRing contains the specified line
//
//----------------------------------------------------------------------------
int                                 // Row number, (-1) if not present
   EdRing::rowNumber(               // Does this EdRing
     EdLine*           edLine)      // Contain this EdLine?
{
   int                 rowx;        // Resultant row number
   EdLine*             line;        // Working EdLine

   #ifdef HCDM
     const char*       info;        // Debugging information
     check();

     tracef("%4d EdRing(%p)::rowNumber(%p=%s)...\n", __LINE__,
            this, edLine, edLine ? edLine->getText() : "N/A");
   #endif

   if( edLine == NULL )             // Fast path for NULL line
     return (-1);

   line= cacheLine;
   if( line != NULL )
   {
     if( edLine == (EdLine*)line->getPrev() )
     {
       cacheRow--;
       cacheLine= edLine;
       IFHCDM(
         tracef("%4d EdRing(%p)::rowNumber(%p) %d-\n", __LINE__,
                this, edLine, cacheRow);
       )
       return cacheRow;
     }

     if( edLine == (EdLine*)line->getNext() )
     {
       cacheRow++;
       cacheLine= edLine;
       IFHCDM(
         tracef("%4d EdRing(%p)::rowNumber(%p) %d+\n", __LINE__,
                this, edLine, cacheRow);
       )
       return cacheRow;
     }
   }

   IFHCDM( info= "="; )
   for(rowx= cacheRow; line != NULL; rowx++)
   {
     if( line == edLine )
       break;

     IFHCDM( info= ">"; )
     line= (EdLine*)line->getNext();
   }

   if( line != edLine )
   {
     IFHCDM( info= "<"; )
     line= (EdLine*)lineList.getHead();
     for(rowx= 0; ; rowx++)
     {
       if( line == edLine || line == cacheLine  || line == NULL )
         break;

       line= (EdLine*)line->getNext();
     }
   }

   if( line != edLine )
   {
     IFHCDM( info= "\?"; )
     rowx= (-1);
   }
   else
   {
     cacheRow=  rowx;
     cacheLine= line;
   }

   IFHCDM(
     tracef("%4d %d= EdRing(%p)::rowNumber(%p) %d[%s]\n", __LINE__,
            rowx, this, edLine, cacheRow, info);
   )

   return rowx;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::undo
//
// Purpose-
//       Undo a remove operation.
//
//----------------------------------------------------------------------------
const char*                         // Return message
   EdRing::undo(                    // Undo remove operation
     EdLine*&          head,        // First undo line
     EdLine*&          tail)        // Final undo line
{
   EdLine*             line;        // Working line
   EdLine*             next;        // Working line
   unsigned            count;       // Number of restore lines
   int                 mark;        // Restore mark

   IFHCDM( tracef("%4d EdRing(%p)::undo()\n", __LINE__, this); )

   if( undoCount == 0 )
     return "Cannot undo";

   undoCount--;
   head= undoArray[undoCount];
   line= (EdLine*)head->getPrev();
   IFHCDM( check(__SOURCE__, __LINE__, line, line); )
   mark= FALSE;
   if( line->ctrl.marked && ((EdLine*)line->getNext())->ctrl.marked )
     mark= TRUE;

   count= 1;
   tail= head;
   for(;;)
   {
     tail->ctrl.marked= mark;
     tail->ctrl.delim= delimiter(mode);
     next= (EdLine*)tail->getNext();
     if( next == NULL )
       break;

     tail= next;
     count++;
   }

   next= (EdLine*)line->getNext();
   head->setPrev(line);
   tail->setNext(next);
   line->setNext(head);
   next->setPrev(tail);
   rows += count;

   resetCache();        
   changed= TRUE;
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRing::write
//
// Purpose-
//       Write (save) the ring
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   EdRing::write(                   // Write (save) the ring
     const char*       fileName)    // Output filename
{
   const char*         result;      // Return message
   EdLine*             line;        // Current line
   int                 size;        // Length of text line
   int                 L;           // Actual data length
   int                 outh;        // File handle
   const char*         text;        // -> Text

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   outh= open(fileName,             // Open the file
              O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,// (in write-only binary mode)
              S_IREAD|S_IWRITE);    // (with full write access)
   if( outh < 0 )                   // If we cannot open the output file
     return "Open failed";

   //-------------------------------------------------------------------------
   // Write the file
   //-------------------------------------------------------------------------
   result= NULL;                    // No error yet
   line= (EdLine*)lineList.getHead()->getNext();
   while( line->getNext() != NULL )
   {
     size= line->getSize();
     if( size > 0 )
     {
       L= ::write(outh, line->getText(), size);
       if( L != size )
       {
         result= "I/O error";
         break;
       }
     }

     text= NULL;
     size= 1;
     switch( line->ctrl.delim )
     {
       case EdLine::DT_NULL:
         text= NULLS4;
         break;

       case EdLine::DT_NUL2:
         text= NULLS4;
         size= 2;
         break;

       case EdLine::DT_NUL3:
         text= NULLS4;
         size= 3;
         break;

       case EdLine::DT_NUL4:
         text= NULLS4;
         size= 4;
         break;

       case EdLine::DT_CR:
         text= "\r";
         break;

       case EdLine::DT_LF:
         text= "\n";
         break;

       case EdLine::DT_CRLF:
         text= "\r\n";
         size= 2;
         break;

       case EdLine::DT_CRCRLF:
         text= CRCRLF;
         size= 3;
         break;

       case EdLine::DT_CRCRCRLF:
         text= "\r\r\r\n";
         size= 4;
         break;

       case EdLine::DT_NONE:
         size= 0;
         break;

       default:                     // Internal coding error
         damaged= TRUE;
         line->ctrl.delim= EdLine::DT_NONE;
         result= "Internal logic error";
         size= 0;
         break;
     }

     if( size > 0 )
     {
       L= ::write(outh, text, size);
       if( L != size )
       {
         result= "I/O error";
         break;
       }
     }

     line= (EdLine*)line->getNext();
   }

   //-------------------------------------------------------------------------
   // Close the file
   //-------------------------------------------------------------------------
   if( close(outh) != 0 )           // If close failure
     result= "Close failed";

   return result;
}

const char*                         // Return message (NULL OK)
   EdRing::write( void )            // Write (save) the ring
{
   const char*         result;      // Return message
   char                fileName[LEN_PN + LEN_FN + 2];
   const char*         pathName;    // Working path name

   int                 i;

   if( type != FT_DATA )
     return "Protected";

   //-------------------------------------------------------------------------
   // Find an available name for the file
   //-------------------------------------------------------------------------
   pathName= getenv("AUTOSAVE");    // Environment autosave path
   if( pathName == NULL )           // If not specified
     pathName= this->pathName;      // Use current path

   if( autoName[0] == '\0' )        // If autosave name unassigned
   {
     for(i=0; i<1000; i++)          // Allocate an autosave name
     {
       sprintf(autoName, "AUTOSAVE.%.3d", i); // Set autosave name
       FileName::concat(fileName, sizeof(fileName), pathName, autoName);
       FileInfo fileInfo(fileName); // File information
       if( fileInfo.exists() == FALSE )
         break;
     }

     // We overwrite AUTOSAVE.999 if necessary
   }

   //-------------------------------------------------------------------------
   // Save the file using autoName
   //-------------------------------------------------------------------------
   FileName::concat(fileName, sizeof(fileName), pathName, autoName);
   result= write(fileName);
   if( result != NULL )
     return result;

   //-------------------------------------------------------------------------
   // Overwrite the file
   //-------------------------------------------------------------------------
   FileName::concat(fileName, sizeof(fileName), this->pathName, this->fileName);
   result= write(fileName);
   if( result != NULL )
     return result;

   //-------------------------------------------------------------------------
   // Delete the autoName file
   //-------------------------------------------------------------------------
   FileName::concat(fileName, sizeof(fileName), pathName, autoName);
   remove(fileName);
   autoName[0]= '\0';

   resetUndo();
   changed= FALSE;
   damaged= FALSE;

   if( mode == FM_RESET )
   {
     #ifdef _OS_WIN
       mode= FM_DOS;
     #else
       mode= FM_UNIX;
     #endif
   }

   return NULL;
}

