//----------------------------------------------------------------------------
//
//       Copyright (C) 2021-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Utf.cpp
//
// Purpose-
//       Utf.h implementation methods: classes Utf, Utf8, Utf16, and Utf32.
//
// Last change date-
//       2024/06/07
//
//----------------------------------------------------------------------------
#include <functional>               // For std::function
#include <new>                      // For std::bad_alloc
#include <string>                   // For std::string

#include <cassert>                  // For assert
#include <cstdlib>                  // For free, malloc, ...
#include <cstring>                  // For strcpy, strlen, ...
#include <endian.h>                 // For endian coversion subroutines
#include <arpa/inet.h>              // For htons, ntohs

#include <pub/Debug.h>              // For pub::Debug, namespace pub::debugging
#include "pub/Utf.h"                // Implementation class
#include <pub/utility.h>            // For pub::to_string

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging
using _LIBPUB_NAMESPACE::utility::to_string;

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Compile-time options
{  HCDM= false                      // Hard Core Debug Mode?

,  ROUND_SIZE= 16                   // Allocation rounding size (power of two)
,  ROUND_MASK= ~(ROUND_SIZE-1)      // Allocation rounding mask
};                                  // Compile-time options

typedef Utf::utf8_t    utf8_t;      // Import Utf::utf8_t
typedef Utf::utf16_t   utf16_t;     // Import Utf::utf16_t
typedef Utf::utf32_t   utf32_t;     // Import Utf::utf32_t

enum { UNI_REPLACEMENT= Utf::UNI_REPLACEMENT }; // Import Utf::UNI_REPLACEMENT
enum UTF16_OPTIONS // UTF-16 iterator controls
{  UTF16_LE=   0x00000001           // Option: UTF-16 little endian
};

enum                                // Unicode characters
{  BYTE_ORDER_MARK32= 0x0000'FEFF   // 32-bit Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE32= 0xFFFE'0000   // 32-bit little endian Byte Order Mark
}; // enum Unicode characters

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const Utf::const_iterator
                       Utf::the_end; // The default end iterator

//----------------------------------------------------------------------------
//
// Struct-
//       Utf::Init
//
// Purpose-
//       Initialization quasi-iterator, ends when next() == 0
//
//----------------------------------------------------------------------------
struct Utf::Init {                  // (Internal) initialization iterator
using Function= std::function<utf32_t(Init*)>; // Get next value function

const Function         update;      // Get next value function
const void*            origin;      // Data origin
size_t                 offset= 0;   // Data offset
uint32_t               itopts= 0;   // Iterator controls
uint32_t               filler= 0;   // (Reserved for expansion)
uint32_t next(void) { return update(this); } // Iterator

   Init(                            // Constructor
     const void*       data,        // Data origin
     const Function&   func)        // Get next value function
:  update(func), origin(data) {}
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetch16
//       store16
//
// Purpose-
//       Fetch a utf16_t (Adjust code for fetch)
//       Store a utf16_t (Adjust code for store)
//
//----------------------------------------------------------------------------
static inline utf16_t               // (The adjusted value)
   fetch16(                         // Fetch a utf16_t
     utf16_t              code,     // The code
     pub::utf16_decoder::MODE
                          mode)     // The decoder mode
{  return (mode == pub::Utf::MODE_BE) ? be16toh(code) : le16toh(code); }

static inline utf16_t               // (The adjusted value)
   store16(                         // Store a utf16_t
     utf16_t              code,     // The host
     pub::utf16_encoder::MODE
                          mode)     // The encoder mode
{  return (mode == pub::Utf::MODE_BE) ? htobe16(code) : htole16(code); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       fetch32
//       store32
//
// Purpose-
//       Fetch a utf32_t (Adjust code for fetch)
//       Store a utf32_t (Adjust code for store)
//
//----------------------------------------------------------------------------
static inline utf32_t               // (The adjusted value)
   fetch32(                         // Fetch a utf32_t
     utf32_t              code,     // The fetched code
     pub::utf32_decoder::MODE
                          mode)     // The decoder mode
{  return (mode == pub::Utf::MODE_BE) ? be32toh(code) : le32toh(code); }

static inline utf32_t               // (The adjusted value)
   store32(                         // Store a utf32_t
     utf32_t              code,     // The store code
     pub::utf32_encoder::MODE
                          mode)     // The encoder mode
{  return (mode == pub::Utf::MODE_BE) ? htobe32(code) : htole32(code); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       utf8code
//       utf8size
//
// Purpose-
//       Get code value
//       Get code length
//
//----------------------------------------------------------------------------
static inline utf32_t
   utf8code(                        // Value of next code,  error corrected
     const void*          origin,   // Code origin
     size_t               offset)   // Code offset
{
   const utf8_t* utf= (utf8_t*)origin;
   utf32_t code= utf[offset];
   if( code < 0x80 )
     return code;

   if( code < 0xC0 || code > 0xF7 ) // If invalid start code
     return UNI_REPLACEMENT;        // Invalid encoding

   int chars= 2;                    // Number of encoding characters
   if( code < 0xE0 ) {
     code &= 0x1F;
   } else if( code < 0xF0 ) {
     code &= 0x0F;
     chars= 3;
   } else {
     code &= 0x07;
     chars= 4;
   }

   for(int i= 1; i<chars; i++) {
     code <<= 6;
     int C= utf[offset + i];
     if( C < 0x80 || C > 0xBF )     // If invalid continuation character
       return UNI_REPLACEMENT;

     code |= (C & 0x3F);
   }

   bool overlong= false;
   if( chars == 2 ) {
     overlong= (code < 0x00080);
   } else if( chars == 3 ) {
     overlong= (code < 0x00800);
   } else {
     overlong= (code < 0x010000);
   }
   if( overlong )
     return UNI_REPLACEMENT;

   return code;
}

static inline int
   utf8size(                        // Length of next code, error corrected
     const void*          origin,   // Code origin
     size_t               offset)   // Code offset
{
   const utf8_t* utf= (utf8_t*)origin;
   utf32_t code= utf[offset];
   if( code < 0x80 )
       return (code == 0 ? 0 : 1);    // (Zero length for code 0x00, else 1)

   if( code < 0xC0 || code > 0xF7 ) // If invalid start code
     return 1;                      // (Only one unit skipped)

   int chars= 2;                    // Number of encoding characters
   if( code >= 0xE0 ) {
     if( code < 0xF0 )
       chars= 3;
     else
       chars= 4;
   }

   for(int i= 1; i<chars; i++) {
     int C= utf[offset + i];
     if( C < 0x80 || C > 0xBF )
       return i;
   }

   return chars;
}

//----------------------------------------------------------------------------
//
// Class-
//       Utf8::const_iterator
//
// Purpose-
//       Implementations
//
//----------------------------------------------------------------------------
   Utf8::const_iterator::const_iterator( // Constructor
     const utf8_t*     data)     // (const) string origin
:  Utf::const_iterator()
{
   origin= data;
   assert( origin );             // String origin pointer required

   value= utf8code(origin, 0);
}

Utf8::const_iterator& Utf8::const_iterator::operator++() // (Prefix operator)
{
   if( value ) {                    // Disallow skip past end
     offset += utf8size(origin, offset);
     value   = utf8code(origin, offset);
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::~Utf8
//       Utf8::Utf8
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   Utf8::~Utf8( void )              // Destructor
{  if( HCDM ) debugf("%4d Utf8(%p)::~Utf8\n", __LINE__, this);

   free(data);
}

   Utf8::Utf8( void )               // Default constructor
{  if( HCDM ) debugf("%4d Utf8(%p)::Utf8\n", __LINE__, this); }

   Utf8::Utf8(                      // Copy constructor
     const Utf8&       src)         // Source Utf8
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf8& %s)\n", __LINE__, this, src.data);

   init(src.get_init());
}

   Utf8::Utf8(                      // Move constructor
           Utf8&&      src)         // Source Utf8
:  data(src.data), size(src.size), codes(src.codes), units(src.units)
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf8&& %s)\n", __LINE__, this, src.data);

   src.reset();
}

