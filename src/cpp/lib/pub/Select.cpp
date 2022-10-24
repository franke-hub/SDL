//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Select.cpp
//
// Purpose-
//       Select method implementations.
//
// Last change date-
//       2022/10/23
//
// Implementation notes-
//       TODO: rename this. Maybe: Asynch, Poller, something better
//       TODO: Add polling-timeout flag
//       TODO: Add queue to control function. Instead of waiting, control(op)
//           allocates and inserts a queue element that control() examines.
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif

#include <atomic>                   // For std::atomic
#include <new>                      // For std::bad_alloc
#include <mutex>                    // For mutex, std::lock_guard, ...
#include <stdexcept>                // For std::runtime_error

#include <assert.h>                 // For assert
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_NONBLOCK, ...
#include <netdb.h>                  // For addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <stdarg.h>                 // For va_* functions
#include <stddef.h>                 // For offsetof
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For getpid, ...
#include <arpa/inet.h>              // For internet address conversions
#include <sys/resource.h>           // For getrlimit
#include <sys/select.h>             // For select, ...
#include <sys/stat.h>               // For stat, ...
#include <sys/un.h>                 // For sockaddr_un
#include <sys/time.h>               // For timeval, ...

#include <pub/utility.h>            // For to_string(), ...
#include <pub/Debug.h>              // For debugging
#include <pub/Event.h>              // For pub::Event
#include "pub/Select.h"             // For pub::Select, implemented
#include "pub/Socket.h"             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define HCDM false                  // Hard Core Debug Mode?
enum
{  xxxx= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode?
,  IOEM= true                       // I/O error Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  USE_AF= AF_INET                  // Use this address family
,  USE_CHECKING= true               // Use internal cross-checking?
}; // enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if HCDM                            // Bringup debugging TODO: REMOVE
#  define DEBUGGING(x) {int ERRNO= errno; x; errno= ERRNO;}
#else
#  define DEBUGGING(x)
#endif

#if EAGAIN == EWOULDBLOCK
#  define IS_BLOCK (errno == EAGAIN )
#else
#  define IS_BLOCK (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

#define IS_RETRY (errno == EINTR)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static constexpr const char* UNIX_BASE= "/tmp/pub_";  // UNIX base file name
static constexpr const char* INET_HOST= "localhost:"; // INET base host name

static std::atomic_int serial= 0;   // Connector serial number

//----------------------------------------------------------------------------
//
// Subroutine-
//       do_again
//
// Purpose-
//       Handle do again condition
//
//----------------------------------------------------------------------------
static Socket*                      // (Always nullptr)
   do_again( void )                 // Handle do again condition
{
   errno= EAGAIN;
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       i2v
//
// Purpose-
//       Convert integer to void*
//
//----------------------------------------------------------------------------
static void*
   i2v(intptr_t i)
{  return (void*)i; }

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
//       sno_handled
//
// Purpose-
//       Message: A should not occur situation has been handled
//
//----------------------------------------------------------------------------
static int
   sno_handled(int line)
{  errorf("%4d %s Should not occur (but handled)\n", line, __FILE__);
   return 0;
}

namespace select::detail {
//----------------------------------------------------------------------------
//
// Class-
//       select::detail::Connector
//
// Purpose-
//       Temporary task: Create the reader socket
//
//----------------------------------------------------------------------------
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
DEBUGGING( debugf("%4d Select target(%s)\n", __LINE__, target.c_str()); )
   rc= listen.bind(target);        // Set connection target
   if( rc ) {
     debugf("Select(%p)::Connector(%p): bind(%s) failed\n", select, this
           , target.c_str());
     return;
   }
   if( USE_AF == AF_INET )
     target= target + std::to_string(listen.get_host_port());
DEBUGGING( debugf("%4d Select target(%s)\n", __LINE__, target.c_str()); )

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
   run( void )                      // Operate the Thread
{
DEBUGGING( debugh("Select(%p)::Connector(%p)\nCreated listener %s\n", select, this, target.c_str()); )
   // Create reader socket
   while( reader == nullptr && operational ) {
     reader= listen.accept();
   }
DEBUGGING( debugh("%4d Select HCDM reader(%p)\n", __LINE__, reader); )

   listen.close();
   if( USE_AF == AF_UNIX )
     unlink(target.c_str());
DEBUGGING( debugh("Select(%p)::Connector(%p)\nRemoved listener %s\n", select, this, target.c_str()); )
}
}; // class Connector
}  // namespace select::detail

