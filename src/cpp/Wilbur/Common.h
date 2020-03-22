//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
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
//       Define the Common area for Wilbur Objects.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <com/define.h>
#include <com/Debug.h>
#include <com/Dispatch.h>
#include <com/Random.h>

#include "HttpClientThread.h"
#include "HttpServerThread.h"
#include "Properties.h"

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

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Background;
class DbMeta;
class HttpServerPluginMap;
class NetClient;

struct Global;

//----------------------------------------------------------------------------
//
// Class-
//       Common
//
// Purpose-
//       Define the Common area for Wilbur Objects.
//       The Common area is unique to a process, but is available to
//       and shared by all threads within that process.
//
// Notes-
//       See also: Global
//
//       This area is allocated in single-thread mode during start-up.
//       The Global area is also allocated at that time.
//       Start up is also responsible for deleting the Global and Common
//       areas during termination.
//
// Implementation notes-
//       <PRELIMINARY>
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
public:
Global*                global;      // The Global area
Random                 random;      // Pseudo-random number

unsigned               fsm;         // Finite State Machine
const char*            wilbur;      // Wilbur's external name

//----------------------------------------------------------------------------
// Threads
Dispatch               dispatcher;   // The generic Thread dispatcher
HttpClientThread       httpClient;   // HTTP client
HttpServerThread       httpServer;   // HTTP server

//----------------------------------------------------------------------------
// Plug-ins
Properties             properties;   // Properties (controls)
HttpServerPluginMap*   httpServerMap;// HttpServer Plugin Map

//----------------------------------------------------------------------------
// Services
Background*            background;   // Timed Background services
DbMeta*                dbMeta;       // The DbMeta object
NetClient*             netClient;    // The NetClient object

//----------------------------------------------------------------------------
// Common::Constructors
//----------------------------------------------------------------------------
public:
   ~Common( void );                 // Destructor

private:
   Common( void );                  // Constructor (SINGLETON)

private:                            // Bitwise copy is prohibited
   Common(const Common&);           // Disallowed copy constructor
   Common& operator=(const Common&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Common::Static methods
//----------------------------------------------------------------------------
private:
static Common*         common;      // The Common singleton

public:
static inline Common*               // -> THE Common area (Singleton)
   get( void )                      // Get -> Common
{
   return common;
}

static Common*                      // -> THE Common area (Singleton)
   activate(                        // Standard Common activation
     const char*       logFile= NULL); // (Override log file name)

//----------------------------------------------------------------------------
// Common::Methods
//----------------------------------------------------------------------------
public:
inline int                          // The Finite State Machine state
   getFSM( void ) const             // Get Finite State Machine state
{  return fsm;
}

void
   shutdown( void );                // Go into SHUTDOWN (CLOSE) state

void
   finalize( void );                // Wait for termination
}; // class Common
#endif // COMMON_H_INCLUDED
