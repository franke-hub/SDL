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
//       DasdOld.h
//
// Purpose-
//       Obsolete Header.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
struct DasdOld : public DasdHeader  // Disk file header
{
//----------------------------------------------------------------------------
// DasdOld::Enumerations and typedefs
//----------------------------------------------------------------------------
enum
{
   ReleaseId=            0x20060101,// Release identifier
   VersionId=                     1,// Version identifier

   L3ArraySize= (DIM_L3)*(DIM_INP), // L3 Array size
   L2ArraySize= (DIM_L2)*(DIM_L3),  // L2 Array size
   L1ArraySize= (DIM_L1)*(DIM_L2),  // L1 Array size
   L0ArraySize= (DIM_OUT)*(DIM_L1), // L0 Array size

   ArraySize= (L3ArraySize+L2ArraySize+L1ArraySize+L0ArraySize)*2
}; // enum

//----------------------------------------------------------------------------
// DasdOld::Attributes
//----------------------------------------------------------------------------
   unsigned          l3ArraySize;   // sizeof(L3 Fanin Array)
   unsigned          l2ArraySize;   // sizeof(L2 Fanin Array)
   unsigned          l1ArraySize;   // sizeof(Ll Fanin Array)
   unsigned          l0ArraySize;   // sizeof(L0 Fanin Array)

   int64_t           randSeed;      // Current random seed
   unsigned          _0001;         // Reserved for expansion
   unsigned short    unitCount;     // Number of Units
   unsigned short    usedCount;     // Number of Units used
   unsigned short    cullCount;     // Number of Units Culled
   unsigned short    outsCount;     // Number of output units

   unsigned          index0;        // First used history index
   unsigned          indexN;        // Highest used history index
   unsigned          julian0;       // Julian date associated with index0
   unsigned          julianN;       // Julian date associated with indexN

   unsigned          evaluation[DIM_UNIT]; // Evaluation array
   Value             output[DIM_UNIT][DIM_OUT]; // Outputs
}; // struct DasdOld
