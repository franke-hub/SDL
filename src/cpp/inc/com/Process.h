//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Process.h
//
// Purpose-
//       Process descriptor.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Process
//
// Purpose-
//       Process descriptor.
//
//----------------------------------------------------------------------------
class Process {                     // Process descriptor
//----------------------------------------------------------------------------
// Process::Attributes
//----------------------------------------------------------------------------
protected:
void*                  attr;        // Hidden attributes

//----------------------------------------------------------------------------
// Process::Constructors
//----------------------------------------------------------------------------
public:
   ~Process( void );                // Destructor
   Process( void );                 // Constructor

//----------------------------------------------------------------------------
// Process::Methods
//----------------------------------------------------------------------------
public:
void
   signal(                          // Signal the Process
     int               sid);        // Using this signal identifier

void
   start(                           // Start the Process
     const char*       functionName,// Name of the function to execute
     const char*       parameterList); // Parameter list string

int                                 // Process return code
   wait( void );                    // Wait for Process to complete
}; // class Process

#endif // PROCESS_H_INCLUDED
