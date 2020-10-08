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
//       DbFile.cpp
//
// Purpose-
//       Implement DbFile object methods
//
// Last change date-
//       2018/01/01 DB4/5 compatibility
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Barrier.h>

#include "Common.h"                 // Only for logf
#include "DbFile.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

#define INITIAL_INDEX ((uint64_t(DbFile::EXTENDED_INDEX) << 48) + 1)
#define INTERNAL_COUNT 256          // Number of internal buffer words

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
//       setSecondary
//
// Purpose-
//       Set the secondary (value) index.
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   setSecondary(                    // Set the secondary (value) index
     Db*                db,         // -> DB (unused)
     const Dbt*         xDbt,       // Index Dbt descriptor
     const Dbt*         vDbt,       // Value Dbt descriptor
     Dbt*               sDbt)       // Resultant (secondary index)
{
   (void)db;                        // (Unused)
   if( *(uint64_t*)xDbt->get_data() == 0 )
     return DB_DONOTINDEX;

   sDbt->set_data(vDbt->get_data());// The secondary key *IS* the value
   sDbt->set_size(vDbt->get_size());
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::~DbFile
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbFile::~DbFile( void )          // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::DbFile
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbFile::DbFile( void )           // Default constructor
:  DbBase()
,  dbAssoc(NULL), dbIndex(NULL), ixValue(NULL)
{
   int                 rc;

   // Open the database and indexes
   {{{{
   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   dbAssoc= new Db(dbEnv, 0);
   dbIndex= new Db(dbEnv, 0);
   ixValue= new Db(dbEnv, 0);
// ixValue->set_flags(DB_DUP);      // Allow duplicates

   // Open the databases, making the handles thread-safe
   u_int32_t flags= DB_CREATE | DB_THREAD;
   dbIndex->open(dbTxn, "DbFile.db", NULL, DB_BTREE, flags, 0);
   ixValue->open(dbTxn, "DbFile_ixFile.db", NULL, DB_BTREE, flags, 0);
   dbAssoc->open(dbTxn, "DbFile_ixLink.db", NULL, DB_BTREE, flags, 0);
   dbIndex->associate(dbTxn, ixValue, setSecondary, 0);

   dbTxn->commit(0);
   }}}}

   //-------------------------------------------------------------------------
   // Get the current insert index
   AutoBarrier lock(barrier);       // Index initialization is protected

   if( insertIX == 0 )
   {
     DbTxn* dbTxn= NULL;
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dbc= NULL;                // Create cursor
     dbIndex->cursor(dbTxn, &dbc, 0);

     Dbt xRet;
     // rc= dbIndex->get(dbTxn, &zDbt, &xRet, 0);
     rc= dbc->get(&zDbt, &xRet, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( xRet.get_size() != sizeof(uint64_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       insertIX= fetch64((uint64_t*)xRet.get_data());
     }
     else
     {
       //---------------------------------------------------------------------
       // The insert index does not exist.
       // Create the INSERT/FIRST index.
       uint64_t xBuff;
       Dbt xDbt(&xBuff, sizeof(uint64_t));
       store64(&xBuff, INITIAL_INDEX);

       rc= dbIndex->put(dbTxn, &zDbt, &xDbt, DB_NOOVERWRITE); // Create INSERT/FIRST
       // rc= dbc->put(&zDbt, &xDbt, DB_NOOVERWRITE); // Create INSERT/FIRST
       if( rc != 0 )
       {
         dbTxn->abort();
         reset();
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
//       DbFile::getAssoc
//
// Purpose-
//       Get assocation with index.
//
//----------------------------------------------------------------------------
uint64_t                            // The association link
   DbFile::getAssoc(                // Get association link
     uint64_t          index)       // For this File index
{
   uint64_t            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated information.
   store64(&xBuff, index);          // Set the index

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   dbAssoc->cursor(dbTxn, &dbc, 0);

   // Get the current map
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     if( vRet.get_size() != sizeof(uint64_t) )
       checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

     result= fetch64((uint64_t*)vRet.get_data());
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::getIndex
//
// Purpose-
//       Get index for value.
//
//----------------------------------------------------------------------------
uint64_t                            // The File index (0 if error/missing)
   DbFile::getIndex(                // Get File index
     const char*       value)       // For this value
{
   uint64_t            result= 0;   // Resultant
   const unsigned      length= strlen(value); // File length

   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vDbt((void*)value, length); // Value Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // Return the existing mapping, if any.
   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                  // Create cursor
   ixValue->cursor(dbTxn, &dbc, 0);

   rc= dbc->pget(&vDbt, &xRet, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     if( xRet.get_size() != sizeof(uint64_t) )
       checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

     result= fetch64((uint64_t*)xRet.get_data());
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::getValue
//
// Purpose-
//       Get value for index.
//
//----------------------------------------------------------------------------
char*                               // Result (NULL if error/missing)
   DbFile::getValue(                // Get File value
     uint64_t          index,       // For this File index
     char*             value)       // (OUTPUT) Result string
{
   char*               result= NULL;// Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated value.
   if( index == 0 )                 // No value for special index
     return NULL;

   store64(&xBuff, index);

   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;
   dbIndex->cursor(dbTxn, &dbc, 0);

   // Get the current map
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     result= value;
     unsigned length= vRet.get_size();
     memcpy(result, vRet.get_data(), length);
     result[length]= '\0';
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::setAssoc
//
// Purpose-
//       Set assocation with index.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DbFile::setAssoc(                // Set association between
     uint64_t          index,       // This File index and
     uint64_t          assoc,       // This Link index, controlled by
     DbTxn*            parent)      // This parent transaction
{
   int                 result= 0;   // Resultant
   uint64_t            aBuff;       // Assoc buffer
   uint64_t            xBuff;       // Index buffer

   Dbt                 aDbt(&aBuff, sizeof(uint64_t)); // Assoc Dbt
   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   int                 rc;

   if( index == 0 )                 // Cannot modify special index
     return (-1);

   store64(&aBuff, assoc);
   store64(&xBuff, index);

   DbTxn* dbTxn;
   dbEnv->txn_begin(parent, &dbTxn, 0);

   Dbc* dbi= NULL;
   dbIndex->cursor(dbTxn, &dbi, 0);

   // Get the current map
   rc= dbi->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     if( assoc == 0 )
     {
       Dbc* dba= NULL;
       dbAssoc->cursor(dbTxn, &dba, 0);
       rc= dba->get(&xDbt, &vRet, DB_SET);
       DBDEBUG(rc, "dba->get");
       if( rc != 0 )
         rc= 0;
       else
       {
         rc= dba->del(0);
         DBDEBUG(rc, "dba->del");
       }

       dba->close();
     }
     else
     {
       rc= dbAssoc->put(dbTxn, &xDbt, &aDbt, DB_OVERWRITE_DUP);
       DBDEBUG(rc, "db->put");
     }
   }

   dbi->close();

   result= rc;
   if( result != 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::insert
//
// Purpose-
//       Insert file name into database.
//
//----------------------------------------------------------------------------
uint64_t                            // The File index (0 if error)
   DbFile::insert(                  // Insert into database
     const char*       value,       // This File value, controlled by
     DbTxn*            parent)      // This parent transaction
{
   uint64_t            result;      // Resultant
   uint64_t            xBuff;       // Index buffer
   const unsigned      length= strlen(value); // File name length

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vDbt((void*)value, length); // Value Dbt

   int                 rc= 0;

   // Verify Http validity
   if( length == 0 || length > MAX_VALUE_LENGTH )
     return 0;

   //-------------------------------------------------------------------------
   // Thread latch
   AutoBarrier lock(barrier);       // Thread latch required (for index)

   // If already indexed, return that value
   result= getIndex(value);
   if( result != 0 )
     return result;

   store64(&xBuff, insertIX);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   // Write the database record
   if( rc == 0 )
   {
     rc= dbIndex->put(dbTxn, &xDbt, &vDbt, DB_NOOVERWRITE);
     DBDEBUG(rc, "db->put");
   }

   // Update the insert index
   if( rc == 0 )
   {
     store64(&xBuff, insertIX+1);
     rc= dbIndex->put(dbTxn, &zDbt, &xDbt, DB_OVERWRITE_DUP);
     DBDEBUG(rc, "db->put");
   }

   // Set the resultant
   result= 0;
   if( rc == 0 )
   {
     result= insertIX;
     insertIX++;
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
//       DbFile::nextIndex
//
// Purpose-
//       Get next database File index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbFile::nextIndex(               // Get next File index
     uint64_t          index)       // After this index
{
   uint64_t            result= 0;   // Resultant

   uint64_t            xBuff;       // Index buffer
   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 xRet;        // Returned index
   Dbt                 vRet;        // Returned value

   int                 rc;

   //-------------------------------------------------------------------------
   // If the File is mapped, return the next mapping.
   DbTxn* dbTxn= NULL;            // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                // Create cursor
   dbIndex->cursor(dbTxn, &dbc, 0);

   store64(&xBuff, index);        // Current index
   rc= dbc->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->get(&xRet, &vRet, DB_NEXT);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( (xRet.get_size() % sizeof(uint64_t)) != 0 )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       result= fetch64((uint64_t*)xRet.get_data());
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::nextValue
//
// Purpose-
//       Get next database File value
//
//----------------------------------------------------------------------------
char*                               // Result (NULL if error/missing)
   DbFile::nextValue(               // Get next File value
     char*             value)       // (IN/OUT) File value
{
   char*               result= NULL;// Resultant
   unsigned            length= strlen(value); // File length

   Dbt                 xRet;        // Returned index
   Dbt                 vDbt((void*)value, length); // Value Dbt
   Dbt                 vRet;        // Returned value
   Dbt                 ignore;      // Returned value (IGNORED)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the File is mapped, return the next mapping.
   DbTxn* dbTxn= NULL;            // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbc= NULL;                // Create cursor
   ixValue->cursor(dbTxn, &dbc, 0);

   rc= dbc->get(&vDbt, &xRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbc->pget(&vRet, &xRet, &ignore, DB_NEXT);
     DBDEBUG(rc, "dbc->pget");
     if( rc == 0 )
     {
       result= value;

       length= vRet.get_size();
       memcpy(result, vRet.get_data(), length);
       result[length]= '\0';
     }
   }

   dbc->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::remove
//
// Purpose-
//       Delete File from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbFile::remove(                  // Remove
     uint64_t          index)       // This File index
{
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   if( index == 0 )
     return (-1);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbi= NULL;                  // Create cursors
   Dbc* dba= NULL;
   dbIndex->cursor(dbTxn, &dbi, 0);
   dbAssoc->cursor(dbTxn, &dba, 0);

   // Delete the current map
   store64(&xBuff, index);          // Set the index
   rc= dbi->get(&xDbt, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     if( vRet.get_size() == 1
         && *((char*)vRet.get_data()) == '*' )
       rc= (-1);
     else
     {
       rc= dbi->del(0);
       DBDEBUG(rc, "dbc->del");
     }
   }

   if( rc == 0 )
   {
     rc= dba->get(&xDbt, &vRet, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc != 0 )
       rc= 0;
     else
     {
       rc= dba->del(0);
       DBDEBUG(rc, "dbc->get");
     }
   }

   dba->close();
   dbi->close();

   if( rc != 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbFile::reset
//
// Purpose-
//       Reset the database.
//
//----------------------------------------------------------------------------
void
   DbFile::reset( void )            // Reset the database
{
   // Create a checkpoint
   dbEnv->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( dbAssoc != NULL )
   {
     delete dbAssoc;
     dbAssoc= NULL;
   }

   if( ixValue != NULL )
   {
     delete ixValue;
     ixValue= NULL;
   }

   if( dbIndex != NULL )
   {
     delete dbIndex;
     dbIndex= NULL;
   }
}

