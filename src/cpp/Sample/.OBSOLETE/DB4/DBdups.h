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
//       DBdups.h
//
// Purpose-
//       Filename database descriptor
//
// Last change date-
//       2010/01/01
//
// Identification-
//       NAME: temp/DBdups.db
//       NAME: temp/DBdups_IXname.db
//       NAME: temp/DBdups_IXsize.db
//       PROG: src/cpp/Test/Db4/DBdups.cpp
//
//----------------------------------------------------------------------------
#ifndef DBDUPS_H_INCLUDED
#define DBDUPS_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct Object;                      // The hidden DBdups Object
struct Record;                      // The DBdups Record

//----------------------------------------------------------------------------
//
// Class-
//       DBdups
//
// Purpose-
//       Filename database descriptor
//
//----------------------------------------------------------------------------
class DBdups {                      // Filename database descriptor
//----------------------------------------------------------------------------
// DBdups::Constructors
//----------------------------------------------------------------------------
public:
   ~DBdups( void );                 // Destructor
   DBdups( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DBdups(const DBdups&);           // Disallowed copy constructor
   DBdups& operator=(const DBdups&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DBdups::Methods
//----------------------------------------------------------------------------
public:
void
   dbLoad(                          // Load the database
     const char*       path);       // Starting from this directory

void
   dbScan( void );                  // Display the resultant

void
   reset( void );                   // Reset [close] the database

//----------------------------------------------------------------------------
// DBdups::Attributes
//----------------------------------------------------------------------------
private:
Object*                object;      // The DBdups object
}; // class DBdups

#endif // DBDUPS_H_INCLUDED
