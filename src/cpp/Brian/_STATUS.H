//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       _STATUS.H
//
// Purpose-
//       Development status
//
// Last change date-
//       2024/11/07
//
//----------------------------------------------------------------------------
#ifndef _STATUS_H_INCLUDED
#define _STATUS_H_INCLUDED

enum STATUS_T                       // The status types
{  INITIAL                          // - Initial, debugging ON
,  TESTING                          // - Testing, debugging ON
,  PRESHIP                          // - Pre-shipped, debugging OFF
,  SHIPPED                          // - Shipped, debugging OFF
}; // enum STATUS_T

enum { _STATUS= TESTING };          // Current development status

#endif // _STATUS_H_INCLUDED
