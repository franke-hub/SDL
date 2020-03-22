//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Archive.h
//
// Purpose-
//       Archive retrieval mechanism.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Usage of any Archive object requires libbz2 and libz
//       BSD only. Windows support is not available.
//
//----------------------------------------------------------------------------
#ifndef ARCHIVE_H_INCLUDED
#define ARCHIVE_H_INCLUDED

#include<stdint.h>
#include<sys/types.h>               // For time_t

#ifndef DATASOURCE_H_INCLUDED
#include <com/DataSource.h>
#endif

//----------------------------------------------------------------------------
// Archive classes (for reference)
//----------------------------------------------------------------------------
class BzipArchive;                  // BZIP  encoded Archive
class DiskArchive;                  // TAR   encoded Archive (disk resident)
class GzipArchive;                  // GZIP  encoded Archive
class Zz32Archive;                  // ZIP32 encoded Archive
class Zz64Archive;                  // ZIP64 encoded Archive

//----------------------------------------------------------------------------
//
// Class-
//       Archive
//
// Purpose-
//       Archive retrieval.
//
// Implementation notes-
//       The base Archive is a file.
//       Encrypted Archives are not supported.
//
//----------------------------------------------------------------------------
class Archive : public DataSource {
//----------------------------------------------------------------------------
// Archive::Attributes (For current item)
//----------------------------------------------------------------------------
protected:
DataSource*            file;        // Input DataSource
unsigned int           mode;        // File mode (per sys/stat.h)
time_t                 time;        // Modification time
unsigned int           object;      // Current object index

//----------------------------------------------------------------------------
// Archive::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Archive( void );                // Destructor

protected:
   Archive( void );                 // Constructor

//----------------------------------------------------------------------------
//
// Method-
//       Archive::make
//
// Purpose-
//       Allocate an Archive.
//
// Implementation notes-
//       The only way to create a Archive is via one of the make methods.
//       Use the resultant archive to retrieve files. Delete it when done.
//
//----------------------------------------------------------------------------
public:
static Archive*                     // Resultant Archive (NULL if none)
   make(                            // Create Archive
     const char*       fileName);   // For this file name

//----------------------------------------------------------------------------
//
// This alternatative get method passes ownership of the DataSource to the
// resultant Archive (if one was created.) If needed, use the take method to
// regain control of the source Archive.
//
//----------------------------------------------------------------------------
static Archive*                     // Resultant Archive (NULL if none)
   make(                            // Create Archive
     DataSource*       archive);    // From this DataSource

//----------------------------------------------------------------------------
//
// Method-
//       Archive::take
//
// Purpose-
//       Take back the source Archive, then delete this Archive.
//
// Implementation note-
//       Once the DataSource is removed, this Archive becomes unusable.
//       The delete side-effect emphasizes this.
//
//----------------------------------------------------------------------------
public:
virtual DataSource*                 // The DataSource (if any)
   take( void );                    // Take DataSource, delete this Archive

//----------------------------------------------------------------------------
// Archive::Accessor methods
//----------------------------------------------------------------------------
public:
inline unsigned int                 // The current item file mode
   getMode( void ) const            // Get item mode
{  return mode; }

inline time_t                       // The current item modification time
   getTime( void ) const            // Get item modification time
{  return time; }

//----------------------------------------------------------------------------
//
// Method-
//       Archive::index
//
// Purpose-
//       Select an Archive object by index
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // The object name (NULL if missing)
   index(                           // Select object number
     unsigned int      index);      // The object index

//----------------------------------------------------------------------------
//
// Method-
//       Archive::next
//
// Purpose-
//       Skip to the next object.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // The next object name
   next( void );                    // Skip to the next object

//----------------------------------------------------------------------------
//
// Method-
//       Archive::setOffset
//
// Purpose-
//       Update current offset.
//
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   setOffset(                       // Set data offset
     int64_t           offset);     // To this value

//----------------------------------------------------------------------------
//
// Method-
//       Archive::read
//
// Purpose-
//       Read from current object.
//
// Implementation notes-
//       The result is set to zero when all input has been read or an
//       error is detected. if getLength() == getOffset(), no error.
//
//----------------------------------------------------------------------------
public:
virtual unsigned int                // Number of bytes read
   read(                            // Read (from current object)
     void*             addr,        // Input buffer address
     unsigned int      size);       // Input buffer length
}; // class Archive
#endif // ARCHIVE_H_INCLUDED
