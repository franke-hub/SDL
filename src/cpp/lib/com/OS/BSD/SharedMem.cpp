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
//       OS/BSD/SharedMem.cpp
//
// Purpose-
//       BSD SharedMem storage methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <com/Debug.h>
#include <com/Exception.h>

#include "com/SharedMem.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SHARED  " // Source filename

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       SharedMem::getToken(unsigned)
//
// Purpose-
//       Create a Token from a constant value.
//
//----------------------------------------------------------------------------
SharedMem::Token                    // The Token
   SharedMem::getToken(             // Convert value to Token
     unsigned          identifier)  // Local constant identifier
{
   return Token(identifier);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::getToken
//
// Purpose-
//       Allocate a persistent token.
//
//----------------------------------------------------------------------------
SharedMem::Token                    // Resultant token
   SharedMem::getToken(             // Convert name to token
     const char*       fileName,    // File name
     unsigned          identifier)  // Initializer
{
   #ifdef HCDM
     debugf("%8s= SharedMem::getToken(%s,%.8X)\n", "", fileName, identifier);
   #endif

   return (Token)::ftok(fileName, identifier);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::access
//
// Purpose-
//       Access a shared segment
//
//----------------------------------------------------------------------------
SharedMem::Segment                  // Resultant segment identifier
   SharedMem::access(               // Convert token to segment
     Size_t            size,        // Required size
     Token             token,       // Associated Token
     int               flags)       // Controls
{
   Segment             result;      // Resultant
   unsigned            protect;     // Protection flags

   protect= S_IRUSR;                // Always allow read access
   if( (flags&SharedMem::Write) != 0 )
     protect |= S_IWUSR;
   if( (flags&SharedMem::Create) != 0 )
     protect |= IPC_CREAT;
   if( (flags&SharedMem::Exclusive) != 0 )
     protect |= IPC_EXCL;

   result= (Segment)::shmget((key_t)token, size, protect);

   #ifdef HCDM
     debugf("%.8lx= SharedMem::access(%ld,%.8lx,%.8lx)\n",
            (long)result, (long)size, (long)token, (long)flags);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::attach
//
// Purpose-
//       Attach a shared segment
//
//----------------------------------------------------------------------------
void*                               // Resultant segment address
   SharedMem::attach(               // Attach segment
     Segment           segment)     // Associated segment
{
   void*               resultant;   // Resultant segment address

   resultant= ::shmat(segment, 0, 0);
   if( (long)resultant == (-1) )
     resultant= NULL;

   #ifdef HCDM
     debugf("%p= SharedMem::attach(%.8X)\n", resultant, segment);
   #endif

   return resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::detach
//
// Purpose-
//       Detach a shared segment
//
//----------------------------------------------------------------------------
void
   SharedMem::detach(               // Detach segment
     const void*       addr)        // -> Segment
{
   #ifdef HCDM
     debugf("%8s= SharedMem::detach(%p)\n", "", addr);
   #endif

   ::shmdt(addr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SharedMem::remove
//
// Purpose-
//       Remove a shared segment
//
//----------------------------------------------------------------------------
void
   SharedMem::remove(               // Remove a segment
     Segment           segment)     // Segment identifier
{
   #ifdef HCDM
     debugf("%8s= SharedMem::remove(%.8X)\n", "", segment);
   #endif

   ::shmctl(segment, IPC_RMID, NULL); // Destroy the segment
}

