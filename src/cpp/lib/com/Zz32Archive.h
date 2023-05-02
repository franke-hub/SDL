//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Zz32Archive.h
//
// Purpose-
//       Define and implement the Zz32Archive object.
//
// Last change date-
//       2020/10/02
//
// Implementation notes-
//       Included from Archive.cpp
//       Implemented for little-endian architecture ONLY.
//
//       Not supported:
//         Encryption (of any type)
//         Achives that span multiple files.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Zz32Archive
//
// Purpose-
//       ZIP (32) Archive
//
//----------------------------------------------------------------------------
class Zz32Archive : public Archive { // ZIP (32) Archive
protected: // INTERNAL STRUCTURES
enum
{  VERSION_ID= 63                   // THIS version-id (6.3)
}; // enum

enum                                // SELF-IDENTIFIERS
{  IDENT_PART= 0x04034B50           // PART identifier
,  IDENT_DATA= 0x08074B50           // DATA identifier (optional)
,  IDENT_CDFH= 0x02014B50           // CDFH identifier
,  IDENT_EOCD= 0x06054B50           // EOCD identifier
}; // enum

enum ATTR                           // (DOS) ATTRibute flags
{  ATTR_NONE=          0x0000       // No flag set (INVALID!)
,  ATTR_READONLY=      0x0001       // Read-only
,  ATTR_HIDDEN=        0x0002       // Hidden
,  ATTR_SYSTEM=        0x0004       // System
,  ATTR_VOLUME_LABEL=  0x0008       // Volume label
,  ATTR_ARCHIVE=       0x0020       // Archive(able)
,  ATTR_NORMAL=        0x0080       // Normal (no other attributes set)
,  ATTR_TEMPORARY=     0x0100       // Temporary (Delete when closed)
,  ATTR_OFFLINE=       0x1000       // Offline (moved to offline storage)
,  ATTR_NOT_INDEXED=   0x2000       // Not content indexed

////////////////////////////////////// Not settable using SetFileAttributes
,  ATTR_DIRECTORY=     0x0010       // Directory (Use CreateDirectory)
,  ATTR_DEVICE=        0x0040       // RESERVED: DO NOT USE!
,  ATTR_SPARSE_FILE=   0x0200       // Sparse file
,  ATTR_REPARSE_POINT= 0x0400       // Reparse point
,  ATTR_COMPRESSED=    0x0800       // COMPRESSED
,  ATTR_ENCRYPTED=     0x4000       // ENCRYPTED
}; // enum ATTR

enum FLAG                           // Flags
{  FLAG_NONE=          0x0000       // No flag set
,  FLAG_CRYPTO=        0x0001       // File is encrypted
,  FLAG_MODIFIER=      0x0006       // Method modifier
,  FLAG_DATA=          0x0008       // DATA follows compressed data
,  FLAG_FOR_METHOD8=   0x0010       // Reserved for use with METHOD8
,  FLAG_PATCH_DATA=    0x0020       // File is compressed patched data
,  FLAG_STRONG_CRYPTO= 0x0040       // File is strongly encrypted
,  FLAG_RESERVED7=     0x0080       // Currently unused
,  FLAG_RESERVED8=     0x0100       // Currently unused
,  FLAG_RESERVED9=     0x0200       // Currently unused
,  FLAG_RESERVED10=    0x0400       // Currently unused
,  FLAG_LANGUAGE_EFS=  0x0800       // Language Encoding Flag (UTF-8)
,  FLAG_RESERVED12=    0x1000       // Reserved by PKWARE (enhanced compression)
,  FLAG_CDFH_CRPYTO=   0x2000       // Central Directory encrypted
,  FLAG_RESERVED14=    0x4000       // Reserved by PKWARE
,  FLAG_RESERVED15=    0x8000       // Reserved by PKWARE
,  FLAG_ANY_CRYPTO=    0x2041       // Bitwise or of CRYPTO flags
}; // enum FLAG

enum MECH                           // Compression mechanism
{  MECH_NONE=          0            // File is stored, no compression used
,  MECH_SHRUNK=        1            // File is shrunk
,  MECH_REDUCED1=      2            // File is reduced, compression factor 1
,  MECH_REDUCED2=      3            // File is reduced, compression factor 2
,  MECH_REDUCED3=      4            // File is reduced, compression factor 3
,  MECH_REDUCED4=      5            // File is reduced, compression factor 4
,  MECH_IMPLODED=      6            // File is imploded
,  MECH_RESERVED7=     7            // Reserved for tokenizing algorithm
,  MECH_DEFLATE=       8            // File is deflated (standard)
,  MECH_DEFLATE64=     9            // File is deflated using Deflate64(tm)
,  MECH_OLD_TERSE=     10           // PKWARE Compression Library Imploding
,  MECH_RESERVED11=    11           // Reserved by PKWARE
,  MECH_BZIP2=         12           // File is compressed using BZIP2
,  MECH_RESERVED13=    13           // Reserved by PKWARE
,  MECH_LZMA=          14           // File is compressed using LZMA (EFS)
,  MECH_RESERVED15=    15           // Reserved by PKWARE
,  MECH_RESERVED16=    16           // Reserved by PKWARE
,  MECH_RESERVED17=    17           // Reserved by PKWARE
,  MECH_NEW_TERSE=     18           // Compressed using new IBM TERSE
,  MECH_IBM_LZ77=      19           // IBM LZ77 z Architecture (PFS)
,  MECH_WAVPACK=       97           // WavPack compressed data
,  MECH_PPMd_VIR1=     98           // PPMd version I, Rev 1
}; // enum MECH

#pragma pack(2)

struct PART {                       // PART descriptor (file header)
uint32_t               ident;       // Self-identifier (0x04034B50)
uint16_t               verNeed;     // Minimum version needed to decode
uint16_t               flags;       // See FLAG
uint16_t               mech;        // Compression mechanism
uint16_t               modTime;     // Modification time
uint16_t               modDate;     // Modification date
uint32_t               crc32;       // CRC-32
uint32_t               compSize;    // Compressed length
uint32_t               fullSize;    // Uncompressed length
uint16_t               nameSize;    // Length of file name string
uint16_t               xtraSize;    // Length of extra fields
////////////////////////////////////// Name field follows structure
}; // struct PART

struct DATA {                       // DATA descriptor
//                     ident;       // (Optional) self-identifier (0x08074B50)
uint32_t               crc32;       // CRC-32
uint32_t               compSize;    // Compressed length
uint32_t               fullSize;    // Uncompressed length
}; // struct DATA

struct CDFH {                       // Central Directory File Header
uint32_t               ident;       // Self-identifier (0x02014B50)
uint16_t               verMake;     // Version used to create archive
uint16_t               verNeed;     // Minium version needed to decode
uint16_t               flags;       // See FLAG
uint16_t               mech;        // Compression mechanism
uint16_t               modTime;     // Modification time
uint16_t               modDate;     // Modification date
uint32_t               crc32;       // CRC-32
uint32_t               compSize;    // Compressed length
uint32_t               fullSize;    // Uncompressed length
uint16_t               nameSize;    // Length of file name string
uint16_t               xtraSize;    // Length of extra name string
uint16_t               commSize;    // Length of comment field
uint16_t               diskS;       // Disk number where file starts
uint16_t               internal;    // Internal file attributes
uint32_t               external;    // External file attributes
uint32_t               offset;      // Relative offset of PART buffer
////////////////////////////////////// NAME field follows structure
////////////////////////////////////// EXTRA field follows NAME
////////////////////////////////////// COMMENT field follows EXTRA

// Methods used to address the name, extra, and comment strings
inline const char* name() const    { return (const char*)(this + 1); }
inline const char* extra() const   { return name() + nameSize; }
inline const char* comment() const { return extra() + xtraSize; }
}; // struct CDFH

//----------------------------------------------------------------------------
// The EOCD record is normally found at the end of the file. However, in order
// to find and read it, this header the comment length must be zero.
// (Otherwise it's just too much of a pain to find it. What if we find one but
// it's IN a comment field?) i.e., EOCD {comment containing EOCD {...}}
struct EOCD {                       // End Of Central Directory Header
uint32_t               ident;       // Self-identifier (0x06054B50)
uint16_t               diskN;       // This disk identifier
uint16_t               diskM;       // Main disk identifier (where directory starts)
uint16_t               cdfhDiskN;   // CDFH count, this disk
uint16_t               cdfhDiskM;   // CDFH count, all disks
uint32_t               cdfhLength;  // CDFH length
uint32_t               cdfhOffset;  // CDFH offset
uint16_t               commSize;    // Length of comment field
}; // struct EOCD

#pragma pack()

protected: // ATTRIBUTES
char                   nameBuffer[2048]; // Name buffer

void*                  blob;        // The CDFH data blob
CDFH**                 cdfh;        // The CDFH array (extracted from the blob)
EOCD                   eocd;        // The EOCD descriptor
PART                   part;        // The PART descriptor

public: // CONSTRUCTORS
virtual
   ~Zz32Archive( void );            // Destructor
   Zz32Archive(                     // Constructor
     DataSource*       file);       // DataSource

static Zz32Archive*                 // The Zz32Archive
   make(                            // Create Archive
     DataSource*       file);       // From this DataSource

public: // METHODS
virtual const char*                 // The object name (NULL if missing)
   index(                           // Select object number
     unsigned int      index);      // The object index

virtual const char*                 // The next object name
   next( void );                    // Skip to the next object

protected: // METHODS
void
   debug(                           // Debugging display of
     PART&             part);       // This PART structure

void
   debug(                           // Debugging display of
     CDFH&             cdfh);       // This CDFH structure

void
   debug(                           // Debugging display of
     EOCD&             cdfh);       // This EOCD structure

protected:
virtual int                         // Return code (0 OK)
   decompress(                      // Decompressor
     int               mode,        // Compression mode
     DataSource*       file,        // DataSource
     size_t            size);       // Compressed size
}; // class Zz32Archive

