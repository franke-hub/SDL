//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdFind.h
//
// Purpose-
//       Editor: Find/replace Popup
//
// Last change date-
//       2020/09/06
//
// Implementation notes-
//       PLACEHOLDEER ONLY: NO (substantial) IMPLEMENTAITON
//
//----------------------------------------------------------------------------
#ifndef EDFIND_H_INCLUDED
#define EDFIND_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include <string>                   // For std::string
#include "EdMisc.h"                 // For EdMisc

//----------------------------------------------------------------------------
//
// Class-
//       EdFind
//
// Purpose-
//       Find Window
//
//----------------------------------------------------------------------------
class EdFind : public EdMisc {        // Editor Find/Replace Windows
//----------------------------------------------------------------------------
// EdFind::Attributes
//----------------------------------------------------------------------------
public:
std::string            locate_string; // The locate string
std::string            change_string; // The change string

//----------------------------------------------------------------------------
// EdFind::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdFind( void )                   // Constructor
:  EdMisc(128, 14, "EdFind")
{
   if( opt_hcdm )
    debugh("EdFind(%p)::EdFind Named(%s)\n", this, get_name().c_str());
}

//----------------------------------------------------------------------------
virtual
   ~EdFind( void )                  // Destructor
{
   if( opt_hcdm )
    debugh("EdFind(%p)::~EdFind\n", this);
}
}; // class EdFind
#endif // EDFIND_H_INCLUDED
