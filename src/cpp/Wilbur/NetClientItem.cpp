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
//       NetClientItem.cpp
//
// Purpose-
//       NetClientItem implementation methods.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       If too many current connections, must DEFER request.
//       (The current implementation returns an error.)
//
//----------------------------------------------------------------------------
#include <Common.h>                 // For logf

#include "NetClientItem.h"          // Implementation class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       NetClientItem::~NetClientItem
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NetClientItem::~NetClientItem( void ) // Destructor
{
   IFSCDM( logf("NetClientItem(%p)::~NetClientItem()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       NetClientItem::NetClientItem
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NetClientItem::NetClientItem( void ) // Constructor
:  DispatchItem()
,  url()
,  rc(-1)
,  data()
{
   IFSCDM( logf("NetClientItem(%p)::NetClientItem()\n", this); )
}

