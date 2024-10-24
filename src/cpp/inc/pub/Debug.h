//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Debug.h
//
// Purpose-
//       Debugging control.
//
// Last change date-
//       2024/03/05
//
// Implementation notes-
//       A file name of ">" or "1>" writes the log to stdout.
//       A file name of "2>" writes the log to stderr.
//
//       The static Debug::debug instance is closed and deleted using a
//       static destructor.
//
//       When used from a DLL, use the DLL library. Otherwise a separate
//       Debug instance is loaded into each DLL where it's used.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DEBUG_H_INCLUDED
#define _LIBPUB_DEBUG_H_INCLUDED

#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For FILE definition
#include <string>                   // For std::string

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Debug
//
// Purpose-
//       Debugging control.
//
//----------------------------------------------------------------------------
class Debug {                       // Debugging control
//----------------------------------------------------------------------------
// Debug::Enumerations
//----------------------------------------------------------------------------
public:
enum Mode                           // Debug::Mode
{  MODE_DEFAULT                     // Default debug mode
,  MODE_IGNORE                      // Ignore all calls
,  MODE_INTENSIVE                   // Hard Core Debug Mode
}; // enum Mode

enum Heading                        // Debug::Heading
{  HEAD_DEFAULT= 0x00000001         // Default heading (HEAD_TIME)
,  HEAD_TIME=    0x00000001         // Include time in heading
,  HEAD_THREAD=  0x00000002         // Include thread to heading
}; // enum Heading

//----------------------------------------------------------------------------
// Debug::Attributes
//----------------------------------------------------------------------------
protected:
FILE*                  handle= nullptr; // Debug file handle
std::string            file_mode= "wb"; // Debug file mode
std::string            file_name= "debug.out"; // Debug file name
int                    head= HEAD_DEFAULT; // Heading options
Mode                   mode= MODE_DEFAULT; // Debugging mode

//----------------------------------------------------------------------------
// Debug::Static attributes
//----------------------------------------------------------------------------
protected:
static Debug*          common;      // -> The Common Debug instance

//----------------------------------------------------------------------------
// Debug::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Debug( void );                  // Destructor
   Debug(                           // Constructor
     const char*       name= nullptr); // The debug file name, default "debug.out"

