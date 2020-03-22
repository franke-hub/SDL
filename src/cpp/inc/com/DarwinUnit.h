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
//       DarwinUnit.h
//
// Purpose-
//       DarwinUnit Abstract Base Class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DARWINUNIT_H_INCLUDED
#define DARWINUNIT_H_INCLUDED

#ifndef  INLINE_H_INCLUDED
#include "inline.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DarwinUnit
//
// Purpose-
//       DarwinUnit (element) descriptor.
//
//----------------------------------------------------------------------------
class DarwinUnit {                  // DarwinUnit
//----------------------------------------------------------------------------
// DarwinUnit::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef unsigned long  Evaluation;  // Resultant for evaluate()

//----------------------------------------------------------------------------
// DarwinUnit::Attributes
//----------------------------------------------------------------------------
public:                             // Must be public,
                                    // used by derivations of DarwinPlex

// Values set by DarwinPlex::evaluate()
Evaluation             evaluation;  // The last evaluation

// Values set by DarwinPlex::generate()
unsigned long          generation;  // The last change generation

unsigned               changed    : 1; // This Unit has changed
unsigned               mutated    : 1; // This Unit has mutated
unsigned               evolChange : 1; // This Unit evolved from a changed Unit
unsigned               evolMutate : 1; // This Unit evolved from a mutated Unit
unsigned                          : 4; // Reserved for DarwinPlex::generate()
unsigned                          : 8; // Reserved for DarwinPlex::generate()

// User controls
unsigned               isValid    : 1; // This Unit's evaluation is valid
                                    // (Reset when changed)
unsigned                        : 7; // Reserved for DarwinPlex::generate()
unsigned                        : 8; // Reserved for DarwinPlex::generate()

//----------------------------------------------------------------------------
// DarwinUnit::Constructor
//----------------------------------------------------------------------------
public:
virtual
   ~DarwinUnit( void );             // Destructor

protected:
   DarwinUnit( void );              // Constructor

private:                            // Bitwise copy is prohibited
   DarwinUnit(const DarwinUnit&);   // Disallowed copy constructor
   DarwinUnit& operator=(const DarwinUnit&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DarwinUnit::Virtual methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // -> ClassName
   className( void ) const;         // Get the UNIQUE class name

//----------------------------------------------------------------------------
// DarwinUnit::Pure virtual methods
//----------------------------------------------------------------------------
public:
virtual void*                       // The concrete class
   castConcrete( void ) const = 0;  // Cast to concrete class

virtual Evaluation                  // Evaluation
   evaluate( void ) = 0;            // Evaluate the Rule

virtual void
   evolve(                          // Evolve the Rule
     const DarwinUnit* father,      // Father object
     const DarwinUnit* mother) = 0; // Mother object

virtual void
   mutate( void ) = 0;              // Mutate the Rule

//----------------------------------------------------------------------------
// DarwinUnit::Static methods
//----------------------------------------------------------------------------
public:
static void
   evolve(                          // Evolve a rule
     unsigned int      size,        // The rule size
     char*             target,      // Target rule
     const char*       father,      // Father rule
     const char*       mother);     // Mother rule

static void
   mutate(                          // Mutate a rule
     unsigned int      size,        // The rule size
     char*             target);     // The rule

static void
   toFile(                          // Write the rule ('1's and '0's)
     FILE*             file,        // The output file
     unsigned int      size,        // The rule size
     const char*       rule);       // The rule

static char*                        // Resultant
   toString(                        // Convert to string ('1's and '0's)
     char*             resultant,   // The output string
     unsigned int      size,        // The rule size
     const char*       rule);       // The rule
}; // class DarwinUnit

#endif // DARWINUNIT_H_INCLUDED
