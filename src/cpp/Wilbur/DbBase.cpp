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
//       DbBase.cpp
//
// Purpose-
//       Implement DbBase object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/List.h>

#include "DbBase.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_BRINGUP TRUE             // Use BRINGUP directory?

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Single-thread barrier
static List<DbBase>    list;        // The DbBase list

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
unsigned               DbBase::fsm= DbBase::FSM_RESET; // Global state
DbEnv*                 DbBase::dbEnv= NULL; // The common DbEnv

#if( USE_BRINGUP )
const char*            DbBase::DATABASE_PATH= "/database/Bringup/";
const char*            DbBase::DATABASE_NAME= "Wilbur/";
const char*            DbBase::DATABASE_TEMP= "temp/";

#else
const char*            DbBase::DATABASE_PATH= "/database/";
const char*            DbBase::DATABASE_NAME= "Wilbur/";
const char*            DbBase::DATABASE_TEMP= "Wilbur/temp/";
#endif

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::~DbBase
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbBase::~DbBase( void )          // Destructor
{
   //-------------------------------------------------------------------------
   // Construction/destruction protected by Barrier
   AutoBarrier lock(barrier);

   //-------------------------------------------------------------------------
   // Remove this DbBase from the list
   list.remove(this, this);

   //-------------------------------------------------------------------------
   // If this is the last DbBase, terminate operation
   if( list.getHead() == NULL )
   {
     if( dbEnv != NULL )
     {
       dbEnv->txn_checkpoint(0, 0, 0);
       dbEnv->close(0);

       delete dbEnv;
       dbEnv= NULL;
     }

     fsm= FSM_RESET;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::DbBase
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbBase::DbBase( void )           // Default constructor
:  List<DbBase>::Link()
{
   //-------------------------------------------------------------------------
   // Construction/destruction protected by Barrier
   AutoBarrier lock(barrier);

   //-------------------------------------------------------------------------
   // Initialize database environment (All DbBase databases are thread-safe)
   if( dbEnv == NULL )
   {
     uint32_t flags= DB_CREATE | DB_RECOVER | DB_REGISTER | DB_THREAD
                   | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
     dbEnv= new DbEnv((int)0);
     dbEnv->set_data_dir(DATABASE_NAME);
     dbEnv->set_tmp_dir(DATABASE_TEMP);
     int rc= dbEnv->open(DATABASE_PATH, flags, 0);
     if( rc != 0 )
     {
       dbEnv->close(0);

       delete dbEnv;
       dbEnv= NULL;

       checkstop(__FILE__, __LINE__, "%d= dbEnv->open(%s)", rc, DATABASE_PATH);
     }
   }

   //-------------------------------------------------------------------------
   // Add this element onto the List
   list.lifo(this);

   //-------------------------------------------------------------------------
   // Go into READY state
   fsm= FSM_READY;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::getTxn
//
// Purpose-
//       Create a transaction.
//
//----------------------------------------------------------------------------
DbTxn*                              // The transaction
   DbBase::getTxn(                  // Create a transaction
     DbTxn*            parent)      // With this parent transaction
{
   DbTxn*              result;      // Resultant

   dbEnv->txn_begin(parent, &result, 0);
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::abort
//
// Purpose-
//       Abort a transaction.
//
//----------------------------------------------------------------------------
void
   DbBase::abort(                   // Abort
     DbTxn*            dbTxn)       // This transaction
{
   dbTxn->abort();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::commit
//
// Purpose-
//       Commit a transaction.
//
//----------------------------------------------------------------------------
void
   DbBase::commit(                  // Commit
     DbTxn*            dbTxn)       // This transaction
{
   dbTxn->commit(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::shutdown
//
// Purpose-
//       Go into CLOSE state.
//
//----------------------------------------------------------------------------
void
   DbBase::shutdown( void )         // Go into CLOSE state
{
   fsm= FSM_CLOSE;

   //-------------------------------------------------------------------------
   // Close all DbBase instances
   for(;;)
   {
     DbBase* object= (DbBase*)list.getHead();
     if( object == NULL )
       break;

     object->~DbBase();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::fetch32
//       DbBase::fetch64
//
// Purpose-
//       Fetch index.
//
//----------------------------------------------------------------------------
uint32_t                            // The uint32_t index
   DbBase::fetch32(                 // Fetch uint32_t index
     const uint32_t*   index)       // From this location
{
   const unsigned char* addr= (const unsigned char*)index;

   uint32_t result= (addr[0] << 24)
                  | (addr[1] << 16)
                  | (addr[2] <<  8)
                  | (addr[3] <<  0);

   return result;
}

uint64_t                            // The uint64_t index
   DbBase::fetch64(                 // Fetch uint64_t index
     const uint64_t*   index)       // From this location
{
   const unsigned char* addr= (const unsigned char*)index;

   uint64_t result= fetch32((uint32_t*)addr);
   result <<= 32;
   result  |= fetch32((uint32_t*)(addr+4));

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::store32
//       DbBase::store64
//
// Purpose-
//       Store index.
//
//----------------------------------------------------------------------------
void
   DbBase::store32(                 // Store uint32_t index
     uint32_t*         index,       // Into this location
     uint32_t          value)       // With this index value
{
   unsigned char* addr= (unsigned char*)index;

   addr[0]= (value >> 24);
   addr[1]= (value >> 16);
   addr[2]= (value >>  8);
   addr[3]= (value >>  0);
}

void
   DbBase::store64(                 // Store uint64_t index
     uint64_t*         index,       // Into this location
     uint64_t          value)       // With this index value
{
   unsigned char* addr= (unsigned char*)index;

   store32((uint32_t*)addr, int32_t(value >> 32));
   store32((uint32_t*)(addr+4), int32_t(value));
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::checkstop
//
// Purpose-
//       Handle checkstop condition
//
//----------------------------------------------------------------------------
void
   DbBase::checkstop(               // Handle checkstop condition
     const char*       file,        // Failing file name
     int               line,        // Failing line number
     const char*       fmt,         // Failure message
                       ...)         // Message parameters
{
   va_list argptr;

   const char* message= "DbBase::CHECKSTOP";
   debugf("%4d %s %s ", line, file, message);
   va_start(argptr, fmt);
   vdebugf(fmt, argptr);
   va_end(argptr);

   debugf("\n");
   Debug::get()->flush();
   throw message;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::dbCheck
//
// Purpose-
//       Check result.
//
//----------------------------------------------------------------------------
void
   DbBase::dbCheck(                 // Write debugging message
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               cc,          // Associated condition code (TRUE)
     const char*       fmt,         // Associated message
                       ...)         // Message parameters
{
   #ifdef HCDM
     va_list argptr;

     va_start(argptr, fmt);
     dbDebugv(file, line, cc, fmt, argptr);
     va_end(argptr);

     if( cc == 0 )
       checkstop(file, line, "dbCheck");
   #else
     if( cc == 0 )
     {
       va_list argptr;

       va_start(argptr, fmt);
       dbDebugv(file, line, cc, fmt, argptr);
       checkstop(file, line, "dbCheck");
       va_end(argptr);
     }
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       DbBase::dbDebug
//       DbBase::dbDebugv
//
// Purpose-
//       Write debugging message
//
//----------------------------------------------------------------------------
void
   DbBase::dbDebug(                 // Write debugging message
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               rc,          // Associated return code
     const char*       fmt,         // Associated message
                       ...)         // Additional parameters
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   dbDebugv(file, line, rc, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   DbBase::dbDebugv(                // Write debugging message
     const char*       file,        // Source file name
     int               line,        // Source line number
     int               rc,          // Associated return code
     const char*       fmt,         // Associated message
     va_list           argptr)      // Message parameters
{
   tracef("%4d %s %d= ", line, file, rc);
   vtracef(fmt, argptr);
   tracef("\n");
}

