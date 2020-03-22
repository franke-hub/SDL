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
//       FileData.h
//
// Purpose-
//       File data container.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef FILEDATA_H_INCLUDED
#define FILEDATA_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       FileData
//
// Purpose-
//       File data container.
//
//----------------------------------------------------------------------------
class FileData {
//----------------------------------------------------------------------------
// FileData::Attributes
//----------------------------------------------------------------------------
protected:
char*                  name;        // The file name (copied)
unsigned long          size;        // The file size
void*                  addr;        // The file data

//----------------------------------------------------------------------------
// FileData::Constructors
//----------------------------------------------------------------------------
public:
   ~FileData( void );               // Default destructor
   FileData( void );                // Default constructor

   FileData(                        // Constructor
     const char*       fileName);   // The file name

private:
   FileData(const FileData&);       // Bitwise copy not allowed
   FileData& operator=(const FileData&);// Bitwise assignment not allowed

//----------------------------------------------------------------------------
// FileData::methods
//----------------------------------------------------------------------------
public:
const void*                         // The file data address
   getFileAddr( void );             // Get file data address

inline unsigned long                // The file data length
   getFileSize( void ) const        // Get file data length
{
   return size;
}

int                                 // Resultant (<0, =0, >0)
   compare(                         // Compare file data
     FileData&         comprahend); // With this comprahend

void
   reset( void );                   // Reset the FileData

void
   reset(                           // Reset the FileData
     const char*       fileName);   // The file name
}; // class FileData

inline int operator==(FileData& L, FileData& R)
{  return (L.compare(R) == 0);}

#endif // FILEDATA_H_INCLUDED
