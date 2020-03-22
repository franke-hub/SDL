//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Logger.h
//
// Purpose-
//       Logger object.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#ifndef DEBUG_H_INCLUDED
#include "Debug.h"                  /* Also includes stdarg.h               */
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Logger
//
// Purpose-
//       Log writer.
//
// Implementation notes-
//       If specified, the debug file name is opened in APPEND mode.
//
//----------------------------------------------------------------------------
class Logger : public Debug {       // Write to log
//----------------------------------------------------------------------------
// Logger::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Logger( void );                 // Destructor
   Logger(                          // Constructor
     const char*       name= NULL); // The debug file name, default "debug.out"

private:
   Logger(const Logger&);           // Disallowed copy constructor
Logger&
   operator=(const Logger&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Logger::Internal methods
//----------------------------------------------------------------------------
protected:
virtual void
   init( void );                    // Initialize

//----------------------------------------------------------------------------
// Logger::Static methods
//----------------------------------------------------------------------------
public:
static void
   log(                             // Write log message
     const char*       format,      // PRINTF format string
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(1,2);
}; // class Logger

#endif /* LOGGER_H_INCLUDED */
