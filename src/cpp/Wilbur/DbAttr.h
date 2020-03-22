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
//       DbAttr.h
//
// Purpose-
//       The attribute database.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DBATTR_H_INCLUDED
#define DBATTR_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbAttr
//
// Purpose-
//       The attribute database.
//
// Identification-
//       NAME: perm/Wilbur/DbAttr.db
//       NAME: perm/Wilbur/DbAttr_assoc.db
//       NAME: perm/Wilbur/DbAttr_value.ix
//       PROG: src/cpp/Wilbur/DbAttr.cpp
//       THIS: src/cpp/Wilbur/DbAttr.h
//
// Implementation notes-
//       <PRELIMINARY>
//
// Implementation notes (special entries)-
//       0x0000000000000000/{0x0000000000000000}  (FIRST)
//       0x0000000000000001/{0xNNNNNNNNNNNNNNNN}  (Next available)
//
// Implementation notes-
//       struct DbAttrIndex {       // The DbAttr index
//         uint64_t    index;       // The index (NETWORK format)
//       }; // struct DbAttrIndex
//
//       struct DbAttrValue {       // The DbAttr value {Reverse link}
//         uint64_t    value;       // The value
//       }; // struct DbAttrValue
//
//       struct Attribute {         // An attribute
//         uint32_t    atype;       // The attribute type
//         uint64_t    assoc;       // The associated link or value
//       }; // struct Attribute
//
//       struct DbAttrAssoc {       // The DbAttr assoc
//         Attribute   assoc[1];    // The attribute association array
//       }; // struct DbAttrAssoc
//
//----------------------------------------------------------------------------
class DbAttr : public DbBase {      // The name (string) database
//----------------------------------------------------------------------------
// DbAttr::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbAssoc;     // INDEX to ASSOC database
Db*                    dbValue;     // INDEX to VALUE database
Db*                    ixValue;     // VALUE to INDEX database

//----------------------------------------------------------------------------
// DbAttr::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 15               // High order 16 bits of uint64_t index
}; // enum

enum TYPE                           // Assocation type
{  TYPE_LINK                        //  0 Generic link
,  TYPE_TEXT                        //  1 Link to DbText
,  TYPE_FILE                        //  2 Link to DbFile
,  TYPE_HTTP                        //  3 Link to DbHttp
,  TYPE_CODE                        //  4 Generic numeric code
,  TYPE_TIME                        //  5 Generic time
,  TYPE_HAS                         //  6 Generic HAS (link) {Contains}
,  TYPE_ISA                         //  7 Generic ISA (link) {Identity}
,  TYPE_COUNT                       //  8 Number of defined types
}; // enum TYPE

//----------------------------------------------------------------------------
// DbAttr::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbAttr( void );                 // Destructor
   DbAttr( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbAttr(const DbAttr&);           // Disallowed copy constructor
   DbAttr& operator=(const DbAttr&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbAttr::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The association (0 if error or missing)
   getAssoc(                        // Get Association
     uint64_t          index,       // For this index and
     unsigned          atype);      // For this association type

virtual unsigned                    // The association record length
   getRecord(                       // Get Association Record
     uint64_t          index,       // For this index
     void*             addr,        // -> Resultant record (Attribute array)
     unsigned          size);       // Maximum resultant size (bytes)

virtual uint64_t                    // The value (0 if error or missing)
   getValue(                        // Get value
     uint64_t          index);      // For this index

virtual int                         // Result (0 if OK)
   setAssoc(                        // Set association
     uint64_t          index,       // For this index and
     unsigned          atype,       // For this association type
     uint64_t          assoc,       // The associated link or value, control by
     DbTxn*            dbTxn= NULL);// This transaction

virtual uint64_t                    // The index (0 if error)
   insert(                          // Insert
     uint64_t          value,       // This value, controlled by
     DbTxn*            dbTxn= NULL);// This transaction

virtual uint64_t                    // Result (0 if error/missing/last)
   nextIndex(                       // Get next index
     uint64_t          index);      // After this index

virtual uint64_t                    // Result (0 if error/missing/last)
   nextValue(                       // Get next value
     uint64_t          value);      // After this value

virtual int                         // Result (0 if OK)
   remAssoc(                        // Remove association
     uint64_t          index,       // For this index and
     unsigned          atype);      // For this association type

virtual int                         // Return code (0 if OK)
   remove(                          // Remove
     uint64_t          index);      // This index

protected:
void
   reset( void );                   // Reset the database
}; // class DbAttr

#endif // DBATTR_H_INCLUDED
