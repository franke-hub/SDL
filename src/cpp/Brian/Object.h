//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       Object.h
//
// Purpose-
//       The base Object type
//
// Last change date-
//       2026/09/07
//
//----------------------------------------------------------------------------
#ifndef OBJECT_H_INCLUDED
#define OBJECT_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Object
//
// Purpose-
//       Define the Brian Object type
//
//----------------------------------------------------------------------------
class Object {                      // The Brian Object type
public:
   Object( void ) = default;        // Default constructor

virtual
   ~Object( void ) = default;       // Default (virtual) destructor
};
#endif // OBJECT_H_INCLUDED
