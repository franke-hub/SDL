//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Approximately.h
//
// Purpose-
//       Calculate an appoximate event counter.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       The underlying algorithm is in the public domain.
//
//----------------------------------------------------------------------------
#ifndef APPROXIMATELY_H_INCLUDED
#define APPROXIMATELY_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Approximately
//
// Purpose-
//       Approximate event counter.
//
//----------------------------------------------------------------------------
class Approximately {               // Approximate event counter
//----------------------------------------------------------------------------
// Approximately::Attributes
//----------------------------------------------------------------------------
protected:
unsigned char          exponent;    // Event count exponent

//----------------------------------------------------------------------------
// Approximately::Constructors
//----------------------------------------------------------------------------
public:
   ~Approximately( void ) {}        // Destructor
   Approximately(                   // Constructor
     unsigned          count = 0);  // Initial count

public:                             // Bitwise copy is allowed
   Approximately(const Approximately& source) // Copy constructor
{  this->exponent= source.exponent; }

Approximately&
   operator=(const Approximately& source) // Assignment operator
{  this->exponent= source.exponent; return *this; }

//----------------------------------------------------------------------------
// Approximately::Accessors
//----------------------------------------------------------------------------
public:
unsigned                            // The number of events
   getCount( void ) const           // Get event count
{
   unsigned result= 1 << exponent;  // The resultant count
   result--;                        // Decrement by one

   return result;
}

//----------------------------------------------------------------------------
// Approximately::Methods
//----------------------------------------------------------------------------
public:
void
   event( void );                   // Count an event
}; // class Approximately

#endif // APPROXIMATELY_H_INCLUDED
