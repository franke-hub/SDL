//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Bit.cpp
//
// Purpose-
//       Bit object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "com/Bit.h"
#if !(INLINING)
#include "com/Bit.i"
#endif

//----------------------------------------------------------------------------
// Static constants
//----------------------------------------------------------------------------
const unsigned char  Bit::bitClr[8]=// Bitfield zero mask table
{  0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe
};

const unsigned char  Bit::bitSet[8]=// Bitfield ones mask table
{  0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
};

