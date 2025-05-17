//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Select.cpp
//
// Purpose-
//       Select.h method implementations.
//
// Last change date-
//       2025/04/25
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif

#include <atomic>                   // For std::atomic
#include <new>                      // For std::bad_alloc
#include <mutex>                    // For std::mutex, std::lock_guard, ...
#include <stdexcept>                // For std::runtime_error
#include <cassert>                  // For assert
#include <cerrno>                   // For errno
#include <cstdarg>                  // For va_* functions
#include <cstddef>                  // For offsetof
#include <cstring>                  // For memset, ...

#include <endian.h>                 // For be64toh
#include <fcntl.h>                  // For O_NONBLOCK, ...
#include <netdb.h>                  // For addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <unistd.h>                 // For getpid, ...
#include <arpa/inet.h>              // For internet address conversions
#include <sys/resource.h>           // For getrlimit
#include <sys/select.h>             // For select, ...
#include <sys/stat.h>               // For stat, ...
#include <sys/un.h>                 // For sockaddr_un
#include <sys/time.h>               // For timeval, ...

#include <pub/Debug.h>              // For debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/List.h>               // For pub::AI_list<>
#include "pub/Select.h"             // For pub::Select, implemented
#include "pub/Socket.h"             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include "pub/utility.i"            // For conversion routines, ...

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode?
,  IOEM= true                       // I/O error Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  USE_AF= AF_INET                  // Use this address family
,  USE_CHECKING= true               // Use internal cross-checking?
,  USE_DO_SELECT= true              // Use internal socket->handler method?
,  USE_ITRACE= false                // Use internal trace?
}; // enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if EAGAIN == EWOULDBLOCK
#  define IS_BLOCK (errno == EAGAIN )
#else
#  define IS_BLOCK (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

#define IS_RETRY (errno == EINTR)

//----------------------------------------------------------------------------
// Struct control_op | Control operation descriptor
//----------------------------------------------------------------------------
enum OP                             // Select operation codes
{ OP_FLUSH=  'F'
, OP_INSERT= 'I'
, OP_MODIFY= 'M'
, OP_REMOVE= 'R'
}; // enum OP

struct control_op {                 // Control operation
char                   op;          // Operation code
char                   _0001;       // (Reserved for alignment)
uint16_t               events;      // Event mask
int32_t                fd;          // Socket file descriptor handle
Socket*                socket;      // The associated Socket
}; // struct control_op

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static constexpr const char* UNIX_BASE= "/tmp/pub_";  // UNIX base file name
static constexpr const char* INET_HOST= "localhost:"; // INET base host name

static std::atomic_int serial= 0;   // Connector serial number

