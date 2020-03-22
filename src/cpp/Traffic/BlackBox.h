//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       BlackBox.h
//
// Purpose-
//       BlackBox history object descriptor.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#ifndef BLACKBOX_H_INCLUDED
#define BLACKBOX_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       BlackBox
//
// Purpose-
//       BlackBox history object descriptor.
//
//----------------------------------------------------------------------------
class BlackBox {                    // BlackBox history object descriptor
//----------------------------------------------------------------------------
// BlackBox::Attributes
//----------------------------------------------------------------------------
protected:
int                    cols;        // Number of columns
int                    rows;        // Number of rows

double**               history;     // The history

//----------------------------------------------------------------------------
// BlackBox::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~BlackBox( void );               // Destructor
   BlackBox(                        // Initializing constructor
     int               rows,        // Number of history rows
     int               cols);       // Number of history columns

private:                            // Bitwise copy is prohibited
   BlackBox(const BlackBox&);       // Disallowed copy constructor
   BlackBox& operator=(const BlackBox&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// BlackBox::Accessor methods
//----------------------------------------------------------------------------
public:
inline const double*                // The row
   getRow(                          // Get history row
     int               row) const   // The history row
{
   double*             result= NULL;// Resultant

   if( row >= 0 && row < rows )
     result= history[row];

   return result;
}

void
   setRow(                          // Add to history
     const double*     row);        // History row

//----------------------------------------------------------------------------
// BlackBox::Methods
//----------------------------------------------------------------------------
public:
void
   debug(                            // Debugging display
     const char*       text= "");    // Associated text
}; // class BlackBox

#endif // BLACKBOX_H_INCLUDED
