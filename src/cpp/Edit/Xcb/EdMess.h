//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdMess.h
//
// Purpose-
//       Editor: Message container
//
// Last change date-
//       2024/08/30
//
//----------------------------------------------------------------------------
#ifndef EDMESS_H_INCLUDED
#define EDMESS_H_INCLUDED

#include <pub/List.h>               // For pub::List

#include "Editor.h"                 // For Editor

//----------------------------------------------------------------------------
//
// Class-
//       EdMess
//
// Purpose-
//       Editor Message container
//
//----------------------------------------------------------------------------
class EdMess : public pub::List<EdMess>::Link { // Editor message descriptor
//----------------------------------------------------------------------------
// EdMess::Attributes
public:
enum                                // Message types
{  T_INFO                           // T_INFO: Informational, any key removes
,  T_MESS                           // T_MESS: Action, button click required
,  T_BUSY                           // T_BUSY: Limited function until complete
};

std::string            mess;        // The message
int                    type= T_INFO; // The message type

//----------------------------------------------------------------------------
// EdMess::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMess(                          // Constructor
     std::string       mess_,       // Message text
     int               type_= T_INFO);

   ~EdMess( void );                 // Destructor
}; // class EdMess
#endif // EDMESS_H_INCLUDED
