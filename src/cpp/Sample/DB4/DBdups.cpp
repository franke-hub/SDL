//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DBdups.cpp
//
// Purpose-
//       Util/FSdups.cpp written for DB4 for use as a transaction stress test
//
// Last change date-
//       2019/04/08
//
// Usage-
//       DBdups
//
//----------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include <exception>
#include <iostream>
#include <db_cxx.h>
using namespace std;

#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/FileData.h>
#include <com/FileInfo.h>
#include <com/FileList.h>
#include <com/FileName.h>
#include <com/nativeio.h>
#include <com/Network.h>
#include <com/syslib.h>
#include <com/Thread.h>

#include "DBdups.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static const char*     ePath= "/database/"; // Environment path
static const char*     dData= "temp/DBdups.db";  // Database name
static const char*     xName= "temp/DBdups_IxName.db";  // NAME index file name
static const char*     xSize= "temp/DBdups_IxSize.db";  // SIZE index file name

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             argc;       // Argument count
static char**          argv;       // Argument array

static ATOMIC32        recno=0;      // Record number, atomically updated

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       The hidden DBdata object
//
//----------------------------------------------------------------------------
struct Object {                     // The hidden DBdata object
DbEnv*                 dbEnv0;      // The environment
Db*                    dbData;      // The database
Db*                    ixName;      // NAME index database
Db*                    ixSize;      // SIZE index database
}; // struct Object

//----------------------------------------------------------------------------
//
// Struct-
//       Record
//
// Purpose-
//       Define a database record
//
// Indexes-
//       [1] The name of the file [without the path qualifier]
//       [2] The size of the file
//
//----------------------------------------------------------------------------
struct Record {                     // Database record
unsigned long          fileSize;    // The size of the file (in bytes)
char                   fileName[4096]; // The absolute name of the file

unsigned long                       // The file size
   getFileSize( void )              // Get file size
{
   return ntohl(fileSize);
}

void
   setFileSize(                     // Set file size
     unsigned long     fileSize)    // To this
{
   this->fileSize= htonl(fileSize);
}

static void
   debug(                           // Debugging information
     const Dbt*        sKey)        // Source Dbt*
{
  const DBT* dbt= sKey->get_const_DBT();
  printf("debugDBT(%p)\n", dbt);
  if( dbt != NULL )
  {
    printf("..data(%p)\n", dbt->data);
    printf("..size(%d)\n", dbt->size);
    printf("..ulen(%d)\n", dbt->ulen);
    printf("..dlen(%d)\n", dbt->dlen);
    printf("..doff(%d)\n", dbt->doff);
    printf("..appd(%p)\n", dbt->app_data);
    printf("..flag(0x%x)\n", dbt->flags);
    snap(dbt->data, dbt->size);
  }
}

static void
   debug(                           // Debugging information
     const Dbt&        sKey)        // Source Dbt&
{
  debug(&sKey);
}

static int                          // Return code, 0 OK
   setNameKey(                      // Extract the name key
     Db*                db,         // -> DB (unused)
     const Dbt*         pKey,       // Primary key descriptor (unused)
     const Dbt*         data,       // Data descriptor
     Dbt*               sKey)       // Resultant
{
// printf("Record::setNameKey()\n");

   Record* record= (Record*)data->get_data();
   sKey->set_data(record->fileName);
   sKey->set_size(strlen(record->fileName) + 1);
   sKey->set_flags(DB_DBT_DUPOK);

// printf("sKey: "); debug(sKey);
   return 0;
}

static int                          // Return code, 0 OK
   setSizeKey(                      // Extract the size key
     Db*                db,         // -> DB (unused)
     const Dbt*         pKey,       // Primary key descriptor (unused)
     const Dbt*         data,       // Data descriptor
     Dbt*               sKey)       // Resultant
{
// printf("Record::setSizeKey()\n");

   Record* record= (Record*)data->get_data();
   sKey->set_data(&record->fileSize);
   sKey->set_size(sizeof(record->fileSize));
   sKey->set_flags(DB_DBT_DUPOK);

// printf("sKey: "); debug(sKey);
   return 0;
}
}; // struct Record

