/*----------------------------------------------------------------------------
**
**       Copyright (C) 2007-2018 Frank Eskesen.
**
**       This file is free content, distributed under the Lesser GNU
**       General Public License, version 3.0.
**       (See accompanying file LICENSE.LGPL-3.0 or the original
**       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       Debug.h                                                            */
/*                                                                          */
/* Purpose-                                                                 */
/*       Debugging control object, C or C++.                                */
/*                                                                          */
/* Last change date-                                                        */
/*       2018/01/01                                                         */
/*                                                                          */
/* Implementation notes-                                                    */
/*       When used from a DLL, the library must also be compiled into a     */
/*       DLL. When compiled as a library, a separate instance is loaded     */
/*       into each DLL that invokes it.                                     */
/*                                                                          */
/* Implementation notes-                                                    */
/*       The static Debug::debug object is closed and deleted using a       */
/*       static destructor.                                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>                  // For FILE definition

#ifndef DEFINE_H_INCLUDED
#include "define.h"                 // For NULL
#endif

#ifdef __cplusplus
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
enum Chain                          // Debug::Chain
{  ChainMiddle= 0                   // Middle of chain
,  ChainLast=   1                   // Last in chain
,  ChainFirst=  2                   // First in chain
,  ChainOnly=   3                   // Only in chain
}; // enum Chain

enum Mode                           // Debug::Mode
{  ModeIgnore                       // Ignore all calls
,  ModeStandard                     // Standard debug mode
,  ModeIntensive                    // Hard Core Debug Mode
}; // enum Mode

//----------------------------------------------------------------------------
// Debug::Attributes
//----------------------------------------------------------------------------
protected:
FILE*                  handle;      // Debug file handle
char                   fileName[512]; // Debug file name

Mode                   mode;        // Debugging mode

// The following fields are used by dump
unsigned short         chain;       // Current chain
unsigned short         fsm;         // Current state

const void*            oldAddr;     // Repeat origin
const void*            newAddr;     // Current address

unsigned char          oldData[17]; // The repeated line string
unsigned char          newData[17]; // The Current line string

//----------------------------------------------------------------------------
// Debug::Static attributes
//----------------------------------------------------------------------------
protected:
static Debug*          debug;       // -> Current default debug object

//----------------------------------------------------------------------------
// Debug::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Debug( void );                  // Destructor
   Debug(                           // Constructor
     const char*       name= NULL); // The debug file name, default "debug.out"

private:
   Debug(const Debug&);             // Disallowed copy constructor
Debug&
   operator=(const Debug&);         // Disallowed assignment operator

//----------------------------------------------------------------------------
// Debug::Internal methods
//----------------------------------------------------------------------------
protected:
virtual void
   init( void );                    // Initialize

virtual int
   term( void );                    // Terminate

static int                          // TRUE iff STDIO
   isSTDIO(                         // Does fileName imply STDIO?
     const char*       fileName);   // The file name

//----------------------------------------------------------------------------
// Debug::Static methods
//----------------------------------------------------------------------------
public:
static Debug*                       // -> The current default debug object
   get( void );                     // Get the current default debug object

static Debug*                       // (The old default debug object)
   set(                             // Set
     Debug*            debug);      // This new default debug object

//----------------------------------------------------------------------------
//
// Debug uses a (hidden) RecursiveBarrier latch to prevent thread interference
// between operations that internally use multiple function calls. The obtain
// and release methods for that latch are exposed.
//
// Note that this latch should only be held for short time periods.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 iff first holder)
   obtain( void );                  // Obtain the RecursiveBarrier latch

static void
   release( void );                 // Release the RecursiveBarrier latch

//----------------------------------------------------------------------------
// Debug::Accessors
//----------------------------------------------------------------------------
public:
inline void
   setMode(                         // Set the Mode
     Mode              mode)        // To this Mode
{
   this->mode= mode;
}

inline FILE*                        // The handle
   getHandle( void )                // Get the handle
{
   if( handle == NULL )             // If not initialized
     init();

   return handle;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Debug::setName
//
// Purpose-
//       Set the log fileName.
//
//       A fileName of ">" or "1>" writes the log to stdout.
//       A fileName of "2>" writes the log to stderr.
//
//----------------------------------------------------------------------------
virtual void
   setName(                         // Name the trace file
     const char*       fname);      // The trace filename

//----------------------------------------------------------------------------
// Debug::Methods
//----------------------------------------------------------------------------
public:
virtual void
   vlogf(                           // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vtracef(                         // Write to trace
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vdebugf(                         // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   verrorf(                         // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   vthrowf(                         // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0) _ATTRIBUTE_NORETURN;

virtual void
   logf(                            // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   tracef(                          // Write to trace
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   errorf(                          // Write to trace and stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);

virtual void
   throwf(                          // Write to trace and stderr, throw exception
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3) _ATTRIBUTE_NORETURN;

virtual void
   dump(                            // Diagnostic dump (on target)
     FILE*             handle,      // Target file
     const void*       raddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     Chain             chain= ChainOnly); // Chaining

virtual void
   dump(                            // Diagnostic dump (to trace)
     const void*       raddr,       // Storage origin
     unsigned long     size,        // Storage length
     const void*       vaddr,       // Virtual origin
     Chain             chain= ChainOnly); // Chaining

virtual void
   dump(                            // Diagnostic dump (on target)
     FILE*             handle,      // Target file
     const void*       raddr,       // Storage origin
     unsigned long     size);       // Storage length

virtual void
   dump(                            // Diagnostic dump (to trace)
     const void*       raddr,       // Storage origin
     unsigned long     size);       // Storage length

virtual void
   flush( void );                   // Flush write the trace file
};

#endif /* defined(__cplusplus) */

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Subsection-                                                              */
/*       C-Language function calls.                                         */
/*                                                                          */
/* Purpose-                                                                 */
/*       C-Language environment calls operate against the default           */
/*       debug object.                                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/
void
   vtraceh(                         /* Write to trace, with heading         */
     const char*       fmt,         /* The PRINTF format string             */
     va_list           argptr)      /* VALIST                               */
   _ATTRIBUTE_PRINTF(1, 0);

