//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Debug.cpp
//
// Purpose-
//       Debug object methods.
//
// Last change date-
//       2021/04/02
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>               // For PRIx64
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/RecursiveBarrier.h>
#include <com/Clock.h>              // For Clock::current()
#include <com/Software.h>           // For Software::getTid
#include <com/Thread.h>             // For Thread::yield

#include "com/Debug.h"

#ifdef _OS_WIN
  #include <windows.h>              // For GetCurrentThreadId
  #include <io.h>                   // For isatty

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

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DEBUG_FILE      "debug.out" // Default debug file name

#define FSM_FIRST                 0 // FIRST - first file line
#define FSM_UNDUP                 1 // UNDUP - not duplicating
#define FSM_INDUP                 2 // INDUP - duplicating lines

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define FALSE                     0 // Boolean FALSE
#define TRUE                      1 // Boolean TRUE

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define L2P(l) ((void*)(l))
#define P2L(p) ((unsigned long)(p))

#define trunc(x,m) (((x) / (m)) * (m))// Truncate x at m
#define round(x,m) trunc(((x)+(m)-1), (m))// Round x to next m

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Debug*                 Debug::debug= NULL; // -> Default debug object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static RecursiveBarrier  barrier= RECURSIVEBARRIER_INIT; // Recursive latch
static char            buffer[512]; // Work buffer (barrier protected)
static int             isInternal= FALSE; // Is Debug::debug allocated?

//           111111111122222222223333333333444444444455555555556666666666
// 0123456789012345678901234567890123456789012345678901234567890123456789
// xxxxxxxx..xxxxxxxx.xxxxxxxx.xxxxxxxx.xxxxxxxx..|................|
static short           charPos[16]= // Character position for dump format
{   0,  2,  4,  6
,   9, 11, 13, 15
,  18, 20, 22, 24
,  27, 29, 31, 33
};

#define STATIC_DESTRUCTOR DebugStaticDestructor_Xniuvjiw_uuqr_NiMiralwU8KL
class STATIC_DESTRUCTOR {
public:
   ~STATIC_DESTRUCTOR( void )
{
   delete(Debug::set(NULL));
}
};
static STATIC_DESTRUCTOR debugStaticDestructor;

