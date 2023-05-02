//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Object.cpp
//
// Purpose-
//       Object implementation methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Atomic.h>
#include <com/AutoPointer.h>
#include <com/Debug.h>               // For throwf()

#include "com/Object.h"

#ifdef _OS_WIN
  #define vsnprintf _vsnprintf

  #ifndef va_copy
    #define va_copy(dest, src) dest= src
  #endif
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef OBJECT_STATISTICS
#define OBJECT_STATISTICS           // If defined, update Object::objectCount
#endif

#include "com/ifmacro.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
volatile int32_t       Object::objectCount= 0; // Number of allocated Objects

static ATOMIC32        recount= 0;  // The RECLAIM depth counter
static Object* volatile
                       head= NULL;  // The RECLAIM list

//----------------------------------------------------------------------------
//
// Method-
//       Object::objectCounter
//
// Purpose-
//       Update Object::objectCount
//
//----------------------------------------------------------------------------
void
   Object::objectCounter(           // Update the objectCount
     int               count)       // With this count
{
#ifdef OBJECT_STATISTICS
   int32_t             oldCount;    // Old reference counter
   int32_t             newCount;    // New reference counter
   int                 cc;

   do
   {
     oldCount= objectCount;
     newCount= oldCount + count;

     cc= csw(&objectCount, oldCount, newCount);
   } while( cc != 0 );
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::~Object
//       Object::Object
//
// Purpose-
//       Destructor/Constructor
//
//----------------------------------------------------------------------------
// Destructor and all constructors are inline

//----------------------------------------------------------------------------
//
// Method-
//       Object::compare
//
// Purpose-
//       Compare to.
//
//----------------------------------------------------------------------------
int                                 // Result (<0,=0,>0)
   Object::compare(                 // Compare to
     const Object&     source) const // This Object
{
   return (char*)this - (char*)(&source);
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::compareCastException
//
// Purpose-
//       Throw CompareCastException
//
//----------------------------------------------------------------------------
void
   Object::compareCastException(     // Throw CompareCastException
     const char*       text) const   // With this text
{
   throwf("%s(%p)::CompareCastException", text, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::hashf
//
// Purpose-
//       Hash function.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant
   Object::hashf( void ) const      // Hash function
{
   uintptr_t result= uintptr_t(this);
   result >>= 3;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::toString
//
// Purpose-
//       Convert to string.
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   Object::toString( void ) const   // Convert to string
{
   char buffer[32];
   sprintf(buffer, "Object@%p", (void*)this);
   std::string result(buffer);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Ref<Object>::~Ref<Object>
//       Ref<Object>::Ref<Object>
//
// Purpose-
//       Destructor/Constructor
//
//----------------------------------------------------------------------------
// Destructor and all constructors are inline

//----------------------------------------------------------------------------
//
// Method-
//       Ref<Object>::nullPointerException
//
// Purpose-
//       Throw NullPointerException.
//
//----------------------------------------------------------------------------
void
   Ref<Object>::nullPointerException( void ) const // Throw NullPointerException
{
   throwf("Ref(%p)::NullPointerException", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Ref<Object>::set
//
// Purpose-
//       Update the Object*.
//
// Implementation note-
//       This is where *ALL* the object clean up work is driven.
//
//----------------------------------------------------------------------------
void
   Ref<Object>::set(                // Update the Object*
     Object*           newObject)   // With this Object*
{
   IFHCDM( debugf("Ref(%p)::set(%p) %p %d\n", this, newObject, object, recount); )

   int32_t             oldCount;    // Old reference counter
   int32_t             newCount;    // New reference counter
   Object*             oldObject;   // Old -> Object
   int                 cc;

   do
   {
     oldObject= this->object;
     cc= csp((ATOMICP**)&this->object, oldObject, newObject);
   } while( cc != 0 );

   if( (void*)oldObject == (void*)newObject ) // If unchanged
     return;

   if( newObject != NULL )
   {
     do
     {
       oldCount= newObject->refCount;
       newCount= oldCount + 1;
       cc= csw(&newObject->refCount, oldCount, newCount);
     } while( cc != 0 );
   }

   if( oldObject != NULL )
   {
     do
     {
       oldCount= oldObject->refCount;
       newCount= oldCount - 1;
       cc= csw(&oldObject->refCount, oldCount, newCount);
     } while( cc != 0 );

     if( newCount == 0 )            // If no more references exist
     {
       // We cannot simply delete the Object beause the delete can then
       // cascade before we return. Instead, we add it to the reclaim list
       // and update the internal recount value. When that value is zero,
       // we are responsible for all deletes until the stack is empty.
       Object* oldReclaim;
       do
       {
         oldReclaim= head;
         oldObject->reclaim= oldReclaim;
         cc= csp((ATOMICP**)&head, oldReclaim, oldObject);
       } while( cc != 0 );

       do                           // Empty the reclaim list
       {
         //-------------------------------------------------------------------
         // In a multi-threaded environment like this one, it is possible for
         // elements to be added to the restack list after we emptied it but
         // before we can get around to decrementing the stack level.
         //
         // Let's fix this.
         // (Added this do {} while( head != NULL && recount == 0 );)
         //
         // Here's why this might actually work:
         // The retry logic is extremely unlikely to be used at all. If used,
         // when we go back to update the recount recursion level, at least
         // one (sometimes more) of the threads will win the race to begin
         // cleaning up the list again. It doesn't matter which thread wins,
         // because only one will win at a time and any thread can handle
         // the case where there is no work to do. With this logic, we do not
         // have to wait for some new event to finish the clean up process.
         // There is a slim possiblity that this small section of code
         // might get heavily used by multiple threads. Even in this case,
         // *every* thread must eventually succeed and exit the loop.
         // Here's why: Luck always runs out. This applies to bad luck too.
         //-------------------------------------------------------------------

         do                         // Increment the stack level
         {
           oldCount= recount;
           newCount= oldCount + 1;
           cc= csw(&recount, oldCount, newCount);
         } while( cc != 0 );

         if( oldCount == 0 )        // If we *might have* created the list
         {
           for(;;)                  // Empty the reclaim list
           {
             do
             {
               newObject= head;
               cc= csp((ATOMICP**)&head, newObject, NULL);
             } while( cc != 0 );

             if( newObject == NULL )
               break;

             while( newObject != NULL )
             {
               oldObject= newObject;
               newObject= oldObject->reclaim;
               IFHCDM( debugf("Ref(%p)::delete(%p)\n", this, oldObject); )
               delete oldObject;
             }
           }
         }

         do                         // Decrement the stack level
         {
           oldCount= recount;
           newCount= oldCount - 1;
           cc= csw(&recount, oldCount, newCount);
         } while( cc != 0 );
       } while( head != NULL && recount == 0 );
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       format
//
// Purpose-
//       Create string from printf format arguments
//
//----------------------------------------------------------------------------
static std::string                  // Resultant
   stringf(                         // Create string from printf arguments
     const char*       format,      // Format string
     va_list           argptr)      // PRINTF arguments
{
   std::string         result;      // Resultant

   char workBuff[512];              // Working buffer
   char* buffer= workBuff;          // -> Buffer

   va_list outptr;
   va_copy(outptr, argptr);
   unsigned L= vsnprintf(buffer, sizeof(workBuff), format, outptr);
   va_end(outptr);

   if( L < sizeof(workBuff) )       // If the normal case
     result= std::string(buffer);
   else
   {
     AutoPointer work(L+1);         // Allocate a work buffer
     buffer= (char*)work.get();

     vsnprintf(buffer, L, format, argptr);
     result= std::string(buffer);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       String::String(const char*, ...)
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   String::String(                  // Copy constructor
     const char*       format,      // Format string
                       ...)         // PRINTF arguments
:  Cloneable(), std::string()
{
   IFHCDM( debugf("String(%p)::String(%s,...)\n", this, format); )

   va_list             argptr;

   va_start(argptr, format);        // Initialize va_ functions
   *this= stringf(format, argptr);  // Construct via assignment
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       String::String(const char*, va_list...)
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   String::String(                  // Copy constructor
     const char*       format,      // Format string
     va_list           argptr)      // PRINTF arguments
:  Cloneable(), std::string()
{
   IFHCDM( debugf("String(%p)::String(%s,va_list)\n", this, format); )

   *this= stringf(format, argptr);  // Construct via assignment
}

//----------------------------------------------------------------------------
//
// Method-
//       String::clone
//
// Purpose-
//       Duplicate this Object.
//
//----------------------------------------------------------------------------
Cloneable*                          // Resultant
   String::clone( void ) const      // Clone this Object
{
   return new String(*this);
}

//----------------------------------------------------------------------------
//
// Method-
//       String::compare
//
// Purpose-
//       Compare to.
//
//----------------------------------------------------------------------------
int                                 // Result (<0,=0,>0)
   String::compare(                 // Compare to
     const Object&     source) const// This Object
{
   const String* that= dynamic_cast<const String*>(&source);
   if( that == NULL )
     compareCastException("String");

   const unsigned char* L= (const unsigned char*)(this->c_str());
   const unsigned char* R= (const unsigned char*)(that->c_str());

   for(;;)
   {
     if( *L != *R )
       break;

     if( *L == '\0' )
       break;

     L++;
     R++;
   }

   return *L - *R;
}

//----------------------------------------------------------------------------
//
// Method-
//       String::hashf
//
// Purpose-
//       (dbj2) Hash function.
//
//----------------------------------------------------------------------------
unsigned                            // Resultant
   String::hashf( void ) const      // Hash function
{
   unsigned hash= 0;                // Start value not relevant
   const char* C= c_str();          // Refer to C string
   while( *C != '\0' )
   {
     hash= (hash << 5) + hash + (*C++); // hash * 33 + (*C++)
   }

   return hash;
}

//----------------------------------------------------------------------------
//
// Method-
//       String::toString
//
// Purpose-
//       Convert to string.
//
//----------------------------------------------------------------------------
std::string                         // Resultant
   String::toString( void ) const   // Convert to string
{
   return *this;
}

