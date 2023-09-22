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
//       2023/09/15
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_RFC7541_H_INCLUDED
#define _LIBPUB_RFC7541_H_INCLUDED

#include <cstdint>                  // For size_t, uint8_t, ...
#include <cstdlib>                  // For free
#include <string>                   // For std::string
#include <vector>                   // For std::vector

//----------------------------------------------------------------------------
//
// Namespace-
//       RFC7541
//
// Purpose-
//       HTTP/2 HPACK compression.
//
//----------------------------------------------------------------------------
namespace RFC7541 {                 // Class RFC7541, HTTP/2 HPACK compression
typedef uint8_t        octet;       // An 8-bit encoded character

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
enum                                // Table dimensions
{  STATIC_INDEX_DIM= 62             // Static index table size
}; // Table dimensions

//----------------------------------------------------------------------------
//
// Struct-
//       Index7541
//
// Purpose-
//       RFC7541 Index address space entry
//
//----------------------------------------------------------------------------
struct Index7541 {                  // RFC7541 Index address space entry
const char*            name;        // (NUL terminated) name
const char*            value;       // (NUL terminated) value
}; // struct Index7541

//----------------------------------------------------------------------------
//
// Struct-
//       RFC7541::Property
//       RFC7541::Properties
//
// Purpose-
//       Name/Value string pair container
//       Name/Value string pair vector
//
//----------------------------------------------------------------------------
struct Property {                   // Name/Value pair container
std::string            name;        // Property name
std::string            value;       // Property value
}; // class RFC7541::Property

typedef std::vector<Property>       Properties;

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Huff
//
// Purpose-
//       HTTP/2 HPACK Huffman compressed data container
//
//----------------------------------------------------------------------------
class Huff {
// RFC7541::Huff::Attributes - - - - - - - - - - - - - - - - - - - - - - - - -
private:
octet*                 addr= nullptr; // The compressed data adress
size_t                 size= 0;     // The compressed data length

// RFC7541::Huff::Constructors, destructor - - - - - - - - - - - - - - - - - -
public:
   Huff( void ) = default;          // The default constructor

   Huff(const std::string& s)       // Copy std::string constructor
{  *this= encode(s); }

   Huff(const Huff& h)              // Copy constructor
{  _copy(h); }

   Huff(Huff&& h)                   // Move constructor
:  addr(h.addr), size(h.size)
{  h.addr= nullptr; h.size= 0; }

