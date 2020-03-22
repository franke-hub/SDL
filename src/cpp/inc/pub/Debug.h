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
//       Debug.h
//
// Purpose-
//       Debugging control.
//
// Last change date-
//       2020/01/24
//
// Implementation notes-
//       A fileName of ">" or "1>" writes the log to stdout.
//       A fileName of "2>" writes the log to stderr.
//
//       The static Debug::debug instance is closed and deleted using a
//       static destructor.
//
//       When used from a DLL, use the DLL library. Otherwise a separate
//       Debug instance is loaded into each DLL where it's used.
//
//----------------------------------------------------------------------------
#ifndef _PUB_DEBUG_H_INCLUDED
#define _PUB_DEBUG_H_INCLUDED

#include <stdarg.h>                 // For va_* functions
#include <stdio.h>                  // For FILE definition
#include <string>                   // For std::string

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
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
{  ModeIgnore                       // Ignore all calls
,  ModeStandard                     // Standard debug mode
,  ModeIntensive                    // Hard Core Debug Mode
}; // enum Mode

enum Heading                        // Debug::Heading
{  HeadTime=    0x00000001          // Add time to heading
,  HeadThread=  0x00000002          // Add thread to heading
,  HeadStandard= HeadTime           // Default heading
}; // enum Heading

//----------------------------------------------------------------------------
// Debug::Attributes
//----------------------------------------------------------------------------
protected:
FILE*                  handle= nullptr; // Debug file handle
std::string            fileMode= "wb"; // Debug file mode
std::string            fileName= "debug.out"; // Debug file name
int                    head= HeadStandard; // Heading options
int                    mode= ModeStandard; // Debugging mode

//----------------------------------------------------------------------------
// Debug::Static attributes
//----------------------------------------------------------------------------
protected:
static Debug*          common;      // -> The Common Debug instance

public:
static int             level;       // The Common Debug level

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

//----------------------------------------------------------------------------
//
// Debug uses a (hidden) static RecursiveLatch to prevent thread interference
// between operations that internally use multiple function calls.
// The lock and unlock methods for that Latch are exposed.
//
// Note: this latch should only be held for short time periods.
//
//----------------------------------------------------------------------------
static void
   lock( void );                    // Obtain the RecursiveLatch

static void
   unlock( void );                  // Release the RecursiveLatch

//----------------------------------------------------------------------------
// Debug::Accessors and control operations
//----------------------------------------------------------------------------
inline void
   clr_head(                        // Clear a Heading options
     Heading           head)        // The Heading option to clear
{  this->head &= ~(int)head; }

void
   flush( void );                   // Flush the trace file, stdout and stderr

inline FILE*                        // The handle
   get_FILE( void )                 // Get the handle
{
   if( handle == nullptr )          // If not initialized
     init();

   return handle;
}

std::string
   get_fileMode( void )             // Get the trace filename
{  return fileMode; }

std::string
   get_fileName( void )             // Get the trace filename
{  return fileName; }

void
   set_fileMode(                    // Set the trace file mode
     const char*       mode);       // The trace filemode

void
   set_fileName(                    // Set the trace file name
     const char*       name);       // The trace filename

inline void
   set_head(                        // Set a Heading option
     Heading           head)        // The Heading option to set
{  this->head |= (int)head; }

inline void
   set_mode(                        // Set the Mode
     Mode              mode)        // To this Mode
{  this->mode= mode; }

//----------------------------------------------------------------------------
// Debug::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   debugh(                          // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   errorf(                          // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   errorh(                          // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   throwf(                         // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3) _ATTRIBUTE_NORETURN;

virtual void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   traceh(                          // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vdebugh(                         // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   verrorf(                         // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   verrorh(                         // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vthrowf(                         // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0) _ATTRIBUTE_NORETURN;

virtual void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vtraceh(                         // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);
}; // class Debug

//----------------------------------------------------------------------------
//
// Namespace-
//       debugging
//
// Purpose-
//       Ease of use function calls operate using the common Debug instance
//
// Implementation notes-
//       "using namespace _PUB_NAMESPACE:debugging;" enables these functions
//
//----------------------------------------------------------------------------
namespace debugging {
void
   debug_clr_head(                  // Clear a Heading options
     Debug::Heading    head);       // The Heading option to clear

void
   debug_flush( void );             // Flush write the trace file

std::string
   debug_get_fileName( void );      // Get the trace file name

void
   debug_set_head(                  // Set a Heading options
     Debug::Heading    head);       // The Heading option to set

void
   debug_set_fileMode(              // Set the trace file mode
     const char*       mode);       // The trace filemode

void
   debug_set_fileName(              // Set the trace file name
     const char*       name);       // The trace filename

void
   debug_set_mode(                  // Set the Mode
     Debug::Mode       mode);       // To this Mode

void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   debugh(                          // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   errorf(                          // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   errorh(                          // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   errorp(                          // Wrap perror(message), only to stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   throwf(                          // Write to trace and stderr, throw exception */
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(1, 2) _ATTRIBUTE_NORETURN;

void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   traceh(                          // Write to trace, with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);

void
   vdebugh(                         // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);

void
   verrorf(                         // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);

void
   verrorh(                         // Write to trace and stderr with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);

void
   vthrowf(                         // Write to trace and stderr, throw exception */
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0) _ATTRIBUTE_NORETURN;

void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);

void
   vtraceh(                         // Write to trace, with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(1, 0);
}  // namespace debugging
}  // namespace _PUB_NAMESPACE
#endif // _PUB_DEBUG_H_INCLUDED
