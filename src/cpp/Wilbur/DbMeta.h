//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbMeta.h
//
// Purpose-
//       The list of databases.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DBMETA_H_INCLUDED
#define DBMETA_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DbAttr;
class DbFile;
class DbHttp;
class DbNada;
class DbRDF3;
class DbRDF4;
class DbText;
class DbTime;
class DbWord;

//----------------------------------------------------------------------------
//
// Class-
//       DbMeta
//
// Purpose-
//       The meta dictionary database.
//       This is a dictionary of dictionaries, used for global cross-reference.
//
// Usage notes-
//       The DbMeta is a container for databases. It is not a database.
//
// Implementation notes-
//       The high-order 16 bits of a database index is the index into the
//       database array contiained in this list.
//
//----------------------------------------------------------------------------
class DbMeta : public DbBase {      // The meta dictionary database
//----------------------------------------------------------------------------
// Database objects
public:                             // PUBLIC access
DbWord*                dbWord;      // [0000] The DbWord database
DbNada*                db0001;      // [0001] (Reserved for expansion)
DbNada*                db0002;      // [0002] (Reserved for expansion)
DbRDF3*                dbRDF3;      // [0003] The DbRDF3 database
DbNada*                dbRDF4;      // [0004] Thd DbRDF4 database
DbNada*                db0005;      // [0005] (Reserved for expansion)
DbNada*                db0006;      // [0006] (Reserved for expansion)
DbNada*                db0007;      // [0007] (Reserved for expansion)
DbNada*                db0008;      // [0008] (Reserved for expansion)
DbNada*                db0009;      // [0009] (Reserved for expansion)
DbNada*                db000A;      // [000A] (Reserved for expansion)
DbNada*                db000B;      // [000B] (Reserved for expansion)
DbNada*                db000C;      // [000C] (Reserved for expansion)
DbNada*                db000D;      // [000D] (Reserved for expansion)
DbTime*                dbTime;      // [000E] The DbTime database
DbAttr*                dbAttr;      // [000F] The DbAttr database
DbNada*                dbName;      // [0010] The DbName database
DbText*                dbText;      // [0011] The DbText database
DbFile*                dbFile;      // [0012] The DbFile database
DbHttp*                dbHttp;      // [0013] The DbHttp database
DbNada*                db0014;      // [0014] (Reserved for expansion)
DbNada*                db0015;      // [0015] (Reserved for expansion)
DbNada*                db0016;      // [0016] (Reserved for expansion)
DbNada*                db0017;      // [0017] (Reserved for expansion)
DbNada*                db0018;      // [0018] (Reserved for expansion)
DbNada*                db0019;      // [0019] (Reserved for expansion)
DbNada*                db001A;      // [001A] (Reserved for expansion)
DbNada*                db001B;      // [001B] (Reserved for expansion)
DbNada*                db001C;      // [001C] (Reserved for expansion)
DbNada*                db001D;      // [001D] (Reserved for expansion)
DbNada*                db001E;      // [001E] (Reserved for expansion)
DbNada*                db001F;      // [001F] (Reserved for expansion)
enum
{  DATABASE_COUNT= 32               // Number of database entries
}; // enum

//----------------------------------------------------------------------------
// DbMeta::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbMeta( void );                 // Destructor

protected:
   DbMeta( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbMeta(const DbMeta&);           // Disallowed copy constructor
   DbMeta& operator=(const DbMeta&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbMeta::Methods
//----------------------------------------------------------------------------
public:
static DbMeta*                      // The DbMeta singleton
   get( void );                     // Get DbMeta singleton
}; // class DbMeta

#endif // DBMETA_H_INCLUDED
