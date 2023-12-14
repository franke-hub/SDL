//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Stream.cpp
//
// Purpose-
//       Test the Stream objects.
//
// Last change date-
//       2023/12/13
//
// Arguments-
//       With no arguments, --client defaulted
//       --bringup  Bringup test (Display object sizes)
//       --client   Basic test
//       --stress   Stress test
//
//       --server   Run server using this host and default port (default)
//         =:port   Run server using this host and specified port
//         =host    Use server at specified host and default port
//         =host:port Use server at specified host and port
//
// Stress test controls-
//       --major=1  One connection/operation stress test
//       --major=2  One connection/operation short test
//       --minor=1  With --major > 0, wait for client completion
//
//----------------------------------------------------------------------------

#include <atomic>                   // For std::atomic
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For mutex, std::lock_guard
#include <cstddef>                  // For offsetof
#include <cstdint>                  // For UINT16_MAX
#include <ctime>                    // For time, ...
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt_long()
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/signal.h>             // For signal, ...

#include <pub/TEST.H>               // For VERIFY macro
#include <pub/Clock.h>              // For pub::Clock
#include <pub/Debug.h>              // For debugging classes and functions
#include <pub/diag-shared_ptr.h>    // For std::pub_diag::Debug_ptr::debug()
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Event.h>              // For pub::Event
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Reporter.h>           // For pub::Reporter
#include <pub/Signals.h>            // For namespace pub::signals
#include <pub/Statistic.h>          // For pub::Statistic
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string, visify
#include <pub/Worker.h>             // For pub::WorkerPool
#include <pub/Wrapper.h>            // For pub::Wrapper::atol, ...

#include "pub/http/Agent.h"         // For pub::http::ClientAgent, ListenAgent
#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub classes
using namespace PUB::debugging;     // For pub debugging functions
using namespace PUB::http;          // For pub::http classes
using namespace PUB::signals;       // For signal handling
using PUB::utility::visify;         // (Import)
using namespace std;                // For std classes

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH) // Directory mode
,  MAX_REQUEST_COUNT= 4             // Maximum running request count
,  MAX_RESPONSE_SIZE= 0x00100000    // Maximum response data length
,  PROT_RW= (PROT_READ | PROT_WRITE) // Read/write access mode
,  TRACE_SIZE= 0x00100000           // Default trace table size (1M)
,  USE_INTENSIVE= true              // Option: Use intensive debug mode
,  USE_ITRACE= true                 // Use internal trace?
,  USE_LOGGER= false                // Option: Use logger
,  USE_REPORT= false                // Option: Use event Reporter
,  USE_REPORT_ITERATION= 0          // Option: Event Reporter iteration count
,  USE_SIGNAL= true                 // Option: Use signal handler
}; // generic enum

enum                                // Default option values
{  DEFAULT_OPTIONS                  // (Dummy)
,  OPT_THREAD= 4                    // Stress test default thread count

,  USE_CLIENT= false                // --client
,  USE_STRESS= 0                    // --stress
,  USE_TRACE=  0                    // --trace
,  USE_VERIFY= false                // --verify
,  USE_WORKER= true                 // --worker (Server threads)
}; // default options

static constexpr const double USE_RUNTIME= 2.0; // --runtime

// Imported Options
typedef const char CC;
static constexpr CC*   HTTP_GET=  Options::HTTP_METHOD_GET;
static constexpr CC*   HTTP_HEAD= Options::HTTP_METHOD_HEAD;
static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;

static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;
static constexpr CC*   HTTP_TYPE= Options::HTTP_HEADER_TYPE;

static constexpr CC*   cert_file= "public.pem";  // The public certificate file
static constexpr CC*   priv_file= "private.pem"; // The private key file

//----------------------------------------------------------------------------
// Internal data area types
//----------------------------------------------------------------------------
struct SIG {                        // The signal event
int                    id;          // The interrupt ID
   SIG(int id) : id(id) {}          // Constructor
}; // SIG

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // Our Debug object
static string          host= Socket::gethostname(); // The connection host
static string          port= ":8080"; // The connection port
static string          test_url= "/"; // The stress test URL
static void*           trace_table= nullptr; // The internal trace area

static ClientAgent*    client_agent= nullptr; // Our ClientAgent
static ListenAgent*    listen_agent= nullptr; // Our ListenAgent

// Interrupt handler
Signal<SIG>            interruptSignal;
Connector<SIG>         interruptConnector=
   interruptSignal.connect([](SIG sig) {
     if( opt_verbose )
       debugf("System signal(%d)\n", sig.id);
     std::pub_diag::Debug_ptr::debug("Signal");
   });

