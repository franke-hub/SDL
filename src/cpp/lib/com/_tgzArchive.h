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
//       _tgzArchive.h
//
// Purpose-
//       Define and implement the _tgzArchive object.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Included from Archive.cpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       _tgzArchive
//
// Purpose-
//       Gzip'ed TAR format Archive
//
//----------------------------------------------------------------------------
class _tgzArchive : public Archive { // TGZ Archive
protected: // ATTRIBUTES
GzipArchive*           gzip;        // The GZIP Archive
DiskArchive*           disk;        // The TAR  Archive

public: // CONSTRUCTORS
virtual
   ~_tgzArchive( void );            // Destructor
   _tgzArchive(                     // Constructor
     DataSource*       file);       // DataSource

public:
static _tgzArchive*                 // Resultant _tgzArchive (NULL if none)
   make(                            // Create _tgzArchive
     DataSource*       file);       // DataSource

virtual DataSource*                 // The DataSource (if any)
   take( void );                    // Take DataSource, delete this Archive

public: // METHODS
virtual const char*                 // The item name (NULL if missing)
   index(                           // Select item number
     unsigned int      index);      // The item index

virtual const char*                 // The next item name
   next( void );                    // Skip to the next item

virtual unsigned int                // Number of bytes read
   read(                            // Read (from current item)
     void*             addr,        // Input buffer address
     unsigned int      size);       // Input buffer length

virtual int                         // Return code (0 OK)
   setOffset(                       // Position within current item
     int64_t           offset);     // Offset
}; // class _tgzArchive

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::~_tgzArchive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   _tgzArchive::~_tgzArchive( void ) // Destructor
{
   // The gzip file is owned by the DiskArchive file. (Don't delete it here.)
   gzip= NULL;

   if( disk != NULL )
   {
     delete disk;
     disk= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::_tgzArchive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   _tgzArchive::_tgzArchive(        // Constructor
     DataSource*       file)        // DataSource
:  Archive()
{
   // Create/validate the GZIP Archive
   gzip= GzipArchive::make(file);   // Create the GZIP Archive

   if( gzip != NULL )
     disk= DiskArchive::make(gzip); // Create the TAR Archive

   if( disk != NULL )
   {
     name= disk->getName();
     mode= disk->getMode();
     time= disk->getTime();

     offset= disk->getOffset();
     length= disk->getLength();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::make
//
// Purpose-
//       Allocate and initialize a _tgzArchive.
//
//----------------------------------------------------------------------------
_tgzArchive*                        // Resultant
   _tgzArchive::make(               // Allocate and initialize a _tgzArchive
     DataSource*       file)        // From this DataSource
{
   _tgzArchive*        result= NULL; // Resultant Archive

   try {
     result= new _tgzArchive(file); // Allocate resultant
     if( result->disk == NULL )     // If failure
     {
       // Only .tar.gz or .tgz file extensions
       fprintf(stderr, "File(%s) invalid format\n", file->getCName());

       delete result;
       result= NULL;
     }
   } catch(const char* X) {
     fprintf(stderr, "File(%s) open failure(%s)\n", file->getCName(), X);
     if( result != NULL )
       delete result;

     result= NULL;
   } catch(...) {
     fprintf(stderr, "File(%s) open failure(...)\n", file->getCName());
     if( result != NULL )
       delete result;

     result= NULL;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::take
//
// Purpose-
//       Return the DataSource
//
//----------------------------------------------------------------------------
DataSource*                         // Resultant
   _tgzArchive::take( void )        // Return the DataSource
{
   DataSource* result= gzip->resetFile();

   delete this;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::index
//
// Function-
//       Select an item by index
//
//----------------------------------------------------------------------------
const char*                         // The item name
   _tgzArchive::index(              // Select
     unsigned int      index)       // This item index
{
   if( disk == NULL )
     return gzip->index(index);

   const char* name= disk->index(index);
   if( name == NULL )
     this->name= "";
   else
     this->name= name;

   mode= disk->getMode();
   time= disk->getTime();

   offset= disk->getOffset();
   length= disk->getLength();

   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::next
//
// Function-
//       Select the next item
//
//----------------------------------------------------------------------------
const char*                         // The item name
   _tgzArchive::next( void )        // Select the next item
{
   if( disk == NULL )
     return gzip->next();

   const char* name= disk->next();
   if( name == NULL )
     this->name= "";
   else
     this->name= name;

   mode= disk->getMode();
   time= disk->getTime();

   offset= disk->getOffset();
   length= disk->getLength();

   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   _tgzArchive::read(               // Read from current item
     void*             addr,        // Into this buffer address
     unsigned int      size)        // For this length
{
   unsigned L= disk->read(addr, size);
   offset= disk->getOffset();
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tgzArchive::setOffset
//
// Function-
//       Set position within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   _tgzArchive::setOffset(          // Set position
     int64_t           offset)      // Offset
{
   int rc= disk->setOffset(offset);
   offset= disk->getOffset();
   return rc;
}

