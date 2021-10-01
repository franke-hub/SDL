//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Number.cpp
//
// Purpose-
//       Implement Number.h
//
// Last change date-
//       2021/10/01
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::to_string

#include "pub/Number.h"             // The implementation class

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
using _PUB_NAMESPACE::utility::to_string;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
}; // enum

// NOTE: ONLY MODE_BRINGUP IMPLEMENTED
// MODE_BRINGUP intended to be used to debug/verify Test_Num.cpp
#define MODE_BRINGUP 0              // Bringup: Only supports intmax_t
#define MODE_BINARY  1              // Binary: Implements com/Binary
#define MODE_MUMBER  2              // Word mode Number
#define IMPLEMENT MODE_BINARY       // Compile mode

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define wordof(x) (x / sizeof(Word))

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
size_t                 Number::MIN_SIZE= sizeof(intmax_t);

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char      toDec[]= "9876543210123456789";
static const char      toHex[]= "0123456789abcdef0x";
static const char      toHEX[]= "0123456789ABCDEF0X";

//----------------------------------------------------------------------------
//
// Subroutine-
//       iMAX
//       uMAX
//
// Purpose-
//       Convert Number& or Number* to intmax_t*
//       Convert Number& or Number* to uintmax_t*
//
//----------------------------------------------------------------------------
#if IMPLEMENT == MODE_BRINGUP
static const intmax_t  imax_0= 0;

static inline const intmax_t*
   iMAX(const Number& rhs)
{  const intmax_t* data= (const intmax_t*)rhs.get_data();
   return data ? data : &imax_0;
}

static inline intmax_t*
   iMAX(Number* lhs)
{  return (intmax_t*)lhs->get_data(); }

static inline const uintmax_t*
   uMAX(const Number& rhs)
{  const uintmax_t* data= (const uintmax_t*)rhs.get_data();
   return data ? data : (uintmax_t*)&imax_0;
}

static inline uintmax_t*
   uMAX(Number* lhs)
{  return (uintmax_t*)lhs->get_data(); }
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       allocate
//       release
//
// Purpose-
//       Allocate storage, throwing std::bad_alloc if failure
//       Release storage
//
// Implementation notes-
//       We might want to have some sort of data allocation pool.
//
//----------------------------------------------------------------------------
static inline Number::Word*         // The allocated storage
   allocate(                        // Allocate Word array
     size_t            size)        // Byte count
{
   if( size == 0 )                  // If default size
     return nullptr;                // Return, empty (zero) Number

   size_t length= size;             // Length in bytes
   length += sizeof(size_t) - 1;    // Round up
   length &= ~(sizeof(size_t) - 1); // Truncate down

   Number::Word* data= (Number::Word*)malloc(length); // Allocate Word buffer
   if( data == nullptr )            // If failure
     throw std::bad_alloc();

   memset(data, 0, length);         // Initialize to zero
   if( HCDM )
     debugf("Number::allocate @%p\n", data);
   return data;
}

