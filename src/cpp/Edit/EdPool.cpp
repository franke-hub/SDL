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
//       EdPool.cpp
//
// Purpose-
//       EdPool object methods.
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

#include "EdPool.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MIN_SEGMENT           65536 // Minimum Segment size (power of 2)
#define MIN_ELEMENT (sizeof(Segment)) // Minimum element size (power of 2)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define EDBUFF Static_FNE20070101_EdBuff // Unlikely name for external

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#undef  ROUND
#define ROUND(n,pow2) ((n+(pow2-1)) & ~(pow2-1))

#undef  TRUNC
#define TRUNC(n,pow2) (n & ~(pow2-1))

//----------------------------------------------------------------------------
//
// Struct-
//       Segment
//
// Purpose-
//       Define a freelist segment
//
// Notes-
//       Free storage within an EDBUFF is either zeroed or is comprised of
//       freelist Segments.  The character '\0' cannot be part of any text
//       record since it is always used as a text delimiter.
//
//----------------------------------------------------------------------------
struct Segment                      // Freelist segment
{
   Segment*            next;        // Next freelist segment
   unsigned long       size;        // Size of this segment
}; // struct Segment

//----------------------------------------------------------------------------
//
// Class-
//       EDBUFF
//
// Purpose-
//       Define editor text buffer.
//
//----------------------------------------------------------------------------
class EDBUFF : public List<EDBUFF>::Link { // Editor text buffer.
//----------------------------------------------------------------------------
// EDBUFF::Constructors
//----------------------------------------------------------------------------
public:
   ~EDBUFF( void );                 // Destructor
   EDBUFF(                          // Constructor
     unsigned          size,        // Required text size
     unsigned          align = 16); // and this alignment

//----------------------------------------------------------------------------
// EDBUFF::Methods
//----------------------------------------------------------------------------
public:
void
   gc( void );                      // Run garbage collector

char*                               // -> Allocated text string
   allocate(                        // Allocate text from buffer
     unsigned          size,        // Of this length
     unsigned          align = 16); // and this alignment

//----------------------------------------------------------------------------
// EDBUFF::Debugging methods
//----------------------------------------------------------------------------
public:
inline int                          // TRUE iff consistent
   check(                           // Debugging check
     const Segment*    segment) const; // -> Segment

inline int                          // TRUE iff consistent
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EDBUFF::Attributes
//----------------------------------------------------------------------------
public:
   char*               text;        // -> Text area
   unsigned long       size;        // Length(text)
   Segment*            head;        // -> First free block
}; // class EDBUFF

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::check(Segment*)
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
inline int                          // TRUE iff consistent
   EDBUFF::check(                   // Debugging check
     const Segment*    segment) const // -> Segment
{
   if( (char*)segment < (char*)text )
   {
     tracef("%4d EDBUFF(%p)::check(%p) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, text, size, head);
     return FALSE;
   }

   if( ((intptr_t)segment & (MIN_ELEMENT-1)) != 0 )
   {
     tracef("%4d EDBUFF(%p)::check(%p) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, text, size, head);
     return FALSE;
   }

   if( ((char*)segment + sizeof(Segment)) > ((char*)text + size) )
   {
     tracef("%4d EDBUFF(%p)::check(%p) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, text, size, head);
     return FALSE;
   }

   if( ((char*)segment - (char*)text + segment->size) > size )
   {
     tracef("%4d EDBUFF(%p)::check(%p,%lx) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, segment->size, text, size, head);
     return FALSE;
   }

   if( segment->next != NULL && (char*)segment >= (char*)segment->next )
   {
     tracef("%4d EDBUFF(%p)::check(%p,%lx) next(%p) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, segment->size, segment->next, text, size, head);
     return FALSE;
   }

   if( segment->size == 0
       || (segment->size % MIN_ELEMENT) != 0
       || segment->size > size )
   {
     tracef("%4d EDBUFF(%p)::check(%p,%lx) next(%p) text(%p,%lx) head(%p)\n", __LINE__, this,
            segment, segment->size, segment->next, text, size, head);
     return FALSE;
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
inline int                          // TRUE iff consistent
   EDBUFF::check( void ) const      // Debugging check
{
   if( text == NULL || size == 0 )
   {
     tracef("%4d EDBUFF(%p)::check() text(%p,%lx)\n", __LINE__, this,
            text, size);
     return FALSE;
   }

   for(Segment* segment= head; segment != NULL; segment= segment->next)
   {
     if( !check(segment) )
     {
       debug("Checkfail");
       return FALSE;
     }
   }

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EDBUFF::debug(                   // Debugging display
     const char*       message) const // Display message
{
   tracef("%4d EDBUFF(%p)::debug(%s) text(%p,%lx) head(%p)\n",
          __LINE__, this, message, text, size, head);
   dump(text, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::~EDBUFF
//
// Purpose-
//       EDBUFF destructor
//
//----------------------------------------------------------------------------
   EDBUFF::~EDBUFF( void )          // Destructor
{
   if( text != NULL )
   {
     free(text);
     text= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::EDBUFF
//
// Purpose-
//       EDBUFF constructor
//
//----------------------------------------------------------------------------
   EDBUFF::EDBUFF(                  // Constructor
     unsigned          size,        // Required text size
     unsigned          align)       // and this alignment
:  List<EDBUFF>::Link()
,  text(NULL)
,  size(size)
,  head(NULL)
{
   // Adjust size for alignment
   if( align < MIN_ELEMENT )
     align = MIN_ELEMENT;
   if( size < align )
     size= align;
   size += align;
   this->size= size;

   // Allocate the text area
   // If this fails, delete this EDBUFF and throw an exception
   text= (char*)malloc(size);
   if( text == NULL )
   {
     delete this;
     throw "NoStorageException";
   }

   // Format the initial free Segment
   memset(text, 0, size);           // Zero the area

   head= (Segment*)text;
   head->next= NULL;
   head->size= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::allocate
//
// Purpose-
//       Allocate text from this EDBUFF
//
//----------------------------------------------------------------------------
char*                               // -> Allocated string
   EDBUFF::allocate(                // Allocate a string
     unsigned          inputSize,   // Of this length
     unsigned          alignment)   // And this alignment
{
   char*               last1Addr;   // The ending address of a Segment (+1)
   char*               checkAddr;   // An address which might satisfy request
   char*               roundAddr;   // checkAddr & ~(alignment-1)
   unsigned            roundSize;   // Number of bytes to allocate
   Segment*            currS;       // -> Current  Segment
   Segment*            nextS;       // -> Next  Segment
   Segment*            prevS;       // -> Previous Segment

   assert( (alignment & (alignment-1)) == 0 ); // Alignment must be power of 2
   if( alignment < MIN_ELEMENT )
     alignment = MIN_ELEMENT;
   roundSize= ROUND(inputSize, MIN_ELEMENT); // Allocate multiple of elements
   assert( roundSize > 0 );         // No bogus allocation size

   prevS= NULL;                     // No prior Segment
   for(currS= head; currS != NULL; currS= currS->next)
   {
     #if defined(HCDM)
       if( !check(currS) )
       {
         debug("Checkstop");
         return NULL;
       }
     #endif

     last1Addr= (char*)currS + currS->size;
     checkAddr= last1Addr - roundSize;
     roundAddr= (char*)TRUNC((intptr_t)checkAddr, (intptr_t)alignment);
     if( roundAddr >= (char*)currS && roundSize <= currS->size ) // If we can allocate from here
     {
       // By boundary alignment we may have created empty spaces at the
       // beginning and end of this area.  If so, add them to the free list
       if( (char*)currS != roundAddr ) // If free space at beginning
       {
         nextS= (Segment*)roundAddr;
         nextS->next= currS->next;
         nextS->size= currS->size - (roundAddr-(char*)currS);

         currS->next= nextS;
         currS->size= roundAddr - (char*)currS;

         prevS= currS;
         currS= nextS;
       }

       if( checkAddr != roundAddr)  // If free space at end
       {
         nextS= (Segment*)((char*)currS + roundSize);
         nextS->next= currS->next;
         nextS->size= currS->size - roundSize;

         currS->next= nextS;
       }

       // Now the current block is exactly the correct size and alignment
       // Remove it from the free list.
       if( prevS == NULL )
         head= currS->next;
       else
         prevS->next= currS->next;

       // Zero and return the allocated element
       memset(roundAddr, 0, sizeof(Segment));
       return roundAddr;
     }
     prevS= currS;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       EDBUFF::gc
//
// Purpose-
//       Run the garbage collector
//
//----------------------------------------------------------------------------
void
   EDBUFF::gc( void )               // Run garbage collector
{
   Segment*            oldS;        // -> Freelist segment
   Segment*            ptrS;        // -> Freelist segment
   char*               textAddr;    // -> Remaining buffer area
   unsigned            textSize;    // Length(textAddr)
   char*               freeAddr;    // -> Candidate area
   unsigned            freeSize;    // Candidate area length

   #ifdef SCDM
     tracef("%4d EDBUFF(%p)::gc()\n", __LINE__, this);
   #endif

   #ifdef HCDM
     check();
   #endif

   // First, delete any remaining Segments
   ptrS= head;                      // Address first Segment
   while( ptrS != NULL )
   {
     oldS= ptrS;
     ptrS= oldS->next;
     memset(oldS, 0, sizeof(Segment));
   }
   head= NULL;

   // Search for free areas
   textAddr= (char*)text;
   textSize= size;
   oldS= NULL;
   while( textSize > 0 )
   {
//   tracef("textAddr(%p) textSize(%d)\n", textAddr, textSize);

     freeAddr= (char*)memchr(textAddr, '\0', textSize); // Look for '\0'
     if( freeAddr == NULL )
       break;

     if( freeAddr != text )         // If this ends a line
       freeAddr++;                  // Keep the ending delimiter

     // We must start a free area on the proper boundary
     while( ((intptr_t)freeAddr & (MIN_ELEMENT-1)) != 0 )
     {
       if( *freeAddr != '\0' )
         break;

       freeAddr++;
     }
     if( freeAddr >= (textAddr + textSize) )
       break;

     textSize -= (freeAddr - textAddr);
     textAddr= freeAddr;
     if( *freeAddr != '\0' )
       continue;

     // Determine the size of this free area
     while( textSize > 0 )
     {
       if( *textAddr != '\0' )
         break;

       textAddr++;
       textSize--;
     }

     // Add the element to the free list
     freeSize= TRUNC((textAddr - freeAddr), MIN_ELEMENT);
//   tracef("freeAddr(%p) freeSize(%d)\n", freeAddr, freeSize);
     if( freeSize >= MIN_ELEMENT )
     {
       ptrS= (Segment*)freeAddr;
//     ptrS->next= NULL;            // (Already verified NULL)
       ptrS->size= freeSize;

       if( oldS == NULL )
         head= ptrS;
       else
         oldS->next= ptrS;

       oldS= ptrS;
     }
   }

   #ifdef HCDM
     check();
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   EdPool::check( void ) const      // Debugging check
{
   EDBUFF*             buff;        // -> Current buffer

   #ifdef SCDM
     tracef("%4d EDBUFF(%p)::check()\n", __LINE__, this);
   #endif

   if( !isCoherent() )
   {
     tracef("%4d EdPool(%p)::check()\n", __LINE__, this);
     debug("Should Not Occur");
     return;
   }

   // Check the buffer
   for(buff= (EDBUFF*)getHead();
       buff != NULL;
       buff= (EDBUFF*)buff->getNext())
   {
     buff->check();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdPool::debug(                   // Debugging display
     const char*       message) const // Display message
{
   EDBUFF*             buff;        // -> Current buffer

   tracef("%4d EdPool(%p)::debug(%s)\n", __LINE__, this, message);

   // Dump the buffer
   for(buff= (EDBUFF*)getHead();
       buff != NULL;
       buff= (EDBUFF*)buff->getNext())
   {
     tracef("..EDBUFF(%p) size(%lx) head(%p) text(%p)\n",
             buff, buff->size, buff->head, buff->text);
     dump(buff->text, buff->size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::~EdPool
//
// Purpose-
//       EdPool destructor
//
//----------------------------------------------------------------------------
   EdPool::~EdPool( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d EdPool(%p)::~EdPool()\n", __LINE__, this);
   #endif

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::EdPool
//
// Purpose-
//       EdPool constructor
//
//----------------------------------------------------------------------------
   EdPool::EdPool( void )           // Constructor
:  List<void>()
{
   #ifdef HCDM
     debugSetIntensiveMode();
   #endif

   #ifdef SCDM
     tracef("%4d EdPool(%p)::EdPool()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::allocate
//
// Purpose-
//       Allocate string storage from this EdPool
//
//----------------------------------------------------------------------------
char*                               // -> Allocated string
   EdPool::allocate(                // Allocate a string
     unsigned          inputSize,   // Of this length
     unsigned          alignment)   // And this alignment
{
   char*               addr;        // Allocated area
   EDBUFF*             buff;        // -> Current buffer
   unsigned            size;        // EDBUFF allocation size

   // Set minimum alignment
   if( alignment < MIN_ELEMENT )
     alignment= MIN_ELEMENT;

   // Search the ring buffer area for an available element
   for(buff= (EDBUFF*)getHead();
       buff != NULL;
       buff= (EDBUFF*)buff->getNext())
   {
     addr= buff->allocate(inputSize, alignment);
     if( addr != NULL )
       return addr;
   }

   // Garbage collect
   for(buff= (EDBUFF*)getHead();
       buff != NULL;
       buff= (EDBUFF*)buff->getNext())
   {
     buff->gc();
   }

   // Search the ring buffer area again
   for(buff= (EDBUFF*)getHead();
       buff != NULL;
       buff= (EDBUFF*)buff->getNext())
   {
     addr= buff->allocate(inputSize, alignment);
     if( addr != NULL )
       return addr;
   }

   // Allocate a new EDBUFF element
   unsigned const alignment2= alignment + alignment; // Minimum size
   size= ROUND(inputSize, MIN_SEGMENT);
   if( size < alignment2 )
     size= alignment2;

   buff= NULL;
   try {
     buff= new EDBUFF(size, alignment);
   } catch (...) {
     buff= NULL;
   }

   if( buff == NULL )
     return NULL;

   lifo(buff);

   addr= buff->allocate(inputSize, alignment);
   assert( addr != NULL );
   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::release
//
// Purpose-
//       Release text into an EdPool
//
//----------------------------------------------------------------------------
void
   EdPool::release(                 // Release a string
     char*             string)      // This string
{
   if( string == NULL )
     return;

   memset(string, 0, strlen(string));
}

//----------------------------------------------------------------------------
//
// Method-
//       EdPool::reset
//
// Purpose-
//       Release all associated storage
//
//----------------------------------------------------------------------------
void
   EdPool::reset( void )            // Release all associated storage
{
   EDBUFF*             buff;        // Current EDBUFF

   #ifdef SCDM
     tracef("%4d EDBUFF(%p)::reset()\n", __LINE__, this);
   #endif

   for(;;)
   {
     buff= (EDBUFF*)remq();
     if( buff == NULL )
       break;

     delete buff;
   }
}

