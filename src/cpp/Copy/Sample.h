//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under creative commons CC0,
//       explicitly released into the Public Domain.
//       (See accompanying html file LICENSE.ZERO or the original contained
//       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
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
//       2023/07/23
//
// Implementation note-
//       The basic header file template is given to the public domain.
//       You can freely use it without attribution of any kind.
//
//       The header file's "look and feel" is explicitly not copyrighted.
//
//----------------------------------------------------------------------------
#ifndef _SAMPLE_H_INCLUDED
#define _SAMPLE_H_INCLUDED

#include <functional>               // For std::function

//----------------------------------------------------------------------------
//
// Class-
//       Sample
//
// Purpose-
//       Sample (pretty much useless) local library object.
//
//----------------------------------------------------------------------------
class Sample {                      // The Sample Object
//----------------------------------------------------------------------------
// Sample::Attributes
//----------------------------------------------------------------------------
protected:
typedef std::function<void(void)>   f_run;  // The run function type
typedef std::string                 string; // We use std::string

f_run                  runner;      // The run function (default empty)
string                 name;        // The name of the Sample
static Sample          global;      // A global Sample

//----------------------------------------------------------------------------
// Sample::Constructors/Destructors
//----------------------------------------------------------------------------
public:
   Sample( void );

virtual
   ~Sample( void );

// Disallowed: Copy constructor, assignment operator
   Sample(const Sample&) = delete;
Sample& operator=(const Sample&) = delete;

void
   debug(const char* info="") const; // Debugging display

//----------------------------------------------------------------------------
// Sample::Accessor methods
//----------------------------------------------------------------------------
string get_string( void ) { return name; }
void set_string(string s) { name= s; }

void on_run(f_run r) { runner= r; } // Replace runner

//----------------------------------------------------------------------------
// Sample::Methods
//----------------------------------------------------------------------------
void
   run( void )                      // Operate this Sample
{  runner(); }                      // Invoke the runner lambda function

void
   start( void );                   // Start this Sample
}; // class Sample
#endif // _SAMPLE_H_INCLUDED
