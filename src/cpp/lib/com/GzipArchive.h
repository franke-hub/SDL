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
//       GzipArchive.h
//
// Purpose-
//       Define and implement the GzipArchive object.
//
// Last change date-
//       2020/10/02
//
// Implementation notes-
//       Included from Archive.cpp
//       Implemented for little-endian architecture ONLY.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       GzipArchive
//
// Purpose-
//       GZIP Archive
//
//----------------------------------------------------------------------------
class GzipArchive : public Archive { // GZIP Archive
protected: // INTERNAL STRUCTURES
enum
{  VERSION_ID= 43                   // THIS version-id (4.3)
,  CHUNK= 131072                    // Input buffer CHUNK size
}; // enum

enum IDENT
{  IDENT_1=            0x1f         // HEAD::id1 identifier
,  IDENT_2=            0x8b         // HEAD::id2 identifier
}; // enum IDENT

enum FLAG                           // Flags
{  FLAG_NONE=          0x00         // No flag set
,  FLAG_TEXT=          0x01         // File is probably ASCII text
,  FLAG_HAS_CRC16=     0x02         // CRC16 follows (complete) header
,  FLAG_HAS_EXTRA=     0x04         // EXTRA field follows structure
,  FLAG_HAS_NAME=      0x08         // NAME  field follows EXTRA field
,  FLAG_HAS_COMMENT=   0x10         // COMMENT field follows NAME field
,  FLAG_RESERVED=      0xE0         // Reserved for expansion
}; // enum FLAG

enum MECH                           // Compression mechanism
{  MECH_NONE=          0            // File is stored, no compression used
,  MECH_RESERVED1=     1            // Reserved
,  MECH_RESERVED2=     2            // Reserved
,  MECH_RESERVED3=     3            // Reserved
,  MECH_RESERVED4=     4            // Reserved
,  MECH_RESERVED5=     5            // Reserved
,  MECH_RESERVED6=     6            // Reserved
,  MECH_RESERVED7=     7            // Reserved
,  MECH_DEFLATE=       8            // DEFLATE compression used (standard)
}; // enum MECH

enum MODE                           // (DOS) Mode flags
{  MODE_NORMAL=        0x0080       // Normal (no other attributes set)
}; // enum MODE

#pragma pack(2)

struct HEAD {                       // GZIP file header
uint8_t                id1;         // Self-identifier (0x1f)
uint8_t                id2;         // Self-identifier (0x8b)
uint8_t                mech;        // Compression mechanism
uint8_t                flag;        // Flags
uint32_t               modTime;     // Modification time
uint8_t                fm;          // Flag modifiers
uint8_t                os;          // Flag modifiers
////////////////////////////////////// uint16_t EXTRA field length (optional)
////////////////////////////////////// Extra field (optional)
////////////////////////////////////// Zero terminated name (optional)
////////////////////////////////////// Zero terminated comment (optional)
////////////////////////////////////// uint16_t CRC16 (optional)
////////////////////////////////////// ... compressed blocks ...
////////////////////////////////////// uint32_t CRC32
////////////////////////////////////// uint32_t ISIZE (uncompressed_size % 2^32)
unsigned int getExtraSize( void );  // Get extra field length
void*        getExtraAddr( void );  // Get extra field address
const char*  getNameAddr( void );   // Get name field address
const char*  getCommentAddr( void );// Get comment field address
int          verifyCRC16( void );   // Verify the heading CRC16
unsigned int getOptionSize( void ); // Get the length of all the optional fields
}; // struct HEAD

#pragma pack()

protected: // ATTRIBUTES
char                   nameBuffer[2048]; // Name buffer

HEAD                   head;        // Heading (without optional fields)

int                    isValid;     // TRUE iff stream initialized
z_stream               stream;      // The decode stream

public: // CONSTRUCTORS
virtual
   ~GzipArchive( void );            // Destructor
   GzipArchive(                     // Constructor
     DataSource*       file);       // DataSource

static GzipArchive*                 // The GzipArchive
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
     size_t            offset);     // Offset

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

