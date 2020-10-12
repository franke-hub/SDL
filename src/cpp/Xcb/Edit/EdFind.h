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
//       2020/10/12
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
public: // None defined

//----------------------------------------------------------------------------
// EdFind::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdFind(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= "EdFind") // Widget name
:  EdMisc(parent, name, 128, 14)
{
   if( opt_hcdm )
    debugh("EdFind(%p)::EdFind\n", this);
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
