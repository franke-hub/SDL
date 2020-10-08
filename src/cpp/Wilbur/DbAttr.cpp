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
//       DbAttr.cpp
//
// Purpose-
//       Implement DbAttr object methods
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

#include "DbAttr.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define INITIAL_INDEX ((uint64_t(DbAttr::EXTENDED_INDEX) << 48) + 1)
#define DIM_ATTRIB 256              // Number of internal attribute words

#include <com/ifmacro.h>            // After #define HCDM
#include "DbBase.i"                 // After #define HCDM

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Single thread latch
static uint64_t        insertIX= 0; // Insert index (Barrier protected)

static const char      unit[8]= {0,0,0,0,0,0,0,1}; // Constant one
static const uint64_t  zero= 0;     // Constant zero
static Dbt             uDbt((void*)unit, sizeof(unit)); // Constant unit Dbt
static Dbt             zDbt((void*)&zero, sizeof(zero)); // Constant zero Dbt

//----------------------------------------------------------------------------
//
// Struct-
//       Attribute
//
// Purpose-
//       The Attribute key/value structure.
//
//----------------------------------------------------------------------------
struct Attribute {                  // The Attribute key/value structure
uint32_t               key;         // Key (attribute)
uint64_t               value;       // Value (or link)
}; // struct Attribute

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
   uint64_t index= DbBase::fetch64((uint64_t*)xDbt->get_data());
   if( index <= 1 )
     return DB_DONOTINDEX;

   sDbt->set_data(vDbt->get_data());// The secondary key *IS* the value
   sDbt->set_size(vDbt->get_size());

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::~DbAttr
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DbAttr::~DbAttr( void )          // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::DbAttr
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DbAttr::DbAttr( void )           // Default constructor
:  DbBase()
,  dbAssoc(NULL), dbValue(NULL), ixValue(NULL)
{
   int                 rc;

   // Open the database and indexes
   {{{{
   DbTxn* dbTxn= NULL;
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   dbAssoc= new Db(dbEnv, 0);
   dbValue= new Db(dbEnv, 0);
   ixValue= new Db(dbEnv, 0);
// ixValue->set_flags(DB_DUP);      // Allow duplicates (NOT)

   // Open the databases, making the handles thread-safe
   uint32_t flags= DB_CREATE | DB_THREAD;
   dbValue->open(dbTxn, "DbAttr.db", NULL, DB_BTREE, flags, 0);
   ixValue->open(dbTxn, "DbAttr_value.ix", NULL, DB_BTREE, flags, 0);
   dbAssoc->open(dbTxn, "DbAttr_assoc.db", NULL, DB_BTREE, flags, 0);
   dbValue->associate(dbTxn, ixValue, setSecondary, 0);

   dbTxn->commit(0);
   }}}}

   //-------------------------------------------------------------------------
   // Get the current insert index
   {{{{
   AutoBarrier lock(barrier);       // Index initialization is protected

   if( insertIX == 0 )              // If not already initialized
   {
     uint64_t xBuff;

     Dbt xKey(&xBuff, sizeof(xBuff));
     Dbt vRet;

     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dbv= NULL;                // Create cursor
     dbValue->cursor(dbTxn, &dbv, 0);

     rc= dbv->get(&uDbt, &vRet, DB_SET); // Get insert key
     DBDEBUG(rc, "dbc->get");
     if( rc != 0 )                  // If the insert key is missing
     {
       //---------------------------------------------------------------------
       // We need to create both the FIRST and the ALLOC index entries
       rc= dbValue->put(dbTxn, &zDbt, &zDbt, DB_NOOVERWRITE);
       DBDEBUG(rc, "db->put");
       if( rc == 0 )
       {
         store64(&xBuff, INITIAL_INDEX); // Create ALLOC
         rc= dbValue->put(dbTxn, &uDbt, &xKey, DB_NOOVERWRITE);
         DBDEBUG(rc, "db->put");
       }

       if( rc != 0 )
       {
         dbv->close();
         dbTxn->abort();

         reset();
         checkstop(__FILE__, __LINE__, "rc(%d)", rc);
       }

       insertIX= INITIAL_INDEX;
     }
     else
     {
       if( vRet.get_size() != sizeof(uint64_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

       insertIX= fetch64((uint64_t*)vRet.get_data());
     }

     dbv->close();
     dbTxn->commit(0);
   }
   }}}}

   //-------------------------------------------------------------------------
   // We need a starting point for the value index search
   if( insertIX == INITIAL_INDEX )
   {
     if( insert(0) == 0 )
       checkstop(__FILE__, __LINE__, "Unable to create initial index");
   }


}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::getAssoc
//
// Purpose-
//       Get associated link for index/key.
//
//----------------------------------------------------------------------------
uint64_t                            // The association
   DbAttr::getAssoc(                // Get association
     uint64_t          index,       // For this index
     unsigned          key)         // And this key
{
   uint64_t            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated information.
   if( index > 1 && index != INITIAL_INDEX )
   {
     store64(&xBuff, index);        // Set the index

     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dba= NULL;                // Create cursor
     dbAssoc->cursor(dbTxn, &dba, 0);

     // Get the current map
     rc= dba->get(&xDbt, &vRet, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( (vRet.get_size() % sizeof(Attribute)) != 0 )
         checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

       // The map exists, get the associated key
       unsigned count= vRet.get_size()/sizeof(Attribute);
       Attribute* attr= (Attribute*)vRet.get_data();
       for(unsigned i= 0; i<count; i++)
       {
         if( key == attr->key )
         {
           result= attr->value;
           break;
         }

         attr++;
       }
     }

     dba->close();
     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::getRecord
//
// Purpose-
//       Get all associated links for index.
//
//----------------------------------------------------------------------------
unsigned                            // The resultant size
   DbAttr::getRecord(               // Get association record
     uint64_t          index,       // For this index
     void*             addr,        // -> Resultant record (Attribute array)
     unsigned          size)        // Maximum resultant size (bytes)
{
   unsigned            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated information.
   if( index > 1 && index != INITIAL_INDEX )
   {
     store64(&xBuff, index);        // Set the index

     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dba= NULL;                // Create cursor
     dbAssoc->cursor(dbTxn, &dba, 0);

     // Get the current map
     rc= dba->get(&xDbt, &vRet, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       result= vRet.get_size();
       if( (result % sizeof(Attribute)) != 0 )
         checkstop(__FILE__, __LINE__, "size(%d)", result);

       if( result < size )
         size= result;
       memcpy(addr, vRet.get_data(), size);
     }

     dba->close();
     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::getValue
//
// Purpose-
//       Get value for index.
//
//----------------------------------------------------------------------------
uint64_t                            // The value (0 if error/missing)
   DbAttr::getValue(                // Get value
     uint64_t          index)       // For this index
{
   uint64_t            result= 0;   // Resultant
   uint64_t            xBuff;       // Index buffer

   Dbt                 xKey(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the associated information.
   if( index > 1 && index != INITIAL_INDEX )
   {
     store64(&xBuff, index);        // Set the index

     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dbv= NULL;                // Create cursor
     dbValue->cursor(dbTxn, &dbv, 0);

     // Get the current map
     rc= dbv->get(&xKey, &vRet, DB_SET);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( vRet.get_size() != sizeof(uint64_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

       // The value exists, get the associated index
       result= fetch64((uint64_t*)vRet.get_data());
     }

     dbv->close();
     dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::setAssoc
//
// Purpose-
//       Set assocation with index.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DbAttr::setAssoc(                // Set association
     uint64_t          index,       // For this index
     unsigned          key,         // The association type
     uint64_t          assoc,       // The association link or value
     DbTxn*            parent)      // Parent transaction
{
   (void)parent;                    // (Unused)
   int                 result= (-1);// Resultant
   Attribute           aBuff[DIM_ATTRIB]; // Association buffer
   uint64_t            xBuff;       // Index buffer

   Dbt                 xDbt(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vDbt;        // Value Dbt
   Dbt                 vRet;        // Value Dbt (Returned)

   unsigned            i;
   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, update the associated information.
   if( index > 1 && index != INITIAL_INDEX )
   {
     store64(&xBuff, index);        // Set the index

     DbTxn* dbTxn= NULL;            // Create transaction
     dbEnv->txn_begin(NULL, &dbTxn, 0);

     Dbc* dba= NULL;                // Create cursor
     dbAssoc->cursor(dbTxn, &dba, 0);

     // Get the current association
     rc= dba->get(&xDbt, &vRet, DB_SET | DB_RMW);
     DBDEBUG(rc, "dbc->get");
     if( rc != 0 )
     {
       Attribute* attr= aBuff;
       vRet.set_data(attr);
       vRet.set_size(sizeof(Attribute));

       attr[0].key= key;
       attr[0].value= assoc;

       rc= dbAssoc->put(dbTxn, &xDbt, &vRet, DB_NOOVERWRITE);
       DBDEBUG(rc, "dbc->put");
       if( rc == 0 )
         result= 0;
     }
     else
     {
       DBDEBUG(sizeof(Attribute), "sizeof(Attribute)\n");
       if( (vRet.get_size() % sizeof(Attribute)) != 0 )
         checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

       // The map exists, update or create the associated key
       unsigned count= vRet.get_size()/sizeof(Attribute);
       Attribute* attr= (Attribute*)vRet.get_data();
       for(i= 0; i<count; i++)
       {
         if( key == attr[i].key )
           break;
       }

       attr= aBuff;
       if( count >= (DIM_ATTRIB-1) )
         attr= (Attribute*)malloc(vRet.get_size() + sizeof(Attribute));

       if( attr != NULL )
       {
         memcpy(attr, vRet.get_data(), vRet.get_size());

         attr[i].key= key;
         attr[i].value= assoc;

         vDbt= vRet;
         vDbt.set_data(attr);
         if( i >= count )
           vDbt.set_size(vRet.get_size() + sizeof(Attribute));

         rc= dba->put(&xDbt, &vDbt, DB_OVERWRITE_DUP);
         DBDEBUG(rc, "dbc->get");
         if( rc == 0 )
           result= 0;

         if( count >= (DIM_ATTRIB-1) )
           free(vDbt.get_data());
       }
     }

     dba->close();
     if( result != 0 )
       dbTxn->abort();
     else
       dbTxn->commit(0);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::insert
//
// Purpose-
//       Insert into database.
//
//----------------------------------------------------------------------------
uint64_t                            // The index (0 if error)
   DbAttr::insert(                  // Insert into database
     uint64_t          value,       // This value, controlled by
     DbTxn*            parent)      // Parent transaction
{
   uint64_t            result= 0;   // Resultant
   uint64_t            vBuff;       // Value buffer
   uint64_t            xBuff;       // Index buffer

   Dbt                 vKey(&vBuff, sizeof(uint64_t)); // Value Dbt
   Dbt                 xKey(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)
   Dbt                 ignore;      // Ignored (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // Thread latch
   AutoBarrier lock(barrier);       // Thread latch required (for index)

   store64(&vBuff, value);
   store64(&xBuff, insertIX);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(parent, &dbTxn, 0);

   Dbc* dbv= NULL;                  // Create cursor
   ixValue->cursor(dbTxn, &dbv, 0);

   // Get the current map
   rc= dbv->pget(&vKey, &vRet, &ignore, DB_SET);
   DBDEBUG(rc, "dbc->pget");
   if( rc == 0 )
   {
     if( vRet.get_size() != sizeof(uint64_t) )
       checkstop(__FILE__, __LINE__, "size(%d)", vRet.get_size());

     // The value exists, get the associated index
     result= fetch64((uint64_t*)vRet.get_data());
   }
   else
   {
     // Write the database record
     rc= dbValue->put(dbTxn, &xKey, &vKey, DB_NOOVERWRITE);
     DBDEBUG(rc, "db->put");
     if( rc == 0 )
     {
       store64(&xBuff, insertIX+1);
       rc= dbValue->put(dbTxn, &uDbt, &xKey, DB_OVERWRITE_DUP);
       DBDEBUG(rc, "db->put");
     }

     if( rc == 0 )
     {
       result= insertIX;
       insertIX++;
     }
   }

   dbv->close();
   if( result == 0 )
     dbTxn->abort();
   else
     dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::nextIndex
//
// Purpose-
//       Get next database content index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbAttr::nextIndex(               // Get next index
     uint64_t          index)       // After this index
{
   uint64_t            result= 0;   // Resultant

   uint64_t            xBuff;       // Index buffer
   Dbt                 xKey(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the next index.
   if( index <= 1 )                 // For index 0, use INITIAL_INDEX
     index= INITIAL_INDEX;

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbv= NULL;                  // Create cursor
   dbValue->cursor(dbTxn, &dbv, 0);

   store64(&xBuff, index);          // Current index
   rc= dbv->get(&xKey, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbv->get(&xRet, &vRet, DB_NEXT);
     DBDEBUG(rc, "dbc->get");
     if( rc == 0 )
     {
       if( xRet.get_size() != sizeof(uint64_t) )
         checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

       result= fetch64((uint64_t*)xRet.get_data());
     }
   }

   dbv->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::nextValue
//
// Purpose-
//       Get next value index
//
//----------------------------------------------------------------------------
uint64_t                            // Result (0 if error/missing)
   DbAttr::nextValue(               // Get next value index
     uint64_t          index)       // After this index
{
   uint64_t            result= 0;   // Resultant

   uint64_t            xBuff;       // Index buffer
   Dbt                 pRet;        // Index Dbt (returned)
   Dbt                 xKey(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 xRet;        // Index Dbt (returned)
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   //-------------------------------------------------------------------------
   // If the index is mapped, return the next index.
   if( index <= 1 )                 // For index 0, use INITIAL_INDEX
     index= INITIAL_INDEX;

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* x2v= NULL;                  // Create cursors
   Dbc* v2x= NULL;
   dbValue->cursor(dbTxn, &x2v, 0);
   ixValue->cursor(dbTxn, &v2x, 0);

   store64(&xBuff, index);        // Current index
   rc= x2v->get(&xKey, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     if( vRet.get_size() > sizeof(uint64_t) )
       vRet.set_size(sizeof(uint64_t));
     rc= v2x->pget(&vRet, &xKey, &pRet, DB_GET_BOTH);
     DBDEBUG(rc, "dbc->pget");
     if( rc == 0 )
     {
       rc= v2x->pget(&vRet, &xRet, &pRet, DB_NEXT);
       DBDEBUG(rc, "dbc->pget");
       if( rc == 0 )
       {
         if( xRet.get_size() != sizeof(uint64_t) )
           checkstop(__FILE__, __LINE__, "size(%d)", xRet.get_size());

         result= fetch64((uint64_t*)xRet.get_data());
       }
     }
   }

   v2x->close();
   x2v->close();
   dbTxn->commit(0);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::remAssoc
//
// Purpose-
//       Remove assocation with index.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DbAttr::remAssoc(                // Remove association
     uint64_t          index,       // For this index
     unsigned          key)         // And this key
{
   (void)index; (void)key; return (-1); // NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       DbAttr::remove
//
// Purpose-
//       Delete content from database.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 if OK)
   DbAttr::remove(                  // Remove
     uint64_t          index)       // This content index
{
   uint64_t            xBuff;       // Index buffer

   Dbt                 xKey(&xBuff, sizeof(uint64_t)); // Index Dbt
   Dbt                 vRet;        // Value Dbt (returned)

   int                 rc;

   if( index <= 1 || index == INITIAL_INDEX )
     return (-1);

   DbTxn* dbTxn= NULL;              // Create transaction
   dbEnv->txn_begin(NULL, &dbTxn, 0);

   Dbc* dbi= NULL;                  // Create cursors
   Dbc* dba= NULL;
   dbAssoc->cursor(dbTxn, &dbi, 0);
   dbValue->cursor(dbTxn, &dba, 0);

   // Delete the current index
   store64(&xBuff, index);          // Set the index
   rc= dbi->get(&xKey, &vRet, DB_SET);
   DBDEBUG(rc, "dbc->get");
   if( rc == 0 )
   {
     rc= dbi->del(0);
     DBDEBUG(rc, "dbc->del");
   }

   if( rc == 0 )
   {
     rc= dba->get(&xKey, &vRet, DB_SET);
     DBDEBUG(rc, "dbc->get");

     if( rc != 0 )
       rc= 0;
     else
     {
       rc= dba->del(0);
       DBDEBUG(rc, "dbc->del");
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
//       DbAttr::reset
//
// Purpose-
//       Reset the database
//
//----------------------------------------------------------------------------
void
   DbAttr::reset( void )            // Reset the database
{
   // Create a checkpoint
   dbEnv->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( dbValue != NULL )
   {
     delete dbValue;
     dbValue= NULL;
   }

   if( ixValue != NULL )
   {
     delete ixValue;
     ixValue= NULL;
   }

   if( dbAssoc != NULL )
   {
     delete dbAssoc;
     dbAssoc= NULL;
   }
}