static inline void
   release(                         // Release Word array
     Number::Word*     data)        // The data area
{  if( HCDM )
     debugf("Number::release @%p\n", data);

   ::free(data);
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::~Number
//       Number::Number
//
// Purpose-
//       Destructors
//       Constructor
//
//----------------------------------------------------------------------------
   Number::~Number( void )          // Destructor
{  if( HCDM )
     debug("~Number");

   release(data);
   data= nullptr;
}

   Number::Number( void )           // Default constructor
:  size(MIN_SIZE)
{  fetch(0); }

//----------------------------------------------------------------------------
//
// Method-
//       Number::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Number::debug(const char* info) const  // Debugging display
{  debugf("Number(%p)::debug(%s) size(0x%lX,#%ld)\n", this, info, size, size);
   if( data ) {
     debugf("..data(%p) 0x", data);
     for(size_t i= 0; i<size; i++) {
       if( i ) debugf("'");
       debugf("%.2x", ((unsigned char*)data)[i]);
     }
     debugf(", #%ld\n", *((intmax_t*)data));
   } else {
     debugf("..data(nullptr)\n");
   }
}

void
   Number::debug( void ) const      // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
//
// Method-
//       Number::set_size
//
// Purpose-
//       Get the number of significant Words
//       Set the Number size, expanding or truncating the value
//
//----------------------------------------------------------------------------
void
   Number::set_size(                // Resize the Number
     size_t            count)       // To this byte count
{
#if IMPLEMENT == MODE_BRINGUP
   count= sizeof(intmax_t);
#endif

   count += (sizeof(Word) - 1);     // Round up
   count &= ~(sizeof(Word) - 1);    // Truncate down
   if( count < MIN_SIZE )
     count= MIN_SIZE;

   if( count == size )              // If size unchanged
     return;

   Word* into= allocate(count);     // Replacement buffer (never nullptr)
   if( count < size ) {             // (Note: data truncated)
     for(size_t i= 0; i<count; ++i)
       into[i]= data[i];
   } else {
     for(size_t i= 0; i<size; ++i)
       into[i]= data[i];
     Word fill= get_fill();
     for(size_t i= size; i<count; ++i)
       into[i]= fill;
   }

   release(data);                   // Delete the original data
   data= into;                      // Set new buffer
   size= count;                     // Set new size
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::operator&=
//       Number::operator|=
//       Number::operator^=
//
// Purpose-
//       Bitwise replacement operators
//
//----------------------------------------------------------------------------
Number&
   Number::operator&=(              // Bitwise AND operator
     const Number&     rhs)
{
   if( rhs.data ) {
     size_t i= 0;
     while( i < size && i < rhs.size ) {
       data[i] &= rhs.data[i];
       ++i;
     }

     Word fill= rhs.get_fill();
     while( i < size )
       data[i++] &= fill;
   } else {
     for(size_t i= 0; i<size; ++i)
       data[i]= 0;
   }

   return *this;
}

Number&
   Number::operator&=(              // Bitwise AND operator
     intmax_t          rhs)
{
   for(size_t i= 0; i<size; ++i) {
     data[i] &= rhs;
     rhs >>= BITS_PER_WORD;
   }

   return *this;
}

//----------------------------------------------------------------------------
Number&
   Number::operator|=(              // Bitwise OR operator
     const Number&     rhs)
{
   if( rhs.data ) {
     size_t i= 0;
     while( i < size && i < rhs.size ) {
       data[i] |= rhs.data[i];
       ++i;
     }

     Word fill= rhs.get_fill();
     while( i < size )
       data[i++] |= fill;
   }

   return *this;
}

Number&
   Number::operator|=(              // Bitwise OR operator
     intmax_t          rhs)
{
   for(size_t i= 0; i<size; ++i) {
     data[i] |= rhs;
     rhs >>= BITS_PER_WORD;
   }

   return *this;
}

//----------------------------------------------------------------------------
Number&
   Number::operator^=(              // Bitwise XOR operator
     const Number&     rhs)
{
   if( rhs.data ) {
     size_t i= 0;
     while( i < size && i < rhs.size ) {
       data[i] ^= rhs.data[i];
       ++i;
     }

     Word fill= rhs.get_fill();
     while( i < size )
       data[i++] ^= fill;
   }

   return *this;
}

Number&
   Number::operator^=(              // Bitwise XOR operator
     intmax_t          rhs)
{
   for(size_t i= 0; i<size; ++i) {
     data[i] ^= rhs;
     rhs >>= BITS_PER_WORD;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::operator~
//       Number::operator+
//       Number::operator-
//
// Purpose-
//       Arithmetic unary operators
//
//----------------------------------------------------------------------------
Number
   Number::operator~( void ) const  // Ones complement operator
{  Number lhs(*this);
   for(size_t i= 0; i<wordof(size); i++)
     lhs.data[i] ^= Word(-1);
   return lhs;
}

Number
   Number::operator+( void ) const  // Plus operator (returns copy!)
{  Number lhs(*this); return lhs; }

Number
   Number::operator-( void ) const  // Negation operator
{  Number lhs= operator~(); lhs += 1; return lhs; }

//----------------------------------------------------------------------------
//
// Method-
//       Number::operator<<=
//       Number::operator>>=
//       Number::operator+=
//       Number::operator-=
//       Number::operator*=
//       Number::operator/=
//       Number::operator%=
//
//       Number::operator++
//       Number::operator--
//
// Purpose-
//       Arithmetic operators
//
//----------------------------------------------------------------------------
Number&
   Number::operator<<=(             // Shift left operator
     size_t          rhs)
{
#if IMPLEMENT != MODE_BRINGUP
   if( data && rhs ) {              // If non-zero data and non-zero shift
     size_t word_shift= rhs / BITS_PER_WORD; // Word shift count
     int    bits_shift= rhs % BITS_PER_WORD; // Bit shift count
     int    ibit_shift= BITS_PER_WORD - bits_shift; // Inverted bits_shift
     word_shift %= wordof(size);

     if( bits_shift == 0 ) {
       for(size_t x= size-1; x >= word_shift; --x) {
         data[x]= data[x-word_shift];
       }
     } else {
       for(size_t x= size-1; x >  word_shift; --x) {
         data[x]= (data[x-word_shift-0]<<bits_shift)
                | (data[x-word_shift-1]>>ibit_shift);
       }

       data[word_shift]= data[0]<<bits_shift;
     }

     for(size_t x= 0; x < word_shift; ++x) {
       data[x]= 0;
     }
   }

  return *this;
#else
   if( data )
     *iMAX(this) <<= rhs;

   return *this;
#endif
}

//----------------------------------------------------------------------------
Number&
   Number::operator>>=(             // Shift right operator
     size_t          rhs)
{
#if IMPLEMENT != MODE_BRINGUP
   if( data && rhs ) {              // If non-zero data and non-zero shift
     size_t word_shift= rhs / BITS_PER_WORD; // Word shift count
     int    bits_shift= rhs % BITS_PER_WORD; // Bit shift count
     int    ibit_shift= BITS_PER_WORD - bits_shift; // Inverted bits_shift
     Word fill= get_fill();
     word_shift %= wordof(size);

     if( bits_shift == 0 ) {
       for(size_t x= 0; x < (size - word_shift); ++x) {
         data[x]= data[x+word_shift];
       }
     } else {
       for(size_t x= 0; x < (size - word_shift - 1); ++x) {
         data[x]= (data[x+word_shift+1]<<ibit_shift)
                | (data[x+word_shift+0]>>bits_shift);
       }

       size_t x= size - word_shift - 1;
       data[x]= (fill << ibit_shift) | (data[size-1] >> bits_shift);
     }

     for(size_t x= size-word_shift; x < size; ++x) {
       data[x]= fill;
     }
   }

   return *this;
#else
   if( data )
     *iMAX(this) >>= rhs;

   return *this;
#endif
}

//----------------------------------------------------------------------------
Number&                             // Sum
   Number::operator+=(              // Addition operator
     const Number&     rhs)
#if IMPLEMENT != MODE_BRINGUP
{  fetch();

   if( rhs.data ) {
     size_t rhs_x= 0;
     size_t lhs_x= 0;
     intmax_t carry= 0;
     while( lhs_x < wordof(size) && rhs_x < wordof(rhs.size) ) {
       carry >>= BITS_PER_WORD;
       carry += data[lhs_x];
       carry += rhs.data[rhs_x++];
       data[lhs_x++]= carry;
     }

     Word fill= rhs.get_fill();
     while( lhs_x < wordof(size) ) {
       carry >>= BITS_PER_WORD;
       carry += data[lhs_x];
       carry += fill;
       data[lhs_x++]= carry;
     }
   }

   return *this;
}
#else
{  *iMAX(this) += *iMAX(rhs); return *this; }
#endif

Number&                             // Sum
   Number::operator+=(              // Addition operator
     intmax_t          rhs)
#if IMPLEMENT != MODE_BRINGUP
{  fetch();

   size_t lhs_x= 0;
   intmax_t carry= 0;
   while( lhs_x < wordof(size) ) {
     carry >>= BITS_PER_WORD;
     carry += data[lhs_x];
     carry += (rhs & WORD_MAX);
     data[lhs_x++]= carry;
     rhs >>= BITS_PER_WORD;
   }

   return *this;
}
#else
{  *iMAX(this) += rhs; return *this; }
#endif

//----------------------------------------------------------------------------
Number&                             // Sum
   Number::operator-=(              // Subtraction operator
     const Number&     rhs)
#if IMPLEMENT != MODE_BRINGUP
{
   fetch();

   if( rhs.data ) {
     size_t rhs_x= 0;
     size_t lhs_x= 0;
     intmax_t carry= 1;
     while( lhs_x < wordof(size) && rhs_x < wordof(rhs.size) ) {
       carry += data[lhs_x];
       carry += Word(~rhs.data[rhs_x++]);
       data[lhs_x++]= carry;
       carry >>= BITS_PER_WORD;
     }

     Word fill= ~rhs.get_fill();
     while( lhs_x < wordof(size) ) {
       carry += data[lhs_x];
       carry += fill;
       data[lhs_x++]= carry;
       carry >>= BITS_PER_WORD;
     }
   }

   return *this;
}
#else
{  *iMAX(this) -= *iMAX(rhs); return *this; }
#endif

Number&                             // Difference
   Number::operator-=(              // Subtraction operator
     intmax_t          rhs)
#if IMPLEMENT != MODE_BRINGUP
{  fetch();

   size_t lhs_x= 0;
   intmax_t carry= 1;
   while( lhs_x < wordof(size) ) {
     carry += data[lhs_x];
     carry += Word(~rhs);
     data[lhs_x++]= carry;
     rhs >>= BITS_PER_WORD;
     carry >>= BITS_PER_WORD;
   }

   return *this;
}
#else
{  *iMAX(this) -= rhs; return *this; }
#endif

//----------------------------------------------------------------------------
Number&                             // Product
   Number::operator*=(              // Multiplication operator
     const Number&     rhs)
#if IMPLEMENT != MODE_BRINGUP
{  fetch();

   Number multiplicand(*this);
   Number multiplier(rhs);
   fetch(0);                        // (The product)
   size_t lhs_x= 0;
   while( lhs_x < wordof(size)) {
     Word word= multiplicand.data[lhs_x++];
     Word mask= 1;
     while( mask != 0 ) {
       if( word & mask )
         *this += multiplier;

       multiplier <<= 1;
       mask <<= 1;
     }
   }

   return *this;
}
#else
{  *iMAX(this) *= *iMAX(rhs); return *this; }
#endif

Number&                             // Product
   Number::operator*=(              // Multiplication operator
     intmax_t          rhs)
#if IMPLEMENT != MODE_BRINGUP
{  fetch();

   Number multiplicand(*this);
   fetch(0);
   size_t lhs_x= 0;
   while( lhs_x < wordof(size) && rhs ) {
     Word word= multiplicand.data[lhs_x++];
     Word mask= 1;
     while( mask != 0 ) {
       if( word & mask )
         *this += rhs;

       rhs <<= 1;
       mask <<= 1;
     }
   }

   return *this;
}
#else
{  *iMAX(this) *= rhs; return *this; }
#endif

//----------------------------------------------------------------------------
Number&                             // Quotient
   Number::operator/=(              // Division operator
     const Number&     rhs)         // (Divisor)
#if IMPLEMENT != MODE_BRINGUP       // Marker
{
   if( rhs == 0 )
     throw std::runtime_error("Divide by zero");

   Number divisor(nullptr, size + rhs.size);
   divisor= rhs;
   divisor <<= size * BITS_PER_BYTE;
   Word rhs_fill= rhs.get_fill();
   if( rhs_fill )                   // If negative, make positive
     divisor.negate();

   Number remainder(*this);
   Word lhs_fill= get_fill();
   if( lhs_fill )                   // If negative, make positive
     remainder.negate();

   fetch(0);                        // (*this is the quotient)

   // TODO: USE Word logic (This uses Byte logic)
   for(size_t i= size; i > 0; --i) {
     Byte byte= 0;
     Byte mask= 0x80;
     while( mask != 0 ) {
       divisor.srl(1);
       if( remainder >= divisor ) {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     ((Byte*)data)[i-1]= byte;
   }

   if( lhs_fill != rhs_fill )       // If required, negate quotient
     this->negate();

   return *this;
}
#else
{  *iMAX(this) /= *iMAX(rhs); return *this; }
#endif

Number&                             // Quotient
   Number::operator/=(              // Division
     intmax_t          rhs)
{  Number RHS(rhs); return this->operator/=(RHS); }

//----------------------------------------------------------------------------
Number&                             // Remainder
   Number::operator%=(              // Modulus operator
     const Number&     rhs)
#if IMPLEMENT != MODE_BRINGUP
{
   if( rhs == 0 )
     throw std::runtime_error("Divide by zero");

   Number divisor(rhs);
   divisor.set_size(size + rhs.size);
   divisor <<= size * BITS_PER_BYTE;
   Word rhs_fill= rhs.get_fill();
   if( rhs_fill )                   // If negative, make positive
     divisor.negate();

   Number remainder(*this);
   Word lhs_fill= get_fill();
   if( lhs_fill )                   // If negative, make positive
     remainder.negate();

   fetch(0);                        // (*this is the quotient)

   // TODO: USE Word logic (This uses Byte logic)
   for(size_t i= size; i > 0; --i) {
     Byte byte= 0;
     Byte mask= 0x80;
     while( mask != 0 ) {
       divisor.srl(1);
       if( remainder >= divisor ) {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     ((Byte*)data)[i-1]= byte;
   }
   if( lhs_fill )
     remainder.negate();

   *this= remainder;
   return *this;
}
#else
{  *iMAX(this) %= *iMAX(rhs); return *this; }
#endif

Number&                             // Remainder
   Number::operator%=(              // Modulus operator
     intmax_t          rhs)
{  Number RHS(rhs); return this->operator%=(RHS); }

//----------------------------------------------------------------------------
Number&
   Number::operator++( void )       // Prefix increment operator
{  *this += 1; return *this; }

Number&
   Number::operator--( void )       // Prefix decrement operator
{  *this -= 1; return *this; }

//----------------------------------------------------------------------------
Number
   Number::operator++(int)         // Postfix increment operator
{  Number lhs(*this); *this += 1; return lhs; }

Number
   Number::operator--( int )       // Postfix decrement operator
{  Number lhs(*this); *this -= 1; return lhs; }

//----------------------------------------------------------------------------
//
// Method-
//       Number::divmod
//       Number::negate
//       Number::srl
//
// Purpose-
//       Divide by divisor, return integer modulus.
//       Negate this Number
//       Shift right logical
//
//----------------------------------------------------------------------------
int                                // Integer modulus
   Number::divmod(                 // Integer divide, return modulus
     int               rhs)        // The divisor
#if IMPLEMENT != MODE_BRINGUP
{
   if( rhs == 0 )
     throw std::runtime_error("Divide by zero");

   Number divisor(nullptr, size + sizeof(intmax_t)); // TODO: sizeof(int)???
   divisor= rhs;
   divisor <<= size * BITS_PER_BYTE;
   Word rhs_fill= rhs < 0 ? -1 : 0;
   if( rhs_fill )                   // If negative, make positive
     divisor.negate();

   Number remainder(*this);
   Word lhs_fill= get_fill();
   if( lhs_fill )                   // If negative, make positive
     remainder.negate();

   fetch(0);                        // (*this is the quotient)

   // TODO: USE Word logic (This uses Byte logic)
   for(size_t i= size; i > 0; --i) {
     Byte byte= 0;
     Byte mask= 0x80;
     while( mask != 0 ) {
       divisor.srl(1);
       if( remainder >= divisor ) {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     ((Byte*)data)[i-1]= byte;
   }

   if( lhs_fill != rhs_fill )       // If required, negate quotient
     this->negate();

   int result= *(int*)remainder.data;
   if( lhs_fill )
     result= -result;               // If required, negate remainder

   return result;
}
#else
{  intmax_t result= *iMAX(this) %= *iMAX(rhs);
   *iMAX(this) /= *iMAX(rhs);
   return result;
}
#endif

Number&                            // (Always *this)
   Number::negate( void )          // Negate this value
{
   intmax_t borrow= 1;
   for(size_t i= 0; i < size; ++i) {
     borrow += Byte(255 - ((Byte*)data)[i]);
     ((Byte*)data)[i]= borrow;
     borrow >>= BITS_PER_BYTE;
   }

   return *this;
}

Number&                            // (*this) >>= rhs
   Number::srl(                    // Shift right logical (unsigned)
     size_t            rhs)
{
#if IMPLEMENT != MODE_BRINGUP
   if( data && rhs ) {              // If non-zero data and non-zero shift
     size_t word_shift= rhs / BITS_PER_WORD; // Word shift count
     int    bits_shift= rhs % BITS_PER_WORD; // Bit shift count
     int    ibit_shift= BITS_PER_WORD - bits_shift; // Inverted bits_shift

     if( word_shift > wordof(size) ) {
       string S= to_string("Number(%p)::operator>>=(%zu)", this, rhs);
       throw std::out_of_range(S);
     }

     if( bits_shift == 0 ) {
       for(size_t x= 0; x < (size - word_shift); ++x) {
         data[x]= data[x+word_shift];
       }
     } else {
       for(size_t x= 0; x < (size - word_shift - 1); ++x) {
         data[x]= (data[x+word_shift+1]<<ibit_shift)
                | (data[x+word_shift+0]>>bits_shift);
       }

       size_t x= size - word_shift - 1;
       data[x]= data[size-1] >> bits_shift;
     }

     for(size_t x= size-word_shift; x < size; ++x) {
       data[x]= 0;
     }
   }

   return *this;
#else
   if( data )
     *uMAX(this) >>= rhs;

   return *this;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::compare
//
// Purpose-
//       Comparison operator
//
//----------------------------------------------------------------------------
int                               // Result: <0, 0, >0
   Number::compare(               // Compare *this :: rhs
     const Number&     rhs) const
{
   Word lhs_fill= get_fill();
   Word rhs_fill= rhs.get_fill();

   if( lhs_fill != rhs_fill ) {
     if( lhs_fill )
       return -1;
     else
       return +1;
   }

   int result= 0;
   if( size >= rhs.size ) {
     for(size_t i= size; i > rhs.size && result == 0; --i)
       result= ((Byte*)data)[i-1] - rhs_fill;
     for(size_t i= rhs.size; result == 0 && i > 0; --i)
       result= ((Byte*)data)[i-1] - ((Byte*)rhs.data)[i-1];
   } else {                         // if(size < rhs.size)
     for(size_t i= rhs.size; i > size && result == 0; --i)
       result= lhs_fill - ((Byte*)rhs.data)[i-1];
     for(size_t i= size; result == 0 && i > 0; --i)
       result= ((Byte*)data)[i-1] - ((Byte*)rhs.data)[i-1];
   }

   return result;
}

int                               // Result: <0, 0, >0
   Number::compare(               // Compare *this :: rhs
     intmax_t          rhs) const
{  Number RHS(rhs); return compare(RHS); }

//----------------------------------------------------------------------------
//
// Method-
//       Number::inp
//       Number::out
//
// Purpose-
//       Set value from c-string type
//       Set string from value
//
//----------------------------------------------------------------------------
void
   Number::inp(                     // Set value using
     const char*       txt)         // This input string
{  (void)txt; throw std::runtime_error("NOT IMPLEMENTED"); }

std::string                         // The string representation
   Number::out(                     // Set string representation
     const char*       fmt) const   // Using this printf format string
#if IMPLEMENT != MODE_BRINGUP
{
   std::string         result;      // Resultant

   Word fill= get_fill();
   for(size_t x= 0; fmt[x] != '\0'; x++) {
     char C= fmt[x];
     if( C != '%' ) {
       result += C;
       continue;
     }

     if( fmt[++x] == '%' ) {
       result += '%';
       continue;
     }

     // Set flags and indicators
     bool isLT= (fill != 0);        // Value is less than zero
     bool isNZ= (*this != 0);       // Value is non-zero

     bool flagFP= false;            // Flag: Field Precision specified
     bool flagLJ= false;            // Flag: Left justification
     bool flagPS= false;            // Flag: Include positive sign
     bool flagBL= false;            // Flag: Use blank in liu of positive sign
     bool flagNS= false;            // Flag: 0, 0x, or 0X prefix
     bool flagZP= false;            // Flag: Use zero padding
     for(;;x++) {
       C= fmt[x];
       if( C == '-' )
         flagLJ= true;
       else if( C == '+' )
         flagPS= true;
       else if( C == ' ' )
         flagBL= true;
       else if( C == '#' )
         flagNS= true;
       else if( C == '0' )
         flagZP= true;
       else
         break;
     }

     // Set field width
     int fw= 0;
     while( fmt[x] >= '0' && fmt[x] <= '9' ) {
       fw *= 10;
       fw += (fmt[x++] - '0');
       if( fw < 0 )
         throw std::runtime_error("Invalid format:field width");
     }

     // Set field precision
     int fp= 0;
     if( fmt[x] == '.' ) {
       x++;
       flagFP= true;
       while( fmt[x] >= '0' && fmt[x] <= '9' ) {
         fp *= 10;
         fp += (fmt[x++] - '0');
         if( fp < 0 )
           throw std::runtime_error("Invalid format:field precision");
       }
     }

     // Working strings and values
     Number      v(*this);          // (Working copy of *this)
     std::string prefix;
     std::string invert;

     // Process format
     switch( fmt[x] ) {             // Handle format
       case 'o': {
         isLT= false;

         while( v != 0) {
           invert += toHex[v.data[0] & 0x0007];
           v.srl(3);
         }

         if( flagNS && (isNZ || flagPS) )
           prefix += '0';
         break;
       }
       case 'u':
         if( isLT ) {               // If signed, force unsigned
           isLT= false;
           v.set_size(size+1);
           v.data[size]= 0;         // (Or: v.data[v.size-1]= 0)
         }
         [[ fallthrough ]]
         ;;

       case 'd':
       case 'i':
         while( v != 0 ) {
           int i= v.divmod(10);
           invert += toDec[i+9];
         }

         if( fmt[x] != 'u' ) {
           if( flagPS && isLT == false )
             prefix += '+';
           else if( isNZ || flagFP == false ) {
             if( isLT )
               prefix += '-';
             else if( flagBL )
               prefix += ' ';
           }
         }
         break;

       case 'x':
       case 'X': {
         isLT= false;

         const char* table= toHex;
         if( fmt[x] == 'X' )
           table= toHEX;

         while( v != 0) {
           invert += table[v.data[0] & 0x000F];
           v.srl(4);
         }

         if( flagNS && isNZ )
           prefix += &table[16];

         break;
       }
       default:
         throw std::runtime_error("Invalid format");
     }

     // Handle precision
     if( flagFP ) {
       fp -= invert.size();
       while( fp > 0 ) {
         invert += '0';
         fp--;
       }
     } else if( !isNZ ) {
       invert= '0';
     }

     // Revert the invert, handling field width
     fw -= (prefix.size() + invert.size());
     if( fw > 0 && flagZP ) {
       while( fw > 0 ) {
         invert += '0';
         fw--;
       }
     }

     if( fw > 0 && flagLJ == false ) {
       while( fw > 0 ) {
         result += ' ';
         fw--;
       }
     }

     result += prefix;
     size_t M= invert.size();
     for(size_t i= 0; i<M; i++)
       result += invert[M-i-1];

     while( fw > 0 ) {
       result += ' ';
       fw--;
     }
   }

   return result;
}
#else
{  (void)fmt;

   char buffer[96];
   sprintf(buffer, "0x%.16lx, #%ld", *iMAX(*this), *iMAX(*this));
   return buffer;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Number::reset
//       Number::fetch(void)
//
// Purpose-
//       Reset, zeroing the Number
//       If nessary, convert empty value to storage representation
//
//----------------------------------------------------------------------------
void
   Number::reset(                   // Delete storate, zeroing the Number
     Word*             word)        // Data to release
{
   release(word);                   // Release the data
   data= nullptr;                   // (Avoids duplicate release)
}

void
   Number::fetch( void )            // If necessary, allocate storage
{  if( data )                       // If already allocated
     return;                        // Do nothing

   data= allocate(size);            // Allocate and zero the data
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::fetch
//       Number::store
//
// Purpose-
//       Fetch data
//       Store data
//
//----------------------------------------------------------------------------
void
   Number::fetch(                   // Fetch (copy)
     const Number&     copy)        // From this Number
{  if( HCDM ) debugf("Number(%p)::fetch(&%p)\n", this, &copy);

   fetch(copy.data, copy.size);     // Fetch from copy
}

void
   Number::fetch(                   // Fetch Byte array
     const Byte*       word,        // From this Byte array
     size_t            count)       // Of this length
{  if( HCDM ) debugf("Number(%p)::fetch(@%p,#%lu)\n", this, word, count);

   fetch();
   if( word != nullptr ) {
     if( count > size ) {
       for(size_t i= 0; i<size; ++i)
         data[i]= word[i];
       return;                      // (this->data truncated)
     }

     for(size_t i= 0; i<count; ++i)
       data[i]= word[i];

     Word fill= 0;
     if( count > 0 && (word[count-1] & WORD_BIT) != 0 )
       fill= -1;

     for(size_t i= count; i<size; i++)
       data[i]= fill;
   }
}

void
   Number::fetch(                   // Fetch
     intmax_t          rhs)         // This source value
{  if( HCDM ) debugf("Number(%p)::fetch(#%lu)\n", this, rhs);

   fetch();
   size_t M= get_size();
   for(size_t i= 0; i<M; i++) {
     data[i]= (Word)rhs;
     rhs >>= BITS_PER_WORD;
   }
}

void
   Number::store(                   // Store Word array
     Word*             into,        // Into this Word array
     size_t            count) const // Of this Word count length
{
   const Word buff= 0;              // Zeroed data buffer
   const Word* from= this->data;
   size_t left= this->size;
   if( from == nullptr ) {
     from= &buff;
     left= 1;
   }

   size_t i= 0;
   while( i<count ) {
     if( i >= left )
       break;
     into[i]= from[i];
     ++i;
   }

   Word fill= get_fill();
   while( i < count )
     into[i++]= fill;
}
}  // namespace _PUB_NAMESPACE
