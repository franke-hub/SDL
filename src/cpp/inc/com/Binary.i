//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Binary.i
//
// Purpose-
//       Implement the methods defined in Binary.h.
//
// Last change date-
//       2020/10/02
//
// Implementation notes-
//       if defined(HCDM), inttypes.h is required.
//
//----------------------------------------------------------------------------
#ifndef BINARY_I_INCLUDED
#define BINARY_I_INCLUDED

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
Binary&                             // Resultant (*this)
   Binary::operator=(               // Assignment operator
     const Binary&     R)           // Source Binary
{
   R.store(dataL, getSize());
   return *this;
}

Binary&                             // Resultant (*this)
   Binary::operator=(               // Assignment operator
     int64_t           R)           // Source value
{
   load(R);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::getData
//
// Purpose-
//       Accessor function
//
//----------------------------------------------------------------------------
const Binary::Byte*                 // The Data
   Binary::getData( void ) const    // Get Data
{
   return dataL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::getFill
//
// Purpose-
//       Accessor function
//
//----------------------------------------------------------------------------
int                                 // The sign extention, 0 or (-1)
   Binary::getFill( void ) const    // Get sign extention
{
   int                 resultant= 0;

   if( getSize() > 0 && dataL[0] >= 0x80 && getSigned() )
     resultant= (-1);

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       SignedBinary<SIZE>::~SignedBinary
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
   SignedBinary<SIZE>::~SignedBinary( void ) // Destructor
{
   #ifdef HCDM
     debugf("SignedBinary<%lu>(%p)::~SignedBinary()\n", SIZE, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Methods-
//       SignedBinary<SIZE>::SignedBinary
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
   SignedBinary<SIZE>::SignedBinary( void ) // Default constructor
:  Binary()
{
   #ifdef HCDM
     debugf("SignedBinary<%lu>(%p)::SignedBinary()\n", SIZE, this);
   #endif

   dataL= array;
   // NO initial value
}

template<Binary::Size_t SIZE>
   SignedBinary<SIZE>::SignedBinary(// Copy constructor
     const SignedBinary&
                       R)           // Source SignedBinary
:  Binary()
{
   #ifdef HCDM
     debugf("SignedBinary<%lu>(%p)::SignedBinary(SignedBinary& %p)\n",
            SIZE, this, &R);
   #endif

   dataL= array;
   R.store(dataL, SIZE);
}

template<Binary::Size_t SIZE>
   SignedBinary<SIZE>::SignedBinary(// Copy constructor
     const Binary&     R)           // Source Binary
:  Binary()
{
   #ifdef HCDM
     debugf("SignedBinary<%lu>(%p)::SignedBinary(Binary& %p)\n",
            SIZE, this, &R);
   #endif

   dataL= array;
   R.store(dataL, SIZE);
}

template<Binary::Size_t SIZE>
   SignedBinary<SIZE>::SignedBinary(// Value constructor
     int64_t           R)           // Value
:  Binary()
{
   #ifdef HCDM
     debugf("SignedBinary<%lu>(%p)::SignedBinary(%" PRId64 "))\n",
            SIZE, this, R);
   #endif

   dataL= array;
   load(R);
}

//----------------------------------------------------------------------------
//
// Method-
//       SignedBinary<SIZE>::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
Binary&                             // Resultant
   SignedBinary<SIZE>::operator=(   // Assignment operator
     const Binary&     R)           // Source Binary
{
   R.store(dataL, SIZE);
   return *this;
}

template<Binary::Size_t SIZE>
Binary&                             // Resultant (*this)
   SignedBinary<SIZE>::operator=(   // Assignment operator
     int64_t           R)           // Source value
{
   load(R);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       SignedBinary<SIZE>::getSize
//
// Purpose-
//       Get the size of the Binary
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
Binary::Size_t                      // The size
   SignedBinary<SIZE>::getSize( void ) const // Get size
{
   return SIZE;                     // Return SIZE
}

//----------------------------------------------------------------------------
//
// Method-
//       UnsignedBinary<SIZE>::~UnsignedBinary
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
   UnsignedBinary<SIZE>::~UnsignedBinary( void ) // Destructor
{
   #ifdef HCDM
     debugf("UnsignedBinary<%lu>(%p)::~UnsignedBinary()\n", SIZE, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Methods-
//       UnsignedBinary<SIZE>::UnsignedBinary
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
   UnsignedBinary<SIZE>::UnsignedBinary( void ) // Default constructor
:  Binary()
{
   #ifdef HCDM
     debugf("UnsignedBinary<%lu>(%p)::UnsignedBinary()\n", SIZE, this);
   #endif

   dataL= array;
   // NO initial value
}

template<Binary::Size_t SIZE>
   UnsignedBinary<SIZE>::UnsignedBinary( // Copy constructor
     const UnsignedBinary&
                       R)           // Source Binary
:  Binary()
{
   #ifdef HCDM
     debugf("UnsignedBinary<%lu>(%p)::UnsignedBinary(UnsignedBinary& %p)\n",
            SIZE, this, &R);
   #endif

   dataL= array;
   R.store(dataL, SIZE);
}

template<Binary::Size_t SIZE>
   UnsignedBinary<SIZE>::UnsignedBinary( // Copy constructor
     const Binary&     R)           // Source Binary
:  Binary()
{
   #ifdef HCDM
     debugf("UnsignedBinary<%lu>(%p)::UnsignedBinary(Binary& %p)\n",
            SIZE, this, &R);
   #endif

   dataL= array;
   R.store(dataL, SIZE);
}

template<Binary::Size_t SIZE>
   UnsignedBinary<SIZE>::UnsignedBinary(// Value constructor
     int64_t           R)           // Value
:  Binary()
{
   #ifdef HCDM
     debugf("UnsignedBinary<%lu>(%p)::UnsignedBinary(%" PRId64 "))\n",
            SIZE, this, R);
   #endif

   dataL= array;
   load(R);
}

//----------------------------------------------------------------------------
//
// Method-
//       UnsignedBinary<SIZE>::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
Binary&                             // Resultant
   UnsignedBinary<SIZE>::operator=( // Assignment operator
     const Binary&     R)           // Source Binary
{
   R.store(dataL, SIZE);
   return *this;
}

template<Binary::Size_t SIZE>
Binary&                             // Resultant (*this)
   UnsignedBinary<SIZE>::operator=( // Assignment operator
     int64_t           R)           // Source value
{
   load(R);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       UnsignedBinary<SIZE>::getSigned
//
// Purpose-
//       Get the signed attribute
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
int                                 // The signed attribute
   UnsignedBinary<SIZE>::getSigned( void ) const // Get signed attribute
{
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       UnsignedBinary<SIZE>::getSize
//
// Purpose-
//       Get the size of the Binary
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
Binary::Size_t                      // The size
   UnsignedBinary<SIZE>::getSize( void ) const // Get size
{
   return SIZE;                     // Return SIZE
}

//----------------------------------------------------------------------------
//
// Methods-
//       VarBinary::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
Binary&                             // Resultant (*this)
   VarBinary::operator=(            // Assignment operator
     const Binary&     R)           // Source Binary
{
   R.store(dataL, sizeL);
   return *this;
}

Binary&                             // Resultant (*this)
   VarBinary::operator=(            // Assignment operator
     int64_t           R)           // Source value
{
   load(R);
   return *this;
}

//----------------------------------------------------------------------------
//
// Function-
//       ::operator<<(ostream&, const Binary&)
//
// Purpose-
//       Define global operator (ostream << Binary)
//
//----------------------------------------------------------------------------
inline std::ostream&                // The updated ostream
   operator<<(                      // OUTPUT operator
     std::ostream&     os,          // The ostream
     const Binary&     binary)      // The Binary
{
   return (os << binary.out());
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator!(const Binary&)
//       ::operator==(const Binary&, ...)
//       ::operator!=(const Binary&, ...)
//       ::operator<=(const Binary&, ...)
//       ::operator>=(const Binary&, ...)
//       ::operator< (const Binary&, ...)
//       ::operator> (const Binary&, ...)
//
// Purpose-
//       Global Binary comparison operators
//
//----------------------------------------------------------------------------
inline int operator!(const Binary& U) {return (U.compare(0) == 0);}
inline int operator==(const Binary& L, const Binary& R) {return L.compare(R) == 0;}
inline int operator!=(const Binary& L, const Binary& R) {return L.compare(R) != 0;}
inline int operator<=(const Binary& L, const Binary& R) {return L.compare(R) <= 0;}
inline int operator>=(const Binary& L, const Binary& R) {return L.compare(R) >= 0;}
inline int operator< (const Binary& L, const Binary& R) {return L.compare(R) <  0;}
inline int operator> (const Binary& L, const Binary& R) {return L.compare(R) >  0;}

inline int operator==(const Binary& L, int64_t R) {return L.compare(R) == 0;}
inline int operator!=(const Binary& L, int64_t R) {return L.compare(R) != 0;}
inline int operator<=(const Binary& L, int64_t R) {return L.compare(R) <= 0;}
inline int operator>=(const Binary& L, int64_t R) {return L.compare(R) >= 0;}
inline int operator< (const Binary& L, int64_t R) {return L.compare(R) <  0;}
inline int operator> (const Binary& L, int64_t R) {return L.compare(R) >  0;}

inline int operator==(int64_t L, const Binary& R) {return R.compare(L) == 0;}
inline int operator!=(int64_t L, const Binary& R) {return R.compare(L) != 0;}
inline int operator<=(int64_t L, const Binary& R) {return R.compare(L) >= 0;}
inline int operator>=(int64_t L, const Binary& R) {return R.compare(L) <= 0;}
inline int operator< (int64_t L, const Binary& R) {return R.compare(L) >  0;}
inline int operator> (int64_t L, const Binary& R) {return R.compare(L) <  0;}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator~(const Binary&)
//       ::operator+(const Binary&)
//       ::operator-(const Binary&)
//
// Purpose-
//       Global unary operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator~(
     const Binary&     U)
{
   VarBinary           resultant(U);

   resultant^=(-1);

   return resultant;
}

inline VarBinary
   operator+(
     const Binary&     U)
{
   VarBinary           resultant(U);

   return resultant;
}

inline VarBinary
   operator-(
     const Binary&     U)
{
   VarBinary           resultant(U);

   resultant.negate();

   return resultant;
}


//----------------------------------------------------------------------------
//
// Methods-
//       ::operator++(Binary&,...)
//       ::operator--(Binary&,...)
//
// Purpose-
//       Global prefix/postfix increment/decrement operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator++(                      // Prefix unary operator ++
     Binary&           U)
{
   VarBinary           resultant(U += 1);

   return resultant;
}

inline VarBinary
   operator++(                      // Postfix unary operator ++
     Binary&           U,
     int               )            // (Indicates postfix)
{
   VarBinary           resultant(U);

   U += 1;

   return resultant;
}

inline VarBinary
   operator--(                      // Prefix unary operator --
     Binary&           U)
{
   VarBinary           resultant(U -= 1);

   return resultant;
}

inline VarBinary
   operator--(                      // Postfix unary operator --
     Binary&           U,
     int               )            // (Indicates postfix)
{
   VarBinary           resultant(U);

   U -= 1;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator<<(const Binary&, int)
//       ::operator>>(const Binary&, int)
//
// Purpose-
//       Global Binary shift operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator<<(
     const Binary&     L,
     int               R)
{
   VarBinary           resultant(L);

   resultant <<= R;

   return resultant;
}

inline VarBinary
   operator>>(
     const Binary&     L,
     int               R)
{
   VarBinary           resultant(L);

   resultant >>= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator&(const Binary&,...)
//
// Purpose-
//       Global Binary bitwise and operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator&(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant &= R;

   return resultant;
}

inline VarBinary
   operator&(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant &= R;

   return resultant;
}

inline VarBinary
   operator&(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant &= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator|(const Binary&,...)
//
// Purpose-
//       Global Binary bitwise or operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator|(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant |= R;

   return resultant;
}

inline VarBinary
   operator|(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant |= R;

   return resultant;
}

inline VarBinary
   operator|(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant |= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator^(const Binary&,...)
//
// Purpose-
//       Global Binary bitwise exclusive or operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator^(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant ^= R;

   return resultant;
}

inline VarBinary
   operator^(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant ^= R;

   return resultant;
}

inline VarBinary
   operator^(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant ^= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator+(const Binary&, ...)
//
// Purpose-
//       Global Binary addition operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator+(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant += R;

   return resultant;
}

inline VarBinary
   operator+(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant += R;

   return resultant;
}

inline VarBinary
   operator+(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant += R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator-(const Binary&, ...)
//
// Purpose-
//       Global Binary subtraction operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator-(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant -= R;

   return resultant;
}

inline VarBinary
   operator-(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant -= R;

   return resultant;
}

inline VarBinary
   operator-(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant -= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator*(const Binary&, ...)
//
// Purpose-
//       Global Binary multiply operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator*(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant *= R;

   return resultant;
}

inline VarBinary
   operator*(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant *= R;

   return resultant;
}

inline VarBinary
   operator*(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant *= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator/(const Binary&, ...)
//
// Purpose-
//       Global Binary divide operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator/(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant /= R;

   return resultant;
}

inline VarBinary
   operator/(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant /= R;

   return resultant;
}

inline VarBinary
   operator/(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant /= R;

   return resultant;
}

//----------------------------------------------------------------------------
//
// Methods-
//       ::operator%(const Binary&, ...)
//
// Purpose-
//       Global Binary modulus operators
//
//----------------------------------------------------------------------------
inline VarBinary
   operator%(
     const Binary&     L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(L, R);
   resultant= L;
   resultant.setSigned(L.getSigned() && R.getSigned());
   resultant %= R;

   return resultant;
}

inline VarBinary
   operator%(
     const Binary&     L,
     int64_t           R)
{
   VarBinary           resultant;

   resultant.initSize(L);
   resultant= L;
   resultant.setSigned(L.getSigned());
   resultant %= R;

   return resultant;
}

inline VarBinary
   operator%(
     int64_t           L,
     const Binary&     R)
{
   VarBinary           resultant;

   resultant.initSize(R);
   resultant= L;
   resultant.setSigned(R.getSigned());
   resultant %= R;

   return resultant;
}

#endif // BINARY_I_INCLUDED
