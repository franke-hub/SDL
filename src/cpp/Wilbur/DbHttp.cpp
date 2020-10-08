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
//       DbHttp.cpp
//
// Purpose-
//       Implement DbHttp object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stddef.h>                 // For offsetof
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Barrier.h>
#include <com/istring.h>

#include "DbHttp.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <DbBase.i>                 // Refresh DbBase macros for HCDM

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define INITIAL_INDEX ((uint64_t(DbHttp::EXTENDED_INDEX) << 48) + 1)

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
// Subroutine-
//       setSecondaryName
//
// Purpose-
//       Set the secondary name index.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   setSecondaryName(                // Set the secondary name index
     Db*                db,         // -> DB (unused)
     const Dbt*         pKey,       // Primary index Dbt
     const Dbt*         vInp,       // (Input) value Dbt
     Dbt*               sKey)       // Resultant (secondary index)
{
   (void)db;                        // Unused
   if( *(uint64_t*)pKey->get_data() == 0 )
     return DB_DONOTINDEX;

   sKey->set_data((char*)vInp->get_data() + offsetof(DbHttp::Value, name));
   sKey->set_size(vInp->get_size() - offsetof(DbHttp::Value, name));
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setSecondaryTime
//
// Purpose-
//       Set the secondary time index.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   setSecondaryTime(                // Set the secondary time index
     Db*                db,         // -> DB (unused)
     const Dbt*         pKey,       // Primary index Dbt
     const Dbt*         vInp,       // (Input) value Dbt
     Dbt*               sKey)       // Resultant (secondary index)
{
   (void)db;                        // Unused
   if( *(uint64_t*)pKey->get_data() == 0 )
     return DB_DONOTINDEX;

   sKey->set_data((char*)vInp->get_data() + offsetof(DbHttp::Value, time));
   sKey->set_size(sizeof(DbHttp::Value::time));
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::~DbHttp
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbHttp::~DbHttp( void )          // Destructor
{
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::DbHttp
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbHttp::DbHttp( void )           // Default constructor
:  DbBase()
,  dbIndex(NULL), ixName(NULL), ixTime(NULL)
{
   int                 rc;

   // Create the database and index objects
   {{{{
   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0); // Create transaction

   dbIndex= new Db(dbEnv, 0);
   ixName=  new Db(dbEnv, 0);
// ixName->set_flags(DB_DUP);       // (Don't) allow duplicates
   ixTime=  new Db(dbEnv, 0);
   ixTime->set_flags(DB_DUP);       // Allow duplicates

   // Open the databases, making the handles free threaded
   u_int32_t flags= DB_CREATE | DB_THREAD;
   dbIndex->open(dbTxn, "DbHttp.db", NULL, DB_BTREE, flags, 0);
   ixName-> open(dbTxn, "DbHttp_ixName.db", NULL, DB_BTREE, flags, 0);
   ixTime-> open(dbTxn, "DbHttp_ixTime.db", NULL, DB_BTREE, flags, 0);
   dbIndex->associate(dbTxn, ixName, setSecondaryName, 0);
   dbIndex->associate(dbTxn, ixTime, setSecondaryTime, 0);

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
//       DbHttp::getValue
//
// Purpose-
//       Get value for index.
//
//----------------------------------------------------------------------------
DbHttp::Value*                      // Result (NULL if error/missing)
   DbHttp::getValue(                // Get Http value
     void*             value,       // (OUT) Resultant
     uint64_t          index)       // For this Http index
{
   Value*              result= NULL;// Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated value.
   if( index != 0 )                 // No value for special index
   {
     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dbc= NULL;                // Create cursor
     dbIndex->cursor(dbTxn, &dbc, 0);

     // Get the current mapping
     store64(&xBuff, index);
     rc= dbc->get(&pKey, &vInp, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       result= (Value*)value;
       unsigned length= vInp.get_size();
       assert( length <= MAX_VALUE_LENGTH );
       assert( length >= offsetof(Value, name) );

       memcpy(result, vInp.get_data(), length);
       ((char*)result)[length]= '\0';
     }

     dbc->close();
     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::setValue
//
// Purpose-
//       Initialize value.
//
//----------------------------------------------------------------------------
DbHttp::Value*                      // Result (NULL if invalid data)
   DbHttp::setValue(                // Set Value
     void*             value,       // (OUT) Resultant
     uint64_t          text,        // For this TEXT index and
     uint64_t          time,        // For this expiration time and
     const char*       name)        // For this URI name
{
   Value*              result= (Value*)value; // Resultant

   //-------------------------------------------------------------------------
   // If present, skip over "http://"
   if( memicmp("http://", name, 7) == 0 )
     name += 7;

   //-------------------------------------------------------------------------
   // Set the value
   store64(&result->text, text);    // Set the text link index
   store64(&result->time, time);    // Set the expiration time
   if( strlen(name) > (MAX_VALUE_LENGTH - offsetof(Value, name)) )
     result= NULL;
   else
     strcpy(result->name, name);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::insert
//
// Purpose-
//       Insert Http value into database.
//
//----------------------------------------------------------------------------
uint64_t                            // The Http index (0 if error)
   DbHttp::insert(                  // Insert
     const Value*      value,       // This Http value, controlled by
     DbTxn*            parent)      // This parent transaction
{
   uint64_t            result= 0;   // Resultant (default, FAILED)
   const unsigned      length= offsetof(Value, name) + strlen(value->name);
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vOut((void*)value, length); // Output value Dbt

   int                 rc;

   // Verify Value length (even including integer length overflow)
   if( length > MAX_VALUE_LENGTH || length < offsetof(Value, name) )
     return 0;

   //-------------------------------------------------------------------------
   // Thread latch
   AutoBarrier lock(barrier);       // Thread latch required (for index)

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   //-------------------------------------------------------------------------
   // Failure example: duplicate URI name insert attempt
   try {
     // Write the database record
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

     // Set the resultant, update the insert index
     if( rc == 0 )
     {
       result= insertIX;
       insertIX++;
     }
   } catch( DbException& x ) {
     debugf("DbHttp(%p)::insert(%s) exception(%s)\n", this,
            value->name, x.what());
   }

   if( result == 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::locate
//
// Purpose-
//       Get index for URI name.
//
//----------------------------------------------------------------------------
uint64_t                            // The Http index (0 if error/missing)
   DbHttp::locate(                  // Get Http index
     const char*       name)        // For this URI name
{
   uint64_t            result= 0;   // Resultant
   unsigned            length= strlen(name); // Http length

   Dbt                 pKey;        // Primary index Dbt
   Dbt                 vInp;        // (Input) value Dbt (IGNORED)

   int                 rc;

   //-------------------------------------------------------------------------
   // If present, skip over "http://"
   if( memicmp("http://", name, 7) == 0 )
   {
     name   += 7;
     length -= 7;
   }

   Dbt xDbt((void*)name, length);   // Name index Dbt

   //-------------------------------------------------------------------------
   // Return the existing mapping index, if any.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixName->cursor(dbTxn, &dbc, 0);

   rc= dbc->pget(&xDbt, &pKey, &vInp, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
     result= getPrimaryIndex(__LINE__, pKey);

   dbc->close();
   dbTxn->commit(0);

   return result;
}

DbHttp::Value*                      // Result (NULL if error/missing)
   DbHttp::locate(                  // Get Http value
     const char*       name,        // For this URI name
     void*             value)       // (OUT) Resultant
{
   Value*              result= NULL; // Resultant
   unsigned            length= strlen(name); // Http length

   Dbt                 vInp;        // (Input) value Dbt

   int                 rc;

   //-------------------------------------------------------------------------
   // If present, skip over "http://"
   if( memicmp("http://", name, 7) == 0 )
   {
     name   += 7;
     length -= 7;
   }

   Dbt xDbt((void*)name, length);   // Name index Dbt

   //-------------------------------------------------------------------------
   // Return the existing mapping value, if any.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixName->cursor(dbTxn, &dbc, 0);

   rc= dbc->get(&xDbt, &vInp, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     result= (Value*)value;
     unsigned length= vInp.get_size();
     assert( length <= MAX_VALUE_LENGTH );
     assert( length >= offsetof(Value, name) );

     memcpy(result, vInp.get_data(), length);
     ((char*)result)[length]= '\0';
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::nextIndex
//
// Purpose-
//       Get next database Http index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbHttp::nextIndex(               // Get next Http index
     uint64_t          index)       // After this index
{
   uint64_t            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Returned value (IGNORED)

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
//       DbHttp::nextName
//
// Purpose-
//       Get next database Http name index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbHttp::nextName(                // Get next Http name index
     const char*       name)        // And this URI
{
   uint64_t            result= 0;   // Resultant
   unsigned            length= strlen(name); // Http length

   Dbt                 pKey;        // Returned (primary) index
   Dbt                 vInp;        // Returned value (IGNORED)

   int                 rc;

   //-------------------------------------------------------------------------
   // If present, skip over "http://"
   if( memicmp("http://", name, 7) == 0 )
   {
     name   += 7;
     length -= 7;
   }

   Dbt sKey((void*)name, length);   // Name index Dbt

   //-------------------------------------------------------------------------
   // If the Http is mapped, return the next mapping.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixName->cursor(dbTxn, &dbc, 0);

   if( name[0] == '\0' )            // If find first
   {
     rc= dbc->pget(&sKey, &pKey, &vInp, DB_FIRST); // Position at first
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
       result= getPrimaryIndex(__LINE__, pKey);
   }
   else
   {
     rc= dbc->get(&sKey, &vInp, DB_SET); // Position at name
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       rc= dbc->pget(&sKey, &pKey, &vInp, DB_NEXT);
       DBDEBUG(rc, "dbc->pget");
       if( rc == 0 )
         result= getPrimaryIndex(__LINE__, pKey);
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::nextTime
//
// Purpose-
//       Get next database Http time index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbHttp::nextTime(                // Get next Http time index
     uint64_t          index,       // After this HTTP index
     uint64_t          time)        // And this expiration time
{
   uint64_t            result= 0;   // Resultant
   int                 found= FALSE; // TRUE when entry found
   uint64_t            xBuff;       // Time index buffer

   Dbt                 pKey;        // Returned (primary) index
   Dbt                 sKey(&xBuff, sizeof(xBuff)); // (Time) index
   Dbt                 vInp;        // Returned value

   int                 rc;

   //-------------------------------------------------------------------------
   // Return the next time index after the specified time index
   store64(&xBuff, time);           // Store time in network format

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixTime->cursor(dbTxn, &dbc, 0);

   if( index == 0 && time == 0 )    // Find first?
   {
     found= TRUE;
     rc= dbc->pget(&sKey, &pKey, &vInp, DB_FIRST); // Position at (first) time
   }
   else
     rc= dbc->pget(&sKey, &pKey, &vInp, DB_SET); // Position at (first) match
   DBDEBUG(rc, "dbc->get");
   while( rc == 0 )
   {
     uint64_t httpIX= getPrimaryIndex(__LINE__, pKey);
     if( found )
     {
       result= httpIX;
       break;
     }

     if( httpIX == index )          // If this is the original item
       found= TRUE;                 // We want the next one

     DbHttp::Value* value= (DbHttp::Value*)vInp.get_data();
     uint64_t timeIX= fetch64(&value->time);
     if( timeIX != time )           // If we didn't find the original item
       break;                       // The next item is meaningless

     // This record did not match, get the next one
     rc= dbc->pget(&sKey, &pKey, &vInp, DB_NEXT);
     DBDEBUG(rc, "dbc->get");
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbHttp::remove
//
// Purpose-
//       Delete Http index from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbHttp::remove(                  // Remove
     uint64_t          index,       // This Http index, controlled by
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
//       DbHttp::revise
//
// Purpose-
//       Delete Http index from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbHttp::revise(                  // Replace
     uint64_t          index,       // This Http index with
     const Value*      value,       // This HTTP value, controlled by
     DbTxn*            parent)      // This parent transaction
{
   const unsigned      length= offsetof(Value, name) + strlen(value->name);
   uint64_t            xBuff;       // Index buffer

   Dbt                 pKey(&xBuff, sizeof(xBuff)); // Primary index Dbt
   Dbt                 vInp;        // Input value Dbt (IGNORED)
   Dbt                 vOut((void*)value, length); // Replacement value Dbt

   int                 rc= (-1);    // (Default, generic fault)

   // Do not allow index[0] replacement or over/underlength value
   if( index == 0
       || length > MAX_VALUE_LENGTH || length < offsetof(Value, name) )
     return rc;

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   //-------------------------------------------------------------------------
   // Failure example: Revise results in a duplicate name
   try {
     store64(&xBuff, index);        // Set the index
     rc= dbc->get(&pKey, &vInp, DB_SET | DB_RMW);
     DBDEBUG(rc, "dbc->pget");

     if( rc == 0 )
     {
       rc= dbc->put(&pKey, &vOut, DB_CURRENT); // Replace the current entry
       DBDEBUG(rc, "db->put");
     }
   } catch( DbException& x ) {
     rc= x.get_errno();             // Indicate error
     if( rc == 0 )                  // (Do not allow zero)
       rc= (-2);

     debugf("DbHttp(%p)::revise(%s) exception(%s)\n", this,
            value->name, x.what());
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
//       DbHttp::close
//
// Purpose-
//       Close the database.
//
//----------------------------------------------------------------------------
void
   DbHttp::close( void )            // Close the database
{
   // Create a checkpoint
   dbEnv->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( ixName != NULL )
   {
     delete ixName;
     ixName= NULL;
   }

   if( ixTime != NULL )
   {
     delete ixTime;
     ixTime= NULL;
   }

   if( dbIndex != NULL )
   {
     delete dbIndex;
     dbIndex= NULL;
   }
}

