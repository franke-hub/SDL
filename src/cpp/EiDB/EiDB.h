//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EiDB.h
//
// Purpose-
//       Describe the Exon/Intron DataBase.
//
// Last change date-
//       2002/09/15
//
// Classes-
         class EiDB;                // Exon/Intron DataBase
//
//----------------------------------------------------------------------------
#ifndef EIDB_H_INCLUDED
#define EIDB_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       EiDB
//
// Purpose-
//       Describe the Exon/Intron DataBase.
//
//----------------------------------------------------------------------------
class EiDB                          // Exon/Intron DataBase
{
//----------------------------------------------------------------------------
// EiDB::Constructors
//----------------------------------------------------------------------------
public:
   ~EiDB( void );                   // Default destructor
   EiDB( void );                    // Default constructor

private:                            // Bitwise copy prohibited
   EiDB(const EiDB&);
   EiDB& operator=(const EiDB&);

//----------------------------------------------------------------------------
// EiDB::methods
//----------------------------------------------------------------------------
public:
const char*                         // -> Line
   getLine(                         // Get line from database
     unsigned          index);      // Line index

unsigned                            // Line index
   putLine(                         // Add line into database
     const char*       line);       // Line to add

unsigned                            // The largest line size
   getLargest( void ) const;        // Get largest line size

unsigned                            // The line count
   getLineCount( void ) const;      // Get line count

void
   empty( void );                   // Empty the database

int                                 // Number of lines released
   trim( void );                    // Remove lines the database
                                    // (For emergency use only)

//----------------------------------------------------------------------------
// EiDB::Attributes
//----------------------------------------------------------------------------
protected:
   unsigned            largest;     // Number of columns in largest line
   unsigned            lineCount;   // Number of input lines
   void*               headArray;   // Head of line array
   unsigned            workIndex;   // Working element index
   void*               workArray;   // Working line array
}; // class EiDB

#endif // EIDB_H_INCLUDED
