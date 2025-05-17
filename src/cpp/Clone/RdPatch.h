//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       RdPatch.h
//
// Purpose-
//       Common controls.
//
// Last change date-
//       2025/01/26
//
// Implementation notes-
//       PATCH: Use /etc/hosts name if available
//       Requires pub library.
//
//----------------------------------------------------------------------------
#ifndef RDPATCH_H_INCLUDED
#define RDPATCH_H_INCLUDED

#include <string>                   // For std::string

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_sockaddr
//
// Purpose-
//       Get socket information for name
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   get_sockaddr(                    // Convert "host:port" to sockaddr
     const std::string&nps,         // The "host:port" name string
     void*             addr,        // OUT: The sockaddr
     int*              size);       // INP/OUT: The sockaddr length
#endif // RDPATCH_H_INCLUDED
