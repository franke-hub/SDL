//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Latch.h
//
// Purpose-
//       Latch descriptor.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef LATCH_H_INCLUDED
#define LATCH_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Latch
//
// Purpose-
//       Latch descriptor.
//
// Implementation notes-
//       The Latch implementation uses a spin latch. That is, if the latch is
//       not immediately available the obtain method spins attempting over and
//       over trying to obtain the latch. A Latch should be held infrequently
//       and then only for short periods of time.
//
//       Use a locking object such as ThreadLock if waiting is required.
//
//----------------------------------------------------------------------------
class Latch {                       // Latch descriptor
//----------------------------------------------------------------------------
// Latch::Attributes
//----------------------------------------------------------------------------
protected:
volatile int32_t       latchWord;   // The LatchWord

//----------------------------------------------------------------------------
// Latch::Constructors
//----------------------------------------------------------------------------
public:
   ~Latch( void );                  // Destructor
   Latch( void );                   // Constructor

//----------------------------------------------------------------------------
// Latch::Accessor methods
//----------------------------------------------------------------------------
public:
int                                 // TRUE if Latch held in SHR mode
   isHeldSHR( void ) const;         // Is this Latch held in SHR mode?

int                                 // TRUE if Latch held in XCL mode
   isHeldXCL( void ) const;         // Is this Latch held in XCL mode?

//----------------------------------------------------------------------------
// Latch::Methods
//----------------------------------------------------------------------------
public:
int                                 // TRUE if Latch obtained
   attemptSHR( void );              // Conditionally obtain SHR Latch

int                                 // TRUE if Latch obtained
   attemptXCL( void );              // Conditionally obtain XCL Latch

void
   obtainSHR( void );               // Obtain the Latch in shared mode

void
   obtainXCL( void );               // Obtain the Latch in exclusive mode

void
   modifySHR( void );               // Downgrade to shared mode

int                                 // TRUE if upgrade succeeded
   modifyXCL( void );               // Upgrade to exclusive mode

void
   releaseSHR( void );              // Release the Latch, held in shared mode

void
   releaseXCL( void );              // Release the Latch, held in exclusive mode
}; // class Latch

//----------------------------------------------------------------------------
//
// Class-
//       AutoLatchSHR
//
// Purpose-
//       Automatic SHR Latch Object
//
// Usage-
//       Given Latch latch;
//       :
//       {{{{
//         AutoLatchSHR lock(latch); // Obtain the latch, released by destructor
//         :                        // Any exit from scope releases the Barrier
//       }}}}
//
//----------------------------------------------------------------------------
class AutoLatchSHR {                // Automatic SHR Latch object
protected:
Latch&                 latch;       // The Latch

public:
inline
   ~AutoLatchSHR( void )            // Destructor
{
   latch.releaseSHR();
}

inline
   AutoLatchSHR(                    // Constructor
     Latch&            latch)       // Associated Latch
:  latch(latch)
{
   latch.obtainSHR();
}
}; // class AutoLatchSHR

//----------------------------------------------------------------------------
//
// Class-
//       AutoLatchXCL
//
// Purpose-
//       Automatic XCL Latch Object
//
// Usage-
//       Given Latch latch;
//       :
//       {{{{
//         AutoLatchXCL lock(latch); // Obtain the latch, released by destructor
//         :                        // Any exit from scope releases the Barrier
//       }}}}
//
//----------------------------------------------------------------------------
class AutoLatchXCL {                // Automatic XCL Latch object
protected:
Latch&                 latch;       // The Latch

public:
inline
   ~AutoLatchXCL( void )            // Destructor
{
   latch.releaseXCL();
}

inline
   AutoLatchXCL(                    // Constructor
     Latch&            latch)       // Associated Latch
:  latch(latch)
{
   latch.obtainXCL();
}
}; // class AutoLatchXCL

#endif // LATCH_H_INCLUDED
