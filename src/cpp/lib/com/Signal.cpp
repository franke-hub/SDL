//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Signal.cpp
//
// Purpose-
//       Instantiate Signal object methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#define _XOPEN_SOURCE 700           // CYGWIN: Required for signal.h

#ifdef _OS_BSD
  #include <pthread.h>              // Must be first
#endif
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <inttypes.h>               // For PRId64, PRIx64, etc.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/List.h>

#include "com/Signal.h"

#ifdef _OS_BSD
  #include <sys/signal.h>           // Must follow com/Signal.h
#endif

#ifdef _OS_WIN
  #include <signal.h>               // Must follow com/Signal.h
  #ifndef SIGKILL
    #define SIGKILL 2
  #endif
#endif

#if defined(__GNUC__) && defined(_OS_LINUX) // For backtrace
  #include <execinfo.h>
  #include <ucontext.h>
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Class-
//       _SystemSignal
//
// Purpose-
//       The system signal handler (Signal friend)
//
//----------------------------------------------------------------------------
class _SystemSignal {               // System signal handler
public:
static void
   drive(                           // Drive the signal handlers
     int               code,        // For this signal code,
     void*             _siginfo= NULL, // This signal info, and
     void*             _context= NULL);// This context
}; // class _SystemSignal

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Protects internal data areas
static List<Signal>*   list= NULL;  // The Signal List
static int             recursion= FALSE; // Recursion indicator

#ifdef _OS_BSD
static struct sigaction
                       restore[32]; // Signal restoration array
#endif

static const char*     signalName[Signal::SC_COUNT]= // Signal name array
{   /*00*/ "00 (Invalid)"
,   /*01*/ "Hangup"
,   /*02*/ "Interrupt"
,   /*03*/ "Quit"
,   /*04*/ "Illegal Instruction"
,   /*05*/ "Trace trap"
,   /*06*/ "Process abort"
,   /*07*/ "EMT Instruction"
,   /*08*/ "Floating point exception"
,   /*09*/ "Kill"
,   /*10*/ "Bus (specification) error"
,   /*11*/ "Segment violation"
,   /*12*/ "Bad argument to system call"
,   /*13*/ "No one to read pipe"
,   /*14*/ "Alarm clock timeout"
,   /*15*/ "Software termination signal"
,   /*16*/ "Ugent I/O channel condition"
,   /*17*/ "Stop"
,   /*18*/ "Interactive stop"
,   /*19*/ "Continue"
,   /*20*/ "Child stop or exit"
,   /*21*/ "Background read from control terminal"
,   /*22*/ "Background write to control terminal"
,   /*23*/ "I/O possible, or completed"
,   /*24*/ "CPU time limit exceeded"
,   /*25*/ "File size limit exceeded"
,   /*26*/ "(Invalid)"
,   /*27*/ "Input data in HFT ring buffer"
,   /*28*/ "Window size changed"
,   /*29*/ "Power fail restart"
,   /*30*/ "User signal 1"
,   /*31*/ "User signal 2"
,   /*32*/ "32 (Invalid)"
,   /*33*/ "33 (Invalid)"
,   /*34*/ "34 (Invalid)"
,   /*35*/ "35 (Invalid)"
,   /*36*/ "36 (Invalid)"
,   /*37*/ "37 (Invalid)"
,   /*38*/ "38 (Invalid)"
,   /*39*/ "39 (Invalid)"
,   /*40*/ "40 (Invalid)"
,   /*41*/ "41 (Invalid)"
,   /*42*/ "42 (Invalid)"
,   /*43*/ "43 (Invalid)"
,   /*44*/ "44 (Invalid)"
,   /*45*/ "45 (Invalid)"
,   /*46*/ "46 (Invalid)"
,   /*47*/ "47 (Invalid)"
,   /*48*/ "48 (Invalid)"
,   /*49*/ "49 (Invalid)"
,   /*50*/ "50 (Invalid)"
,   /*51*/ "51 (Invalid)"
,   /*52*/ "52 (Invalid)"
,   /*53*/ "53 (Invalid)"
,   /*54*/ "54 (Invalid)"
,   /*55*/ "55 (Invalid)"
,   /*56*/ "56 (Invalid)"
,   /*57*/ "57 (Invalid)"
,   /*58*/ "58 (Invalid)"
,   /*59*/ "59 (Invalid)"
,   /*60*/ "60 (Invalid)"
,   /*61*/ "61 (Invalid)"
,   /*62*/ "62 (Invalid)"
,   /*63*/ "63 (Invalid)"
}; // signalName[]

