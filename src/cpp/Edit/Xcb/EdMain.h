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
//       2020/12/08
//
// Implementation note-
//       PLACEHOLDER.
//
//----------------------------------------------------------------------------
#ifndef EDMAIN_H_INCLUDED
#define EDMAIN_H_INCLUDED

#include <Editor.h>                 // For namespace editor::debug

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
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdMain(%p)::EdMain\n", this);
}

//----------------------------------------------------------------------------
// EdMain::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdMain( void )                  // Destructor
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdMain(%p)::~EdMain\n", this);
}; // class EdMain
#endif // EDMAIN_H_INCLUDED
