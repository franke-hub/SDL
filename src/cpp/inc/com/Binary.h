//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Binary.h
//
// Purpose-
//       Define a number of any size, fixed or variable.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       The size is specified in bytes, or octets and can range from one
//       up to the maximum positive length contained in an int.
//
// Usage example-
//       SignedBinary<6>   value(); // Some 48-bit precision value
//       SignedBinary<2>   other(); // Some 16-bit precision value
//       SignedBinary<5>   third(); // Some 40-bit precision value
//
//       value= 1; other= 2;
//       third= value + value * other - value / other;
//       cout << third << endl;
//
// Usage notes-
//       Neither the size nor the signed attribute of a VarBinary change as
//       a result of an operation. These attributes are set during
//       construction, but may be explicitly changed at any time thereafter
//       using the setSize() and setSigned() functions.
//
//       Global operators are defined in Binary.i
//
//----------------------------------------------------------------------------
#ifndef BINARY_H_INCLUDED
#define BINARY_H_INCLUDED

#include <stdint.h>

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
#include <ostream>
#include <string>

//----------------------------------------------------------------------------
//
// Class-
//       Binary
//
// Purpose-
//       Define the Binary base class.
//
//----------------------------------------------------------------------------
class Binary {                      // Generic binary number
//----------------------------------------------------------------------------
// Binary::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef unsigned char  Byte;        // The data Byte type
typedef unsigned long  Size_t;      // The data size type

enum                                // Byte definitions
{  BITS_PER_BYTE= 8                 // Number of bits in each byte
}; // enum

//----------------------------------------------------------------------------
// Binary::Attributes
//----------------------------------------------------------------------------
protected:
Byte*                  dataL;       // -> data

//----------------------------------------------------------------------------
// Binary::Static attributes
//----------------------------------------------------------------------------
protected:
// Valid only if Binary.cpp compiled with INSTRUMENTATION defined
static int             objectCount; // Number of instantiated objects

//----------------------------------------------------------------------------
// Binary::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Binary( void );                 // Destructor

   Binary( void );                  // Default constructor

private:
   Binary(                          // Copy constructor
     const Binary&    source);      // Source Binary

//----------------------------------------------------------------------------
// Binary::Assignment operators
//----------------------------------------------------------------------------
public:
inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     const Binary&     source);     // Source Binary

inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     int64_t           source);     // Source value

//----------------------------------------------------------------------------
// Binary::Bitwise replacement operators
//----------------------------------------------------------------------------
public:
Binary&                             // Resultant (*this)
   operator&=(                      // Replace AND operator
     const Binary&     operand);    // Operand

Binary&                             // Resultant (*this)
   operator&=(                      // Replace AND operator
     int64_t           operand);    // Operand

Binary&                             // Resultant (*this)
   operator|=(                      // Replace OR operator
     const Binary&     operand);    // Operand

Binary&                             // Resultant (*this)
   operator|=(                      // Replace OR operator
     int64_t           operand);    // Operand

Binary&                             // Resultant (*this)
   operator^=(                      // Replace XOR operator
     const Binary&     operand);    // Operand

Binary&                             // Resultant (*this)
   operator^=(                      // Replace XOR operator
     int64_t           operand);    // Operand

//----------------------------------------------------------------------------
// Binary::Arithmetic replacement operators
//----------------------------------------------------------------------------
public:
Binary&                             // Resultant (*this)
   operator<<=(                     // Replace shift left operation
     int               bits);       // Bit count

Binary&                             // Resultant (*this)
   operator>>=(                     // Replace shift right operation
     int               bits);       // Bit count

Binary&                             // Resultant (*this)
   negate( void );                  // Negate value

Binary&                             // Resultant (*this)
   operator+=(                      // Replace ADD operator
     const Binary&     addend);     // Addend

Binary&                             // Resultant (*this)
   operator+=(                      // Replace ADD operator
     int64_t           addend);     // Addend