//----------------------------------------------------------------------------
//
// Subroutine-
//       initialize [WINDOWS VERSION]
//
// Purpose-
//       Set up signal handlers.
//
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
static void
   sigexit(                         // Signal handler
     int               code)        // Signal identifier
{
   IFHCDM( debugf("Signal::sigexit(%d)\n", code); )

   switch( code )                   // Unbelievable!
   {
     case SIGINT:
       code= Signal::SC_INTERRUPT;
       break;

     case SIGILL:
       code= Signal::SC_INVALIDOP;
       break;

     case SIGFPE:
       code= Signal::SC_FPEXCEPTION;
       break;

     case SIGSEGV:
       code= Signal::SC_SEGERROR;
       break;

     case SIGTERM:
       code= Signal::SC_TERMINATE;
       break;

     case SIGBREAK:
       code= Signal::SC_USER2;
       break;

     case SIGABRT:
       code= Signal::SC_ABORT;
       break;

     default:
       break;
   }

   _SystemSignal::drive(code) ;     // Drive the handlers
   fflush(stdout);                  // Force the standard buffers
   fflush(stderr);
}

static void
   initialize( void )               // Set up signal handlers
{
   IFHCDM( debugf("Signal::initialize()\n"); )

   signal(SIGINT,   &sigexit);
   signal(SIGILL,   &sigexit);
   signal(SIGFPE,   &sigexit);
   signal(SIGSEGV,  &sigexit);
   signal(SIGTERM,  &sigexit);
   signal(SIGBREAK, &sigexit);
   signal(SIGABRT,  &sigexit);

   list= new List<Signal>();
}

#else // _OS_BSD
//----------------------------------------------------------------------------
//
// Subroutine-
//       initialize [BSD/LINUX VERSION]
//
// Purpose-
//       Set up signal handlers.
//
//----------------------------------------------------------------------------
static void
   sigexit(                         // Signal handler
     int               code,        // Signal identifier
     siginfo_t*        siginfo,     // Signal information
     void*             context)     // User context
{
// IFHCDM( debugf("Signal::sigexit(%d)\n", code); )

   // Drive the handlers
   _SystemSignal::drive(code, siginfo, context);
   fflush(stdout);                  // Force the standard buffers
   fflush(stderr);
}