//----------------------------------------------------------------------------
//
// Struct-
//       control_op
//
// Purpose-
//       Select internal operators
//
//----------------------------------------------------------------------------
enum OP                             // Select operation codes
{ OP_INSERT= 'I'
, OP_MODIFY= 'M'
, OP_REMOVE= 'R'
, OP_TICKLE= 'T'
}; // enum OP

struct control_op {                 // Control operation
char                   op;          // Operation code
char                   _0001;       // (Reserved for alignment)
uint16_t               events;      // Event mask
int32_t                fd;          // Socket file descriptor handle
}; // struct control_op

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

   select::detail::Connector connector= this;
   if( connector.operational == false )
     sno_exception(__LINE__);
DEBUGGING( debugf("Select(%p)::Connector(%p) target(%s)\n", this, &connector, connector.target.c_str()); )

   writer= new Socket();            // Create the writer Socket
   writer->open(USE_AF, SOCK_STREAM);

   // Set writer Socket options and flags
   int optval= true;
   writer->set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

DEBUGGING( debugf("%4d Select HCDM\n", __LINE__); )
   int rc= writer->connect(connector.target);
   if( rc ) {
     debugf("%4d Select(%p) target(%s) connect error %d:%s\n", __LINE__, this
           , connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }
   connector.join();
DEBUGGING(
   debugf("%4d Select HCDM rc(%d) writer(%p) reader(%p)\n", __LINE__, rc
         , writer, connector.reader);
)

   rc= writer->set_flags( writer->get_flags() | O_NONBLOCK );
   if( rc ) {
     debugf("%4d Select(%p)::Select(%s) set_flags error %d:%s\n", __LINE__
           , this, connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }
DEBUGGING( debugf("\n\n"); )
DEBUGGING( writer->debug("WRITER BRINGUP TEST"); )

   reader= connector.reader;
   if( reader == nullptr )
     sno_exception(__LINE__);

DEBUGGING( debugf("\n\n"); )
   // Manually insert the reader socket into our tables
   int fd= reader->get_handle();
   resize(fd);

   struct pollfd* pollfd= this->pollfd + 0; // (The first pollfd)
   pollfd->fd= fd;
   pollfd->events= POLLIN;
   pollfd->revents= 0;
   sindex[fd]= 0;
   sarray[fd]= reader;
   reader->selector= this;
   ++used;

DEBUGGING( debugf("\n\n"); )
DEBUGGING( reader->debug("READER BRINGUP TEST"); )
   rc= reader->set_flags( reader->get_flags() | O_NONBLOCK );
   if( rc ) {
     debugf("%4d Select(%p)::Select(%s) set_flags error %d:%s\n", __LINE__
           , this, connector.target.c_str(), errno, strerror(errno));
     sno_exception(__LINE__);
   }
}

   Select::~Select( void )
{  if( HCDM )
     debugf("Select(%p)::~Select\n", this);

   // Complete any pending operations. Hopefully they're close ops.
   control();

   // Manually remove our reader socket from our tables
   if( reader ) {                   // If we have a reader socket
     int fd= reader->get_handle();  // Get the poll index
     int px= sindex[fd];
     for(int i= px; i<(used-1); ++i)
       pollfd[i]= pollfd[i+1];

     sarray[fd]= nullptr;
     sindex[fd]= -1;
     reader->selector= nullptr;
     --used;
   }

   // Delete the AF_UNIX file node
   if( USE_AF == AF_UNIX && false )
     unlink(reader->get_unix_name());

// Locking here can't be necessary. Consider that if it was, then right after
// the lock's released any waiter's going to reference deallocated storage.
// Users must insure that there are no dangling references to deleted Select
// objects. That is, all Sockets must be removed.
// We obtain the shr_latch anyway so that if a dangling reference error
// occurs, debugging information is more consistent.

// Disassociating a Socket from the Select *shouldn't* be necessary here.
// If it was, then there's a good chance that whatever code was using the
// Select will still think it exists.
// An example might help explain why:
// Select accesses are protected by the xcl_latch. A Select references
// each inserted Socket and each inserted Socket references the Select.
// Suppose the Select destructor is called and then an associated Socket
// is closed in a different thread before the destructor completes. The Socket
// close operation invokes Select::remove, which blocks but then resumes
// after the destructor exits. Select::remove then references the (at
// least partially) deleted Select.
// Perhaps we could gimshuckle some way of fixing this particular problem, but
// applications also need to correlate Select and Socket references and
// insure that their Select object isn't deleted while Sockets reference
// it. We can't check the application's correlation method, we can only check
// our own. So we check, and if there's a *possible* dangling reference we
// complain knowing that if a problem does exist, it will be hard to debug.

// >>>>>>>>>>>>>>>>>>>>>>>> ** USER DEBUGGING NOTE ** <<<<<<<<<<<<<<<<<<<<<<<<
// Before deleting a Select object, you should insure that no Socket
// objects still reference it. That will remove the annoying error message
// you got and quite likely also avoid some hard to debug future error.
// >>>>>>>>>>>>>>>>>>>>>>>> ** USER DEBUGGING NOTE ** <<<<<<<<<<<<<<<<<<<<<<<<
   std::lock_guard<decltype(shr_latch)> lock(shr_latch);
   for(int px= 0; px < used; ++px) {
     int fd= pollfd[px].fd;
     if( fd >= 0 && fd < size ) {
       Socket* socket= sarray[fd];
       if( socket ) {
         #define FMT "%4d Select(%p) Socket(%p) fd(%d) User error: " \
                     "Dangling reference\n"
         errorf(FMT, __LINE__, this, socket, fd);
         sno_handled(__LINE__);     // See ** USER DEBUGGING NOTE **, above
         debug("Additional debugging information");
         socket->selector= nullptr;
       } else if( USE_CHECKING ) {
         sno_handled(__LINE__);     // (socket[fd] == nullptr)
       }
     } else if( USE_CHECKING ) {
       sno_handled(__LINE__);       // (pollfd[px].fd >= size)
     }
   }

   free(pollfd);
   free(sarray);
   free(sindex);
   delete reader;
   delete writer;

   pollfd= nullptr;
   sarray= nullptr;
   sindex= nullptr;
   reader= nullptr;
   writer= nullptr;
   size= 0;
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
//       This may be called with or without the shr_latch held, but not with
//       the xcl_latch held.
//
//       struct pollfd* pollfd;     // Array of pollfd's 0..used
//       Socket**       sarray;     // Array of Socket's, indexed by fd
//       int*           sindex;     // File descriptor to pollfd index
//
//----------------------------------------------------------------------------
int                                 // Number of detected errors
   Select::debug(                   // Debugging display
     const char*       info) const  // Caller information
{
   std::lock_guard<decltype(shr_latch)> lock(shr_latch);

   int error_count= 0;

   debugf("Select(%p)::debug(%s)\n", this, info);
   debugf("..reader(%p) handle(%d)\n", reader, reader->get_handle());
   debugf("..writer(%p) handle(%d)\n", writer, writer->get_handle());
   debugf("..pollfd(%p) sarray(%p) sindex(%p)\n", pollfd, sarray, sindex);
   debugf("..left(%u) next(%u) size(%u) used(%u)\n"
         , left.load(), next, size, used);
   debugf("..pollfd %d\n", used);
   for(int px= 0; px<used; ++px) {
     int fd= pollfd[px].fd;
     const Socket* socket= sarray[fd];
     debugf("....[%3d] fd[%3d] pollfd{%.4x,%.4x} socket(%p)\n", px, fd
           , pollfd[px].events, pollfd[px].revents, socket);
     if( socket == nullptr ) {
       ++error_count;
       debugf("....[%3d] ERROR: NO ASSOCIATED SOCKET\n", px);
     } else if( socket->handle != pollfd[px].fd ) {
       ++error_count;
       debugf("....[%3d] ERROR: SOCKET.HANDLE(%3d) FD(%3d) MISMATCH\n", px
             , socket->handle, fd);
     }
     if( px != sindex[fd] ) {
       ++error_count;
       debugf("....[%3d] ERROR: SINDEX[%3d] MISMATCH(%3d)\n", px, fd
              , sindex[fd]);
     }
   }

   debugf("..sarray\n");
   for(int sx= 0; sx<size; ++sx) {
     Socket* socket= sarray[sx];
     if( socket ) {
       debugf("....[%3d] %p->(%3d)\n", sx, socket, socket->handle);
       if( sx != socket->handle ) {
         ++error_count;
         debugf("....[%3d] ERROR: SOCKET HANDLE MISMATCH\n", sx);
       }
     }
   }

   debugf("..sindex\n");
   for(int fd= 0; fd<size; ++fd) {
     if( sindex[fd] >= 0 )
       debugf("....[%3d] -> [%3d]\n", fd, sindex[fd]);
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::control( const control op& )
//
// Purpose-
//       Enqueue a control operations
//
// Implentation notes-
//       TODO: instead of waiting, enqueue a work element that's checked
//       by control. (Ops need to be handled in order. How do we synchronize?)
//       Maybe add a sequence number to the op.
//
//----------------------------------------------------------------------------
void
   Select::control(                 // Transmit control operation
     const control_op& op)          // The operation to send
{  if( HCDM )
     debugh("Select(%p)::control({%c,,0x%.4x,%d})\n", this
           , op.op, op.events, op.fd);

DEBUGGING( debugh("WR control {%c, %.4x, %d}\n", op.op, op.events, op.fd); )
   for(uint32_t spinCount= 1;;spinCount++) { // Retry until successful
     ssize_t L= writer->write(&op, sizeof(op));
     if( L == sizeof(op) )
       return;                      // (Operation queued)
     int ERRNO= errno;              // (Preserve errno)
DEBUGGING( debugh("%zd= writer->write() %d%s\n", L, ERRNO, strerror(ERRNO)); )

     if( !IS_BLOCK && !IS_RETRY ) { // If unexpected error
       debugh("Select(%p)::control({%c,,0x%.4x,%d}) %d:%s\n", this
             , op.op, op.events, op.fd, ERRNO, strerror(ERRNO));
       sno_exception(__LINE__);
     }

     // All we can do is wait. We use progressively longer wait intervals.
     if( spinCount & 0x0000000f )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
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
//       Callers MUST NOT hold shr_latch OR xcl_latch: Obtains xcl_latch
//
//----------------------------------------------------------------------------
void
   Select::control( void )          // Read and handle queued operations
{  if( HCDM )
     debugh("Select(%p)::control\n", this);

   ssize_t             L;           // Input read length
   control_op          op;          // Input data area

   std::lock_guard<decltype(ctl_latch)> ctl_lock(ctl_latch);
   std::unique_lock<decltype(xcl_latch)> lock(xcl_latch);

   for(;;) {
     L= reader->read(&op, sizeof(op));
DEBUGGING( debugh("%zd= read() %d:%s\n", L, errno, strerror(errno)); )
     if( L != sizeof(op) )
       break;
DEBUGGING( debugh("RD control {%c, %.4x, %d}\n", op.op, op.events, op.fd); )

     if( op.op == OP_TICKLE ) {
       Trace::trace(".SEL", ".NOP");
       continue;
     }

     // File descriptor and socket->handle consistency checks
     int fd= op.fd;
     if( fd < 0 )
       sno_exception(__LINE__);

     Socket* socket= nullptr;
     if( fd < size )
       socket= sarray[fd];
     if( socket == nullptr ) {      // If socket not assigned
       lock.unlock();               // (Debug requires shared lock)
       debug("HCDM");
       sno_exception(__LINE__);     // Internal error, not user error
     }

     if( fd != socket->handle ) {   // If socket/handle mismatch
       lock.unlock();               // (Debug requires shared lock)
       debug("HCDM");
       sno_exception(__LINE__);     // Internal error, not user error
     }

     switch( op.op ) {
       case OP_INSERT: {
         Trace::trace(".SEL", ".INS", i2v(fd), socket);
         struct pollfd* pollfd= this->pollfd + used;
         pollfd->fd= fd;
         pollfd->events= op.events;
         pollfd->revents= 0;
         sindex[fd]= used;
         sarray[fd]= socket;
         socket->selector= this;

         ++used;
//       left= next= 0;             // Not needed, revents == 0
//       left= next= 0;             // Not needed, revents == 0
         break;
       }
       case OP_MODIFY: {
         Trace::trace(".SEL", ".MOD", i2v(fd), socket);
         int px= sindex[fd];
         pollfd[px].events= op.events;
         pollfd[px].revents= 0;

//       left= next= 0;             // Not needed, revents == 0
         break;
       }
       case OP_REMOVE: {
         Trace::trace(".SEL", ".REM", i2v(fd), socket);
         int px= sindex[fd];      // Get the poll index
         if( USE_CHECKING && (px <= 0 || px >= size) ) { // If invalid fd
           debug("HCDM");
           sno_exception(__LINE__); // Internal error, not user error
         }

         --used;
         for(int i= px; i<used; ++i) {
           pollfd[i]= pollfd[i+1];
           sindex[pollfd[i].fd]= i;
         }

         socket->selector= nullptr;
         sarray[fd]= nullptr;
         sindex[fd]= -1;
         left= next= 0;
         break;
       }
       default:
         sno_exception(__LINE__);
     }
   }

DEBUGGING(
   debugh("Select::control %zd= read() %d:%s\n", L, errno, strerror(errno));
)

   if( L < 0 ) {
     if( !IS_BLOCK && !IS_RETRY )
       sno_handled(__LINE__);
   } else {
     sno_handled(__LINE__);
   }

   pollfd[0].revents= 0;            // (No control operations enqueued)
   do_again();
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
//       MUST NOT hold shr_latch OR xcl_latch: Conditionally invokes resize()
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::insert(                  // Insert Socket
     Socket*           socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::insert(%p,0x%.4x) fd(%d)\n", this
           , socket, events, socket->handle);

   std::lock_guard<decltype(ctl_latch)> lock(ctl_latch);

   int fd= socket->handle;
   if( fd < 0 ) {
     errno= EINVAL;
     return -1;
   }

   if( socket->selector ) {
     errorf("Select(%p)::insert(%p) already inserted(%p)\n", this
           , socket, socket->selector);
     errno= EINVAL;
     return -1;
   }

   if( fd >= size )
     resize(fd);

   if( USE_CHECKING && sarray[fd] ) {
     // Internal error: another socket's already using the file descriptor
     debugh("Select(%p)::insert(%p, 0x%.4x) Socket(%p) fd(%d)\n", this
           , socket, events, reader, reader->get_handle());
     debug("ShouldNotOccur");
     sno_exception(__LINE__);
   }

   sarray[fd]= socket;              // Partially update table
   socket->selector= this;

   control_op op= {OP_INSERT, 0, (uint16_t)events, fd};
   control(op);                     // Enqueue the INSERT operation
// Listen creates select which inserts, so we can't invoke control now.
// control();                       // Process the INSERT operation

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
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::modify(                  // Modify Socket
     const Socket*     socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::modify(%p,0x%.4x)\n", this, socket, events);

   if( socket->selector != this ) { // If owned by a different selector
     sno_handled(__LINE__);
     return socket->selector->modify(socket, events);
   }

   int fd= socket->handle;
   control_op op= {OP_MODIFY, 0, (uint16_t)events, fd};
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
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::remove(                  // Remove Socket
     Socket*           socket)      // The associated Socket
{  if( HCDM )
     debugh("Select(%p)::remove(%p) fd(%d)\n", this, socket, socket->handle);

   std::lock_guard<decltype(ctl_latch)> lock(ctl_latch);

   if( socket->selector == nullptr ) { // If Socket isn't owned by a selector
     if( socket->handle < 0 ) {     // (Socket::close may have been blocked)
       errno= EINVAL;
       return -1;
     }
     if( IOEM ) {
       errorf("%4d %s remove Socket(%p) selector(nullptr) fd(%d)\n"
             , __LINE__, __FILE__, socket, socket->handle);
       debug("Additional debugging information");
       debug_backtrace();
     }
     errno= EINVAL;
     return -1;
   }

   if( socket->selector != this ) { // If owned by a different selector
     sno_handled(__LINE__);
     return socket->selector->remove(socket);
   }

   control_op op= {OP_REMOVE, 0, 0, socket->get_handle()};
   control(op);                     // Enqueue the REMOVE operation
   control();                       // Process the REMOVE operation

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

   if( left > 0 )                   // Handle incomplete selection
     return select();

   {{{{
     std::unique_lock<decltype(shr_latch)> lock(shr_latch);

     for(int px= 0; px<used; ++px)
       pollfd[px].revents= 0;

     left= poll(pollfd, used, timeout);
   }}}}
   Trace::trace(".SEL", "POLL", i2v(left));
DEBUGGING(
   debugh("%4d Select left(%d)= poll(%d)\n", __LINE__, left.load(), used);
)
   if( left )                       // Handle initial selection
     return select();

   return do_again();
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

   if( left )                       // Handle incomplete selection
     return select();

   {{{{
     std::unique_lock<decltype(shr_latch)> lock(shr_latch);

     for(int px= 0; px<used; ++px)
       pollfd[px].revents= 0;

     left= ppoll(pollfd, used, timeout, signals);
   }}}}
   Trace::trace(".SEL", "POLL", i2v(left));
DEBUGGING(
   debugh("%4d Select left(%d)= poll(%d)\n", __LINE__, left.load(), used);
)
   if( left )                       // Handle initial selection
     return select();

   return do_again();
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
{
   if( pollfd[0].revents ) {        // Handle control operations
     control();
     if( --left <= 0 )
       return do_again();
   }

   Trace::trace(".SEL", ".SEL", i2v(left));
   std::lock_guard<decltype(shr_latch)> lock(shr_latch);

   /**************************************************************************
   There are two mechanisms for handling polling events. The choice is
   determined by the USE_SELECT_FUNCTION definition.

   With USE_SELECT_FUNCTION == true:
     All events are driven from this method with the shr_latch held.
     This method always returns nullptr.

   With USE_SELECT_FUNCTION == false:
     Sockets are returned one by one to the polling driver, and it can drive
     event handlers with no lock held.
     This method only returns nullptr when all events have been handled.

   It hasn't been determined which method is better.
   We may need two select functions to allow the caller to select the
   mechanism rather than semi-hard coding the choice here.
   **************************************************************************/
   if( USE_SELECT_FUNCTION ) {      // Test mechanism
     for(int px= 1; px<used; ++px) {
       struct pollfd* poll= pollfd + px;
       if( poll->revents != 0 ) {
         --left;
         Socket* socket= sarray[poll->fd];
         socket->do_select(poll->revents);
       }
     }

#if 0 // TODO: DEBUG THIS.
     if( left > 0 ) {
DEBUGGING( debugh("left(%d)\n", left); )
       sno_handled(__LINE__);
       left= 0;
     }
#else
     left= 0;
#endif

     return do_again();
   } else {                         // Original: Use select socket
     // Handle ready events
     for(int px= next; px<used; ++px) {
       if( pollfd[px].revents != 0 ) {
         --left;
         next= px + 1;
         int fd= pollfd[px].fd;
         return sarray[fd];
       }
     }

     for(int px= 1; px<next; ++px) {
       if( pollfd[px].revents != 0 ) {
         --left;
         next= px + 1;
         int fd= pollfd[px].fd;
         return sarray[fd];
       }
     }

     // ERROR: The number of elements set < number of elements found
     // ** THIS IS AN INTERNAL LOGIC ERROR, NOT AN APPLICATION ERROR **
     errorf("%4d Select internal error, info(%d)\n", __LINE__, left.load());
     sno_handled(__LINE__);
     left= 0;
     return do_again();
   }
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
//       Caller MUST NOT hold shr_latch or xcl_latch.
//       Uses 20 bytes per fd.
//
//----------------------------------------------------------------------------
inline void
   Select::resize(                  // Resize the Select tables
     int               fd)          // For this file (positive) descriptor
{  if( HCDM )
     debugf("Select(%p)::resize(%d)\n", this, fd);

   std::lock_guard<decltype(xcl_latch)> lock(xcl_latch);

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
   Socket** new_sarray= (Socket**)realloc(sarray, new_size * sizeof(Socket*));
   int* new_sindex= (int*)realloc(sindex, new_size * sizeof(int));
   if( new_pollfd == nullptr||new_sarray == nullptr||new_sindex == nullptr ) {
     free(new_pollfd);
     free(new_sarray);
     free(new_sindex);
     throw std::bad_alloc();
   }

   int diff= new_size - size;
   memset(new_pollfd + size, 0x00, diff * sizeof(pollfd));
   memset(new_sarray + size, 0x00, diff * sizeof(Socket*));
   memset(new_sindex + size, 0xff, diff * sizeof(int));

   pollfd= new_pollfd;
   sarray= new_sarray;
   sindex= new_sindex;
   size= new_size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Select::tickle
//
// Purpose-
//       Drive select completion
//
//----------------------------------------------------------------------------
void
   Select::tickle( void )           // Drive select completion
{  if( HCDM )
     debugh("Select(%p)::tickle\n", this);

   std::lock_guard<decltype(ctl_latch)> lock(ctl_latch);

   control_op op= {OP_TICKLE, 0, 0, 0};
   control(op);                     // Enqueue the TICKLE operation
   control();                       // Process the TICKLE operation
}
} // namespace _LIBPUB_NAMESPACE
