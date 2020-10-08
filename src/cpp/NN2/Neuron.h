//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
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
//       2018/01/01
//
// Implementation notes-
//       The Neuron base class is a purely virtual object.
//       It defines the functions all Neurons have.
//
//----------------------------------------------------------------------------
#ifndef NEURON_H_INCLUDED
#define NEURON_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Neuron
//
// Purpose-
//       Neuron base class descriptor.
//
//----------------------------------------------------------------------------
class Neuron {                      // Neuron descriptor
//----------------------------------------------------------------------------
// Neuron::Typedefs
//----------------------------------------------------------------------------
public:
typedef unsigned long     RC;       // Return code, generally Fanin count
typedef unsigned long     Token;    // Token locator
typedef unsigned long     Count;    // Token count
typedef int               Pulse;    // Fanin value

//----------------------------------------------------------------------------
// Neuron::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Neuron()                        // Virtual destructor
{//debugf("Neuron(%p).~Neuron\n", this);
}

   Neuron( void )                   // Default Constructor
{//debugf("Neuron(%p).Neuron\n", this);
}

   Neuron(const Neuron&) = delete;  // Disallowed copy constructor
   Neuron& operator=(const Neuron&) = delete;// Disallowed assignment operator

//----------------------------------------------------------------------------
// Neuron::Methods
//----------------------------------------------------------------------------
public:
virtual RC                          // Fanin count
   fanin(                           // Process fanin
     Token             token,       // Storage locator
     Pulse             pulse)       // Weighted fanout value
{  (void)token; (void)pulse; return 1; } // Base class implementation

virtual RC                          // Fanin count
   fanout(                          // Process fanout
     Token             token,       // Starting Token
     Count             count)       // Token count
{  (void)token; (void)count; return 0; } // Base class implementation

virtual Count                       // The number of associated Tokens
   length( void ) const             // Return the associated Token count
{  return 0; }                      // No Tokens in the base class

virtual Token                       // The first associated Token
   origin( void ) const             // Return the Token's origin
{  return 0; }                      // No Tokens in the base class
}; // class Neuron

#endif // NEURON_H_INCLUDED