   Utf8::Utf8(                      // Constructor
     const utf8_t*     src)         // Source utf8_t string
{  if( HCDM ) debugf("%4d Utf8(%p)::Utf8(char* %s)\n", __LINE__, this, src);

   init(Utf8::get_init(src));
}

   Utf8::Utf8(                      // Constructor
     const utf16_t*    src)         // Source utf16_t string
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf16& %p)\n", __LINE__, this, src);

   init(Utf16::get_init(src));
}

   Utf8::Utf8(                      // Constructor
     const utf32_t*    src)         // Source utf32_t string
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf32& %p)\n", __LINE__, this, src);

   init(Utf32::get_init(src));
}

   Utf8::Utf8(                      // Constructor
     const Utf16&      src)         // Source Utf16 object
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf16& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

   Utf8::Utf8(                      // Constructor
     const Utf32&      src)         // Source Utf32 object
{  if( HCDM )
     debugf("%4d Utf8(%p)::Utf8(Utf32& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::get_init
//       Utf8::init
//
// Purpose-
//       Initialization
//
//----------------------------------------------------------------------------
static utf32_t                      // Resultant
   utf8iter(                        // Get next code point
     Utf::Init*        self)        // For this Utf::Init
{
   uint32_t value= utf8code(self->origin, self->offset);
   if( value )
     self->offset += utf8size(self->origin, self->offset);
   return value;
}

Utf::Init
   Utf8::get_init(                  // Get initializer
     const utf8_t*     data)        // From data
{
   if( data == nullptr )
     data= (const utf8_t*)"";

   Init init(data, utf8iter);
   return init;
}

Utf::Init
   Utf8::get_init( void ) const     // Get initializer
{  return get_init(data); }

void
   Utf8::init(                      // Initialize
     const Init&       init)        // Using this initializer
{
   // Handle empty initializer
   this->codes= 0;
   this->units= 0;
   if( init.origin == nullptr )
     return;

   // Count codes and units
   Init copy= init;
   size_t codes= 0;
   size_t units= 0;
   utf32_t value= copy.next();
   while( value ) {
     codes++;
     units += length(value);
     value= copy.next();
   }
   if( codes == 0 )                 // If empty source
     return;

   // If needed, update the data buffer
   size_t size= units + ROUND_SIZE;
   size &= ROUND_MASK;
   if( size > this->size ) {
     free(data);
     data= (utf8_t*)malloc(size);
     if( data == nullptr ) {
       this->size= 0;
       throw std::bad_alloc();
     }
     this->size= size;
   }

   // Encode the source
   copy.offset= 0;                  // Start over
   value= copy.next();
   while( value ) {
     this->codes++;
     // assert( (this->units + length(value)) < size ); // See TLDR comment
     this->units += encode(value, data + this->units);
     value= copy.next();
   }
   data[units]= '\0';

   // TLDR: Checking overflow for each operation doesn't add protection.
   // Anything that could cause the data to change between scans could more
   // easily corrupt the data buffer without needing this code as its proxy.
   // This after the fact check is too late to avoid a buffer overrun, but
   // it (or a SEGFAULT) is enough to detect that it happened.
   assert( this->codes == codes && this->units == units );
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::operators
//
// Purpose-
//       Operator method implementations
//
//----------------------------------------------------------------------------
Utf8& Utf8::operator= (const Utf8& src) // Copy Utf8 object
{
   if( this != &src )               // (Already equal to self)
     init(src.get_init());          // (And this would free data before copy)

   return *this;
}

Utf8& Utf8::operator= (      Utf8&& src) // Move Utf8 object
{
   if( this != &src ) {             // (Already equal to self)
     free(data);                    // (And this would free data before move)

     data= src.data;
     size= src.size;
     codes= src.codes;
     units= src.units;

     src.reset();
   }

   return *this;
}

Utf8& Utf8::operator= (const Utf16& src) // Copy Utf16 object
{
   init(src.get_init());
   return *this;
}

Utf8& Utf8::operator= (const Utf32& src) // Copy Utf32 object
{
   init(src.get_init());
   return *this;
}

Utf8& Utf8::operator= (const utf8_t* src) // Copy utf8_t string
{
   init(Utf8::get_init(src));
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::get_code
//
// Purpose-
//       Get the length of a string in code points
//
//----------------------------------------------------------------------------
size_t                              // The length of the (utf8) string
   Utf8::get_codes(                 // Get length (in code points)
     const std::string src)         // Of this string
{
   const char* origin= src.c_str();
   size_t offset= 0;
   size_t length= src.length();

   size_t col_count= 0;
   while( offset < length ) {
     ++col_count;
     unsigned L= utf8size(origin, offset);
     offset += L;
   }

   return col_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::decode
//
// Purpose-
//       Decode the next code point
//
// Implementation notes-
//       REMEMBER, there might not be a terminating character
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The next UTF32 code point
   Utf8::decode(                    // Decode next code point
     const utf8_t*     utf)         // Encoding buffer pointer
{  return utf8code(utf, 0); }

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::encode
//
// Purpose-
//       Encode a code point
//
//----------------------------------------------------------------------------
unsigned                            // The UTF8 encoding length
   Utf8::encode(                    // Get UTF8 encoding
     utf32_t           code,        // For this code point
     utf8_t*           buff)        // (OUT) Encoding buffer
{
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   if( code < 0x0000'0080 ) {       // Single byte encoding
     buff[0]= (utf8_t)code;
     return 1;
   }

   if( code < 0x0000'0800 ) {       // Two byte encoding
     buff[1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buff[0]= (utf8_t)(code | 0xC0);
     return 2;
   }

   if( code < 0x0001'0000 ) {       // Three byte encoding
     buff[2]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buff[1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buff[0]= (utf8_t)(code | 0xE0);
     return 3;
   }

   // Four byte encoding
   buff[3]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buff[2]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buff[1]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buff[0]= (utf8_t)(code | 0xF0);
   return 4;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::index
//
// Purpose-
//       Get byte offset for code point index
//
//----------------------------------------------------------------------------
size_t                              // The utf8_t* offset
   Utf8::index(                     // Get utf8_t* offset for
     const utf8_t*     addr,        // This ('\0' terminated) utf8_t* string
     size_t            X)           // And this code point index
{
   size_t O= 0;                     // Current offset
   while( X > 0 ) {
     O += utf8size(addr, O);        // The current start character
     --X;
   }

   return O;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::length
//
// Purpose-
//       Get encoding length
//
//----------------------------------------------------------------------------
unsigned                            // The UTF8 encoding length
   Utf8::length(                    // Get UTF8 encoding length
     const utf8_t*     buff)        // For this buffer
{  return utf8size(buff,0); }

//----------------------------------------------------------------------------
//
// Method-
//       Utf8::reset
//
// Purpose-
//       Reset (empty) the Utf8
//
//----------------------------------------------------------------------------
void
   Utf8::reset( void )              // Reset (empty) this Utf8
{
   free(data);
   data= nullptr;
   size= units= codes= 0;
}

//----------------------------------------------------------------------------
//
// Utf16 utilities, not directly exposed.
//
// Subroutine-
//       utf16code
//       utf16size
//
// Purpose-
//       Get code value
//       Get code length
//
//----------------------------------------------------------------------------
static inline utf16_t               // Corrected value
   utf16load(                       // Load
     const utf16_t*       data,     // This data
     uint32_t             opts)     // Using these options
{
   if( opts & UTF16_LE ) {          // If little endian encoding
     const utf8_t* utf8= (utf8_t*)data; // Invert byte order
     return (utf8[1] << 8) | utf8[0];
   }

   return ntohs(*data);
}

static inline utf32_t               // Next encoding
   utf16code(                       // Value of next code,  error corrected
     const void*          origin,   // Code origin
     size_t               offset,   // Code offset
     uint32_t             itopts)   // Iterator options
{
   const utf16_t* utf= (utf16_t*)origin;
   utf32_t code= utf16load(utf + offset, itopts);
   if( code >= 0x00D800 && code <= 0x00DFFF ) { // If multi-unit encoding
     if( code >= 0x00DC00 )         // If sequence error
       return UNI_REPLACEMENT;
     uint16_t word= utf16load(utf + offset + 1, itopts);
     if( word < 0x00DC00 || word > 0x00DFFF ) // If invalid pair
       return UNI_REPLACEMENT;

     code &= 0x03FF;
     code <<= 10;
     code |= (word & 0x03FF);
     code += 0x010000;
     if( ! Utf::is_unicode(code) )
       return UNI_REPLACEMENT;
   }

   return code;
}

static inline int                   // Encoding length, in units
   utf16size(                       // Length of next code, error corrected
     const void*          origin,   // Code origin
     size_t               offset,   // Code offset
     uint32_t             itopts)   // Iteration options
{
   const utf16_t* utf= (utf16_t*)origin;
   utf32_t code= utf16load(utf + offset, itopts);
   if( code >= 0x00D800 && code <= 0x00DFFF ) { // If multi-unit encoding
     if( code >= 0x00DC00 )         // If sequence error
       return 1;
     uint16_t word= utf16load(utf + offset + 1, itopts);
     if( word < 0x00DC00 || word > 0x00DFFF ) // If invalid pair
       return 1;

     return 2;
   }

   return 1;
}

//----------------------------------------------------------------------------
//
// Class-
//       Utf16::const_iterator
//
// Purpose-
//       Implementations
//
//----------------------------------------------------------------------------
   Utf16::const_iterator::const_iterator( // Constructor
     const utf16_t*    data)     // (const) string origin
{
   origin= data;
   assert( origin );             // String origin pointer required

   if( ntohs(*data) == MARK_ORDER_BYTE ) // If little endian encoding
     itopts |= UTF16_LE;

   value= utf16code(origin, 0, itopts);
}

Utf16::const_iterator& Utf16::const_iterator::operator++() // (Prefix operator)
{
   if( value ) {                    // Disallow skip past end
     offset += utf16size(origin, offset, itopts);
     value   = utf16code(origin, offset, itopts);
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf16::~Utf16
//       Utf16::Utf16
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   Utf16::~Utf16( void )            // Destructor
{  if( HCDM ) debugf("%4d Utf16(%p)::~Utf16\n", __LINE__, this);

   free(data);
}

   Utf16::Utf16( void )             // Default constructor
{  if( HCDM ) debugf("%4d Utf16(%p)::Utf16\n", __LINE__, this); }

   Utf16::Utf16(                    // Copy constructor
     const Utf16&      src)         // Source Utf16
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(Utf16& %p)\n", __LINE__, this, src.data);

   init(src.get_init());
}

   Utf16::Utf16(                    // Move constructor
           Utf16&&     src)         // Source Utf16
:  data(src.data), size(src.size), codes(src.codes), units(src.units)
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(Utf16&& %p)\n", __LINE__, this, src.data);

   src.reset();
}

   Utf16::Utf16(                    // Constructor
     const utf8_t*     src)         // Source utf8_t string
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(utf8_t* %s)\n", __LINE__, this, src);

   init(Utf8::get_init(src));
}

   Utf16::Utf16(                    // Constructor
     const utf16_t*    src)         // Source utf16_t string
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(utf16_t* %p)\n", __LINE__, this, src);

   init(Utf16::get_init(src));
}

   Utf16::Utf16(                    // Constructor
     const utf32_t*    src)         // Source utf32_t string
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(utf32_t* %p)\n", __LINE__, this, src);

   init(Utf32::get_init(src));
}

   Utf16::Utf16(                    // Constructor
     const Utf8&       src)         // Source Utf8 object
:  Utf16(src.get_data())
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(Utf8& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

   Utf16::Utf16(                    // Constructor
     const Utf32&      src)         // Source Utf32 object
:  Utf16(src.get_data())
{  if( HCDM )
     debugf("%4d Utf16(%p)::Utf16(Utf32& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf16::get_init
//       Utf16::init
//
// Purpose-
//       Initialization
//
//----------------------------------------------------------------------------
static utf32_t                      // Resultant
   utf16iter(                       // Get next code point
     Utf::Init*        self)        // For this Utf::Init
{
   uint32_t value= utf16code(self->origin, self->offset, self->itopts);
   if( value )
     self->offset += utf16size(self->origin, self->offset, self->itopts);
   return value;
}

Utf::Init
   Utf16::get_init(                 // Get initializer
     const utf16_t*    data)        // From data
{
   Init init(data, utf16iter);
   if( ntohs(*data) == MARK_ORDER_BYTE ) // If little endian encoding
     init.itopts |= UTF16_LE;
   return init;
}

Utf::Init
   Utf16::get_init( void ) const    // Get initializer
{  return get_init(data); }

void
   Utf16::init(                     // Initialize
     const Init&       init)        // Using this initializer
{
   // Handle empty initializer
   this->codes= 0;
   this->units= 0;
   if( init.origin == nullptr )
     return;

   // Count codes and units
   Init copy= init;
   size_t codes= 0;
   size_t units= 0;
   utf32_t value= copy.next();
   while( value ) {
     codes++;
     units += length(value);
     value= copy.next();
   }
   if( codes == 0 )                 // If empty source
     return;

   // If needed, update the data buffer
   size_t size= units + ROUND_SIZE;
   size &= ROUND_MASK;
   if( size > this->size ) {
     free(data);
     data= (utf16_t*)malloc(size * sizeof(utf16_t));
     if( data == nullptr ) {
       this->size= 0;
       throw std::bad_alloc();
     }
     this->size= size;
   }

   // Encode the source
   copy.offset= 0;                  // Start over
   value= copy.next();
   while( value ) {
     this->codes++;
     // assert( (this->units + length(value)) < size ); // See Utf8 TLDR comment
     this->units += encode(value, data + this->units);
     value= copy.next();
   }
   data[units]= 0;

   assert( this->codes == codes && this->units == units );
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf16::operators
//
// Purpose-
//       Operator method implementations
//
//----------------------------------------------------------------------------
Utf16& Utf16::operator= (const Utf16& src) // Copy Utf16 object
{
   if( this != &src )               // (Already equal to self)
     init(src.get_init());          // (And this would free data before copy)

   return *this;
}

Utf16& Utf16::operator= (      Utf16&& src) // Move Utf16 object
{
   if( this != &src ) {             // (Already equal to self)
     free(data);                    // (And this would free data before move)

     data= src.data;
     size= src.size;
     codes= src.codes;
     units= src.units;

     src.reset();
   }

   return *this;
}

Utf16& Utf16::operator= (const utf8_t* src) // Copy utf8_t string
{
   init(Utf8::get_init(src));
   return *this;
}

Utf16& Utf16::operator= (const Utf8& src) // Copy Utf8 object
{
   init(src.get_init());
   return *this;
}

Utf16& Utf16::operator= (const Utf32& src) // Copy Utf32 object
{
   init(src.get_init());
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf16::decode
//
// Purpose-
//       Decode the next code point
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The next UTF32 code point
   Utf16::decode(                   // Decode next code point
     const utf16_t*    buff)        // Encoding buffer pointer
{
   throw std::runtime_error("NOT CODED YET");
   (void)buff; return 0;
}

//----------------------------------------------------------------------------
// Encode a UTF32 code point, setting the big endian encoding buffer.
unsigned                            // The UTF16 encoding length
   Utf16::encode(                   // Get UTF16 encoding
     utf32_t           code,        // For this code point
     utf16_t*          buff)        // (OUT) Encoding buffer
{
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   if( code < 0x010000 ) {
     *buff= htons((utf16_t)code);
     return 1;
   }

   code -= 0x010000;
   buff[1]= htons((code & 0x003ff) | 0xDC00);
   code >>= 10;
   buff[0]= htons((code & 0x003ff) | 0xD800);

   return 2;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf16::reset
//
// Purpose-
//       Reset (empty) the Utf16
//
//----------------------------------------------------------------------------
void
   Utf16::reset( void )             // Reset (empty) this Utf16
{
   free(data);
   data= nullptr;
   size= units= codes= 0;
}

//----------------------------------------------------------------------------
//
// Class-
//       Utf32::const_iterator
//
// Purpose-
//       Implementations
//
//----------------------------------------------------------------------------
   Utf32::const_iterator::const_iterator( // Constructor
     const utf32_t*    data)     // (const) string origin
:  Utf::const_iterator()
{
   origin= data;
   assert( origin );             // String origin pointer required

   value= *data;
}

Utf32::const_iterator& Utf32::const_iterator::operator++() // (Prefix operator)
{
   if( value ) {                    // Disallow skip past end
     offset++;
     value= *((utf32_t*)origin + offset);
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf32::~Utf32
//       Utf32::Utf32
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   Utf32::~Utf32( void )            // Destructor
{  if( HCDM ) debugf("%4d Utf32(%p)::~Utf32\n", __LINE__, this);

   free(data);
}

   Utf32::Utf32( void )             // Default constructor
{  if( HCDM ) debugf("%4d Utf32(%p)::Utf32\n", __LINE__, this); }

   Utf32::Utf32(                    // Copy constructor
     const Utf32&      src)         // Source Utf32
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(Utf32& %p)\n", __LINE__, this, src.data);

   init(src.get_init());
}

   Utf32::Utf32(                    // Move constructor
           Utf32&&     src)         // Source Utf32
:  data(src.data), size(src.size), codes(src.codes)
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(Utf32&& %p)\n", __LINE__, this, src.data);

   src.reset();
}

   Utf32::Utf32(                    // Constructor
     const utf8_t*     src)         // Source utf8_t string
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(utf8_t* %s)\n", __LINE__, this, src);

   init(Utf8::get_init(src));
}

   Utf32::Utf32(                    // Constructor
     const utf16_t*    src)         // Source utf16_t string
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(utf16_t* %p)\n", __LINE__, this, src);

   init(Utf16::get_init(src));
}

   Utf32::Utf32(                    // Constructor
     const utf32_t*    src)         // Source utf32_t string
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(utf32_t* %p)\n", __LINE__, this, src);

   init(Utf32::get_init(src));
}

   Utf32::Utf32(                    // Constructor
     const Utf8&       src)         // Source Utf8 object
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(Utf8& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

   Utf32::Utf32(                    // Constructor
     const Utf16&      src)         // Source Utf16 object
{  if( HCDM )
     debugf("%4d Utf32(%p)::Utf32(Utf16& %p)\n", __LINE__, this, &src);

   init(src.get_init());
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf32::get_init
//       Utf32::init
//
// Purpose-
//       Initialization
//
//----------------------------------------------------------------------------
static utf32_t                      // Resultant
   utf32init(                       // Initialization function
     Utf::Init*        self)        // For this Utf::Init
{
   uint32_t value= *((utf32_t*)self->origin + self->offset);
   if( value )
     self->offset++;
   return value;
}

Utf::Init
   Utf32::get_init(                 // Get initializer
     const utf32_t*    data)        // From data
{
   Init init(data, utf32init);
   return init;
}

Utf::Init
   Utf32::get_init( void ) const    // Get initializer
{  return get_init(data); }

void
   Utf32::init(                     // Initialize
     const Init&       init)        // Using this initializer
{
   // Handle empty initializer
   this->codes= 0;
   if( init.origin == nullptr )
     return;

   // Count codes (For uint32_t, units == codes)
   Init copy= init;
   size_t codes= 0;
   utf32_t value= copy.next();
   while( value ) {
     codes++;
     value= copy.next();
   }
   if( codes == 0 )                 // If empty source
     return;

   // If needed, update the data buffer
   size_t size= codes + ROUND_SIZE;
   size &= ROUND_MASK;
   if( size > this->size ) {
     free(data);
     data= (utf32_t*)malloc(size * sizeof(utf32_t));
     if( data == nullptr ) {
       this->size= 0;
       throw std::bad_alloc();
     }
     this->size= size;
   }

   // Encode the source
   copy.offset= 0;                  // Start over
   value= copy.next();
   while( value ) {
     // assert( (this->codes + length(value)) < size ); // See Utf8 TLDR comment
     this->codes += encode(value, data + this->codes);
     value= copy.next();
   }
   data[codes]= 0;

   assert( this->codes == codes );
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf32::operators
//
// Purpose-
//       Operator method implementations
//
//----------------------------------------------------------------------------
Utf32& Utf32::operator= (const Utf32& src) // Copy Utf32 object
{
   if( this != &src )               // (Already equal to self)
     init(src.get_init());          // (And this would free data before copy)

   return *this;
}

Utf32& Utf32::operator= (      Utf32&& src) // Move Utf32 object
{
   if( this != &src ) {             // (Already equal to self)
     free(data);                    // (And this would free data before move)

     data= src.data;
     size= src.size;
     codes= src.codes;

     src.reset();
   }

   return *this;
}

Utf32& Utf32::operator= (const utf8_t* src) // Copy utf8_t string
{
   init(Utf8::get_init(src));
   return *this;
}

Utf32& Utf32::operator= (const Utf8& src) // Copy Utf8 object
{
   init(src.get_init());
   return *this;
}

Utf32& Utf32::operator= (const Utf16& src) // Copy Utf16 object
{
   init(src.get_init());
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf32::decode
//
// Purpose-
//       Decode the next code point
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The next UTF32 code point
   Utf32::decode(                   // Decode next code point
     const utf32_t*    buff)        // Encoding buffer pointer
{
   uint32_t code= *buff;
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   return code;
}

//----------------------------------------------------------------------------
// Encode a UTF32 code point, setting the encoding buffer.
unsigned                            // The UTF32 encoding length
   Utf32::encode(                   // Get UTF32 encoding
     utf32_t           code,        // For this code point
     utf32_t*          buff)        // (OUT) Encoding buffer
{
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   *buff= code;
   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Utf32::reset
//
// Purpose-
//       Reset (empty) the Utf32
//
//----------------------------------------------------------------------------
void
   Utf32::reset( void )             // Reset (empty) this Utf32
{
   free(data);
   data= nullptr;
   size= codes= 0;
}

//============================================================================
//
// Method-
//       utf8_decoder::utf8_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf8_decoder::utf8_decoder(      // Copy constructor
     const utf8_decoder& from)      // Source utf8_decoder
:  buffer(from.buffer), length(from.length)
{  }

   utf8_decoder::utf8_decoder(      // Constructor
     const utf8_t*     addr,        // Decode buffer address
     Length            size)        // Decode buffer length
:  buffer(addr), length(size)
{  }

   utf8_decoder::utf8_decoder(      // Constructor
     const utf8_t*     addr)        // Decode buffer address
:  buffer(addr), length(::strlen((char*)addr) + 1)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::copy_column
//
// Purpose-
//       Copy the current column, including continuation characters
//
// Implementation note-
//       The resultant's column number is zero.
//
//----------------------------------------------------------------------------
utf8_decoder                        // The current column substring
   utf8_decoder::copy_column( void ) const // Copy the current column
{
   utf8_decoder copy;

   copy.buffer= buffer + offset;
   copy.length= length - offset;

   copy.decode();                   // (Include the current column codepoint)
   while( copy.is_combining() )     // Include combining codepoints
     copy.decode();

   copy.length= copy.offset;
   copy.column= copy.offset= 0;

   return copy;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::get_points
//
// Purpose-
//       Get the total codepoint count
//
//----------------------------------------------------------------------------
Utf::Points                         // The total codepoint count
   utf8_decoder::get_points( void ) // Get total codepoint count
{
   utf8_decoder copy(*this);

   // Decode the copy, counting codepoints as we go
   Points points= 0;
   for(uint32_t point= copy.decode(); point; point= copy.decode()) {
     if( !is_combining(point) )
       ++points;
   }

   return points;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::current
//
// Purpose-
//       Decode the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current encoding
   utf8_decoder::current( void )  const // Get current encoding
{
   if( offset >= length )
     return 0;

   utf32_t code= buffer[offset];
   if( code < 0x80 )                // If ASCII encoding
     return code;                   // (Done)

   if( code < 0xC0 || code > 0xF7 ) // If invalid start code
     return UNI_REPLACEMENT;        // Use UNI_REPLACEMENT

   // Multiple character encodings
   unsigned size= 2;                // Number of encoding characters
   if( code < 0xE0 ) {              // (0XC0 .. 0xDF)
//// size= 2;                       // Two character encoding
     code &= 0x1F;
   } else if( code < 0xF0 ) {       // (0XE0 .. 0xEF)
     size= 3;                       // Three character encoding
     code &= 0x0F;
   } else {                         // (0XF0 .. 0xF7)
     size= 4;                       // Four character encoding
     code &= 0x07;
   }

   // Decode continuation characters, rejecting invalid encodings
   if( size > (length - offset ) )
     return UNI_REPLACEMENT;

   for(unsigned i= 1; i<size; ++i) {
     int C= buffer[offset + i];
     if( C < 0x80 || C > 0xBF )
       return UNI_REPLACEMENT;

     code <<= 6;
     code  |= (C & 0x3F);
   }

   // Check for overlong encoding (code < 0x80 already handled)
   if( size == 2 ) {
     if( code < 0x0000'0080 )
       return UNI_REPLACEMENT;
   } else if( size == 3 ) {
     if( code < 0x0000'0800 )
       return UNI_REPLACEMENT;
   } else /* (size == 4) */ {
     if( code < 0x0001'0000 )
       return UNI_REPLACEMENT;
   }

   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::decode
//
// Purpose-
//       Decode the next codepoint, updating column and offset
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The current codepoint
   utf8_decoder::decode( void )     // Decode next codepoint
{
   if( offset >= length )
     return 0;

   utf32_t code= buffer[offset];
   if( code < 0x80 ) {              // If ASCII encoding
     ++column;
     ++offset;
     return code;
   }

   if( code < 0xC0 || code > 0xF7 ) { // If invalid start code
     ++column;
     ++offset;
     return UNI_REPLACEMENT;        // Use UNI_REPLACEMENT
   }

   // Multiple character encodings
   unsigned size= 2;                // Number of encoding characters
   if( code < 0xE0 ) {              // (0XC0 .. 0xDF)
//// size= 2;                       // Two character encoding
     code &= 0x1F;
   } else if( code < 0xF0 ) {       // (0XE0 .. 0xEF)
     size= 3;                       // Three character encoding
     code &= 0x0F;
   } else {                         // (0XF0 .. 0xF7)
     size= 4;                       // Four character encoding
     code &= 0x07;
   }

   // Decode continuation characters, rejecting invalid encodings
   if( size > (length - offset ) ) {
     offset= length;
     return UNI_REPLACEMENT;
   }

   ++offset;                        // (Account for the lead character)
   for(unsigned i= 1; i<size; ++i) {
     int C= buffer[offset++];
     if( C < 0x80 || C > 0xBF ) {
       ++column;
       return UNI_REPLACEMENT;
     }

     code <<= 6;
     code  |= (C & 0x3F);
   }

   // Check for overlong encoding (size == 1 cannot be overlong)
   if( size == 2 ) {
     if( code < 0x0000'0080 )
       code= UNI_REPLACEMENT;
   } else if( size == 3 ) {
     if( code < 0x0000'0800 )
       code= UNI_REPLACEMENT;
   } else /* (size == 4) */ {
     if( code < 0x0001'0000 )
       code= UNI_REPLACEMENT;
   }

   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;
   if( offset == Offset(size) || !is_combining(code) )
     ++column;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::index
//
// Purpose-
//       Set column to the specfied column index
//
//----------------------------------------------------------------------------
Utf::Length                         // Number of characters past end of buffer
   utf8_decoder::index(             // Set the column
     Column            col)         // To this column index
{
   if( col >= column ) {
     col -= column;
     for(uint32_t point= decode(); point; point= decode()) {
       if( col == 0 )
         return 0;

       if( !is_combining(point) )
         --col;
     }
   } else {
     // Recode this if backspace is implemented
     column= 0;
     for(uint32_t point= decode(); point; point= decode()) {
       if( col == 0 )
         return 0;

       if( !is_combining(point) )
         --col;
     }
   }

   return col;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf8_decoder::reset(             // Reset the decoder
     const utf8_t*     addr,        // Encoding buffer pointer
     Length            size)        // Encoding buffer pointer (byte) Length
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= 0;
   offset= 0;
}

//============================================================================
//
// Method-
//       utf8_encoder::utf8_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf8_encoder::utf8_encoder(      // Address/length encoder
     utf8_t*           addr,        // Address
     Length            size)        // Length
:  buffer(addr), length(size)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::utf8_encode
//
// Purpose-
//       Encode one codepoint
//
// Implementation notes-
//       Currently no combining codepoints have 4 character encodings, but
//       who knows what tomorrow brings?
//       We check whether 4 character encodings are combining anyway.
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length
   utf8_encoder::encode(            // Encode
     utf32_t           code)        // This codepoint
{
   if( offset >= length )           // If buffer full
     return 0;

   if( code < 0x0000'0080 ) {       // Single byte encoding
     buffer[offset++]= (utf8_t)code;
     ++column;
     return 1;
   }

   Length left= length - offset;    // The available buffer length
   if( code < 0x0000'0800 ) {       // Two byte encoding
     if( left < 2 )
       return 0;

     buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 0]= (utf8_t)(code | 0xC0);
     if( offset == 0 || !is_combining(code) )
       ++column;
     offset += 2;
     return 2;
   }

   if( !is_unicode(code) )          // If invalid codepoint
     code= UNI_REPLACEMENT;         // Use replacement codepoint
   if( code < 0x0001'0000 ) {       // Three byte encoding
     if( left < 3 )
       return 0;

     buffer[offset + 2]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buffer[offset + 0]= (utf8_t)(code | 0xE0);
     if( offset == 0 || !is_combining(code) )
       ++column;
     offset += 3;
     return 3;
   }

   // Four byte encoding
   if( left < 4 )
     return 0;

   buffer[offset + 3]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 2]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 1]= (utf8_t)((code & 0x3F) | 0x80);
   code >>= 6;
   buffer[offset + 0]= (utf8_t)(code | 0xF0);
   if( offset == 0 || !is_combining(code) )
     ++column;
   offset += 4;
   return 4;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf8_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf8_encoder::reset(             // Reset the encoder
     utf8_t*           addr,        // Encoding buffer pointer
     Length            size)        // Encoding buffer pointer (byte) Length
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= 0;
   offset= 0;
}

//============================================================================
//
// Method-
//       utf16_decoder::utf16_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf16_decoder::utf16_decoder(    // Copy constructor
     const utf16_decoder& from)     // Source utf16_decoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{
   reset(buffer, length);
   if( offset == 0 )
     mode= from.mode;
}

   utf16_decoder::utf16_decoder(    // Buffer constructor
     const utf16_t*    addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
:  buffer(addr), length(size)
{
   reset(buffer, length);
}

   utf16_decoder::utf16_decoder(    // Buffer constructor
     const utf16_t*    addr)        // Buffer address
:  buffer(addr)
{
   if( buffer )
     length= strlen(addr) + 1;

   reset(buffer, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf16_decoder::set_mode(         // Set decoding mode
     MODE              M)           // The decoding mode
{
   if( offset || M > MODE_LE )      // If decoding started or invalid MODE
     throw utf_error("set_mode usage error");

   this->mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::current
//
// Purpose-
//       Get the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf16_decoder::current( void ) const // Get current codepoint
{
   throw std::runtime_error("NOT CODED YET");
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::decode
//
// Purpose-
//       Decode the current codepoint, updating column and offset
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf16_decoder::decode( void )    // Decode the current codepoint
{
   if( offset >= length )
     return 0;

   utf32_t code= fetch16(buffer[offset], mode);
   if( code < 0x00'D800 || code >= 0x00'E000 ) { // If standard encoding
     ++offset;
     if( !is_combining(code) )
       ++column;
     return code;
   }

   // Surrogate pair encoding
   if( code >= 0x00'DC00 ) {        // Second half of encoding first: ERROR
     ++offset;
     ++column;
     return UNI_REPLACEMENT;
   }

   if( 2 > (length - offset ) ) {   // If second half missing (not in buffer)
     ++offset;
     ++column;
     return UNI_REPLACEMENT;
   }

   utf32_t half= fetch16(buffer[offset+1], mode); // Get second half of pair
   if( half < 0x00'DC00 || half >= 0x00'E000 ) { // If second half invalid
     ++offset;
     ++column;
     return UNI_REPLACEMENT;
   }

   // Resultant is always in unicode range but never in surrogate pair range.
   code= 0x01'0000 + ((code & 0x00'03FF) << 10 | (half & 0x00'03FF));
   if( offset == 0 || !is_combining(code) )
     ++column;
   offset += 2;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::index
//
// Purpose-
//       Index to the specified Column
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of (bytes) past end
   utf16_decoder::index(            // Set column index to
     Column            IX)          // This column index
{
   throw std::runtime_error("NOT CODED YET");
   (void)IX; return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf16_decoder::reset(            // Reset
     const utf16_t*    addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= offset= 0;
   mode= MODE_BE;

   // Check for BYTE_ORDER_MARK or MARK_ORDER_BYTE
   if( length > 0 ) {
     uint32_t code= be16toh(buffer[0]);
     if( code == BYTE_ORDER_MARK )
       offset= 1;
     else if( code == MARK_ORDER_BYTE ) {
       offset= 1;
       mode= MODE_LE;
     }
   }
}

//============================================================================
//
// Method-
//       utf16_encoder::utf16_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf16_encoder::utf16_encoder(    // Buffer constructor
     utf16_t*          addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{  reset(addr, size); }

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf16_encoder::set_mode(         // Set encoding mode
     MODE              M)           // The encoding mode
{
   if( offset || M > MODE_LE )      // If encoding started or invalid MODE
     throw utf_error("set_mode usage error");

   this->mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::encode
//
// Purpose-
//       Encode a codepoint
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   utf16_encoder::encode(           // Encode
     utf32_t           code)        // This codepoint
{
   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   if( code < 0x01'0000 ) {
     if( (length - offset) < 1 )
       return 0;

     buffer[offset++]= store16((utf16_t)code, mode);
     if( !is_combining(code) )
       ++column;

     return 1;
   }

   if( (length - offset) < 2 )
     return 0;

   code -= 0x01'0000;
   buffer[offset + 1]= store16((code & 0x00'03ff) | 0x00'DC00, mode);
   code >>= 10;
   buffer[offset + 0]= store16((code & 0x00'03ff) | 0x00'D800, mode);
   offset += 2;
   if( !is_combining(code) )
     ++column;

   return 2;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf16_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf16_encoder::reset(            // Reset
     utf16_t*          addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= offset= 0;
   mode= MODE_BE;
}

//============================================================================
//
// Method-
//       utf32_decoder::utf32_decoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf32_decoder::utf32_decoder(    // Copy constructor
     const utf32_decoder& from)     // Source utf32_decoder
:  buffer(from.buffer), length(from.length), mode(from.mode)
{
   reset(buffer, length);
   if( offset == 0 )
     mode= from.mode;
}

   utf32_decoder::utf32_decoder(    // Buffer constructor
     const utf32_t*    addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
:  buffer(addr), length(size)
{
   reset(buffer, length);
}

   utf32_decoder::utf32_decoder(    // Buffer constructor
     const utf32_t*    addr)        // Buffer address
:  buffer(addr)
{
   if( buffer )
     length= strlen(addr) + 1;

   reset(buffer, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf32_decoder::set_mode(         // Set decoding mode
     MODE              M)           // The decoding mode
{
   if( offset || M > MODE_LE )      // If decoding started or invalid MODE
     throw utf_error("set_mode usage error");

   this->mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::current
//
// Purpose-
//       Get the current codepoint
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf32_decoder::current( void ) const // Get current codepoint
{
   throw std::runtime_error("NOT CODED YET");
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::decode
//
// Purpose-
//       Decode the current codepoint, updating column and offset
//
//----------------------------------------------------------------------------
utf32_t                             // The current codepoint
   utf32_decoder::decode( void )    // Decode the current codepoint
{
   if( offset >= length )
     return 0;

   utf32_t code= fetch32(buffer[offset], mode);
   if( !is_unicode(code) )
     code= UNI_REPLACEMENT;

   if( is_combining(code) ) {
     if( offset <= 1 ) {
       if( offset == 0 )
         ++column;
       else if( fetch32(buffer[0], mode) == BYTE_ORDER_MARK )
         ++column;
       }
   } else {
     ++column;
   }
   ++offset;

   return code;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::index
//
// Purpose-
//       Index to the specified Column
//
//----------------------------------------------------------------------------
Utf::Length                         // The number of (bytes) past end
   utf32_decoder::index(            // Set column index to
     Column            IX)          // This column index
{
   throw std::runtime_error("NOT CODED YET");
   (void)IX; return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_decoder::reset
//
// Purpose-
//       Reset the decoder
//
//----------------------------------------------------------------------------
void
   utf32_decoder::reset(            // Reset
     const utf32_t*    addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= offset= 0;
   mode= MODE_BE;

   // Check for BYTE_ORDER_MARK or MARK_ORDER_BYTE
   if( length > 0 ) {
     uint32_t code= be32toh(buffer[0]);
     if( code == BYTE_ORDER_MARK32 ) {
       ++offset;
       mode= MODE_BE;
     } else if( code == MARK_ORDER_BYTE32 ) {
       ++offset;
       mode= MODE_LE;
     }
   }
}

//============================================================================
//
// Method-
//       utf32_encoder::utf32_encoder
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   utf32_encoder::utf32_encoder(    // Buffer constructor
     utf32_t*          addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{  reset(addr, size); }

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::set_mode
//
// Purpose-
//       Set the decoder MODE
//
//----------------------------------------------------------------------------
void
   utf32_encoder::set_mode(         // Set encoding mode
     MODE              M)           // The encoding mode
{
   if( offset || M > MODE_LE )      // If encoding started or invalid MODE
     throw utf_error("set_mode usage error");

   this->mode= M;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::encode
//
// Purpose-
//       Encode a codepoint
//
//----------------------------------------------------------------------------
unsigned                            // The encoding length, in units
   utf32_encoder::encode(           // Encode
     utf32_t           code)        // This codepoint
{
   if( offset >= length )           // If buffer full
     return 0;

   if( !is_unicode(code) )          // If code point is invalid
     code= UNI_REPLACEMENT;         // Encode replacement character

   if( offset == 0 || !is_combining(code) )
     ++column;
   buffer[offset++]= store32(code, mode);

   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       utf32_encoder::reset
//
// Purpose-
//       Reset the encoder
//
//----------------------------------------------------------------------------
void
   utf32_encoder::reset(            // Reset
     utf32_t*          addr,        // Buffer address
     Length            size)        // Buffer length (in bytes)
{
   if( addr == nullptr )
     size= 0;

   buffer= addr;
   length= size;
   column= offset= 0;
   mode= MODE_BE;
}
}  // namespace _LIBPUB_NAMESPACE
