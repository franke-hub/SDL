//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Frame.h
//
// Purpose-
//       HTTP Frame description.
//
// Last change date-
//       2022/10/16
//
// Implementation notes-
//       References: RFC7540, RFC7541, RFC8740
//       This file is internal to the pub library, ~/src/cpp/lib/pub.
//
//       TODO: VERIFY THAT THE PADDING LENGTH MAY BE ZERO (+1 vs +2)
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_FRAME_HPP_INCLUDED
#define _PUB_HTTP_FRAME_HPP_INCLUDED

#include <stdint.h>                 // For uint8_t, uint32_t, ...

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros
#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
//
// Struct-
//       Frame
//
// Purpose-
//       HTTP/2 Frame definition
//
// Implementation note: Padded data-
//       uint8_t size      (Padding length - 1)
//       uint8_t data[*]   (The actual data)
//       uint8_t pad[size] (Random padding)
//
// Implementation notes-
//       Users MUST insure flag fields are initialized as required:
//         The following methods depend upon (flag & F_PADDED)
//           get_payload_addr, get_payload_size, set_payload_size
//         Methods get_padding_addr and get_padding_size are only valid when
//           (flag & F_PADDED) is non-zero.
//
//----------------------------------------------------------------------------
struct Frame {                      // HTTP/2 Frame descriptor
uint8_t                length[3]= {0,0,0}; // Data length
uint8_t                type= 0;     // Frame type
uint8_t                flag= 0;     // Flags
uint8_t                stream[4]= {0,0,0,0}; // Stream ID

enum type_t                         // Frame type values
{  T_DATA=                     0x00 // Data frame
,  T_HEADERS=                  0x01 // Headers frame
,  T_PRIORITY=                 0x02 // Priority update
,  T_RST_STREAM=               0x03 // Reset Stream
,  T_SETTTINGS=                0x04 // Settings
,  T_PUSH_PROMISE=             0x05 // Push promise
,  T_PING=                     0x06 // Ping
,  T_GOAWAY=                   0x07 // Go away
,  T_WINDOW_UPDATE=            0x08 // Window update
,  T_CONTINUATION=             0x09 // Continuation
,  T_F0=                       0xF0 // (Reserved for experimental use)
,  T_FF=                       0xFF // (Reserved for experimental use)
}; // enum type_t

enum flag_t                         // Flags
{  F_NONE=                     0x00 // No flags set
,  F_ACK=                      0x01 // Acknowledgement
,  F_END_STREAM=               0x01 // End of stream
,  F_END_HEADERS=              0x04 // End of headers
,  F_PADDED=                   0x08 // Data padded
,  F_PRIORITY=                 0x20 // Priority information included
}; // enum flag_t

//----------------------------------------------------------------------------
// Frame::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The length
   get_length( void )               // Get length
{  return length[0] << 16 | length[1] << 8 | length[2]; }

void*                               // The padding address
   get_padding_addr( void )         // Get padding address
// if( !(flag & F_PADDED) ) throw "SNO"; // UNCHECKED, SHOULD NOT OCCUR
{  uint8_t* addr= (uint8_t*)this + sizeof(Frame) + get_length();
   return addr - ((uint8_t*)this)[sizeof(Frame)];
}

int                                 // The padding length
   get_padding_size( void )         // Get padding length
// if( !(flag & F_PADDED) ) throw "SNO"; // UNCHECKED, SHOULD NOT OCCUR
{  return ((uint8_t*)this)[sizeof(Frame)]; }

uint8_t*                            // The payload address
   get_payload_addr( void )         // Get payload address
{  uint8_t* addr= (uint8_t*)this + sizeof(Frame);
   if( flag & F_PADDED ) ++addr;
   return addr;
}

uint32_t                            // The payload length
   get_payload_size( void )         // Get payload length
{  uint32_t size= get_length();
   if( flag & F_PADDED ) size -= (((uint8_t*)this)[sizeof(Frame)] + 1);
   return size;
}

uint32_t                            // The Stream ID
   get_stream( void )               // Get Stream ID
{  uint32_t V= (stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3]);
   return V & 0x7FFFFFFF;          // MUST NOT return reserved bit
}

void
   set_padding_size(                // Set padding length
     int               V)           // Given this payload length
// if( !(flag & F_PADDED) ) throw "SNO"; // UNCHECKED, SHOULD NOT OCCUR
// if( V > 255 ) throw "SNO";       // UNCHECKED, SHOULD NOT OCCUR
{  ((uint8_t*)this)[sizeof(Frame)]= V; }

