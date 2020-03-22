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
//       SynapseBundle.h
//
// Purpose-
//       Define the SynapseBundle object.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef SYNAPSEBUNDLE_H_INCLUDED
#define SYNAPSEBUNDLE_H_INCLUDED

#ifndef OBJECT_H_INCLUDED
#include "Object.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Synapse;                      // The Synapse class

//----------------------------------------------------------------------------
//
// Class-
//       SynapseBundle
//
// Purpose-
//       SynapseBundle descriptor.
//
//----------------------------------------------------------------------------
class SynapseBundle : public Object { // SynapseBundle descriptor
//----------------------------------------------------------------------------
// SynapseBundle::Attributes
//----------------------------------------------------------------------------
unsigned int           bCount;      // Number of bundles
unsigned int           iCount;      // Number of input Axions
unsigned int           oCount;      // Number of output Neurons

Synapse**              bundle;      // The Synapse bundle

//----------------------------------------------------------------------------
// SynapseBundle::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SynapseBundle( void );          // Destructor

   //-------------------------------------------------------------------------
   // Notes:
   //   iCount and oCount must be non-zero multiples of eight.
   SynapseBundle(                   // Constructor
     unsigned int      bCount,      // Number of bundles
     unsigned int      iCount,      // Number of input Axions/Synapse
     unsigned int      oCount);     // Number of output Neurons/Synapse

//----------------------------------------------------------------------------
// SynapseBundle::Accessor methods
//----------------------------------------------------------------------------
public:
inline unsigned int                 // The number of bundles
   getBCount( void ) const          // Get number of bundles
{  return bCount; }

inline unsigned int                 // The number of input Axions
   getICount( void ) const          // Get number of input Axions
{  return iCount; }

inline unsigned int                 // The number of output Neurons
   getOCount( void ) const          // Get number of output Neurons
{  return oCount; }

inline Synapse*                     // The associated Synapse
   getSynapse(                      // Get associated Synapse
     unsigned int      index) const // For this bundle index
{  return bundle[index]; }          // Note: No range checking

//----------------------------------------------------------------------------
// Synapse::Methods
//----------------------------------------------------------------------------
public:
virtual void
   update( void );                  // Read inputs, write outputs
}; // class SynapseBundle

#endif // SYNAPSEBUNDLE_H_INCLUDED
