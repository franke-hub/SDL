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
//      Dasd.h
//
// Purpose-
//       Disk data layout.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DASD_H_INCLUDED
#define DASD_H_INCLUDED

#ifndef TYPES_H_INCLUDED
#include "Types.h"
#endif

#ifndef STOCK_H_INCLUDED
#include "Stock.h"
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       DasdHeader
//
// Purpose-
//       Disk file header.
//
//----------------------------------------------------------------------------
struct DasdHeader                   // Disk file header
{
//----------------------------------------------------------------------------
// DasdHeader::Attributes
//----------------------------------------------------------------------------
   char              cbid[16];      // Control block identifier
#define PLEX_CBID "FANIN DATA FILE" // Control block identifier data

   unsigned          releaseId;     // Release identifier
   unsigned          versionId;     // Version identifier
   unsigned          julianDay;     // Julian day
   unsigned          julianTod;     // Julian time of day (milliseconds)
}; // struct DasdHeader

//----------------------------------------------------------------------------
//
// Struct-
//       DasdNew
//
// Purpose-
//       Current Header.
//
//----------------------------------------------------------------------------
struct DasdNew : public DasdHeader  // Disk file header
{
//----------------------------------------------------------------------------
// DasdNew::Enumerations and typedefs
//----------------------------------------------------------------------------
enum
{
   ReleaseId=            0x20070101,// Release identifier
   VersionId=                     1,// Version identifier

   L3ArraySize= (DIM_L3)*(DIM_INP), // L3 Array size
   L2ArraySize= (DIM_L2)*(DIM_L3),  // L2 Array size
   L1ArraySize= (DIM_L1)*(DIM_L2),  // L1 Array size
   L0ArraySize= (DIM_OUT)*(DIM_L1), // L0 Array size

   ArraySize= (L3ArraySize+L2ArraySize+L1ArraySize+L0ArraySize)*2
}; // enum

//----------------------------------------------------------------------------
// DasdNew::Attributes
//----------------------------------------------------------------------------
   unsigned          l3ArraySize;   // sizeof(L3 Fanin Array)
   unsigned          l2ArraySize;   // sizeof(L2 Fanin Array)
   unsigned          l1ArraySize;   // sizeof(Ll Fanin Array)
   unsigned          l0ArraySize;   // sizeof(L0 Fanin Array)

   int64_t           randSeed;      // Current random seed
   unsigned          generation;    // Generation number
   unsigned short    unitCount;     // Number of Units
   unsigned short    usedCount;     // Number of Units used
   unsigned short    cullCount;     // Number of Units Culled
   unsigned short    outsCount;     // Number of output units

   unsigned          index0;        // First used history index
   unsigned          indexN;        // Highest used history index
   unsigned          julian0;       // Julian date associated with index0
   unsigned          julianN;       // Julian date associated with indexN

   struct Evaluation
   {
     unsigned long   evaluation;    // Evaluation
     unsigned long   cash;          // Cash valuation
     unsigned long   stock;         // Stock valuation
     unsigned long   lastTransfer;  // Last transfer date
     unsigned long   fee;           // Fees paid
     Value           outs[DIM_OUT]; // Output evaluation array
   }                 unit[DIM_UNIT];// Unit descriptor array
}; // struct DasdNew

#include "OldDasd.h"

#endif // DASD_H_INCLUDED
