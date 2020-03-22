//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbRDF3.h
//
// Purpose-
//       The RDF3 database. ** PRELIMINARY, NOT IMPLENTED **
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       This is a W3C Resource Description Format triple database.
//
//----------------------------------------------------------------------------
#ifndef DBRDF3_H_INCLUDED
#define DBRDF3_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbRDF3
//
// Purpose-
//       The RDF3 database.
//
// Identification-
//       NAME: Wilbur/DbRDF3.db             [Primary index]
//       NAME: Wilbur/DbRDF3_ixS2Pri.db     [Subject to primary index]
//       NAME: Wilbur/DbRDF3_ixO2Pri.db     [Object  to primary index]
//       PROG: src/cpp/Wilbur/DbRDF3.cpp
//       THIS: src/cpp/Wilbur/DbRDF3.h
//
// Implementation notes-
//       Index key 0 contains the next insert index key.
//
// Implementation notes-
//       struct DbRDF3Index {       // The DbRDF3 index
//         uint64_t    index;       // The RDF3 index (NETWORK format)
//       }; // struct DbRDF3Index
//
//       struct DbRDF3Value {       // The DbRDF3 value
//         uint32_t    sChunk;      // The subject CHUNK index
//         uint32_t    oChunk;      // The object  CHUNK index
//         uint32_t    vIndex;      // The verb (predicate) action index
//       }; // struct DbRDF3Value
//
//----------------------------------------------------------------------------
class DbRDF3 : public DbBase {      // The RDF3 database
//----------------------------------------------------------------------------
// DbRDF3::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint32_t       S_Chunk;     // Subject Chunk
typedef uint32_t       O_Chunk;     // Object  Chunk
typedef uint32_t       V_Index;     // Verb Index

//----------------------------------------------------------------------------
// DbRDF3::Attributes
//----------------------------------------------------------------------------
protected:
Db*                    dbIndex;     // INDEX to VALUE database (single-value)
Db*                    ixS2X;       // SUBJ  to INDEX database (multi-value)
Db*                    ixO2X;       // OBJ   to INDEX database (multi-value)

//----------------------------------------------------------------------------
// DbRDF3::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= 3                // High order 16 bits of uint64_t index
,  O_INDEX= 0x00030001              // High order 32 bits of object index
,  S_INDEX= 0x00030002              // High order 32 bits of subject index
,  V_INDEX= 0x00030003              // High order 32 bits of verb index
}; // enum

//----------------------------------------------------------------------------
// DbRDF3::struct Value
//----------------------------------------------------------------------------
struct Value {                      // The DbRDF3 value
   O_Chunk             oChunk;      // The object  chunk index
   S_Chunk             sChunk;      // The subject chunk index
   V_Index             vIndex;      // The verb action index
}; // struct Value

//----------------------------------------------------------------------------
// DbRDF3::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbRDF3( void );                 // Destructor
   DbRDF3( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbRDF3(const DbRDF3&);           // Disallowed copy constructor
   DbRDF3& operator=(const DbRDF3&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// DbRDF3::Methods
//----------------------------------------------------------------------------
public:
virtual Value*                      // value (NULL if error/missing)
   getValue(                        // Get RDF3 value
     uint64_t          index);      // For this RDF3 index

static Value*                       // Value (NULL if error/missing)
   setValue(                        // Set
     Value*            value,       // This RDF3 value to
     O_Chunk           oChunk,      // This object chunk index, and
     S_Chunk           sChunk,      // This subject chunk index,
     V_Index           vIndex);     // This action index

virtual uint64_t                    // Result (0 if error/missing/last)
   nextIndex(                       // Get next RDF3 index
     uint64_t          index);      // After this RDF3 index

virtual uint64_t                    // Result (0 if error/missing/last)
   nextObject(                      // Get next object RDF3 index
     uint64_t          index,       // After this RDF3 index
     O_Chunk           oChunk);     // For this object chunk

virtual uint64_t                    // Result (0 if error/missing/last)
   nextSubject(                     // Get next subject RDF3 index
     uint64_t          index,       // After this RDF3 index
     S_Chunk           sChunk);     // For this subject chunk

virtual int                         // Return code (0 if sucessful)
   remove(                          // Remove
     uint64_t          index,       // This RDF3 index, controlled by
     DbTxn*            dbTxn= NULL); // This transaction

virtual int                         // Return code (0 if sucessful)
   revise(                          // Replace
     uint64_t          index,       // This RDF3 index with
     const Value*      value,       // This RDF3 value, controlled by
     DbTxn*            dbTxn= NULL); // This transaction

protected:
void
   close( void );                   // Close the database
}; // class DbRDF3

#endif // DBRDF3_H_INCLUDED
