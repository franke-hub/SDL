//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RMconn.h
//
// Purpose-
//       RequestManagement descriptors.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef RMCONN_H_INCLUDED
#define RMCONN_H_INCLUDED

#include <com/Network.h>

//----------------------------------------------------------------------------
//
// Struct-
//       RMconnQ
//
// Purpose-
//       Describe RM connection data request.
//
//----------------------------------------------------------------------------
struct RMconnQ {                    // RM connection data request
enum FC
{
   FC_Connect,                      // Request connection port
   FC_Final                         // Close
};

   Network::Net32      fc;          // Function code
}; // struct RMconnQ

//----------------------------------------------------------------------------
//
// Struct-
//       RMconnS
//
// Purpose-
//       Describe RM connection data response.
//
//----------------------------------------------------------------------------
struct RMconnS {                    // RM connection data response
   Network::Net64      host;        // Connection IP address
   Network::Net32      port;        // Connection port number
}; // struct RMconnS

#endif // RMCONN_H_INCLUDED
