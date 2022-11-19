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
//       Select.h method implementations.
//
// Last change date-
//       2022/11/18
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
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/List.h>               // For pub::AI_list<>
#include "pub/Select.h"             // For pub::Select, implemented
#include "pub/Socket.h"             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_DEBUGGING false         // Use DEBUGGING macro?
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode?
,  IOEM= true                       // I/O error Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  USE_AF= AF_INET                  // Use this address family
,  USE_CHECKING= true               // Use internal cross-checking?
,  USE_SELECT_FUNCTION= false       // Use (test) socket->selected method?
}; // enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if USE_DEBUGGING                   // Bringup debugging TODO: REMOVE
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
//       Control operation descriptor
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
const Socket*          socket;      // The associated Socket
}; // struct control_op

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
   fdpndx[fd]= 0;
   fdsock[fd]= reader;
   reader->select= this;
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

//- - - - - - - - - - - - - - - - - -- - - - - - - - - - - - - - - - - - - - -
   Select::~Select( void )
{  if( HCDM )
     debugf("Select(%p)::~Select\n", this);

   // Complete any pending operations. Hopefully they're close ops.
   control();

   // Manually remove our reader socket from our tables
   if( reader ) {                   // If we have a reader socket
     int fd= reader->get_handle();  // Get the poll index
     int px= fdpndx[fd];
     for(int i= px; i<(used-1); ++i)
       pollfd[i]= pollfd[i+1];

     fdpndx[fd]= -1;
     fdsock[fd]= nullptr;
     reader->select= nullptr;
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
       Socket* socket= fdsock[fd];
       if( socket ) {
         #define FMT "%4d Select(%p) Socket(%p) fd(%d) User error: " \
                     "Dangling reference\n"
         errorf(FMT, __LINE__, this, socket, fd);
         sno_handled(__LINE__);     // See ** USER DEBUGGING NOTE **, above
         debug("Additional debugging information");
         socket->select= nullptr;
       } else if( USE_CHECKING ) {
         sno_handled(__LINE__);     // (socket[fd] == nullptr)
       }
     } else if( USE_CHECKING ) {
       sno_handled(__LINE__);       // (pollfd[px].fd >= size)
     }
   }

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
//       Socket close operations, which queue a remove operation, may complete
//       before the remove operation is processed leaving the Socket* invalid.
//       Accessing a Socket* can result in a SEGFAULT, so we don't do it.
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
   debugf("..pollfd(%p) fdpndx(%p) fdsock(%p)\n", pollfd, fdpndx, fdsock);
   debugf("..left(%u) next(%u) size(%u) used(%u)\n"
         , left.load(), next, size, used);
   debugf("..pollfd %d\n", used);
   for(int px= 0; px<used; ++px) {
     int fd= pollfd[px].fd;
     const Socket* socket= fdsock[fd];
     debugf("....[%3d] fd[%3d] pollfd{%.4x,%.4x} socket(%p)\n", px, fd
           , pollfd[px].events, pollfd[px].revents, socket);
     if( socket == nullptr ) {
       ++error_count;
       debugf("....[%3d] ERROR: NO ASSOCIATED SOCKET\n", px);
     }
     if( px != fdpndx[fd] ) {
       ++error_count;
       debugf("....[%3d] ERROR: fdpndx[%3d] fd[%3d]\n", px, fd
              , fdpndx[fd]);
     }
   }

   debugf("..fdpndx\n");
   for(int fd= 0; fd<size; ++fd) {
     if( fdpndx[fd] >= 0 )
       debugf("....[%3d] -> [%3d]\n", fd, fdpndx[fd]);
   }

   debugf("..fdsock\n");
   for(int sx= 0; sx<size; ++sx) {
     Socket* socket= fdsock[sx];
     if( socket )
       debugf("....[%3d] %p\n", sx, socket);
   }

   SelectItem* item= (SelectItem*)todo_list.get_tail();
   debugf("..dolist(%p) [tail..head]\n", item);
   while( item ) {
     Item* next= item->get_prev();
     control_op& op= item->op;
     debugf("....%p->%p {%c,%.4x,%2d,%p}\n", item, next
           , op.op, op.events, op.fd, op.socket);
     item= (SelectItem*)next;
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
//----------------------------------------------------------------------------
void
   Select::control(                 // Transmit control operation
     const control_op& op)          // The operation to send
{  if( HCDM )
     debugh("Select(%p)::control({%c,,0x%.4x,%d,%p})\n", this
           , op.op, op.events, op.fd, op.socket);
   Trace::Record* R= Trace::trace(".SEL", ">CTL", op.socket);
   if( R )
     memcpy(R->value + sizeof(op.socket), &op, sizeof(op.socket));

   SelectItem* item= new SelectItem();
   item->op= op;
   Item* tail= todo_list.fifo(item);

   if( tail == nullptr ) {
     ssize_t L= writer->write(&op.op, 1);
     while( L < 0 && IS_RETRY )
       L= writer->write(&op.op, 1);
     if( L < 0 && !IS_BLOCK ) {
       debugf("Select(%p)::control write error: %d:%s\n", this
             , errno, strerror(errno));
       sno_exception(__LINE__);
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
//       Callers MUST NOT hold shr_latch OR xcl_latch: Obtains xcl_latch
//       (It cannot be called from Socket::do_select)
//
//----------------------------------------------------------------------------
void
   Select::control( void )          // Read and handle queued operations
{  if( HCDM )
     debugh("Select(%p)::control\n", this);

   std::unique_lock<decltype(xcl_latch)> lock(xcl_latch);
// Trace::trace(".SEL", "=XCL", this); // TODO: REMOVE

   char buffer[8];                  // (Only one byte should be used)
   ssize_t L= reader->read(buffer, sizeof(buffer));
   while( L < 0 && IS_RETRY )
     L= reader->read(buffer, sizeof(buffer));

   if( L < 0 && !IS_BLOCK ) {
     debugf("Select(%p)::control read error: %d:%s\n", this
           , errno, strerror(errno));
     sno_exception(__LINE__);
   }
   pollfd[0].revents= 0;            // (No control operations enqueued)

   // Process queued operations
   for(auto it= todo_list.begin(); it != todo_list.end(); ++it) {
     SelectItem* item= static_cast<SelectItem*>(it.get());
     control_op& op= item->op;

     const Socket* socket= op.socket;
     int fd= op.fd;
     if( fd < 0 )
       sno_exception(__LINE__);

     if( op.op == OP_MODIFY || op.op == OP_REMOVE ) {
       if( fd >= size || fdsock[fd] != socket ) {
         lock.unlock();             // (Debug requires shared lock)
         debug("HCDM");
         sno_exception(__LINE__);   // Internal error, not user error
       }
     }

     switch( op.op ) {
       case OP_INSERT: {
         Trace::trace(".SEL", "=INS", socket, i2v(fd));

         if( fd >= size )
           resize(fd);

         if( USE_CHECKING && fdsock[fd] ) {
           // Internal error: another socket's already using the file descriptor
           debugh("Select(%p)::insert(%p) fdsock[%d](%p)\n", this
                 , socket, fd, fdsock[fd]);
           lock.unlock();           // (Debug requires shared lock)
           debug("HCDM");
           sno_exception(__LINE__);
         }

         // Perform the insert
         struct pollfd* pollfd= this->pollfd + used;
         pollfd->fd= fd;
         pollfd->events= op.events;
         pollfd->revents= 0;
         fdpndx[fd]= used;
         fdsock[fd]= const_cast<Socket*>(socket);

         ++used;
//       left= next= 0;             // Not needed, revents == 0
//       left= next= 0;             // Not needed, revents == 0
         break;
       }
       case OP_MODIFY: {
         Trace::trace(".SEL", "=MOD", socket, i2v(fd));
         int px= fdpndx[fd];
         pollfd[px].events= op.events;
         pollfd[px].revents= 0;

//       left= next= 0;             // Not needed, revents == 0
         break;
       }
       case OP_REMOVE: {
         Trace::trace(".SEL", "=REM", socket, i2v(fd));
         int px= fdpndx[fd];        // Get the poll index
         if( USE_CHECKING && (px <= 0 || px >= size) ) { // If invalid fd
           debug("HCDM");
           sno_exception(__LINE__); // Internal error, not user error
         }

         --used;
         for(int i= px; i<used; ++i) {
           pollfd[i]= pollfd[i+1];
           fdpndx[pollfd[i].fd]= i;
         }

         fdsock[fd]= nullptr;
         fdpndx[fd]= -1;
         left= next= 0;
         break;
       }
       case OP_TICKLE: {
         Trace::trace(".SEL", "=NOP");
         break;
       }
       default:
         sno_exception(__LINE__);
     }

     item->post();                  // (Uses dispatch post logic)
   }

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
//       Operation does not complete until next polling operation
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Select::insert(                  // Insert Socket
     Socket*           socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::insert(%p,0x%.4x) fd(%d)\n", this
           , socket, events, socket->handle);

   int fd= socket->handle;
   if( fd < 0 ) {
     errno= EINVAL;
     return -1;
   }

   Select* old_value= nullptr;
   for(;;) {
     if( socket->select.compare_exchange_weak(old_value, this) )
       break;
     if( old_value != nullptr ) {
       errorf("Select(%p)::insert(%p) but Select(%p) already inserted\n", this
             , socket, old_value);
       errno= EINVAL;
       return -1;
     }
   }

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
     const Socket*     socket,      // The associated Socket
     int               events)      // The associated poll events
{  if( HCDM )
     debugh("Select(%p)::modify(%p,0x%.4x)\n", this, socket, events);

   if( socket->select != this ) {   // If Socket/Select mismatch
     errno= EINVAL;
     return -1;
   }

   int fd= socket->get_handle();
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
     debugh("Select(%p)::remove(%p) fd(%d)\n", this, socket, socket->handle);

   Select* old_value= this;
   Select* new_value= nullptr;
   for(;;) {
     if( socket->select.compare_exchange_weak(old_value, new_value) )
       break;
     if( old_value != this ) {      // If Socket/Select mismatch
       errno= EINVAL;               // (Possibly a duplicate close)
       return -1;
     }
   }

   int fd= socket->get_handle();
   if( fd < 0 || fd >= size ) {     // If Socket is closed or invalid handle
     Trace::stop();                 // Terminate tracing
     debugf("%4d %s *UNEXPECTED* %d\n", __LINE__, __FILE__, fd);
     errno= EINVAL;                 // (Unexpected)
     return -1;
   }

   // Since the operation won't complete until the next poll,
   // insure that no event is pending for any current poll
   std::lock_guard<decltype(shr_latch)> lock(shr_latch);

   // The enqueue needs to be done while holding the shr_latch so that
   // we can be sure that fdpndx[fd] refers to the removed socket.
   control_op op= {OP_REMOVE, 0, 0, fd, socket};
   control(op);                     // Enqueue the REMOVE operation

   int px= fdpndx[fd];
   if( px > 0 && px < used ) {      // (Test shouldn't be needed)
     pollfd[px].revents= 0;         // Don't report events
     pollfd[px].events= 0;          // Don't poll for new events
   } else {
     Trace::stop();                 // Terminate tracing
     debugf("%4d %s *UNEXPECTED* %d %d\n", __LINE__, __FILE__, fd, px);
   }

   Trace::trace(".SEL", "HCDM", this, i2v(__LINE__));

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
// Trace::trace(".SEL", "POLL", this, i2v(left));
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
// Trace::trace(".SEL", "POLL", this, i2v(left));
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
   if( pollfd[0].revents || todo_list.get_tail() ) {
     control();
     left= 0;
     return do_again();
   }

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
         int fd= poll->fd;
         Socket* socket= fdsock[fd];
         int revents= poll->revents;
         Trace::trace(".SEL", "=RUN", socket, i2v(intptr_t(revents)<<32 | fd));
         socket->do_select(revents);
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
       struct pollfd* poll= pollfd + px;
       int revents= poll->revents;
       if( revents != 0 ) {
         --left;
         next= px + 1;
         int fd= pollfd[px].fd;
         Trace::trace(".SEL", "=SEL", fdsock[fd]
                     , i2v(intptr_t(revents)<<32 | fd));
         return fdsock[fd];
       }
     }

     for(int px= 1; px<next; ++px) {
       struct pollfd* poll= pollfd + px;
       int revents= poll->revents;
       if( revents != 0 ) {
         --left;
         next= px + 1;
         int fd= pollfd[px].fd;
         Trace::trace(".SEL", "=SEL", fdsock[fd]
                     , i2v(intptr_t(revents)<<32 | fd));
         return fdsock[fd];
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
     free(new_pollfd);
     free(new_fdpndx);
     free(new_fdsock);
     throw std::bad_alloc();
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

//----------------------------------------------------------------------------
//
// Method-
//       Select::shutdown
//
// Purpose-
//       Insure all enqueued operation complete.
//
// Implementation notes-
//       Called after polling task is no longer operational but may or may
//       not still be running.
//
//----------------------------------------------------------------------------
void
   Select::shutdown( void )         // Insure operation completion
{  if( HCDM )
     debugh("Select(%p)::shutdown\n", this);

   control_op op= {OP_TICKLE, 0, 0, 0, nullptr};
   control(op);
   control();                       // Chase any pending operations
}
} // namespace _LIBPUB_NAMESPACE
