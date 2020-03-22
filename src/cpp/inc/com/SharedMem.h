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
//       SharedMem.h
//
// Purpose-
//       Shared Memory accessor object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SHAREDMEM_H_INCLUDED
#define SHAREDMEM_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       SharedMem
//
// Purpose-
//       Shared storage access.
//
//----------------------------------------------------------------------------
class SharedMem {                   // Shared storage access
//----------------------------------------------------------------------------
// SharedMem::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef unsigned long  Size_t;      // Size type

enum                                // Generic enumeration
{  InvalidSegment=     0xffffffff   // Invalid Segment
,  InvalidToken=       0xffffffff   // Invalid Token
}; // enum

enum Control                        // Control flags
{  Keep=               0x80000000   // Do not delete segment
,  Create=             0x00008000   // Create segment if non-existent
,  Exclusive=          0x00004000   // Segment must not already be attached
,  Write=              0x00000001   // Allow write access
}; // enum Control

typedef void         Address;       // Storage address
typedef unsigned     Token;         // Token identifier
typedef unsigned     Segment;       // Segment identifier

//----------------------------------------------------------------------------
// SharedMem::Attributes
//----------------------------------------------------------------------------
protected:
Address*             address;       // (Local) storage address
Segment              segment;       // Segment

unsigned             length;        // Storage length
Token                token;         // Token
int                  control;       // Controls

//----------------------------------------------------------------------------
// SharedMem::Constructors
//----------------------------------------------------------------------------
public:
   ~SharedMem( void );              // Destructor
   SharedMem(                       // Constructor
     Size_t          length,        // Length of storage region
     Token           token,         // Associated token
     int             control);      // Control flags

//----------------------------------------------------------------------------
// SharedMem::Methods
//----------------------------------------------------------------------------
public:
inline Address*                     // Associated Address
   getAddress( void ) const;        // Get associated Address

inline Segment                      // Associated Segment
   getSegment( void ) const;        // Get associated Segment

inline Size_t                       // The Length
   getLength( void ) const;         // Get Length

inline Token                        // Associated Token
   getToken( void ) const;          // Get associated Token

static Token                        // Resultant Token
   getToken(                        // Allocate a persistent Token
     unsigned        identifier);   // Local constant identifier

static Token                        // Resultant Token
   getToken(                        // Allocate a persistent Token
     const char*     fileName,      // (Existent) filename
     unsigned        identifier);   // Local constant identifier

static Segment                      // Resultant Segment
   access(                          // Access a segment
     Size_t          length,        // Segment size
     Token           token,         // Token identifier
     int             control);      // Controls

static Address*                     // Storage address
   attach(                          // Attach a Segment
     Segment         segment);      // Segment identifier

static void
   detach(                          // Detach a Segment
     const Address*  address);      // Shared storage address

static void
   remove(                          // Remove (destroy) a Segment
     Segment         segment);      // Segment identifier
}; // class SharedMem

#include "SharedMem.i"

#endif // SHAREDMEM_H_INCLUDED
