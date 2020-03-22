//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NetClientItem.h
//
// Purpose-
//       Work Item object for use with the NetClient.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       TODO: Needs work
//
//----------------------------------------------------------------------------
#ifndef NETCLIENTITEM_H_INCLUDED
#define NETCLIENTITEM_H_INCLUDED

#ifndef NETCLIENT_H_INCLUDED
#include "NetClient.h"              // Includes com/Dispatch.h
#endif

#include <com/DataSource.h>
#include "Url.h"

//----------------------------------------------------------------------------
//
// Class-
//       NetClientItem
//
// Purpose-
//       Http request/response information.
//
// Implementation note-
//       Need to figure out content and get/set mechanisms.
//
//----------------------------------------------------------------------------
class NetClientItem : public DispatchItem {
//----------------------------------------------------------------------------
// NetClientItem::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FC                             // Function codes
{  FC_CLOSE= 1                      // Close
,  FC_TIMER= 2                      // Timer
}; // enum FC

//----------------------------------------------------------------------------
// NetClientItem::Attributes
//----------------------------------------------------------------------------
public:
Url                       url;      // The URL to connect
int                       rc;       // The HTTP response code
DataSource                data;     // The HTTP response

//----------------------------------------------------------------------------
// NetClientItem::Constructors
//----------------------------------------------------------------------------
public:
   ~NetClientItem( void );          // Destructor
   NetClientItem( void );           // Default constructor
}; // class NetClientItem

#endif // NETCLIENTITEM_H_INCLUDED
