//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbBase.h
//
// Purpose-
//       The DbBase is the base class for all database Objects.
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#ifndef DBBASE_H_INCLUDED
#define DBBASE_H_INCLUDED

#include <db_cxx.h>                 // This include is guaranteed
#include <stdarg.h>                 // This include is required (va_list)
#include <stdint.h>                 // This include is required (uint*_t)
#include <string>                   // This include is guaranteed
#include <com/define.h>             // This include is required (_ATTRIBUTE_PRINTF)
#include <com/List.h>               // This include is required (base class)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Db;                           // Used in derived classes
class DbEnv;                        // Used here
class DbTxn;                        // Used here

//----------------------------------------------------------------------------
//
// Class-
//       DbBase
//
// Purpose-
//       The DbBase is the base class for all Wilbur database Objects.
//       It contains the DB4 database environment.
//
// Implementation notes-
//       <PRELIMINARY>
//
//----------------------------------------------------------------------------
class DbBase : public List<DbBase>::Link { // DbBase
//----------------------------------------------------------------------------
// DbBase::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine
{  FSM_RESET= 0                     // Reset, inactive
,  FSM_READY                        // Ready, operational
,  FSM_CLOSE                        // Close, shutdown in progress
}; // enum FSM

//----------------------------------------------------------------------------
// DbBase::Attributes
//----------------------------------------------------------------------------
protected:
static unsigned        fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// Database objects
public:
static DbEnv*          dbEnv;       // The (thread-safe) DB environment
static const char*     DATABASE_PATH; // Database prefix
static const char*     DATABASE_NAME; // Database folder
static const char*     DATABASE_TEMP; // Database temp folder

//----------------------------------------------------------------------------
// DbBase::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbBase( void );                 // Destructor
   DbBase( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbBase(const DbBase&);           // Disallowed copy constructor
   DbBase& operator=(const DbBase&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DbBase::Object methods
//----------------------------------------------------------------------------
public:
DbTxn*                              // -> DbTxn
   getTxn(                          // Create transaction
     DbTxn*            parent= NULL); // Using this parent transaction

//----------------------------------------------------------------------------
// DbBase::Static methods
//----------------------------------------------------------------------------
public:
static void
   abort(                           // ABORT
     DbTxn*            dbTxn);      // This transaction

static void
   commit(                          // COMMIT
     DbTxn*            dbTxn);      // This transaction

inline static int                   // The Finite State Machine state
   getFSM( void )                   // Get Finite State Machine state
{  return fsm;
}

static void
   shutdown( void );                // Delete ALL DbBases

//----------------------------------------------------------------------------
// DbBase::Utility methods
//----------------------------------------------------------------------------
static uint32_t                     // The uint32_t index
   fetch32(                         // Fetch uint32_t index (ntohl equivalent)
     const uint32_t*   index);      // From this location

static uint64_t                     // The uint64_t index
   fetch64(                         // Fetch uint64_t index
     const uint64_t*   index);      // From this location

static void
   store32(                         // Store uint32_t index (htonl equivalent)
     uint32_t*         index,       // Into this location
     uint32_t          value);      // With this index value

static void
   store64(                         // Store uint64_t index
     uint64_t*         index,       // Into this location
     uint64_t          value);      // With this index value

static void
   checkstop(                       // Checkstop
     const char*       file,        // Source file name
     int               line,        // Source line number
     const char*       message,     // Failure message
                       ...)         // Message parameters
     _ATTRIBUTE_PRINTF(3, 4);

static void
   dbCheck(                         // Check condition
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               cc,          // Associated condition code
     const char*       message,     // Associated message
                       ...)         // Message parameters
     _ATTRIBUTE_PRINTF(4, 5);

static void
   dbDebug(                         // Write debugging message
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               rc,          // Associated return code
     const char*       message,     // Associated message
                       ...)         // Message parameters
     _ATTRIBUTE_PRINTF(4, 5);

static void
   dbDebugv(                        // Write debugging message
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               rc,          // Associated return code
     const char*       message,     // Associated message
     va_list           argptr)      // VALIST
     _ATTRIBUTE_PRINTF(4, 0);
}; // class DbBase
#include "DbBase.i"                 // Define macros

#endif // DBBASE_H_INCLUDED
