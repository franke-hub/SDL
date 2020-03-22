//----------------------------------------------------------------------------
//
//       Copyright (c) 2011 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbTime.h
//
// Purpose-
//       The timer event database.
//
// Last change date-
//       2011/01/01
//
//----------------------------------------------------------------------------
#ifndef DBTIME_H_INCLUDED
#define DBTIME_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct DbInfo;

//----------------------------------------------------------------------------
//
// Class-
//       DbTime
//
// Purpose-
//       The timer event database.
//
// Identification-
//       NAME: perm/Wilbur/DbTime.db
//       NAME: perm/Wilbur/DbTime.ix
//       PROG: src/cpp/Wilbur/DbTime.cpp
//       THIS: src/cpp/Wilbur/DbTime.h
//
// Implementation notes-
//       The value for this database is the Julian second that the event
//       should be driven. Since the timer event database is only checked
//       hourly, the event will be driven somewhat later than that time.
//
// Implementation notes (special entries)-
//       0x0000000000000000/{0xNNNNNNNNNNNNNNNN}  (FIRST/INSERT)
//
// Implementation notes-
//       struct DbTimeIndex {       // The DbTime index
//         uint64_t    index;       // The index (NETWORK format)
//       }; // struct DbTimeIndex
//
//       struct DbTimeValue {       // The DbTime value
//         uint64_t    xTime;       // The expiration time (NETWORK format)
//       }; // struct DbTimeValue
//
//       struct DbLayout {          // The DbTime value
//         uint64_t    index;       // The index (NETWORK format)
//         uint64_t    xTime;       // The xTime (NETWORK format)
//         DbInfo      assoc;       // Associated data
//       }; // struct DbTimeValue
//
//----------------------------------------------------------------------------
class DbTime : public DbBase {      // The name (string) database
//----------------------------------------------------------------------------
// DbTime::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbValue;     // INDEX to VALUE database
Db*                    ixValue;     // VALUE to INDEX database

//----------------------------------------------------------------------------
// DbTime::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 14               // High order 16 bits of uint64_t index
}; // enum

//----------------------------------------------------------------------------
// DbTime::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbTime( void );                 // Destructor
   DbTime( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbTime(const DbTime&);           // Disallowed copy constructor
   DbTime& operator=(const DbTime&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbTime::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The index (0 if error)
   insert(                          // Insert
     uint64_t          value,       // Expiration time (Julian second)
     DbInfo*           assoc,       // Associated information
     DbTxn*            parent= NULL); // Parent transaction

virtual int                         // Return code (0 if OK)
   locate(                          // Locate
     uint64_t          index,       // This index
     DbInfo*           assoc= NULL, // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value= NULL);// (OUT) Expiration time (Julian second)

virtual uint64_t                    // Result (0 if error/missing/last)
   nextIndex(                       // Get next index
     uint64_t          index);      // After this index

virtual uint64_t                    // Result (0 if error/missing/last)
   nextValue(                       // Get next value index
     uint64_t          index,       // After this index
     DbInfo*           assoc= NULL, // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value= NULL);// (OUT) Expiration time (Julian second)

virtual int                         // Return code (0 if OK)
   remove(                          // Remove
     uint64_t          index);      // This index

virtual int                         // Return code (0 if OK)
   replace(                         // Locate
     uint64_t          index,       // This index
     DbInfo*           assoc= NULL, // (OUT) Associated DbInfo (May be NULL)
     uint64_t*         value= NULL);// (OUT) Expiration time (Julian second)

protected:
void
   reset( void );                   // Reset the database
}; // class DbTime

#endif // DBTIME_H_INCLUDED