//----------------------------------------------------------------------------
//
// Subroutine-
//       isDIFFER
//
// Purpose-
//       Do the output FILE and handle FILE differ?
//
//----------------------------------------------------------------------------
static int                          // TRUE iff different
   isDIFFER(                        // Do the output and handle FILEs differ?
     FILE*             output,      // The output handle
     FILE*             handle)      // The trace handle
{
   // Files do not differ if both are tty
   if( isatty(fileno(output)) && isatty(fileno(handle)) )
     return FALSE;

   return output != handle;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::~Debug
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Debug::~Debug( void )            // Destructor
{
   IFHCDM( fprintf(stderr, "Debug(%p)::~Debug()\n", this); )

   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::Debug
//
// Function-
//       Constructor.
//
// Implementation notes-
//       The constructor MUST NOT call init(), which would defeat the
//       Logger append init method.
//
//----------------------------------------------------------------------------
   Debug::Debug(                    // Constructor
     const char*       name)        // The output filename, default "debug.out"
:  handle(NULL)
,  mode(ModeStandard)
,  chain(ChainOnly)
,  fsm(FSM_FIRST)
,  oldAddr(0)
,  newAddr(0)
{
   IFHCDM(
     fprintf(stderr, "Debug(%p)::Debug(%s)\n", this, name == NULL ? "debug.out" : name);
   )

   if( name == NULL || strlen(name) >= sizeof(fileName) )
     name= "debug.out";
   strcpy(fileName, name);
   memset(oldData, 0, sizeof(oldData));
   memset(newData, 0, sizeof(newData));

   IFHCDM( mode= ModeIntensive; )
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::get
//
// Function-
//       Extract the current default debug object.
//
//----------------------------------------------------------------------------
Debug*                              // -> Current default debug object
   Debug::get( void )               // Extract the current default debug object
{
   Debug* result= Debug::debug;
   if( result == NULL )
   {
     AutoRecursiveBarrier lock(barrier);

     result= Debug::debug;
     if( result == NULL )
     {
       Debug::debug= result= new Debug();
       isInternal= TRUE;
     }

     #if defined(HCDM) || FALSE
       fprintf(stderr, "%p= Debug(*)::get()\n", result);
     #endif
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::init
//
// Function-
//       Activate the trace file.
//
// Implementation nodes-
//       Caller must hold Barrier latch.
//
//----------------------------------------------------------------------------
void
   Debug::init( void )              // Activate the trace file
{
   IFHCDM( fprintf(stderr, "Debug(%p)::init()\n", this); )

   if( handle == NULL )             // If still not active
   {
     if( isSTDIO(fileName) )
     {
       if( fileName[0] == '>' || fileName[0] == '1' )
         handle= stdout;
       else
         handle= stderr;
     }
     else
     {
       handle= fopen(fileName, "w");// Open the trace file
       if( handle == NULL )         // If the open failed
       {
         fprintf(stderr, "DEBUG: Error: file(%s) open error\n", fileName);
         handle= stderr;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::isSTDIO
//
// Purpose-
//       Does the fileName imply a STDIO FILE?
//
//----------------------------------------------------------------------------
int                                 // TRUE iff STDIO
   Debug::isSTDIO(                  // Does fileName imply STDIO?
     const char*       fileName)    // The file name
{
   if( fileName[0] == '>' && fileName[1] == '\0' )
     return TRUE;

   if( fileName[0] == '1' || fileName[0] == '2' )
   {
     if( fileName[1] == '>' && fileName[2] == '\0' )
       return TRUE;
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::obtain
//
// Purpose-
//       Obtain the RecursiveBarrier latch
//
//----------------------------------------------------------------------------
int                                 // Return code (0 iff first holder)
   Debug::obtain( void )            // Obtain the RecursiveBarrier latch
{
   return barrier.obtain();
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::release
//
// Purpose-
//       Release the RecursiveBarrier latch
//
//----------------------------------------------------------------------------
void
   Debug::release( void )           // Release the RecursiveBarrier latch
{
   barrier.release();
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::set
//
// Function-
//       Update the default debug object.
//
//----------------------------------------------------------------------------
Debug*                              // The removed Debug object
   Debug::set(                      // Set
     Debug*            object)      // This new default debug object
{
   #if defined(HCDM) || FALSE
     fprintf(stderr, "Debug(*)::set(%p) %p %d\n",
                     object, Debug::debug, isInternal);
   #endif

   AutoRecursiveBarrier lock(barrier);
   Debug* removed = Debug::debug;

   if( isInternal )
   {
     isInternal= FALSE;
     delete Debug::debug;
     removed= NULL;
   }

   Debug::debug= object;
   return removed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::term
//
// Function-
//       Deactivate the trace file.
//
//----------------------------------------------------------------------------
int                                 // TRUE if no data lost
   Debug::term( void )              // Deactivate the trace file
{
   int                 rc= TRUE;    // Resultant, return code

   IFHCDM( fprintf(stderr, "Debug(%p)::term()\n", this); )

   AutoRecursiveBarrier lock(barrier);

   if( handle != NULL && handle != stdout && handle != stderr) // If close required
   {
     rc= fclose(handle);            // Close the file
     if( rc != 0 )                  // If error encountered
       fprintf(stderr, "DEBUG: Error: file(%s), close error(%d)\n",
                       fileName, rc);

     rc= (rc == 0);                 // Function return code
   }

   handle= NULL;                    // Indicate closed
   if( this == debug )              // If this is the default
   {
     debug= NULL;                   // No default exists
     isInternal= FALSE;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vlogf
//
// Function-
//       Debugging (trace only) vprintf facility, with heading
//
//----------------------------------------------------------------------------
void
   Debug::vlogf(                    // Debug trace logf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   AutoRecursiveBarrier lock(barrier);

   tracef("%14.3f ", Clock::current()); // Time of day heading
   vtracef(fmt, argptr);
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vtracef
//
// Function-
//       Debugging (trace only) vprintf facility.
//
//----------------------------------------------------------------------------
void
   Debug::vtracef(                  // Debug trace printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != ModeIgnore )         // If not ignore mode
   {
     AutoRecursiveBarrier lock(barrier);

     if( handle == NULL )           // If trace file not already open
       init();                      // Open it now

     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == ModeIntensive )    // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vdebugf
//
// Function-
//       Debugging (stdout + trace) vprintf facility.
//
//----------------------------------------------------------------------------
void
   Debug::vdebugf(                  // Debug printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != ModeIgnore )         // If not ignore mode
   {
     AutoRecursiveBarrier lock(barrier);

     if( handle == NULL )           // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stdout, handle) )
     {
       va_list outptr;
       va_copy(outptr, argptr);
       vfprintf(stdout, fmt, outptr); // Write to stdout
       va_end(outptr);
     }
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == ModeIntensive )    // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::verrorf
//
// Function-
//       Debugging (stderr + trace) vprintf facility.
//
//----------------------------------------------------------------------------
void
   Debug::verrorf(                  // Debug error printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != ModeIgnore )         // If not ignore mode
   {
     AutoRecursiveBarrier lock(barrier);

     if( handle == NULL )           // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stderr, handle) )
     {
       va_list errptr;
       va_copy(errptr, argptr);
       vfprintf(stderr, fmt, errptr); // Write to stderr
       va_end(errptr);
     }
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == ModeIntensive )    // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vthrowf
//
// Function-
//       Debugging (stderr + throw) vprintf facility
//
// Implementation notes-
//       DO NOT write to trace file. This results in unwanted debug.out files.
//
//----------------------------------------------------------------------------
void
   Debug::vthrowf(                  // Debug vprintf exception facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   {{{{
     AutoRecursiveBarrier lock(barrier);
     va_list errptr;
     va_copy(errptr, argptr);
     vfprintf(stderr, fmt, errptr); // Write to stderr
     va_end(errptr);

     fprintf(stderr, "\n");
   }}}}
   fflush(stderr);

   int L= vsnprintf(buffer, sizeof(buffer), fmt, argptr);
   if( L < 0 || size_t(L) >= sizeof(buffer) ) // If cannot properly format
     throw fmt;                     // Just throw format string

   throw buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::logf
//
// Function-
//       Debugging (trace only) printf facility, with heading
//
//----------------------------------------------------------------------------
void
   Debug::logf(                     // Debug tracef facility, with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogf(fmt, argptr);              // Message with heading
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::tracef
//
// Function-
//       Debugging (trace only) printf facility.
//
//----------------------------------------------------------------------------
void
   Debug::tracef(                   // Debug trace printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::debugf
//
// Function-
//       Debugging (stdout + trace) printf facility.
//
//----------------------------------------------------------------------------
void
   Debug::debugf(                   // Debug printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::errorf
//
// Function-
//       Debugging (stderr + trace) printf facility.
//
//----------------------------------------------------------------------------
void
   Debug::errorf(                   // Debug error printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::throwf
//
// Function-
//       Debugging (stderr + trace + throw) printf facility
//
//----------------------------------------------------------------------------
void
   Debug::throwf(                   // Debug printf exception facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vthrowf(fmt, argptr);            // ALWAYS THROWS EXCEPTION
   va_end(argptr);                  // Close va_ functions

   throw "ShouldNotOccur";          // Should not occur
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::dump
//
// Function-
//       Debugging storage dump.
//
// Implementation notes-
//       Variable "oldAddr" contains both the dump origin address and the
//       address where a duplicate range may have begun.  Its offset value
//       (oldAddr&0x000f) contains the number of bytes which have not been
//       initialized in "oldData".
//
//       Variable "newAddr" contains the current dump address, but is set
//       only when a dump chain is incomplete.  Its offset value contains
//       the number of bytes which have been initialized in "newData".
//
//----------------------------------------------------------------------------
void
   Debug::dump(                     // Debug storage dump (to file)
     FILE*             handle,      // Output handle
     const void*       paddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     Chain             chain)       // Debug::Chain
{
   char                output[128]; // Format work area

   unsigned int        offset;      // Offset (0 if not first line)
   unsigned int        length;      // A length
   unsigned int        i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   AutoRecursiveBarrier lock(barrier);

   #ifdef SCDM
     debugf("::dump((%p,%lx), %p, %d) fsm(%d)\n",
            paddr, size, vaddr, chain, fsm);
   #endif

   if( (chain&ChainFirst) != 0 )    // If this request begins a chain
   {
     if( (this->chain&ChainLast) == 0 )
     {
       errorf("%4d %s Chain fault old(%d) new(%d)\n", __LINE__, __FILE__,
              this->chain, chain);
       dump(handle, paddr, 0, newAddr, ChainLast);
     }
     fsm= FSM_FIRST;
     oldAddr= vaddr;
   }
   else                             // If this request continues a chain
   {
     if( (this->chain&ChainLast) != 0 )
     {
       errorf("%4d %s Chain fault old(%d) new(%d)\n", __LINE__, __FILE__,
              this->chain, chain);
       dump(handle, paddr, 0, newAddr, ChainLast);
       fsm= FSM_FIRST;
       oldAddr= vaddr;
     }
     else if( vaddr != newAddr )
     {
       errorf("%4d %s Origin reset old(%p) new(%p)\n",
              __LINE__, __FILE__, newAddr, vaddr);
       dump(handle, paddr, 0, newAddr, ChainLast);
       fsm= FSM_FIRST;
       oldAddr= vaddr;
     }
   }

   //-----------------------------------------------------------------------
   // Initialize the data line
   //-----------------------------------------------------------------------
   offset= (intptr_t)vaddr & 15;
   length= 16 - offset;
   if( length > size )
     length= size;

   if( length > 0 )
     memcpy(newData+offset, paddr, length);

   vaddr= (void*)((intptr_t)vaddr & (-16));
   size  += offset;

   #ifdef SCDM
     debugf("vaddr(%p) offset(%d) size(%ld) fsm(%d)\n",
            vaddr, offset, size, fsm);
   #endif

   //-------------------------------------------------------------------------
   // Format lines
   //-------------------------------------------------------------------------
   while( size > 0 )                // Format lines
   {
     if( size < 16 )                // If the line is not full
     {
       if( (chain&ChainLast) == 0 ) // If not end of dump
       {
         vaddr= (char*)vaddr + size;
         break;
       }
     }

     // At this point:
     //   1) The remaining size is >= 16, or
     //   2) This is the last line to be dumped.

     switch(fsm)                    // Process by state
     {
       case FSM_FIRST:              // If FIRST file line
         break;                     // Cannot go into UNDUP state

       case FSM_UNDUP:              // If UNDUP state
         if( size <= 16 )           // If this is the last line
           break;                   // Do not go into INDUP state

         if( memcmp(newData, oldData, 16) == 0 ) // If duplicate line
           fsm= FSM_INDUP;          // Go into duplicate state
         break;

       case FSM_INDUP:              // If INDUP state
         if( size < 16 || memcmp(newData, oldData, 16) != 0 )
         {
           fsm= FSM_UNDUP;
           if( sizeof(void*) > 4 )
             fprintf(handle, "%.16" PRIX64 "  to %.16" PRIX64
                             ", lines same as above\n",
                             (int64_t)(void*)(intptr_t)oldAddr,
                             (int64_t)(void*)(intptr_t)vaddr-1);
           else
             fprintf(handle, "%.8lX  to %.8lX, lines same as above\n",
                             (long)(intptr_t)oldAddr, (long)(intptr_t)vaddr-1);
         }

         break;

       default:                     // Otherwise
         throwf("%4d %s internal error\n", __LINE__, __FILE__);
     }

     switch(fsm)                    // Process by state
     {
       case FSM_FIRST:              // If FIRST file line
       case FSM_UNDUP:              // If UNDUP state
         if( size == 0 )
           break;

         memcpy(oldData, newData, 16); // Save the string for compare

         for(i=0; i<16; i++)
           sprintf(output+charPos[i], "%.2X ", newData[i]&0x00ff);

         for(i=0; i<16; i++)
         {
           if( !isprint(newData[i]) )
             newData[i]= '.';
         }

         offset= (intptr_t)oldAddr & 15; // Invalid data count
         for(i=0; i<offset; i++)    // Handle first line
         {
           newData[i]= '~';
           output[charPos[i]]= '~';
           output[charPos[i]+1]= '~';
         }

         for(i=size; i<16; i++)     // Handle last line
         {
           newData[i]= '~';
           output[charPos[i]]= '~';
           output[charPos[i]+1]= '~';
         }

         if( sizeof(void*) > 4 )
           fprintf(handle, "%.16" PRIX64 "  %s |%.16s|\n",
                           (int64_t)(intptr_t)vaddr, output, newData);
         else
           fprintf(handle, "%.8lX  %s |%.16s|\n",
                           (long)(intptr_t)vaddr, output, newData);

         oldAddr= vaddr;            // Save duplicate origin (zero offset)
         if( offset == 0 )
           fsm= FSM_UNDUP;

         break;

       case FSM_INDUP:              // If INDUP state
         break;

       default:                     // Otherwise
         throwf("%4d %s internal error\n", __LINE__, __FILE__);
     }

     // Set for next line
     if( size < 16 )                // If the line was not full
       break;                       // It must have been end of dump

     paddr= (void*)((char*)paddr + (16-offset));
     vaddr= (char*)vaddr + 16;
     size  -= 16;
     offset = 0;

     if( size >= 16 )
       memcpy(newData, paddr, 16);
     else if( size > 0 )
       memcpy(newData, paddr, size);
   }

   if( fsm == FSM_INDUP ) {         // If INDUP state
     if( sizeof(void*) > 4 )
       fprintf(handle, "%.16" PRIX64 "  to %.16" PRIX64
                       ", lines same as above\n",
                       (int64_t)(void*)(intptr_t)oldAddr,
                       (int64_t)(void*)(intptr_t)vaddr-1);
     else
       fprintf(handle, "%.8lX  to %.8lX, lines same as above\n",
                       (long)(intptr_t)oldAddr, (long)(intptr_t)vaddr-1);
   }

   newAddr= vaddr;
   this->chain= chain;
}

void
   Debug::dump(                     // Debug storage dump (to trace)
     const void*       paddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     Chain             chain)       // Chaining control
{
   if( handle == NULL )
     init();

   dump(handle, paddr, size, vaddr, chain);
}

void
   Debug::dump(                     // Debug storage dump (to trace)
     FILE*             handle,      // Output handle
     const void*       paddr,       // Storage origin
     unsigned long     size)        // Storage length
{
   dump(handle, paddr, size, paddr);
}

void
   Debug::dump(                     // Debug storage dump (to trace)
     const void*       paddr,       // Storage origin
     unsigned long     size)        // Storage length
{
   if( handle == NULL )
     init();

   dump(handle, paddr, size, paddr);
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::flush
//
// Function-
//       Force trace file to disk.
//
//----------------------------------------------------------------------------
void
   Debug::flush( void )             // Flush trace file to disk
{
   AutoRecursiveBarrier lock(barrier);

   if( handle != NULL )             // If trace is active
   {
     fflush(handle);                // Flush the trace file

     if( handle != stdout && handle != stderr )
     {
       int rc= fclose(handle);      // Close the trace file
       if( rc != 0 )                // If the close failed
         fprintf(stderr, "DEBUG: Error: file(%s) close error\n", fileName);

       handle= fopen(fileName, "a+"); // Re-open the trace file
       if( handle == NULL )         // If the re-open failed
       {
         fprintf(stderr, "DEBUG: Error: file(%s) open(\"a+\") error\n", fileName);
         handle= stderr;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::setName
//
// Function-
//       Name the trace file.
//
//----------------------------------------------------------------------------
void
   Debug::setName(                  // Name the trace file
     const char*       fname)       // The trace filename
{
   IFHCDM( fprintf(stderr, "Debug(%p)::setName(%s)\n", this, fname); )

   term();                          // Deactivate trace

   if( strlen(fname) >= sizeof(fileName) ) // If invalid trace name
     fprintf(stderr, "DEBUG: Error: setName(%s) too long\n", fname);
   else
     strcpy(fileName, fname);       // Set the trace filename
}

extern "C" {
//----------------------------------------------------------------------------
//
// Subsection-
//       C-Language function calls.
//
// Purpose-
//       C-Language environment calls operate against the default
//       debug object.
//
//----------------------------------------------------------------------------
void
   vtraceh(                         // Debug vtracef facility, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   Debug::get()->vlogf(fmt, argptr);
}

void
   vtracef(                         // Debug vtracef facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   Debug::get()->vtracef(fmt, argptr);
}

void
   vdebugf(                         // Debug vdebugf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   Debug::get()->vdebugf(fmt, argptr);
}

void
   verrorf(                         // Debug verrorf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   Debug::get()->verrorf(fmt, argptr);
}

void
   vthrowf(                         // Debug vthrowf exception facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   Debug::get()->vthrowf(fmt, argptr);
   throw "ShouldNotOccur";
}

void
   traceh(                          // Debug trace printf facility, with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtraceh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   tracef(                          // Debug trace printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   debugf(                          // Debug debug printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   errorf(                          // Debug error printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   throwf(                          // Debug printf exception facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vthrowf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   debugFlush( void )               // Flush write the trace file
{
   Debug::get()->flush();
}

void
   debugSetName(                    // Name the trace file
     const char*       fname)       // The trace filename
{
   Debug::get()->setName(fname);
}

void
   debugSetIgnoreMode( void )       // Set Ignore mode
{
   Debug::get()->setMode(Debug::ModeIgnore);
}

void
   debugSetIntensiveMode( void )    // Set Intensive mode
{
   Debug::get()->setMode(Debug::ModeIntensive);
}

void
   debugSetStandardMode( void )     // Set Standard mode
{
   Debug::get()->setMode(Debug::ModeStandard);
}

extern void
   dump(                            // Diagnostic dump (to trace)
     const void*       paddr,       // Storage origin
     unsigned long     size)        // Storage length
{
   FILE* handle= Debug::get()->getHandle();
   Debug::get()->dump(handle, paddr, size, paddr, Debug::ChainOnly);
}

extern void
   dumpv(                           // Diagnostic dump (to trace)
     const void*       paddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     int               chain)       // Chaining control
{
   FILE* handle= Debug::get()->getHandle();
   Debug::get()->dump(handle, paddr, size, vaddr, Debug::Chain(chain));
}

extern void
   snap(                            // Diagnostic dump (to stdout)
     const void*       paddr,       // Storage origin
     unsigned long     size)        // Storage length
{
   Debug::get()->dump(stdout, paddr, size);
}

extern void
   snapv(                           // Diagnostic dump (to stdout)
     const void*       paddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     int               chain)       // Chaining control
{
   Debug::get()->dump(stdout, paddr, size, vaddr, Debug::Chain(chain));
}
} // extern "C"
