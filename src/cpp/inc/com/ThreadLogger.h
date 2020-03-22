//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ThreadLogger.h
//
// Purpose-
//       ThreadLogger object.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef THREADLOGGER_H_INCLUDED
#define THREADLOGGER_H_INCLUDED

#include "com/Logger.h"

//----------------------------------------------------------------------------
//
// Class-
//       ThreadLogger
//
// Purpose-
//       Thread log writer.
//
// Implementation notes-
//       If specified, the debug file name is opened in APPEND mode.
//
//       The ThreadLogger object should be constructed in the system control
//       thread, before any other threads are started and before any other
//       debugging activity takes place and set as the default debug object.
//       Example:
//         extern int main(int argc, char* argv() {
//           Debug::set(new ThreadLogger()); // Initialize ThreadLogger
//           :                      // Multithreaded code
//           return 0;
//         } // extern int main
//
//       Debug deletes the ThreadLogger object when the main thread exits.
//       It is not necessary to explicitly close or delete it.
//
//----------------------------------------------------------------------------
class ThreadLogger : public Logger { // Write to log
//----------------------------------------------------------------------------
// ThreadLogger::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ThreadLogger( void );           // Destructor
   ThreadLogger(                    // Constructor
     const char*       name= NULL); // The debug file name, default "debug.out"

private:
   ThreadLogger(const ThreadLogger&); // Disallowed copy constructor
ThreadLogger&
   operator=(const ThreadLogger&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// ThreadLogger::Methods
//----------------------------------------------------------------------------
public:
virtual void
   vlogf(                           // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2, 0);

virtual void
   logf(                            // Write to trace with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
   _ATTRIBUTE_PRINTF(2, 3);
}; // class ThreadLogger

#endif // THREADLOGGER_H_INCLUDED
