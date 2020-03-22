//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//--------------------------------------------------------------------------
//
// Title-
//       Synapse.h
//
// Purpose-
//       Define the Synapse object.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef SYNAPSE_H_INCLUDED
#define SYNAPSE_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include "Object.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Synapse
//
// Purpose-
//       Synapse descriptor.
//
// Description-
//       The weighted inputs for each Neuron are summed. This total is added
//       to the remainder (residue) value for each Neuron. If the firing
//       threshold is reached, the output is set and the residue set to zero.
//       Otherwise, the residue is set to the total minus the leakage, but
//       never less than zero. Weights range from -256..+256, disallowing 0.
//       Each block of eight inputs have the same weight.
//
// Logics-
//       For each cycle:
//
//       Gets[i]= Rems[i]+Sigma(Inps*Weight)
//       if( Gets[i] >= Trig[i] )
//         N[i]= 1
//         Rems[i]= 0
//       else
//         N[i]= 0
//         Rems[i]= MAX(Gets[i] - Loss[i], 0) // Never less than zero
//
// Example-
//
//        -- Positive --            ------ Negative ------
//
//        I   I   I   I               I     I     I     I     G  R  L  T  O
//       [0] [1] [2] [3]            [K-3] [K-2] [K-1]  [K]    e  e  e  r  u
//        |   |   |   |               |     |     |     |     t  m  a  i  t
//        0   0   1   1               0     1     0     1     s  s  k  g  s
//        |   |   |   |               |     |     |     |  N  |  |  |  |  |
//       -|---|---|---|---|---|---|---|-----|-----|-----|-[0]-x--x--x--x--x-
//       -|---|---|---|---|---|---|---|-----|-----|-----|-[1]-x--x--x--x--x-
//       -|---|---|---|---|---|---|---|-----|-----|-----|-[2]-x--x--x--x--x-
//       -|---|---|---|---|---|---|---|-----|-----|-----|-[3]-x--x--x--x--x-
//        :   :   :   :   :   :   :   :     :     :     :  :
//       -|---|---|---|---|---|---|---|-----|-----|-----|-[M]-x--x--x--x--x-
//        |   |   |   |   |   |   |   |     |     |     |     |  |  |  |  |
//
//             Gets Rems Leak Trig Outs
//       N[0]=    x    x    x    x    x
//       N[1]=    x    x    x    x    x
//       N[2]=    x    x    x    x    x
//       N[3]=    x    x    x    x    x
//       N[M]=    x    x    x    x    x
//
// Training-
//       TBD.
//       MaxInput= PositiveInps - NegativeInps
//       Higher MaxInput values promote Neuron outputs.
//       Higher trigger values inhibit Neuron outputs.
//       Higher loss values inhibit delayed Neuron outputs.
//       OUT= Trig >= (Inps + MAX(Rems - Loss, 0))
//
//----------------------------------------------------------------------------
class Synapse : public Object {     // Synapse descriptor
//----------------------------------------------------------------------------
// Synapse::Enumerations and typedefs
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Synapse::Attributes
//----------------------------------------------------------------------------
unsigned int           iCount;      // Number of input Axions
unsigned int           oCount;      // Number of output Neurons

unsigned char*         inps;        // Input bit vector
unsigned char*         sets;        // Input to output bit control vector
unsigned char*         outs;        // Neuron output bit vector

unsigned char*         inwv;        // Input weight vector
unsigned char*         rems;        // Neuron residue vector (Remainder)
unsigned char*         leak;        // Neuron leakage vector
unsigned char*         trig;        // Neuron firing threshold vector

//----------------------------------------------------------------------------
// Synapse::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Synapse( void );                // Destructor

   //-------------------------------------------------------------------------
   // Notes:
   //   iCount and oCount must be non-zero multiples of eight.
   Synapse(                         // Constructor
     unsigned int      iCount,      // Number of input Axions
     unsigned int      oCount);     // Number of output Neurons

//----------------------------------------------------------------------------
// Synapse::Accessor methods
//----------------------------------------------------------------------------
public:
inline unsigned int                 // The number of input Axions
   getICount( void ) const          // Get number of input Axions
{  return iCount; }

inline unsigned int                 // The number of output Neurons
   getOCount( void ) const          // Get number of output Neurons
{  return oCount; }

int                                 // The total weight of set bits
   getBits(                         // Get total weight of set bits
     unsigned int      index) const;// For this Neuron index

inline unsigned char*               // The input vector
   getInps( void ) const            // Get input vector
{  return inps; }

inline unsigned char*               // The leak vector
   getLeak( void ) const            // Get leak vector
{  return leak; }

unsigned int                        // The associated leak value
   getLeak(                         // Get associated leak value
     unsigned int      index) const;// For this Neuron index

void
   setLeak(                         // Set leak value
     unsigned int      index,       // For this Neuron index
     unsigned int      value);      // To this value (range 0..255)

inline unsigned char*               // The output vector
   getOuts( void ) const            // Get output vector
{  return outs; }

inline unsigned char*               // The remainder vector
   getRems( void ) const            // Get remainder vector
{  return rems; }

inline unsigned char*               // The transform vector
   getSets( void ) const            // Get transform vector
{  return sets; }

inline unsigned char*               // The transform vector
   getSets(                         // Get transform vector
     unsigned int      index) const // For this Neuron index
{  return sets + ((iCount*index)>>3); }

inline unsigned char*               // The trigger vector
   getTrig( void ) const            // Get trigger vector
{  return trig; }

unsigned int                        // The associated trigger value
   getTrig(                         // Get associated trigger value
     unsigned int      index) const;// For this Neuron index

void
   setTrig(                         // Set trigger value
     unsigned int      index,       // For this Neuron index
     unsigned int      value);      // To this value (range 1..256)

inline unsigned char*               // The input weight vector
   getWeight( void ) const          // Get input weight vector
{  return inwv; }

// Axion weights are bundled into groups of eight
int                                 // The associated weight
   getWeight(                       // Get associated weight
     unsigned int      index) const;// For this Axion (bundle) index

void
   setWeight(                       // Set weight value
     unsigned int      index,       // For this Axion (bundle) index
     int               weight);     // To this value (range +/-1..255)

//----------------------------------------------------------------------------
// Synapse::Methods
//----------------------------------------------------------------------------
public:
int                                 // The evaluation
   evaluate(                        // Get evaluation
     unsigned int      index) const;// For this Neuron index

virtual void
   disable(                         // Disable (set to 0)
     unsigned int      inp,         // Input (axion) index
     unsigned int      out);        // Output (neuron) index

virtual void
   enable(                          // Enable (set to 1)
     unsigned int      inp,         // Input (axion) index
     unsigned int      out);        // Output (neuron) index

virtual void
   update( void );                  // Read inputs, write outputs
}; // class Synapse

#endif // SYNAPSE_H_INCLUDED
