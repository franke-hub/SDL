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
//       Process.h
//
// Purpose-
//       Process.cpp helper classes.
//
// Last change date-
//       2020/07/18
//
// Implementation notes-
//       Requires: #include <pub/Debug.h>; using namespace pub::debugging;
//
//----------------------------------------------------------------------------
#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Return strerror(errno)
//
//----------------------------------------------------------------------------
static inline const char* error( void ) { return strerror(errno); }

//----------------------------------------------------------------------------
//
// Class-
//       Catcher
//
// Purpose-
//       Add try/catch around function.
//
//----------------------------------------------------------------------------
class Catcher {                     // Try/catch wrapper
//----------------------------------------------------------------------------
// Catcher::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   Catcher( void ) = default;       // Default constructor
virtual
   ~Catcher( void ) = default;      // Virtual destructor

//----------------------------------------------------------------------------
// Catcher::methods
//----------------------------------------------------------------------------
public:
virtual int                         // Error counter
   run( void )                      // Run with try/catch wrapper
{  return 0; }                      // Base class does almost nothing

int                                 // Error counter
   start( void )                    // Start: invoke run
{
   try {
     return run();
   } catch(pub::Exception& X) {
     debugf("%s\n", std::string(X).c_str());
   } catch(std::exception& X) {
     debugf("std::exception.what(%s))\n", X.what());
   } catch(const char* X) {
     debugf("catch(const char* '%s')\n", X);
   } catch(...) {
     debugf("catch(...)\n");
   }

   return 1;                        // (Exception occurred)
}
}; // class Catcher

//----------------------------------------------------------------------------
//
// Class-
//       Lock
//
// Purpose-
//       Global process named lock
//
//----------------------------------------------------------------------------
class Lock {                        // Global process lock
//----------------------------------------------------------------------------
// Lock::Attributes
//----------------------------------------------------------------------------
public:
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
   if( sem == nullptr ) {           // If failure
     fprintf(stderr, "%4d sem_open(%s) ", __LINE__, name);
     perror("failed");
   }
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
// Lock::Basic-lockable
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
static int                          // Return code, 0 OK
   unlink(                          // Destroy (unlink) the Lock
     const char*       name)        // The Lock name
{  return sem_unlink(name); }
}; // class Lock
#endif // PROCESS_H_INCLUDED