// Test controls
typedef std::atomic_size_t          atomic_count_t;
static atomic_count_t  error_count= 0; // Error counter
static atomic_count_t  send_op_count= 0; // The total send complete count
static Event           test_ended;  // The test ended Event
static Event           test_start;  // The start test Event
static int             running= false; // Test running indicator

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
extern int             opt_hcdm;    // (Defined by Wrapper.h)
static int             opt_iodm= IODM;  // --iodm (I/O Debug Mode)
static int             opt_index;   // Option index

static const char*     opt_debug= nullptr; // --debug
static int             opt_bringup= false; // Run bringup test?
static int             opt_client= USE_CLIENT; // Run basic client test?
static int             opt_major= 0; // Major test id TODO: REMOVE
static int             opt_minor= 0; // Minor test id TODO: REMOVE
static double          opt_runtime= USE_RUNTIME; // Stress test run time, in seconds
static int             opt_ssl= false;  // Run SSL client/server?
static int             opt_stress= USE_STRESS; // Run client stress test?
static size_t          opt_trace= USE_TRACE; // Create trace file?
extern int             opt_verbose; // (Defined by Wrapper.h)
static int             opt_verify= USE_VERIFY; // Verify file data?
static int             opt_worker= USE_WORKER; // Create server threads?
static int             use_remote_server= false;

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm
,  {"iodm",    no_argument,       &opt_iodm,    true} // --iodm

,  {"debug",   required_argument, nullptr,      0} // --debug <string>
,  {"verbose", optional_argument, &opt_verbose, 1} // --verbose {optional}
,  {"bringup", no_argument,       &opt_bringup, true} // --bringup
,  {"client",  no_argument,       &opt_client,  true} // --client
,  {"major",   optional_argument, &opt_major,   1}    // --major
,  {"minor",   optional_argument, &opt_minor,   1}    // --minor
,  {"runtime", required_argument, nullptr,      0}    // --runtime <string>
,  {"server",  optional_argument, nullptr,      0}    // --server
,  {"ssl",     no_argument,       &opt_ssl,  true}    // --stress
,  {"stress",  optional_argument, &opt_stress,  OPT_THREAD} // --stress
,  {"trace",   optional_argument, nullptr,      0}    // --trace
,  {"verify",  no_argument,       &opt_verify,  true} // --verify
,  {"worker",  no_argument,       &opt_worker,  true} // --worker

// This option can be used if USE_WORKER defaulted true
,  {"no-worker", no_argument,     &opt_worker,  false} // --no-worker
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_IODM

,  OPT_DEBUG
,  OPT_VERBOSE
,  OPT_BRINGUP
,  OPT_CLIENT
,  OPT_MAJOR
,  OPT_MINOR
,  OPT_RUNTIME
,  OPT_SERVER
,  OPT_SSL
,  OPT_STRESS
,  OPT_TRACE
,  OPT_VERIFY
,  OPT_WORKER

,  OPT_NO_WORKER
,  OPT_SIZE
};

//----------------------------------------------------------------------------
// T_Stream.hpp: Definition and implementation of test classes
//----------------------------------------------------------------------------
#include "T_Stream.hpp"             // Prerequisite includes, etc

//----------------------------------------------------------------------------
// Global constructor/destructor (For hard core debugging)
//----------------------------------------------------------------------------
static struct Global {
   Global( void )
{
   if( HCDM )
     printf("%4d %s Global!\n", __LINE__, __FILE__);
}

   ~Global( void )
{
   if( HCDM )
     printf("%4d %s Global~\n", __LINE__, __FILE__);
}
} global_constructor_destructor;

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void term( void );           // Terminate

