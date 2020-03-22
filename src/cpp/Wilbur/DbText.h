//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbText.h
//
// Purpose-
//       The TEXT content database.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef DBTEXT_H_INCLUDED
#define DBTEXT_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbText
//
// Purpose-
//       The text content database.
//
// Identification-
//       NAME: perm/Wilbur/DbText.db
//       PROG: src/cpp/Wilbur/DbText.cpp
//       THIS: src/cpp/Wilbur/DbText.h
//
// Implementation notes-
//       Index key 0 contains the next insert index key.
//
// Implementation notes-
//       struct DbTextIndex {       // The DbText index
//         uint64_t    index;       // The TEXT index (NETWORK format)
//       }; // struct DbTextIndex
//
//       struct DbTextValue {       // The DbText content
//         char        value[1];    // The content (including '\0' delimiter)
//       }; // struct DbTextValue
//
//----------------------------------------------------------------------------
class DbText : public DbBase {      // The name (string) database
//----------------------------------------------------------------------------
// DbText::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbIndex;     // INDEX to VALUE database

//----------------------------------------------------------------------------
// DbText::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 17               // High order 16 bits of uint64_t index
}; // enum

//----------------------------------------------------------------------------
// DbText::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbText( void );                 // Destructor
   DbText( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbText(const DbText&);           // Disallowed copy constructor
   DbText& operator=(const DbText&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbText::Methods
//----------------------------------------------------------------------------
public:
virtual char*                       // MALLOC result (NULL if error/missing)
   getValue(                        // Get content
     uint64_t          index);      // For this index

virtual uint64_t                    // The index (0 if error)
   insert(                          // Insert
     const char*       value,       // This content, controlled by
     DbTxn*            parent= NULL); // This transaction

inline uint64_t                     // The index (0 if error)
   insert(                          // Insert
     std::string&      value,       // This content, under control of
     DbTxn*            parent= NULL)// This transaction
{  return insert(value.c_str(), parent);
}

virtual uint64_t                    // Result (0 if error/missing/last)
   nextIndex(                       // Get next index
     uint64_t          index);      // After this index

virtual int                         // Return code (0 if OK)
   remove(                          // Remove
     uint64_t          index,       // This index, controlled by
     DbTxn*            parent= NULL); // This transaction

virtual int                         // Return code (0 if OK)
   revise(                          // Replace value
     uint64_t          index,       // For this index
     const char*       value,       // This content, controlled by
     DbTxn*            parent= NULL); // This transaction

inline int                          // Return code (0 if OK)
   revise(                          // Replace value
     uint64_t          index,       // For this index
     std::string&      value,       // This content, controlled by
     DbTxn*            parent= NULL)// This transaction
{  return revise(index, value.c_str(), parent);
}

protected:
void
   close( void );                   // Close the database
}; // class DbText

#endif // DBTEXT_H_INCLUDED