protected: // METHODS
void
   debug(                           // Debugging display of
     HEAD&             head,        // Of this HEAD structure
     int               full= FALSE);// TRUE iff extended header present
}; // class GzipArchive

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::HEAD
//
// Purpose-
//       Methods defined in GzipArchive::HEAD
//
//----------------------------------------------------------------------------
unsigned int
   GzipArchive::HEAD::getExtraSize( void ) // Get size of EXTRA field
{
   int result= 0;

   if( (flag & FLAG_HAS_EXTRA) != 0 )
   {
     uint16_t* length= (uint16_t*)(this + 1);
     result= *length;
   }

   return result;
}

void*
   GzipArchive::HEAD::getExtraAddr( void ) // Get address of EXTRA field
{
   void* result= NULL;

   if( (flag & FLAG_HAS_EXTRA) != 0 )
   {
     uint16_t* length= (uint16_t*)(this + 1);
     result= (void*)(length + 1);
   }

   return result;
}

const char*
   GzipArchive::HEAD::getNameAddr( void ) // Get address of NAME field
{
   const char*         addr= (const char*)(this + 1); // Option address base
   unsigned int        size= 0;             // Working length

   if( (flag & FLAG_HAS_EXTRA) != 0 )
   {
     uint16_t* length= (uint16_t*)(addr);
     size += (sizeof(uint16_t) + *length);
   }

   const char* result= NULL;
   if( (flag & FLAG_HAS_NAME) != 0 )
     result= addr + size;

   return result;
}

const char*
   GzipArchive::HEAD::getCommentAddr( void ) // Get address of COMMENT field
{
   const char* result= NULL;

   if( (flag & FLAG_HAS_COMMENT) != 0 )
   {
     result= (const char*)(this + 1); // Pointing at EXTRA/NAME field

     if( (flag & FLAG_HAS_EXTRA) != 0 )
     {
       uint16_t* length= (uint16_t*)(this + 1);

       result += sizeof(uint16_t);
       result += *length;
     }

     if( (flag & FLAG_HAS_NAME) != 0 )
     {
       result += strlen(result);    // Pointing at NAME delimiter
       result++;                    // Skip NAME delimiter
     }
   }

   return result;
}

int
   GzipArchive::HEAD::verifyCRC16( void ) // Verify the header's CRC16
{
   const char*         addr= (const char*)(this + 1); // Option address base
   unsigned int        size= 0;             // Working length

   if( (flag & FLAG_HAS_EXTRA) != 0 )
   {
     uint16_t* length= (uint16_t*)(addr);
     size += (sizeof(uint16_t) + *length);
   }

   if( (flag & FLAG_HAS_NAME) != 0 )
   {
     const char* temp= addr + size;
     size += (strlen(temp) + 1);
   }

   if( (flag & FLAG_HAS_COMMENT) != 0 )
   {
     const char* temp= addr + size;
     size += (strlen(temp) + 1);
   }

   int result= TRUE;
   if( (flag & FLAG_HAS_CRC16) != 0 )
   {
     CRC32 crc32;
     crc32.accumulate(addr, size);

     uint16_t* value= (uint16_t*)(addr + size);
debugf("CRC16: WANT(%.4x) GOT(%.4x)\n", *value, crc32.getValue()&0x0000ffff);
   }

   return result;
}

