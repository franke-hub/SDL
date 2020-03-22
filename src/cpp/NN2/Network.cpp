//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Network.cpp
//
// Purpose-
//       Define static data and anything else we don't want to look at much.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include "Network.h"

namespace NETWORK_H_NAMESPACE {
//----------------------------------------------------------------------------
// void nop(void*): Really does nothing except avoid a compiler complaint
//----------------------------------------------------------------------------
void _nop(void* thing) { assert( thing ); } // Don't nop(nullptr), OK?

//----------------------------------------------------------------------------
// _testID: Specific test identifier. (0 if no explicit test.)
//----------------------------------------------------------------------------
const int              _testID= 0;

//----------------------------------------------------------------------------
// Network::_verbose: IFDEBUG Debugging verbosity. ), higher is noisier
//----------------------------------------------------------------------------
int                    Network::_verbose= 1; // Global debug verbosity
}  // namespace NETWORK_H_NAMESPACE

