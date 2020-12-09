//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdTabs.h
//
// Purpose-
//       Editor: Tabs (placeholder)
//
// Last change date-
//       2020/12/08
//
//----------------------------------------------------------------------------
#ifndef EDTABS_H_INCLUDED
#define EDTABS_H_INCLUDED

#include "Editor.h"                 // For namespace editor::debug
#include "EdMisc.h"                 // For EdMisc

//----------------------------------------------------------------------------
//
// Class-
//       EdTabs
//
// Purpose-
//       Tabs Window
//
//----------------------------------------------------------------------------
class EdTabs : public EdMisc {      // Editor tabs Window (placeholder)
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
// EdTabs::Attributes
//----------------------------------------------------------------------------
public: // None defined

//----------------------------------------------------------------------------
// EdTabs::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdTabs(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= "EdTabs") // Widget name
:  EdMisc(parent, name, 128, 14)
{
   (void)parent;                    // TODO: Parent always first (possible?)
   if( editor::debug::opt_hcdm )
    editor::debug::debugh("EdTabs(%p)::EdTabs\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdTabs( void )                  // Destructor
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdTabs(%p)::~EdTabs\n", this);
}

//----------------------------------------------------------------------------
// EdTabs::Event handlers
//----------------------------------------------------------------------------
public: // None defined
}; // class EdTabs
#endif // EDTABS_H_INCLUDED
