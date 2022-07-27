//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/Select.h
//
// Purpose-
//       Socket selector
//
// Last change date-
//       2022/07/26
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SELECT_H_INCLUDED
#define _LIBPUB_SELECT_H_INCLUDED

#include <functional>               // For std::function
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <errno.h>                  // For EINVAL
#include <fcntl.h>                  // For fcntl
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <sys/poll.h>               // For struct pollfd, ...
#include <sys/socket.h>             // For socket methods

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros
#include "pub/Exception.h"          // For SocketException
#include "pub/Object.h"             // For base class, _PUB_NAMESPACE, ...
#include "pub/Socket.h"             // For pub::Socket

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct control_op;                  // (Internal) control operation

//----------------------------------------------------------------------------
//
// Class-
//       Select
//
// Purpose-
//       Socket selector
//
// Implementation notes-
//       ** THREAD SAFE, BUT NOT CURRENTLY MULTI-THREAD CAPABLE **
//         Polling operations can hold the Selector mutex for long intervals,
//         blocking socket inserts, modification, and removal.
//
//       Sockets may only be associated with one Select object.
//       Sockets are automatically removed from a Select whenever they
//       are opened, closed, or deleted.
//
//       A Selector is intended for use with a large number of sockets.
//       It contains element arrays indexed by the file descriptor, each
//       allocated large enough to contain *all* file descriptors.
//
//----------------------------------------------------------------------------
class Select {                      // Socket selector
//----------------------------------------------------------------------------
// Select::Typdefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::function<void(void)>             v_func; // with_lock() function

//----------------------------------------------------------------------------
// Select::Attributes
//----------------------------------------------------------------------------
protected:
Socket*                reader= nullptr; // Internal reader socket
Socket*                writer= nullptr; // Internal writer socket

mutable std::recursive_mutex
                       select_mutex;    // Internal select mutex
mutable std::recursive_mutex
                       socket_mutex;    // Internal socket mutex
mutable std::recursive_mutex
                       update_mutex;    // Internal update mutex
struct pollfd*         pollfd= nullptr; // Array of pollfd's
Socket**               sarray= nullptr; // Array of Socket's
int*                   sindex= nullptr; // File descriptor to pollfd index

int                    left= 0;     // Number of remaining selections
int                    next= 0;     // The next selection index
int                    size= 0;     // Number of result elements available
int                    used= 0;     // Number of result elements used

//----------------------------------------------------------------------------
// Select::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Select();
   ~Select();

//----------------------------------------------------------------------------
// Select::methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const; // Caller information

void
   with_lock(v_func f)              // Run function holding socket_mutex
{  std::lock_guard<decltype(socket_mutex)> lock(socket_mutex); f(); }

const struct pollfd*                // The associated pollfd
   get_pollfd(                      // Extract pollfd
     const Socket*     socket) const // For this Socket
{  std::lock_guard<decltype(update_mutex)> lock(update_mutex);

   int fd= socket->handle;
   if( fd < 0 || fd >= size ) {     // (Not valid, most likely closed)
     errno= EBADF;
     return nullptr;
   }

   fd= sindex[fd];
   if( fd < 0 || fd >= size ) {     // (Not mapped, most likely user error)
     errno= EINVAL;
     return nullptr;
   }

   return &pollfd[fd];
}

const Socket*                       // The associated Socket*
   get_socket(                      // Extract Socket
     int               fd) const    // For this file descriptor
{  std::lock_guard<decltype(update_mutex)> lock(update_mutex);

   if( fd < 0 || fd >= size ) {
     errno= EINVAL;
     return nullptr;
   }

   return sarray[fd];
}

void
   control( void );                 // Drain control operation queue

int                                 // Return code, 0 expected
   insert(                          // Insert a Socket onto the list
     Socket*           socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   modify(                          // Replace the Socket's poll events mask
     const Socket*     socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   remove(                          // Remove the Selector
     Socket*           socket);     // For this Socket

Socket*                             // The next selected Socket, or nullptr
   select(                          // Select next Socket
     int               timeout);    // Timeout, in milliseconds

Socket*                             // The next selected Socket, or nullptr
   select(                          // Select next Socket
     const struct timespec*         // (tv_sec, tv_nsec)
                       timeout,     // Timeout, infinite if omitted
     const sigset_t*   signals);    // Signal set

//============================================================================
protected:
void
   control(                         // Send control operation
     const control_op& op);         // The control operation

Socket*                             // The next selected Socket
   remain( void );                  // Select next remaining Socket

inline void
   resize(                          // Resize the Select
     int               fd);         // For this file descriptor
}; // class Select
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SELECT_H_INCLUDED
