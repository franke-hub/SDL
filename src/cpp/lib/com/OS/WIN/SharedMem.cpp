//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/SharedMem.cpp
//
// Purpose-
//       Simulate BSD shared storage methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <windows.h>
#include <sys/stat.h>

#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Exception.h>
#include <com/FileName.h>

#include "com/SharedMem.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SHARED  " // Source filename

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_TOKEN               256 // The number of supported tokens
#define SIZEOF_GLOBAL (sizeof(GlobalObject)*MAX_TOKEN)
#define GLOBAL_CONTROLS (SharedMem::Create | SharedMem::Write)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifdef _OS_WIN
typedef unsigned       key_t;       // ftok key-type
#define sleep          _sleep       // Renamed symbol
#define stat           _stat        // Renamed symbol
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       LocalObject
//
// Purpose-
//       Define the local static attributes.
//
//----------------------------------------------------------------------------
struct LocalObject                  // Static local attributes
{
   SharedMem::Token    token;       // Associated Token
   size_t              size;        // Size of shared storage region

   int                 getCount;    // Number of access()s
   int                 attCount;    // Number of attach()s

   HANDLE              hand;        // Handle for shared storage region
   void*               addr;        // Address of shared storage region
}; // struct LocalObject

//----------------------------------------------------------------------------
//
// Struct-
//       GlobalObject
//
// Purpose-
//       Define the global static attributes.
//
//----------------------------------------------------------------------------
struct GlobalObject                 // Static global attributes
{
   SharedMem::Token    token;       // Associated Token
   size_t              size;        // Size of shared storage region

   int                 count;       // Process use count
   unsigned            _000C;       // Not used - available
}; // struct GlobalObject

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Thread barrier
static LocalObject     localObject[MAX_TOKEN]; // The static local object array

