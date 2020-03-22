//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Synapse.cpp
//
// Purpose-
//       Synapse object methods.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Synapse.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static unsigned int    bitCount[256]= // Bit value to bit count conversion
{  0                                //   0 00000000
,  1                                //   1 00000001
,  1                                //   2 00000010
,  2                                //   3 00000011
,  1                                //   4 00000100
,  2                                //   5 00000101
,  2                                //   6 00000110
,  3                                //   7 00000111
,  1                                //   8 00001000
,  2                                //   9 00001001
,  2                                //  10 00001010
,  3                                //  11 00001011
,  2                                //  12 00001100
,  3                                //  13 00001101
,  3                                //  14 00001110
,  4                                //  15 00001111
,  1                                //  16 00010000
,  2                                //  17 00010001
,  2                                //  18 00010010
,  3                                //  19 00010011
,  2                                //  20 00010100
,  3                                //  21 00010101
,  3                                //  22 00010110
,  4                                //  23 00010111
,  2                                //  24 00011000
,  3                                //  25 00011001
,  3                                //  26 00011010
,  4                                //  27 00011011
,  3                                //  28 00011100
,  4                                //  29 00011101
,  4                                //  30 00011110
,  5                                //  31 00011111
,  1                                //  32 00100000
,  2                                //  33 00100001
,  2                                //  34 00100010
,  3                                //  35 00100011
,  2                                //  36 00100100
,  3                                //  37 00100101
,  3                                //  38 00100110
,  4                                //  39 00100111
,  2                                //  40 00101000
,  3                                //  41 00101001
,  3                                //  42 00101010
,  4                                //  43 00101011
,  3                                //  44 00101100
,  4                                //  45 00101101
,  4                                //  46 00101110
,  5                                //  47 00101111
,  2                                //  48 00110000
,  3                                //  49 00110001
,  3                                //  50 00110010
,  4                                //  51 00110011
,  3                                //  52 00110100
,  4                                //  53 00110101
,  4                                //  54 00110110
,  5                                //  55 00110111
,  3                                //  56 00111000
,  4                                //  57 00111001
,  4                                //  58 00111010
,  5                                //  59 00111011
,  4                                //  60 00111100
,  5                                //  61 00111101
,  5                                //  62 00111110
,  6                                //  63 00111111
,  1                                //  64 01000000
,  2                                //  65 01000001
,  2                                //  66 01000010
,  3                                //  67 01000011
,  2                                //  68 01000100
,  3                                //  69 01000101
,  3                                //  70 01000110
,  4                                //  71 01000111
,  2                                //  72 01001000
,  3                                //  73 01001001
,  3                                //  74 01001010
,  4                                //  75 01001011
,  3                                //  76 01001100
,  4                                //  77 01001101
,  4                                //  78 01001110
,  5                                //  79 01001111
,  2                                //  80 01010000
,  3                                //  81 01010001
,  3                                //  82 01010010
,  4                                //  83 01010011
,  3                                //  84 01010100
,  4                                //  85 01010101
,  4                                //  86 01010110
,  5                                //  87 01010111
,  3                                //  88 01011000
,  4                                //  89 01011001
,  4                                //  90 01011010
,  5                                //  91 01011011
,  4                                //  92 01011100
,  5                                //  93 01011101
,  5                                //  94 01011110
,  6                                //  95 01011111
,  2                                //  96 01100000
,  3                                //  97 01100001
,  3                                //  98 01100010
,  4                                //  99 01100011
,  3                                // 100 01100100
,  4                                // 101 01100101
,  4                                // 102 01100110
,  5                                // 103 01100111
,  3                                // 104 01101000
,  4                                // 105 01101001
,  4                                // 106 01101010
,  5                                // 107 01101011
,  4                                // 108 01101100
,  5                                // 109 01101101
,  5                                // 110 01101110
,  6                                // 111 01101111
,  3                                // 112 01110000
,  4                                // 113 01110001
,  4                                // 114 01110010
,  5                                // 115 01110011
,  4                                // 116 01110100
,  5                                // 117 01110101
,  5                                // 118 01110110
,  6                                // 119 01110111
,  4                                // 120 01111000
,  5                                // 121 01111001
,  5                                // 122 01111010
,  6                                // 123 01111011
,  5                                // 124 01111100
,  6                                // 125 01111101
,  6                                // 126 01111110
,  7                                // 127 01111111
,  1                                // 128 10000000
,  2                                // 129 10000001
,  2                                // 130 10000010
,  3                                // 131 10000011
,  2                                // 132 10000100
,  3                                // 133 10000101
,  3                                // 134 10000110
,  4                                // 135 10000111
,  2                                // 136 10001000
,  3                                // 137 10001001
,  3                                // 138 10001010
,  4                                // 139 10001011
,  3                                // 140 10001100
,  4                                // 141 10001101
,  4                                // 142 10001110
,  5                                // 143 10001111
,  2                                // 144 10010000
,  3                                // 145 10010001
,  3                                // 146 10010010
,  4                                // 147 10010011
,  3                                // 148 10010100
,  4                                // 149 10010101
,  4                                // 150 10010110
,  5                                // 151 10010111
,  3                                // 152 10011000
,  4                                // 153 10011001
,  4                                // 154 10011010
,  5                                // 155 10011011
,  4                                // 156 10011100
,  5                                // 157 10011101
,  5                                // 158 10011110
,  6                                // 159 10011111
,  2                                // 160 10100000
,  3                                // 161 10100001
,  3                                // 162 10100010
,  4                                // 163 10100011
,  3                                // 164 10100100
,  4                                // 165 10100101
,  4                                // 166 10100110
,  5                                // 167 10100111
,  3                                // 168 10101000
,  4                                // 169 10101001
,  4                                // 170 10101010
,  5                                // 171 10101011
,  4                                // 172 10101100
,  5                                // 173 10101101
,  5                                // 174 10101110
,  6                                // 175 10101111
,  3                                // 176 10110000
,  4                                // 177 10110001
,  4                                // 178 10110010
,  5                                // 179 10110011
,  4                                // 180 10110100
,  5                                // 181 10110101
,  5                                // 182 10110110
,  6                                // 183 10110111
,  4                                // 184 10111000
,  5                                // 185 10111001
,  5                                // 186 10111010
,  6                                // 187 10111011
,  5                                // 188 10111100
,  6                                // 189 10111101
,  6                                // 190 10111110
,  7                                // 191 10111111
,  2                                // 192 11000000
,  3                                // 193 11000001
,  3                                // 194 11000010
,  4                                // 195 11000011
,  3                                // 196 11000100
,  4                                // 197 11000101
,  4                                // 198 11000110
,  5                                // 199 11000111
,  3                                // 200 11001000
,  4                                // 201 11001001
,  4                                // 202 11001010
,  5                                // 203 11001011
,  4                                // 204 11001100
,  5                                // 205 11001101
,  5                                // 206 11001110
,  6                                // 207 11001111
,  3                                // 208 11010000
,  4                                // 209 11010001
,  4                                // 210 11010010
,  5                                // 211 11010011
,  4                                // 212 11010100
,  5                                // 213 11010101
,  5                                // 214 11010110
,  6                                // 215 11010111
,  4                                // 216 11011000
,  5                                // 217 11011001
,  5                                // 218 11011010
,  6                                // 219 11011011
,  5                                // 220 11011100
,  6                                // 221 11011101
,  6                                // 222 11011110
,  7                                // 223 11011111
,  3                                // 224 11100000
,  4                                // 225 11100001
,  4                                // 226 11100010
,  5                                // 227 11100011
,  4                                // 228 11100100
,  5                                // 229 11100101
,  5                                // 230 11100110
,  6                                // 231 11100111
,  4                                // 232 11101000
,  5                                // 233 11101001
,  5                                // 234 11101010
,  6                                // 235 11101011
,  5                                // 236 11101100
,  6                                // 237 11101101
,  6                                // 238 11101110
,  7                                // 239 11101111
,  4                                // 240 11110000
,  5                                // 241 11110001
,  5                                // 242 11110010
,  6                                // 243 11110011
,  5                                // 244 11110100
,  6                                // 245 11110101
,  6                                // 246 11110110
,  7                                // 247 11110111
,  5                                // 248 11111000
,  6                                // 249 11111001
,  6                                // 250 11111010
,  7                                // 251 11111011
,  6                                // 252 11111100
,  7                                // 253 11111101
,  7                                // 254 11111110
,  8                                // 255 11111111
};

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::~Synapse
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Synapse::~Synapse( void )        // Destructor
{
   if( inps != NULL )
   {
     free(inps);
     inps= NULL;
   }

   if( sets != NULL )
   {
     free(sets);
     sets= NULL;
   }

   if( outs != NULL )
   {
     free(outs);
     outs= NULL;
   }

   if( inwv != NULL )
   {
     free(inwv);
     inwv= NULL;
   }

   if( rems != NULL )
   {
     free(rems);
     rems= NULL;
   }

   if( leak != NULL )
   {
     free(leak);
     leak= NULL;
   }

   if( trig != NULL )
   {
     free(trig);
     trig= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::Synapse
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Synapse::Synapse(                // Constructor
     unsigned int      iCount,      // Number of input Axions
     unsigned int      oCount)      // Number of output Neurons
:  Object()
,  iCount(iCount), oCount(oCount)
,  inps(NULL), sets(NULL), outs(NULL)
,  inwv(NULL), rems(NULL), leak(NULL), trig(NULL)
{
   if( iCount == 0 || oCount == 0
       || (iCount & 7) != 0 || (oCount & 7) != 0 )
     throw "Synapse: Parameter error";

   inps= (unsigned char*)malloc(iCount>>3);
   sets= (unsigned char*)malloc((iCount>>3) * oCount);
   outs= (unsigned char*)malloc(oCount>>3);

   inwv= (unsigned char*)malloc(iCount>>3);
   rems= (unsigned char*)malloc(oCount);
   leak= (unsigned char*)malloc(oCount);
   trig= (unsigned char*)malloc(oCount);

   if( inps == NULL || inwv == NULL || sets == NULL || outs == NULL
       || rems == NULL || leak == NULL || trig == NULL )
     throw "Synapse: Storage shortage";

   memset(inps, 0, iCount >> 3);    // Default: NO inputs
   memset(inwv, 0, iCount >> 3);    // Default: Weight= 1
   memset(sets, 0, (iCount>>3) * oCount); // Default: NO sets
   memset(rems, 0, oCount);         // Default: NO remainders
   memset(leak, 0, oCount);         // Default: NO leakage
   memset(trig, 0, oCount);         // Default: Trigger= 1
   memset(outs, 0, oCount >> 3);    // Default: NO outputs

   #if( 0 )
     int bytes= sizeof(*this);      // Overhead
     bytes += (iCount >> 3);        // inps
     bytes += (iCount >> 3);        // inwv
     bytes += ((iCount>>3) * oCount); // sets
     bytes += (oCount);             // rems
     bytes += (oCount);             // leak
     bytes += (oCount);             // trig
     bytes += (oCount >> 3);        // outs
     printf("%8d sizeof(Synapse)\n", bytes);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::Methods
//
// Purpose-
//       Method placeholder.
//
//----------------------------------------------------------------------------
int                                 // The total weight of set bits
   Synapse::getBits(                // Get total weight of set bits
     unsigned int      index) const // For this Neuron index
{
   assert( index < oCount );        // Verify parameters

   const int M= (iCount >> 3);      // Number of Axion bytes
   const unsigned char* const sets= this->sets + M*index; // Set[row]

   int result= 0;                   // Number of inputs
   for(int i= 0; i<M; i++)
   {
     int weight= ((signed char*)inwv)[i];
     if( weight >= 0 )
       weight++;

     result += (bitCount[sets[i]] * weight);
   }

   return result;
}

unsigned int                        // The associated leak value
   Synapse::getLeak(                // Get associated leak value
     unsigned int      index) const // For this Neuron index
{
   assert( index < oCount );        // Verify parameters

   return leak[index];
}

void
   Synapse::setLeak(                // Set leak value
     unsigned int      index,       // For this Neuron index
     unsigned int      value)       // To this value (range 0..255)
{
   assert( index < oCount );        // Verify parameters
   assert( value <= 255 );

   leak[index]= value;
}

unsigned int                        // The associated trigger value
   Synapse::getTrig(                // Get associated trigger value
     unsigned int      index) const // For this Neuron index
{
   assert( index < oCount );        // Verify parameters

   return trig[index] + 1;
}

void
   Synapse::setTrig(                // Set trigger value
     unsigned int      index,       // For this Neuron index
     unsigned int      value)       // To this value (range 1..256)
{
   assert( index < oCount );        // Verify parameters
   assert( value > 0 && value <= 256 );

   trig[index]= value - 1;
}

int                                 // The associated weight value
   Synapse::getWeight(              // Get associated weight value
     unsigned int      index) const // For this Axion (bundle) index
{
   assert( index < iCount );        // Verify parameters

   int weight= inwv[index>>3];
   if( weight >= 0 )
     weight++;

   return weight;
}

void
   Synapse::setWeight(              // Set weight value
     unsigned int      index,       // For this Axion (bundle) index
     int               weight)      // To this value (range 0..255)
{
   assert( index < iCount );        // Verify parameters
   assert( weight != 0 && weight <= 256 && weight >= (-256) );

   if( weight > 0 )
     weight--;

   inwv[index>>3]= weight;
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::disable
//
// Purpose-
//       Clear Axion/Neuron crossbar switch entry
//
//----------------------------------------------------------------------------
void
   Synapse::disable(                // Disable (set to 0)
     unsigned int      inp,         // Input index
     unsigned int      out)         // Output index
{
   assert( inp < iCount && out < oCount ); // Verify parameters

   int iCount= (this->iCount >> 3); // Number of bytes for each set row
   int iByteIndex= (inp >> 3);      // Input byte index
   int iBitsIndex= (inp & 7);       // Input bit index
   int setsIndex= (out * iCount) + iByteIndex; // Address proper byte

   int mask= 0x00000080 >> iBitsIndex; // Inverted mask
   mask= (mask ^ 0x000000ff);       // Reverted mask
   sets[setsIndex] &= mask;         // Clear the bit
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::enable
//
// Purpose-
//       Set Axion/Neuron crossbar switch entry
//
//----------------------------------------------------------------------------
void
   Synapse::enable(                 // Enable (set to 1)
     unsigned int      inp,         // Input index
     unsigned int      out)         // Output index
{
   assert( inp < iCount && out < oCount ); // Verify parameters

   int iCount= (this->iCount >> 3); // Number of bytes for each set row
   int iByteIndex= (inp >> 3);      // Input byte index
   int iBitsIndex= (inp & 7);       // Input bit index
   int setsIndex= (out * iCount) + iByteIndex; // Address proper byte

   int mask= 0x00000080 >> iBitsIndex; // Reverted mask
   sets[setsIndex] |= mask;         // Set the bit
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::evaluate
//
// Purpose-
//       Get weighted bit count
//
//----------------------------------------------------------------------------
int                                 // The evaluation
   Synapse::evaluate(               // Get evaluation (without remainder)
     unsigned int      index) const // For this Neuron index
{
   assert( index < iCount );        // Verify parameter
   unsigned char* sets= getSets(index); // The associated set

   const int M= (iCount >> 3);      // Number of Axion bytes

   int gets= 0;                     // Ignore remainder
   for(int i= 0; i<M; i++)          // For each input Axion byte
   {
     int weight= ((signed char*)inwv)[i];
     if( weight >= 0 )
       weight++;

     int mask= inps[i] & sets[i];
     gets += (bitCount[mask] * weight);
   }

   return gets;
}

//----------------------------------------------------------------------------
//
// Method-
//       Synapse::update
//
// Purpose-
//       Read inputs, write outputs
//
// Implementation notes-
//       Leakage is accounted for AFTER the signal bits are counted so that
//       only one if(gets<0) test is required.
//
//----------------------------------------------------------------------------
void
   Synapse::update( void )          // Read inputs, write outputs
{
   unsigned char*      sets= this->sets; // Working sets pointer

   const int M= (iCount >> 3);      // Number of Axion bytes
   for(int n= 0; n<oCount; n++)     // For each Neuron
   {
     int gets= rems[n];             // Start with remainder

     for(int i= 0; i<M; i++)        // For each input Axion byte
     {
       int weight= ((signed char*)inwv)[i];
       if( weight >= 0 )
         weight++;

       int mask= inps[i] & sets[i];
       gets += (bitCount[mask] * weight);
     }

     int byteIndex= (n >> 3);       // Neuron byte index
     int bitsIndex= (n & 7);        // Neuron bit index
     if( gets > trig[n] )           // Neuron ON (Zero based trigger)
     {
       rems[n]= 0;                  // NO residue
       outs[byteIndex] |= (0x00000080 >> bitsIndex); // Set resultant bit
     }
     else                           // Neuron OFF
     {
       gets -= leak[n];             // Account for leakage
       if( gets < 0 )               // Minimum zero residue
         gets= 0;

       rems[n]= gets;               // Update residue
       outs[byteIndex] &= ((0x00000080 >> bitsIndex) ^ 0x000000ff); // Clear resultant bit
     }

     sets += M;                     // Address next set
   }
}

