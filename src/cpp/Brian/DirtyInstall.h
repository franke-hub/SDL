//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DirtyInstall.h
//
// Purpose-
//       Quick and dirty tests.
//
// Last change date-
//       2021/07/09
//
//----------------------------------------------------------------------------
#ifndef DIRTY_INSTALL_H_INCLUDED
#define DIRTY_INSTALL_H_INCLUDED

#include <pub/Dispatch.h>
#include <pub/Debug.h>

#include "Common.h"
#include "Command.h"
#include "Service.h"

#include "Install.h"

using _PUB_NAMESPACE::Debug;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
//
// Class-
//       DirtyInstall
//
// Purpose-
//       Quick and dirty tests
//
//----------------------------------------------------------------------------
class DirtyInstall : public Install { // Quick and dirty tests
//----------------------------------------------------------------------------
// DirtyInstall::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DirtyInstall( void )            // Destructor
{  traceh("DirtyInstall(%p)::~DirtyInstall\n", this); }

   DirtyInstall( void )             // Constructor
:  Install()
{  traceh("DirtyInstall(%p)::DirtyInstall\n", this);
   Common* common= Common::get();

   // Service bringup test
   if( true  ) {
     ServiceMap[new Service("one")];
     ServiceMap[new Service("two")];
     pub::dispatch::Wait wait;
     pub::dispatch::Item item(22, &wait);
     common->work("one", &item);
     wait.wait();
     wait.reset();
     common->work("two", &item);
     wait.wait();
   }

   // Command bringup test
   if( false ) {
     int argc= 2;
     const char* argv[]= { "one", "two" };
     CommandMap[new Command("alpha")]; // Insert Bringup Command
     CommandMap[new Command("beta1")]; // Insert Console Command
     CommandMap["alpha"].work(argc, (char**)argv); // Run Bringup Command
     CommandMap["beta1"].work(argc, (char**)argv); // Run Bringup Command
   }
}

   DirtyInstall(const DirtyInstall&) = delete; // Disallowed copy constructor
   DirtyInstall& operator=(const DirtyInstall&) = delete; // Disallowed assignment operator
}; // class DirtyInstall
#endif // DIRTY_INSTALL_H_INCLUDED