//----------------------------------------------------------------------------
//
// Subroutine-
//       o2v     control_op [op,events,fd] to void*
//
// Purpose-
//       Convert control_op [op,events,fd] to void*
//
//----------------------------------------------------------------------------
static inline void*
   o2v(const control_op& op)
{  return i2v(i2i(op.op)<<56 | i2i(op.events)<<32 | op.fd); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       op_errno
//
// Purpose-
//       Set errno, return -1
//
//----------------------------------------------------------------------------
static int                          // (Always -1)
   op_errno(int _errno)             // Set errno, return -1
{  errno= _errno; return -1; }      // (Handle errno return)

//----------------------------------------------------------------------------
//
// Subroutine-
//       sno_exception
//
// Purpose-
//       Message: A should not occur situation occured.
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   sno_exception(int line)
{
   errorf("%4d %s Should not occur (but did)\n", line, __FILE__);
   throw std::runtime_error("Should not occur");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trace_ctl
//
// Purpose-
//       Internal Select control operation trace
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace_ctl(                       // Internal Select control op trace
     const Select*     select,      // The Select
     const Socket*     socket,      // The Socket
     const char*       op_unit,     // The (4 character) operation id
     const char*       op_name,     // The control operation name
     const control_op& op)          // The control operation data
{
   if( USE_ITRACE )
     Trace::trace(".SEL", op_unit, select, socket, c2v(op_name), o2v(op));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trace_sel
//
// Purpose-
//       Internal Select trace
//
//----------------------------------------------------------------------------
_LIBPUB_FLATTEN
_LIBPUB_HOT
static inline void
   trace_sel(                       // Internal Select trace
     Select*           select,      // The Select
     Socket*           socket,      // The Socket
     int               events,      // pollfd.events
     int               revents,     // pollfd.events
     int               fd)          // The file descriptor
{
   if( USE_ITRACE ) {
     Trace::Record* R= Trace::trace(sizeof(Trace::Record) + 32);
     if( R ) {
       uintptr_t one= uintptr_t(socket);
       uintptr_t two= uintptr_t( uintptr_t(events) << 48
                               | uintptr_t(revents) << 32 | fd);
       char* const C2= (char*)&(((void const**)R->value)[2]);
       char* const C3= (char*)&(((void const**)R->value)[3]);
       for(unsigned i= sizeof(void*); i>0; --i) {
         C2[i - 1]= char(one);
         C3[i - 1]= char(two);
         one >>= 8;
         two >>= 8;
       }
       ((void const**)R->value)[4]= 0;
       ((void const**)R->value)[5]= 0;
       R->trace(".SEL", "=SEL", select);
     }
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       Connector
//
// Purpose-
//       Only used in a temporary task to create the reader socket
//
//----------------------------------------------------------------------------
namespace {                         // (Anonymous namespace)
class Connector : public Thread {   // Connector Thread
// (Public) attributes
public:
Event                  event;       // The startup Event
Socket                 listen;      // The (single-use) listener socket
Socket*                reader= nullptr; // The reader socket
const Select*          select= nullptr; // The associated Select
std::string            target;      // The connection target

int                    operational= false; // TRUE iff listener active

// Constructor/destructor
   Connector( const Select* owner )
:  Thread(), select(owner)
{  if( HCDM && VERBOSE )
     debugf("Select(%p)::Connector(%p)::Connector\n", owner, this);
   int rc= listen.open(USE_AF, SOCK_STREAM);
   if( rc ) {
     debugf("Select(%p)::Connector(%p): listen open failed\n", select, this);
     return;
   }

   int optval= true;                // (Needed before the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   target= INET_HOST;
   if( USE_AF == AF_UNIX )
     target= UNIX_BASE + std::to_string(++serial);
   rc= listen.bind(target);        // Set connection target
   if( rc ) {
     debugf("Select(%p)::Connector(%p): bind(%s) failed\n", select, this
           , target.c_str());
     return;
   }
   if( USE_AF == AF_INET )
     target= target + std::to_string(listen.get_host_port());

   rc= listen.listen();            // Indicate listener socket
   if( rc ) {
     debugf("Select(%p)::Connector(%p): listen(%s) failed\n", select, this
           , target.c_str());
     return;
   }

   operational= true;
   start();
}

// ~Connector( void ) = default;    // Does nothing
   ~Connector( void )
{  if( HCDM && VERBOSE )
     debugf("Select(%p)::Connector(%p)::~Connector\n", select, this);
}

// Method run ----------------------------------------------------------------
void
   run( void )                      // Accept the Reader connection
{
   // Create reader socket
   while( reader == nullptr && operational )
     reader= listen.accept();

   listen.close();
   if( USE_AF == AF_UNIX )
     unlink(target.c_str());
}
}; // class Connector
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Struct-
//       SelectItem
//
// Purpose-
//       Our list work item
//
// Implementation notes-
//       Although we use pub::dispatch::Item and pub::dispatch::Done, we don't
//       use pub::dispatch::Task. Instead of driving a task, we write (one
//       byte) using our writer->reader socket. This completes the current or
//       (if polling is inactive,) the next polling operation. We handle all
//       enqueued work items at this time.
//
//----------------------------------------------------------------------------
struct SelectItem : public dispatch::Item { // Queue item
control_op             op;          // The control operation
}; // struct SelectItem

//----------------------------------------------------------------------------
//
// Method-
//       Select::Select
//       Select::~Select
//
// Purpose-
//       Constructor
//       Destructor
//
// Implementation notes-
//       Since the Select object can't be referenced until construction
//       completes, we don't obtain locks in the constructor.
//
//----------------------------------------------------------------------------
   Select::Select( void )
{  if( HCDM )
     debugf("Select(%p)::Select\n", this);

   if( USE_ITRACE )
     Trace::trace(".NEW", "=SEL", this);

   Connector connector= this;
   if( connector.operational == false )
     sno_exception(__LINE__);

   writer= new Socket();            // Create the writer Socket
   writer->open(USE_AF, SOCK_STREAM);

   // Set writer Socket options and flags
   int optval= true;
   writer->set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   int rc= writer->connect(connector.target);
   if( rc ) {
     debugf("%4d Select(%p) target(%s) connect error %d:%s\n", __LINE__, this
           , connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }
   connector.join();

   rc= writer->set_flags( writer->get_flags() | O_NONBLOCK );
   if( rc ) {
     debugf("%4d Select(%p)::Select(%s) set_flags error %d:%s\n", __LINE__
           , this, connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }

   reader= connector.reader;
   if( reader == nullptr )
     sno_exception(__LINE__);

   // Manually insert the reader socket into our tables
   int fd= reader->get_handle();
   resize(fd);

   struct pollfd* poll= this->pollfd + 0; // (The first pollfd)
   poll->fd= fd;
   poll->events= POLLIN;
   poll->revents= 0;
   fdpndx[fd]= 0;
   fdsock[fd]= reader;
   ++used;

   rc= reader->set_flags( reader->get_flags() | O_NONBLOCK );
   if( rc ) {
     debugf("%4d Select(%p)::Select(%s) set_flags error %d:%s\n", __LINE__
           , this, connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }
}

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
   Select::~Select( void )
{  if( HCDM )
     debugf("Select(%p)::~Select\n", this);

   if( USE_ITRACE )
     Trace::trace(".DEL", "=SEL", this);

   // Complete any pending operations. Hopefully they're close or flush ops.
   control();

   // Implementation notes:
   // A check for "Sockets still in the Select table" was removed from here.
   // While Select objects contain Socket references, the reference to a
   // Select object from a Socket has been removed, fixing a design  flaw.
   // Sockets still do provide a std::function that Select invokes, but we're
   // deleting the Select, so this Select (at least) isn't going to invoke it.
   //
   // To avoid an error message, we used to manually remove the reader Socket
   // from our tables before making that check. That's no longer needed.
   //
   // We're in the Select destructor. Latching isn't useful. Any other thread
   // that could still access this Select will SEGFAULT if it does.

   free(pollfd);
   free(fdpndx);
   free(fdsock);
   delete reader;
   delete writer;

   pollfd= nullptr;
   fdpndx= nullptr;
   fdsock= nullptr;
   reader= nullptr;
   writer= nullptr;
   used= size= next= ipix= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::debug
//
// Purpose-
//       Debugging display
//
// Implementation note-
//       Caller must hold either the shr_latch or the xcl_latch.
//
//       Socket close operations, which queue a remove operation, may complete
//       before the remove operation is processed leaving the Socket* invalid.
//       Accessing a Socket* can result in a SEGFAULT, so we don't do it.
//
//----------------------------------------------------------------------------
int                                 // Number of detected errors
   Select::debug(                   // Debugging display
     const char*       info) const  // Caller information
{
   int error_count= 0;

   debugf("Select(%p)::debug(%s)\n", this, info);
   debugf("..reader(%p) handle(%d)\n", reader, reader->get_handle());
   debugf("..writer(%p) handle(%d)\n", writer, writer->get_handle());
   debugf("..pollfd(%p) fdpndx(%p) fdsock(%p)\n", pollfd, fdpndx, fdsock);
   debugf("..ipix(%u) next(%u) size(%u) used(%u)\n", ipix, next, size, used);
   debugf("..pollfd %d\n", used);
   for(int px= 0; px<used; ++px) {
     int fd= pollfd[px].fd;
     const Socket* socket= fdsock[fd];
     debugf("....[%4d] fd[%.4x] pollfd{%.4x,%.4x} socket(%p)\n", px, fd
           , pollfd[px].events, pollfd[px].revents, socket);
     if( socket == nullptr ) {
       ++error_count;
       debugf("....[%4d] ERROR: NO ASSOCIATED SOCKET\n", px);
     }
     if( px != fdpndx[fd] ) {
       ++error_count;
       debugf("....[%4d] fd[%.4x] ERROR: [%4d] != fdpndx[%.4x]\n", px, fd
              , px, fdpndx[fd]);
     }
   }

   debugf("..fdpndx\n");
   for(int fd= 0; fd<size; ++fd) {
     if( fdpndx[fd] >= 0 )
       debugf("....[%.4x] px[%4d]\n", fd, fdpndx[fd]);
   }

   debugf("..fdsock\n");
   for(int sx= 0; sx<size; ++sx) {
     Socket* socket= fdsock[sx];
     if( socket ) {
       if( socket->get_handle() == sx )
         debugf("....[%.4x] %p\n", sx, socket);
       else
         debugf("....[%.4x] %p->[%.4x] *ERROR*\n", sx, socket
               , socket->get_handle());
     }
   }

   SelectItem* item= (SelectItem*)todo_list.get_tail();
   debugf("..dolist(%p) [tail..head]\n", item);
   while( item ) {
     Item* next= item->get_prev();
     control_op& op= item->op;
     debugf("....%.12zx->%.12zx {%c,%.4x,%.4x,%p}\n"
           , intptr_t(item), intptr_t(next)
           , op.op, op.events, op.fd, op.socket);
     item= (SelectItem*)next;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::close
//
// Purpose-
//       Close the Socket
//
// Implementation note-
//       Calling close when aready closed is an (ignorable) error.
//       Close *MUST NOT* be called from a Socket asynchronous event handler.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Select::close(                   // Close
     Socket*           socket)      // This Socket
{  if( HCDM )
     debugh("Select(%p)::close(%p) handle(%d)\n", this, socket
           , socket ? socket->get_handle() : -1);

   int fd= socket ? socket->get_handle() : -1;
   if( fd < 0 )
     return op_errno(EINVAL);

   if( fd >= size || fdsock[fd] != socket ) // If inconsistent
     return op_errno(EINVAL);

   remove(socket);                  // Remove the Socket
   flush();                         // (Force remove completion)

   return socket->close();          // Close the Socket
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::empty
//
// Purpose-
//       Remove *all* application Sockets from the Select, leaving it empty
//
//----------------------------------------------------------------------------
void
   Select::empty( void )            // Empty the Select
{  if( HCDM )
     debugh("Select(%p)::empty\n", this);

   flush();                         // (We shouldn't leave operations pending)

   std::unique_lock<decltype(xcl_latch)> lock(xcl_latch);

   for(int px= 1; px < used; ++px) { // (Skipping the reader)
     int fd= pollfd[px].fd;
     if( fd >= 0 && fd < size ) {
       Socket* socket= fdsock[fd];
       if( socket ) {               // (The Socket handle isn't relevant)
         fdsock[fd]= nullptr;
         fdpndx[fd]= -1;
       }
     }
   }

   // Initial state: Only the reader socket remains
   ipix= next= 0;
   used= 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::control( const control op& )
//
// Purpose-
//       Enqueue a control operations
//
// Implementation notes-
//       Since adding an item to the queue and writing to complete the
//       polling operation are separate operations, it's possible for
//       writer->write or reader->read to block. In either case, this is
//       treated as if the blocked operation completed sucessfully.
//
//----------------------------------------------------------------------------
void
   Select::control(                 // Transmit control operation
     const control_op& op)          // The operation to send
{  if( HCDM )
     debugh("Select(%p)::control({%c,%.4x,%.4x,%p})\n", this
           , op.op, op.events, op.fd, op.socket);

   if( USE_ITRACE )
     Trace::trace(".SEL", ">CTL", this, c2v("ENQUEUE"), op.socket, o2v(op));


//debugf("%4d HCDM %s=%s %zx\n", __LINE__, u.buffer, _c, size_t(be64toh(v2i(u.result[0]))));

   SelectItem* item= new SelectItem();
   item->op= op;
   Item* tail= todo_list.fifo(item);

   if( tail == nullptr ) {
     ssize_t L= writer->write(&op.op, 1);
     while( L < 0 ) {
       if( IS_BLOCK )
         break;

       if( !IS_RETRY ) {
         debugf("Select(%p)::control write error: %d:%s\n", this
               , errno, strerror(errno));
         sno_exception(__LINE__);
       }

       L= writer->write(&op.op, 1);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::control( void )
//
// Purpose-
//       Read and handle all queued control operations
//
// Implementation notes-
//       See implementation note in control(const control_op&), above.
//
//       Obtains xcl_latch. Callers MUST NOT hold shr_latch OR xcl_latch.
//       It cannot be called from any Socket::on_select function since they
//       may be called with the shr_latch held. Note that Socket::close
//       invokes this method indirectly when it invokes select->flush.
//
//----------------------------------------------------------------------------
void
   Select::control( void )          // Read and handle queued operations
{  if( HCDM )
     debugh("Select(%p)::control\n", this);

   std::unique_lock<decltype(xcl_latch)> lock(xcl_latch);

   char buffer[8];                  // (Only one byte should be used)
   ssize_t L= reader->read(buffer, sizeof(buffer));
   while( L < 0 ) {
     if( IS_BLOCK )
       break;

     if( !IS_RETRY ) {
       debugf("Select(%p)::control read error: %d:%s\n", this
             , errno, strerror(errno));
       sno_exception(__LINE__);
     }

     L= reader->read(buffer, sizeof(buffer));
   }

   if( pollfd[0].revents )
     pollfd[0].revents= 0;

   // Process (all) queued operations
   for(auto it= todo_list.begin(); it != todo_list.end(); ++it) {
     SelectItem* item= static_cast<SelectItem*>(it.get());
     control_op& op= item->op;

     Socket* socket= op.socket;
     int fd= op.fd;

     switch( op.op ) {
       case OP_FLUSH: {
         if( USE_ITRACE )
           Trace::trace(".SEL", "=FSH", this, c2v("FLUSH"));
         break;
       }
       case OP_INSERT: {
         if( USE_ITRACE )
           Trace::trace(".SEL", "=INS", this, c2v("INSERT")
                       , socket, o2v(op));

         if( fd < 0 )
           sno_exception(__LINE__); // This is an INTERNAL ERROR

         if( fd >= size )
           resize(fd);

         if( fdsock[fd] ) {
           // Error: another socket's already using the file descriptor
           debugh("Select(%p)::insert(%p) fdsock[%d](%p)\n", this
                 , socket, fd, fdsock[fd]);
           debug("HCDM");
           sno_exception(__LINE__); // This is a USER ERROR
         }

         // Perform the insert
         struct pollfd* poll= this->pollfd + used;
         poll->fd= fd;
         poll->events= op.events;
         poll->revents= 0;
         fdpndx[fd]= used;
         fdsock[fd]= const_cast<Socket*>(socket);

         ++used;
         break;
       }
       case OP_MODIFY: {
         if( USE_ITRACE )
           Trace::trace(".SEL", "=MOD", this, c2v("MODIFY")
                       , socket, o2v(op));

         if( fd < 0 || fd >= size )
           sno_exception(__LINE__); // This is an INTERNAL ERROR

         int px= fdpndx[fd];
         if( px >= 0 && px < used && fdsock[fd] == socket ) {
           struct pollfd* poll= this->pollfd + px;
           poll->events= op.events;
           poll->revents= 0;
           break;
         }

         // Consistency check failed
         debugh("Select(%p)::modify(%p) fdsock[%d](%p) px(%d) used(%d)\n"
               , this, socket, fd, fdsock[fd], px, used);
         debug("HCDM");
         sno_exception(__LINE__);
         break;
       }
       case OP_REMOVE: {
         if( USE_ITRACE )
           Trace::trace(".SEL", "=REM", this, c2v("REMOVE")
                       , socket, o2v(op));

         if( fd < 0 || fd >= size )
           sno_exception(__LINE__); // This is an INTERNAL ERROR

         if( fdsock[fd] != socket ) // If duplicate remove
           break;

         int px= fdpndx[fd];
         if( px > 0 && px < used ) {
           --used;
           for(int i= px; i<used; ++i) {
             pollfd[i]= pollfd[i+1];
             fdpndx[pollfd[i].fd]= i;
           }

           fdsock[fd]= nullptr;
           fdpndx[fd]= -1;
           if( px <= ipix )
             --ipix;
           if( px == next )
             --next;
           break;
         }

         // Consistency check failed
         debugh("Select(%p)::remove(%p) fdsock[%d](%p) px(%d) used(%d)\n"
               , this, socket, fd, fdsock[fd], px, used);
         debug("HCDM");
         sno_exception(__LINE__);
         break;
       }
       default:
         sno_exception(__LINE__);
     }

     item->post();                  // (Uses dispatch post logic)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::flush
//
// Purpose-
//       Insure all enqueued operations have completed.
//
// Implementation notes-
//       May be used whether or not polling is active, but
//       *MUST NOT* be called from a Socket asynchronous event handler.
//
//----------------------------------------------------------------------------
void
   Select::flush( void )            // Insure operation completion
{  if( HCDM )
     debugh("Select(%p)::flush\n", this);

   control_op op= {OP_FLUSH, 0, 0, 0, nullptr};
   control(op);
   control();                       // Chase any pending operations
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::insert
//
// Purpose-
//       Insert a Socket onto the Socket array
//
// Implementation notes-
//       Operation does not complete until next polling operation
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::insert(                  // Insert Socket
     Socket*           socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::insert(%p,0x%.4x) fd(%d)\n", this
           , socket, events, socket->get_handle());

   int fd= socket->get_handle();
   if( fd < 0 )
     return op_errno(EINVAL);

   control_op op= {OP_INSERT, 0, (uint16_t)events, fd, socket};
   control(op);                     // Enqueue the INSERT operation

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::modify
//
// Purpose-
//       Modify a Socket's events mask
//
// Implementation notes-
//       Operation does not complete until next polling operation
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::modify(                  // Modify Socket
     Socket*           socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::modify(%p,0x%.4x)\n", this, socket, events);

   int fd= socket->get_handle();
   if( fd < 0 || fd >= size || fdsock[fd] != socket ) // If inconsistent
     return op_errno(EINVAL);

   control_op op= {OP_MODIFY, 0, (uint16_t)events, fd, socket};
   control(op);

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::remove
//
// Purpose-
//       Remove a Socket from the Socket array
//
// Implementation notes-
//       Operation does not complete until next polling operation
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::remove(                  // Remove Socket
     Socket*           socket)      // The associated Socket
{  if( HCDM )
     debugh("Select(%p)::remove(%p) fd(%d)\n", this
           , socket, socket->get_handle());

   // The error checks and the enqueue needs to be done while holding the
   // shr_latch to insure that fdpndx[fd] refers to the removed socket.
   std::lock_guard<decltype(shr_latch)> lock(shr_latch);

   int fd= socket->get_handle();
   if( fd < 0 || fd >= size )       // If closed or invalid file descriptor
     return op_errno(EINVAL);

   int px= fdpndx[fd];
   if( fdsock[fd] != socket || px < 0 || px >= used ) {
#if 0 // Attempt to debug this
     Trace::trace(".SEL", "RBUG", this, c2v("DEBUG")
                 , socket, i2v(fd), i2v(px), i2v(__LINE__));
     Trace::stop();                 // Terminate tracing
     debugf("%4d %s *UNEXPECTED* %p [%.4x] %d\n", __LINE__, __FILE__
           , socket, fd, px);
     debug("unexpected");
     // sno_exception(__LINE__);
#else
     return op_errno(EINVAL);
#endif
   }

   pollfd[px].revents= 0;           // Don't report events
   pollfd[px].events= 0;            // Don't poll for new events

   control_op op= {OP_REMOVE, 0, 0, fd, socket};
   control(op);                     // Enqueue the REMOVE operation

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::select
//
// Purpose-
//       Select the next available Socket from the Socket array w/timeout
//
//----------------------------------------------------------------------------
Socket*                             // The next selected Socket, or nullptr
   Select::select(                  // Select next Socket
     int               timeout)     // Timeout, in milliseconds
{  if( HCDM && VERBOSE > 1 )
     debugh("Select(%p)::select(%d)\n", this, timeout);

   // Handle pending control operations
   if( todo_list.get_tail() != nullptr || pollfd[0].revents )
     control();

   Socket* socket= select();
   if( socket )
     return socket;

   {{{{
     std::lock_guard<decltype(shr_latch)> lock(shr_latch);

     // POLL operation
     for(int px= 0; px<used; ++px)
       pollfd[px].revents= 0;

     int rc= poll(pollfd, used, timeout);
     while( rc < 0 && IS_RETRY )
       rc= poll(pollfd, used, timeout);

     if( rc == 0 ) {                // If poll timeout
       ipix= 0;
       return nullptr;
     }

     if( rc < 0 ) {                 // If poll I/O error (should not occur)
       if( USE_ITRACE ) {
         Trace::trace(".SEL", "PERR", this, i2v(errno));
         Trace::stop();
       }
       debugf("Select(%p)::select poll error %d:%s\n", this
             , errno, strerror(errno));
       debug("poll error");
       sno_exception(__LINE__);
     }

     if( next == 0 )
       next= 1;
     ipix= next;

     if( USE_ITRACE )
       Trace::trace(".SEL", "POLL", this, i2v(i2i(next)<<32 | rc));
   }}}}

   return select();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// IMPLEMENTATION NOTE: NOT TESTED
Socket*                             // The next selected Socket, or nullptr
   Select::select(                  // Select next Socket
     const struct timespec*         // (tv_sec, tv_nsec)
                       timeout,     // Timeout, infinite if omitted
     const sigset_t*   signals)     // Signal set
{  if( HCDM )
     debugh("Select(%p)::select({%'ld,%'ld},%p)\n", this
           , timeout->tv_sec, timeout->tv_nsec, signals);

   // Handle pending control operations
   if( todo_list.get_tail() != nullptr || pollfd[0].revents )
     control();

   Socket* socket= select();
   if( socket )
     return socket;

   {{{{
     std::lock_guard<decltype(shr_latch)> lock(shr_latch);

     // POLL operation
     for(int px= 0; px<used; ++px)
       pollfd[px].revents= 0;

     int rc= ppoll(pollfd, used, timeout, signals);
     while( rc < 0 && IS_RETRY )
       rc= ppoll(pollfd, used, timeout, signals);

     if( rc == 0 ) {                  // If poll timeout
       ipix= 0;
       return nullptr;
     }

     if( rc < 0 ) {                   // If poll I/O error (should not occur)
       if( USE_ITRACE ) {
         Trace::trace(".SEL", "PERR", this, i2v(errno));
         Trace::stop();
       }
       debugf("Select(%p)::select ppoll error %d:%s\n", this
             , errno, strerror(errno));
       debug("ppoll error");
       sno_exception(__LINE__);
     }

     if( next == 0 )
       next= 1;
     ipix= next;

     if( USE_ITRACE )
       Trace::trace(".SEL", "POLL", this, i2v(i2i(next)<<32 | rc));
   }}}}

   return select();
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Select::select( void )
//
// Purpose-
//       Select the next remaining Socket
//
//----------------------------------------------------------------------------
Socket*                             // The next selected Socket
   Select::select( void )           // Select the next remaining Socket
{  if( HCDM )
     debugh("%4d Select(%p) USE_DO_SELECT(%s)\n", __LINE__, this
           , USE_DO_SELECT ? "true"  : "false");

   std::unique_lock<decltype(shr_latch)> lock(shr_latch);

   if( ipix == 0 )
     return nullptr;                // Poll complete

   // Note that we only check for pending control operations once even when
   // USE_DO_SELECT == true. Socket on_select functions are supposed to be low
   // overhead so even multiple instances shouldn't significant much delay.
   if( todo_list.get_tail() != nullptr || pollfd[0].revents )
     return nullptr;                // Control operation pending

   /**************************************************************************
   There are two mechanisms for handling polling events. The choice is
   determined by the USE_DO_SELECT definition (in enum above.)

   With USE_DO_SELECT == true:
     All pending events are driven from this method with the shr_latch held.
     This method always returns nullptr.

   With USE_DO_SELECT == false:
     Sockets are returned one by one to the polling driver with no lock held.
     This method only returns nullptr when all events have been handled.

   It hasn't been determined which method is better.
   We may need two select functions to allow the caller to select the
   mechanism rather than semi-hard coding the choice here.
   **************************************************************************/
   if( next >= ipix ) {
     for(int px= next; px<used; ++px) {
       struct pollfd* poll= this->pollfd + px;
       int revents= poll->revents;
       if( revents ) {
         next= px + 1;
         int fd= poll->fd;
         Socket* socket= fdsock[fd];
         trace_sel(this, socket, poll->events, revents, fd);
         if( USE_DO_SELECT ) {      // Use internal do_select mechanism?
           socket->do_select(revents); // (Holding shr_latch)
           // return nullptr;       // (Process *all* events, not just one)
         } else {
           return socket;           // (Caller will do_select w/o shr_latch)
         }
       }
     }
     next= 1;
   }

   for(int px= next; px<ipix; ++px) {
     struct pollfd* poll= this->pollfd + px;
     int revents= poll->revents;
     if( revents != 0 ) {
       next= px + 1;
       if( next == ipix )
         ipix= 0;
       int fd= poll->fd;
       Socket* socket= fdsock[fd];
       trace_sel(this, socket, poll->events, revents, fd);
       if( USE_DO_SELECT ) {      // Use internal do_select mechanism?
         socket->do_select(revents); // (Holding shr_latch)
         // return nullptr;       // (Process *all* events, not just one)
       } else {
         return socket;
       }
     }
   }

   ipix= 0;
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Select::resize
//
// Purpose-
//       Resize the internal tables
//
// Implementation note-
//       Caller must hold xcl_latch.
//       Uses 20 bytes per fd.
//
//----------------------------------------------------------------------------
inline void
   Select::resize(                  // Resize the Select tables
     int               fd)          // For this file (positive) descriptor
{  if( HCDM )
     debugf("Select(%p)::resize(%d)\n", this, fd);

   int new_size= 0;
   if( fd < 32 )
     new_size= 32;
   else if( fd < 128 )
     new_size= 128;
   else if( fd < 512 )
     new_size= 512;
   else {
     struct rlimit limits;
     int rc= getrlimit(RLIMIT_NOFILE, &limits);
     if( rc ) {
       errorf("%4d %s %d=getrlimit %d:%s\n", __LINE__, __FILE__, rc
             , errno, strerror(errno));
       limits.rlim_cur= 1024;
       limits.rlim_max= 4096;
     }

     if( size_t(fd) < size_t(limits.rlim_cur) )
       new_size= limits.rlim_cur;
     else if( size_t(fd) < size_t(limits.rlim_max) )
       new_size= limits.rlim_max;
     else {
       // Request for file descriptor index >= limits.rlim_max
       // This should not be possible.
       debugf("%4d Select fd(%d) >= limit(%ld)\n", __LINE__
             , fd, limits.rlim_max);
       sno_exception(__LINE__);
     }
   }

   struct pollfd*
   new_pollfd= (struct pollfd*)realloc(pollfd, new_size * sizeof(pollfd));
   int* new_fdpndx= (int*)realloc(fdpndx, new_size * sizeof(int));
   Socket** new_fdsock= (Socket**)realloc(fdsock, new_size * sizeof(Socket*));
   if( new_pollfd == nullptr||new_fdsock == nullptr||new_fdpndx == nullptr ) {
     if( new_pollfd )               // Replace reallocated (and moved) areas
       pollfd= new_pollfd;          // (Leaving extra space undefined)
     if( new_fdpndx )
       fdpndx= new_fdpndx;
     if( new_fdsock )
       fdsock= new_fdsock;

     throw std::bad_alloc();        // (One or more realloction failures)
   }

   int diff= new_size - size;
   memset(new_pollfd + size, 0x00, diff * sizeof(pollfd));
   memset(new_fdpndx + size, 0xff, diff * sizeof(int));
   memset(new_fdsock + size, 0x00, diff * sizeof(Socket*));

   pollfd= new_pollfd;
   fdpndx= new_fdpndx;
   fdsock= new_fdsock;
   size= new_size;
}
} // namespace _LIBPUB_NAMESPACE