Binary&                             // Resultant (*this)
   operator-=(                      // Replace SUBTRACT operator
     const Binary&     subtrahend); // Subtrahend

Binary&                             // Resultant (*this)
   operator-=(                      // Replace SUBTRACT operator
     int64_t           subtrahend); // Subtrahend

Binary&                             // Resultant (*this)
   operator*=(                      // Replace MULTIPLY operator
     const Binary&     multiplicand); // Multiplicand

Binary&                             // Resultant (*this)
   operator*=(                      // Replace MULTIPLY operator
     int64_t           multiplicand); // Multiplicand

Binary&                             // Resultant (*this)
   operator/=(                      // Replace DIVIDE operator
     const Binary&     divisor);    // Divisor

Binary&                             // Resultant (*this)
   operator/=(                      // Replace DIVIDE operator
     int64_t           divisor);    // Divisor

Binary&                             // Resultant (*this)
   operator%=(                      // Replace MODULUS operator
     const Binary&     divisor);    // Divisor

Binary&                             // Resultant (*this)
   operator%=(                      // Replace MODULUS operator
     int64_t           divisor);    // Divisor

//----------------------------------------------------------------------------
// Binary::Methods
//----------------------------------------------------------------------------
public:
int                                 // Resultant (<0, 0, >0)
   compare(                         // Compare with
     const Binary&     comprahend) const; // Comprahend

int                                 // Resultant (<0, 0, >0)
   compare(                         // Compare with
     int64_t           comprahend) const; // Comprahend

inline const Byte*                  // The Data
   getData( void ) const;           // Get Data

inline int                          // The sign extention, 0 or (-1)
   getFill( void ) const;           // Get sign extention

virtual int                         // The signed attribute
   getSigned( void ) const;         // Get signed attribute

virtual Size_t                      // The size
   getSize( void ) const;           // Get SIZE

void
   inp(                             // Convert from string
     const char*       string);     // Source string

void
   inp(                             // Convert from string
     const std::string string);     // Source string

void
   load(                            // Load from Byte*
     const Byte*       source,      // Data byte array
     Size_t            size);       // Array size

void
   load(                            // Load from value
     int64_t           source);     // Data value

std::string                         // The output string
   out(                             // Convert to string
     const char*       format) const// As specified by format string
   _ATTRIBUTE_PRINTF(2, 0);

std::string                         // The output string
   out( void ) const;               // Convert to string

Size_t                              // The significant size
   sigSize( void ) const;           // Get significant size

void
   store(                           // Store into Byte*
     Byte*             target,      // Data byte array
     Size_t            size) const; // Array size

int64_t                             // Resultant
   toInt( void ) const;             // Return int64_t value
}; // class Binary

//----------------------------------------------------------------------------
//
// Class-
//       SignedBinary
//
// Purpose-
//       Define a fixed length signed number
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
class SignedBinary : public Binary {// A signed number
//----------------------------------------------------------------------------
// SignedBinary<SIZE>::Attributes
//----------------------------------------------------------------------------
protected:
Byte                   array[SIZE]; // The (invarient-size) data byte array

//----------------------------------------------------------------------------
// SignedBinary<SIZE>::Constructors
//----------------------------------------------------------------------------
public:
inline virtual
   ~SignedBinary( void );           // Destructor

inline
   SignedBinary( void );            // Default constructor

inline
   SignedBinary(                    // Copy constructor
     const SignedBinary&
                       source);     // Source Binary

inline
   SignedBinary(                    // Copy constructor
     const Binary&     source);     // Source Binary

inline
   SignedBinary(                    // Value constructor
     int64_t           source);     // Value

//----------------------------------------------------------------------------
// SignedBinary::Assignment operators
//----------------------------------------------------------------------------
public:
inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     const Binary&     source);     // Source Binary

inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     int64_t           source);     // Source value

