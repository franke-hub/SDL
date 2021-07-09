//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Common.h
//
// Purpose-
//       Define the Brian Common area.
//
// Last change date-
//       2021/07/09
//
//----------------------------------------------------------------------------
#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <pub/Dispatch.h>
#include <pub/Event.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Service;

//----------------------------------------------------------------------------
//
// Class-
//       Common
//
// Purpose-
//       Define the Brian common area.
//       The Common area is unique to a process, but is available to
//       and shared by all threads within that process.
//
// Notes-
//       This area is allocated in single-thread mode during start-up.
//       Start up is also responsible for deleting the Common area during
//       termination.
//
//----------------------------------------------------------------------------
class Common {                      // Common data area
//----------------------------------------------------------------------------
// Common::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine
{  FSM_RESET= 0                     // Reset, inactive
,  FSM_READY                        // Ready, operational
,  FSM_CLOSE                        // Close, shutdown in progress
}; // enum FSM

//----------------------------------------------------------------------------
// Common::Attributes
//----------------------------------------------------------------------------
private:
static Common*         common;      // The Common singleton
pub::Event             event;       // The termination event
unsigned               fsm;         // Finite State Machine

protected:
const std::string      brian;       // Brian's external name

//----------------------------------------------------------------------------
// Common::Constructors
//----------------------------------------------------------------------------
private:
   Common( void );                  // Constructor (SINGLETON)

   Common(const Common&) = delete;  // Disallowed copy constructor
   Common& operator=(const Common&) = delete; // Disallowed assignment operator

public:
   ~Common( void );                 // Destructor

static Common*                      // -> THE Common area (Singleton)
   make( void );                    // Create the Common singleton

//----------------------------------------------------------------------------
// Common::Accessors
//----------------------------------------------------------------------------
public:
static inline Common*               // -> THE Common area (Singleton)
   get( void )                      // Get -> Common
{  return common; }

inline int                          // The Finite State Machine state
   get_FSM( void ) const            // Get Finite State Machine state
{  return fsm; }

inline const std::string            // Our external name
   get_name(  void ) const          // Get external name
{  return brian; }

//----------------------------------------------------------------------------
// Common::Methods
//----------------------------------------------------------------------------
public:
void*                               // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     pub::dispatch::Item*
                       item)        // Complete this work Item
{  return pub::dispatch::Disp::delay(seconds, item); }

void
   shutdown( void );                // Go into SHUTDOWN (CLOSE) state

void
   wait( void );                    // Wait for termination

void
   work(                            // Drive the Task
     pub::dispatch::Task*
                       task,        // -> Task
     pub::dispatch::Item*
                       item)        // -> Item
{  task->enqueue(item); }

void
   work(                            // Drive a service
     std::string       name,        // The Service name
     pub::dispatch::Item*
                       item);       // -> work Item
}; // class Common
#endif // COMMON_H_INCLUDED
