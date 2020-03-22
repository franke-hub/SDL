//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Cache.h
//
// Purpose-
//       Cache controller. *NOT IMPLEMENTED*
//
// Last change date-
//       2013/01/01
//
// Problems-
//       How to control object reference/release?
//         Use Pointer/Object paradigm.
//
//       Timer runs every timeout milliseconds. How to synchronize shutdown?
//         No answer.
//
//----------------------------------------------------------------------------
#ifndef CACHE_H_INCLUDED
#define CACHE_H_INCLUDED

#include <map>
#include <string>

#include <com/Mutex.h>
#include <com/Object.h>
#include <com/Dispatch.h>

//----------------------------------------------------------------------------
//
// Struct-
//       Cached
//
// Purpose-
//       Cached item.
//
//----------------------------------------------------------------------------
struct Cached {                     // Cached item
//----------------------------------------------------------------------------
// Cached::Attributes
//----------------------------------------------------------------------------
double                 timeout;     // Cache entry Clock timeout
Ref<Object>            object;      // The cached object
}; // struct Cached

//----------------------------------------------------------------------------
//
// Class-
//       Cache
//
// Purpose-
//       Cache controller.
//
//----------------------------------------------------------------------------
class Cache {                       // Cache controller
//----------------------------------------------------------------------------
// Cache::Attributes
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Cached> Map;

protected:
Mutex                  mutex;       // Cache modification mutex
int                    fsm;         // Finite State Machine
                                    // 0 (RESET)
                                    // 1 (READY)
                                    // 2 (TIMER)
                                    // 3 (TIMER_RESET)
                                    // 4 (READY_RESET)

Map                    map;         // The cache
int                    timeout;     // Cache entry timeout (in milliseconds)

DispatchItem           item;        // Our work Item

//----------------------------------------------------------------------------
// Cache::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Cache( void );                  // Destructor
   Cache(                           // (Default) Constructor
     int               timeout= 3000); // Timeout (in milliseconds)

private:                            // Disallowed:
   Cache(const Cache&);             // Copy constructor
Cache& operator=(const Cache&);     // Assignment operator

//----------------------------------------------------------------------------
// Cache::Accessors
//----------------------------------------------------------------------------
public:
virtual Ref<Object>                 // Resultant object reference
   get(                             // Get object
     std::string       item);       // With this item descriptor

virtual void
   set(                             // Set object
     std::string       item,        // With this item descriptor
     Ref<Object>       object);     // Object reference

virtual void
   setTimeout(                      // Set new timeout
     int               time);       // Timeout (in milliseconds)

//----------------------------------------------------------------------------
// Cache::Methods
//----------------------------------------------------------------------------
public:
virtual void
   reset( void );                   // Reset the Cache
}; // class Cache

#endif // CACHE_H_INCLUDED
