//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Signal.h
//
// Purpose-
//       Define signal handler.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef SIGNAL_H_INCLUDED
#define SIGNAL_H_INCLUDED

#include <stdint.h>

#ifndef LIST_H_INCLUDED
#include "List.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class _SystemSignal;                // The system-defined signal handler

//----------------------------------------------------------------------------
//
// Class-
//       Signal
//
// Purpose-
//       Signal handler
//
// Usage notes-
//       Signal handlers should not use non-reentrant routines such as
//       malloc or printf.
//
//       A signal during construction or destruction of any signal handler
//       is handled using the default action. The default action for all
//       signal handlers invokes printf to display an error message.
//
// Usage notes-
//       SC_KILL can only be generated. It cannot be handled. When generated,
//       it results in the termination of the process.
//
//----------------------------------------------------------------------------
class Signal : public List<Signal>::Link { // Signal handler
friend class _SystemSignal;         // Driven by _SystemSignal

//----------------------------------------------------------------------------
// Signal::Attributes
//----------------------------------------------------------------------------
private:
uint64_t               mask;        // The signal mask

//----------------------------------------------------------------------------
// Signal::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum SignalCode                     // Signal code
{  SC_INVALID=          0           // (Invalid)
,  SC_HANGUP=           1           // Hangup
,  SC_INTERRUPT=        2           // Interrupt
,  SC_QUIT=             3           // Quit
,  SC_INVALIDOP=        4           // Illegal Instruction
,  SC_TRACETRAP=        5           // Trace trap
,  SC_ABORT=            6           // Process abort
,  SC_AIXEMT=           7           // EMT Instruction (AIX specific)
,  SC_FPEXCEPTION=      8           // Floating point exception
,  SC_KILL=             9           // Kill
,  SC_BUSERROR=        10           // Bus (specification) error
,  SC_SEGERROR=        11           // Segment violation
,  SC_SYSERROR=        12           // Bad argument to system call
,  SC_PIPE=            13           // No one to read pipe
,  SC_ALARM=           14           // Alarm clock timeout
,  SC_TERMINATE=       15           // Software termination signal
,  SC_URGENTIO=        16           // Ugent I/O channel condition
,  SC_STOP=            17           // Stop
,  SC_INTERACT=        18           // Interactive stop
,  SC_CONTINUE=        19           // Continue
,  SC_CHILDSTOP=       20           // Child stop or exit
,  SC_BGRDCONTROL=     21           // Background read from control terminal
,  SC_BGWRCONTROL=     22           // Background write to control terminal
,  SC_IOPOSSIBLE=      23           // I/O possible, or completed
,  SC_TIMELIMIT=       24           // CPU time limit exceeded
,  SC_SIZELIMIT=       25           // File size limit exceeded
   // Invalid=         26           // (Invalid)
,  SC_MESSAGE=         27           // Input data in HFT ring buffer
,  SC_WINDOWSIZE=      28           // Window size changed
,  SC_POWEROUT=        29           // Power fail restart
,  SC_USER1=           30           // User signal 1
,  SC_USER2=           31           // User signal 2
   // Invalid=         32           // (Invalid)
   // Invalid=         33           // (Invalid)
   // Invalid=         34           // (Invalid)
   // Invalid=         35           // (Invalid)
   // Invalid=         36           // (Invalid)
   // Invalid=         37           // (Invalid)
   // Invalid=         38           // (Invalid)
   // Invalid=         39           // (Invalid)
   // Invalid=         40           // (Invalid)
   // Invalid=         41           // (Invalid)
   // Invalid=         42           // (Invalid)
   // Invalid=         43           // (Invalid)
   // Invalid=         44           // (Invalid)
   // Invalid=         45           // (Invalid)
   // Invalid=         46           // (Invalid)
   // Invalid=         47           // (Invalid)
   // Invalid=         48           // (Invalid)
   // Invalid=         49           // (Invalid)
   // Invalid=         50           // (Invalid)
   // Invalid=         51           // (Invalid)
   // Invalid=         52           // (Invalid)
   // Invalid=         53           // (Invalid)
   // Invalid=         54           // (Invalid)
   // Invalid=         55           // (Invalid)
   // Invalid=         56           // (Invalid)
   // Invalid=         57           // (Invalid)
   // Invalid=         58           // (Invalid)
   // Invalid=         59           // (Invalid)
   // Invalid=         60           // (Invalid)
   // Invalid=         61           // (Invalid)
   // Invalid=         62           // (Invalid)
   // Invalid=         63           // (Invalid)
,  SC_COUNT=           64           // Number of signals
}; // enum SignalCode

enum SignalMask                     // Signal masks
{  SM_DEFAULT= 0x000000000300FFFFULL // Includes all error conditions
,  SM_MAXIMUM= 0xFFFFFFFFFFFFFFFFULL // Includes everything
}; // enum SignalMask

//----------------------------------------------------------------------------
// Signal::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Signal( void );                 // Destructor
   Signal( void );                  // Default constructor (SM_DEFAULT)

   Signal(                          // Constructor
     uint64_t          mask);       // Signal mask (63..0)

//----------------------------------------------------------------------------
// Signal::Methods
//----------------------------------------------------------------------------
public:
void
   disable(                         // Disable signal handling
     SignalCode        signal);     // For this SignalCode

void
   enable(                          // Enable signal handling
     SignalCode        signal);     // For this SignalCode

virtual int                         // Return code (0 iff COMPLETELY handled)
   handle(                          // Handle a signal
     SignalCode        signal);     // The signal to handle

//----------------------------------------------------------------------------
// Signal::Static methods
//----------------------------------------------------------------------------
public:
static const char*                  // Associated signal description
   getSignalName(                   // Convert SignalCode to text
     SignalCode        signal);     // The SignalCode

static void
   generate(                        // Generate
     SignalCode        signal);     // This SignalCode
}; // class Signal

#endif // SIGNAL_H_INCLUDED
