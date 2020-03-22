//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Reader.i
//
// Purpose-
//       Reader inline methods.
//
// Last change date-
//       2003/06/22
//
//----------------------------------------------------------------------------
#ifndef READER_I_INCLUDED
#define READER_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Reader::getFilename
//
// Function-
//       fileName accessor.
//
//----------------------------------------------------------------------------
const char*                         // The file name
   Reader::getFilename( void ) const// Get file name
{
   return fileName;
}

//----------------------------------------------------------------------------
//
// Method-
//       Reader::getLine
//
// Function-
//       fileLine accessor.
//
//----------------------------------------------------------------------------
long                                // The file line
   Reader::getLine( void ) const    // Get file line
{
   return fileLine;
}

#endif // READER_I_INCLUDED
