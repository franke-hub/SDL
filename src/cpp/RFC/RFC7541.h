//----------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       RFC7541.h
//
// Purpose-
//       RFC7541, HTTP/2 HPACK compression
//
// Last change date-
//       2023/09/04
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_RFC7541_H_INCLUDED
#define _LIBPUB_RFC7541_H_INCLUDED

#include <cstdint>                  // For size_t, uint8_t, ...
#include <cstdlib>                  // For free
#include <string>                   // For std::string

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541
//
// Purpose-
//       HTTP/2 HPACK compression.
//
//----------------------------------------------------------------------------
class RFC7541 {                     // Class RFC7541, HTTP/2 HPACK compression
public:
typedef uint8_t        octet;       // An 8-bit encoded character

public:
//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Huff
//
// Purpose-
//       HTTP/2 HPACK compressed data container
//
//----------------------------------------------------------------------------
class Huff { //= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
friend class RFC7541;
// Attributes- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
private:
octet*                 addr= nullptr; // The compressed data adress
size_t                 size= 0;     // The compressed data length

// RFC7541::Huff::Constructors, destructor - - - - - - - - - - - - - - - - - -
public:
   Huff( void ) = default;          // The default constructor

   Huff(const std::string& s)       // Copy std::string constructor
{  *this= RFC7541::encode(s); }

   Huff(const Huff& h)              // Copy constructor
{  _copy(h); }

   Huff(Huff&& h)                   // Move constructor
:  addr(h.addr), size(h.size)
{  h.addr= nullptr; h.size= 0; }

   ~Huff( void )                    // Destructor
{  free(addr); addr= nullptr; size= 0; }

// RFC7541::Huff::Assignment operators - - - - - - - - - - - - - - - - - - - -
Huff& operator=(const std::string& s) // Assign from std::string
{  *this= RFC7541::encode(s); return *this; }

Huff& operator=(const Huff& h)      // Copy assignment
{  _copy(h); return *this; }

Huff& operator=(Huff&& h)           // Move assignment
{  free(addr); addr= h.addr; size=h.size; h.addr= nullptr; h.size= 0;
   return *this;
}

// Comparison operators
bool  operator==(const Huff& h) const; // Equality comparison
bool  operator!=(const Huff& h) const; // Inequality comparison

// RFC7541::Huff::Accessors- - - - - - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char* info= nullptr) const; // Debugging display

std::string                         // Resultant std::string
   decode( void ) const             // Decode (this) compressed string
{  return RFC7541::decode(addr, size); }

const octet*                        // The compressed data address
   get_addr( void ) const           // Get compressed data address
{  return addr; }

size_t                              // The compressed data length
   get_size( void ) const           // Get compressed data length
{  return size; }

protected:
void
   _copy(const Huff&);              // Copy from const Huff&
}; // class RFC7541::Huff= = = = = = = = = = = = = = = = = = = = = = = = = = =

//----------------------------------------------------------------------------
// RFC7541 Constructors, destructor
//----------------------------------------------------------------------------
public:
   RFC7541( void ) = default;
   ~RFC7541( void ) = default;

//----------------------------------------------------------------------------
// RFC7541 Methods
//----------------------------------------------------------------------------
static void
   debug(const char* info= nullptr); // Debugging display

static std::string                  // Resultant std::string
   decode(const octet* addr, size_t size); // Decode compressed string

static std::string                  // Resultant std::string
   decode(const Huff& h)            // Decode compressed string
{  return decode(h.get_addr(), h.get_size()); }

static Huff                         // Resultant Huff
   encode(const std::string& s);    // Create from std::string

static size_t                       // Get encoded length (in bytes)
   encoded_length(const std::string& s); // For this std::string
}; // class RFC7541
#endif // _LIBPUB_RFC7541_H_INCLUDED
