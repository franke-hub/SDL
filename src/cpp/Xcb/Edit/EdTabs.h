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
//       EdTabs.h
//
// Purpose-
//       Editor: Tabs (placeholder)
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#ifndef EDTABS_H_INCLUDED
#define EDTABS_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include "Xcb/Window.h"             // For xcb::Window
#include "EdMisc.h"                 // For EdMisc

#include "Editor.h"                 // For Editor

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
public:
Editor*                editor;      // The associated Editor

//----------------------------------------------------------------------------
// EdTabs::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdTabs(                          // Constructor
     Editor*           editor,      // The control Editor
     Widget*           parent= nullptr) // Parent Widget
:  EdMisc(128, 14, "EdTabs"), editor(editor)
{
   if( opt_hcdm )
    debugh("EdTabs(%p)::EdTabs\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdTabs( void )                  // Destructor
{
   if( opt_hcdm )
    debugh("EdTabs(%p)::~EdTabs\n", this);
}

//----------------------------------------------------------------------------
// EdTabs::Event handlers
//----------------------------------------------------------------------------
public: // None defined
}; // class EdTabs
#endif // EDTABS_H_INCLUDED
