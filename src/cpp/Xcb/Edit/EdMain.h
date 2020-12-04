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
//       EdMain.h
//
// Purpose-
//       Editor: Main Window
//
// Last change date-
//       2020/11/27
//
// Implementation note-
//       PLACEHOLDER.
//
//----------------------------------------------------------------------------
#ifndef EDMAIN_H_INCLUDED
#define EDMAIN_H_INCLUDED

#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Font.h"               // For Font
#include "Xcb/Device.h"             // For Device
#include "Xcb/Global.h"             // For ENQUEUE macro
#include "Xcb/Keysym.h"             // For X11/keysymdef.h
#include "Xcb/Types.h"              // For Types
#include "Xcb/TextWindow.h"         // For TextWindow base class

#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/Widget.h"             // For xcb::Widget

//----------------------------------------------------------------------------
//
// Class-
//       EdMain
//
// Purpose-
//       Editor Main Window (PLACEHOLDER)
//
//----------------------------------------------------------------------------
class EdMain {                      // Editor Main Window
//----------------------------------------------------------------------------
// EdMain::Attributes
public: // NONE DEFINED

//----------------------------------------------------------------------------
// EdMain::Constructor
//----------------------------------------------------------------------------
public:
   EdMain( void )                   // Constructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdMain(%p)::EdMain\n", this);
}

//----------------------------------------------------------------------------
// EdMain::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdMain( void )                  // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdMain(%p)::~EdMain\n", this);
}; // class EdMain
#endif // EDMAIN_H_INCLUDED
