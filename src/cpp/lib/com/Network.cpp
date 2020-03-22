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
//       Network.cpp
//
// Purpose-
//       Instantiate Network methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
  #include <winsock.h>

#else
  #include <netinet/in.h>
#endif

#include "com/Network.h"

//----------------------------------------------------------------------------
//
// Union-
//       Word64
//
// Purpose-
//       Define union {uint64_t word; Byte byte[8]}
//
//----------------------------------------------------------------------------
union Word64                        // 64 bit word
{
   uint64_t            word;        // Word representation
   Network::Byte       byte[8];     // Byte representation
};

//----------------------------------------------------------------------------
//
// Method-
//       Network::hton16
//       Network::hton32
//       Network::hton64
//
// Purpose-
//       Convert host value to network value.
//
//----------------------------------------------------------------------------
Network::Net16                      // (Network format) Resultant
   Network::hton16(                 // Convert into Net16
     Host16            host16)      // From host format
{
   return htons(host16);
}

Network::Net32                      // (Network format) Resultant
   Network::hton32(                 // Convert into Net32
     Host32            host32)      // From host format
{
   return htonl(host32);
}

Network::Net64                      // (Network format) Resultant
   Network::hton64(                 // Convert into Net64
     Host64            host64)      // From host format
{
   Word64              cw;          // Conversion word

   cw.byte[0]= host64 >> 56;
   cw.byte[1]= host64 >> 48;
   cw.byte[2]= host64 >> 40;
   cw.byte[3]= host64 >> 32;
   cw.byte[4]= host64 >> 24;
   cw.byte[5]= host64 >> 16;
   cw.byte[6]= host64 >>  8;
   cw.byte[7]= host64;
   return cw.word;
}

//----------------------------------------------------------------------------
//
// Method-
//       Network::load16
//       Network::load32
//       Network::load64
//
// Purpose-
//       Convert network value to host value.
//
//----------------------------------------------------------------------------
Network::Host16                     // (Host format) Resultant
   Network::load16(                 // Load (Network format) data
     const Byte*       byte)        // (Network format) data
{
   Host16              resultant;

   resultant  = (Host16)byte[0] <<  8;
   resultant |= (Host16)byte[1];
   return resultant;
}

Network::Host32                     // (Host format) Resultant
   Network::load32(                 // Load (Network format) data
     const Byte*       byte)        // (Network format) data
{
   Host32              resultant;

   resultant  = (Host32)byte[0] << 24;
   resultant |= (Host32)byte[1] << 16;
   resultant |= (Host32)byte[2] <<  8;
   resultant |= (Host32)byte[3];
   return resultant;
}

Network::Host64                     // (Host format) Resultant
   Network::load64(                 // Load (Network format) data
     const Byte*       byte)        // (Network format) data
{
   Host64              resultant;

   resultant  = (Host64)byte[0] << 56;
   resultant |= (Host64)byte[1] << 48;
   resultant |= (Host64)byte[2] << 40;
   resultant |= (Host64)byte[3] << 32;
   resultant |= (Host64)byte[4] << 24;
   resultant |= (Host64)byte[5] << 16;
   resultant |= (Host64)byte[6] <<  8;
   resultant |= (Host64)byte[7];
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Network::ntoh16
//       Network::ntoh32
//       Network::ntoh64
//
// Purpose-
//       Convert network value to host value.
//
//----------------------------------------------------------------------------
Network::Host16                     // (Host format) Resultant
   Network::ntoh16(                 // Convert from Net16
     Net16             net16)       // From network format
{
   return ntohs(net16);
}

Network::Host32                     // (Host format) Resultant
   Network::ntoh32(                 // Convert from Net32
     Net32             net32)       // From network format
{
   return ntohl(net32);
}

Network::Host64                     // (Host format) Resultant
   Network::ntoh64(                 // Convert from Net64
     Net64             net64)       // From network format
{
   Host64              resultant;   // Resultant
   Word64              cw;          // Conversion word

   cw.word= net64;
   resultant  = 0;
   resultant |= (Host64)cw.byte[0] << 56;
   resultant |= (Host64)cw.byte[1] << 48;
   resultant |= (Host64)cw.byte[2] << 40;
   resultant |= (Host64)cw.byte[3] << 32;
   resultant |= (Host64)cw.byte[4] << 24;
   resultant |= (Host64)cw.byte[5] << 16;
   resultant |= (Host64)cw.byte[6] <<  8;
   resultant |= (Host64)cw.byte[7];
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Network::store16
//       Network::store32
//       Network::store64
//
// Purpose-
//       Convert network value to host value.
//
//----------------------------------------------------------------------------
void
   Network::store16(                // Load (Network format) data
     Host16            host16,      // (Host format) source
     Byte*             byte)        // (Network format) resultant
{
   byte[0]= host16 >>  8;
   byte[1]= host16;
}

void
   Network::store32(                // Load (Network format) data
     Host32            host32,      // (Host format) source
     Byte*             byte)        // (Network format) resultant
{
   byte[0]= host32 >> 24;
   byte[1]= host32 >> 16;
   byte[2]= host32 >>  8;
   byte[3]= host32;
}

void
   Network::store64(                // Load (Network format) data
     Host64            host64,      // (Host format) source
     Byte*             byte)        // (Network format) resultant
{
   byte[0]= host64 >> 56;
   byte[1]= host64 >> 48;
   byte[2]= host64 >> 40;
   byte[3]= host64 >> 32;
   byte[4]= host64 >> 24;
   byte[5]= host64 >> 16;
   byte[6]= host64 >>  8;
   byte[7]= host64;
}