//----------------------------------------------------------------------------
//
// Method-
//       Zz32Archive::~Zz32Archive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Zz32Archive::~Zz32Archive( void )// Destructor
{
   if( blob != NULL )
   {
     free(blob);
     blob= NULL;
   }

   if( cdfh != NULL )
   {
     delete [] cdfh;
     cdfh= NULL;
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
//       Zz32Archive::Zz32Archive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Zz32Archive::Zz32Archive(        // Constructor
     DataSource*       file)        // DataSource
:  Archive()
,  blob(NULL)
,  cdfh(NULL)
{
   unsigned int        L;           // Read length
   int                 rc;          // Return code

   // Initialize
   nameBuffer[0]= '\0';             // No name or diagnostic message

   // Read the EOCD
   rc= file->setOffset(file->getLength() - sizeof(eocd));
   if( rc != 0 )
   {
     sprintf(nameBuffer, "%d= setOffset", rc);
     return;
   }

   L= file->read(&eocd, sizeof(eocd));
   if( L != sizeof(eocd) )
   {
     sprintf(nameBuffer, "%d= read(%d)", L, (int)sizeof(EOCD));
     return;
   }

   // Verify the EOCD
   if( eocd.ident != IDENT_EOCD )
   {
     sprintf(nameBuffer, "EOCD ident(%x)", eocd.ident);
     return;
   }

   if( eocd.commSize != 0 )
   {
     sprintf(nameBuffer, "EOCD commSize(%d)", eocd.commSize);
     return;
   }

   if( eocd.diskN != eocd.diskM )
   {
     sprintf(nameBuffer, "EOCD diskN(%d) diskM(%d)", eocd.diskN, eocd.diskM);
     return;
   }

   if( eocd.cdfhOffset >= file->getLength()
       || eocd.cdfhLength >= file->getLength()
       || (eocd.cdfhOffset+eocd.cdfhLength+sizeof(EOCD)) > file->getLength() )
   {
     sprintf(nameBuffer, "EOCD size(%" PRId64 ") offset(%d) length(%d)",
             file->getLength(), eocd.cdfhOffset, eocd.cdfhLength);
     return;
   }

   // Allocate the CDFH blob and index array
   blob= malloc(eocd.cdfhLength);   // Allocate the cdfh BLOB
   cdfh= new CDFH*[eocd.cdfhDiskN]; // Allocate the cdfh array
   if( blob == NULL || cdfh == NULL )
   {
     sprintf(nameBuffer, "EOCD blob(%p).%d cdfh(%p).%d No storage",
             blob, eocd.cdfhLength, cdfh, eocd.cdfhDiskN);
     return;
   }

   rc= file->setOffset(eocd.cdfhOffset);
   if( rc != 0 )
   {
     sprintf(nameBuffer, "%d= setOffset", rc);
     return;
   }

   L= file->read(blob, eocd.cdfhLength);
   if( L != eocd.cdfhLength )
   {
     sprintf(nameBuffer, "%d= read(%d)", L, eocd.cdfhLength);
     return;
   }

   // Load the CDFH index array
   char* C= (char*)blob;            // Working cdfh BLOB pointer
   uint32_t offset= 0;              // Working offset
   for(int i= 0; i<eocd.cdfhDiskN; i++) // Set up cdfh array
   {
     CDFH* cdfh= (CDFH*)(C + offset); // Set current element
     uint32_t length= sizeof(CDFH) + cdfh->nameSize
                    + cdfh->xtraSize + cdfh->commSize;
     if( (offset+length) > eocd.cdfhLength )
     {
       sprintf(nameBuffer, "[%d] CDFH size(%d) offset(%d) length(%d)",
               i, eocd.cdfhLength, offset, length);
       return;
     }

     if( cdfh->ident != IDENT_CDFH )
     {
       sprintf(nameBuffer, "[%d] CDFH signature(%x)", i, cdfh->ident);
       return;
     }

     if( VERSION_ID < cdfh->verNeed )
     {
       sprintf(nameBuffer, "[%d] CDFH verNeed(%d) VERSION_ID(%d)", i,
               cdfh->verNeed, VERSION_ID);
       return;
     }

     if( (cdfh->flags & FLAG_ANY_CRYPTO) != 0 )
     {
       sprintf(nameBuffer, "[%d] CDFH flags(%x) ENCRYPTED", i, cdfh->flags);
       return;
     }

     if( cdfh->mech != MECH_DEFLATE && cdfh->mech != MECH_NONE )
     {
       sprintf(nameBuffer, "[%d] CDFH mech(%d) NOT SUPPORTED", i, cdfh->mech);
       return;
     }

     this->cdfh[i]= cdfh;
     offset += length;
   }

   // OPERATIONAL, take over control of file
   this->file= file;                // (Deleted by ~Archive)

   // Initialize for first object
   if( index(0) == NULL )
   {
     sprintf(nameBuffer, "index[0] NULL");
     return;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz32Archive::make
//
// Purpose-
//       Allocate and initialize a Zz32Archive.
//
//----------------------------------------------------------------------------
Zz32Archive*                        // Resultant Archive
   Zz32Archive::make(               // Allocate and initialize a Zz32Archive
     DataSource*       file)        // From this DataSource
{
   Zz32Archive*        result= NULL; // Resultant Archive

   try {
     result= new Zz32Archive(file); // Allocate resultant
     if( result->file == NULL )     // If failure
     {
       if( stricmp(".zip", FileName::getExtension(file->getCName())) == 0 )
         fprintf(stderr, "File(%s) ERROR: %s\n",
                 file->getCName(), result->nameBuffer);

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
//       Zz32Archive::index
//
// Function-
//       Select an object by index
//
//----------------------------------------------------------------------------
const char*                         // The object name
   Zz32Archive::index(              // Select
     unsigned int      object)      // This object object
{
// IFHCDM( debugf("Zz32Archive(%p)::index(%d)\n", this, object); )

   // Reset the current data block
   if( origin != NULL )
   {
     free(origin);
     origin= NULL;
   }

   name= "";
   offset= 0;
   length= 0;

   // Update the index
   this->object= object;
   if( object >= eocd.cdfhDiskN )
     return NULL;

   CDFH* cdfh= this->cdfh[object];  // Address the current CDFH
   if( cdfh->nameSize >= (sizeof(nameBuffer)-1) )
   {
     memcpy(nameBuffer, cdfh->name(), sizeof(nameBuffer)-1);
     nameBuffer[sizeof(nameBuffer)-1]= '\0';
     debugf("Zz32Archive::index(%d) name(%s) TOO LONG\n", object, nameBuffer);
   }
   else
   {
     memcpy(nameBuffer, cdfh->name(), cdfh->nameSize);
     nameBuffer[cdfh->nameSize]= '\0';
   }
   name= nameBuffer;

   // Integrity check: Disallow "/../" name qualifiers
   if( strstr(nameBuffer, "/../") != NULL )
   {
     debugf("Zz32Archive::index(%d) name(%s)", object, nameBuffer);
     return NULL;
   }

   switch( cdfh->verMake >> 8 )
   {
     case 0:                        // MS-DOS and OS/2
       #ifdef _OS_WIN
         mode= cdfh->external;
       #else
         if( cdfh->external == ATTR_DIRECTORY )
           mode= 0x000041ED;        // PATH: rwx r-x r-x
         else
           mode= 0x000081A4;        // FILE: rw- r-- r--
       #endif
       break;

     case 3:                        // UNIX
       #ifdef _OS_WIN
         mode= cdfh->external & 0x0000ffff;
       #else
         mode= cdfh->external >> 16;
       #endif
       break;

////////////////////////////////////// Supported source systems
//   case 1:                        // Amiga
//   case 2:                        // OpenVMS
//   case 4:                        // VM/CMS
//   case 5:                        // Atari ST
//   case 6:                        // OS/2 HPFS
//   case 7:                        // Macintosh
//   case 8:                        // Z-System
//   case 9:                        // CP/M
//   case 10:                       // Windows NTFS
//   case 11:                       // MVS (OS/390 S/OS)
//   case 12:                       // VSE
//   case 13:                       // Acorn Risc
//   case 14:                       // VFAT
//   case 15:                       // Alternate MVS
//   case 16:                       // BeOS
//   case 17:                       // Tandem
//   case 18:                       // OS/400
//   case 19:                       // OS/X (Darwin)
     default:                       // Undefined
       #ifdef _OS_WIN
         mode= ATTR_NORMAL;
         if( cdfh->nameSize > 0 && name[cdfh->nameSize-1] == '/' )
           mode= ATTR_DIRECTORY;
       #else
         mode= 0x000081FF;          // FILE: rwx rwx rwx
         if( cdfh->nameSize > 0 && name[cdfh->nameSize-1] == '/' )
           mode= 0x000041FF;        // PATH: rwx rwx rwx
       #endif
       break;
   }

   time= FAT_DATE_TIME(cdfh->modDate, cdfh->modTime);
// debugf("x(%.8x) ", mode);
// debugf("index(%d) file(%s) mode(%.8x) time(%8ld)\n", object, name, mode, (long)time);
// debug(*cdfh);

   // Load and verify the file header
   file->setOffset(cdfh->offset);
   unsigned int L= file->read(&part, sizeof(part));
   if( L != sizeof(part) )
   {
     debugf("%4d Zz32Archive, %u= read(%u)\n", __LINE__, L, (int)sizeof(part));
     return NULL;
   }

   if( (part.flags&FLAG_DATA) != 0 ) // If CRC was not known
     part.crc32= cdfh->crc32;        // Use the known value

   if( part.ident != IDENT_PART
       || part.mech     != cdfh->mech
       || part.modTime  != cdfh->modTime
       || part.modDate  != cdfh->modDate
       || part.crc32    != cdfh->crc32
       || part.compSize != cdfh->compSize
       || part.fullSize != cdfh->fullSize
       || part.nameSize != cdfh->nameSize ) // xtraSize may differ
   {
     debugf("%4d Zz32Archive, CDFH/FILE mismatch\n", __LINE__);

     debug(*cdfh);
     debug(part);

     return NULL;
   }

   // Skip over the name and extra fields
   file->setOffset(file->getOffset() + part.nameSize + part.xtraSize);

   // Initialize new data blob
   length= cdfh->fullSize;
   if( length == 0 )
   {
     if( cdfh->crc32 != 0 )
     {
       debugf("%4d Zz32Archive crc32(%.8x), but LENGTH(0)\n", __LINE__,
              cdfh->crc32);
       return NULL;
     }
   }
   else
   {
     // Allocate the resultant data area
     origin= (unsigned char*)malloc(length);
     if( origin == NULL )
     {
       debugf("%4d Zz32Archive NULL= malloc(%" PRId64 ")\n", __LINE__, length);
       return NULL;
     }

     // Decompress it
     int rc= decompress(cdfh->mech, file, cdfh->compSize);
     if( rc != 0 )
     {
       debugf("%4d Zz32Archive %d= decompress(%s)\n", __LINE__, rc, nameBuffer);
       return NULL;
     }

     CRC32 crc32;
     crc32.accumulate(origin, (long)length);
     if( crc32.getValue() != cdfh->crc32 )
     {
       debugf("%4d Zz32Archive crc32(%.8x), expected(%.8x)\n", __LINE__,
              crc32.getValue(), cdfh->crc32);
       return NULL;
     }
   }

   return nameBuffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz32Archive::next
//
// Function-
//       Select the next object
//
//----------------------------------------------------------------------------
const char*                         // The object name
   Zz32Archive::next( void )        // Select the next object
{
   object++;
   return index(object);
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz32Archive::decompress
//
// Purpose-
//       ZLIB decompressor.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Zz32Archive::decompress(         // Decompressor
     int               mode,        // Compression mode
     DataSource*       file,        // DataSource
     size_t            size)        // Compressed Archive length
{
   Bytef*              inp;         // Compressed input buffer
   unsigned long       offset;      // Current offset
   z_stream            stream;      // ZLIB stream
   int                 zrc;         // Working return code

// IFHCDM( debugf("Archive(%p)::decompress(%d,%s,%" PRId64 ")\n", this,
//         mode, getName(), size);
// )

   zrc= Z_DATA_ERROR;               // Default, data error
   switch( mode )
   {
     case COMP_NONE:
       if( length != size )         // If invald input length
         break;

       offset= file->read(origin, length);
       if( offset != length )
         break;

       zrc= Z_OK;
       break;

     case COMP_ZLIB:
       inp= (Bytef*)malloc(length); // Allocate compressed buffer
       if( inp == NULL )
         return Z_MEM_ERROR;

       // Initialize ZLIB decompressor
       memset(&stream, 0, sizeof(stream)); // Initialize stream
       stream.zalloc=  Z_NULL;
       stream.zfree=   Z_NULL;
       stream.opaque=  Z_NULL;
       stream.next_in= inp;
       stream.avail_in= file->read(inp, size);
       if( stream.avail_in != size )
       {
         free(inp);
         return Z_DATA_ERROR;
       }

       stream.next_out= (Bytef*)origin;
       stream.avail_out= length;

       //---------------------------------------------------------------------
       // Run ZLIB decompressor
       //---------------------------------------------------------------------
       zrc= inflateInit2(&stream, -MAX_WBITS);
       if( zrc == Z_OK )
       {
         zrc= inflate(&stream, Z_NO_FLUSH);
         if( zrc == Z_STREAM_END )
           zrc= Z_OK;

         inflateEnd(&stream);
       }

       free(inp);
       break;

     default:
       break;
   }

   //-------------------------------------------------------------------------
   // ZLIB decompressor complete
   //-------------------------------------------------------------------------
   return zrc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Zz32Archive::debug
//
// Function-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Zz32Archive::debug(              // Debugging display
     PART&             part)        // Of this PART
{
   debugf("Zz32Acrhive(%p)::debugPART(%p)\n", this, &part);

   debugf(">>>>ident(%.8x)\n", part.ident);
   debugf("..verNeed(%4d) 0x%.4x\n", part.verNeed, part.verNeed);
   debugf("....flags(%.4x)\n", part.flags);
   debugf(".....mech(%.4x)\n", part.mech);
   debugf("..modTime(%6d) 0x%.4x\n", part.modTime, part.modTime);
   debugf("..modDate(%6d) 0x%.4x\n", part.modDate, part.modDate);
   debugf("....crc32(%.8x)\n", part.crc32);
   debugf(".compSize(%6d)\n", part.compSize);
   debugf(".fullSize(%6d)\n", part.fullSize);
   debugf(".nameSize(%6d)\n", part.nameSize);
   debugf(".xtraSize(%6d)\n", part.xtraSize);
}

void
   Zz32Archive::debug(              // Debugging display
     CDFH&             cdfh)        // Of this CDFH structure
{
   debugf("Zz32Archive(%p)::debugCDFH(%p)\n", this, &cdfh);

   debugf(">>>ident(%.8x)\n", cdfh.ident);
   debugf(".verMake(%4d) 0x%.4x\n", cdfh.verMake, cdfh.verMake);
   debugf(".verNeed(%4d) 0x%.4x\n", cdfh.verNeed, cdfh.verNeed);
   debugf("...flags(%.4x)\n", cdfh.flags);
   debugf("....mech(%.4x)\n", cdfh.mech);
   debugf(".modTime(%6d) 0x%.4x\n", cdfh.modTime, cdfh.modTime);
   debugf(".modDate(%6d) 0x%.4x\n", cdfh.modDate, cdfh.modDate);
   debugf("...crc32(%.8x)\n", cdfh.crc32);
   debugf("compSize(%6d)\n", cdfh.compSize);
   debugf("fullSize(%6d)\n", cdfh.fullSize);
   debugf("nameSize(%6d)\n", cdfh.nameSize);
   debugf("xtraSize(%6d)\n", cdfh.xtraSize);
   debugf("commSize(%6d)\n", cdfh.commSize);
   debugf("internal(%.4x)\n", cdfh.internal);
   debugf("external(%.8x)\n", cdfh.external); // ZERO if input from stdin
   debugf("  offset(%.8x)\n", cdfh.offset);
// EXTERNAL::UNIX
// 41ed0010 FOLDER/   drwxr-xr-x+    0100 0001 1110 1101 0000 0000 0001 0000
// 81a40000 FILE      -rw-r--r--+    1000 0001 1010 0100 0000 0000 0000 0000
//                                   MASK ---r wxrw xrwx
//                <   > <   ><   ><   >
// _IFDIR   000 000 100 000 000 000 000
// _IFREG   000 001 000 000 000 000 000
// _IFLNK   000 001 010 000 000 000 000
// _IMASK   000 001 111 000 000 000 000
// OWNER                    rwx         owner
// GROUP                        rwx     group
// OTHER                            rwx other
// S_ISUID              1               Set USERID on execution
// S_ISGID               1              Set GROUPID on execution
// S_IXVTX                1             Save swapped text, even after use
//
// EXTERNAL::DOS
//  (Low order byte is the MS_DOS directory attribute byte.)
//
}

void
   Zz32Archive::debug(              // Debugging display
     EOCD&             eocd)        // Of this EOCD structure
{
   debugf("Zz32Archive(%p)::debugEOCD(%p)\n", this, &eocd);

   debugf(".....ident(%.8x)\n", eocd.ident);
   debugf(".....diskN(%8d)\n", eocd.diskN);
   debugf(".....diskM(%8d)\n", eocd.diskM);
   debugf(".cdfhDiskN(%8d)\n", eocd.cdfhDiskN);
   debugf(".cdfhDiskM(%8d)\n", eocd.cdfhDiskM);
   debugf("cdfhLength(%8d)\n", eocd.cdfhLength);
   debugf("cdfhOffset(%8d)\n", eocd.cdfhOffset);
   debugf("..commSize(%8d)\n", eocd.commSize);
}

