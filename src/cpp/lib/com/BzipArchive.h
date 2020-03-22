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
//       BzipArchive.h
//
// Purpose-
//       Define and implement the BzipArchive object.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Included from Archive.cpp
//       Only files with bzip-type extensions are supported.
//
//----------------------------------------------------------------------------
#include <bzlib.h>                  // BZLIB header

//----------------------------------------------------------------------------
//
// Subroutine-
//       fromHiLo
//
// Purpose-
//       Convert High/Low integers into 64 bit values
//
//----------------------------------------------------------------------------
static inline int64_t               // Resultant
   fromHi(                          // Convert to 64-bit integer
     int32_t           hi,          // The high-order value
     int32_t           lo)          // The low-order value
{
   int64_t result= hi;              // Get high-order value
   result <<= 32;                   // Reposition it properly
   result |= lo;                    // Append low-order value

   return result;
}

//----------------------------------------------------------------------------
//
// Class-
//       BzipArchive
//
// Purpose-
//       BZIP Archive
//
//----------------------------------------------------------------------------
class BzipArchive : public Archive { // BZIP Archive
protected: // INTERNAL STRUCTURES
struct HEAD {                       // BZIP file heading
char                   ident[2];    // "BZ" signature
char                   version;     // Version ('h')
char                   versize;     // Blocksize ('1' .. '9')
}; // struct HEAD

protected: // ATTRIBUTES
enum
{  CHUNK= 131072                    // Input buffer CHUNK size
}; // enum

int                    isValid;     // TRUE iff stream initialized
bz_stream              stream;      // The decode stream
char                   nameBuffer[2048]; // The file name buffer

public: // CONSTRUCTORS
virtual
   ~BzipArchive( void );            // Destructor
   BzipArchive(                     // Constructor
     DataSource*       file);       // DataSource

static BzipArchive*                 // The BzipArchive
   make(                            // Create Archive
     DataSource*       file);       // From this DataSource

public: // METHODS
virtual const char*                 // The object name (NULL if missing)
   index(                           // Select object number
     unsigned int      index);      // The object index

virtual const char*                 // The next object name
   next( void );                    // Skip to the next object

virtual unsigned int                // Number of bytes read
   read(                            // Read (from current item)
     void*             addr,        // Input buffer address
     unsigned int      size);       // Input buffer length

virtual int                         // Return code (0 OK)
   setOffset(                       // Position within current item
     int64_t           offset);     // Offset

//----------------------------------------------------------------------------
// The resetFile method is not externally visible.
//----------------------------------------------------------------------------
inline DataSource*
   resetFile( void )
{
   DataSource* result= file;
   file= NULL;
   return result;
}
}; // class BzipArchive

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::~BzipArchive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   BzipArchive::~BzipArchive( void )// Destructor
{
   if( isValid )                    // If inflate active
   {
     isValid= FALSE;
     BZ2_bzDecompressEnd(&stream);
   }

   if( origin != NULL )
   {
     free(origin);
     origin= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::BzipArchive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   BzipArchive::BzipArchive(        // Constructor
     DataSource*       file)        // Source FileArchive
:  Archive()
,  isValid(FALSE)
{
   // Initialize
   nameBuffer[0]= '\0';             // No name or diagnostic message
   #ifdef _OS_WIN
     mode= MODE_NORMAL;             // FILE: NORMAL
   #else
     mode= 0x000081A4;              // FILE: rw- r-- r--
   #endif

   // Convert the file name
   FileName info(file->getName().c_str()); // Examine the file name
   const char* type= info.getExtension(); // Get the extension
   if( stricmp(type, ".bz2") == 0 || stricmp(type, ".bz") == 0 )
   {
     const char* name= info.getNameOnly();
     if( strlen(name) >= sizeof(nameBuffer) )
     {
       sprintf(nameBuffer, "Name too long\n");
       return;
     }

     strcpy(nameBuffer, name);
   }
   else if( stricmp(type, ".tbz2") == 0 || stricmp(type, ".tbz") == 0 )
   {
     const char* name= info.getNameOnly();
     if( strlen(name) >= (sizeof(nameBuffer) - 4) )
     {
       sprintf(nameBuffer, "Name too long\n");
       return;
     }

     strcpy(nameBuffer, name);
     strcat(nameBuffer, ".tar");
   }
   this->name= nameBuffer;

   // Load the header
   origin= (unsigned char*)malloc(CHUNK); // Our input data chunk
   if( origin == NULL )
   {
     sprintf(nameBuffer, "No Storage\n");
     return;
   }

   file->setOffset(0);              // Begin at the beginning
   unsigned L= file->read(origin, CHUNK); // Read first chunk
   if( L < sizeof(HEAD) )
   {
     sprintf(nameBuffer, "Missing header\n");
     return;
   }

   // Validate the header
   HEAD* head= (HEAD*)origin;
   if( head->ident[0] != 'B' || head->ident[1] != 'Z' || head->version != 'h'
       || head->versize < '1' || head->versize > '9' )
   {
     sprintf(nameBuffer, "Invalid id: %c%c%c%c\n", head->ident[0],
             head->ident[1], head->version, head->versize);
     return;
   }

   // Header accepted
   this->file= file;                // (Required by index)
   if( index(0) == NULL )
     this->file= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::make
//
// Purpose-
//       Allocate and initialize a BzipArchive.
//
//----------------------------------------------------------------------------
BzipArchive*                        // Resultant Archive
   BzipArchive::make(               // Allocate and initialize a BzipArchive
     DataSource*       file)        // The DataSource
{
// IFHCDM( debugf("BzipArchive::make(%s)\n", file->getName().c_str()); )

   BzipArchive* result= NULL;       // Resultant Archive
   try {
     result= new BzipArchive(file); // Allocate resultant
     if( result->file == NULL )     // If failure
     {
       if( stricmp(".gz", FileName::getExtension(file->getName().c_str())) == 0 )
         fprintf(stderr, "File(%s) ERROR: %s\n",
                 file->getName().c_str(), result->nameBuffer);

       delete result;
       result= NULL;
     }
   } catch(...) {
     if( result != NULL )
       delete result;

     result= NULL;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::index
//
// Function-
//       Select an object by index
//
//----------------------------------------------------------------------------
const char*                         // The object name
   BzipArchive::index(              // Select
     unsigned int      object)      // This object object
{
// IFHCDM( debugf("BzipArchive(%p)::index(%d)\n", this, object); )

   // Reset the inflate stream
   if( isValid )
   {
     isValid= FALSE;
     BZ2_bzDecompressEnd(&stream);
   }
   memset(&stream, 0, sizeof(stream));

   this->object= object;
   offset= 0;
   length= 0;                       // (Unknown length)

   if( object != 0 )
     return NULL;

   // Initialize decoding
   file->setOffset(0);              // (Re)position the file

   stream.bzalloc= Z_NULL;
   stream.bzfree=  Z_NULL;
   stream.opaque=  Z_NULL;
   stream.next_in= (char*)origin;
   stream.avail_in= file->read(origin, CHUNK);

   stream.next_out= Z_NULL;
   stream.avail_out= 0;

   //-------------------------------------------------------------------------
   // Initialize the ZLIB decompressor (With decoding)
   //-------------------------------------------------------------------------
   int zrc= BZ2_bzDecompressInit(&stream, 0, 0); // MIN verbosity, LARGE
   if( zrc != BZ_OK )
   {
     sprintf(nameBuffer, "bzDecompressInit error(%d)\n", zrc);
     return NULL;
   }

   isValid= TRUE;
   return name.c_str();
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::next
//
// Function-
//       Select the next object
//
//----------------------------------------------------------------------------
const char*                         // The object name
   BzipArchive::next( void )        // Select the next object
{
   return index(object + 1);
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   BzipArchive::read(               // Read from current item
     void*             addr,        // Into this buffer address
     unsigned int      size)        // For this length
{
// IFHCDM( debugf("BzipArchive(%p)::read(%p,%d)\n", this, addr, size); )

   unsigned L= 0;                   // Number of bytes read
   if( isValid )
   {
     if( stream.avail_in == 0 )     // If read required
     {
       stream.next_in= (char*)origin;
       stream.avail_in= file->read(origin, CHUNK);
       if( stream.avail_in == 0 )
         throwf("Bzip(%s) decode error", getCName());
     }

     stream.next_out= (char*)addr;
     stream.avail_out= size;
     if( size > 0 )
     {
       int zrc= BZ2_bzDecompress(&stream);
       L= size - stream.avail_out;
       if( zrc == BZ_STREAM_END )
       {
         isValid= FALSE;
         BZ2_bzDecompressEnd(&stream);
       }
       else if( zrc != Z_OK )
         throwf("Bzip(%s) decode error(%d)", getCName(), zrc);

       offset += L;
     }
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       BzipArchive::setOffset
//
// Function-
//       Set position within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   BzipArchive::setOffset(          // Set position
     int64_t           offset)      // Offset
{
   int rc= (-1);                    // Return code

   if( offset >= 0 )                // If valid offset
   {
     if( offset < this->offset )
       index(0);                    // Reposition to offset 0
     else
       offset -= this->offset;      // Relative to current

     // Lame way to seek forward
     char buffer[512];
     while( offset >= sizeof(buffer) )
     {
       int L= read(buffer, sizeof(buffer));
       if( L == 0 )
       {
         debugf("BzipArchive seek past EOF\n");
         break;
       }

       offset -= L;
     }

     if( offset > 0 )
     {
       int L= read(buffer, offset);
       if( L != offset )
       {
         debugf("BzipArchive seek past EOF\n");
         return (-1);
       }
     }

     rc= 0;                       // Must reach here for success
   }

   return rc;
}

