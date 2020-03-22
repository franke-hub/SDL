//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EiDB.cpp
//
// Purpose-
//       Exon/Intron DataBase object methods.
//
// Last change date-
//       2002/09/15
//
// Description-
//       This routine is an in-storage database accessor.  It contains
//       methods for inserting lines into the database, extracting lines and
//       for storage allocation control.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "EiDB.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "EIDB    " // Source file

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SEGMENT_SIZE          65536 // Number of lines in a Segment

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Segment
//
// Purpose-
//       Describe line collection.
//
//----------------------------------------------------------------------------
struct Segment {                    // Line array segment
   Segment*            next;        // Next segment in collection
   unsigned            used;        // Number of elements in use
   char*               line[SEGMENT_SIZE]; // Line array
}; // struct Segment

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocateSegment
//
// Purpose-
//       Allocate a Segment
//
//----------------------------------------------------------------------------
Segment*                            // -> Segment
   allocateSegment( void )          // Allocate a Segment
{
   Segment*            ptrS;        // Working Segment pointer

   ptrS= (Segment*)malloc(sizeof(Segment));
   if( ptrS == NULL )
   {
     fprintf(stderr, "No storage(%zu)\n", sizeof(Segment));
     return NULL;
   }
   memset(ptrS, 0, sizeof(*ptrS));

   return ptrS;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       releaseSegment
//
// Purpose-
//       Release a Segment
//
//----------------------------------------------------------------------------
void
   releaseSegment(                  // Release a Segment
     Segment*          ptrS)        // Segment to release
{
   int                 i;

   for(i=0; i<ptrS->used; i++)
     free(ptrS->line[i]);

   free(ptrS);
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::~EiDB
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   EiDB::~EiDB( void )              // Destructor
{
   empty();
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::EiDB
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   EiDB::EiDB( void )               // Constructor
:  largest(0)
,  lineCount(0)
,  headArray(NULL)
,  workIndex(0)
,  workArray(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::getLargest
//
// Purpose-
//       Get the size of the largest line.
//
//----------------------------------------------------------------------------
unsigned                            // The size of the largest line
   EiDB::getLargest( void ) const   // Get size of the largest line
{
   return largest;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::getLineCount
//
// Purpose-
//       Extract the line count.
//
//----------------------------------------------------------------------------
unsigned                            // The line count
   EiDB::getLineCount( void ) const // Get line count
{
   return lineCount;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::getLine
//
// Purpose-
//       Get a line from the database.
//
//----------------------------------------------------------------------------
const char*                         // Return line
   EiDB::getLine(                   // Get line from the database
     unsigned          inpIndex)    // Line index
{
   unsigned            index= inpIndex; // Working line index
   Segment*            ptrS;        // Working Segment pointer

   if( headArray == NULL )
   {
     fprintf(stderr, "EiDB::line(%u), empty\n", index);
     throw "NotOpenException";
   }

   if( index < workIndex )
   {
     workIndex= 0;
     workArray= headArray;
   }

   ptrS= (Segment*)workArray;
   index -= workIndex;
   while( ptrS != NULL )
   {
     if( ptrS->used > index )
       return ptrS->line[index];

     workIndex += ptrS->used;
     index -= ptrS->used;

     ptrS= ptrS->next;
     workArray= ptrS;
   }

   fprintf(stderr, "EiDB::line(%u), lineCount(%u)\n", inpIndex, lineCount);
   throw "RangeException";
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::putLine
//
// Purpose-
//       Add a line into the database.
//
//----------------------------------------------------------------------------
unsigned                            // Line index
   EiDB::putLine(                   // Add line into database
     const char*       line)        // Line to add
{
   char*               copy;        // Line to insert
   Segment*            ptrS;        // Working Segment pointer
   Segment*            newS;        // Working Segment pointer

   ptrS= (Segment*)workArray;
   if( ptrS == NULL )
   {
     headArray= ptrS= allocateSegment();
     workArray= headArray;
     if( ptrS == NULL )
       return unsigned(-1);
   }

   while( ptrS->next != NULL )
   {
     workIndex += ptrS->used;
     workArray= ptrS= ptrS->next;
   }

   if( ptrS->used >= SEGMENT_SIZE )
   {
     workIndex += ptrS->used;
     newS= allocateSegment();
     ptrS->next= newS;
     workArray= ptrS= newS;
     if( ptrS == NULL )
     {
       workIndex= 0;
       workArray= headArray;
       return unsigned(-1);
     }
   }

   // Copy and insert the line
   copy= strdup(line);
   if( copy == NULL )
     return unsigned(-1);

   if( strlen(copy) > largest )
     largest= strlen(copy);

   ptrS->line[ptrS->used]= copy;
   ptrS->used++;
   lineCount++;

   return lineCount;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::empty
//
// Purpose-
//       Empty the database.
//
//----------------------------------------------------------------------------
void
   EiDB::empty( void )              // Empty the database
{
   Segment*            ptrS;        // Working Segment pointer

   while( headArray != NULL )
   {
     ptrS= (Segment*)headArray;
     headArray= ptrS->next;

     releaseSegment(ptrS);
   }

   workArray= NULL;
   workIndex= 0;
   lineCount= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       EiDB::trim
//
// Purpose-
//       Trim the database.
//
//----------------------------------------------------------------------------
int                                 // Number of lines released
   EiDB::trim( void )               // Trim the database
{
   int                 result;      // Resultant
   Segment*            ptrS;        // Working Segment pointer
   Segment*            oldS;        // Working Segment pointer

   // Find the segment to remove
   ptrS= (Segment*)headArray;
   if( ptrS == NULL )
     return (-1);

   oldS= NULL;
   while( ptrS->next != NULL )
   {
     oldS= ptrS;
     ptrS= ptrS->next;
   }
   result= ptrS->used;

   // Remove the segment from the list
   if( oldS == NULL )
   {
     headArray= NULL;
     lineCount -= result;
   }
   else
     oldS->next= NULL;

   // Reset workArray
   workArray= headArray;
   workIndex= 0;

   // Delete the last Segment
   releaseSegment(ptrS);

   return result;
}

