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
//       NN.h
//
// Purpose-
//       (NN) Neural Net: Wrapper for types.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       NN is a wrapper class, contining no data.
//       Constructors, destructors and virtual methods are not allowed.
//
//----------------------------------------------------------------------------
#ifndef NN_H_INCLUDED
#define NN_H_INCLUDED

#ifndef SYSMAC_H_INCLUDED
#include <com/sysmac.h>
#endif
#undef max
#undef min

#include "NNtype.h"

//----------------------------------------------------------------------------
//
// Struct-
//       NN
//
// Purpose-
//       Wrapper for Neural Net Types.
//
//----------------------------------------------------------------------------
struct NN                           // Neural Net
{
//----------------------------------------------------------------------------
// NN::Part
//----------------------------------------------------------------------------
enum Part                           // Values for PartId
{
   PartControl=                   0,// Control area
   PartBundle=                    1,// Bundles
   PartNeuron=                    2,// Neurons
   PartFanin=                     3,// Fan-ins
   PartCount=                     4 // Number of part indexes
}; // enum Part

//----------------------------------------------------------------------------
// NN::Typedefs
//----------------------------------------------------------------------------
struct Color                        // Color descriptor
{
   unsigned char       p;           // Assigned PEL
   unsigned char       r, g, b;     // Red, Green, Blue components
}; // struct NN::Color

typedef int32_t        Tick;        // Clock tick type

typedef char           FileName[PGS_FNSIZE]; // File name
typedef char           FileInfo[PGS_FNSIZE]; // File comment

typedef unsigned char  Boolean;     // Boolean type
typedef unsigned char* String;      // String type
typedef float          Value;       // Value type
typedef float          Weight;      // Weight type

//----------------------------------------------------------------------------
// NN::Pointer components
//----------------------------------------------------------------------------
typedef uint16_t       FileId;      // File identifier
typedef uint16_t       PartId;      // Partition identifier
typedef uint64_t       Offset;      // Offset (in partition)

//----------------------------------------------------------------------------
// NN::Pointers
//----------------------------------------------------------------------------
struct FPO                          // Long (File,Part,Offset) pointer
{
   Offset              o;           // Offset
   FileId              f;           // File identifier
   PartId              p;           // Partition identifier
}; // struct NN::FPO

struct FO                           // Medium (File,Offset) pointer
{
   Offset              o;           // Offset
   FileId              f;           // File identifier
}; // struct NN::FO

typedef Offset         Vaddr;       // Virtual address
typedef void*          Raddr;       // Real address
}; // struct NN

#endif // NN_H_INCLUDED
