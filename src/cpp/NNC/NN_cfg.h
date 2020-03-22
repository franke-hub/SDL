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
//       NN_cfg.h
//
// Purpose-
//       (NN) Neural Net: Configuration controls
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NN_CFG_H_INCLUDED
#define NN_CFG_H_INCLUDED

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       NN_cfg
//
// Purpose-
//       Configuration controls.
//
//----------------------------------------------------------------------------
struct NN_cfg                       // Configuration controls
{
//----------------------------------------------------------------------------
// NN_cfg::Enumerations and typedefs
//----------------------------------------------------------------------------
enum                                // VPS controls
{
   VPS_FRAMESIZE=              4096,// VPS frame size
   VPS_FILENO=                    1,// Number of files
   VPS_PARTNO=        NN::PartCount // Number of partitions per file
}; // enum

}; // struct NN_cfg

#endif // NN_CFG_H_INCLUDED