void
   vtracef(                         /* Write to trace                       */
     const char*       fmt,         /* The PRINTF format string             */
     va_list           argptr)      /* VALIST                               */
   _ATTRIBUTE_PRINTF(1, 0);

void
   vdebugf(                         /* Write to trace and stdout            */
     const char*       fmt,         /* The PRINTF format string             */
     va_list           argptr)      /* VALIST                               */
   _ATTRIBUTE_PRINTF(1, 0);

void
   verrorf(                         /* Write to trace and stderr            */
     const char*       fmt,         /* The PRINTF format string             */
     va_list           argptr)      /* VALIST                               */
   _ATTRIBUTE_PRINTF(1, 0);

void
   vthrowf(                         /* Write to trace and stderr, throw exception */
     const char*       fmt,         /* The PRINTF format string             */
     va_list           argptr)      /* VALIST                               */
   _ATTRIBUTE_PRINTF(1, 0) _ATTRIBUTE_NORETURN;

void
   traceh(                          /* Write to trace, with heading         */
     const char*       fmt,         /* The PRINTF format string             */
                       ...)         /* PRINTF argruments                    */
   _ATTRIBUTE_PRINTF(1, 2);

void
   tracef(                          /* Write to trace                       */
     const char*       fmt,         /* The PRINTF format string             */
                       ...)         /* PRINTF argruments                    */
   _ATTRIBUTE_PRINTF(1, 2);

void
   debugf(                          /* Write to trace and stdout            */
     const char*       fmt,         /* The PRINTF format string             */
                       ...)         /* PRINTF argruments                    */
   _ATTRIBUTE_PRINTF(1, 2);

void
   errorf(                          /* Write to trace and stderr            */
     const char*       fmt,         /* The PRINTF format string             */
                       ...)         /* PRINTF argruments                    */
   _ATTRIBUTE_PRINTF(1, 2);

void
   throwf(                          /* Write to trace and stderr, throw exception */
     const char*       fmt,         /* The PRINTF format string             */
                       ...)         /* The PRINTF argument list             */
   _ATTRIBUTE_PRINTF(1, 2) _ATTRIBUTE_NORETURN;

void
   debugFlush( void );              /* Flush write the trace file           */

void
   debugSetName(                    /* Name the trace file                  */
     const char*       fname);      /* The trace filename                   */

void
   debugSetIgnoreMode( void );      /* Set Ignore mode                      */

void
   debugSetIntensiveMode( void );   /* Set Intensive mode                   */

void
   debugSetStandardMode( void );    /* Set Standard mode                    */

void
   dump(                            /* Diagnostic dump (to trace)           */
     const void*       paddr,       /* Storage origin                       */
     unsigned long     size);       /* Storage length                       */

void
   dumpv(                           /* Diagnostic dump (to trace)           */
     const void*       paddr,       /* Storage origin                       */
     unsigned long     size,        /* Storage length                       */
     const void*       vaddr,       /* Virtual origin                       */
     int               chain= 3);   /* Chaining control                     */

void
   snap(                            /* Diagnostic dump (to stdout)          */
     const void*       paddr,       /* Storage origin                       */
     unsigned long     size);       /* Storage length                       */

void
   snapv(                           /* Diagnostic dump (to stdout)          */
     const void*       paddr,       /* Storage origin                       */
     unsigned long     size,        /* Storage length                       */
     const void*       vaddr,       /* Virtual origin                       */
     int               chain= 3);   /* Chaining control                     */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DEBUG_H_INCLUDED */
