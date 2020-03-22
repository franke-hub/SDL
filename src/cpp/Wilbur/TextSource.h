//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TextSource.h
//
// Purpose-
//       A C-string DataSource.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef TEXTSOURCE_H_INCLUDED
#define TEXTSOURCE_H_INCLUDED

#include <com/DataSource.h>

//----------------------------------------------------------------------------
//
// Class-
//       TextSource
//
// Purpose-
//       C-string DataSource
//
//----------------------------------------------------------------------------
class TextSource : public DataSource { // Host file memory map
//----------------------------------------------------------------------------
// TextSource::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TextSource( void );             // Destructor
   TextSource( void );              // Default constructor

   TextSource(                      // Constructor
     const char*       text);       // Text (c-string)

//----------------------------------------------------------------------------
// TextSource::Methods
//----------------------------------------------------------------------------
public:
void
   close( void );                   // Close the TextSource

int                                 // Return code (0 OK)
   open(                            // Open the TextSource
     const char*       text);       // With this c-string text
}; // class TextSource

#endif // TEXTSOURCE_H_INCLUDED
