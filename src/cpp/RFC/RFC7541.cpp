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
//       RFC7541.cpp
//
// Purpose-
//       RFC7541, HTTP/2 HPACK compression
//
// Last change date-
//       2023/10/13
//
//----------------------------------------------------------------------------
#include <cassert>                  // For assert
#include <cstdint>                  // For uint32_t, uint16_t, ...
#include <cstring>                  // For memcpy, memcmp, ...
#include <cctype>                   // For isprint
#include <functional>               // For std::hash<std::string>
#include <new>                      // For std::bad_alloc, placement new
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <pub/Must.h>               // For pub::must::strdup
#include <pub/utility.h>            // For pub::utility::dump
#include <pub/Debug.h>              // For pub::debugf, ...

#include "RFC7541.h"                // For class RFC7541, implemented

// Namespace accessors
#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub objects
using namespace PUB::debugging;     // For debugging subroutines

typedef RFC7541::octet octet;       // Import octet, for subroutines

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Hash table size definitions
,  HASH_MASK= 0x0000003F            // Hash table index mask
,  HASH_SIZE= 64                    // Size of hash table (power of 2)

,  STATIC_ENTRY_DIM= 62             // Predefined entry table size
,  USE_CHECKING= true               // Enable checks (or check debugging)?
}; // (generic) enum

enum                                // Octet constants
{  BITS_PER_OCTET= 8                // Number of bits in an octet
,  BITS_USED_MASK= 7                // Number of bits used in last byte
,  LOG2_PER_OCTET= 3                // Log2(BITS_PER_OCTET)
}; // Octet constants

//----------------------------------------------------------------------------
// Internal objects and tables
//----------------------------------------------------------------------------
#include "RFC7541.hpp"              // Internal objects and tables

// Fill validation table
static const int       fill_table[8]= { 0, 1, 3, 7, 15, 31, 63, 127};