//----------------------------------------------------------------------------
// SignedBinary<SIZE>::Methods
//----------------------------------------------------------------------------
public:
virtual Size_t                      // The size
   getSize( void ) const;           // Get SIZE
}; // class SignedBinary<SIZE>

//----------------------------------------------------------------------------
//
// Class-
//       UnsignedBinary
//
// Purpose-
//       Define a fixed length unsigned number
//
//----------------------------------------------------------------------------
template<Binary::Size_t SIZE>
class UnsignedBinary : public Binary { // An unsigned number
//----------------------------------------------------------------------------
// UnsignedBinary<SIZE>::Attributes
//----------------------------------------------------------------------------
protected:
Byte                   array[SIZE]; // The (invarient-size) data byte array

//----------------------------------------------------------------------------
// UnsignedBinary<SIZE>::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~UnsignedBinary( void );         // Destructor

inline
   UnsignedBinary( void );          // Default constructor

inline
   UnsignedBinary(                  // Copy constructor
     const UnsignedBinary&
                       source);     // Source UnsignedBinary

inline
   UnsignedBinary(                  // Copy constructor
     const Binary&     source);     // Source Binary

inline
   UnsignedBinary(                  // Value constructor
     int64_t           source);     // Value

//----------------------------------------------------------------------------
// UnsignedBinary::Assignment operators
//----------------------------------------------------------------------------
public:
inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     const Binary&     source);     // Source Binary

inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     int64_t           source);     // Source value

//----------------------------------------------------------------------------
// UnsignedBinary<SIZE>::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Get signed attribute
   getSigned( void ) const;         // Get signed attribute

virtual Size_t                      // The size
   getSize( void ) const;           // Get SIZE
}; // class UnsignedBinary<SIZE>

//----------------------------------------------------------------------------
//
// Class-
//       VarBinary
//
// Purpose-
//       Define the VarBinary class.
//
//----------------------------------------------------------------------------
class VarBinary : public Binary {   // Variable length number
//----------------------------------------------------------------------------
// VarBinary::Attributes
//----------------------------------------------------------------------------
protected:
int                    signL;       // TRUE iff signed
Size_t                 sizeL;       // The data size

//----------------------------------------------------------------------------
// VarBinary::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~VarBinary( void );              // Destructor
   VarBinary( void );               // Default constructor

   VarBinary(                       // Copy constructor
     const VarBinary&  source);     // Source VarBinary

   VarBinary(                       // Copy constructor
     const Binary&     source);     // Source Binary

   VarBinary(                       // Value constructor
     int64_t           source);     // Value

//----------------------------------------------------------------------------
// VarBinary::Assignment operators
//----------------------------------------------------------------------------
public:
inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     const Binary&     source);     // Source Binary

inline Binary&                      // Resultant
   operator=(                       // Assignment operator
     int64_t           source);     // Source value

//----------------------------------------------------------------------------
// VarBinary::Arithmetic operators
//----------------------------------------------------------------------------
int                                 // Remainder
   div(                             // Divide, return remainder
     int               divisor);    // Divisor

//----------------------------------------------------------------------------
// VarBinary::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // The signed attribute
   getSigned( void ) const;         // Get signed attribute

virtual void
   setSigned(                       // Set signed attribute
     int               sign);       // New signed attribute

virtual Size_t                      // The size
   getSize( void ) const;           // Get size

void
   initSize(                        // Initialize size (Content undefined)
     Size_t            size);       // New size

void                                // size= max(U.getSize(), sizeof(int64_t))
   initSize(                        // Initialize size (Content undefined)
     const Binary&     U);          // Implicit new size

void                                // size= max(L.getSize(), R.getSize())
   initSize(                        // Initialize size (Content undefined)
     const Binary&     L,           // Implicit new size
     const Binary&     R);          // Implicit new size

void
   setSize(                         // Set size (Preserve content)
     Size_t            size);       // New size
}; // class VarBinary

#include "Binary.i"

#endif // BINARY_H_INCLUDED
