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
//       Status.h
//
// Purpose-
//       Status block.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       1) A status block is used to hold status information which may
//          require inter-Thread synchronization.  The status information
//          is put into the status block with post() and retrieved with
//          wait(), which causes a thread to block waiting for the status
//          information to arrive if it has not already arrived.  wait()
//          does not block if the information has already been posted.
//
//       2) Any number of threads may wait() using the same status block
//
//       3) Only one post() is allowed at a time.  The status block may
//          be reused but the reset() method must be used between uses.
//
// See also-
//       Events.h
//
//----------------------------------------------------------------------------
#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Status
//
// Purpose-
//       Status block.
//
//----------------------------------------------------------------------------
class Status {                      // Status block
//----------------------------------------------------------------------------
// Status::Attributes
//----------------------------------------------------------------------------
protected:
void*                  handle;      // Hidden attributes
long                   value;       // The Status

//----------------------------------------------------------------------------
// Status::Constructors
//----------------------------------------------------------------------------
public:
   ~Status( void );                 // Destructor
   Status( void );                  // Constructor

//----------------------------------------------------------------------------
// Status::Methods
//----------------------------------------------------------------------------
public:
long                                // The status
   wait( void );                    // Wait for operation completion

void
   post(                            // Post status
     long              status);     // The status value

void
   reset( void );                   // Reset the status block

//----------------------------------------------------------------------------
// Status::Bitwise copy prohibited
//----------------------------------------------------------------------------
private:                            // Bitwise copy is prohibited
   Status(const Status&);           // Disallowed copy constructor
   Status& operator=(const Status&);// Disallowed assignment operator
}; // class Status

#endif // STATUS_H_INCLUDED
