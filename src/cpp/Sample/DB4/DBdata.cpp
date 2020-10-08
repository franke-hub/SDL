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
//       DBdata.cpp
//
// Purpose-
//       Master database directory.
//
// Last change date-
//       2019/04/08
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <db_cxx.h>

#include <exception>
#include <iostream>
using namespace std;

#include <com/Debug.h>
#include <com/FileData.h>

#include "DBdata.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

#ifndef MAIN
#define MAIN                        // If defined, include mainline code
#endif

#ifndef MAX_NAME_INDEX
#define MAX_NAME_INDEX 16          // Maximum name index
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static const char*     headName= "perm/DBdata.db"; // Database name
static const char*     nameName= "perm/DBdata_IXname.db";  // Name    index name
static const char*     progName= "perm/DBdata_IXprog.db";  // Program index name
//atic const char*     owner= "src/cpp/DB4/DBdata.cpp"; // Registered owner

//----------------------------------------------------------------------------
// DBdata::Constants
//----------------------------------------------------------------------------
const char*            DBdata::DATABASE_PATH= "/database/";

//----------------------------------------------------------------------------
//
// Subroutine-
//       HCDM_printf
//
// Purpose-
//       If defined(HCDM), print
//
//----------------------------------------------------------------------------
static void
   HCDM_printf(                     // HCDM printf
     const char*       fmt,         // The printf format
                       ...)         // The printf arguments
{
   #ifdef HCDM
     va_list           argptr;      // Argument list pointer

     va_start(argptr, fmt);         // Initialize va_ functions
     vprintf(fmt, argptr);
     va_end(argptr);                // Close va_ functions
   #else                            // Parameter unused without HCDM defined
     (void)fmt;
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DBT_buffer
//
// Purpose-
//       Display DBT buffer (HCDM only)
//
//----------------------------------------------------------------------------
static inline void
   DBT_buffer(                      // Print DBT buffer
     const Dbt*        dbt)         // The DBT
{
   #ifdef HCDM
     char              buffer[64];  // Working buffer

     int size= dbt->get_size();
     if( size >= sizeof(buffer) )
       size= sizeof(buffer) - 1;

     memcpy(buffer, dbt->get_data(), size);
     buffer[size]= '\0';
     printf("%s", buffer);
   #else                            // Parameter unused without HCDM defined
     (void)dbt;
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DBT_debug
//
// Purpose-
//       Display DBT
//
//----------------------------------------------------------------------------
static inline void
   DBT_debug(                       // Print DBT itself
     const void*       dbtVoid)     // The DBT
{
   DBT* dbt= (DBT*)dbtVoid;

   printf("Dbt(%p)::debug()\n", dbt);
   printf(">>data(%p)\n", dbt->data);
   printf(">>size(%lu)\n", (unsigned long)dbt->size);
   printf(">>ulen(%lu)\n", (unsigned long)dbt->ulen);
   printf(">>dlen(%lu)\n", (unsigned long)dbt->dlen);
   printf(">>doff(%lu)\n", (unsigned long)dbt->doff);
   printf(">>dapp(%p)\n", dbt->app_data);
   printf(">>flag(0x%.8lx)\n", (unsigned long)dbt->flags);
}

//----------------------------------------------------------------------------
//
// Macro-
//       ZERO_OBJ, ZERO_PTR
//
// Purpose-
//       Clear a structure to zeroes
//
//----------------------------------------------------------------------------
#define ZERO_OBJ(x) memset((char*)&x, 0, sizeof(x))
#define ZERO_PTR(x) memset((char*)x, 0, sizeof(*x))

//----------------------------------------------------------------------------
//
// Class-
//       MyDB
//
// Purpose-
//       Db with working space for multiple Dbts
//
//----------------------------------------------------------------------------
class MyDB : public Db {            // Extended DB
public:
Dbt                    workDBT[MAX_NAME_INDEX]; // Working Dbt area

public:
virtual
   ~MyDB( void );                   // Destructor
   MyDB(                            // Constructor
     DbEnv*            enviro,      // Environment
     u_int32_t         flags);      // Flags
}; // struct MyDB

   MyDB::~MyDB( void ) { }          // Destructor
   MyDB::MyDB(                      // Constructor
     DbEnv*            enviro,      // Environment
     u_int32_t         flags)       // Flags
:  Db(enviro,flags) { }

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
DbEnv*                 dbEnvi;      // The environment
Db*                    dbHead;      // THIS database
Db*                    ixName;      // NAME database
Db*                    ixProg;      // PROG index database

Dbc*                   csName;      // The NAME (ixName) cursor
Dbt                    dbtK;        // Current key
Dbt                    dbtD;        // Current data
}; // struct Object

//----------------------------------------------------------------------------
//
// Struct-
//       HeadRecord
//
// Purpose-
//       Define a dbHead database record.
//
//----------------------------------------------------------------------------
struct HeadRecord {                 // Database master record
char*                  headName;    // The header file name
char*                  headFile;    // The header file

static int                          // Return code, 0 OK
   setProgKey(                      // Extract the PROG key
     const char*       desc,        // The header file
     Dbt*              sKey)        // Resultant
{
   const char* CO= strstr(desc, "PROG: ");
   if( CO == NULL )
     return 1;
   CO += 6;

   const char* CT= strchr(CO, '\n');
   if( CT == NULL )
     CT= CO+strlen(CO);
   if( CT[-1] == '\r' )
     CT--;

   unsigned size= CT-CO;
   sKey->set_data((char*)CO);
   sKey->set_size(size);
   sKey->set_flags(DB_DBT_DUPOK);

   #if( 0 )
     HCDM_printf("HeadRecord::setProgKey(");
     DBT_buffer(sKey);
     HCDM_printf(")\n");
   #endif

   return 0;
}
}; // struct HeadRecord

//----------------------------------------------------------------------------
//
// Struct-
//       NameRecord
//
// Purpose-
//       Define an ixName database record
//
//----------------------------------------------------------------------------
struct NameRecord {                 // Database name index record
char*                  nameItem;    // The NAME: item (database name)
char*                  headFile;    // The header file

static int                          // Return code, 0 OK
   setNameKey(                      // Extract the name key(s)
     Db*               db,          // -> DB
     const Dbt*        key,         // Key descriptor (unused)
     const Dbt*        data,        // Data descriptor
     Dbt*              sKey)        // Resultant
{
   (void)key;                       // Unused parameter
   int                 count= 0;    // Number of Multiple DBT

   char* CO= (char*)data->get_data();
   char* CM= strstr(CO, "THIS: ");
   if( CM == NULL )
     return 1;

   CO= strstr(CO, "NAME: ");
   if( CO == NULL || CO > CM )
     return 2;
   CO += 6;

   char* CT= strchr(CO, '\n');
   if( CT == NULL )
     CT= CO+strlen(CO);
   if( CT[-1] == '\r' )
     CT--;

   unsigned size= CT-CO;
   sKey->set_data(CO);
   sKey->set_size(size);

   HCDM_printf("NameRecord::setNameKey("); DBT_buffer(sKey); HCDM_printf(")\n");

   char* CN= strstr(CT, "NAME: ");
   if( CN != NULL && CN < CM )
   {
     MyDB* myDB= dynamic_cast<MyDB*>(db);
     if( myDB == NULL )
       fprintf(stderr, "NameRecord::setNameKey() dynamic_cast failure\n");
     else
     {
       memcpy((char*)&myDB->workDBT[0], sKey, sizeof(Dbt));

       for(count= 1; CN != NULL && CN < CM; count++)
       {
         if( count >= MAX_NAME_INDEX )
         {
           fprintf(stderr, "NameRecord::setNameKey() MAX_NAME_INDEX");
           break;
         }

         CO= CN + 6;
         CT= strchr(CO, '\n');
         if( CT == NULL )
           CT= CO+strlen(CO);
         if( CT[-1] == '\r' )
           CT--;

         size= CT-CO;
         sKey->set_data(CO);
         sKey->set_size(size);
         memcpy((char*)&myDB->workDBT[count], sKey, sizeof(Dbt));

         CN= strstr(CT, "NAME: ");
       }

       sKey->set_data(myDB->workDBT);
       sKey->set_size(count);
       sKey->set_flags(DB_DBT_MULTIPLE);
     }
   }

   return 0;
}
}; // struct HeadRecord

//----------------------------------------------------------------------------
//
// Struct-
//       ProgRecord
//
// Purpose-
//       Define an ixProg database record
//
//----------------------------------------------------------------------------
struct ProgRecord {                 // Database prog index record
char*                  progItem;    // The PROG: item (database owner)
char*                  headFile;    // The header file

static int                          // Return code, 0 OK
   setProgKey(                      // Extract the PROG key
     Db*               db,          // -> DB (unused)
     const Dbt*        key,         // Key descriptor (unused)
     const Dbt*        data,        // Data descriptor
     Dbt*              sKey)        // Resultant
{
   (void)db; (void)key;             // Unused parameters
   char* CO= (char*)data->get_data();
   char* CM= strstr(CO, "THIS: ");
   if( CM == NULL )
     return 1;

   CO= strstr(CO, "PROG: ");
   if( CO == NULL || CO > CM )
     return 2;
   CO += 6;

   char* CT= strchr(CO, '\n');
   if( CT == NULL )
     CT= CO+strlen(CO);
   if( CT[-1] == '\r' )
     CT--;

   unsigned size= CT-CO;
   sKey->set_data(CO);
   sKey->set_size(size);
   sKey->set_flags(DB_DBT_DUPOK);

   HCDM_printf("ProgRecord::setProgKey("); DBT_buffer(sKey); HCDM_printf(")\n");

   return 0;
}
}; // struct ProgRecord

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
     void*             object)      // -> Object
{
   Object* O= (Object*)object;      // Convert to Object*
   if( O == NULL )                  // If not initialized
   {
     u_int32_t         flags;       // FLAGS

     // Allocate the Object
     O= (Object*)malloc(sizeof(Object));
     if( O == NULL )
       throw "Storage shortage";
     ZERO_PTR(O);

     // Create the environment
     flags= DB_CREATE | DB_RECOVER | DB_REGISTER
          | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN;
     O->dbEnvi= new DbEnv(0);
     O->dbEnvi->set_tmp_dir("temp");
     O->dbEnvi->open(DBdata::DATABASE_PATH, flags, 0);

     // Create the database and indexes
     {{{{
     DbTxn* dbTxn= NULL;
     O->dbEnvi->txn_begin(NULL, &dbTxn, 0);

     O->dbHead= new Db(O->dbEnvi, 0);
     O->ixName= new MyDB(O->dbEnvi, 0);
     O->ixProg= new Db(O->dbEnvi, 0);

     // Allow duplicates in PROG database
     flags= DB_DUP;
     O->ixProg->set_flags(flags);

     // Open the databases
     flags= DB_CREATE;
     O->dbHead->open(dbTxn, headName, NULL, DB_BTREE, flags, 0);
     O->ixName->open(dbTxn, nameName, NULL, DB_BTREE, flags, 0);
     O->ixProg->open(dbTxn, progName, NULL, DB_BTREE, flags, 0);

     O->dbHead->associate(dbTxn, O->ixName, NameRecord::setNameKey, 0);
     O->dbHead->associate(dbTxn, O->ixProg, ProgRecord::setProgKey, 0);

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
static Object*                      // -> Object, always NULL
   dbTerm(                          // Terminate the database environment
     Object*           O)           // -> Object
{
   if( O != NULL )
   {
     // Close all the open cursors
     if( O->csName != NULL )
     {
       O->csName->close();
       O->csName= NULL;
     }

     // Create a checkpoint
     if( O->dbEnvi != NULL )
       O->dbEnvi->txn_checkpoint(0, 0, 0);

     // Close all the open databases
     if( O->dbHead != NULL )
     {
       delete O->dbHead;
       O->dbHead= NULL;
     }

     if( O->ixName != NULL )
     {
       delete O->ixName;
       O->ixName= NULL;
     }

     if( O->ixProg != NULL )
     {
       delete O->ixProg;
       O->ixProg= NULL;
     }

     // Close the environment
     if( O->dbEnvi != NULL )
     {
       delete O->dbEnvi;
       O->dbEnvi= NULL;
     }

     // Delete the object
     free(O);
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       first
//
// Function-
//       Position to the first NAME record
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   first(                           // Position to the first NAME record
     Object*           O)           // -> Object
{
   int                 rc;          // Called routine return code

   if( O->csName == NULL )
   {
     rc= O->ixName->cursor(NULL, &O->csName, 0);
     HCDM_printf("%d= ixName->cursor()\n", rc);
   }

   rc= O->csName->get(&O->dbtK, &O->dbtD, DB_FIRST);
   HCDM_printf("%d= csName->get(DB_FIRST)\n", rc);

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::~DBdata
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DBdata::~DBdata( void )          // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::DBdata
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DBdata::DBdata( void )           // Constructor
:  object(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::getDesc
//
// Purpose-
//       Get (current) database descriptor.
//
//----------------------------------------------------------------------------
const char*                         // (Current) database descriptor
   DBdata::getDesc( void )          // Get database descriptor
{
   object= dbInit(object);
   Object* O= (Object*)object;

   if( O->csName == NULL )
     ::first(O);

   if( O->csName->get(&O->dbtK, &O->dbtD, DB_CURRENT) != 0 )
     return NULL;

   return (char*)O->dbtD.get_data();
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::getName
//
// Purpose-
//       Get database name.
//
//----------------------------------------------------------------------------
const char*                         // (Current) database name
   DBdata::getName(                 // Get database name
    char*              result)      // Result, length > FILENAME_MAX
{
   result[0]= '\0';
   object= dbInit(object);
   Object* O= (Object*)object;

   if( O->csName == NULL )
     ::first(O);

   if( O->csName->get(&O->dbtK, &O->dbtD, DB_CURRENT) != 0 )
     return NULL;

   unsigned size= O->dbtK.get_size();
   if( size > FILENAME_MAX )
     size= FILENAME_MAX;
   memcpy(result, O->dbtK.get_data(), size);
   result[size]= '\0';

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::getProg
//
// Purpose-
//       Get database owner.
//
//----------------------------------------------------------------------------
const char*                         // (Current) database program
   DBdata::getProg(                 // Get database program
    char*              result)      // Result, length > FILENAME_MAX
{
   result[0]= '\0';
   const char* desc= getDesc();
   if( desc == NULL )
     return NULL;

   Dbt prog;
   if( HeadRecord::setProgKey(desc, &prog) != 0 )
     return NULL;

   int size= prog.get_size();
   if( size >= FILENAME_MAX )
     return NULL;

   memcpy(result, prog.get_data(), size);
   result[size]= '\0';

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::first
//
// Purpose-
//       Position at the first NAME of the first record.
//
//----------------------------------------------------------------------------
void
   DBdata::first( void )            // Get first database name
{
   object= dbInit(object);
   Object* O= (Object*)object;

   ::first(O);
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::insert
//
// Purpose-
//       Insert (replace) database descriptor
//
//----------------------------------------------------------------------------
void
   DBdata::insert(                  // Insert (replace) database descriptor
     const char*       name,        // The database descriptor name
     const char*       desc)        // The database descriptor
{
   int                 rc;          // Called routine return code

   object= dbInit(object);
   Object* O= (Object*)object;

   Dbc*   cursor= NULL;             // Working cursor
   DbTxn* intran= NULL;             // Working transaction
   Dbt    key, data;                // The key, data pair

   ZERO_OBJ(key);
   key.set_data((char*)name);
   key.set_size(strlen(name));
   O->dbEnvi->txn_begin(NULL, &intran, 0);
   O->dbHead->cursor(intran, &cursor, 0); // OPEN the cursor
   try {
     rc= cursor->get(&key, &data, DB_SET);
     HCDM_printf("%4d %d= get(%s)\n", __LINE__, rc, name);

     if( rc == 0 )
     {
       Dbc* delCursor= NULL;
       cursor->dup(&delCursor, DB_POSITION);
       rc= delCursor->del(0);
       HCDM_printf("%4d %d= del(%s)\n", __LINE__, rc, name);
       delCursor->close();
     }

     ZERO_OBJ(data);
     data.set_data((char*)desc);
     data.set_size(strlen(desc));
     rc= cursor->put(&key, &data, DB_KEYFIRST);
     HCDM_printf("%4d %d= put(%s)\n", __LINE__, rc, name);

     if( rc != 0 )
       throw "INSERT FAILURE";

     cursor->close();
     intran->commit(0);
   } catch(exception& x) {          // STL exception
     cerr << "STL exception: " << x.what() << endl;
     cursor->close();
     intran->abort();
   } catch(const char* x) {         // CHAR* exception
     cerr << "USR exception: " << x << endl;
     cursor->close();
     intran->abort();
   } catch(...) {                   // Other exception
     cerr << "SYSTEM exception" << endl;
     cursor->close();
     intran->abort();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::locateName
//
// Purpose-
//       Position at the specified NAME record.
//
//----------------------------------------------------------------------------
void
   DBdata::locateName(              // Locate database descriptor
     const char*       name)        // Using this database name
{
   // TODO: NEEDS WORK
   (void)name; throw "NOT CODED YET";
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::next
//
// Purpose-
//       Position at the next NAME record.
//
//----------------------------------------------------------------------------
const char*                         // The database descriptor, or NULL
   DBdata::next( void )             // Position at next NAME
{
   object= dbInit(object);
   Object* O= (Object*)object;

   if( O->csName == NULL )
     ::first(O);

   int rc= O->csName->get(&O->dbtK, &O->dbtD, DB_NEXT);
   HCDM_printf("%d %d= csName->get(DB_NEXT)\n", __LINE__, rc);

   const char* result= NULL;
   if( rc == 0 )
     result= (const char*)O->dbtD.get_data();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::remove
//
// Purpose-
//       Remove (delete) database descriptor
//
//----------------------------------------------------------------------------
void
   DBdata::remove(                  // Remove (delete) database descriptor
     const char*       name)        // The database descriptor name
{
   int                 rc;          // Called routine return code

   object= dbInit(object);
   Object* O= (Object*)object;

   Dbt    key;                      // The key descriptor

   ZERO_OBJ(key);
   key.set_data((char*)name);
   key.set_size(strlen(name));
   try {
     rc= O->dbHead->del(NULL, &key, 0);
     HCDM_printf("%4d %d= del(%s)\n", __LINE__, rc, name);
   } catch(exception& x) {          // STL exception
     cerr << "STL exception: " << x.what() << endl;
   } catch(const char* x) {         // CHAR* exception
     cerr << "USR exception: " << x << endl;
   } catch(...) {                   // Other exception
     cerr << "SYSTEM exception" << endl;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DBdata::reset
//
// Purpose-
//       Reset [close] the DBdata.
//
//----------------------------------------------------------------------------
void
   DBdata::reset( void )            // Reset [close] the DBdata
{
   object= dbTerm((Object*)object);
}

#ifdef MAIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       insertDatabase
//
// Function-
//       Insert database entry
//
//----------------------------------------------------------------------------
static void
   insertDatabase(                  // Insert database entry
      DBdata*          db,          // -> DBdata
      const char*      name)        // The database entry name
{
   printf("insertDatabase(%s)\n", name);

   FileData fileData(name);
   const char* desc= (const char*)fileData.getFileAddr();
   if( desc == NULL || strlen(desc) != fileData.getFileSize() )
   {
     fprintf(stderr, "Unable to load(%s) %s\n", name,
                     desc == NULL ? "Non-existent" : "Not a text file");
     return;
   }

   //-------------------------------------------------------------------------
   // Remove the first path qualifier from the name
   const char* CC= strchr(name, '/');
   if( CC == NULL )
     CC= name;
   else
     CC += 1;

   db->insert(CC, desc);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       removeDatabase
//
// Function-
//       Remove database entry
//
//----------------------------------------------------------------------------
static void
   removeDatabase(                  // Insert database entry
      DBdata*          db,          // -> DBdata
      const char*      name)        // The database entry name
{
   printf("removetDatabase(%s)\n", name);

   //-------------------------------------------------------------------------
   // Remove the first path qualifier from the name
   const char* CC= strchr(name, '/');
   if( CC == NULL )
     CC= name;
   else
     CC += 1;

   db->remove(CC);
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nameIterator
//
// Function-
//       Iterate the names
//
//----------------------------------------------------------------------------
static void
   nameIterator(                    // Iterate the NAME index
     DBdata*           db)          // -> DBdata
{
   printf("DBdata names:\n");

   db->first();
   for(;;)
   {
     char nmBuff[FILENAME_MAX+1];   // Result buffer
     const char* name= db->getName(nmBuff);
     if( name == NULL )
       break;

     char pgBuff[FILENAME_MAX+1];   // Result buffer
     const char* prog= db->getProg(pgBuff);
     printf("%s(%s)\n", name, prog);
     if( db->next() == NULL )
       break;
   }

   printf("\n");
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
   main(                            // Mainline code
      int              argc,        // Argument count
      char*            argv[])      // Argument array
{
   DBdata*             db= NULL;    // -> DBdata object

   //-------------------------------------------------------------------------
   // Operate, handling exceptions
   //-------------------------------------------------------------------------
   try {
     db= new DBdata();

     for(int argx= 1; argx<argc; argx++)
     {
       const char* CC= argv[argx];
       if( CC[0] == '-' && CC[1] == '-' )
         CC++;

       if( strcmp(CC, "-list") == 0 )
         nameIterator(db);
       else if( strcmp(CC, "-insert") == 0 )
         insertDatabase(db, argv[++argx]);
       else if( strcmp(CC, "-remove") == 0 )
         removeDatabase(db, argv[++argx]);
       else
       {
         fprintf(stderr, "Invalid parameter(%s)\n\n", CC);
         printf("%s {cmd ... }\n"
                "Where cmd is one of:\n"
                "-insert name (Load database entry[name])\n"
                "-list        (List database entries)\n"
                "-remove name (Remove database entry[name])\n"
                , argv[0]);
         throw "Invalid parameter";
       }
     }

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
     delete db;                     // Terminate
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
#endif // defined(MAIN)

