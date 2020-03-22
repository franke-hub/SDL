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
//       ParseINI.h
//
// Purpose-
//       File parameter controls.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       An external parameter file consists of sections,
//         [section-name]                       <; comment to end of line>
//
//       and parameter value declarations,
//         parameter-name <= <parameter-value>> <; comment to end of line>
//
//       Parameter value declartions are allowed without a section name.
//       These values may be extracted using a NULL value for the section
//       name specifier.
//
//       Names and values are limited to ParseINI::MAXSIZE characters.
//       Larger strings are truncated.
//
//       Leading and trailing blanks are removed from both the parameter
//       name and the parameter value, but quotations can be used if these
//       are required, or if a semicolon is required in a name or value.
//
//       Lines in the parameter file beginning with a semicolon are
//       considered comments and are ignored.
//
//----------------------------------------------------------------------------
#ifndef PARSEINI_H_INCLUDED
#define PARSEINI_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       ParseINI
//
// Purpose-
//       External Parameter controls.
//
//----------------------------------------------------------------------------
class ParseINI {
//----------------------------------------------------------------------------
// ParseINI::Attributes
//----------------------------------------------------------------------------
private:
void*                  object;      // The ParseINI Object

//----------------------------------------------------------------------------
// ParseINI::Constants for Parameterization
//----------------------------------------------------------------------------
public:
   enum {
   MAXSIZE= 1024                    // The largest string length
};

//----------------------------------------------------------------------------
// ParseINI::Constructors
//----------------------------------------------------------------------------
public:
   ~ParseINI( void ) {destroy();}   // Default destructor
   ParseINI( void )                 // Default constructor
:  object(NULL) {}                  // Default constructor

void
   construct( void );               // In situ constructor
void
   destroy( void );                 // In situ destructor

//----------------------------------------------------------------------------
// ParseINI::methods
//----------------------------------------------------------------------------
public:
void
   open(                            // Open the parameter file
     const char*       parmFile);   // The parameter file name

void
   close( void );                   // Close the parameter file

const char*                         // The parameter's value
   getValue(                        // Extract parameter value
     const char*       sectName,    // The section name (NULL allowed)
     const char*       parmName);   // The parameter's name
}; // class ParseINI

#endif // PARSEINI_H_INCLUDED