//----------------------------------------------------------------------------
//
// Subroutine-
//       usage
//
// Function-
//       write usage information and exit
//
//----------------------------------------------------------------------------
static void
   usage(                           // Write usage information and exit
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   printf("Usage: %s {options} {.type ...}\n"
          "Search filesystem (from current directory) looking for duplicates\n"
          "Duplicate files are written to stdout\n"
          "\n"
          "Options:\n"
          "  <No options available>\n"
          , argv[0]);

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Function-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   int                 argx;        // Argument index

   for(argx= 1; argx<argc; argx++)
   {
     if( *argv[argx] != '-' )       // If not a switch
       break;                       // List of types found

     usage(argc, argv);
   }

   ::argc= argc-argx;
   ::argv= &argv[argx];

   if( ::argc != 0 )
     usage(argc, argv);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       xalloc
//
// Function-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void*                        // -> Allocated memory, never NULL
   xalloc(                          // Guaranteed allocation
      size_t           size)        // Of this length
{
   void* result= malloc(size);
   if( result == NULL )
     throw "Storage shortage";

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbInit
//
// Function-
//       Initialize the database environment
//
//----------------------------------------------------------------------------
static Object*                      // -> Object
   dbInit(                          // Initialize the database environment
     Object*           O)           // -> Object
{
   if( O == NULL )
   {
     u_int32_t         flags;       // FLAGS

     // Allocate the Object
     O= (Object*)xalloc(sizeof(Object));
     memset(O, 0, sizeof(*O));

     // Create the environment, making the handle thread-safe
     flags= DB_CREATE | DB_RECOVER | DB_REGISTER | DB_THREAD
          | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
     O->dbEnv0= new DbEnv(0);
     O->dbEnv0->set_data_dir("data");
     O->dbEnv0->set_tmp_dir("temp");
     O->dbEnv0->open(ePath, flags, 0);

     // Create the database and indexes
     {{{{
     DbTxn* dbTxn= NULL;
     O->dbEnv0->txn_begin(NULL, &dbTxn, 0);

     O->dbData= new Db(O->dbEnv0, 0);
     O->ixName= new Db(O->dbEnv0, 0);
     O->ixSize= new Db(O->dbEnv0, 0);

     // Allow duplicates in NAME and SIZE database
     flags= DB_DUP;
     O->ixName->set_flags(flags);
     O->ixSize->set_flags(flags);

     // Open the databases, making the handles thread-safe
     flags= DB_CREATE | DB_THREAD;
     O->dbData->open(dbTxn, dData, NULL, DB_RECNO, flags, 0);
     O->ixName->open(dbTxn, xName, NULL, DB_BTREE, flags, 0);
     O->ixSize->open(dbTxn, xSize, NULL, DB_BTREE, flags, 0);

     O->dbData->associate(dbTxn, O->ixName, Record::setNameKey, 0);
     O->dbData->associate(dbTxn, O->ixSize, Record::setSizeKey, 0);

     O->dbData->truncate(dbTxn, &flags, 0);

     dbTxn->commit(0);
     }}}}
   }

   return O;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dbTerm
//
// Function-
//       Terminate the database environment
//
//----------------------------------------------------------------------------
static void
   dbTerm(                          // Terminate the database environment
     Object&           O)           // Object&
{
   // Create a checkpoint
   if( O.dbEnv0 != NULL )
     O.dbEnv0->txn_checkpoint(0, 0, 0);

   // Delete all the open databases
   if( O.ixName != NULL )
   {
     delete O.ixName;
     O.ixName= NULL;
   }

   if( O.ixSize != NULL )
   {
     delete O.ixSize;
     O.ixSize= NULL;
   }

   if( O.dbData != NULL )
   {
     delete O.dbData;
     O.dbData= NULL;
   }

   // Delete the environment
   if( O.dbEnv0 != NULL )
   {
     delete O.dbEnv0;
     O.dbEnv0= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdups::dbLoad
//
// Function-
//       Load the database
//
//----------------------------------------------------------------------------
void
   DBdups::dbLoad(                  // Load the database
     const char*       path)        // Starting from this directory
{
// printf("dbLoad(%s)\n", path);    // Say hello

   Object& O= *object;

   u_int32_t          flags;        // FLAGS

   FileList fileList(path, "*");    // The (relative) directory
   for(;;fileList.getNext())        // Load database files
   {
     const char* name= fileList.getCurrent();
     if( name == NULL )
       break;

     if( strcmp(name, ".") == 0 || strcmp(name, "..") == 0 )
       continue;

     FileInfo fileInfo(path, name);
     if( !fileInfo.isLink() )       // If this is not a link
     {
       if( fileInfo.isFile()        // If this is a readable file
         && fileInfo.isReadable() )
       {
         Record record;
         record.setFileSize(fileInfo.getFileSize());
         FileName fileName(path, name);
         fileName.resolve();
         strcpy(record.fileName, fileName.getFileName());
//       printf("%8ld %s\n", record.getFileSize(), record.fileName);

         // Atomically assign a record number (See implementation notes)
         int cc;
         int32_t oldValue, newValue;
         do
         {
           oldValue= recno;
           newValue= oldValue + 1;
           cc= csw(&recno, oldValue, newValue);
         } while( cc != 0 );

         // Write the database record
         Dbt pKey(&newValue, sizeof(newValue));
         Dbt data(&record, sizeof(record.fileSize) + strlen(record.fileName) + 1);
         flags= DB_AUTO_COMMIT;
         O.dbData->put(NULL, &pKey, &data, flags);
       }
       else if( fileInfo.isPath()   // If this is a readable directory
         && fileInfo.isReadable() )
       {
         Record record;
         FileName fileName(path, name);
         fileName.resolve();
         strcpy(record.fileName, fileName.getFileName());
         dbLoad(record.fileName);
       }
     }
   }

// printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdups::~DBdups
//
// Function-
//       Destructor
//
//----------------------------------------------------------------------------
   DBdups::~DBdups( void )          // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdups::DBdups
//
// Function-
//       Constructor
//
//----------------------------------------------------------------------------
   DBdups::DBdups( void )           // Constructor
:  object(dbInit(NULL))
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdups::dbScan
//
// Function-
//       Scan the database [Single threaded mode]
//
//----------------------------------------------------------------------------
void
   DBdups::dbScan( void )           // Scan the database
{
// printf("dbScan()\n");

   Object& O= *object;

   int                 rc;          // Return code

   Dbc* iDbc= NULL;                 // Working cursor
   Dbt  iKey, iDat, pKey;           // Working data areas

   DbTxn* dbTxn= NULL;
   O.dbEnv0->txn_begin(NULL, &dbTxn, 0);
   O.ixSize->cursor(dbTxn, &iDbc, 0);  // OPEN the cursor
   while( (rc= iDbc->pget(&iKey, &pKey, &iDat, DB_NEXT)) == 0 )
   {
     Record* iRec= (Record*)iDat.get_data();
//   printf("%8ld %s\n", iRec->getFileSize(), iRec->fileName);
     FileData iFile(iRec->fileName);

     Dbc* jDbc= NULL;
     Dbt  jKey, jDat, dKey;
     iDbc->dup(&jDbc, DB_POSITION);
     while( (rc=jDbc->pget(&jKey, &dKey, &jDat, DB_NEXT)) == 0 )
     {
       Record* jRec= (Record*)jDat.get_data();
//     printf("..%8ld %s\n", jRec->getFileSize(), jRec->fileName);
       if( iRec->getFileSize() != jRec->getFileSize() )
         break;

       // Compare the files
       FileData jFile(jRec->fileName);
       if( iFile == jFile )
       {
         printf("%s == %s\n", iRec->fileName, jRec->fileName);

         Dbc* kDbc= NULL;
         jDbc->dup(&kDbc, DB_POSITION);
         kDbc->del(0);              // DELETE the record
         kDbc->close();
       }
     }

     jDbc->close();                 // CLOSE the secondary cursor
   }

   iDbc->close();                   // CLOSE the cursor
   dbTxn->commit(0);                // COMMIT the transaction
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdups::reset
//
// Function-
//       Close the database
//
//----------------------------------------------------------------------------
void
   DBdups::reset( void )            // Scan the database
{
   if( object != NULL )
     dbTerm(*object);

   free(object);
   object= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Function-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(                            // FSDUMP utility
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Argument anlaysis
   //-------------------------------------------------------------------------
   parm(argc, argv);                // Parameter analysis

   //-------------------------------------------------------------------------
   // Operate, handling exceptions
   //-------------------------------------------------------------------------
// set_terminate(__gnu_cxx::__verbose_terminate_handler);
   DBdups* dbDups= NULL;            // The DBdups object

   try {
     dbDups= new DBdups();          // Initialize the database
     dbDups->dbLoad(".");           // Load the database
     dbDups->dbScan();              // Scan the database
   } catch(DbException& x) {        // Database exception
     cerr << "DbException: " << x.what() << endl;
   } catch(exception& x) {          // STL exception
     cerr << "STL exception: " << x.what() << endl;
   } catch(const char* x) {         // CHAR* exception
     cerr << "USR exception: " << x << endl;
   } catch(...) {                   // Other exception
     cerr << "SYSTEM exception" << endl;
   }

   try {
     delete dbDups;
   } catch(DbException& x) {        // Database exception
     cerr << "DbException: " << x.what() << endl;
   } catch(exception& x) {          // STL exception
     cerr << "STL exception: " << x.what() << endl;
   } catch(const char* x) {         // CHAR* exception
     cerr << "USR exception: " << x << endl;
   } catch(...) {                   // Other exception
     cerr << "SYSTEM exception" << endl;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Implementation notes-
//   It probably would have been better to use either the file size or the
//   file name as one of the primary keys, but let's figure out the RECNO
//   access method while we're here. And, oh my:
//
// DB4 usage notes-
//   Db->put(,,,DB_APPEND) cannot be used with DB_THREAD
//      (This is why RECNO is atomically incremented)
//
//   dbScan runs as a (single) transaction. When it didn't, it wasn't
//   possible to delete the jCursor record. The operation would either
//   fail (running without a transaction) or deadlock (attempting to
//   wrap a transaction around the jCursor fetch and delete.) This might
//   have been avoided if the DB was closed and re-opened in single
//   thread mode, but this was not tried.
//
//----------------------------------------------------------------------------

