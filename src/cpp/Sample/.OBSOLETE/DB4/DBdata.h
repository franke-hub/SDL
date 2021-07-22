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
//       DBdata.h
//
// Purpose-
//       Master database directory record
//
// Last change date-
//       2010/01/01
//
// Identification-
//       NAME: perm/DBdata.db
//       NAME: perm/DBdata_IXname.db
//       NAME: perm/DBdata_IXprog.db
//       PROG: src/cpp/DB4/DBdata.cpp
//       THIS: src/cpp/DB4/DBdata.h
//
// Notes-
//       The primary database, perm/DBdata.db, is indexed using the THIS:
//       line in the Identification section. Duplicates are not allowed.
//       Name: (The remainder of the THIS: line.)
//       Data: (The entire include file.)
//
//       The secondary database, perm/DBdata_IXprog.db, is indexed using the
//       PROG: line in the Identification section. Duplicates are allowed.
//       Name: (The remainder of the PROG: line.)
//       Data: (The entire include file.)
//
//       The tertiary database, perm/DBdata_IXname.db, is indexed using each
//       NAME: line in the Identification section. Duplicates are not allowed.
//       Name: (The remainder of the NAME: line.)
//       Data: (The entire include file.)
//
//       IMPLEMENTATION LIMIT: 16 NAME: identifiers.
//       Note: no other identifier is recognized after the THIS: identifier.
//
//----------------------------------------------------------------------------
#ifndef DBDATA_H_INCLUDED
#define DBDATA_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       DBdata
//
// Purpose-
//       Master database directory record.
//
//----------------------------------------------------------------------------
class DBdata {                      // Master database directory record
//----------------------------------------------------------------------------
// DBdata::Constructors
//----------------------------------------------------------------------------
public:
   ~DBdata( void );                 // Destructor
   DBdata( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DBdata(const DBdata&);           // Disallowed copy constructor
   DBdata& operator=(const DBdata&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DBdata::Constants
//----------------------------------------------------------------------------
public:
static const char*     DATABASE_PATH; // The database environment path

//----------------------------------------------------------------------------
// DBdata::Methods
//----------------------------------------------------------------------------
public:
const char*                         // (Current) database descriptor
   getDesc( void );                 // Get database descriptor

const char*                         // (Current) database name
   getName(                         // Get database name
     char*             result);     // Resultant, length > FILENAME_MAX

const char*                         // (Current) database program
   getProg(                         // Get database program
     char*             result);     // Resultant, length > FILENAME_MAX

void
   first( void );                   // Position at first record

void
   insert(                          // Insert (replace) database descriptor
     const char*       name,        // The database descriptor name
     const char*       desc);       // The database descriptor

void
   locateName(                      // Locate database descriptor
     const char*       name);       // Using this database name

const char*                         // (Current) database descriptor
   next( void );                    // Position at next NAME

void
   remove(                          // Remove (delete) database descriptor
     const char*       name);       // The database descriptor name

void
   reset( void );                   // Reset [close] the DBdata

//----------------------------------------------------------------------------
// DBdata::Attributes
//----------------------------------------------------------------------------
private:
void*                  object;      // The DBdata object
}; // class DBdata

#endif // DBDATA_H_INCLUDED
