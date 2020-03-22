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
//       DarwinPlex.h
//
// Purpose-
//       DarwinPlex base class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DARWINPLEX_H_INCLUDED
#define DARWINPLEX_H_INCLUDED

#ifndef  INLINE_H_INCLUDED
#include "inline.h"
#endif

#include "Random.h"

#ifndef DARWINUNIT_H_INCLUDED
#include "DarwinUnit.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DarwinPlex
//
// Purpose-
//       Group of DarwinUnit descriptors.
//
//----------------------------------------------------------------------------
class DarwinPlex {                  // DarwinUnit group
//----------------------------------------------------------------------------
// DarwinPlex::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef unsigned long  Generation;  // Generation index

//----------------------------------------------------------------------------
// DarwinPlex::Attributes
//----------------------------------------------------------------------------
protected:
const char*            className;   // The UNIQUE class name to which all
                                    // DarwinUnits belong

unsigned int           count;       // The maximum number of DarwinUnits
unsigned int           used;        // The number of DarwinUnits used
DarwinUnit**           unit;        // The array of DarwinUnit pointers

Generation             generation;  // The current generation
unsigned int           mutation;    // The mutation count

public:
double                 probCull;    // The cull probability
double                 probMute;    // The mutation probability

//----------------------------------------------------------------------------
// DarwinPlex::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DarwinPlex( void );             // Destructor

   DarwinPlex(                      // Constructor
     unsigned int      elements);   // The number of group elements

private:                            // Bitwise copy is prohibited
   DarwinPlex(const DarwinPlex&);   // Disallowed copy constructor
   DarwinPlex& operator=(const DarwinPlex&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DarwinPlex::Accessors
//----------------------------------------------------------------------------
public:
INLINE unsigned int                 // The number of units to cull/generation
   getCull( void ) const;           // Get cull count

INLINE Generation                   // The current generation
   getGeneration( void ) const;     // Get current generation

INLINE void
   setGeneration(                   // Set current generation
     Generation        generation); // To this value

INLINE unsigned int                 // The mutation count
   getMutation( void ) const;       // Get mutation counter

INLINE DarwinUnit*                  // -> DarwinUnit
   getUnit(                         // Get a DarwinUnit
     unsigned int      index) const;// The DarwinUnit index

INLINE unsigned int                 // The allocated element index
   setUnit(                         // Set a DarwinUnit
     DarwinUnit*       element);    // -> DarwinUnit

INLINE unsigned int                 // The number of used elements
   getUsed( void ) const;           // Get number of used elements

//----------------------------------------------------------------------------
// DarwinPlex::Virtual methods
//----------------------------------------------------------------------------
public:
virtual void
   evaluate( void );                // Evaluate all DarwinUnits

//----------------------------------------------------------------------------
// DarwinPlex::Methods
//----------------------------------------------------------------------------
public:
void
   generate( void );                // Create a new generation
}; // class DarwinPlex

#if INLINING
#include "DarwinPlex.i"
#endif

#endif // DARWINPLEX_H_INCLUDED
