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
//       Number.i
//
// Purpose-
//       Implement the methods defined in Number.h.
//
// Last change date-
//       2021/10/01
//
// Implementation notes-
//       Only included by Number.h (inside namespace _PUB_NAMESPACE)
//
//----------------------------------------------------------------------------
#ifndef _PUB_NUMBER_I_INCLUDED
#define _PUB_NUMBER_I_INCLUDED

//----------------------------------------------------------------------------
//
// Methods-
//       Number::Number
//
// Purpose-
//       Inline constructors
//
//----------------------------------------------------------------------------
   Number::Number(                  // Copy constructor
     const Number&     copy)        // Copying this Number
:  size(copy.size)
{  fetch(copy); }

   Number::Number(                  // Move constructor
     Number&&          move) noexcept // Moving this Number
:  data(move.data), size(move.size) // Take the data
{  move.reset(nullptr); }           // Reset the source

   Number::Number(                  // Value constructor
     intmax_t          copy)        // Copying this value
:  size(sizeof(intmax_t))
{  fetch(copy); }

   Number::Number(                  // Copy constructor
     const Byte*       word,        // Source Byte array
     size_t            count)       // Array byte count
{
   count += (sizeof(Word) - 1);      // Round up
   count &= ~(sizeof(Word) - 1);     // Truncate down
   if( count < MIN_SIZE )
     count= MIN_SIZE;
   size= count;

   fetch(word, count);
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::operator=
//
// Purpose-
//       Inline assignment operators
//
//----------------------------------------------------------------------------
Number&                             // (Always *this)
   Number::operator=(               // Copy assignment operator
     const Number&     copy)        // Source Number
{  fetch(copy); return *this; }

Number&                             // (Always *this)
   Number::operator=(               // Move assignment operator
     Number&&          move) noexcept // Source Number
{  this->data= move.data; this->size= move.size; // Take the data
   move.reset(nullptr);             // Reset the source
   return *this;
}

Number&                             // (Always *this)
   Number::operator=(               // Value assignment operator
     intmax_t          rhs)         // Source value
{  fetch(rhs); return *this; }

//----------------------------------------------------------------------------
//
// Method-
//       Number::get_data
//       Number::get_fill
//       Number::get_size
//       Number::get_minsize
//       Number::set_minsize
//
// Purpose-
//       Inline accessor function
//
//----------------------------------------------------------------------------
const Number::Byte*                 // The low order Byte
   Number::get_data( void ) const   // Get Data
{  return data; }

Number::Word                        // The fill Word
   Number::get_fill( void ) const   // Get fill Word
{  if( data == nullptr )
     return 0;
   return (data[size-1] & WORD_BIT) ? -1 : 0;
}

size_t
   Number::get_size( void ) const   // Get Word count
{  return size; }

size_t
   Number::get_minsize( void )      // Get global minimum Word count
{  return MIN_SIZE; }

void
   Number::set_minsize(             // Set global minimum Word count
     size_t            count)
{
   count += (sizeof(Word) - 1);     // Round up
   count &= ~(sizeof(Word) - 1);    // Truncate down
   if( count < sizeof(intmax_t) )   // (The minimum minimum)
     count= sizeof(intmax_t);

   MIN_SIZE= count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Number::inp(std::string)
//       Number::out(void)
//
// Purpose-
//       Inline conversion methods
//
//----------------------------------------------------------------------------
inline void
   Number::inp(
     std::string       S)
{  inp(S.c_str()); }

inline std::string
   Number::out( void ) const
{  return out("%d"); }

//----------------------------------------------------------------------------
//
// Function-
//       ::operator<<(ostream&, const Number&)
//
// Purpose-
//       Global operator (ostream << Number&)
//
//----------------------------------------------------------------------------
inline std::ostream&                // The updated ostream
   operator<<(                      // OUTPUT operator
     std::ostream&     OS,          // The ostream
     const Number&     NN)          // The Number
{  return (OS << NN.out()); }

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator !(const Number&)
//       ::operator==(const Number&, ...), ::operator==(..., const Number&)
//       ::operator!=(const Number&, ...), ::operator!=(..., const Number&)
//       ::operator<=(const Number&, ...), ::operator<=(..., const Number&)
//       ::operator>=(const Number&, ...), ::operator>=(..., const Number&)
//       ::operator< (const Number&, ...), ::operator< (..., const Number&)
//       ::operator> (const Number&, ...), ::operator> (..., const Number&)
//
// Purpose-
//       Global Number comparison operators
//
//----------------------------------------------------------------------------
inline bool operator! (const Number& rhs) {return (rhs.compare(0) == 0);}
inline bool operator==(const Number& lhs, const Number& rhs) {return lhs.compare(rhs) == 0;}
inline bool operator!=(const Number& lhs, const Number& rhs) {return lhs.compare(rhs) != 0;}
inline bool operator<=(const Number& lhs, const Number& rhs) {return lhs.compare(rhs) <= 0;}
inline bool operator>=(const Number& lhs, const Number& rhs) {return lhs.compare(rhs) >= 0;}
inline bool operator< (const Number& lhs, const Number& rhs) {return lhs.compare(rhs) <  0;}
inline bool operator> (const Number& lhs, const Number& rhs) {return lhs.compare(rhs) >  0;}

inline bool operator==(const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) == 0;}
inline bool operator!=(const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) != 0;}
inline bool operator<=(const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) <= 0;}
inline bool operator>=(const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) >= 0;}
inline bool operator< (const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) <  0;}
inline bool operator> (const Number& lhs, intmax_t rhs) {return lhs.compare(rhs) >  0;}