void
   set_payload_size(                // Set the Frame length
     uint32_t          V)           // Given this payload length
{  if( flag & F_PADDED ) V += (((uint8_t*)this)[sizeof(Frame)] + 1);
   set_length(V);
}

void
   set_length(                      // Set length
     uint32_t          V)           // To this value
// if( V >= 0x01000000 ) throw "SNO"; // UNCHECKED, SHOULD NOT OCCUR
{  length[2]= V;
   length[1]= V >> 8;
   length[0]= V >> 16;
}

void
   set_stream(                      // Set stream ID
     uint32_t          V)           // To this value
{  V &= 0x7FFFFFFF;                 // MUST NOT set reserved bit
   stream[3]= V;
   stream[2]= V >> 8;
   stream[1]= V >> 16;
   stream[0]= V >> 24;
}
}; // struct Frame

//----------------------------------------------------------------------------
//
// Struct-
//       FrameContinue
//
// Purpose-
//       HTTP/2 Frame type T_CONTINUATION payload
//
// Implementation notes-
//       FLAGS: END_HEADERS
//
//----------------------------------------------------------------------------
struct FrameContinue {              // Frame type T_CONTINUATION payload
char                   hbf[0];      // Header Block Fragment, see RFC7541

//----------------------------------------------------------------------------
// FrameContinue::Accessors
//----------------------------------------------------------------------------
// NOT CODED YET                    // Follows T_HEADERS, T_PUSH_PROMISE
}; // struct FrameContinue

//----------------------------------------------------------------------------
//
// Struct-
//       FrameData
//
// Purpose-
//       HTTP/2 Frame type T_DATA payload
//
// Implementation notes-
//       FLAGS: F_END_STREAM, F_PADDED
//
//----------------------------------------------------------------------------
struct FrameData {                  // Frame type T_DATA payload
uint8_t                payload[0];  // The (variable length) Frame payload
}; // struct FrameData

//----------------------------------------------------------------------------
//
// Struct-
//       FrameEC
//
// Purpose-
//       Define the error codes for Frame types T_GOAWAY and T_RST_STREAM
//
// Implementation notes-
//       Implementations MUST NOT trigger special behavior for unknown codes.
//
//----------------------------------------------------------------------------
struct FrameEC {                    // Error code (defined) values
enum code_t                         // Error code values
{  NO_ERROR=                 0x0000 // No error occurred
,  PROTOCOL_ERROR=           0x0001 // Protocol error detected
,  INTERNAL_ERROR=           0x0002 // Implemtation fault
,  FLOW_CONTROL_ERROR=       0x0003 // Flow-control limits exceeded
,  SETTINGS_TIMEOUT=         0x0004 // Settings not acknowledged
,  STREAM_CLOSED=            0x0005 // Steam closed (normally)
,  FRAME_SIZE_ERROR=         0x0006 // Frame size incorrect
,  REFUSED_STREAM=           0x0007 // Stream not processed
,  CANCEL=                   0x0008 // Stream cancelled
,  COMPRESSION_ERROR=        0x0009 // Compression state not updated
,  CONNECT_ERROR=            0x000A // TCP Connection error for CONNECT method
,  ENHANCE_YOUR_CALM=        0x000B // Processing capability exceeded
,  INADEQUATE_SECURITY=      0x000C // Negotiated TLS parameters not acceptabe
,  HTTP_1_1_REQUIRED=        0x000D // Use HTTP/1.1 for the request
}; // enum code_t
}; // struct FrameEC

//----------------------------------------------------------------------------
//
// Struct-
//       FrameGoaway
//
// Purpose-
//       HTTP/2 Frame type T_GOAWAY payload
//
// Implementation notes-
//       FLAGS: F_NONE
//
//----------------------------------------------------------------------------
struct FrameGoaway : public FrameEC { // Frame type T_GOAWAY payload
uint8_t                stream[4]= {0,0,0,0}; // Stream ID
uint8_t                code[4]= {0,0,0,0}; // Error code
uint8_t                debug[0];    // Additional debugging data

//----------------------------------------------------------------------------
// FrameGoaway::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The error code
   get_code( void )                 // Get error code
{  return code[0] << 24 | code[1] << 16 | code[2] << 8 | code[3]; }

uint32_t                            // The Stream ID
   get_stream( void )               // Get Stream ID
{  uint32_t V= (stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3]);
   return V & 0x7FFFFFFF;          // MUST NOT return reserved bit
}

void
   set_code(                        // Set error code
     uint32_t          V)           // To this value
{  code[3]= V;
   code[2]= V >> 8;
   code[1]= V >> 16;
   code[0]= V >> 24;
}

