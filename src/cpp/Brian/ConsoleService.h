//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ConsoleService.h
//
// Purpose-
//       The ConsoleService.
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#ifndef CONSOLESERVICE_H_INCLUDED
#define CONSOLESERVICE_H_INCLUDED

#include "Service.h"                // For Service, base class

//----------------------------------------------------------------------------
//
// Class-
//       ConsoleService
//
// Purpose-
//       Handle Console input.
//
//----------------------------------------------------------------------------
class ConsoleService                // The ConsoleService
   : public Service, public Service::has_stop, public Service::has_wait {
//----------------------------------------------------------------------------
// ConsoleService::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// ConsoleService::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleService( void )           // Constructor
:  Service("Console") {}

   ConsoleService(const ConsoleService&) = delete; // Disallowed copy constructor
   ConsoleService& operator=(const ConsoleService&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// ConsoleService::Methods
//----------------------------------------------------------------------------
public:
virtual void
   stop( void );                    // Stop the ConsoleService

virtual void
   wait( void );                    // Wait for ConsoleService termination
}; // class ConsoleService
#endif // CONSOLESERVICE_H_INCLUDED
