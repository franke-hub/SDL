//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Neuron.h
//
// Purpose-
//       Define the Neuron object.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       Neuron is a flat class.
//       Constructors, destructors and virtual methods are not allowed.
//
//----------------------------------------------------------------------------
#ifndef NEURON_H_INCLUDED
#define NEURON_H_INCLUDED

#ifndef TYPES_H_INCLUDED
#include "Types.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Fanin;

//----------------------------------------------------------------------------
//
// Class-
//       Neuron
//
// Purpose-
//       Neuron descriptor.
//
//----------------------------------------------------------------------------
class Neuron                        // Neuron descriptor
{
//----------------------------------------------------------------------------
// Neuron::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum                                // Generic constants
{
   CBID=                     0xFE01 // Control block validator
}; // enum

//----------------------------------------------------------------------------
// Neuron::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Neuron( void );                 // Destructor
   Neuron( void );                  // Default constructor

private:                            // Bitwise copy is prohibited
   Neuron(const Neuron&);           // Disallowed copy constructor
   Neuron& operator=(const Neuron&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Neuron::Accessor methods
//----------------------------------------------------------------------------
public:
inline Value                        // Value
   getValue( void ) const;          // Extract value

inline void
   setValue(                        // Set Neuron Value
     Value           value);        // to this

//----------------------------------------------------------------------------
// Neuron::Methods
//----------------------------------------------------------------------------
protected:
virtual Value                       // Current value
   compute( void );                 // Compute value

inline Value                        // Sum of fanin[i].resolve*fanin[i].weight
   sigma( void );                   // Compute sum of inputs

public:
inline Value                        // Current value
   resolve( void );                 // Resolve value

inline void
   setFanin(                        // Set Fanin array
     unsigned        count,         // Number of elements in array
     Fanin*          array);        // -> Fanin array

//----------------------------------------------------------------------------
// Neuron::Static attributes
//----------------------------------------------------------------------------
public:
static Tick          globalClock;   // The global clock
static unsigned long globalCount;   // Number of Neuron reads

//----------------------------------------------------------------------------
// Neuron::Attributes
//----------------------------------------------------------------------------
protected:
                                    // 0000 Virtual Function Table
   unsigned short    cbid;          // 0004 Control block identifier
   unsigned short    _0006;         // 0006 (reserved for expansion)

   struct                           // 0008 Status and exception controls
   {
     unsigned int               : 8;// Reserved, available
     unsigned int               : 8;// Reserved, available

     unsigned int    any        : 1;// Exception summary indicator
     unsigned int               : 7;// Reserved, available

     unsigned int               : 6;// Reserved, available
     unsigned int    breakpt    : 1;// This neuron has a breakpoint
     unsigned int    disabled   : 1;// This neuron is disabled
   } ex;

   Tick              clock;         // 000C Last clock tick

   unsigned          faninCount;    // 0010 The number of fanin elements
   Fanin*            faninArray;    // 0014 -> Fanin array extension

   void*             _0010;         // 0018 (Reserved for expansion)
   Value             value;         // 001C Raw output value
}; // class Neuron                  // 0020

//----------------------------------------------------------------------------
//
// Class-
//       NeuronValue
//
// Purpose-
//       Value Neuron descriptor.
//
//----------------------------------------------------------------------------
class NeuronValue : public Neuron   // NeuronValue descriptor
{
//----------------------------------------------------------------------------
// NeuronValue::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NeuronValue( void );            // Destructor
   NeuronValue( void );             // Default constructor

//----------------------------------------------------------------------------
// NeuronValue::Methods
//----------------------------------------------------------------------------
protected:
virtual Value                       // Current value
   compute( void );                 // Compute value
};

#endif // NEURON_H_INCLUDED
