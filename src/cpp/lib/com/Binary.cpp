//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Binary.cpp
//
// Purpose-
//       Binary object methods.
//
// Last change date-
//       2020/10/02
//
//----------------------------------------------------------------------------
#define __STDC_FORMAT_MACROS        // For linux inttypes.h
#include <inttypes.h>               // For PRId64

#include <ostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/Debug.h>

#include "com/Binary.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef INSTRUMENTATION
#undef  INSTRUMENTATION             // If defined, include instrumentation
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define DEBUG(b) debug(b, #b)

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
int                    Binary::objectCount= 0; // Valid with INSTRUMENTATION

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char      toDec[]= "9876543210123456789";
static const char      toHex[]= "0123456789abcdef0x";
static const char      toHEX[]= "0123456789ABCDEF0X";

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::debug
//
// Purpose-
//       Debug a Binary object
//
//----------------------------------------------------------------------------
static inline void
   debug(
     const Binary&     U,
     const char*       name= "Binary")
{
   const Binary::Byte* dataU= U.getData();
   Binary::Size_t      sizeU= U.getSize();
   const char*         signU;

   Binary::Size_t      i;

   debugf("debug(%s(%p) ", name, &U);

   signU= "=";
   if( U.getSigned() )
   {
     signU= "+";
     if( U.getFill() < 0 )
       signU= "-";
   }
   debugf("getSize(%lu) getData(%p): %s", sizeU, dataU, signU);

   for(i= 0; i<sizeU; i++)
   {
     if( i != 0 )
       debugf(".");

     debugf("%.2x", dataU[i]);
   }

   debugf(",%" PRId64 ")", U.toInt());
}

static inline void
   debug(
     const Binary*     U,
     const char*       name= "this")
{
   debug(*U, name);
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::~Binary
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Binary::~Binary( void )
{
   #ifdef HCDM
     debugf("Binary(%p)::~Binary()\n", this);
   #endif

#ifdef INSTRUMENTATION
   objectCount--;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::Binary
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Binary::Binary( void )
:  dataL(NULL)
{
   #ifdef HCDM
     debugf("Binary(%p)::Binary()\n", this);
   #endif

#ifdef INSTRUMENTATION
   objectCount++;
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::load
//
// Purpose-
//       Load from Byte*
//
//----------------------------------------------------------------------------
void
   Binary::load(                    // Load from Byte*
     const Byte*       dataR,       // Data byte array
     Size_t            sizeR)       // Array size
{
   Byte                fillR;
   Size_t              sizeL= getSize();

   while( sizeL > 0 && sizeR > 0 )
     dataL[--sizeL]= dataR[--sizeR];

   if( sizeL > 0 )
   {
     fillR= 0;
     if( sizeR > 0 && dataR[0] >= 0x80 )
       fillR= 0xFF;

     while( sizeL > 0 )
       dataL[--sizeL]= fillR;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::load
//
// Purpose-
//       Load from int64_t
//
//----------------------------------------------------------------------------
void
   Binary::load(                    // Load from value
     int64_t           R)           // Data value
{
   Size_t              sizeL= getSize();

   while( sizeL > 0 )
   {
     dataL[--sizeL]= R;
     R >>= BITS_PER_BYTE;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::store
//
// Purpose-
//       Store into Byte*
//
//----------------------------------------------------------------------------
void
   Binary::store(                   // Store into Byte*
     Byte*             dataR,       // Data byte array
     Size_t            sizeR) const // Array size
{
   Byte                fillL;
   Size_t              sizeL= getSize();

   while( sizeL > 0 && sizeR > 0 )
     dataR[--sizeR]= dataL[--sizeL];

   if( sizeR > 0 )
   {
     fillL= getFill();
     while( sizeR > 0 )
       dataR[--sizeR]= fillL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::toInt
//
// Purpose-
//       Convert into int64_t
//
//----------------------------------------------------------------------------
int64_t                             // Resultant
   Binary::toInt( void ) const      // Convert into int64_t
{
   int64_t             R;
   Size_t              sizeL= getSize();
   Size_t              sizeR= sizeof(R);

   R= 0;
   if( sizeR > sizeL )
   {
     R= getFill();
     sizeR= sizeL;
   }

   while( sizeR > 0 )
   {
     R <<= BITS_PER_BYTE;
     R  |= dataL[sizeL-sizeR];
     --sizeR;
   }

   return R;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator&=
//
// Purpose-
//       Replacement bitwise and operator
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator&=(
     const Binary&     R)
{
   const Byte*         dataR= R.getData();
   Byte                fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   while( sizeL > 0 && sizeR > 0 )
     dataL[--sizeL] &= dataR[--sizeR];

   while( sizeL > 0 )
     dataL[--sizeL] &= fillR;

   return *this;
}

Binary&
   Binary::operator&=(
     int64_t           R)
{
   Size_t              sizeL= getSize();

   while( sizeL > 0 )
   {
     dataL[--sizeL] &= R;
     R >>= BITS_PER_BYTE;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator|=
//
// Purpose-
//       Replacement bitwise or operator
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator|=(
     const Binary&     R)
{
   const Byte*         dataR= R.getData();
   Byte                fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   while( sizeL > 0 && sizeR > 0 )
     dataL[--sizeL] |= dataR[--sizeR];

   while( sizeL > 0 )
     dataL[--sizeL] |= fillR;

   return *this;
}

Binary&
   Binary::operator|=(
     int64_t           R)
{
   Size_t              sizeL= getSize();

   while( sizeL > 0 )
   {
     dataL[--sizeL] |= R;
     R >>= BITS_PER_BYTE;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator^=
//
// Purpose-
//       Replacement bitwise xor operator
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator^=(
     const Binary&     R)
{
   const Byte*         dataR= R.getData();
   Byte                fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   while( sizeL > 0 && sizeR > 0 )
     dataL[--sizeL] ^= dataR[--sizeR];

   while( sizeL > 0 )
     dataL[--sizeL] ^= fillR;

   return *this;
}

Binary&
   Binary::operator^=(
     int64_t           R)
{
   Size_t              sizeL= getSize();

   while( sizeL > 0 )
   {
     dataL[--sizeL] ^= R;
     R >>= BITS_PER_BYTE;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::operator<<=
//
// Purpose-
//       Replacement shift left operator
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator<<=(
     int               bits)
{
   int                 byteCount= unsigned(bits) / BITS_PER_BYTE;
   int                 bitsCount= unsigned(bits) % BITS_PER_BYTE;
   int                 bitiCount= BITS_PER_BYTE - bitsCount;
   Size_t              sizeL= getSize();
   Size_t              sizeR= byteCount;
   Size_t              i;

   if( sizeL > 0 && bits != 0 )
   {
     sizeR %= sizeL;                // sizeR= sizeR % sizeL

     if( bitsCount == 0 )
     {
       for(i=0; (sizeR + i) < sizeL; i++)
         dataL[i]= dataL[sizeR+i];
     }
     else
     {
       for(i=0; (sizeR + i) < (sizeL - 1); i++)
         dataL[i]= (dataL[sizeR+i]<<bitsCount) | (dataL[sizeR+i+1]>>bitiCount);

       if( (sizeR + i) == (sizeL - 1) )
       {
//       dataL[i++]= (dataL[sizeR+i]<<bitsCount); // (!Compiler bug!)
         dataL[i]= (dataL[sizeR+i]<<bitsCount);
         i++;
       }
     }

     while( i < sizeL )
       dataL[i++]= 0;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::operator>>=
//
// Purpose-
//       Replacement shift right operator
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator>>=(
     int               bits)
{
   int                 byteCount= unsigned(bits) / BITS_PER_BYTE;
   int                 bitsCount= unsigned(bits) % BITS_PER_BYTE;
   int                 bitiCount= BITS_PER_BYTE - bitsCount;
   Byte                fillL= getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= byteCount;
   Size_t              i;

   if( sizeL > 0 && bits != 0 )
   {
     sizeR %= sizeL;

     if( bitsCount == 0 )
     {
       for(i= sizeL; i > sizeR; i--)
         dataL[i-1]= dataL[i-sizeR-1];
     }
     else
     {
       for(i= sizeL; i > (sizeR + 1); i--)
         dataL[i-1]= (dataL[i-sizeR-2]<<bitiCount) | (dataL[i-sizeR-1]>>bitsCount);

       if( i == (sizeR + 1) )
       {
         dataL[i-1]= (fillL<<bitiCount) | (dataL[0]>>bitsCount);
         i--;
       }
     }

     while( i > 0 )
       dataL[--i]= fillL;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator+=
//
// Purpose-
//       Replacement addition operators
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator+=(
     const Binary&     R)
{
   int                 carry;
   const Byte*         dataR= R.getData();
   Byte                fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   carry= 0;
   while( sizeL > 0 && sizeR > 0 )
   {
     carry += dataL[--sizeL];
     carry += dataR[--sizeR];
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
   }

   while( sizeL > 0 )
   {
     carry += dataL[--sizeL];
     carry += fillR;
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
   }

   return *this;
}

Binary&
   Binary::operator+=(
     int64_t           R)
{
   int                 carry;
   Size_t              sizeL= getSize();

   carry= 0;
   while( sizeL > 0 )
   {
     carry += dataL[--sizeL];
     carry += (R & 0x00ff);
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
     R >>= BITS_PER_BYTE;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator-=
//
// Purpose-
//       Replacement subtraction operators
//
//----------------------------------------------------------------------------
Binary&
   Binary::negate( void )
{
   int                 carry;
   Size_t              sizeL= getSize();

   carry= 1;
   while( sizeL > 0 )
   {
     carry += Byte(255 - dataL[--sizeL]);
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
   }

   return *this;
}

Binary&
   Binary::operator-=(
     const Binary&     R)
{
   int                 carry;
   const Byte*         dataR= R.getData();
   Byte                fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   carry= 1;
   while( sizeL > 0 && sizeR > 0 )
   {
     carry += dataL[--sizeL];
     carry += Byte(255 - dataR[--sizeR]);
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
   }

   fillR= 0xFF - fillR;
   while( sizeL > 0 )
   {
     carry += dataL[--sizeL];
     carry += fillR;
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
   }

   return *this;
}

Binary&
   Binary::operator-=(
     int64_t           R)
{
   int                 carry;
   Size_t              sizeL= getSize();

   carry= 1;
   while( sizeL > 0 )
   {
     carry += dataL[--sizeL];
     carry += Byte(~R);
     dataL[sizeL]= carry;
     carry >>= BITS_PER_BYTE;
     R >>= BITS_PER_BYTE;
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator*=
//
// Purpose-
//       Replacement multiplication operators
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator*=(
     const Binary&     R)
{
   VarBinary           resultant;
   VarBinary           multiplier(R);

   Byte                byte;
   Byte                mask;
   Size_t              sizeL= getSize();

   resultant.initSize(getSize());
   resultant.load(0);
   while( sizeL > 0 )
   {
     byte= dataL[--sizeL];
     mask= 0x01;
     while( mask != 0 )
     {
       if( (byte & mask) != 0 )
         resultant += multiplier;

       multiplier <<= 1;
       mask <<= 1;
     }
   }

   resultant.store(dataL, getSize());
   return *this;
}

Binary&
   Binary::operator*=(
     int64_t           R)
{
   VarBinary           resultant;

   Byte                byte;
   Byte                mask;
   Size_t              sizeL= getSize();

   resultant.initSize(getSize());
   resultant.load(0);
   while( sizeL > 0 && R != 0 )
   {
     byte= dataL[--sizeL];
     mask= 0x01;
     while( mask != 0 )
     {
       if( (byte & mask) != 0 )
         resultant += R;

       R <<= 1;
       mask <<= 1;
     }
   }

   resultant.store(dataL, getSize());
   return *this;
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator/=
//
// Purpose-
//       Replacement divide operators
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator/=(
     const Binary&     R)
{
   #ifdef HCDM
     debugf("Binary(%p)::operator/=(Binary&(%p))\n", this, &R);
   #endif

   VarBinary           remainder(*this);
   VarBinary           divisor;

   Byte                byte;
   int                 fillL= getFill();
   int                 fillR= R.getFill();
   Byte                mask;
   Size_t              sizeL= getSize();

   Size_t              i;

   if( R == 0 )
     throw "DivideByZero";

   divisor.initSize(getSize() + R.getSize());
   divisor.operator=(R);
   divisor <<= (getSize() * BITS_PER_BYTE);
   if( fillR < 0 )
     divisor.negate();
   if( fillL < 0 )
     remainder.negate();
   divisor.setSigned(FALSE);
   remainder.setSigned(FALSE);

   load(0);
   for(i= 0; i < sizeL; i++)
   {
     byte= 0;
     mask= 0x80;
     while( mask != 0 )
     {
       divisor >>= 1;
       if( remainder >= divisor )
       {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     dataL[i]= byte;
   }
   if( fillL != fillR )
     negate();

   return *this;
}

Binary&
   Binary::operator/=(
     int64_t           R)
{
   SignedBinary<8>     operand(R);

   return this->operator/=(operand);
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::operator%=
//
// Purpose-
//       Replacement modulus operators
//
//----------------------------------------------------------------------------
Binary&
   Binary::operator%=(
     const Binary&     R)
{
   #ifdef HCDM
     debugf("Binary(%p)::operator%%=(Binary&(%p))\n", this, &R);
   #endif

   VarBinary           remainder(*this);
   VarBinary           divisor;

   Byte                byte;
   int                 fillL= getFill();
   int                 fillR= R.getFill();
   Byte                mask;
   Size_t              sizeL= getSize();

   Size_t              i;

   if( R == 0 )
     throw "DivideByZero";

   divisor.initSize(getSize() + R.getSize());
   divisor.operator=(R);
   divisor <<= (getSize() * BITS_PER_BYTE);
   if( fillR < 0 )
     divisor.negate();
   if( fillL < 0 )
     remainder.negate();
   divisor.setSigned(FALSE);
   remainder.setSigned(FALSE);

   load(0);
   for(i= 0; i < sizeL; i++)
   {
     byte= 0;
     mask= 0x80;
     while( mask != 0 )
     {
       divisor >>= 1;
       if( remainder >= divisor )
       {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     dataL[i]= byte;
   }
   if( fillL < 0 )
     remainder.negate();

   remainder.store(dataL, getSize());
   return *this;
}

Binary&
   Binary::operator%=(
     int64_t           R)
{
   SignedBinary<8>     operand(R);

   return this->operator%=(operand);
}

//----------------------------------------------------------------------------
//
// Methods-
//       VarBinary::div(
//
// Purpose-
//       Replacement divide, return modulus.
//
//----------------------------------------------------------------------------
int                                 // Remainder
   VarBinary::div(                  // Replacement divide, return remainder
     int               R)           // Divisor
{
   #ifdef HCDM
     debugf("Binary(%p)::div(%d)\n", this, R);
   #endif

   VarBinary           remainder(*this);
   VarBinary           divisor;

   Byte                byte;
   int                 fillL= getFill();
   int                 fillR= R >> 31;
   Byte                mask;
   Size_t              sizeL= getSize();

   Size_t              i;

   if( R == 0 )
     throw "DivideByZero";

   divisor.initSize(getSize() + sizeof(R));
   divisor.operator=(R);
   divisor <<= (getSize() * BITS_PER_BYTE);
   if( fillR < 0 )
     divisor.negate();
   if( fillL < 0 )
     remainder.negate();
   divisor.setSigned(FALSE);
   remainder.setSigned(FALSE);

   load(0);
   for(i= 0; i < sizeL; i++)
   {
     byte= 0;
     mask= 0x80;
     while( mask != 0 )
     {
       divisor >>= 1;
       if( remainder >= divisor )
       {
         byte |= mask;
         remainder -= divisor;
       }

       mask >>= 1;
     }
     dataL[i]= byte;
   }
   if( fillL != fillR )
     negate();

   if( fillL < 0 )
     remainder.negate();

   return (int)remainder.toInt();
}

//----------------------------------------------------------------------------
//
// Methods-
//       Binary::compare
//
// Purpose-
//       Comparison.
//       Resultant: <0 iff *this < comprahend
//       Resultant: =0 iff *this == comprahend
//       Resultant: >0 iff *this > comprahend
//
//----------------------------------------------------------------------------
int                                 // Resultant (<0, 0, >0)
   Binary::compare(                 // Comparison
     const Binary&     R) const     // Comprahend
{
   int                 resultant= 0;

   const Byte*         dataR= R.getData();
   Byte                fillB;
   int                 fillL= getFill();
   int                 fillR= R.getFill();
   Size_t              sizeL= getSize();
   Size_t              sizeR= R.getSize();

   Size_t              xL= 0;
   Size_t              xR= 0;

   // Compare
   fillB= fillR;
   while( resultant == 0 && sizeL > (sizeR + xL) )
     resultant= dataL[xL++] - fillB;

   fillB= fillL;
   while( resultant == 0 && sizeR > (sizeL + xR) )
     resultant= fillB - dataR[xR++];

   while( resultant == 0 && xL < sizeL )
     resultant= dataL[xL++] - dataR[xR++];

   // If either is unsigned, the comparison is unsigned
   if( getSigned() && R.getSigned() )
   {
     if( fillL != fillR )
       resultant= -resultant;
   }

   return resultant;
}

int                                 // Resultant (<0, 0, >0)
   Binary::compare(                 // Comparison
     int64_t           R) const     // Comprahend
{
   int                 resultant= 0;

   Byte                fillB;
   int                 fillL= getFill();
   int                 fillR= R >> 63;
   Size_t              sizeL= getSize();
   Byte                tempR;

   Size_t              xL= 0;
   Size_t              xR= 0;

   // Compare
   fillB= fillR;
   while( resultant == 0 && sizeL > (sizeof(R) + xL) )
     resultant= dataL[xL++] - fillB;

   fillB= fillL;
   while( resultant == 0 && sizeof(R) > (sizeL + xR) )
   {
     xR++;
     tempR= R>>((sizeof(R)-xR)*BITS_PER_BYTE);
     resultant= fillB - tempR;
   }

   while( resultant == 0 && xL < sizeL )
   {
     xR++;
     tempR= R>>((sizeof(R)-xR)*BITS_PER_BYTE);
     resultant= dataL[xL++] - tempR;
   }

   // If either is unsigned, the comparison is unsigned
   // Since we aren't allowed to know whether the integer is signed,
   // we just assume that it has the signed attribute of our Binary.
   if( getSigned() )
   {
     if( fillL != fillR )
       resultant= -resultant;
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::getSigned
//
// Purpose-
//       Accessor function
//
//----------------------------------------------------------------------------
int                                 // The signed attribute
   Binary::getSigned( void ) const  // Get signed attribute
{
   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::getSize
//
// Purpose-
//       Accessor function
//
//----------------------------------------------------------------------------
Binary::Size_t                      // The size attribute
   Binary::getSize( void ) const    // Get size attribute
{
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::inp
//
// Purpose-
//       Convert from string
//
//----------------------------------------------------------------------------
void
   Binary::inp(                     // Convert to string
     const char*       string)      // The source string
{
   unsigned char       C;           // Working character
   int                 fillL;       // TRUE if negate required
   Size_t              sizeL= getSize();

   fillL= 0;
   if( *string == '-' )
   {
     fillL= (-1);
     string++;
   }

   load(0);
   C= *string;
   while( C != '\0' )
   {
     if( C < '0' || C > '9' )
       throw "Binary::inp.InvalidFormatException";

     operator*=(10);
     operator+=(C - '0');

     if( sizeL == 0 || dataL[0] >= 0x80 )
       throw "Binary::inp.OverflowException";

     string++;
     C= *string;
   }

   if( fillL < 0 )
     negate();
}

void
   Binary::inp(                     // Convert to string
     std::string       string)      // The source string
{
   inp(string.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::out
//
// Purpose-
//       Convert to std::string
//
//----------------------------------------------------------------------------
std::string                         // The resultant string
   Binary::out(                     // Convert to string
     const char*       fmt) const   // The format string
{
   std::string         result;      // Resultant

   unsigned char       C;           // Working character
   int                 fillL= getFill(); // The actual sign
   int                 flagFP;      // Flag: Field Precision specified
   int                 flagLJ;      // Flag: Left justification
   int                 flagPS;      // Flag: Include positive sign
   int                 flagBL;      // Flag: Use blank in liu of positive sign
   int                 flagNS;      // Flag: 0, 0x, or 0X prefix
   int                 flagZP;      // Flag: Use zero padding
   int                 fw;          // Field width
   int                 fp;          // Field precision
   int                 isLT;        // Value is less than zero
   int                 isNZ;        // Found non-zero value
   Size_t              M;           // Working size
   const char*         table;       // Conversion table
   int                 x;           // Format index

   if( fmt != NULL )
   {
     for(x= 0; fmt[x] != '\0'; x++)
     {
       if( fmt[x] != '%' )
       {
         result += fmt[x];
         continue;
       }

       if( fmt[++x] == '%' )
       {
         result += '%';
         continue;
       }

       // Set flags and indicators
       isLT= (fillL != 0);
       isNZ= FALSE;

       flagFP= FALSE;
       flagLJ= FALSE;
       flagPS= FALSE;
       flagBL= FALSE;
       flagNS= FALSE;
       flagZP= FALSE;
       for(;;x++)
       {
         C= fmt[x];
         if( C == '-' )
           flagLJ= TRUE;
         else if( C == '+' )
           flagPS= TRUE;
         else if( C == ' ' )
           flagBL= TRUE;
         else if( C == '#' )
           flagNS= TRUE;
         else if( C == '0' )
           flagZP= TRUE;
         else
           break;
       }

       // Set field width
       fw= 0;
       while( fmt[x] >= '0' && fmt[x] <= '9' )
       {
         fw *= 10;
         fw += (fmt[x++] - '0');
       }

       // Set field precision
       fp= 0;
       if( fmt[x] == '.' )
       {
         x++;
         flagFP= TRUE;
         while( fmt[x] >= '0' && fmt[x] <= '9' )
         {
           fp *= 10;
           fp += (fmt[x++] - '0');
         }
       }

       // Working strings and values
       VarBinary     v(*this);
       std::string   prefix;
       std::string   invert;

       // Process format
       switch( fmt[x] )             // Handle format
       {
         case 'o':
           isLT= FALSE;
           v.setSigned(FALSE);

           M= v.getSize() - 1;
           while( v != 0)
           {
             isNZ= TRUE;
             invert += toHex[(v.getData()[M] & 0x0007)];
             v >>= 3;
           }

           if( flagNS && (isNZ || flagPS) )
             prefix += '0';
           break;

         case 'u':
           isLT= FALSE;
           v.setSigned(FALSE);
           [[ fallthrough ]]
           ;;

         case 'd':
         case 'i':
           while( v != 0 )
           {
             isNZ= TRUE;
             int i= v.div(10);
             invert += toDec[i+9];
           }

           if( fmt[x] != 'u' )
           {
             if( flagPS && isLT == FALSE )
               prefix += '+';
             else if( isNZ || flagFP == FALSE )
             {
               if( isLT )
                 prefix += '-';
               else if( flagBL )
                 prefix += ' ';
             }
           }
           break;

         case 'x':
         case 'X':
           isLT= FALSE;
           v.setSigned(FALSE);

           table= toHex;
           if( fmt[x] == 'X' )
             table= toHEX;

           M= v.getSize() - 1;
           while( v != 0)
           {
             isNZ= TRUE;
             invert += table[(v.getData()[M] & 0x000F)];
             v >>= 4;
           }

           if( flagNS && isNZ )
             prefix += &table[16];

           break;

         default:
           throw "Binary::out.BadFormatException";
       }

       // Handle precision
       if( flagFP )
       {
         fp -= invert.size();
         while( fp > 0 )
         {
           invert += '0';
           fp--;
         }
       }
       else if( !isNZ )
         invert= '0';

       // Revert the invert, handling field width
       fw -= (prefix.size() + invert.size());
       if( fw > 0 && flagZP )
       {
         while( fw > 0 )
         {
           invert += '0';
           fw--;
         }
       }

       if( fw > 0 && flagLJ == FALSE )
       {
         while( fw > 0 )
         {
           result += ' ';
           fw--;
         }
       }

       result += prefix;
       M= invert.size();
       for(size_t i= 0; i<M; i++)
         result += invert[M-i-1];

       while( fw > 0 )
       {
         result += ' ';
         fw--;
       }
     }
   }

   return result;
}

std::string                         // The resultant string
   Binary::out( void ) const        // Convert to string
{
   return out("%d");
}

//----------------------------------------------------------------------------
//
// Method-
//       Binary::sigSize
//
// Purpose-
//       Get the number of significant bytes, e.g. the minimum size of a
//       signed VarBinary that could contain the same value.
//
//----------------------------------------------------------------------------
Binary::Size_t                      // The significant size
   Binary::sigSize( void ) const    // Get significant size
{
   Size_t              result= getSize();

   Size_t              i;

   if( result > 0 )                 // If some size
   {
     if( getSigned() && (dataL[0] >= 0x80) ) // Negative value
     {
       for(i= 0; i < result; i++)
       {
         if( dataL[i] != 0xff )
         {
           if( dataL[i] < 0x80 )
             result++;
           break;
         }
       }
       if( i == result )
         result++;

       result -= i;
     }
     else                           // Unsigned or positive
     {
       for(i= 0; i < result; i++)
       {
         if( dataL[i] != 0x00 )
         {
           if( dataL[i] >= 0x80 )
             result++;
           break;
         }
       }

       result -= i;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::~VarBinary
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   VarBinary::~VarBinary( void )    // Destructor
{
   #ifdef HCDM
     debugf("VarBinary(%p)::~VarBinary()\n", this);
   #endif

   initSize(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::VarBinary
//
// Purpose-
//       Constructors.
//
//----------------------------------------------------------------------------
   VarBinary::VarBinary( void )     // Default constructor
:  Binary()
,  signL(TRUE)
,  sizeL(0)
{
   #ifdef HCDM
     debugf("VarBinary(%p)::VarBinary(void)\n", this);
   #endif
}

   VarBinary::VarBinary(            // Copy constructor
     const VarBinary&  R)           // Source VarBinary
:  Binary()
,  signL(R.getSigned())
,  sizeL(0)
{
   #ifdef HCDM
     debugf("VarBinary(%p)::VarBinary(Binary& %p)\n", this, &R);
   #endif

   initSize(R);
   R.store(dataL, sizeL);
}

   VarBinary::VarBinary(            // Copy constructor
     const Binary&     R)           // Source Binary
:  Binary()
,  signL(R.getSigned())
,  sizeL(0)
{
   #ifdef HCDM
     debugf("VarBinary(%p)::VarBinary(Binary& %p)\n", this, &R);
   #endif

   initSize(R);
   R.store(dataL, sizeL);
}

   VarBinary::VarBinary(            // Value constructor
     int64_t           R)           // Value
:  Binary()
,  signL(TRUE)
,  sizeL(0)
{
   #ifdef HCDM
     debugf("VarBinary(%p)::VarBinary(int64_t %" PRId64 ")\n", this, R);
   #endif

   initSize(sizeof(int64_t));
   load(R);
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::getSigned
//
// Purpose-
//       Return the signed attribute
//
//----------------------------------------------------------------------------
int                                 // The signed attribute
   VarBinary::getSigned( void ) const // Get signed attribute
{
   return (signL != 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::setSigned
//
// Purpose-
//       Update the signed attribute
//
//----------------------------------------------------------------------------
void
   VarBinary::setSigned(            // Set signed attribute
     int               signL)       // New signed attribute
{
   this->signL= signL;
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::getSize
//
// Purpose-
//       Return the size of the VarBinary
//
//----------------------------------------------------------------------------
Binary::Size_t                      // The size
   VarBinary::getSize( void ) const // Get size
{
   return sizeL;
}

//----------------------------------------------------------------------------
//
// Methods-
//       VarBinary::initSize
//
// Purpose-
//       Initialize the size of the Binary, destroying its value.
//
//----------------------------------------------------------------------------
void
   VarBinary::initSize(             // Initialize size
     Size_t            sizeL)       // New size
{
   #ifdef HCDM
     debugf("<<<VarBinary(%p)::initSize(%ld) %p\n", this, sizeL, this->dataL);
   #endif

   if( this->sizeL > 0 )            // If data to delete
   {
     delete[] this->dataL;
     this->dataL= NULL;
     this->sizeL= 0;
   }

   if( sizeL > 0 )                  // If new size is positive
   {
     dataL= new Byte[sizeL];
     this->sizeL= sizeL;
   }

   #ifdef HCDM
     debugf(">>>VarBinary(%p)::initSize(%ld) %p\n", this, this->sizeL, this->dataL);
   #endif
}

void                                // size= max(U.getSize(), sizeof(int64_t))
   VarBinary::initSize(             // Initialize size (Content undefined)
     const Binary&     U)           // Implicit new size
{
   Size_t              sizeL= U.getSize();

   if( sizeL < sizeof(int64_t) )
     sizeL= sizeof(int64_t);

   initSize(sizeL);
}

void                                // size= max(L.getSize(), R.getSize())
   VarBinary::initSize(             // Initialize size (Content undefined)
     const Binary&     L,           // Implicit new size
     const Binary&     R)           // Implicit new size
{
   Size_t              sizeL= L.getSize();

   if( sizeL < R.getSize() )
     sizeL= R.getSize();

   initSize(sizeL);
}

//----------------------------------------------------------------------------
//
// Method-
//       VarBinary::setSize
//
// Purpose-
//       Update the size of the Binary, but not its value.
//
//----------------------------------------------------------------------------
void
   VarBinary::setSize(              // Set size
     Size_t            sizeN)       // New size
{
   #ifdef HCDM
     debugf("<<<VarBinary(%p)::setSize(%ld) %p\n", this, sizeN, this->dataL);
   #endif

   Byte*               dataL;       // The new data buffer
   const Byte*         dataR;       // The old data buffer
   Byte                fillL;       // Fill byte
   Size_t              sizeL;       // New length (working value)
   Size_t              sizeR;       // Old length (working value)

   if( sizeN == 0 )                 // If new size is zero (can't be less)
   {
     if( this->sizeL > 0 )          // If data to delete
     {
       delete[] this->dataL;
       this->dataL= NULL;
       this->sizeL= 0;
     }
   }
   else                             // If new size is positive
   {
     dataL= new Byte[sizeN];
     sizeL= sizeN;
     dataR= this->dataL;
     sizeR= this->sizeL;

     while( sizeL > 0 && sizeR > 0 )
       dataL[--sizeL]= dataR[--sizeR];

     fillL= getFill();
     while( sizeL > 0 )
       dataL[--sizeL]= fillL;

     delete[] this->dataL;
     this->dataL= dataL;
     this->sizeL= sizeN;
   }

   #ifdef HCDM
     debugf(">>>VarBinary(%p)::setSize(%ld) %p\n", this, this->sizeL, this->dataL);
   #endif
}

#if 0
//----------------------------------------------------------------------------
//
// Methods-
//       Binary::Global operators
//
// Purpose-
//       Prototypes for global operators implemented in Binary.i
//
//----------------------------------------------------------------------------
VarBinary operator~(const Binary& U);   // Unary ~
VarBinary operator+(const Binary& U);   // Unary +
VarBinary operator-(const Binary& U);   // Unary -

VarBinary operator++(Binary& U);        // Prefix operator
VarBinary operator++(Binary& U, int R); // Postfix operator

VarBinary operator--(Binary& U);        // Prefix operator
VarBinary operator--(Binary& U, int R); // Postfix operator

VarBinary operator<<(const Binary& L, int R); // Shift left
VarBinary operator>>(const Binary& L, int R); // Shift right

VarBinary operator&(const Binary& L, const Binary& R);
VarBinary operator&(const Binary& L, int64_t       R);
VarBinary operator&(int64_t       L, const Binary& R);

VarBinary operator|(const Binary& L, const Binary& R);
VarBinary operator|(const Binary& L, int64_t       R);
VarBinary operator|(int64_t       L, const Binary& R);

VarBinary operator^(const Binary& L, const Binary& R);
VarBinary operator^(const Binary& L, int64_t       R);
VarBinary operator^(int64_t       L, const Binary& R);

VarBinary operator+(const Binary& L, const Binary& R);
VarBinary operator+(const Binary& L, int64_t       R);
VarBinary operator+(int64_t       L, const Binary& R);

VarBinary operator-(const Binary& L, const Binary& R);
VarBinary operator-(const Binary& L, int64_t       R);
VarBinary operator-(int64_t       L, const Binary& R);

VarBinary operator*(const Binary& L, const Binary& R);
VarBinary operator*(const Binary& L, int64_t       R);
VarBinary operator*(int64_t       L, const Binary& R);

VarBinary operator/(const Binary& L, const Binary& R);
VarBinary operator/(const Binary& L, int64_t       R);
VarBinary operator/(int64_t       L, const Binary& R);

VarBinary operator%(const Binary& L, const Binary& R);
VarBinary operator%(const Binary& L, int64_t       R);
VarBinary operator%(int64_t       L, const Binary& R);
#endif

