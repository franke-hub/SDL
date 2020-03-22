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
//       Global.h
//
// Purpose-
//       Define the Global area for Wilbur Objects.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       Global
//
// Purpose-
//       Define the Global area for Wilbur Objects.
//       The Global area is in shared memory. It is available to and shared
//       by all processes and all threads.
//
// Notes-
//       See also: Common
//
// Implementation notes-
//       <PRELIMINARY>
//       There is no current need for a true shared memory Global area.
//       This version of the Global area is process local.
//       Even so, this area should be treated as if it were in shared memory.
//       It may not contain any pointers. It can only contain offsets.
//
//----------------------------------------------------------------------------
struct Global {                     // Global data area
//----------------------------------------------------------------------------
// Global::Attributes
//----------------------------------------------------------------------------
public:
const char             VERSION_ID[32]; // Version identifier (must be first)
unsigned               refCounter;  // Reference count
}; // struct Global

#endif // GLOBAL_H_INCLUDED