namespace RFC7541 {                 // Implementation namespace
//----------------------------------------------------------------------------
// Globals
//----------------------------------------------------------------------------
// Operational controls
int                    Pack::hcdm= 0; // Hard Core Debug Mode
int                    Pack::verbose= 0; // Debugging verbosity

//----------------------------------------------------------------------------
//
// Extern-
//       RFC7541::type_to_bits
//       RFC7541::type_to_mask
//
// Purpose-
//       RFC7541::get_encode_bits data area
//       RFC7541::get_encode_mask data area
//
//----------------------------------------------------------------------------
octet type_to_bits[8]= {   7,    0,    6,    5,    0,    4,    0,    4};
octet type_to_mask[8]= {0x80, 0x40, 0x40, 0x20, 0x10, 0x10, 0x00, 0x00};

//----------------------------------------------------------------------------
// Convert the input octet into an ENCODE_TYPE
ENCODE_TYPE
   get_encode_type(int C)           // Convert input octet into ENCODE_TYPE
{
   if( C == 0x0080 ) {
     if( USE_CHECKING )
       debugf("%4d %s Disallowed encoding: 0x80\n", __LINE__, __FILE__);
     throw connection_error("Disallowed encoding: 0x80");
   }
   if( C == 0x0040 )
     return ET_INSERT_NOINDEX;
   if( C == 0x0010 )
     return ET_NEVER_NOINDEX;
   if( C == 0x0000 )
     return ET_CONST_NOINDEX;

   if( C & 0x0080 )                 // (DO NOT change change testing order)
     return ET_INDEX;
   if( C & 0x0040 )
     return ET_INSERT;
   if( C & 0x0020 )
     return ET_RESIZE;
   if( C & 0x0010 )
     return ET_NEVER;
   return ET_CONST;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_bits
//
// Purpose-
//       Convert value to bit string (used for debugging display)
//
//----------------------------------------------------------------------------
#if 0  // This works OK, it's just unused
static inline std::string
   to_bits(                              // Convert to bit string
     long              value,            // Value to convert
     int               width)            // Number of bits to convert
{
   std::string out_string;
   while( width > 0 ) {
     if( (value >> (width - 1)) & 0x00000001 )
       out_string += '1';
     else
       out_string += '0';
     --width;
   }

   return out_string;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Huff::debug(const char* info) const // Debugging display
{
   debugf("Huff(%p).debug(%s) {%p,%zd}\n", this, info, addr, size);
   pub::utility::dump(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::operator==
//       RFC7541::Huff::operator!=
//
// Purpose-
//       Equality comparison operator
//       Inequality comparison operator
//
//----------------------------------------------------------------------------
bool
   Huff::operator==(const Huff& h) const // Equality comparison
{  if( HCDM )
     debugf("Huff(%p)::op==(const Huff&(%p))\n", this, &h);

   if( addr == h.addr && size == h.size ) // Self or empty comparison
     return true;

   if( size == h.size ) {           // If size is the same (won't be zero)
     if( memcmp(addr, h.addr, h.size) == 0 ) // If values are identical
       return true;
   }

   return false;
}

bool
   Huff::operator!=(const Huff& h) const // Inequality comparison
{  return !operator==(h); }

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::copy
//
// Purpose-
//       Copy method
//
//----------------------------------------------------------------------------
void
   Huff::copy(                      // Copy from
      const Huff&      h)           // This Huff
{  if( HCDM )
     debugf("Huff(%p)::copy(Huff(%p) {%p,%zd}) addr(%p)\n", this
           , &h, h.addr, h.size, addr);

   free(addr);
   if( h.size ) {
     addr= (octet*)malloc(h.size);
     if( addr == nullptr ) {
       size= 0;
       throw bad_alloc();
     }
     memcpy(addr, h.addr, h.size);
   }

   size= h.size;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::decode(const octet*, size_t)
//
// Purpose-
//       Decode compressed string
//
//----------------------------------------------------------------------------
string                              // The decompressed string
   Huff::decode(                    // Decode buffer
      const octet*     addr,        // Buffer address
      size_t           size)        // Buffer length
{  if( HCDM ) debugf("RFC7541::Huff::decode(%p,%zd)\n", addr, size);

   enum {BUFF_DIM= 15};             // The output buffer size
   size_t              inp_index= 0; // The input buffer index
   string              out_string;  // The output string
   char                out_buffer[BUFF_DIM+1]; // The output buffer
   int                 out_index= 0; // The output buffer index

   // DECODE -----------------------------------------------------------------
   enum {ACC_WIDTH= 64};            // Accumulator width, in bits
   uint64_t accumulator= 0;         // Accumulator
   int      acc_index= 0;           // Accumulator bit index
   bool     end_of_string= false;   // End of string encountered

   while( inp_index <= size ) {
     // Test character in range
     for(int index_ix= 0; index_ix<DECODE_INDEX_DIM; ++index_ix) {
       const Bits7541& B= decode_index[index_ix];

       // Implementation note: If the number of bits between code points
       // increases more than BITS_PER_OCTET between values, use:
       // while( B.bits > acc_index ) instead of if( ... )
       if( B.bits > acc_index ) {   // If more accumulator bits are needed
         if( inp_index >= size ) {
           // When bits between code_points can be > BITS_PER_OCTET, use:
           //   if( acc_index > BITS_PER_OCTET )
           //     throw std::runtime_error("encoding error");
           end_of_string= true;
           break;
         }

         accumulator <<= BITS_PER_OCTET;
         accumulator  |= addr[inp_index++];
         acc_index += BITS_PER_OCTET;
       }

       // The accumulator has enough bytes to test
       uint64_t decumulator= (accumulator >> (acc_index - B.bits));
       decumulator <<= (ACC_WIDTH - B.bits); // Clear high order bits
       decumulator >>= (ACC_WIDTH - B.bits);

       if( decumulator>B.max_encode ) { // If more bits required
         if( index_ix == DECODE_INDEX_DIM ) // If at end of table
           throw std::runtime_error("encoding error: size");
         continue;
       } else {                     // We have enough bits
         int index= decumulator - B.min_encode;
         if( index < 0 )            // If invalid encoding
           throw std::runtime_error("encoding error: value");
         index += B.min_index;

         if( out_index >= BUFF_DIM ) {
           string addend(out_buffer, BUFF_DIM);
           out_string += addend;
           out_index= 0;
         }
         out_buffer[out_index++]= decode_table[index].decode;

         acc_index -= B.bits;
         break;
       }
     }
     if( end_of_string )
       break;
   }

   if( out_index ) {                // Add remainder
     string addend(out_buffer, out_index);
     out_string += addend;
   }

   if( acc_index ) {
     accumulator <<= (ACC_WIDTH - acc_index); // Zero any removed bits
     accumulator >>= (ACC_WIDTH - acc_index);
   } else {
     accumulator= 0;                // (accum <<= ACC_WIDTH) == accum
   }
   if( acc_index > 7 || accumulator != (uint64_t)fill_table[acc_index] )
     throw std::runtime_error("encoding error: fill");

   return out_string;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::encode(Writer&, const string&)
//
// Purpose-
//       Encode string (including length prefix)
//
// Implementation note:
//       Phase-0 implementation
//
//----------------------------------------------------------------------------
void
   Huff::encode(                    // Encode string (ONLY)
     Writer&           writer,      // (OUTPUT) Writer Ioda
     const string&     S)           // The string to encode
{
   Huff huff(S);
   Integer::encode(writer, huff.size);
   writer.write(huff.addr, huff.size);
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::encode(const string&)
//
// Purpose-
//       Encode string
//
//----------------------------------------------------------------------------
Huff                                // The encoded string
   Huff::encode(                    // Encode
      const string&    s)           // This string
{  if( HCDM ) debugf("RFC7541::Huff::encode(%s)\n", s.c_str());

   Huff h;
   size_t size= encoded_length(s);  // The encoded length
   if( size == 0 )                  // If empty string
     return h;                      // Use empty encoding

   h.addr= (octet*)malloc(size);    // Allocate octet buffer
   if( h.addr == nullptr )
     throw bad_alloc();
   h.size= size;

   // ENCODE -----------------------------------------------------------------
   enum {ACC_WIDTH= 64};            // Accumulator width, in bits
   uint64_t accumulator= 0;         // Accumulator
   int      acc_index= 0;           // Accumulator bit index
   size_t   out_index= 0;           // Resultant (h.data) byte index

   for(auto it= s.cbegin(); it != s.cend(); ++it) {
     const Huff7541& H= encode_table[(octet)*it];
     //const octet O= (octet)*it;     // The next octet
     //const int   bits= encode_table[O].bits; // The encoded octet size, in bits
     if( (acc_index + H.bits) > ACC_WIDTH ) { // If accumulator dump required
       while( acc_index > BITS_PER_OCTET ) { // While full bytes exist
         h.addr[out_index++]= (accumulator >> (acc_index - BITS_PER_OCTET));
         acc_index -= BITS_PER_OCTET;
       }
     }

     accumulator <<= H.bits;
     accumulator  |= H.encode;
     acc_index += H.bits;
   }

   // Flush accumulator bytes, if present
   while( acc_index >= BITS_PER_OCTET ) { // While full bytes exist
     h.addr[out_index++]= (accumulator >> (acc_index - BITS_PER_OCTET));
     acc_index -= BITS_PER_OCTET;
   }

   // Handle last character fill
   if( acc_index ) {
     accumulator <<= BITS_PER_OCTET;
     accumulator  |= 0x000000FF;
     accumulator >>= acc_index;
     h.addr[out_index++]= (octet)accumulator;
   }

   // Internal cross-check: encoded_length() correctness
   if( USE_CHECKING && out_index != size ) { // Correctness check
     debugf("%4d %s SHOULD NOT OCCUR%s\n", __LINE__, __FILE__
           , out_index > size ? " - BUFFER OVERFLOW" : "");
     debugf("..out_index(%zd) ..size(%zd)\n", out_index, size);
     throw connection_error("out_index != size");
   }

   return h;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Huff::encoded_length
//
// Purpose-
//       Get the encoded length of a string (in bytes)
//
//----------------------------------------------------------------------------
size_t                              // The encoded length
   Huff::encoded_length(            // Get encoded length
      const string&    s)           // Of this string
{
   size_t size= 0;                  // The encoded length (in bits)

   for(auto it= s.cbegin(); it != s.cend(); ++it) {
     octet c= (octet)*it;
     size += encode_table[c].bits;
   }

   size  += BITS_USED_MASK;         // Account for fractional byte
   size >>= LOG2_PER_OCTET;         // Size in bytes

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::Pack
//       RFC7541::Pack::~Pack
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Pack::Pack( void )               // Default constructor
:  entry_array()
{  if( HCDM )                       // (Is really = default)
     debugf("Pack(%p)::Pack()\n", this);

   init();
   resize(65536);                   // Use default pack size
}

   Pack::Pack(Value_t size)         // Table size constructor
:  entry_array()
{  if( HCDM )
     debugf("Pack(%p)::Pack(%u)\n", this, size);

   init();
   resize(size);
}

   Pack::~Pack( void )              // Destructor
{  if( HCDM )
     debugf("Pack(%p)::~Pack()\n", this);

   resize(0);
   term();
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::init
//       RFC7541::Pack::term
//
// Purpose-
//       Initialize
//       Terminate
//
//----------------------------------------------------------------------------
void Pack::init(void)               // Initialize
{
   encode_size= DEFAULT_TABLE_SIZE;
   entry_size= encode_size / SPEC_ENTRY_SIZE;
   size_t
   size= sizeof(Entry*) * entry_size;
   entry_array= (Entry**)must::malloc(size);
   memset(entry_array, 0, size);

   entry_used= 0;                   // Number of Entries in use
   entry_ins= 1;                    // Current insert Array_ix
   entry_old= 1;                    // Current oldest Array_ix
}

void Pack::term(void)               // Terminate
{
   free(entry_array);
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::debug
//
// Purpose-
//       Debugging display
//
// Implementation notes-
//       The hcdm version verifies entix2entry(Entry_ix) and its inverse
//       entry2entix(Entry*) method.
//       The same sort of thing needs to be done for entry_ins < entry_old.
//       (When entry_ins == entry_old, entry_used == 0)
//
//----------------------------------------------------------------------------
void
   Pack::debug(const char* info) const // Debugging display
{
   debugf("Pack(%p).debug(%s)\n", this, info);
   debugf("..used %u of %u\n", get_encode_used(), get_encode_size());
   debugf("..entry_used(%u) entry_old(%u) entry_ins(%u) entry_size(%u)\n"
         , entry_used, entry_old, entry_ins, entry_size);

   for(Index_ix index_ix= entry_size; index_ix > 0; --index_ix) {
     Array_ix array_ix= entry_size - index_ix + 1;
     Entry* entry= entry_array[index_ix - 1];

     if( entry ) {
       Entry_ix entry_ix= 0;
       try {
         entry_ix= entry2entix(entry);
         if( array_ix != entry->index ) {
           if( USE_CHECKING )
             debugf("%4d cpp [%2u] [%2u]%p[%2u]: array_ix != entry->index\n"
                   , __LINE__ , index_ix, array_ix, entry, entry->index);
           throw connection_error("consistency fault");
         }
         if( entry != entix2entry(entry_ix) ) {
           if( USE_CHECKING )
             debugf("%4d cpp [%2u] [%2u]%p[%2u] [%u]: %p != %p= ix2entry(%u)\n"
                   , __LINE__ , index_ix, array_ix, entry, entry->index
                   , entry_ix, entry, entix2entry(entry_ix), entry_ix);
           throw connection_error("consistency fault");
         }
       } catch(std::exception& X) {
         if( USE_CHECKING )
           debugf("%4d cpp [%2u] [%2u]%p[%2u]: Exception(%s)\n", __LINE__
                 , index_ix, array_ix, entry, entry->index, X.what());
         throw X;
       }
       debugf("[%2u] [%2u]%p[%2u] [%2u] '%s': '%s'\n"
             , index_ix - 1, array_ix, entry, entry->index, entry_ix
             , entry->name, entry->value);
     } else {
       debugf("[%2u] [%2u]nullptr\n", index_ix - 1, array_ix);
     }
   }

   if( hcdm && verbose > 1 )
     entry_map.debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::decode
//
// Purpose-
//       Decode packed data
//
//----------------------------------------------------------------------------
Properties                          // Decoded Properties
   Pack::decode(                    // Decode Properties
     Reader&           reader)      // (INPUT) IodaReader
{  if( HCDM ) debugf("Pack(%p).decode\n", this);

   Properties properties;

   size_t length= reader.get_length();
   while( length ) {                // Decode the header
     int C= reader.peek();
     if( C == EOF )
       break;

     ENCODE_TYPE et= get_encode_type(C);
     // debugf("%4d cpp C(0x%.2x) et(%d)\n", __LINE__, C, et);
     switch( et ) {
       case ET_INDEX:               // Name and value (already) indexed
       { Entry_ix entry_ix= Integer::decode(reader, 7);
         const Entry* entry= entix2entry(entry_ix);
         assert( entry != nullptr && entry->value != nullptr );

         // Name/value are NOT Huffman encoded
         properties.append(entry->name, entry->value, ET_INDEX);
         if( verbose )
           debugf("Decode: '%s': '%s' ET_INDEX\n", entry->name, entry->value);
         break;
       }
       case ET_INSERT:              // Indexed name, literal value, insert
       case ET_NEVER:               // Indexed name, literal value, const table
       case ET_CONST:               // Indexed name, literal value, const table
       { Entry_ix entry_ix= Integer::decode(reader, type_to_bits[et]);
         const Entry* entry= entix2entry(entry_ix);
         assert( entry != nullptr );

         string name= entry->name;
         string value= string_decode(reader);

         Property property(name, value, et);
         properties.append(property);
         if( verbose )
           debugf("Decode: '%s': '%s' %s\n", name.c_str(), value.c_str()
                 , type_to_name[et]);
         if( et == ET_INSERT ) {
           insert(property);
           if( verbose )
             debugf("Insert: '%s': '%s' ET_INSERT\n"
                   , name.c_str(), value.c_str());
         }
         break;
       }
       case ET_INSERT_NOINDEX:      // Literal name and value, insert
       case ET_CONST_NOINDEX:       // Literal name and value, const table
       case ET_NEVER_NOINDEX:       // Literal name and value, const table
       { reader.get();              // (Consume the ENCODE_TYPE character)
         bool n_encoded= (reader.peek()  & 0x80);
         string name=  string_decode(reader);
         bool v_encoded= (reader.peek()  & 0x80);
         string value= string_decode(reader);
         Property property(name, value, et, n_encoded, v_encoded);
         properties.append(property);
         if( verbose )
           debugf("Decode: '%s': '%s' %s\n", name.c_str(), value.c_str()
                 , type_to_name[et]);

         if( et == ET_INSERT_NOINDEX ) {
           insert(property);
           if( verbose )
             debugf("Insert: '%s': '%s' %s\n", name.c_str(), value.c_str()
                   , type_to_name[et]);
         }
         break;
       }
       case ET_RESIZE:              // Resize operation (MUST precede others)
       { throw std::runtime_error("ET_RESIZE NOT CODED YET");
         break;
       }
       default:
       { debugf("%4d %s C(0x%.2x) et(%d)\n", __LINE__, __FILE__, C, et);
         throw std::runtime_error("SHOULD NOT OCCUR");
       }
     }
   }

   return properties;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::encode
//
// Purpose-
//       Encode Properties
//
// Implementation notes-
//       Because length attacks are possible, it's unclear whether the indexing
//       check logic should be used for anything other than ET_INSERT.
//
//----------------------------------------------------------------------------
void
   Pack::encode(                    // Encode Properties
     Writer&           writer,      // Ioda (Writer)
     const Properties& properties)  // Properties to encode
{  if( HCDM ) {
     debugf("Pack(%p).encode(%p)\n", this, &properties);
     if( VERBOSE > 1 )
       properties.debug("Pack::encode");
   }

   for(size_t i= 0; i<properties.size(); ++i) {
     const Property property= properties[i];
     const char* name=  property.name.c_str();
     const char* value= property.value.c_str();
     if( verbose )
       debugf("Encode: '%s': '%s'\n", name, value);

     int         et= property.et;   // Requested encoded type
     Entry*      entry= nullptr;    // Default, no Entry found (or allowed)
     switch( et ) {
       case ET_INDEX:               // Entry, downgrade to INSERT if needed
         entry= entry_map.locate(name, value);
         if( entry == nullptr )  {  // Name/value match not found
           entry= entry_map.locate(name);
           if( entry )
             et= ET_INSERT;
           else
             et= ET_INSERT_NOINDEX;
         }
         break;

       case ET_INSERT_NOINDEX:      // (No indexing)
       case ET_NEVER_NOINDEX:
       case ET_CONST_NOINDEX:
         break;

       case ET_INSERT:
         entry= entry_map.locate(name);
         if( entry == nullptr )
           et= ET_INSERT_NOINDEX;
         break;

       case ET_NEVER:
         entry= entry_map.locate(name);
         if( entry == nullptr )
           et= ET_NEVER_NOINDEX;
         break;

       case ET_CONST:
         entry= entry_map.locate(name);
         if( entry == nullptr )
           et= ET_CONST_NOINDEX;
         break;

       default:
         if( USE_CHECKING ) {
           debugf("%4d %s Invalid property encoding(%d)\n", __LINE__, __FILE__
                 , et);
           property.debug("Invalid encoding");
         }
         throw connection_error("Invalid encoding");

     }

     // Handle indexing
     if( entry ) {
       Entry_ix entry_ix= entry2entix(entry);
       if( et == ET_INDEX ) {       // Entry only
         Integer::encode(writer, entry_ix);
         continue;
       }

       // Encode Entry_ix and value
       Integer::encode(writer, entry_ix, type_to_mask[et], type_to_bits[et]);
       string_encode(writer, property.value, property.v_encoded);
       if( et == ET_INSERT ) {
         insert(property);
         if( verbose )
           debugf("Insert: '%s': '%s' ET_INSERT\n", name, value);
       }
       continue;
     }

     // Write encode_mask, name, and value
     writer.put(get_encode_mask((ENCODE_TYPE)et));
     string_encode(writer, property.name,  property.n_encoded);
     string_encode(writer, property.value, property.v_encoded);
     if( et == ET_INSERT_NOINDEX ) {
       insert(property);
       if( verbose )
         debugf("Insert: '%s': '%s' ET_INSERT_NOINDEX\n", name, value);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::entix2entry
//
// Purpose-
//       Get the Entry* for a logical Entry_ix
//
//----------------------------------------------------------------------------
const Entry*                        // The Entry*
   Pack::entix2entry(               // Locate Entry*
     Entry_ix          entry_ix) const // For this logical Entry_ix
{  if( HCDM )
     debugf("Pack(%p).entix2entry(%u)\n", this, entry_ix);

   // Handle static index
   if( entry_ix < STATIC_ENTRY_DIM )
     return &static_entry[entry_ix];

   // Handle dynamic index
   Value_t dynam_ix= entry_ix - STATIC_ENTRY_DIM;
   if( dynam_ix >= entry_used ) {
     if( USE_CHECKING )
       debugf("%4d cpp entix2entry(%u) dynam_ix(%u)\n", __LINE__
             , entry_ix, dynam_ix);
     debug("range error");
     throw connection_error("entix2entry range error");
   }

   // If entry_array has only one section containing entries
   if( entry_ins > entry_old ) {
     Array_ix array_ix= entry_ins - dynam_ix - 1;
     Index_ix index_ix= entry_size - array_ix;
     Entry* entry= entry_array[index_ix];
     if( entry )
       return entry;

     if( USE_CHECKING )             // If entry not found (internal logic error)
       debugf("%4d cpp entix2entry(%u) dynam_ix(%u) array_ix(%u)\n"
             , __LINE__, entry_ix, dynam_ix, array_ix);
     debug("constency fault");
     throw connection_error("consistency fault");
   }

   // If entry_array has a top and bottom entry section
   if( dynam_ix < (entry_ins - 1) ) { // If entry is in top section
     Array_ix array_ix= entry_ins - dynam_ix - 1;
     Index_ix index_ix= entry_size - array_ix;
     Entry* entry= entry_array[index_ix];
     if( entry )
       return entry;

     if( USE_CHECKING )         // If entry not found (internal logic error)
       debugf("%4d cpp entix2entry(%u) dynam_ix(%u) array_ix(%u)\n"
             , __LINE__, entry_ix, dynam_ix, array_ix);
     debug("constency fault");
     throw connection_error("consistency fault");
   } else {                       // Entry must be in bottom section
     Index_ix index_ix= dynam_ix - entry_ins + 1;
     if( index_ix <= (entry_size - entry_old) ) {
       Entry* entry= entry_array[index_ix];
       if( entry )
         return entry;
     }

     if( USE_CHECKING )             // If entry not found (internal logic error)
       debugf("%4d cpp entix2entry(%u) dynam_ix(%u) index_ix(%u)\n"
             , __LINE__, entry_ix, dynam_ix, index_ix);
     debug("constency fault");
     throw connection_error("consistency fault");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::entry2entix
//
// Purpose-
//       Get the logical Entry_ix for an Entry*
//
// Implementation notes-
//       The oldest dynamic Entry index is STATIC_INDEX_DIM
//
//----------------------------------------------------------------------------
RFC7541::Entry_ix                   // The logical index
   Pack::entry2entix(               // Get logical index
     const Entry*      entry) const // For this Entry*
{  if( HCDM )
     debugf("Pack(%p).entry2entix(%p)\n", this, entry);

   // Handle static index
   Array_ix array_ix= entry->index;
   if( entry->is_static() )
     return array_ix;

   // Handle dynamic index
   if( entry_ins > entry_old ) {
     if( array_ix >= entry_ins || array_ix < entry_old ) {
       debugf("%4d cpp ERROR !{entry_old(%u)<=array_ix(%u)<entry_ins(%u)}\n"
             , __LINE__, entry_old, array_ix, entry_ins);
       debug("constency fault");
       throw connection_error("consistency fault");
     }

     Entry_ix entry_ix= STATIC_ENTRY_DIM + entry_ins - array_ix - 1;
     return entry_ix;
   } else {
     if( array_ix < entry_ins ) {   // If entry is in top section
       Entry_ix entry_ix= STATIC_ENTRY_DIM + entry_ins - array_ix - 1;
       return entry_ix;
     }
     if( array_ix >= entry_old ) {  // If entry is in bottom section
       Index_ix index_ix= entry_size - array_ix;
       Entry_ix entry_ix= STATIC_ENTRY_DIM + entry_ins + index_ix - 1;
       return entry_ix;
     }

     debugf("%4d cpp ERROR !{entry_ins(%u)<=array_ix(%u)>entry_old(%u)}\n"
           , __LINE__, entry_ins, array_ix, entry_old);
     debug("constency fault");
     throw connection_error("consistency fault");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::evict
//
// Purpose-
//       Evict entries from the encode_table
//
//----------------------------------------------------------------------------
void
   Pack::evict(size_t size)         // Evict entries from the encode table
{  if( HCDM )
     debugf("Pack(%p)::evict(%zu)\n", this, size);

   while( entry_used ) {            // While entries remain
     if( encode_size >= (entry_used * SPEC_ENTRY_SIZE) + value_used + size )
       break;

     remove();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::get_setting
//
// Purpose-
//       Get a connection setting
//
//----------------------------------------------------------------------------
#if 0
RFC7541::Value_t                    // The connection setting
   Pack::get_setting(int I) const   // Get connection setting[I]
{  if( HCDM )
     debugf("Pack(%p).get_setting(%d)\n", this, I);

   return 4096;                     // Hey, why not this?
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::insert
//
// Purpose-
//       Insert an Entry into the entry_array and the entry_map
//
//----------------------------------------------------------------------------
void
   Pack::insert(Entry* entry)
{
   if( entry->is_dynamic() ) {      // If dynamic Entry
     size_t nv_size= strlen(entry->name) + strlen(entry->value);
     size_t spec_size= SPEC_ENTRY_SIZE + nv_size;

     // Make room for new Entry (Whether or not it will fit)
//   evict(spec_size);
     while( entry_used ) {          // While entries remain
       if( encode_size >= (get_encode_used() + spec_size) )
         break;
       remove();
     }

     // Don't insert an Entry that won't fit by itself
     if( spec_size > encode_size ) {
       delete entry;
       return;
     }

     // Insert the Entry into the entry_array
     if( entry_ins > entry_size )
       entry_ins= 1;
     Array_ix array_ix= entry_ins++;
     entry->index= array_ix;
     Index_ix index_ix= entry_size - array_ix;
     entry_array[index_ix]= entry;

     // Account for used storage
     ++entry_used;
     value_used += nv_size;
   }

   entry_map.insert(entry);
}

void
   Pack::insert(                    // Allocate and insert Entry into table
     const Property&   property)    // Using this Property
{  insert(new Entry(property)); }

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::remove
//
// Purpose-
//       Remove the oldest Entry
//
//----------------------------------------------------------------------------
void
   Pack::remove( void )
{
   if( entry_used == 0 ) {
     if( USE_CHECKING )
       debugf("Pack(%p)::remove, nothing to remove\n", this);
     throw connection_error("Pack::remove when empty");
   }

   Array_ix array_ix= entry_old;
   Index_ix index_ix= entry_size - array_ix;
   Entry* entry= entry_array[index_ix];
   if( entry == nullptr ) {
     if( USE_CHECKING )
       debugf("%4d cpp consistency check index_ix(%u)\n", __LINE__, index_ix);
     debug("constency fault");
     throw connection_error("consistency fault");
   }
   Value_t size= strlen(entry->name);
   if( entry->value )
     size += strlen(entry->value);
   if( size > value_used ) {
     debugf("%4d RFC7541 size(%u) > value_used(%u)\n", __LINE__
           , size, value_used);
     value_used= 0;
   } else {
     value_used -= size;
   }

   if( verbose )
     debugf("Remove: '%s': '%s'\n", entry->name, entry->value);
   entry_map.remove(entry);
   delete entry;
   entry_array[index_ix]= nullptr;  // (So it won't get used in method debug)
   if( ++entry_old > entry_size )
     entry_old= 1;

   if( --entry_used == 0 ) {
     entry_old= entry_ins= 1;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::resize
//
// Purpose-
//       Resize the encode_table
//
//----------------------------------------------------------------------------
void
   Pack::resize(Value_t size)       // Resize the encode_table
{  if( HCDM )
     debugf("Pack(%p)::resize(%u)\n", this, size);

   if( size > DEFAULT_TABLE_SIZE ) { // TODO: USE HEADER_TABLE_LIMIT instead
     if( USE_CHECKING )
       debugf("Pack(%p)::resize(%u) > DEFAULT_TABLE_SIZE(%u)\n", this, size
             , DEFAULT_TABLE_SIZE);
     throw connection_error("Pack::resize size>DEFAULT_TABLE_SIZE");
   }

   while( (entry_used * SPEC_ENTRY_SIZE + value_used) > size )
     remove();

   encode_size= size;
   entry_size= encode_size / SPEC_ENTRY_SIZE;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::string_decode
//
// Purpose-
//       Retrieve input string, including encoding
//
// Implementation note-
//       Phase-0 implementation, uses std::string intermediaries
//
//----------------------------------------------------------------------------
std::string                         // Resultant string
   Pack::string_decode(             // Retrieve input string
     Reader&           reader)      // (INPUT) IodaReader
{
   bool encoded= reader.peek() & 0x80; // Huffman encoded?
   Value_t size= Integer::decode(reader, 7); // Get the string length

   // Retrieve the input string, encoded or not
   enum {BUFF_DIM= 15};             // The output buffer size
   string              out_string;  // The output string
   char                out_buffer[BUFF_DIM+1]; // The output buffer
   int                 out_index= 0; // The output buffer index
   Value_t             out_total= 0; // The out_string length

   while( out_total < size ) {
     int C= decoder_get(reader);
     out_buffer[out_index++]= C;
     if( out_index >= BUFF_DIM ) {
       string addend(out_buffer, BUFF_DIM);
       out_string += addend;
       out_index= 0;
     }
     ++out_total;
   }

   if( out_index ) {
     string addend(out_buffer, out_index);
     out_string += addend;
   }

   if( encoded )
     out_string= Huff::decode((octet*)out_string.c_str(), size);

   if( HCDM && VERBOSE > 0 )
     debugf("%s= {%d} string_decode(reader)\n", out_string.c_str(), size);
   return out_string;
}

//----------------------------------------------------------------------------
//
// Method-
//       RFC7541::Pack::string_encode
//
// Purpose-
//       Encode output string
//
//----------------------------------------------------------------------------
void
   Pack::string_encode(             // Encode output string
     Writer&           writer,      // (OUTPUT) IodaWriter
     string            text,        // The string
     bool              encoded)     // Huffman encoded?
{
   if( encoded ) {
     Huff::encode(writer, text);
   } else {
     Integer::encode(writer, text.size(), 0x00, 7);
     writer.put(text);
   }
}
}  // Namespace RFC7541
