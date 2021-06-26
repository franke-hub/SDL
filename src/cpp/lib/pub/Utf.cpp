//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
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
//       2021/06/26
//
//----------------------------------------------------------------------------
#include <functional>               // For std::function
#include <new>                      // For std::bad_alloc
#include <string>                   // For std::string

#include <cassert>                  // For assert
#include <cstdlib>                  // For free, malloc, ...
#include <cstring>                  // For strcpy, strlen, ...
#include <arpa/inet.h>              // For htons, ntohs

#include "pub/Debug.h"              // For pub::Debug, namespace pub::debugging
#include "pub/Utf.h"                // Implementation class
#include "pub/utility.h"            // For pub::to_string

#define _PUB _PUB_NAMESPACE
using namespace _PUB::debugging;    // For debugging utility functions
using _PUB::utility::to_string;     // For pub::utility::to_string

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Compile-time options
{  HCDM= false                      // Hard Core Debug Mode?
,  ROUND_SIZE= 16                   // Allocation rounding size (power of two)
,  ROUND_MASK= ~(ROUND_SIZE-1)      // Allocation rounding mask
};                                  // Compile-time options

enum                                // Unicode characters
{  BYTE_ORDER_MARK= 0x00FEFF        // Byte Order Mark, a.k.a BOM
,  MARK_ORDER_BYTE= 0x00FFFE        // Little endian Byte Order Mark
}; // enum Unicode characters

//----------------------------------------------------------------------------
// _PUB_NAMESPACE ------------------------------------------------------------
//----------------------------------------------------------------------------
namespace _PUB_NAMESPACE {
typedef Utf::utf8_t    utf8_t;      // Import Utf::utf8_t
typedef Utf::utf16_t   utf16_t;     // Import Utf::utf16_t
typedef Utf::utf32_t   utf32_t;     // Import Utf::utf32_t

enum { UNI_REPLACEMENT= Utf::UNI_REPLACEMENT }; // Import Utf::UNI_REPLACEMENT
enum UTF16_OPTIONS // UTF-16 iterator controls
{  UTF16_LE=   0x00000001           // Option: UTF-16 little endian
};

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
// Utf8 utilities, not directly exposed.
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
//       pub::Utf8::~Utf8
//       pub::Utf8::Utf8
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
//       pub::Utf8::get_init
//       pub::Utf8::init
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
//       pub::Utf8::operators
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
//       pub::Utf8::decode
//
// Purpose-
//       Decode the next code point
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The next UTF32 code point
   Utf8::decode(                    // Decode next code point
     const utf8_t*     utf)         // Encoding buffer pointer
{  return utf8code(utf, 0); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::Utf8::encode
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

   unsigned size= 1;
   if( code >= 0x000080 ) {
     size= 2;
     if( code >= 0x000800 ) {
       size= 3;
       if( code >= 0x010000 ) {
         size= 4;
       }
     }
   }

   for(unsigned i= size; i > 1; --i) {
     buff[i-1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
   }

   static const utf8_t lead[8]= {0x00, 0xC0, 0xE0, 0xF0};
   buff[0]= (utf8_t)(code | lead[size-1]);
   return size;
#if 0 // Alternately ---------------------------------------------------------
   if( code < 0x000080 ) {          // Single unit encoding
     buff[0]= (utf8_t)code;
     return 1;
   }

   if( code < 0x000800 ) {          // Two unit encoding
     buff[1]= (utf8_t)((code & 0x3F) | 0x80);
     code >>= 6;
     buff[0]= (utf8_t)(code | 0xC0);
     return 2;
   }

   if( code < 0x010000 ) {          // Three unit encoding
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
#endif // Alternately --------------------------------------------------------
}

//----------------------------------------------------------------------------
//
// Method-
//       pub::Utf8::reset
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
//       pub::Utf16::~Utf16
//       pub::Utf16::Utf16
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
//       pub::Utf16::get_init
//       pub::Utf16::init
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
//       pub::Utf16::operators
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
//       pub::Utf16::decode
//
// Purpose-
//       Decode the next code point
//
//----------------------------------------------------------------------------
Utf::utf32_t                        // The next UTF32 code point
   Utf16::decode(                   // Decode next code point
     const utf16_t*    buff)        // Encoding buffer pointer
{  (void)buff; // NOT CODED YET

   return 0;
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
//       pub::Utf16::reset
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
//       pub::Utf32::~Utf32
//       pub::Utf32::Utf32
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
//       pub::Utf32::get_init
//       pub::Utf32::init
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
//       pub::Utf32::operators
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
//       pub::Utf32::decode
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
//       pub::Utf32::reset
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
}  // namespace _PUB_NAMESPACE
