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
//       Scode.h
//
// Purpose-
//       Status code container.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef SCODE_H_INCLUDED
#define SCODE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Scode
//
// Purpose-
//       Describe the generic status code container.
//
//----------------------------------------------------------------------------
class Scode {                       // Status code containuer
//----------------------------------------------------------------------------
// Scode::Attributes
//----------------------------------------------------------------------------
protected:
int                    scode;       // The status code

//----------------------------------------------------------------------------
// Scode::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Define the I/O error codes
{
   SC_RESET=                  (-31),// Reset
   SC_FAULT=                  (-30),// System function failure
   SC_PERM=                   (-29),// Permanent error
   SC_TOF=                     (-2),// Top of file
   SC_EOF=                     (-1),// End of file
   SC_0=                          0 // No error
}; // enum

//----------------------------------------------------------------------------
// Scode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Scode( void );                  // Default destructor
   Scode( void );                   // Default constructor

//----------------------------------------------------------------------------
// Method-
//       getScode
//
// Purpose-
//       Extract the status code.
//----------------------------------------------------------------------------
public:
virtual int                         // The status code
   getScode( void ) const;          // Get the status code

//----------------------------------------------------------------------------
// Method-
//       setScode
//
// Purpose-
//       Set the status code.
//----------------------------------------------------------------------------
protected:
virtual int                         // The status code
   setScode(                        // Set the status code
     int               value);      // To this value
}; // class Scode

#endif // SCODE_H_INCLUDED
