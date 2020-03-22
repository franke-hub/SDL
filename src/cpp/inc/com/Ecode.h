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
//       Ecode.h
//
// Purpose-
//       Error code container.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef ECODE_H_INCLUDED
#define ECODE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Ecode
//
// Purpose-
//       Describe the generic error code container.
//
//----------------------------------------------------------------------------
class Ecode {                       // Error code containuer
//----------------------------------------------------------------------------
// Ecode::Attributes
//----------------------------------------------------------------------------
protected:
int                    ecode;       // The error code

//----------------------------------------------------------------------------
// Ecode::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Define the I/O error codes
{
   EC_RESET=                  (-31),// Reset
   EC_FAULT=                  (-30),// System function failure
   EC_PERM=                   (-29),// Permanent error
   EC_TEMP=                   (-15),// Temporary error
   EC_PARM=                    (-9),// Invalid parameter
   EC_MODE=                    (-8),// Invalid mode
   EC_EOM=                     (-5),// End of media
   EC_LEN=                     (-4),// Length error
   EC_ERR=                     (-3),// Error character
   EC_TOF=                     (-2),// Top of file
   EC_EOF=                     (-1),// End of file
   EC_0=                          0 // No error
}; // enum

//----------------------------------------------------------------------------
// Ecode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Ecode( void );                  // Default destructor
   Ecode( void );                   // Default constructor

//----------------------------------------------------------------------------
// Method-
//       getEcode
//
// Purpose-
//       Extract the error code.
//----------------------------------------------------------------------------
public:
virtual int                         // The error code
   getEcode( void ) const;          // Get the error code

//----------------------------------------------------------------------------
// Method-
//       setEcode
//
// Purpose-
//       Set the error code.
//----------------------------------------------------------------------------
public:
virtual int                         // The error code
   setEcode(                        // Set the error code
     int               value);      // To this value
}; // class Ecode

#endif // ECODE_H_INCLUDED