   Debug(const Debug&) = delete;    // Disallowed copy constructor
Debug& operator=(const Debug&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Debug::Internal methods
//----------------------------------------------------------------------------
protected:
void
   heading(                         // Write heading
     FILE*             file);       // To this FILE

virtual void
   init( void );                    // Initialize

virtual void
   term( void );                    // Terminate

//----------------------------------------------------------------------------
// Debug::Static methods
//----------------------------------------------------------------------------
public:
static void
   debug(                           // ((INTERNAL USE ONLY)) debugging
     const char*       info = nullptr); // Caller information

// WARNING: In a multi-threaded environment it is possible for the Debug
// object to change before it is used. Use:
//   Debug::lock(); Debug::get()->op(); Debug::unlock();
static Debug*                       // -> The common Debug instance
   get( void );                     // Get the common Debug instance

static Debug*                       // (The old common Debug instance)
   set(                             // Set
     Debug*            debug);      // This new common Debug instance)

static Debug*                       // (The current common Debug instance)
   show( void )                     // Get the current common Debug instance
{  return common; }                 // Without trying to create it

//----------------------------------------------------------------------------
//
// Debug uses a (hidden) static RecursiveLatch to prevent thread interference
// between operations that internally use multiple function calls.
// The Lockable interface methods for that Latch are exposed.
//
// Note: this latch should only be held for short time periods.
//
//----------------------------------------------------------------------------
static void
   lock( void );                    // Obtain the RecursiveLatch

static bool                         // TRUE if the RecursiveLatch was obtained
   try_lock( void );                // Attempt to obtain the RecursiveLatch

static void
   unlock( void );                  // Release the RecursiveLatch

//----------------------------------------------------------------------------
// Debug::Accessors and control operations
//----------------------------------------------------------------------------
void
   flush( void );                   // Flush the trace file, stdout and stderr

FILE*                               // The handle
   get_FILE( void )                 // Get the handle
{
   if( handle == nullptr )          // If not initialized
     init();

   return handle;
}

std::string
   get_file_mode( void )            // Get the trace file mode
{  return file_mode; }

std::string
   get_file_name( void )            // Get the trace file name
{  return file_name; }

int                                 // The current heading options
   get_head( void )                 // Get the Heading options
{  return head; }

Mode                                // The current Mode
   get_mode( void )                 // Get the Mode
{  return mode; }

void
   set_file_mode(                   // Set the trace file mode
     const char*       mode);       // The trace file mode

void
   set_file_name(                   // Set the trace file name
     const char*       name);       // The trace file name

void
   set_head(                        // Set the Heading options
     int               head)        // The Heading option(s) to set
{  this->head= head; }

void
   set_mode(                        // Set the Mode
     Mode              mode)        // To this Mode
{  this->mode= mode; }

//----------------------------------------------------------------------------
// Debug::Methods
//----------------------------------------------------------------------------
public:
void
   backtrace( void );               // Write backtrace information

_LIBPUB_PRINTF(2, 3)
virtual void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

virtual void
_LIBPUB_PRINTF(2, 3)
   debugh(                          // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(2, 3)
virtual void
   errorf(                          // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(2, 3)
virtual void
   errorh(                          // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

[[noreturn]]
_LIBPUB_PRINTF(2, 3)
virtual void
   throwf(                          // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(2, 3)
virtual void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(2, 3)
virtual void
   traceh(                          // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(2, 0)
virtual void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(2, 0)
virtual void
   vdebugh(                         // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(2, 0)
virtual void
   verrorf(                         // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(2, 0)
virtual void
   verrorh(                         // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

[[noreturn]]
_LIBPUB_PRINTF(2, 0)
virtual void
   vthrowf(                         // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(2, 0)
virtual void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(2, 0)
virtual void
   vtraceh(                         // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST
}; // class Debug

//----------------------------------------------------------------------------
//
// Namespace-
//       _LIBPUB_NAMESPACE::debugging
//
// Purpose-
//       Ease of use function calls operate using the common Debug instance
//
//----------------------------------------------------------------------------
namespace debugging {
namespace options {
//----------------------------------------------------------------------------
// (Settable) application debugging controls:
extern int             opt_check;   // Enable checking?      [default: false]
extern int             opt_hcdm;    // Hard Core Debug Mode? [default: false]
extern int             opt_verbose; // Debugging verbosity   [default: -1]

//----------------------------------------------------------------------------
// (Settable) library debugging controls:
extern int             pub_check;   // Enable checking?      [default: false]
extern int             pub_hcdm;    // Hard Core Debug Mode? [default: false]
extern int             pub_verbose; // Debugging verbosity   [default: -1]
}  // namespace options

void
   debug_backtrace( void );         // Write backtrace information

void
   debug_flush( void );             // Flush write the trace file

std::string
   debug_get_file_mode( void );     // Get the trace file mode

std::string
   debug_get_file_name( void );     // Get the trace file name

int                                 // The current Heading options
   debug_get_head( void );          // Get the current Heading options

Debug::Mode                         // The current debug Mode
   debug_get_mode( void );          // Get the current debug Mode

void
   debug_set_file_mode(             // Set the trace file mode
     const char*       mode);       // The trace file mode

void
   debug_set_file_name(             // Set the trace file name
     const char*       name);       // The trace file name

void
   debug_set_head(                  // Set the Heading options
     int               head);       // The Heading option(s) to set

void
   debug_set_mode(                  // Set the Mode
     Debug::Mode       mode);       // To this Mode

_LIBPUB_PRINTF(1, 2)
void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

_LIBPUB_PRINTF(1, 2)
void
   debugh(                          // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

_LIBPUB_PRINTF(1, 2)
void
   errorf(                          // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

_LIBPUB_PRINTF(1, 2)
void
   errorh(                          // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

[[noreturn]]
_LIBPUB_PRINTF(1, 2)
void
   throwf(                          // Write to trace and stderr, throw exception */
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

_LIBPUB_PRINTF(1, 2)
void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

_LIBPUB_PRINTF(1, 2)
void
   traceh(                          // Write to trace, with heading
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

_LIBPUB_PRINTF(1, 0)
void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(1, 0)
void
   vdebugh(                         // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(1, 0)
void
   verrorf(                         // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(1, 0)
void
   verrorh(                         // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

[[noreturn]]
_LIBPUB_PRINTF(1, 0)
void
   vthrowf(                         // Write to trace and stderr, throw exception */
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(1, 0)
void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST

_LIBPUB_PRINTF(1, 0)
void
   vtraceh(                         // Write to trace, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr);     // VALIST
}  // namespace debugging
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DEBUG_H_INCLUDED
