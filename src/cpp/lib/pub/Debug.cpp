//----------------------------------------------------------------------------
//
//       Copyright (C) 2007-2022 Frank Eskesen.
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
//       2022/04/08
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard, ...
#include <sstream>                  // For std::stringstream
#include <thread>                   // For std::this_thread

#include <assert.h>                 // For debugging
#include <errno.h>                  // For errno
#include <inttypes.h>               // For PRIx64
#include <stdarg.h>                 // For va_list, ...
#include <stdio.h>                  // For FILE I/O
#include <time.h>                   // For clock_gettime, timespec, ...
#include <unistd.h>                 // For isatty

#define BOOST_STACKTRACE_LINK       // (Use static library)
#define BOOST_STACKTRACE_USE_BACKTRACE
#include <boost/stacktrace.hpp>     // For boost::stacktrace::stacktrace()

#include <pub/utility.h>            // For utility::to_string
#include <pub/Interval.h>           // For performance debugging
#include <pub/Latch.h>              // For Mutex performance comparison
#include <pub/Named.h>              // For Named Threads
#include <pub/Thread.h>             // For Threads

#include "pub/Debug.h"              // For pub::Debug, implemented

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
// Dependent macros
//----------------------------------------------------------------------------
#include <pub/ifmacro.h>

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Debug*                 Debug::common= nullptr; // -> The common Debug instance

int                    debugging::options::opt_check= false;
int                    debugging::options::opt_hcdm= false;
int                    debugging::options::opt_verbose= -1;

int                    debugging::options::pub_check= false;
int                    debugging::options::pub_hcdm= false;
int                    debugging::options::pub_verbose= -1;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
#if false
static std::recursive_mutex mutex;  // Recursive serialization Latch
#else
static RecursiveLatch  mutex;       // Recursive serialization Latch
#endif
static Debug*          internal= nullptr; // The auto-allocated Debug object

static struct GlobalDestructor {    // On unload, remove Debug::global
inline
   ~GlobalDestructor( void )
{  Debug::set(nullptr); }           // Cleans up both Debug::common & internal
}  globalDestructor;

