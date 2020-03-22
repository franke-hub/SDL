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
//       Network.h
//
// Purpose-
//       Network object descriptors.
//
// Last change date-
//       2007/01/01
//
// Definitions-
//       A "Host format" value is the native host value, e.g. one containing
//       the representation normally used within the host. This value may be
//       stored in any format, so its content is not consistent across
//       differing machine architectures.
//
//       A "Network format" value is the value that corresponds with a "Host
//       format" value, such that when the value is stored it will be stored
//       as a big-endian value.
//
// Usage-
//       The hton and ntoh functions are similar to the functions defined by
//       /usr/include/netinet/in.h, with the addition here of 64-bit methods.
//       The Network object methods are not optimized. ntohl, ntohs, htonl,
//       and htons are preferred when performance is an issue.
//
//       The load method is similar to ntoh and the store method is similar
//       to hton. These methods load and store from Byte arrays, thus avoiding
//       structure alignment considerations.
//
//----------------------------------------------------------------------------
#ifndef NETWORK_H_INCLUDED
#define NETWORK_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Network
//
// Purpose-
//       Network object methods.
//
//----------------------------------------------------------------------------
class Network {                     // Network object methods
//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint16_t       Host16;      // Host format 16 bit value
typedef uint32_t       Host32;      // Host format 32 bit value
typedef uint64_t       Host64;      // Host format 64 bit value
typedef unsigned char  Byte;        // Network data byte
typedef uint16_t       Net16;       // Network format 16 bit value
typedef uint32_t       Net32;       // Network format 32 bit value
typedef uint64_t       Net64;       // Network format 64 bit value

//----------------------------------------------------------------------------
// Network::Static methods
//----------------------------------------------------------------------------
public:
static Net16                        // (Network format) resultant
   hton16(                          // Convert Host16 => Net16
     Host16            host16);     // (Host format) source

static Net32                        // (Network format) resultant
   hton32(                          // Convert Host32 => Net32
     Host32            host32);     // (Host format) source

static Net64                        // (Network format) resultant
   hton64(                          // Convert Host64 => Net64
     Host64            host64);     // (Host format) source

static Host16                       // (Host format) resultant
   load16(                          // Load (Network format) data
     const Byte*       byte);       // (Network format) source

static Host32                       // (Host format) resultant
   load32(                          // Load (Network format) data
     const Byte*       byte);       // (Network format) source

static Host64                       // (Host format) resultant
   load64(                          // Load (Network format) data
     const Byte*       byte);       // (Network format) source

static Host16                       // (Host format) resultant
   ntoh16(                          // Convert Net16 => Host16
     Net16             net16);      // (Network format) source

static Host32                       // (Host format) resultant
   ntoh32(                          // Convert Net32 => Host32
     Net32             net32);      // (Network format) source

static Host64                       // (Host format) resultant
   ntoh64(                          // Convert Net64 => Host64
     Net64             net64);      // (Network format) source

static void
   store16(                         // Store (Host format) data
     Host16            host16,      // (Host format) source
     Byte*             byte);       // (Network format) resultant

static void
   store32(                         // Store (Host format) data
     Host32            host32,      // (Host format) source
     Byte*             byte);       // (Network format) resultant

static void
   store64(                         // Store (Host format) data
     Host64            host64,      // (Host format) source
     Byte*             byte);       // (Network format) resultant
}; // class Network

#endif // NETWORK_H_INCLUDED