void
   set_stream(                      // Set stream ID
     uint32_t          V)           // To this value
{  V &= 0x7FFFFFFF;                 // MUST NOT set reserved bit
   stream[3]= V;
   stream[2]= V >> 8;
   stream[1]= V >> 16;
   stream[0]= V >> 24;
}
}; // struct FrameGoaway

//----------------------------------------------------------------------------
//
// Struct-
//       FrameHeaders
//
// Purpose-
//       HTTP/2 Frame type T_HEADERS payload
//
// Implementation notes-
//       FLAGS: F_END_HEADERS, F_END_STREAM, F_PADDED, F_PRIORITY
//
//----------------------------------------------------------------------------
struct FrameHeaders {               // Frame type T_HEADERS payload
char                   hbf[0];      // Header Block Fragment, see RFC7541

//----------------------------------------------------------------------------
// FrameHeaders::Accessors
//----------------------------------------------------------------------------
// NOT CODED YET
}; // struct FrameHeaders

//----------------------------------------------------------------------------
//
// Struct-
//       FramePing
//
// Purpose-
//       HTTP/2 Frame type T_PING payload
//
// Implementation notes-
//       FLAGS: F_ACK
//       MUST use stream 0.
//
//----------------------------------------------------------------------------
struct FramePing {                  // Frame type T_PING payload
char                   data[0];     // Opaque data, length 8..64
}; // struct FramePing

//----------------------------------------------------------------------------
//
// Struct-
//       FramePriority
//
// Purpose-
//       HTTP/2 Frame type T_PRIORITY payload
//
// Implementation notes-
//       FLAGS: F_NONE
//       FramePriority is optionally included in FrameHeaders.
//
//----------------------------------------------------------------------------
struct FramePriority {              // Frame type T_PRIORITY payload
enum depend_t                       // Dependency type
{  EXCLUSIVE= 0x80000000            // Indicates exclusive dependency
,  STREAM_ID= 0x7FFFFFFF            // Stream ID mask
}; // enum depend_t
uint8_t                depend[4]= {0,0,0,0}; // Stream ID (+EXCLUSIVE)
uint8_t                weight= {0}; // Priority weight

//----------------------------------------------------------------------------
// FramePriority::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The Dependency stream
   get_depend( void )               // Get Dependency stream
{  return depend[0] << 24 | depend[1] << 16 | depend[2] << 8 | depend[3]; }

int                                 // The priority weight
   get_weight( void )               // Get priority weight
{  return (weight + 1); }

void
   set_depend(                      // Set dependency stream
     uint32_t          V)           // To this value
{  depend[3]= V;
   depend[2]= V >> 8;
   depend[1]= V >> 16;
   depend[0]= V >> 24;
}

void
   set_weight(                      // Set priority weight
     int               V)           // To this value
// if( V < 1 ) V= 1; else if( V > 256 ) V= 256; // (UNCHECKED)
{  weight= V - 1; }
}; // struct FramePriority

//----------------------------------------------------------------------------
//
// Struct-
//       FramePromise
//
// Purpose-
//       HTTP/2 Frame type T_PUSH_PROMISE payload
//
// Implementation notes-
//       FLAGS: END_HEADERS, F_PADDED
//
//----------------------------------------------------------------------------
struct FramePromise {               // Frame type T_PUSH_PROMISE payload
uint8_t                stream[4]= {0,0,0,0}; // Stream ID
char                   hbf[0];      // Header Block Fragment, see RFC7541

//----------------------------------------------------------------------------
// FramePromise::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The Stream ID
   get_stream( void )               // Get Stream ID
{  uint32_t V= (stream[0] << 24 | stream[1] << 16 | stream[2] << 8 | stream[3]);
   return V & 0x7FFFFFFF;          // MUST NOT return reserved bit
}

void
   set_stream(                      // Set stream ID
     uint32_t          V)           // To this value
{  V &= 0x7FFFFFFF;                 // MUST NOT set reserved bit
   stream[3]= V;
   stream[2]= V >> 8;
   stream[1]= V >> 16;
   stream[0]= V >> 24;
}
}; // struct FramePromise

//----------------------------------------------------------------------------
//
// Struct-
//       FrameReset
//
// Purpose-
//       HTTP/2 Frame type T_RST_STREAM payload
//
// Implementation notes-
//       FLAGS: F_NONE
//
//----------------------------------------------------------------------------
struct FrameReset : public FrameEC { // Frame type T_RST_STREAM payload
uint8_t               code[4]= {0,0,0,0}; // Error code

//----------------------------------------------------------------------------
// FrameReset::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The error code
   get_code( void )                 // Get error code
{  return code[0] << 24 | code[1] << 16 | code[2] << 8 | code[3]; }

void
   set_code(                        // Set error code
     uint32_t          V)           // To this value
{  this->code[3]= V;
   this->code[2]= V >> 8;
   this->code[1]= V >> 16;
   this->code[0]= V >> 24;
}
}; // struct FrameReset