static void
   initialize( void )               // Set up signal handlers
{
   IFHCDM( debugf("Signal::initialize()\n"); )

   struct sigaction inpvec;
   memset(&inpvec, 0, sizeof(inpvec));
   inpvec.sa_flags= SA_SIGINFO;     // Use sa_sigaction rather than sa_handler
   inpvec.sa_sigaction= &sigexit;
   for(int i= 1; i<32; i++) // Set up exits
     sigaction(i, &inpvec, &restore[i]);

   list= new List<Signal>();
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       terminate
//
// Purpose-
//       Restore initial signal handlers.
//
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
static void
   terminate( void )                // Restore signal handlers
{
   IFHCDM( debugf("Signal::terminate()\n"); )

   delete list;
   list= NULL;
}

#else // _OS_BSD
static void
   terminate( void )                // Restore signal handlers
{
   IFHCDM( debugf("Signal::terminate()\n"); )

   struct sigaction outvec;
   for(int i= 1; i<32; i++)
     sigaction(i, &restore[i], &outvec);

   delete list;
   list= NULL;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Signal::~Signal
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Signal::~Signal( void )          // Destructor
{
   IFHCDM( debugf("Signal(%p)::~Signal()\n", this); )

   AutoBarrier lock(barrier);

   list->remove(this, this);
   if( list->getHead() == NULL )
     terminate();
}

//----------------------------------------------------------------------------
//
// Method-
//       Signal::Signal
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Signal::Signal( void )           // Constructor
:  List<Signal>::Link()
,  mask(SM_DEFAULT)
{
   IFHCDM( debugf("Signal(%p)::Signal()\n", this); )

   AutoBarrier lock(barrier);

   if( list == NULL )
     initialize();

   list->lifo(this);
}

   Signal::Signal(                  // Constructor
     uint64_t          mask)        // Interrupt mask
:  List<Signal>::Link()
,  mask(mask)
{
   IFHCDM( debugf("Signal(%p)::Signal(0x%.10" PRIx64 ")\n", this, mask); )

   AutoBarrier lock(barrier);

   if( list == NULL )
     initialize();

   list->lifo(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Signal::disable
//
// Purpose-
//       Disable signal handling for a signal.
//
//----------------------------------------------------------------------------
void
   Signal::disable(                 // Disable signal handling
     SignalCode        ec)          // For this SignalCode
{
   uint64_t code= (1 << ec);
   code= ~code;
   mask &= code;
}

//----------------------------------------------------------------------------
//
// Method-
//       Signal::enable
//
// Purpose-
//       Enable signal handling for a signal.
//
//----------------------------------------------------------------------------
void
   Signal::enable(                  // Enable signal handling
     SignalCode        ec)          // For this SignalCode
{
   uint64_t code= (1 << ec);
   mask |= code;
}

//----------------------------------------------------------------------------
//
// Method-
//       Signal::getSignalName
//
// Purpose-
//       Describe a SignalCode.
//
//----------------------------------------------------------------------------
const char*                         // SignalCode description
   Signal::getSignalName(           // Convert SignalCode to text
     SignalCode        ec)          // SignalCode
{
   if( ec < 0 || ec >= SC_COUNT )
     return "Invalid SignalCode";

   return signalName[ec];
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Signal::handle
//
// Purpose-
//       Handle a signal (message and exit).
//
//----------------------------------------------------------------------------
int                                 // Return code (0 iff handled OK)
   Signal::handle(                  // Handle a signal
     SignalCode        ident)       // Signal identifier
{
   fprintf(stderr, "\n");
   fprintf(stderr, "Signal::handle(%d) %s\n", ident, getSignalName(ident));
   fflush(stderr);

   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Signal::generate
//
// Purpose-
//       Generate a Signal
//
//----------------------------------------------------------------------------
void
   Signal::generate(                // Generate
     SignalCode        ec)          // This SignalCode
{
   _SystemSignal::drive(ec);        // Drive the handlers
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemSignal::drive
//
// Function-
//       Drive all signal handlers.
//
//----------------------------------------------------------------------------
void
   _SystemSignal::drive(            // Drive signal handlers
     int               code,        // Signal identifier
     void*             _siginfo,    // BSD signal info
     void*             _context)    // BSD context
{
   IFHCDM( debugf("_SystemSignal::drive(%d)\n", code); )

   if( recursion )                  // If recursive call
   {
     fprintf(stderr, "%4d Signal(%d) recursion(%d)\n", __LINE__,
                     code, recursion);
     raise(SIGKILL);
   }

   recursion= code;                 // Indicate recursion
   if( code == 0 )                  // Insure non-zero value
     recursion= (-1);               // (Should never occur)

// In CYGWIN, this statement is not executed after above debugf)
// fprintf(stderr, "_SystemSignal::drive(%d) printf\n", code);

   #ifdef _OS_BSD
     sigset_t oset;                 // Old blocked signal set
     sigset_t nset;                 // New blocked signal set
     sigfillset(&nset);             // (All signals blocked)
     pthread_sigmask(SIG_SETMASK, &nset, &oset); // Block signals
   #endif
   if( barrier.attempt() == 0 )     // Lock recursion disallowed
   {
     Signal* signal= list->getHead();
     while( signal != NULL)
     {
       uint64_t mask= (uint64_t)1 << code;
       if( (signal->mask & mask) != 0 )
       {
         int rc= signal->handle(Signal::SignalCode(code));
         if( rc == 0 )
         {
           barrier.release();
           #ifdef _OS_BSD
             pthread_sigmask(SIG_SETMASK, &oset, &nset); // Enable signals
           #endif
           recursion= FALSE;
           return;
         }
       }

       signal= signal->getNext();
     }

     barrier.release();
   }

   //-------------------------------------------------------------------------
   // The signal was not handled. Handle it now.
   char buffer[64];
   sprintf(buffer, "Signal(%d) %s", code, signalName[code & 0x0000001f]);

   switch(code)
   {
     case Signal::SC_ALARM:         // 14 Alarm clock timeout
     case Signal::SC_STOP:          // 17 Stop
     case Signal::SC_CONTINUE:      // 19 Continue
     case Signal::SC_CHILDSTOP:     // 20 Child stop or exit
     case Signal::SC_BGRDCONTROL:   // 21 Background read from control terminal
     case Signal::SC_BGWRCONTROL:   // 22 Background write to control terminal
     case Signal::SC_WINDOWSIZE:    // 28 Window size changed
     case Signal::SC_MESSAGE:       // 27 Input data in HFT ring buffer
     case Signal::SC_POWEROUT:      // 29 Power fail restart
     case Signal::SC_USER1:         // 30 User signal 1
     case Signal::SC_USER2:         // 31 User signal 2
       IFHCDM(          debugf("%s IGNORED\n", buffer); )
//     ELHCDM( fprintf(stderr, "%s IGNORED\n", buffer); )

       #ifdef _OS_BSD
         pthread_sigmask(SIG_SETMASK, &oset, &nset); // Enable signals
       #endif
       recursion= FALSE;
       return;

//   case Signal::SC_INVALID:       //  0 (Invalid)
//   case Signal::SC_HANGUP:        //  1 Hangup
//   case Signal::SC_INTERRUPT:     //  2 Interrupt
//   case Signal::SC_QUIT:          //  3 Quit
//   case Signal::SC_INVALIDOP:     //  4 Illegal Instruction
//   case Signal::SC_TRACETRAP:     //  5 Trace trap
//   case Signal::SC_ABORT:         //  6 Process abort
//   case Signal::SC_AIXEMT:        //  7 EMT Instruction (AIX specific)
//   case Signal::SC_FPEXCEPTION:   //  8 Floating point exception
//   case Signal::SC_KILL:          //  9 Kill
//   case Signal::SC_BUSERROR:      // 10 Bus (specification) error
//   case Signal::SC_SEGERROR:      // 11 Segment violation
//   case Signal::SC_SYSERROR:      // 12 Bad argument to system call
//   case Signal::SC_PIPE:          // 13 No one to read pipe
//   case Signal::SC_TERMINATE:     // 15 Software termination signal
//   case Signal::SC_URGENTIO:      // 16 Ugent I/O channel condition
//   case Signal::SC_STOP:          // 17 Stop
//   case Signal::SC_INTERACT:      // 18 Interactive stop
//   case Signal::SC_IOPOSSIBLE:    // 23 I/O possible, or completed
//   case Signal::SC_TIMELIMIT:     // 24 CPU time limit exceeded
//   case Signal::SC_SIZELIMIT:     // 25 File size limit exceeded
     default:
       break;
   }

   //-------------------------------------------------------------------------
   // Diagnostic backtrace dump     // (Only supported on Linux)
   #if defined(__GNUC__) && defined(_OS_LINUX)
     #define BT_ARRAY_DIM 128
     void*               array[BT_ARRAY_DIM];

     struct sig_ucontext {          // From /usr/include/asm-generic/ucontext.h
       unsigned long     uc_flags;
       struct ucontext  *uc_link;
       stack_t           uc_stack;
       struct sigcontext uc_mcontext;
       sigset_t          uc_sigmask;   /* mask last for extensibility */
     };

//   siginfo_t*    si= (siginfo_t*)_siginfo;
     (void)_siginfo;                // (Currently) unused
     sig_ucontext* uc= (sig_ucontext*)_context;
     sigcontext*   sc= &uc->uc_mcontext;

     //-----------------------------------------------------------------------
     // Backtrace begin message
     debugf("\n");
     debugf("Signal(%d) %s\n\n", code, strsignal(code));

     //-----------------------------------------------------------------------
     // Failing registers
     /* Get the address at the time the signal was raised */
     #if defined(__i386__)          // GCC specific
       debugf(" EIP: %.8lx      EFLAGS: %.8lx\n", sc->eip, sc->eflags);
       debugf(" EBP: %.8lx         ESP: %.8lx\n", sc->ebp, sc->esp);
       debugf(" EAX: %.8lx         EBX: %.8lx\n", sc->eax, sc->ebx);
       debugf(" ECX: %.8lx         EDX: %.8lx\n", sc->ecx, sc->edx);
       debugf(" EDI: %.8lx         ESI: %.8lx\n", sc->edi, sc->esi);

     #elif defined(__x86_64__)      // GCC specific
       debugf(" RIP: %.16lx     EFLAGS: %.16lx\n", sc->rip, sc->eflags);
       debugf(" RBP: %.16lx        RSP: %.16lx\n", sc->rbp, sc->rsp);
       debugf(" RAX: %.16lx        RBX: %.16lx\n", sc->rax, sc->rbx);
       debugf(" RCX: %.16lx        RDX: %.16lx\n", sc->rcx, sc->rdx);
       debugf(" RDI: %.16lx        RSI: %.16lx\n", sc->rdi, sc->rsi);
       debugf(" R08: %.16lx        R09: %.16lx\n", sc->r8,  sc->r9 );
       debugf(" R10: %.16lx        R11: %.16lx\n", sc->r10, sc->r11);
       debugf(" R12: %.16lx        R13: %.16lx\n", sc->r12, sc->r13);
       debugf(" R14: %.16lx        R15: %.16lx\n", sc->r14, sc->r15);

     #else
       #error Unsupported architecture. // TODO: Add support for other arch.
     #endif

     //-----------------------------------------------------------------------
     // Caller's backtrace
     debugf("\n");
     int size= backtrace(array, BT_ARRAY_DIM);
     char** messages= backtrace_symbols(array, size);
     for(int i= 0; i < size && messages != NULL; ++i) // Skip first stack frame
       debugf("[bt]: [%2d] %s\n", i-3, messages[i]);

     free(messages);
   #else
     (void)_siginfo;                // Unused parameter
     (void)_context;                // Unused parameter
   #endif

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   IFHCDM( debugf("%s NOT HANDLED, EXIT\n", buffer);
     Debug::get()->flush();
//   raise(SIGKILL);                // This works, but exits with code 0!
     exit(1);
   )
   ELHCDM( throwf("%s EXCEPTION", buffer); )
}

