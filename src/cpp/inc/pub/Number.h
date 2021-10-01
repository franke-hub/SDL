//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Number.h
//
// Purpose-
//       Number of any (byte) size multiple precision.
//
// Last change date-
//       2021/10/01
//
//----------------------------------------------------------------------------
#ifndef _PUB_NUMBER_H_INCLUDED
#define _PUB_NUMBER_H_INCLUDED

#include <ostream>                  // For std::ostream
#include <string>                   // For std::string
#include <utility>                  // For std::pair
#include <stdint.h>                 // For size_t

#include <pub/macro/config.h>       // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {          // namespace pub
//----------------------------------------------------------------------------
//
// Class-
//       Number
//
// Purpose-
//       Define the Number base class.
//
// Implementation notes-
//       Arithmetic operations do not change the size of a Number.
//       All arithmetic operator methods treat Numbers as signed.
//
//       A Number's size is always rounded up to a Word multiple, and to a
//       minimum of MIN_SIZE. MIN_SIZE is always a Word multiple and never
//       less than sizeof(intmax_t).
//
//----------------------------------------------------------------------------
class Number {                      // Generic number of any size
//----------------------------------------------------------------------------
// Number::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint8_t        Byte;        // The Byte data type
typedef uint8_t        Word;        // The data Word type

enum Endian
{  NUM_BIG_ENDIAN                   // Big endian
,  NUM_LIL_ENDIAN                   // Little endian
}; // enum Endian

enum                                // Generic enum
{  BITS_PER_BYTE= 8                 // Number of bits in each Byte
,  BITS_PER_WORD= 8                 // Number of bits in each Word
,  NUM_BYTE_ORDER= NUM_LIL_ENDIAN   // Implementation Endian (!!MAY CHANGE!!)
}; // enum

static const Word      WORD_BIT= 0x80; // The high-order Word sign bit mask
static const Word      WORD_MAX= 0xFF; // The maximum word value

//----------------------------------------------------------------------------
// Number::Attributes
//----------------------------------------------------------------------------
protected:
// The high order size bit is the signed attribute
Word*                  data= nullptr; // The Word array
size_t                 size= 0;     // The Number size, in bytes

//----------------------------------------------------------------------------
// Number::Static attributes
//----------------------------------------------------------------------------
static size_t          MIN_SIZE;    // Global default minimum size (default 8)

//----------------------------------------------------------------------------
// Number::Destructor, constructors
//----------------------------------------------------------------------------
public:
   ~Number( void );                 // Destructor
   Number( void );                  // Default constructor

inline
   Number(const Number&);           // Copy constructor
inline
   Number(Number&&) noexcept;       // Move constructor
inline
   Number(intmax_t);                // Value constructor
inline
   Number(const Byte*, size_t);     // Copy Byte array constructor

//----------------------------------------------------------------------------
// Number::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging informative display
void debug( void ) const;           // Debugging display

//----------------------------------------------------------------------------
// Number::Assignment operators
//----------------------------------------------------------------------------
inline Number& operator=(const Number&); // Copy assignment
inline Number& operator=(Number&&) noexcept; // Move assignment
inline Number& operator=(intmax_t);  // Copy from integer

//----------------------------------------------------------------------------
// Number::Accessor methods
//----------------------------------------------------------------------------
// Note: get_data() can return nullptr for zero value (if moved from source)
inline const Byte*     get_data( void ) const; // Get data address
inline Word            get_fill( void ) const; // Get fill Word (-1 or 0)
inline size_t          get_size( void ) const; // Get size (in bytes)
       void            set_size(size_t); // Update the (Byte) size

static inline size_t   get_minsize( void ); // Get global minimum size
static inline void     set_minsize(size_t); // Set global minimum size

//----------------------------------------------------------------------------
// Number::Bitwise replacement operators
//----------------------------------------------------------------------------
Number& operator&=(const Number&);  // Replacement AND operator
Number& operator&=(intmax_t);       // Replacement AND operator

Number& operator|=(const Number&);  // Replacement OR operator
Number& operator|=(intmax_t);       // Replacement OR operator

Number& operator^=(const Number&);  // Replacement XOR operator
Number& operator^=(intmax_t);       // Replacement XOR operator

//----------------------------------------------------------------------------
// Number::Unary operators
//----------------------------------------------------------------------------
Number operator~() const;           // Unary ones complement (*this)
Number operator+(void) const;       // Unary equal to (*this)
Number operator-(void) const;       // Negate (*this)

//----------------------------------------------------------------------------
// Number::Arithmetic replacement operators
//----------------------------------------------------------------------------
Number& operator<<=(size_t);        // Shift left (*this)
Number& operator>>=(size_t);        // Shift right (*this)

Number& operator+=(const Number&);  // Add (to) (*this)
Number& operator+=(intmax_t);       // Add (to) (*this)

Number& operator-=(const Number&);  // Subtract (from) (*this)
Number& operator-=(intmax_t);       // Subtract (from) (*this)

Number& operator*=(const Number&);  // Multiply (*this) (by)
Number& operator*=(intmax_t);       // Multiply (*this) (by)

Number& operator/=(const Number&);  // Divide (*this) (by)
Number& operator/=(intmax_t);       // Divide (*this) (by)

Number& operator%=(const Number&);  // Modulus (*this) (by)
Number& operator%=(intmax_t);       // Modulus (*this) (by)

Number& operator++(void);           // Prefix increment operator
Number& operator--(void);           // Prefix decrement operator

Number  operator++(int);            // Postfix increment operator
Number  operator--(int);            // Postfix decrement operator

//----------------------------------------------------------------------------
// Number::Methods
//----------------------------------------------------------------------------
#if 0
Number&                             // (unsigned *this) +=
   add(const Number&);              // Unsigned add
Number&                             // (unsigned *this) +=
   add(intmax_t);                   // Unsigned add

Number&                             // (unsigned *this) -=
   sub(const Number&);              // Unsigned subtract
Number&                             // (unsigned *this) -=
   sub(intmax_t);                   // Unsigned subtract

Number&                             // (unsigned *this) *=
   mul(const Number&);              // Unsigned multiply
Number&                             // (unsigned *this) *=
   mul(intmax_t);                   // Unsigned multiply

Number&                             // (unsigned *this) /=
   div(const Number&);              // Unsigned divide
Number&                             // (unsigned *this) /=
   div(intmax_t);                   // Unsigned divide

Number&                             // (unsigned *this) %=
   mod(const Number&);              // Unsigned modulus
Number&                             // (unsigned *this) %=
   mod(intmax_t);                   // Unsigned modulus
#endif

int                                 // The integer modulus
   divmod(int);                     // Divide by integer, return modulus

Number&                             // (Always *this)
   negate( void );                  // Negate this Number

Number&                             // (unsigned *this) >>=
   srl(size_t bits);                // Shift right logical

int compare(const Number&) const;   // Compare (to), returns (<0, 0, >0)
int compare(intmax_t) const;        // Compare (to), returns (<0, 0, >0)

void inp(const char*);              // Load from ASCII string
void inp(std::string S);            // Load from ASCII string
std::string out(const char*) const; // Convert to std::string
std::string out(void) const;        // Convert to std::string

void reset(Byte*);                  // Reset, optionally releasing storage
void fetch(void);                   // Fetch nothing, insure storage allocated

void fetch(const Number&);          // Fetch from Number
void fetch(const Byte*, size_t);    // Fetch from Byte array
void fetch(intmax_t);               // Fetch from signed integer

void store(Byte*, size_t) const;    // Store into Byte array w\ expand/truncate
}; // class Number

#include "Number.i"                 // Inline methods and global operators

} // namespace _PUB_NAMESPACE
#endif // _PUB_NUMBER_H_INCLUDED
