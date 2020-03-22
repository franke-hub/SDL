//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
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
//       2020/01/29
//
// Implementation note-
//       The basic header file template is given to the public domain.
//       You can freely use it without attribution of any kind.
//
//       The header file's "look and feel" is explicitly not copyrighted.
//
//----------------------------------------------------------------------------
#ifndef _PUB_SAMPLE_H_INCLUDED
#define _PUB_SAMPLE_H_INCLUDED

#include "pub/config.h"             // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Sample
//
// Purpose-
//       A standard Sample Object.
//
//----------------------------------------------------------------------------
class Sample {                      // The Sample Object
//----------------------------------------------------------------------------
// Sample::Attributes
//----------------------------------------------------------------------------
protected:
typedef std::string    string;      // We use std::string

string                 name;        // The name of the Sample
static Sample          global;      // A global Sample

//----------------------------------------------------------------------------
// Sample::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Sample( void );
   Sample( void );

// Disallowed: Copy constructor, assignment operator
   Sample(const Sample&) = delete;
Sample& operator=(const Sample&) = delete;

void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
// Sample::Methods
//----------------------------------------------------------------------------
public:
// OVERRIDE this method
virtual void
   run( void );                     // Operate this Sample

void
   start( void );                   // Start this Sample
}; // class Sample
}  // namespace _PUB_NAMESPACE
#endif // _PUB_SAMPLE_H_INCLUDED
