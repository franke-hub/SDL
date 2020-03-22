//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.h
//
// Purpose-
//       Common includes.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <stdarg.h>
#include <stdio.h>
#include <cstddef>
#include <iostream>
#include <string.h>
#include <sys/types.h>

using namespace std;

#include <com/Logger.h>
#include <com/Verify.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
// Enumerations
//----------------------------------------------------------------------------
enum                                // Expose Logger levels
{  LevelAll=    0
,  LevelInfo=   1
,  LevelStd=    2
,  LevelError=  3
,  LevelAbort=  4
,  LevelIgnore= 5
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
#define DIM 32
extern int             bugLevel;    // Demonstrate bugs?
extern const char*     nameList[DIM]; // Name array

//----------------------------------------------------------------------------
//
// Subroutine-
//       trash
//
// Purpose-
//       Insure that the call stack does not contain residual data.
//
//----------------------------------------------------------------------------
extern void
   trash( void );                   // Remove residual data from call stack

//----------------------------------------------------------------------------
//
// Subroutine-
//       wtlc
//
// Purpose-
//       Write to log conditional.
//
//----------------------------------------------------------------------------
extern int             __logLevel;  // The current log level

inline int
   getLogLevel( void )              // Get log level
{
   return __logLevel;
}

inline void
   setLogLevel(                     // Set log level
     int               level)       // To this
{
   __logLevel= level;
}

inline void
   wtlc(                            // Write to log condiional
     int               level,
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
   _ATTRIBUTE_PRINTF(2, 3);

inline void
   wtlc(                            // Write to log condiional
     int               level,
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   if( level < __logLevel )
     return;

   va_start(argptr, fmt);           // Initialize va_ functions
   Debug::get()->vlogf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

#endif // MAIN_H_INCLUDED
