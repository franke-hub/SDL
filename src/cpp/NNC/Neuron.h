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
//       Neuron.h
//
// Purpose-
//       Define the Neuron object.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       Neuron is a flat class.
//       Constructors, destructors and virtual methods are not allowed.
//
//----------------------------------------------------------------------------
#ifndef NEURON_H_INCLUDED
#define NEURON_H_INCLUDED

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Neuron
//
// Purpose-
//       Neuron descriptor.
//
//----------------------------------------------------------------------------
class Neuron                        // Neuron descriptor
{
//----------------------------------------------------------------------------
// Neuron::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Type                           // Neuron Type
{
   TypeError=           0,
   TypeNop=             1,
   TypeClock=           2,
   TypeFileRD=          4,
   TypeFileWR=          5,
   TypeStore=           6,
   Type0007=            7,
   Type0008=            8,
   Type0009=            9,
   Type0010=           10,
   Type0011=           11,
   Type0012=           12,
   Type0013=           13,
   Type0014=           14,
   Type0015=           15,
   Type0016=           16,
   Type0017=           17,
   Type0018=           18,
   TypeTrain=          19,

   TypeInc=            20,
   TypeDec=            21,
   TypeAdd=            22,
   TypeSub=            23,
   TypeMul=            24,
   TypeDiv=            25,
   Type0026=           26,
   TypeAbs=            27,
   TypeNeg=            28,
   TypeSigmoid=        29,
   Type0030=           30,
   Type0031=           31,
   Type0032=           32,
   Type0033=           33,
   Type0034=           34,
   Type0035=           35,
   Type0036=           36,
   Type0037=           37,
   Type0038=           38,
   Type0039=           39,

   TypeAnd=            40,
   TypeOr=             41,
   TypeNand=           42,
   TypeNor=            43,
   Type0044=           44,
   Type0045=           45,
   Type0046=           46,
   Type0047=           47,
   Type0048=           48,
   Type0049=           49,

   TypeIf=             50,
   TypeWhile=          51,
   TypeUntil=          53,
   Type0054=           54,
   Type0055=           55,
   Type0056=           56,
   Type0057=           57,
   Type0058=           58,
   Type0059=           59,
   TypeCOUNT=          60,          // The number of neuron types

   // Aliases
   TypeDefault=        TypeSigmoid,
   TypeConstant=       TypeNop
}; // enum Type

enum                                // Generic constants
{
   CBID=               0xFE01       // Control block validator
};

//----------------------------------------------------------------------------
// Neuron::Constructors
//----------------------------------------------------------------------------
public:
   // No default constructor or destructor

private:                            // Bitwise copy is prohibited
   Neuron(const Neuron&);           // Disallowed copy constructor
   Neuron& operator=(const Neuron&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Neuron::Compile-time methods
//----------------------------------------------------------------------------
public:
static NN::Vaddr                    // Resultant offset
   index(                           // Compute Vaddr(Neuron[index])
     NN::Vaddr         base,        // Base Neuron address
     unsigned          index);      // Element index

//----------------------------------------------------------------------------
// Neuron::Methods
//----------------------------------------------------------------------------
public:
inline int                          // TRUE if valid
   isValid( void ) const;           // Is this Neuron valid?

inline NN::Value                    // Current value
   resolve( void );                 // Resolve value

//----------------------------------------------------------------------------
// Neuron::Attributes
//----------------------------------------------------------------------------
public:
   uint16_t            cbid;        // 0000 Control block identifier
   uint16_t            type;        // 0002 Type identifier

   struct                           // 0004 Status and exception controls
   {
     unsigned int      eof      : 1;// End of file (neuron scan)
     unsigned int               : 7;// Reserved, available
     unsigned int               : 8;// Reserved, available

     unsigned int               : 8;// Reserved, available
     unsigned int               : 4;// Reserved, available
     unsigned int      training : 1;// This neuron is in training
     unsigned int      breakpt  : 1;// This neuron has a breakpoint
     unsigned int      disabled : 1;// This neuron is disabled
     unsigned int      any      : 1;// Exception indicator:
                                    // summary of other indicators
   } ex;

   NN::Tick            clock;       // 0008 Last clock tick
// NN::Tick            train;       // 000C Last train tick
   NN::Value           value;       // 000C Raw output value

   NN::Vaddr           faninVaddr;  // 0010 -> Fanin array extension
   NN::Raddr           faninRaddr;  // 0018 -> Fanin array extension
   uint32_t            faninCount;  // 001C The number of fanin elements
}; // class Neuron                  // 0020

#endif // NEURON_H_INCLUDED
