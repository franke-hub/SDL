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
//       DbHttp.h
//
// Purpose-
//       The HTTP database.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef DBHTTP_H_INCLUDED
#define DBHTTP_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbHttp
//
// Purpose-
//       The HTTP database.
//
// Identification-
//       NAME: Wilbur/DbHttp.db             [Primary index]
//       NAME: Wilbur/DbHttp_ixName.db      [Secondary Name index]
//       NAME: Wilbur/DbHttp_ixTime.db      [Secondary Time index]
//       PROG: src/cpp/Wilbur/DbHttp.cpp
//       THIS: src/cpp/Wilbur/DbHttp.h
//
// Usage notes-
//       The maximum supported record size is MAX_VALUE_LENGTH.
//
// Implementation notes-
//       Index key 0 contains the next insert index key.
//
// Implementation notes-
//       struct DbHttpIndex {       // The DbHttp index
//         uint64_t    index;       // The HTTP index (NETWORK format)
//       }; // struct DbHttpIndex
//
//       struct DbHttpValue {       // The DbHttp value
//         uint64_t    text;        // The DbText link
//         uint64_t    time;        // Expiration time (Julian second)
//         char        name[1];     // The URI name (w/o '\0' terminator)
//                                  // (Does not include "http://" prefix.)
//       }; // struct DbHttpValue
//
//----------------------------------------------------------------------------
class DbHttp : public DbBase {      // The HTTP database
//----------------------------------------------------------------------------
// DbHttp::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbIndex;     // INDEX to VALUE database
Db*                    ixName;      // NAME  to INDEX database
Db*                    ixTime;      // TIME  to INDEX database

//----------------------------------------------------------------------------
// DbHttp::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 19               // High order 16 bits of uint64_t index
,  MAX_VALUE_LENGTH= 4095           // (See usage notes)
}; // enum

//----------------------------------------------------------------------------
// DbHttp::struct Value
//
// The '\0' name string terminator is required and returned in function calls
// but is not present in the database.
//----------------------------------------------------------------------------
struct Value {                      // The DbHttp value
   uint64_t            text;        // The DbText link
   uint64_t            time;        // Expiration time (Julian second)
   char                name[1];     // The URI name (w/o "http://" prefix.)
}; // struct Value

//----------------------------------------------------------------------------
// DbHttp::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbHttp( void );                 // Destructor
   DbHttp( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbHttp(const DbHttp&);           // Disallowed copy constructor
   DbHttp& operator=(const DbHttp&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbHttp::Methods
//----------------------------------------------------------------------------
public:
virtual Value*                      // value (NULL if error/missing)
   getValue(                        // Get HTTP value
     void*             value,       // (OUT) Result, size(MAX_VALUE_LENGTH+1)
     uint64_t          index);      // For this HTTP index

static Value*                       // value (NULL if error/missing)
   setValue(                        // Set HTTP value (with NUL terminator)
     void*             value,       // (OUT) Result, size(MAX_VALUE_LENGTH+1)
     uint64_t          text,        // For this TEXT index
     uint64_t          time,        // For this expiration time
     const char*       name);       // And this URI name

virtual uint64_t                    // The HTTP index (0 if error)
   insert(                          // Insert
     const Value*      value,       // This HTTP value, controlled by
     DbTxn*            dbTxn= NULL);// This transaction

virtual uint64_t                    // The Http index (0 if error/missing)
   locate(                          // Get Http index
     const char*       name);       // For this URI

inline uint64_t                     // The Http index (0 if error/missing)
   locate(                          // Get Http index
     std::string&      name)        // For this URI
{  return locate(name.c_str());
}

virtual Value*                      // value (NULL if error/missing)
   locate(                          // Get Http index
     const char*       name,        // For this URI
     void*             value);      // (OUT) Result, size(MAX_VALUE_LENGTH+1)

inline Value*                       // value (NULL if error/missing)
   locate(                          // Get Http value
     std::string&      name,        // For this URI
     void*             value)       // (OUT) Result, size(MAX_VALUE_LENGTH+1)
{  return locate(name.c_str(), value);
}

virtual uint64_t                    // Result (0 if error/missing/last)
   nextIndex(                       // Get next HTTP index
     uint64_t          index);      // After this HTTP index

virtual uint64_t                    // Result (0 if error/missing/last)
   nextName(                        // Get next HTTP name index
     const char*       name);       // And this URI

virtual uint64_t                    // Result (0 if error/missing/last)
   nextTime(                        // Get next HTTP time index
     uint64_t          index,       // After this HTTP index
     uint64_t          time);       // And this time

virtual int                         // Return code (0 if sucessful)
   remove(                          // Remove
     uint64_t          index,       // This HTTP index, controlled by
     DbTxn*            dbTxn= NULL); // This transaction

virtual int                         // Return code (0 if sucessful)
   revise(                          // Replace
     uint64_t          index,       // This HTTP index with
     const Value*      value,       // This HTTP value, controlled by
     DbTxn*            dbTxn= NULL); // This transaction

protected:
void
   close( void );                   // Close the database
}; // class DbHttp

#endif // DBHTTP_H_INCLUDED