//----------------------------------------------------------------------------
//
// Subroutine-
//       isDIFFER
//
// Purpose-
//       Do the output FILE and handle FILE differ?
//
//----------------------------------------------------------------------------
static bool                         // true iff different
   isDIFFER(                        // Do the output and handle FILEs differ?
     FILE*             output,      // The output handle
     FILE*             handle)      // The trace handle
{
   bool differ= true;               // Default, files differ
   if( output != handle ) {         // If they are not the same file
     // Files do not differ if both are tty
     int ERRNO= errno;              // Linux isatty can set errno
     if( isatty(fileno(output)) && isatty(fileno(handle)) )
       differ= false;
     errno= ERRNO;                  // (Preserving errno)
   }

   return differ;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isSTDIO
//
// Purpose-
//       Does a file name imply a STDIO FILE?
//
//----------------------------------------------------------------------------
static bool                         // true iff STDIO
   isSTDIO(                         // Does file name imply STDIO?
     const char*       file_name)   // The file name
{
   if( file_name[0] == '>' && file_name[1] == '\0' )
     return true;

   if( file_name[0] == '1' || file_name[0] == '2' )
   {
     if( file_name[1] == '>' && file_name[2] == '\0' )
       return true;
   }

   return false;
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
{  IFHCDM( fprintf(stderr, "Debug(%p)::~Debug()\n", this); )
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
//       Constructors MUST NOT call init(), which would defeat the purpose of
//       the set_file_name and set_file_mode functions.
//
//----------------------------------------------------------------------------
   Debug::Debug(                    // Constructor
     const char*       name)        // The output file_name, default "debug.out"
{  IFHCDM( fprintf(stderr, "Debug(%p)::Debug(%s)\n", this, name); )
   if( name != nullptr && name[0] != '\0' )
     this->file_name= name;

   IFHCDM( mode= MODE_INTENSIVE; )
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::debug
//
// Function-
//       Internal debugging method
//
//----------------------------------------------------------------------------
void
   Debug::debug(                    // Internal debugging message
     const char*       info)        // Caller information
{
#if 0
   if( info )                       // If caller information present
     printf("%s: ", info);
   if( common ) {
     printf("common(%p)->handle(%p)\n", common, common->handle );
   } else {
     printf("common(%p)\n", common);
   }
#else                               // No function, parameters ignored
   (void)info;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::heading
//
// Function-
//       Write debugging heading.
//
// Implementation notes-
//       The mutex Latch must be held.
//
//----------------------------------------------------------------------------
void
   Debug::heading(                  // Debug heading
     FILE*             file)        // The target FILE
{
   if( head & HEAD_TIME )           // Time of day heading
   {
     struct timespec     ticker;    // UTC time base
     clock_gettime(CLOCK_REALTIME, &ticker);
     double tod= (double)ticker.tv_sec;
     tod      += (double)ticker.tv_nsec / 1000000000.0;

     fprintf(file, "%14.3f ", tod);
   }

   if( head & HEAD_THREAD )         // Thread heading
   {
     Thread* current= Thread::current();
     Named* named= dynamic_cast<Named*>(current);
     if( named )
       fprintf(file, "<%13s> ", named->get_name().c_str());
     else
     {
       if( sizeof(void*) == 8 )
         fprintf(file, "<@%.12lx> ", (unsigned long)(uintptr_t)current);
       else
         fprintf(file, "<@%.8lx> ",  (unsigned long)(uintptr_t)current);
     }
   }
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
//       Caller must hold mutex Latch.
//
//----------------------------------------------------------------------------
void
   Debug::init( void )              // Activate the trace file
{  IFHCDM( fprintf(stderr, "Debug(%p)::init()\n", this); )

   if( handle == nullptr )          // If still not active
   {
     if( isSTDIO(file_name.c_str()) )
     {
       if( file_name[0] == '>' || file_name[0] == '1' )
         handle= stdout;
       else
         handle= stderr;
     }
     else
     {
       handle= fopen(file_name.c_str(), file_mode.c_str());// Open the trace file
       if( handle == nullptr )      // If the open failed
       {
         fprintf(stderr, "DEBUG: Error: fopen(%s,%s) error\n",
                         file_name.c_str(), file_mode.c_str());
         handle= stderr;
       }
     }
   }
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
void
   Debug::term( void )              // Deactivate the trace file
{  IFHCDM( fprintf(stderr, "Debug(%p)::term()\n", this); )

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( handle != nullptr && handle != stdout && handle != stderr) // If close required
   {
     int rc= fclose(handle);        // Close the file
     if( rc != 0 )                  // If error encountered
       fprintf(stderr, "DEBUG: Error: file(%s), close error(%d)\n",
                       file_name.c_str(), rc);
   }

   handle= nullptr;                 // Indicate closed
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
   Debug* result= Debug::common;
   if( result == nullptr )
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     result= Debug::common;
     if( result == nullptr )
     {
       Debug::common= result= new Debug();
       internal= result;
     }

     #if defined(HCDM) || false
       fprintf(stderr, "%p= Debug(*)::get()\n", result);
     #endif
   }

   return result;
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
   #if defined(HCDM) || false
     fprintf(stderr, "Debug(*)::set(%p) %p %d\n",
                     object, Debug::common, isInternal);
   #endif

   std::lock_guard<decltype(mutex)> lock(mutex);
   Debug* removed = Debug::common;

   if( removed == internal )
   {
     delete internal;
     internal= nullptr;

     removed= nullptr;
   }

   Debug::common= object;
   return removed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::lock
//
// Purpose-
//       Lock the static mutex
//
//----------------------------------------------------------------------------
void
   Debug::lock( void )              // Lock the mutex
{  return mutex.lock(); }

//----------------------------------------------------------------------------
//
// Method-
//       Debug::unlock
//
// Purpose-
//       Unlock the static mutex
//
//----------------------------------------------------------------------------
void
   Debug::unlock( void )            // Unlock the mutex
{  mutex.unlock(); }

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
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( handle != nullptr )          // If trace is active
   {
     fflush(handle);                // Flush the trace file

     if( handle != stdout && handle != stderr )
     {
       int rc= fclose(handle);      // Close the trace file
       if( rc != 0 )                // If the close failed
         fprintf(stderr, "DEBUG: Error: file(%s) close error\n", file_name.c_str());

       handle= fopen(file_name.c_str(), "ab"); // Re-open the trace file
       if( handle == nullptr )      // If the re-open failed
       {
         fprintf(stderr, "DEBUG: Error: file(%s) open(\"ab\") error\n"
                       , file_name.c_str());
         handle= stderr;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::set_file_mode
//
// Function-
//       Set the trace file modee
//
//----------------------------------------------------------------------------
void
   Debug::set_file_mode(            // Set the trace file mode
     const char*       mode)        // The trace file mode
{  IFHCDM( fprintf(stderr, "Debug(%p)::setMode(%s)\n", this, name); )
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( handle )                     // If file is open
     throw Exception(
         utility::to_string("Debug(%p)::set_file_mode, File(%s) open",
                            this, file_name.c_str()));

   file_mode= mode;                 // Set the file mode
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::set_file_name
//
// Function-
//       Set the trace file name
//
//----------------------------------------------------------------------------
void
   Debug::set_file_name(            // Set the trace file name
     const char*       name)        // The trace file name
{  IFHCDM( fprintf(stderr, "Debug(%p)::setName(%s)\n", this, name); )
   std::lock_guard<decltype(mutex)> lock(mutex);

   term();                          // Deactivate trace

   if( name == nullptr || name[0] == '\0' )
     name= "debug.out";
   file_name= name;
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
_LIBPUB_PRINTF(2, 3)
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
//       Debug::debugh
//
// Function-
//       Debugging (stdout + trace) printf facility, with heading
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
void
   Debug::debugh(                   // Debug printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugh(fmt, argptr);
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
_LIBPUB_PRINTF(2, 3)
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
//       Debug::errorh
//
// Function-
//       Debugging (stderr + trace) printf facility.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
void
   Debug::errorh(                   // Debug error printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   verrorh(fmt, argptr);
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
[[noreturn]]
_LIBPUB_PRINTF(2, 3)
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
//       Debug::tracef
//
// Function-
//       Debugging (trace only) printf facility.
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
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
//       Debug::traceh
//
// Function-
//       Debugging (trace only) printf facility, with heading
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 3)
void
   Debug::traceh(                   // Debug tracef facility, with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtraceh(fmt, argptr);            // Message with heading
   va_end(argptr);                  // Close va_ functions
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
_LIBPUB_PRINTF(2, 0)
void
   Debug::vdebugf(                  // Debug printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stdout, handle) )
     {
       va_list outptr;
       va_copy(outptr, argptr);
       vfprintf(stdout, fmt, outptr); // Write to stdout
       va_end(outptr);
     }
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vdebugh
//
// Function-
//       Debugging (stdout + trace) vprintf facility, with heading
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 0)
void
   Debug::vdebugh(                  // Debug printf facility, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stdout, handle) )
     {
       heading(stdout);

       va_list outptr;
       va_copy(outptr, argptr);
       vfprintf(stdout, fmt, outptr); // Write to stdout
       va_end(outptr);
     }

     heading(handle);
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
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
_LIBPUB_PRINTF(2, 0)
void
   Debug::verrorf(                  // Debug error printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stderr, handle) )
     {
       va_list errptr;
       va_copy(errptr, argptr);
       vfprintf(stderr, fmt, errptr); // Write to stderr
       va_end(errptr);
     }
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::verrorh
//
// Function-
//       Debugging (stderr + trace) vprintf facility, with heading
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 0)
void
   Debug::verrorh(                  // Debug error printf facility with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     if( isDIFFER(stderr, handle) )
     {
       heading(stderr);

       va_list errptr;
       va_copy(errptr, argptr);
       vfprintf(stderr, fmt, errptr); // Write to stderr
       va_end(errptr);
     }

     heading(handle);
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
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
//----------------------------------------------------------------------------
[[noreturn]]
_LIBPUB_PRINTF(2, 0)
void
   Debug::vthrowf(                  // Debug vprintf exception facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   fflush(stdout);

   {{{{
     va_list errptr;
     va_copy(errptr, argptr);
     vfprintf(stderr, fmt, errptr); // Write to stderr
     va_end(errptr);

     fprintf(stderr, "\n");
   }}}}
   fflush(stderr);

   // If trace file is already open and is neither stdout nor stderr
   if( handle != nullptr && handle != stdout && handle != stderr )
   {{{{
     va_list logptr;
     va_copy(logptr, argptr);
     vfprintf(handle, fmt, logptr); // Write to trace
     va_end(logptr);

     fprintf(handle, "\n");
     fflush(handle);                // Flush the handle buffer
     if( mode == MODE_INTENSIVE )   // If intensive trace mode
       flush();                     // Intensive buffer flush
   }}}}

static char            buffer[512]; // Work buffer (Mutex protected)
   int L= vsnprintf(buffer, sizeof(buffer), fmt, argptr);
   if( L < 0 || size_t(L) >= sizeof(buffer) ) // If cannot properly format
     throw fmt;                     // Just throw format string

   throw buffer;
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
_LIBPUB_PRINTF(2, 0)
void
   Debug::vtracef(                  // Debug trace printf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug::vtraceh
//
// Function-
//       Debugging (trace only) vprintf facility, with heading
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(2, 0)
void
   Debug::vtraceh(                  // Debug trace vprintf, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{
   if( mode != MODE_IGNORE )        // If not ignore mode
   {
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( handle == nullptr )        // If trace file not already open
       init();                      // Open it now

     heading(handle);               // Write heading
     vfprintf(handle, fmt, argptr); // Write to trace

     if( mode == MODE_INTENSIVE )   // If intensive trace mode
       flush();                     // Flush the buffer
   }
}

namespace debugging {
//----------------------------------------------------------------------------
//
// Subsection-
//       Debugging namespace function calls.
//
// Purpose-
//       These functions use the default debug object.
//
//----------------------------------------------------------------------------
void
   debug_backtrace( void )          // Display backtrace information
{
   auto trace= boost::stacktrace::stacktrace();
   auto array= trace.as_vector();
   for(size_t i= 1; i<array.size(); i++) {
     auto frame= array[i];
     debugf("[bt] %2zd %s at %s:%zd\n", i-1, frame.name().c_str()
           , frame.source_file().c_str(), frame.source_line());
   }
   debug_flush();
}

void
   debug_clr_head(                  // Clear a Heading options
     Debug::Heading    head)        // The Heading option to clear
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->clr_head(head);
}

void
   debug_flush( void )              // Flush write the trace file
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->flush();
}

std::string                         // The trace file name
   debug_get_file_name( void )      // Get the trace file name
{  std::lock_guard<decltype(mutex)> lock(mutex);
   return Debug::get()->get_file_name();
}

void
   debug_set_head(                  // Set a Heading options
     Debug::Heading    head)        // The Heading option to set
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->set_head(head);
}

void
   debug_set_file_mode(             // Set the trace file mode
     const char*       mode)        // The trace file mode
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->set_file_mode(mode);
}

void
   debug_set_file_name(             // Set the trace file name
     const char*       name)        // The trace file name
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->set_file_name(name);
}

void
   debug_set_mode(                  // Set the Mode
     Debug::Mode       mode)        // To this Mode
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->set_mode(mode);
}

_LIBPUB_PRINTF(1, 2)
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

_LIBPUB_PRINTF(1, 2)
void
   debugh(                          // Debug debug printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vdebugh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

_LIBPUB_PRINTF(1, 2)
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

_LIBPUB_PRINTF(1, 2)
void
   errorh(                          // Debug error printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   verrorh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

_LIBPUB_PRINTF(1, 2)
void                                // Note: Does not use Debug object
   errorp(                          // Write message to stderr (only)
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   fflush(NULL);                    // Flush everything first

   char buffer[4096];               // Format buffer
   va_start(argptr, fmt);           // Initialize va_ functions
   vsnprintf(buffer, sizeof(buffer), fmt, argptr); // Format the message
   va_end(argptr);                  // Close va_ functions

   perror(buffer);                  // Write the message in one line
   fflush(stderr);                  // Flush stderr
}

[[noreturn]]
_LIBPUB_PRINTF(1, 2)
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

_LIBPUB_PRINTF(1, 2)
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

_LIBPUB_PRINTF(1, 2)
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

_LIBPUB_PRINTF(1, 0)
void
   vdebugf(                         // Debug vdebugf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->vdebugf(fmt, argptr);
}

_LIBPUB_PRINTF(1, 0)
void
   vdebugh(                         // Debug vdebugf facility with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->vdebugh(fmt, argptr);
}

_LIBPUB_PRINTF(1, 0)
void
   verrorf(                         // Debug verrorf facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->verrorf(fmt, argptr);
}

_LIBPUB_PRINTF(1, 0)
void
   verrorh(                         // Debug verrorf facility with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->verrorh(fmt, argptr);
}

[[noreturn]]
_LIBPUB_PRINTF(1, 0)
void
   vthrowf(                         // Debug vthrowf exception facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->vthrowf(fmt, argptr);
   throw "ShouldNotOccur";
}

_LIBPUB_PRINTF(1, 0)
void
   vtracef(                         // Debug vtracef facility
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->vtracef(fmt, argptr);
}

_LIBPUB_PRINTF(1, 0)
void
   vtraceh(                         // Debug vtracef facility, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
{  std::lock_guard<decltype(mutex)> lock(mutex);
   Debug::get()->vtraceh(fmt, argptr);
}
}  // namespace _PUB_NAMESPACE::debugging
}  // namespace _PUB_NAMESPACE
