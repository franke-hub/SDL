//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Reader.h
//
// Purpose-
//       Define the Reader object.
//
// Last change date-
//       2003/06/22
//
// Classes-
         class Reader;
//
// Description-
//       The Reader object is used to read a file, keeping track of
//       the line number.
//
//----------------------------------------------------------------------------
#ifndef READER_H_INCLUDED
#define READER_H_INCLUDED

#include "stdio.h"

//----------------------------------------------------------------------------
//
// Class-
//       Reader
//
// Purpose-
//       File reader which keeps track of the line number.
//
//----------------------------------------------------------------------------
class Reader                        // Reader
{
//----------------------------------------------------------------------------
// Reader::Constructors
//----------------------------------------------------------------------------
public:
   ~Reader( void );                 // Destructor
   Reader( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   Reader(const Reader&);           // Disallowed copy constructor
Reader&
   operator=(const Reader&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Reader::Accessor methods
//----------------------------------------------------------------------------
public:
inline const char*                  // The file name
   getFilename( void ) const;       // Get file name

inline long                         // The file line
   getLine( void ) const;           // Get file line

//----------------------------------------------------------------------------
// Reader::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   open(                            // Open the Reader
     const char*     fileName,      // The name of the File to be read
     unsigned        mode = 0);     // The associated mode (ignored)

int                                 // Return code (0 OK)
   close( void );                   // Close the Reader

int                                 // The line delimiter
   readLine(                        // Read a line
     char*           addr,          // Data address
     unsigned        size);         // Data length

//----------------------------------------------------------------------------
// Reader::Attributes
//----------------------------------------------------------------------------
protected:
   const char*       fileName;      // The file name
   FILE*             fileHandle;    // The file handle

   long              fileLine;      // The file line number
}; // class Reader

#include "Reader.i"

#endif // READER_H_INCLUDED
