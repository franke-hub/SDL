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
//       DbNada.h
//
// Purpose-
//       Placeholder, does nothing.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DBNADA_H_INCLUDED
#define DBNADA_H_INCLUDED

#ifndef DBBASE_H_INCLUDED
#include "DbBase.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DbNada
//
// Purpose-
//       Placeholder, does nothing.
//
// Identification-
//       NAME: Wilbur/DbNada.db
//       PROG: src/cpp/Wilbur/DbNada.cpp
//       THIS: src/cpp/Wilbur/DbNada.h
//
// Implementation notes-
//       struct DbNadaIndex {       // The DbNada index
//         uint64_t    index;       // The Nada index (NETWORK format)
//       }; // struct DbNadaIndex
//
//       struct DbNadaValue {       // The DbNada value
//       }; // struct DbNadaValue
//
// Implementation notes-
//       <PRELIMINARY>
//
//----------------------------------------------------------------------------
class DbNada : public DbBase {      // The Nada database
//----------------------------------------------------------------------------
// DbWord::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  EXTENDED_INDEX= (-1)             // High order 16 bits of uint64_t index
,  MAX_VALUE_LENGTH= 255            // (See usage notes)
}; // enum

//----------------------------------------------------------------------------
// DbNada::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DbNada( void );                 // Destructor
   DbNada( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   DbNada(const DbNada&);           // Disallowed copy constructor
   DbNada& operator=(const DbNada&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DbNada::Methods
//----------------------------------------------------------------------------
public:
}; // class DbNada

#endif // DBNADA_H_INCLUDED
