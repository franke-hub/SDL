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
//       Mutex.h
//
// Purpose-
//       Thread-level mutual Exclusion Object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MUTEX_H_INCLUDED
#define MUTEX_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Mutex
//
// Purpose-
//       Mutual Exclusion Object
//
//----------------------------------------------------------------------------
class Mutex {                       // Mutual exclusion object
//----------------------------------------------------------------------------
// Mutex::Attributes
//----------------------------------------------------------------------------
protected:
void*                  object;      // -> Hidden object

//----------------------------------------------------------------------------
// Mutex::Constructors
//----------------------------------------------------------------------------
public:
   ~Mutex( void );                  // Destructor
   Mutex( void );                   // Constructor

private:                            // Bitwise copy is prohibited
   Mutex(const Mutex&);             // Disallowed copy constructor
   Mutex& operator=(const Mutex&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Mutex::Methods
//----------------------------------------------------------------------------
public:
void
   debugRelease(                    // Release the Mutex object w\debugging
     const char*       file,        // File name
     int               line);       // Line number

void
   debugReserve(                    // Reserve the Mutex object w\debugging
     const char*       file,        // File name
     int               line);       // Line number

void
   release( void );                 // Release the Mutex object

void
   reserve( void );                 // Reserve the Mutex object
}; // class Mutex

//----------------------------------------------------------------------------
//
// Class-
//       AutoMutex
//
// Purpose-
//       Automatic Mutual Exclusion Object
//
// Usage-
//       Given Mutex mutex;
//       :
//       {{{{
//         AutoMutex lock(mutex);   // Obtain the lock, released by destructor
//         :                        // Any exit from scope releases the Mutex
//       }}}}
//
//----------------------------------------------------------------------------
class AutoMutex {                   // Automatic mutual exclusion object
protected:
Mutex&                 mutex;       // The mutex

public:
inline
   ~AutoMutex( void )               // Destructor
{
   mutex.release();
}

inline
   AutoMutex(                       // Constructor
     Mutex&            mutex)       // Associated Mutex
:  mutex(mutex)
{
   mutex.reserve();
}
}; // class AutoMutex

#endif // MUTEX_H_INCLUDED
