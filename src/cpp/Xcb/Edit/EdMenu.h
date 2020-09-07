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
//       EdMenu.h
//
// Purpose-
//       Editor: Menu (placeholder)
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#ifndef EDMENU_H_INCLUDED
#define EDMENU_H_INCLUDED

#include "Editor.h"                 // TODO: REMOVE. For include file debugging

#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/Layout.h"             // For xcb::Layout

//----------------------------------------------------------------------------
//
// Class-
//       EdMenu
//
// Purpose-
//       Menu Window
//
//----------------------------------------------------------------------------
class EdMenu : public xcb::Layout { // Editor menu Window (placeholder)
//----------------------------------------------------------------------------
// EdMenu::Attributes
public: // None defined

//----------------------------------------------------------------------------
// EdMenu::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMenu(                          // Constructor
     Widget*           parent= nullptr) // Parent Widget
:  Layout(parent, "EdMenu")
{
   if( opt_hcdm )
    debugh("EdMenu(%p)::EdMenu Named(%s)\n", this, get_name().c_str());

   use_size= {128, 14};
   min_size= use_size;
}

//----------------------------------------------------------------------------
virtual
   ~EdMenu( void )                  // Destructor
{
   if( opt_hcdm )
    debugh("EdMenu(%p)::~EdMenu\n", this);
}

//----------------------------------------------------------------------------
// EdMenu::Event handlers
//----------------------------------------------------------------------------
public: // None defined
}; // class EdMenu
#endif // EDMENU_H_INCLUDED