   ~Huff( void )                    // Destructor
{  free(addr); addr= nullptr; size= 0; }

// RFC7541::Huff::Assignment operators - - - - - - - - - - - - - - - - - - - -
Huff& operator=(const std::string& s) // Assign from std::string
{  *this= encode(s); return *this; }

Huff& operator=(const Huff& h)      // Copy assignment
{  _copy(h); return *this; }

Huff& operator=(Huff&& h)           // Move assignment
{  free(addr); addr= h.addr; size=h.size; h.addr= nullptr; h.size= 0;
   return *this;
}

// RFC7541::Huff::Comparison operators - - - - - - - - - - - - - - - - - - - -
bool  operator==(const Huff& h) const; // Equality comparison
bool  operator!=(const Huff& h) const; // Inequality comparison

// RFC7541::Huff::Accessors- - - - - - - - - - - - - - - - - - - - - - - - - -
const octet*                        // The compressed data address
   get_addr( void ) const           // Get compressed data address
{  return addr; }

size_t                              // The compressed data length
   get_size( void ) const           // Get compressed data length
{  return size; }

// RFC7541::Huff::Methods- - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char* info= nullptr) const; // Debugging display

std::string                         // Resultant std::string
   decode( void ) const             // Decode (this) compressed string
{  return decode(addr, size); }

static std::string                  // Resultant std::string
   decode(                          // Decode a compressed string
     const octet*      addr,        // Compressed string address
     size_t            size);       // Compressed string length

static Huff                         // Resultant Huff
   encode(const std::string& s);    // Create from std::string

static size_t                       // Get encoded length (in bytes)
   encoded_length(const std::string& s); // For this std::string

// RFC7541::Huff::Internal methods - - - - - - - - - - - - - - - - - - - - - -
private:
void
   _copy(const Huff&);              // Copy from const Huff&
}; // class RFC7541::Huff

//----------------------------------------------------------------------------
//
// Class-
//       RFC7541::Pack
//
// Purpose-
//       HTTP/2 HPACK encoder/decoder
//
// Implementation notes-
//       Phase[0] prototype version
//
//       The maximum combined index and value size of the HPACK Property
//       index and value dictionary (defined by the Settings frame) is
//       UINT32_MAX.
//       Therefore uint32_t indexes and offsets may be freely used, EXCEPT
//       specification storage sizes need to handle arithmetic overflow
//       possibility so size_t values are used for them.
//
//----------------------------------------------------------------------------
class Pack {                        // HPACK encoder/decoder
// RFC7541::Pack::Typedefs and enumerations- - - - - - - - - - - - - - - - - -
public:
typedef uint32_t       IPack;       // (Maximum) Pack sized integer type
typedef IPack          Index;       // Name index
typedef IPack          Value;       // Value index/offset

// RFC7541::Pack::Attributes - - - - - - - - - - - - - - - - - - - - - - - - -
private: // Production
public:  // Phase[0]
// Phase[0]: index_size == value_size == encode_size; Cannot need expansion.
Index7541**            index_table= nullptr; // The index table [index_size]
IPack                  index_size= 0; // Allocated index table entry count
IPack                  index_used= 0; // Number of index table entries used
IPack                  index_ins= 0;  // Index table insert index
IPack                  index_old= 0;  // Index table oldest index

std::string**          value_table= nullptr; // The value table [value_size]
IPack                  value_size= 0; // Allocated value table entry count
IPack                  value_used= 0; // Number of value table entries used

// Specification defined storage usage values
size_t                 encode_size= 0;  // Total (virtual) storage available
size_t                 encode_used= 0;  // Total (virtual) storage used
size_t                 encode_index= 0; // Total index storage used (octets)
size_t                 encode_value= 0; // Total value storage used (octets)

// RFC7541::Pack::Constructors, destructor - - - - - - - - - - - - - - - - - -
public:
   Pack( void );                    // Default constructor (encode_size == 0)
   Pack(IPack);                     // Constructor, specifying encode_size
   Pack(const Pack&) = delete;      // No copy constructor
   Pack(Pack&&) = delete;           // No move constructor

   ~Pack( void );                   // Destructor

// RFC7541::Pack::Debugging methods- - - - - - - - - - - - - - - - - - - - - -
void
   debug(const char* info= nullptr) const; // Debugging display

// RFC7541::Pack::Assignment operators - - - - - - - - - - - - - - - - - - - -
Pack& operator=(const Pack&) = delete; // No copy assignment operator
Pack& operator=(Pack&&) = delete;   // No move assignment operator

// RFC7541::Pack::Comparison operators - - - - - - - - - - - - - - - - - - - -
bool  operator==(const Pack& h) const = delete; // No equality comparison
bool  operator!=(const Pack& h) const = delete; // No inequality comparison

// RFC7541::Pack::Accessor methods - - - - - - - - - - - - - - - - - - - - - -
// (None defined)

// RFC7541::Pack::Methods- - - - - - - - - - - - - - - - - - - - - - - - - - -
Properties
   decode(const std::string&);      // Decode packed data

std::string
   encode(const Properties&);       // Encode Properties

void
   resize(IPack);                   // Update the table size

// RFC7541::Pack::Internal methods - - - - - - - - - - - - - - - - - - - - - -
private:
IPack                               // Setting value
   get_setting(int) const;          // Get setting value
}; // class RFC7541::Pack

//----------------------------------------------------------------------------
//
// (Static) subroutine-
//       RFC7541::debug
//
// Purpose-
//       (Bringup) debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr); // Debugging display

//----------------------------------------------------------------------------
//
// (Static) utility subroutine-
//       RFC7541::dump_properties
//
// Purpose-
//       Dump Properties
//
//----------------------------------------------------------------------------
void
   dump_properties(const Properties&); // Dump Properties

//----------------------------------------------------------------------------
//
// (Static) utility subroutine-
//       RFC7541::load_properties
//
// Purpose-
//       Load Properties
//
//----------------------------------------------------------------------------
Properties                          // The Properties
   load_properties( void );         // Load Properties, parameter(s) TBD
}  // namespace RFC7541
#endif // _LIBPUB_RFC7541_H_INCLUDED
