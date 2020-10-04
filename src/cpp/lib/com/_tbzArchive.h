//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       _tbzArchive.h
//
// Purpose-
//       Define and implement the _tbzArchive object.
//
// Last change date-
//       2020/10/02
//
// Implementation notes-
//       Included from Archive.cpp
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       _tbzArchive
//
// Purpose-
//       Bzip'ed TAR format Archive
//
//----------------------------------------------------------------------------
class _tbzArchive : public Archive { // TBZ Archive
protected: // ATTRIBUTES
BzipArchive*           bzip;        // The BZIP Archive
DiskArchive*           disk;        // The TAR  Archive

public: // CONSTRUCTORS
virtual
   ~_tbzArchive( void );            // Destructor
   _tbzArchive(                     // Constructor
     DataSource*       file);       // DataSource

public:
static _tbzArchive*                 // Resultant _tpzArchive (NULL if none)
   make(                            // Create _tbzArchive
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
     size_t            offset);     // Offset
}; // class _tbzArchive

//----------------------------------------------------------------------------
//
// Method-
//       _tbzArchive::~_tbzArchive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   _tbzArchive::~_tbzArchive( void ) // Destructor
{
   // The bzip file is owned by the DiskArchive file. (Don't delete it here.)
   bzip= NULL;

   if( disk != NULL )
   {
     delete disk;
     disk= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _tbzArchive::_tbzArchive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   _tbzArchive::_tbzArchive(        // Constructor
     DataSource*       file)        // DataSource
:  Archive()
{
   // Create/validate the BZIP Archive
   bzip= BzipArchive::make(file);   // Create the BZIP Archive

   if( bzip != NULL )
     disk= DiskArchive::make(bzip); // Create the TAR Archive

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
//       _tbzArchive::make
//
// Purpose-
//       Allocate and initialize a _tbzArchive.
//
//----------------------------------------------------------------------------
_tbzArchive*                        // Resultant
   _tbzArchive::make(               // Allocate and initialize a _tbzArchive
     DataSource*       file)        // Source Archive
{
   _tbzArchive*        result= NULL; // Resultant Archive

   try {
     result= new _tbzArchive(file); // Allocate resultant
     if( result->disk == NULL )     // If failure
     {
       // Only called for valid file name extensions
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
//       _tbzArchive::take
//
// Purpose-
//       Return the DataSource
//
//----------------------------------------------------------------------------
DataSource*                         // Resultant
   _tbzArchive::take( void )        // Return the DataSource
{
   DataSource* result= bzip->resetFile();

   delete this;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       _tbzArchive::index
//
// Function-
//       Select an item by index
//
//----------------------------------------------------------------------------
const char*                         // The item name
   _tbzArchive::index(              // Select
     unsigned int      index)       // This item index
{
   if( disk == NULL )
     return bzip->index(index);

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
//       _tbzArchive::next
//
// Function-
//       Select the next item
//
//----------------------------------------------------------------------------
const char*                         // The item name
   _tbzArchive::next( void )        // Select the next item
{
   if( disk == NULL )
     return bzip->next();

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
//       _tbzArchive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   _tbzArchive::read(               // Read from current item
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
//       _tbzArchive::setOffset
//
// Function-
//       Set position within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   _tbzArchive::setOffset(          // Set position
     size_t            offset)      // Offset
{
   int rc= disk->setOffset(offset);
   offset= disk->getOffset();
   return rc;
}