inline bool operator==(intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) == 0;}
inline bool operator!=(intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) != 0;}
inline bool operator<=(intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) >= 0;}
inline bool operator>=(intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) <= 0;}
inline bool operator< (intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) >  0;}
inline bool operator> (intmax_t lhs, const Number& rhs) {return rhs.compare(lhs) <  0;}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator<<(const Number&, int)
//       ::operator>>(const Number&, int)
//
// Purpose-
//       Global Number shift operators
//
//----------------------------------------------------------------------------
inline Number
   operator<<(
     const Number&     rhs,
     size_t            bits)
{  Number lhs(rhs); lhs <<= bits; return lhs; }

inline Number
   operator>>(
     const Number&     rhs,
     size_t            bits)
{  Number lhs(rhs); lhs >>= bits; return lhs; }

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator&(const Number&,...)
//       ::operator|(const Number&,...)
//       ::operator^(const Number&,...)
//
// Purpose-
//       Global Number bitwise and operators
//       Global Number bitwise or operators
//       Global Number bitwise exclusive or operators
//
//----------------------------------------------------------------------------
inline Number
   operator&(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS &= rhs; return LHS; }

inline Number
   operator&(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS &= rhs; return LHS; }

inline Number
   operator&(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS &= rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator|(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS |= rhs; return LHS; }

inline Number
   operator|(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS |= rhs; return LHS; }

inline Number
   operator|(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS |= rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator^(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS ^= rhs; return LHS; }

inline Number
   operator^(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS ^= rhs; return LHS; }

inline Number
   operator^(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS ^= rhs; return LHS; }

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator+(const Number&, ...), ::operator+(..., const Number&)
//       ::operator-(const Number&, ...), ::operator-(..., const Number&)
//       ::operator*(const Number&, ...), ::operator*(..., const Number&)
//       ::operator/(const Number&, ...), ::operator/(..., const Number&)
//       ::operator%(const Number&, ...), ::operator%(..., const Number&)
//
// Purpose-
//       Global arithmetic operators
//
//----------------------------------------------------------------------------
inline Number
   operator+(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS += rhs; return LHS; }

inline Number
   operator+(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS += rhs; return LHS; }

inline Number
   operator+(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS += rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator-(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS -= rhs; return LHS; }

inline Number
   operator-(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS -= rhs; return LHS; }

inline Number
   operator-(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS -= rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator*(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS *= rhs; return LHS; }

inline Number
   operator*(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS *= rhs; return LHS; }

inline Number
   operator*(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS *= rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator/(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS /= rhs; return LHS; }

inline Number
   operator/(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS /= rhs; return LHS; }

inline Number
   operator/(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS /= rhs; return LHS; }

//----------------------------------------------------------------------------
inline Number
   operator%(
     const Number&     lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS %= rhs; return LHS; }

inline Number
   operator%(
     const Number&     lhs,
     intmax_t          rhs)
{  Number LHS(lhs); LHS %= rhs; return LHS; }

inline Number
   operator%(
     intmax_t          lhs,
     const Number&     rhs)
{  Number LHS(lhs); LHS %= rhs; return LHS; }
#endif // _PUB_NUMBER_I_INCLUDED
