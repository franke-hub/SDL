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
//       DbFile.h
//
// Purpose-
//       The FILE database, associating file names with permanent indexes.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DBFILE_H_INCLUDED
#define DBFILE_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbFile
//
// Purpose-
//       The FILE database.
//
// Identification-
//       NAME: Wilbur/DbFile.db
//       NAME: Wilbur/DbFile_ixFile.db
//       NAME: Wilbur/DbFile_ixLink.db
//       PROG: src/cpp/Wilbur/DbFile.cpp
//       THIS: src/cpp/Wilbur/DbFile.h
//
// Usage notes-
//       To remove associated links, use setAssoc(index, 0).
//
// Implementation notes-
//       <PRELIMINARY>
//
// Implementation notes (special entries)-
//       0x0000000000000000/0xNNNNNNNNNNNNNNNN" (FIRST/INSERT)
//
// Implementation notes-
//       struct DbFileIndex {       // The DbFile index
//         uint64_t    index;       // The File index (NETWORK format)
//       }; // struct DbFileIndex
//
//       struct DbFileValue {       // The DbFile value
//         char        value[1];    // The File name (w/o '\0' terminator)
//       }; // struct DbFileValue
//
//       struct DbFileAssoc {       // The DbFile association
//         uint64_t    assoc;       // The Link index (NETWORK format)
//       }; // struct DbFileAssoc
//
//----------------------------------------------------------------------------
class DbFile : public DbBase {      // The File database
//----------------------------------------------------------------------------
// DbFile::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbAssoc;     // INDEX to ASSOC database
Db*                    dbIndex;     // INDEX to VALUE database
Db*                    ixValue;     // VALUE to INDEX database

//----------------------------------------------------------------------------
// DbFile::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 18               // High order 16 bits of uint64_t index
,  MAX_VALUE_LENGTH= 4095           // (See usage notes)
}; // enum

//----------------------------------------------------------------------------
// DbFile::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbFile( void );                 // Destructor
   DbFile( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbFile(const DbFile&);           // Disallowed copy constructor
   DbFile& operator=(const DbFile&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbFile::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The association link
   getAssoc(                        // Get association link
     uint64_t          index);      // For this File index

virtual uint64_t                    // The File index (0 if error/missing)
   getIndex(                        // Get File index
     const char*       value);      // For this File value

inline uint64_t                     // The File index (0 if error/missing)
   getIndex(                        // Get File index
     std::string&      value)       // For this File value
{  return getIndex(value.c_str());
}

virtual char*                       // Result (NULL if error/missing)
   getValue(                        // Get File value
     uint64_t          index,       // For this File index
     char*             result);     // (OUTPUT) Result string

virtual int                         // Return code (0 if OK)
   setAssoc(                        // Set Association between
     uint64_t          index,       // This File index and
     uint64_t          assoc,       // This Link index, controlled by
     DbTxn*            dbTxn= NULL);// This transaction

virtual uint64_t                    // The File index (0 if error)
   insert(                          // Get File index (insert if missing)
     const char*       value,       // For this File value, controlled by
     DbTxn*            dbTxn= NULL);// This transaction

inline uint64_t                     // The File index (0 if error)
   insert(                          // Get File index (insert if missing)
     std::string&      value,       // For this File value, controlled by
     DbTxn*            dbTxn= NULL) // This transaction
{  return insert(value.c_str(), dbTxn);
}

virtual uint64_t                    // Result (0 if error/missing)
   nextIndex(                       // Get next File index
     uint64_t          index);      // After this File index

virtual char*                       // Result (NULL if error/missing)
   nextValue(                       // Get next File value
     char*             value);      // (IN/OUT) Current/Next File value

virtual int                         // Return code (0 if error)
   remove(                          // Remove
     uint64_t          index);      // This File index

protected:
void
   reset( void );                   // Reset the database
}; // class DbFile

#endif // DBFILE_H_INCLUDED