//----------------------------------------------------------------------------
//
// Struct-
//       FrameSettings
//
// Purpose-
//       HTTP/2 Frame type T_SETTINGS payload
//
// Implementation notes-
//       See 'struct Settings', below
//
//----------------------------------------------------------------------------
struct FrameSettings {              // Frame type T_SETTINGS payload
uint8_t                ident[2];    // Registry identifier
uint8_t                value[4];    // Value

enum id_t                           // Registry identifiers
{  S_INVALID=                0x0000 // (Invalid identifier)
,  S_HEADER_TABLE_SIZE=      0x0001 // Header table size
,  S_ENABLE_PUSH=            0x0002 // Enable push
,  S_MAX_CONCURRENT_STREAMS= 0x0003 // Maximum concurrent streams
,  S_INITIAL_WINDOW_SIZE=    0x0004 // Initial window size
,  S_MAX_FRAME_SIZE=         0x0005 // Maximum frame size
,  S_MAX_HEADER_LIST_SIZE=   0x0006 // Maximum header list size
,  S_MAX_SETTINGS                   // Number of registry identifiers
}; // enum id_t

enum default_t                      // Registry identifier default values
{  D_HEADER_TABLE_SIZE= 4096        // Header table size
,  D_ENABLE_PUSH= 1                 // Enable push
,  D_MAX_CONCURRENT_STREAMS= -1     // Maximum concurrent streams
,  D_INITIAL_WINDOW_SIZE= 65535     // Initial window size
,  D_MAX_FRAME_SIZE= 16384          // Maximum frame size
,  D_MAX_HEADER_LIST_SIZE= -1       // Maximum header list size
}; // enum default_t

//----------------------------------------------------------------------------
// FrameSettings::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The registry identifier
   get_ident( void )                // Get registry identifier
{  return ident[0] <<  8 | ident[1]; }

uint32_t                            // The value
   get_value( void )                // Get value
{  return value[0] << 24 | value[1] << 16 | value[2] << 8 | value[3]; }

void
   set_ident(int X, uint32_t V)     // Set ident
{  ident[1]= V; ident[0]= V >> 8; }

void
   set_value(uint32_t V)            // Set value
{  value[0]= V >> 24; value[1]= V >> 16; value[2]= V >> 8; value[3]= V; }
}; // struct FrameSettings

//----------------------------------------------------------------------------
//
// Struct-
//       FrameUpdate
//
// Purpose-
//       HTTP/2 Frame type T_WINDOW_UPDATE payload
//
// Implementation notes-
//       FLAGS: F_NONE
//
//----------------------------------------------------------------------------
struct FrameUpdate {                // Frame type T_WINDOW_UPDATE payload
uint8_t               size[4]= {0,0,0,0}; // Window size increment, 1..2^31-1

//----------------------------------------------------------------------------
// FrameUpdate::Accessors
//----------------------------------------------------------------------------
uint32_t                            // The window size increment
   get_size( void )                 // Get window size increment
{  uint32_t V= size[0] << 24 | size[1] << 16 | size[2] << 8 | size[3];
   return V & 0x7FFFFFFF;
}

void
   set_size(                        // Set window size increment
     uint32_t          V)           // To this value
{  V &= 0x7FFFFFFF;
   this->size[3]= V;
   this->size[2]= V >> 8;
   this->size[1]= V >> 16;
   this->size[0]= V >> 24;
}
}; // struct FrameUpdate

//----------------------------------------------------------------------------
//
// Struct-
//       Settings
//
// Purpose-
//       HTTP/2 Settings table
//
// Implementation notes-
//       See 'struct FrameSettings', above
//
//----------------------------------------------------------------------------
struct Settings {                   // Settings value table
typedef uint32_t       Value_t;     // A settings value

Value_t                setting[FrameSetting::S_MAX_SETTINGS]; // Settings array

//----------------------------------------------------------------------------
// Settings::Accessors
//----------------------------------------------------------------------------
Value_t                             // The setting[X].value
   get_value(int X)                 // Get setting[X].value
{  return setting[X]; }

void
   set_value(int X, Value_t V)      // Set setting[X].value
{  setting[X]= V; }
}; // struct Settings
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _PUB_HTTP_FRAME_HPP_INCLUDED
