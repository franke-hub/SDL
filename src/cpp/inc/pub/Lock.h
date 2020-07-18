//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Lock.h
//
// Purpose-
//       Inter-process named Lock.
//
// Last change date-
//       2020/07/18
//
//----------------------------------------------------------------------------
#ifndef _PUB_LOCK_H_INCLUDED
#define _PUB_LOCK_H_INCLUDED

#include <stdexcept>                // For std::logic_error
#include <string>                   // For std::string
#include <fcntl.h>                  // For O_* constants
#include <sys/stat.h>               // For S_* constants and macros
#include <semaphore.h>              // For semaphore

namespace pub {                     // The pub library namespace
//----------------------------------------------------------------------------
//
// Class-
//       Lock
//
// Purpose-
//       Global process named lock
//
// Implementation note-
//       Lock names must begin with '/' and contain no other '/' characters.
//
//----------------------------------------------------------------------------
class Lock {                        // Global process lock
//----------------------------------------------------------------------------
// Lock::Attributes
//----------------------------------------------------------------------------
protected:
sem_t*                 sem;         // Our global semaphore

//----------------------------------------------------------------------------
// Lock::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   Lock(                            // Constructor
     const char*       name)        // The lock (semaphore) name
:  sem(nullptr)                     // Initialized nullptr in case of error
{
   sem= sem_open(name, O_CREAT, S_IRWXU, 1); // Create the semaphore
   if( sem == nullptr )             // If failure
     throw std::logic_error("sem_open(" + std::string(name) + ") failed");
}

virtual
   ~Lock( void )                    // Destructor
{
   if( sem ) {
     sem_close(sem);
     sem= nullptr;
   }
}

//----------------------------------------------------------------------------
// Lock::Implement BasicLockable (for use in std::lock_guard)
//----------------------------------------------------------------------------
public:
void
   lock( void )                     // Obtain the Lock
{  sem_wait(sem); }

void
   unlock( void )                   // Release the Lock
{  sem_post(sem); }

//----------------------------------------------------------------------------
// Lock::Methods
//----------------------------------------------------------------------------
public:
// The create method is not normally needed. Its inclusion allows users to
// override the default open flags and/or mode.
static int                          // Return code, 0 OK
   create(                          // Create a Lock
     const char*       name,        // The name of the Lock
     int               oflag= O_CREAT, // The open flags
     mode_t            omode= S_IRWXU) // The open mode
{  return sem_open(name, oflag, omode, 1) == nullptr; }

static int                          // Return code, 0 OK
   unlink(                          // Destroy (unlink) the Lock
     const char*       name)        // The Lock name
{  return sem_unlink(name); }
}; // class Lock
}  // namespace pub
#endif // _PUB_LOCK_H_INCLUDED
