//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileSource.h
//
// Purpose-
//       A FileSource memory maps a host file.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef FILESOURCE_H_INCLUDED
#define FILESOURCE_H_INCLUDED

#ifndef DATASOURCE_H_INCLUDED
#include "com/DataSource.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       FileSource
//
// Purpose-
//       Memory map of a host file.
//
//----------------------------------------------------------------------------
class FileSource : public DataSource { // Host file memory map
//----------------------------------------------------------------------------
// FileSource::Attributes
//----------------------------------------------------------------------------
protected:
int                    handle;      // File handle

//----------------------------------------------------------------------------
// FileSource::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FileSource( void );             // Destructor
   FileSource( void );              // Default constructor

   FileSource(                      // Constructor
     const char*       fileName);   // File name

//----------------------------------------------------------------------------
// FileSource::Methods
//----------------------------------------------------------------------------
public:
virtual DataSource*                 // -> DataSource
   clone(                           // Clone this DataSource
     const char*       name) const; // With this (relative) name

void
   close( void );                   // Close the file

int                                 // Return code (0 OK)
   open(                            // Open a file
     const char*       fileName);   // With this file name
}; // class FileSource

#endif // FILESOURCE_H_INCLUDED