static HANDLE          globalHandle= NULL; // The static global array handle
static GlobalObject*   globalObject= NULL; // The static global object array

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::globalReserve
//
// Purpose-
//       Allocate the global lock.
//
//----------------------------------------------------------------------------
static HANDLE                       // HANDLE for globalRelease
   globalReserve( void )            // Obtain the global lock
{
   HANDLE              hMutex;      // Mutex handle

   hMutex= CreateMutex(             // Create the physical Mutex
               NULL,                // (No security attribute)
               TRUE,                // We are the initial owner
               "SharedMem::globalMutex"); // Name of Mutex
   if( hMutex == NULL )             // If cannot create the Mutex
   {
     errorf("%s %d: Error: CreateMutex(\"globalMutex\")\n",
            __SOURCE__, __LINE__);
     return NULL;
   }

   return hMutex;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::globalRelease
//
// Purpose-
//       Release the global lock.
//
//----------------------------------------------------------------------------
static void
   globalRelease(                   // Release the global lock
     HANDLE            hMutex)      // Global handle
{
   ReleaseMutex(hMutex);
   CloseHandle(hMutex);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::allocateHandle
//
// Purpose-
//       Allocate a shared storage segment.
//
//----------------------------------------------------------------------------
static HANDLE                       // Allocated HANDLE
   allocateHandle(                  // Allocate a storage segment handle
     unsigned          id,          // The segment identifier
     unsigned          size,        // The segment size
     unsigned          flags)       // Controls
{
   char                fileName[256]; // (Dummy) file name
   DWORD               protect;     // Protection attributes
   HANDLE              h;           // File handle

   sprintf(fileName, "C:/WINDOWS/TEMP/SharedMem/%.8X.MAP", id); // Dummy name

   if( (flags&SharedMem::Write) == 0 ) // If read/only access required
     protect= PAGE_READONLY;
   else                             // If read/write access required
     protect= PAGE_READWRITE;

   h= CreateFileMapping(            // Create a file mapping
          (HANDLE)(-1),             // Allocate from paging file
          NULL,                     // No security attributes
          protect,                  // Protection attributes
          0,                        // High order size
          size,                     // Low order size
          fileName);                // Name of file-mapping object
   if( h == NULL )                  // If file mapping failure
     errorf("%s %d: Error: CreateFileMapping(%u,%x)\n",
            __SOURCE__, __LINE__, size, (unsigned)protect);

   return h;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::addressSegment
//
// Purpose-
//       Address a shared storage segment.
//
//----------------------------------------------------------------------------
static void*                        // -> Allocated segment
   addressSegment(                  // Address a storage segment
     HANDLE            h,           // File handle
     unsigned          size)        // The segment size
{
   void*               p;           // -> Storage

   p= MapViewOfFile(                // Point at the file
          h,                        // File Mapping Object
          FILE_MAP_WRITE,           // Read/Write access
          0, 0,                     // High/Low mapping origin
          size);                    // Number of bytes to map
   if( p == NULL )                  // If file mapping failure
     errorf("%s %d: Error: MapViewOfFile(%u)\n",
            __SOURCE__, __LINE__, size);

   return p;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::globalInit
//
// Purpose-
//       Initialize associated global objects.
//
// Notes-
//       The local latch must be held.
//
//----------------------------------------------------------------------------
static void
   globalInit( void )               // Initialize global
{
   HANDLE              hGlobal;     // Global handle
   int                 extant;      // TRUE if already exists
   int                 i;

   #ifdef HCDM
     debugf("%8s= SharedMem::globalInit()\n", "");
   #endif

   // Initialization sequencing
   if( globalObject != NULL )
     return;

   // We are the initializer for this Process
   for(i= 0; i<MAX_TOKEN; i++)      // Initialize the local array
   {
     localObject[i].token=    SharedMem::InvalidToken;
     localObject[i].size=     0;

     localObject[i].getCount= 0;
     localObject[i].attCount= 0;
     localObject[i].hand=     NULL;
     localObject[i].addr=     NULL;
   }

   // We may also be the global initializer
   hGlobal= globalReserve();        // Obtain the global lock
   {
     if( hGlobal == NULL )
       return;
     globalHandle= allocateHandle(0xffffffff, SIZEOF_GLOBAL, GLOBAL_CONTROLS);
     if( globalHandle == NULL )
     {
       globalRelease(hGlobal);
       return;
     }
     extant= FALSE;                 // Default, we are the initializer
     if( GetLastError() == ERROR_ALREADY_EXISTS ) // If we are not the first
       extant= TRUE;                // Indicate already allocated

     globalObject= (GlobalObject*)addressSegment(globalHandle, SIZEOF_GLOBAL);
     if( globalObject == NULL )
     {
       CloseHandle(globalHandle);
       globalHandle= NULL;

       globalRelease(hGlobal);
       return;
     }

     if( !extant )                  // If global initialization required
     {
       #ifdef HCDM
         debugf("%8s= SharedMem::globalInit() !EXTANT\n", "");
       #endif
       for(i= 0; i<MAX_TOKEN; i++)
       {
         globalObject[i].token= SharedMem::InvalidToken;
         globalObject[i].size=  0;
         globalObject[i].count= 0;
       }
     }
   }
   globalRelease(hGlobal);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::localTerm
//
// Purpose-
//       Terminate usage of local objects.
//
// Notes-
//       The local latch must be held.
//
//----------------------------------------------------------------------------
static void
   localTerm( void )                // Terminate
{
   int                 i;

   #ifdef HCDM
     debugf("%8s= SharedMem::localTerm()\n", "");
   #endif

   // Check current local state
   for(i= 0; i<MAX_TOKEN; i++)
   {
     if( localObject[i].getCount > 0 )
       return;
   }

   // We are the terminator for this Process
   #ifdef HCDM
     debugf("%8s= SharedMem::localTerm() process terminator\n", "");
   #endif

   if( globalObject != NULL )       // If a view exists
     UnmapViewOfFile(globalObject); // Release it

   if( globalHandle != NULL )       // If a handle exists
     CloseHandle(globalHandle);     // Release it

   globalObject= NULL;
   globalHandle= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getToken(unsigned)
//
// Purpose-
//       Create a Token from a constant value.
//
//----------------------------------------------------------------------------
SharedMem::Token                    // The Token
   SharedMem::getToken(             // Convert value to Token
     unsigned          identifier)  // Local constant identifier
{
   return Token(identifier);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::getToken
//
// Purpose-
//       Allocate a persistent token.
//
//----------------------------------------------------------------------------
SharedMem::Token                    // Resultant token
   SharedMem::getToken(             // Convert name to token
     const char*       fileName,    // File name
     unsigned          identifier)  // Initializer
{
   unsigned            result;      // Resultant
   struct stat         s;           // File status

   int                 rc;

   FileName target(fileName);
   target.resolve();
   const char* cwd= target.getFileName();
   rc= stat(cwd, &s);               // Get statistics
   if( rc != 0 )                    // If failure
   {
     // If this is a filename argv[0], the .exe may be missing
     cwd= target.append(".exe");    // Try .exe extension
     rc= stat(cwd, &s);             // Get statistics
     if( rc != 0 )                  // If failure
     {
       fprintf(stderr, "Shared::getToken(%s,%d), file non-existant\n",
                       fileName, identifier);
       throw "UsageUserException";
     }
   }

   result= identifier;              // Initialize
   while( *cwd != '\0' )
   {
     result *= 8;
     result += *cwd;
     cwd++;
   }

   #ifdef HCDM
     debugf("%8x= Shared::getToken(%s,%x)\n", result, cwd, identifier);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::access
//
// Purpose-
//       Access a pseudo-shared segment
//
//----------------------------------------------------------------------------
SharedMem::Segment                  // Resultant segment identifier
   SharedMem::access(               // Convert token to segment
     Size_t            size,        // Required size
     Token             token,       // Associated Token
     int               flags)       // Controls
{
   Segment             result;      // Resultant segment
   HANDLE              hGlobal;     // Global handle
   HANDLE              h;           // Segment handle
   int                 i;

   // Initialization sequencing
   AutoBarrier lock(barrier);
   {
     // Parameter checks
     result= InvalidSegment;        // Default, invalid segment
     if( token == InvalidToken )    // If invalid token
       goto trace_exit;             // Error: invalid token

     if( globalObject == NULL )
     {
       globalInit();
       if( globalObject == NULL )
         throw "SystemResourceException";
     }

     hGlobal= globalReserve();      // Obtain the global lock
     if( hGlobal == NULL )
       throw "SystemResourceException";

     // Locate existing segment
     for(i=0; i<MAX_TOKEN; i++)
     {
       if( globalObject[i].token == token )
       {
         if( (flags&Exclusive) != 0 ) // If first use required
           goto exit;               // Error: already created

         if( globalObject[i].size < size ) // If the object is too small
           goto exit;               // Error: existing segment too small

         if( localObject[i].getCount == 0 ) // If local attachment to global
         {
           h= allocateHandle(i, size, flags);// Allocate a handle for it
           if( h == NULL )          // If failure
             goto exit;             // Error: system error

           globalObject[i].count++; // New Process atttach

           localObject[i].token=    token;
           localObject[i].size=     size;

           localObject[i].getCount= 1;
           localObject[i].attCount= 0;
           localObject[i].hand=     h;
           localObject[i].addr=     NULL;
         }

         result= i;                 // Set the resultant segment identifier
         goto exit;
       }
     }

     // Locate existing segment
     if( (flags&Create) == 0 )      // If the segment must exist
       goto exit;                   // Error: segment does not exist

     for(i=0; i<MAX_TOKEN; i++)
     {
       if( globalObject[i].token == InvalidToken )
       {
         h= allocateHandle(i, size, flags);  // Allocate a handle for it
         if( h == NULL )            // If failure
           goto exit;               // Error: system error

         localObject[i].token=    token;
         localObject[i].size=     size;

         localObject[i].getCount= 1;
         localObject[i].attCount= 0;
         localObject[i].hand=     h;
         localObject[i].addr=     NULL;

         globalObject[i].token= token;
         globalObject[i].size=  size;
         globalObject[i].count= 1;

         result= i;                 // Set the resultant segment identifier
         goto exit;
       }
     }

     errorf("%s %d: No free Tokens\n", __SOURCE__, __LINE__);
     result= InvalidSegment;

     // Completion sequencing
exit:
     globalRelease(hGlobal);        // Release the global lock
   }

trace_exit:
   #ifdef HCDM
     debugf("%.8lx= SharedMem::access(%ld,%.8lx,%.8lx)\n",
            (long)result, (long)size, (long)token, (long)flags);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::attach
//
// Purpose-
//       Attach a pseudo-shared segment
//
//----------------------------------------------------------------------------
void*                               // Resultant segment address
   SharedMem::attach(               // Attach pseudo-segment
     Segment           segment)     // Associated segment
{
   void*               result;      // Resultant segment address

   #ifdef HCDM
     debugf("%8s= SharedMem::attach(%.8X)\n", "Started", segment);
   #endif

   // Initialization sequencing
   AutoBarrier lock(barrier);
   {
     if( globalObject == NULL )
     {
       globalInit();
       if( globalObject == NULL )
         throw "SystemResourceException";
     }

     // Consistency checks
     assert( segment < MAX_TOKEN );
     assert( localObject[segment].token != InvalidToken );
     assert( localObject[segment].token == globalObject[segment].token );
     assert( localObject[segment].size  == globalObject[segment].size  );
     assert( localObject[segment].getCount > 0 );

     result= localObject[segment].addr;
     if( result == NULL )
     {
       result= addressSegment(localObject[segment].hand,
                              localObject[segment].size);
       if( result == NULL )
         return NULL;

       localObject[segment].addr= result;
     }

     localObject[segment].attCount++;
   }

   #ifdef HCDM
     debugf("%8p= SharedMem::attach(%.8X)\n", result, segment);
   #endif
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::detach
//
// Purpose-
//       Detach a pseudo-shared segment
//
//----------------------------------------------------------------------------
void
   SharedMem::detach(               // Detach pseudo-segment
     const Address*    addr)        // -> Segment
{
   int                 i;

   #ifdef HCDM
     debugf("%8s= SharedMem::detach(%p)\n", "", addr);
   #endif

   // Initialization sequencing
   AutoBarrier lock(barrier);
   {
     if( globalObject == NULL )
     {
       globalInit();
       if( globalObject == NULL )
         throw "SystemResourceException";
     }

     // Detach the segment
     for(i=0; i<MAX_TOKEN; i++)
     {
       if( localObject[i].addr == addr )
       {
         assert( localObject[i].attCount > 0 );
         localObject[i].attCount--;

         if( localObject[i].attCount == 0 )
         {
           UnmapViewOfFile((void*)addr); // Release the view
           localObject[i].addr= NULL;
         }

         return;
       }
     }
   }
   errorf("%s %4d: SharedMem::detach(%p) not attached\n",
          __SOURCE__, __LINE__, addr);
   throw "UsageUserException";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::remove
//
// Purpose-
//       Remove a pseudo-shared segment
//
//----------------------------------------------------------------------------
void
   SharedMem::remove(               // Remove a segment
     Segment           segment)     // Segment identifier
{
   HANDLE              hGlobal;     // Global handle

   #ifdef HCDM
     debugf("%8s= SharedMem::remove(%.8X)\n", "", segment);
   #endif

   // Initialization sequencing
   AutoBarrier lock(barrier);
   {
     if( globalObject == NULL )
     {
       globalInit();
       if( globalObject == NULL )
         throw "SystemResourceException";
     }

     // Consistency checks
     assert( segment < MAX_TOKEN );
     assert( localObject[segment].token != InvalidToken );
     assert( localObject[segment].token == globalObject[segment].token );
     assert( localObject[segment].size  == globalObject[segment].size  );
     assert( localObject[segment].getCount > 0 );

     // Remove the segment
     localObject[segment].getCount--;
     if( localObject[segment].getCount == 0 )
     {
       assert( localObject[segment].attCount == 0 );

       hGlobal= globalReserve();    // Obtain the global lock
       if( hGlobal != NULL )
       {
         globalObject[segment].count--;
         if( globalObject[segment].count == 0 )
           globalObject[segment].token= InvalidToken;

         globalRelease(hGlobal);    // Release the global lock
       }
       localTerm();                 // See if any local segments remain
     }
   }
}