//----------------------------------------------------------------------------
// Signal handlers
//----------------------------------------------------------------------------
typedef void (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
//
// Subroutine-
//       major_name
//       minor_name
//
// Purpose-
//       Give test controls a readable format
//
//----------------------------------------------------------------------------
static const char*                  // The test descriptor name
   major_name( void )               // Get test descriptor name
{
   if( opt_major <= 0 )
     return "";

   if( opt_major > 1 )
     return ": One connection/operation, short test";

   return ": One connection/operation, stress test";
}

static const char*                  // The test modifier name
   minor_name( void )               // Get test modifier name
{
   if( opt_major <= 0 || opt_minor <= 0 )
     return "";

   return ": Wait after Client close";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n"
              , OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n"
              , OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options> parameter ...\n", __FILE__ );
   fprintf(stderr, "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"
                   "  --iodm\tI/O Debug Mode\n"

                   "  --debug\targument\n"
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   "  --bringup\tRun bringup test\n"
                   "  --client\tRun client basic test\n"
                   "  --stress\t{=n} Run client stress test\n"
                   "  --runtime\tSet test run time (seconds)\n"
                   "  --server\t{=host{:port}|=:port} Specify server\n"
                   "  --ssl\tUse SSL sockets\n"
                   "  --trace\tActivate internal trace\n"
                   "  --server\tRun server\n"
                   "  --verify\tVerify file data\n"
                   "  --worker\tUse server threads\n"
          );

   exit(1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
static void
   sig_handler(                     // Handle signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     fprintf(stderr, "sig_handler(%d) recursion\n", id);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* text= "SIG????";
   if( id == SIGINT ) text= "SIGINT";
   else if( id == SIGSEGV ) text= "SIGSEGV";
   else if( id == SIGUSR1 ) text= "SIGUSR1";
   else if( id == SIGUSR2 ) text= "SIGUSR2";
   errorf("\n\nsig_handler(%d) %s\n\n", id, text);

   switch(id) {                     // Handle the signal
     case SIGSEGV:                  // (Program fault)
       Trace::trace(".BUG", __LINE__, "SIGSEGV");
       Trace::stop();

       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();           // Attempt diagnosis (recursion aborts)
       debugf("..terminated..\n");
       term();
       exit(EXIT_FAILURE);
       break;

     case SIGINT:                   // (No configured action)
     case SIGUSR1:                  // (No configured action)
     case SIGUSR2:                  // (No configured action)
     default:
       Trace::trace(".SIG", __LINE__, text);

       SIG sig(id);
       interruptSignal.signal(sig);
       break;
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Return "true" or "false"
//
//----------------------------------------------------------------------------
static const char*                  // "true" or "false"
   torf(                            // True or False?
     int               cc)          // For this condition
{  return cc ? "true" : "false"; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 expected
   init( void)                      // Initialize
{
   if( HCDM )
     opt_hcdm= true;

   if( opt_hcdm && opt_verbose < 1 )
     opt_verbose= 1;

   if( USE_SIGNAL ) {
//   sys1_handler= signal(SIGINT,  sig_handler);
     sys2_handler= signal(SIGSEGV, sig_handler);
     usr1_handler= signal(SIGUSR1, sig_handler);
     usr2_handler= signal(SIGUSR2, sig_handler);
   }

   Debug* extant= Debug::show();
   debug= new Debug("debug.out");   // Create Debug object
   if( extant )
     debug->set_file_mode("ab");
   Debug::set(debug);
   if( opt_hcdm || USE_INTENSIVE ) { // If Hard Core INTENSIVE Debug Mode
     debug_set_mode(Debug::MODE_INTENSIVE);
     debugh("HCDM: MODE_INTENSIVE\n");
   }
   debug_set_head(Debug::HEAD_THREAD);

   if( opt_trace ) {                // If --trace specified
     //-----------------------------------------------------------------------
     // Create memory-mapped trace file
     string S= "./trace.mem";
     int fd= open(S.c_str(), O_RDWR | O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
     if( fd < 0 ) {
       fprintf(stderr, "%4d open(%s) %s\n", __LINE__
                     , S.c_str(), strerror(errno));
       return 1;
     }

     int rc= ftruncate(fd, opt_trace); // (Expand to opt_trace size)
     if( rc ) {
       fprintf(stderr, "%4d ftruncate(%s,%.8zx) %s\n", __LINE__
                     , S.c_str(), opt_trace, strerror(errno));
       return 1;
     }

     trace_table= mmap(nullptr, opt_trace, PROT_RW, MAP_SHARED, fd, 0);
     if( trace_table == MAP_FAILED ) { // If no can do
       fprintf(stderr, "%4d mmap(%s,%.8zx) %s\n", __LINE__
                     , S.c_str(), opt_trace, strerror(errno));
       trace_table= nullptr;
       return 1;
     }

     Trace::table= PUB::Trace::make(trace_table, opt_trace);
     close(fd);                     // Descriptor not needed once mapped

     Trace::trace(".INI", 0, "TRACE STARTED") ;
   }

   client_agent= new ClientAgent();
   listen_agent= new ListenAgent();

   setlocale(LC_NUMERIC, "");       // For printf("%'d\n", 123456789);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   // Remove client/server agent
   delete client_agent;
   delete listen_agent;
   client_agent= nullptr;
   listen_agent= nullptr;

   //-------------------------------------------------------------------------
   // Restore system signal handlers
   if( sys1_handler ) signal(SIGINT,  sys1_handler);
   if( sys2_handler ) signal(SIGSEGV, sys2_handler);
   if( usr1_handler ) signal(SIGUSR1, usr1_handler);
   if( usr2_handler ) signal(SIGUSR2, usr2_handler);
   sys1_handler= sys2_handler= usr1_handler= usr2_handler= nullptr;

   // Release the trace table (which disables tracing)
   Trace::trace(".XIT", 0, "TRACE STOPPED") ;
   if( trace_table ) {
     Trace::table= nullptr;
     munmap(trace_table, opt_trace);
     trace_table= nullptr;
   }

   // Terminate debugging
#if 0  // TODO: REMOVE (when thread/worker tracing removed)
   opt_hcdm= false;
   Debug::set(nullptr);
   delete debug;
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_bringup
//
// Purpose-
//       Bringup test
//
//----------------------------------------------------------------------------
static void
   size_of(                         // Display size of object
     const char*       name,        // Object name
     size_t            size)        // Object size
{  debugf("0x%.4zx = sizeof(%s)\n", size, name); }

static inline int                   // Error count
   test_bringup( void )             // Bringup test
{
   typedef PUB::http::Client        Client;
   typedef PUB::http::Server        Server;

   debugf("\ntest_bringup\n");
   int error_count= 0;

   size_of("Client",        sizeof(PUB::http::Client));
   size_of("ClientAgent",   sizeof(PUB::http::ClientAgent));
   size_of("ClientThread",  sizeof(ClientThread));
   size_of("Listen",        sizeof(PUB::http::Listen));
   size_of("ListenAgent",   sizeof(PUB::http::ListenAgent));
   size_of("Options",       sizeof(PUB::http::Options));
   size_of("Request",       sizeof(PUB::http::Request));
   size_of("Response",      sizeof(PUB::http::Response));
   size_of("Server",        sizeof(PUB::http::Server));
   size_of("ServerThread",  sizeof(ServerThread));
   size_of("Stream",        sizeof(PUB::http::Stream));

#pragma GCC diagnostic ignored "-Winvalid-offsetof"
   debugf("\n");
   debugf("0x%.4zx = offsetof(Client, task_inp)\n", offsetof(Client, task_inp));
   debugf("0x%.4zx = offsetof(Client, task_out)\n", offsetof(Client, task_out));
   debugf("0x%.4zx = offsetof(Server, task_inp)\n", offsetof(Server, task_inp));
   debugf("0x%.4zx = offsetof(Server, task_out)\n", offsetof(Server, task_out));

   if( false ) {                    // Bringup internal tests
     debugf("\npage200(\"BODY\")\n%s", page200("BODY").c_str());
     debugf("\npage403(\"/FILE\")\n%s", page403("/FILE").c_str());
     debugf("\npage404(\"/FILE\")\n%s", page404("/FILE").c_str());
     debugf("\npage405(\"METH\")\n%s", page405("METH").c_str());
     debugf("\npage500(\"OOPS\")\n%s", page500("OOPS").c_str());
   }
   debugf("\n");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void                         // Exit if error detected
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 ) {
     switch( C ) {
       case 0:
       {{{{
         switch( opt_index ) {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
           case OPT_IODM:
           case OPT_BRINGUP:
           case OPT_CLIENT:
           case OPT_VERIFY:
           case OPT_WORKER:

           case OPT_NO_WORKER:
             break;

           case OPT_DEBUG:
             opt_debug= optarg;
             break;

           case OPT_MAJOR:
             if( optarg )
               opt_major= parm_int();
             break;

           case OPT_MINOR:
             if( optarg )
               opt_minor= parm_int();
             break;

           case OPT_RUNTIME:
             opt_runtime= atof(optarg);
             break;

           case OPT_SERVER:
             if( optarg ) {
               const char* C= strchr(optarg, ':'); // Find port delimiter
               if( C ) {            // If port delimiter specified
                 if( C != optarg ) {  // If host also specified
                   use_remote_server= true;
                   host= string(optarg, (C-optarg)); // Specify host
                 }
                 port= C;           // Specify port
               } else {
                 use_remote_server= true;
                 host= optarg;
               }
             }
             break;

           case OPT_STRESS:
             if( optarg ) {
               opt_stress= parm_int();
               if( opt_stress < 0 )
                 opt_stress= 0;
             }
             break;

           case OPT_TRACE:
             opt_trace= TRACE_SIZE;
             if( optarg )
               opt_trace= Wrapper::atol(optarg);
             if( opt_trace < Trace::TABLE_SIZE_MIN )
               opt_trace= Trace::TABLE_SIZE_MIN;
             else if( opt_trace > Trace::TABLE_SIZE_MAX )
               opt_trace= Trace::TABLE_SIZE_MAX;
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option has no argument '%s'.\n"
                         , __LINE__, argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '%s'.\n"
                         , __LINE__, argv[optind-1]);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
         break;
     }
   }

   // Parameter verification
   if( !opt_bringup && !opt_client && !opt_stress && use_remote_server )
     opt_client= true;

   for(int i= optind; i < argc; ++i) {
     debugf("Unexpected parameter: %s\n", argv[i]);
     opt_help= true;
   }

   // If modified stress test, opt_stress should be specified
   if( opt_major > 0 && opt_stress == 0 )
     opt_stress= 2;

   // If short test, opt_runtime is not relevant
   if( opt_major > 1 )
     opt_runtime= 0;

   // Return sequence
   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Argument analysis (Exit if error)
   init();                          // Initialize

   if( opt_verbose ) {
     debugf("%s %s %s (Compiled)\n", __FILE__, __DATE__, __TIME__);

     time_t tod;
     time(&tod);
     struct tm* info= localtime(&tod);
     char buff[32];
     strftime(buff, sizeof(buff), "%b %e %Y %R:%S", info);
     debugf("%s %s (Started)\n", __FILE__, buff);

     debugf("\n");
     debugf("Settings:\n");
     debugf("%5.1f: runtime\n",  opt_runtime);
     debugf("%5s: server: %s%s\n", use_remote_server ? "using" : "local"
           , host.c_str(), port.c_str());
     debugf("%5s: hcdm\n",   torf(opt_hcdm));
     debugf("%5s: iodm\n",   torf(opt_iodm));
     debugf("%5d: verbose\n",opt_verbose);

     debugf("%5s: client\n", torf(opt_client));
     debugf("%5s: ssl\n",    torf(opt_ssl));
     if( opt_stress )
       debugf("%5s: stress=%d\n", torf(opt_stress), opt_stress);
     else
       debugf("%5s: stress\n", torf(opt_stress));
     debugf("%5s: trace 0x%.8zx\n", torf(opt_trace), opt_trace);
     debugf("%5s: worker\n", torf(opt_worker));

     // Debugging, experimentation
     debugf("\n");
     debugf("%5d: MAX_REQUEST_COUNT\n", MAX_REQUEST_COUNT);
     debugf("%5s: Protocol (unencrypted)\n", "HTTP1");
     debugf("%5d: --major%s\n", opt_major, major_name());
     debugf("%5d: --minor%s\n", opt_minor, minor_name());
     debugf("\n\n");
   }

   //-------------------------------------------------------------------------
   // Run the tests (with try wrapper)
   //-------------------------------------------------------------------------
   try {
     if( opt_bringup )
       error_count += test_bringup();

     ServerThread* server= nullptr;
     if( !use_remote_server ) {
       server= new ServerThread();
       server->ready.wait();
     }

     if( opt_client || opt_stress ) {
       if( opt_client ) {
         error_count= 0;
         ClientThread::test_client();
         ClientThread::statistics();
       }
       if( opt_stress ) {
         error_count= 0;
         ClientThread::test_stress();
         ClientThread::statistics();
       }
     } else if( server ) {
       TimerThread timer_thread;
       timer_thread.start();
       timer_thread.join();
     }

     if( server ) {
       server->stop();
       server->ended.wait();
       delete server;
     }
   } catch(PUB::Exception& X) { // - - - - - - - - - - - - - - - - - - - - - -
     ++error_count;
     debugf("%4d T_Stream: %s\n", __LINE__, ((string)X).c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d T_Stream: std::exception(%s)\n", __LINE__, X.what());
   } catch(const char* X) {
     ++error_count;
     debugf("%4d T_Stream: catch(\"%s\")\n", __LINE__, X);
   } catch(...) {
     ++error_count;
     debugf("%4d T_Stream: catch(...)\n", __LINE__);
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   Thread::sleep(0.5);              // (Delay to allow cleanup)
   debugf("\n");
   if( error_count == 0 )
     debugf("NO errors detected\n");
   else if( error_count == 1 )
     debugf("1 error detected\n");
   else {
     debugf("%zd errors detected\n", error_count.load());
     error_count= 1;
   }

   term();                          // Terminate
   return error_count.load() != 0;
}
