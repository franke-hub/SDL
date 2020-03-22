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
//       DbText.cpp
//
// Purpose-
//       Implement DbText object methods
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

#include "DbText.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>            // After #define HCDM
#include "DbBase.i"                 // After #define HCDM

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define INITIAL_INDEX ((uint64_t(DbText::EXTENDED_INDEX) << 48) + 1)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Single thread latch
static uint64_t        insertIX= 0; // Insert index (Barrier protected)

static const uint64_t  zero= 0;     // Constant zero
static Dbt             zDbt((void*)&zero, sizeof(zero)); // Constant zero Dbt

//----------------------------------------------------------------------------
//
// Subroutine-
//       getPrimaryIndex
//
// Purpose-
//       Validate, then return a primary index value.
//
//----------------------------------------------------------------------------
static uint64_t                     // The primary index value
   getPrimaryIndex(                 // Get the primary index value
     int                line,       // Line number
     const Dbt&         pKey)       // Primary index Dbt
{
   if( pKey.get_size() != sizeof(uint64_t) )
     DbBase::checkstop(__FILE__, line, "size(%d)", pKey.get_size());

   return DbBase::fetch64((uint64_t*)pKey.get_data());
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::~DbText
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbText::~DbText( void )          // Destructor
{
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::DbText
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbText::DbText( void )           // Default constructor
:  DbBase()
,  dbIndex(NULL)
{
   int                 rc;

   // Create the database object
   {{{{
   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0); // Create transaction

   dbIndex= new Db(dbEnv, 0);

   // Open the database, making the handles free threaded
   u_int32_t flags= DB_CREATE | DB_THREAD;
   dbIndex->open(dbTxn, "DbText.db", NULL, DB_BTREE, flags, 0);

   dbTxn->commit(0);
   }}}}

   //-------------------------------------------------------------------------
   // Get the current insert index
   AutoBarrier lock(barrier);       // Index initialization is protected

   if( insertIX == 0 )              // If not already initialized
   {
     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dbc= NULL;                // Create cursor
     dbIndex->cursor(dbTxn, &dbc, 0);

     Dbt vInp;                      // Input value (KEY)
     rc= dbc->get(&zDbt, &vInp, DB_SET); // Get insert key
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
       insertIX= getPrimaryIndex(__LINE__, vInp);
     else
     {
       //---------------------------------------------------------------------
       // The insert index does not exist. Create it.
       uint64_t xBuff;
       store64(&xBuff, INITIAL_INDEX);
       Dbt vOut(&xBuff, sizeof(xBuff));

       rc= dbIndex->put(dbTxn, &zDbt, &vOut, DB_NOOVERWRITE);
       DBDEBUG(rc, "db->put");
       if( rc != 0 )
       {
         dbc->close();
         dbTxn->abort();

         close();
         checkstop(__FILE__, __LINE__, "rc(%d)", rc);
       }

       insertIX= INITIAL_INDEX;
     }

     dbc->close();
     dbTxn->commit(0);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::getValue
//
// Purpose-
//       Get value for index.
//
//----------------------------------------------------------------------------
char*                               // MALLOC result (NULL if error/missing)
   DbText::getValue(                // Get value
     uint64_t          index)       // For this index
{
   char*               result= NULL;// Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated value.
   if( index == 0 )                 // No value for special index
     return NULL;

   store64(&xBuff, index);          // Set the index
   vInp.set_flags(DB_DBT_MALLOC);   // MALLOC the result
   rc= dbIndex->get(NULL, &pKey, &vInp, DB_READ_COMMITTED);
   DBDEBUG(rc, "db->get");
   if( rc == 0 )
     result= (char*)vInp.get_data();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::insert
//
// Purpose-
//       Insert into database.
//
//----------------------------------------------------------------------------
uint64_t                            // The index (0 if error)
   DbText::insert(                  // Insert into database
     const char*       value,       // This content, under control of
     DbTxn*            parent)      // This transaction
{
   uint64_t            result= 0;   // Resultant
   const unsigned      length= strlen(value) + 1; // Value length
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vOut((void*)value, length); // Output value Dbt

   int                 rc;

   //-------------------------------------------------------------------------
   // Thread latch
   AutoBarrier lock(barrier);       // Thread latch required (for index)

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   // Write the database records
   store64(&xBuff, insertIX);
   rc= dbIndex->put(dbTxn, &pKey, &vOut, DB_NOOVERWRITE);
   DBDEBUG(rc, "db->put");

   // Replace the insert index
   if( rc == 0 )
   {
     store64(&xBuff, insertIX+1);
     rc= dbIndex->put(dbTxn, &zDbt, &pKey, DB_OVERWRITE_DUP);
     DBDEBUG(rc, "db->put");
   }

   if( rc != 0 )
     dbTxn->abort();
   else
   {
     result= insertIX;
     insertIX++;

     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::nextIndex
//
// Purpose-
//       Get next database content index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbText::nextIndex(               // Get next index
     uint64_t          index)       // After this index
{
   uint64_t            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt (IGNORED)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the next primary index.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   store64(&xBuff, index);          // Current index
   rc= dbc->get(&pKey, &vInp, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->get(&pKey, &vInp, DB_NEXT);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
       result= getPrimaryIndex(__LINE__, pKey);
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::remove
//
// Purpose-
//       Delete content from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbText::remove(                  // Remove
     uint64_t          index,       // This text index, controlled by
     DbTxn*            parent)      // This parent transaction
{
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt (IGNORED)

   int                 rc;

   if( index == 0 )
     return (-1);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   // Delete the current mapping
   store64(&xBuff, index);          // Set the index
   rc= dbc->get(&pKey, &vInp, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->del(0);
     DBDEBUG(rc, "dbc->del");
   }

   dbc->close();

   if( rc != 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::revise
//
// Purpose-
//       Replace value.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DbText::revise(                  // Replace value
     uint64_t          index,       // For this index
     const char*       value,       // This content, under control of
     DbTxn*            parent)      // This transaction
{
   const unsigned      length= strlen(value);
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt (IGNORED)
   Dbt                 vOut((void*)value, length); // Replacement value Dbt

   int                 rc= (-1);    // (Default, generic fault)

   //-------------------------------------------------------------------------
   // If the index is mapped, set the associated links.
   if( index == 0 )                 // Disallow index[0] replacement
     return -1;

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   store64(&xBuff, index);          // Set the index
   rc= dbc->get(&pKey, &vInp, DB_SET | DB_RMW);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->put(&pKey, &vOut, DB_CURRENT);
     DBDEBUG(rc, "dbc->put");
   }

   dbc->close();

   if( rc != 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbText::close
//
// Purpose-
//       Reset the database
//
//----------------------------------------------------------------------------
void
   DbText::close( void )            // Close the database
{
   // Create a checkpoint
   dbEnv->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( dbIndex != NULL )
   {
     delete dbIndex;
     dbIndex= NULL;
   }
}