unsigned int
   GzipArchive::HEAD::getOptionSize( void ) // Get total option length
{
   const char*         addr= (const char*)(this + 1); // Option address base
   unsigned int        size= 0;             // Working length

   if( (flag & FLAG_HAS_EXTRA) != 0 )
   {
     uint16_t* length= (uint16_t*)(addr);
     size += (sizeof(uint16_t) + *length);
   }

   if( (flag & FLAG_HAS_NAME) != 0 )
   {
     const char* temp= addr + size;
     size += (strlen(temp) + 1);
   }

   if( (flag & FLAG_HAS_COMMENT) != 0 )
   {
     const char* temp= addr + size;
     size += (strlen(temp) + 1);
   }

   if( (flag & FLAG_HAS_CRC16) != 0 )
     size += sizeof(uint16_t);

   return size;
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::~GzipArchive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   GzipArchive::~GzipArchive( void )// Destructor
{
   if( isValid )                    // If inflate active
   {
     isValid= FALSE;
     inflateEnd(&stream);
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
//       GzipArchive::GzipArchive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   GzipArchive::GzipArchive(        // Constructor
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

   // Load the header
   origin= (unsigned char*)malloc(CHUNK); // Our input data chunk
   if( origin == NULL )
   {
     sprintf(nameBuffer, "No Storage\n");
     return;
   }
   HEAD* head= (HEAD*)origin;       // Use the origin as a work area
   memset(head, 0, CHUNK);          // Zero the storage

   file->setOffset(0);              // Begin at the beginning
   unsigned L= file->read(head, CHUNK-8); // (HEAD cannot pass ending zeros)
   if( L < sizeof(HEAD) )
   {
     sprintf(nameBuffer, "Missing header\n");
     return;
   }

   // Validate the header
   if( head->id1 != IDENT_1 || head->id2 != IDENT_2 )
   {
     sprintf(nameBuffer, "Invalid id: %.2x,%.2x\n", head->id1, head->id2);
     return;
   }

   const char* name= head->getNameAddr(); // Get the name address
   if( name != NULL && strlen(name) > (sizeof(nameBuffer)-1) ) // If name too long
   {
     strcpy(nameBuffer, "Name too long\n");
     return;
   }
   strcpy(nameBuffer, name);        // Save the name

   name= head->getCommentAddr();    // Get the comment address
   if( name != NULL && strlen(name) > (sizeof(nameBuffer)-1) ) // If comment too long
   {
     strcpy(nameBuffer, "Comment too long\n");
     return;
   }

   if( head->verifyCRC16() == FALSE ) // If invalid CRC
   {
     sprintf(nameBuffer, "HEAD: Invalid CRC16\n");
     return;
   }

// debug(*head, TRUE);

   // Heading accepted
   this->name= nameBuffer;

   this->file= file;                // (Required by index)
   if( index(0) == NULL )
     this->file= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::make
//
// Purpose-
//       Allocate and initialize a GzipArchive.
//
//----------------------------------------------------------------------------
GzipArchive*                        // Resultant Archive
   GzipArchive::make(               // Allocate and initialize a GzipArchive
     DataSource*       file)        // Using this DataSource
{
// IFHCDM( debugf("GzipArchive::make(%s)\n", file->getName()); )

   GzipArchive* result= NULL;       // Resultant Archive
   try {
     result= new GzipArchive(file); // Allocate resultant
     if( result->file == NULL )     // If failure
     {
       if( stricmp(".gz", FileName::getExtension(file->getCName())) == 0 )
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
//       GzipArchive::index
//
// Function-
//       Select an object by index
//
//----------------------------------------------------------------------------
const char*                         // The object name
   GzipArchive::index(              // Select
     unsigned int      object)      // This object object
{
// IFHCDM( debugf("GzipArchive(%p)::index(%d)\n", this, object); )

   // Reset the inflate stream
   if( isValid )
   {
     isValid= FALSE;

     inflateEnd(&stream);
   }
   memset(&stream, 0, sizeof(stream));

   offset= 0;
   length= 0;                       // (Unknown length)

   if( object != 0 )
     return NULL;

   // Initialize decoding
   file->setOffset(0);              // (Re)position the file

   stream.zalloc=  Z_NULL;
   stream.zfree=   Z_NULL;
   stream.opaque=  Z_NULL;
   stream.next_in= (Bytef*)origin;
   stream.avail_in= file->read(origin, CHUNK);

   stream.next_out= Z_NULL;
   stream.avail_out= 0;

   //-------------------------------------------------------------------------
   // Initialize the ZLIB decompressor (With decoding)
   //-------------------------------------------------------------------------
   int zrc= inflateInit2(&stream, MAX_WBITS+32);
   if( zrc != Z_OK )
   {
     sprintf(nameBuffer, "inflateInit error(%d)\n", zrc);
     return NULL;
   }

   isValid= TRUE;
   return name.c_str();
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::next
//
// Function-
//       Select the next object
//
//----------------------------------------------------------------------------
const char*                         // The object name
   GzipArchive::next( void )        // Select the next object
{
   object++;
   return index(object);
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   GzipArchive::read(               // Read from current item
     void*             addr,        // Into this buffer address
     unsigned int      size)        // For this length
{
// IFHCDM( debugf("GzipArchive(%p)::read(%p,%d)\n", this, addr, size); )

   unsigned L= 0;                   // Number of bytes read
   if( isValid )
   {
     if( stream.avail_in == 0 )     // If read required
     {
       stream.next_in= (Bytef*)origin;
       stream.avail_in= file->read(origin, CHUNK);
       if( stream.avail_in == 0 )
         throwf("GZIP(%s) decode error", getCName());
     }

     stream.next_out= (Bytef*)addr;
     stream.avail_out= size;
     if( size > 0 )
     {
       int zrc= inflate(&stream, Z_NO_FLUSH);
       L= size - stream.avail_out;
       if( zrc == Z_STREAM_END )
       {
         isValid= FALSE;
         inflateEnd(&stream);
       }
       else if( zrc != Z_OK )
       {
         const char* text= stream.msg;
         if( text == NULL )
           text= "(no message)";

         debugSTREAM(stream);
         throwf("GZIP(%s) decode error(%d) %s", getCName(), zrc, text);
       }

       offset += L;
     }
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::setOffset
//
// Function-
//       Set offset within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   GzipArchive::setOffset(          // Set position
     size_t            offset)      // Offset
{
   if( offset < this->offset )      // If reverse seek
     index(0);                      // Reposition to offset 0
   else
     offset -= this->offset;        // Relative to current

   // Lame way to seek forward
   char buffer[512];
   while( offset >= sizeof(buffer) )
   {
     size_t L= read(buffer, sizeof(buffer));
     if( L == 0 )
     {
       debugf("GzipArchive seek past EOF\n");
       return (-1);
     }

     offset -= L;
   }

   if( offset > 0 )
   {
     size_t L= read(buffer, offset);
     if( L != offset )
     {
       debugf("GzipArchive seek past EOF\n");
       return (-1);
     }
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       GzipArchive::debug
//
// Function-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   GzipArchive::debug(              // Debugging display
     HEAD&             head,        // Of this HEAD structure
     int               full)        // TRUE iff extended header present
{
   debugf("GzipArchive(%p)::debugHEAD(%p)\n", this, &head);

   debugf(">>ident(%.2x,%.2x)\n", head.id1, head.id2);
   debugf("...mech(%2d)\n", head.mech);
   debugf("...flag(%.2x)\n", head.flag);
   debugf("modTime(%8d)\n", head.modTime);
   debugf(".....fm(%.2x)\n", head.fm);
   debugf(".....os(%2d)\n", head.os);

   if( full )                       // If extended header present
   {
     if( (head.flag & FLAG_HAS_EXTRA) != 0 )
     {
       debugf("Extra(%d)\n", head.getExtraSize());
       dump(head.getExtraAddr(), head.getExtraSize());
     }

     if( (head.flag & FLAG_HAS_NAME) != 0 )
       debugf("...name(%s)\n", head.getNameAddr());

     if( (head.flag & FLAG_HAS_COMMENT) != 0 )
       debugf("comment(%s)\n", head.getCommentAddr());

     if( (head.flag & FLAG_HAS_CRC16) != 0 )
       debugf("..CRC16(%s)\n", head.verifyCRC16() ? "VALID" : "ERROR");
   }
}

