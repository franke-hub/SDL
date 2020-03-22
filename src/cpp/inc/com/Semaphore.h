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
//       Semaphore.h
//
// Purpose-
//       Thread-level signaling Object.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef SEMAPHORE_H_INCLUDED
#define SEMAPHORE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Semaphore
//
// Purpose-
//       Thread signaling Object
//
// Usage notes-
//       Semaphore::post is known in the literature as the P operation.
//       Semaphore::wait is known in the literature as the V operation.
//
//----------------------------------------------------------------------------
class Semaphore {                   // Thread signaling object
//----------------------------------------------------------------------------
// Semaphore::Attributes
//----------------------------------------------------------------------------
protected:
void*                  object;      // -> Hidden object

//----------------------------------------------------------------------------
// Semaphore::Constructors
//----------------------------------------------------------------------------
public:
   ~Semaphore( void );              // Destructor
   Semaphore(                       // Constructor
     int               count= 1);   // Initial share count

private:                            // Bitwise copy is prohibited
   Semaphore(const Semaphore&);     // Disallowed copy constructor
   Semaphore& operator=(const Semaphore&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Semaphore::Methods
//----------------------------------------------------------------------------
public:
void
   debugPost(                       // Post the Semaphore object w\debugging
     const char*       file,        // File name
     int               line);       // Line number

void
   debugWait(                       // Wait for Semaphore object w\debugging
     const char*       file,        // File name
     int               line);       // Line number

void
   post( void );                    // Post the Semaphore (Make available)

void
   wait( void );                    // Wait for Semaphore availability

int                                 // Return code (0 iff successful)
   wait(                            // Attempt wait for Semaphore availability
     double            seconds);    // Timeout delay (0.0 for immediate)
}; // class Semaphore

#endif // SEMAPHORE_H_INCLUDED
