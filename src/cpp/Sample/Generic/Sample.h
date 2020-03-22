//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.h
//
// Purpose-
//       Sample include file.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SAMPLE_H_INCLUDED
#define SAMPLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Sample
//
// Purpose-
//       Sample class.
//
//----------------------------------------------------------------------------
class Sample {                      // Sample object
//----------------------------------------------------------------------------
// Sample::Enumerations and typedefs
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Sample::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~Sample( void );                 // Destructor
   Sample( void );                  // Default constructor

//----------------------------------------------------------------------------
// Sample::Methods
//----------------------------------------------------------------------------
public:
static inline void
   method( void );                  // Sample method

//----------------------------------------------------------------------------
// Sample::Static attributes
//----------------------------------------------------------------------------
private:
   // None defined

//----------------------------------------------------------------------------
// Sample::Attributes
//----------------------------------------------------------------------------
private:
   // None defined
}; // class Sample

#include "Sample.i"

#endif // SAMPLE_H_INCLUDED
