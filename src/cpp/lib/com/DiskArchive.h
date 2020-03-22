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
//       DiskArchive.h
//
// Purpose-
//       Define and implement the DiskArchive object.
//
// Last change date-
//       2014/01/01
//
// Implementation notes-
//       Included from Archive.cpp
//       Only disk archives are supported (not tape archives.)
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Subroutine-
//       toOctal
//
// Purpose-
//       Convert a string to an octal value
//
//----------------------------------------------------------------------------
static long                         // Return value (-1 iff invalid)
   toOctal(                         // Extract octal value
     const char*       string)      // From this string
{
   long                result;      // Resultant
   int                 C;           // The current character

   result= 0;
   while( *string != '\0' )
   {
     C= *string;                    // The current character
     if( C < '0' || C > '7' )       // If invalid octal value
       return (-1);                 // INVALID

     result *= 8;
     result += (C - '0');
     string++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Class-
//       DiskArchive
//
// Purpose-
//       Disk resident TAR format Archive
//
//----------------------------------------------------------------------------
class DiskArchive : public Archive { // FILE Archive
protected: // INTERNAL STRUCTURES
#pragma pack(1)

struct GNU_HEADER {                 // TAR GNU header
char                   name[100];   // 000 File name
char                   mode[8];     // 100 Mode, octal c-string
char                   uid[8];      // 108 User ID, octal c-string
char                   gid[8];      // 116 Group ID, octal c-string
char                   size[12];    // 124 File size, octal c-string
char                   mtime[12];   // 136 Modification time, octal c-string
char                   chksum[8];   // 148 Checksum, octal c-string
char                   typeflag;    // 156 Type
char                   link[100];   // 157 Link name, c-string
char                   magic[8];    // 257 "ustar  " self-identifier (w\ null)
char                   uname[32];   // 265 User name, c-string
char                   gname[32];   // 297 Group name, c-string
char                   devmajor[8]; // 329
char                   devminor[8]; // 337
char                   prefix[155]; // 345
}; // struct GNU_HEADER             // 500 (Total length)

struct POSIX_HEADER {               // TAR posix header
char                   name[100];   // 000 File name
char                   mode[8];     // 100 Mode, octal c-string
char                   uid[8];      // 108 User ID, octal c-string
char                   gid[8];      // 116 Group ID, octal c-string
char                   size[12];    // 124 File size, octal c-string
char                   mtime[12];   // 136 Modification time, octal c-string
char                   chksum[8];   // 148 Checksum, octal c-string
char                   typeflag;    // 156 Type
char                   link[100];   // 157 Link name, c-string
char                   magic[6];    // 257 "ustar" self-identifier (w\ null)
char                   version[2];  // 263 "00" version identifier (no null)
char                   uname[32];   // 265 User name, c-string
char                   gname[32];   // 297 Group name, c-string
char                   devmajor[8]; // 329
char                   devminor[8]; // 337
char                   prefix[155]; // 345
}; // struct POSIX_HEADER           // 500 (Total length)

#pragma pack()

static const char*     GMAGIC;      // With trailing NUL character
static const char*     TMAGIC;      // With trailing NUL character
static const char*     TVERSION;    // Trailing NUL not used

enum                                // Generic enum
{  BLOCK_SIZE= 512                  // The data block size
}; // enum

enum TYPE                           // Values for typeflag
{  REGTYPE=  '0'                    // Regular file
,  AREGTYPE= '\0'                   // Regular file (deprecated)
,  LNKTYPE=  '1'                    // Link
,  SYMTYPE=  '2'                    // Reserved
,  CHRTYPE=  '3'                    // Character special
,  BLKTYPE=  '4'                    // Block special
,  DIRTYPE=  '5'                    // Directory
,  FIFOTYPE= '6'                    // FIFO special
,  CONTTYPE= '7'                    // Reserved
,  XHDTYPE=  'x'                    // Extended header, refers to next file
,  XGLTYPE=  'g'                    // Global extended header
,  LONGTYPE= 'L'                    // (GNU) Long name descriptor
}; // enum TYPE

enum MODE // octal                  // Values for mode
{  TSUID=    04000                  // Set UID on execution
,  TSGID=    02000                  // Set GID on execution
,  TSVTX=    01000                  // Reserved
,  TUREAD=   00400                  // OWNER read  permitted
,  TUWRITE=  00200                  // OWNER write permitted
,  TUEXEC=   00100                  // OWNER exec  permitted
,  TGREAD=   00040                  // GROUP read  permitted
,  TGWRITE=  00020                  // GROUP write permitted
,  TGEXEC=   00010                  // GROUP exec  permitted
,  TOREAD=   00004                  // OTHER read  permitted
,  TOWRITE=  00002                  // OTHER write permitted
,  TOEXEC=   00001                  // OTHER exec  permitted
}; // enum MODE

struct Buffer {                     // Header buffer
GNU_HEADER             head;        // The GNU header
char                   suffix[12];  // (Unused) suffix
};

protected: // ATTRIBUTES
uint64_t               origin;      // Origin of this file
Buffer                 buffer;      // The GNU/POSIX header buffer

public: // CONSTRUCTORS
virtual
   ~DiskArchive( void );            // Destructor
   DiskArchive(                     // Constructor
     DataSource*       file);       // DataSource

public:
static DiskArchive*                 // Resultant Archive (NULL if none)
   make(                            // Create DiskArchive
     DataSource*       file);       // From this DataSource

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
}; // class DiskArchive

const char*            DiskArchive::GMAGIC= "ustar  "; // With trailing NUL
const char*            DiskArchive::TMAGIC= "ustar"; // With trailing NUL
const char*            DiskArchive::TVERSION= "00";  // Trailing NUL not used

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::~DiskArchive
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   DiskArchive::~DiskArchive( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::DiskArchive
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   DiskArchive::DiskArchive(        // Constructor
     DataSource*       file)        // DataSource
:  Archive()
,  origin(0)
{
   this->file= file;                // (Required for index)

   if( index(0) == NULL )
     this->file= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::make
//
// Purpose-
//       Allocate and initialize a DiskArchive.
//
//----------------------------------------------------------------------------
DiskArchive*                        // Resultant
   DiskArchive::make(               // Allocate and initialize a DiskArchive
     DataSource*       file)        // Using this DataSource
{
   DiskArchive*        result= NULL; // Resultant Archive

   try {
     result= new DiskArchive(file); // Allocate resultant
     if( result->file == NULL )     // If failure
     {
       if( stricmp("tar", FileName::getExtension(file->getCName())) == 0 )
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
//       DiskArchive::index
//
// Function-
//       Select an item by index
//
//----------------------------------------------------------------------------
const char*                         // The item name
   DiskArchive::index(              // Select
     unsigned int      index)       // This item index
{
   if( index == 0 )
     origin= 0;
   else
   {
     if( index == (object + 1) )
     {
       length += (BLOCK_SIZE-1);    // Round up length
       length &= ~(BLOCK_SIZE-1);   // Truncate length
       origin += length;            // Set new origin
     }
     else
       throwf("%4d DiskArchive NOT CODED YET", __LINE__); // TODO: CODE
   }

   // Initialize for next object
   name= "";
   object= index;
   offset= 0;
   length= 0;

   int rc= file->setOffset(origin);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d DiskArchive(%s) %d= setOffset(%" PRId64 ")\n",
             __LINE__, file->getCName(), rc, origin);
     return NULL;
   }

   unsigned L= file->read(&buffer, sizeof(buffer));
   if( L != sizeof(buffer) )
   {
     fprintf(stderr, "%4d DiskArchive: %d= read(%ld)", __LINE__,
             L, (long)sizeof(buffer));
     return NULL;
   }
   memset(buffer.suffix, 0, sizeof(buffer.suffix)); // Insure ending delimiter

   // Extract archive element
   GNU_HEADER* head= &buffer.head;  // Have GNU or POSIX header
   if( strcmp(GMAGIC, head->magic) != 0 )
   {
     if( strcmp(TMAGIC, head->magic) != 0 // If invalid format
         || head->magic[6] != TVERSION[0]
         || head->magic[7] != TVERSION[1] )
     {
       #ifdef HCDM
         // Check for end of Archive
         int isEOF= TRUE;
         const unsigned char* C= (const unsigned char*)&buffer;
         for(int i= 0; i<sizeof(buffer); i++)
         {
           if( C[i] != '\0' )
           {
             isEOF= FALSE;
             break;
           }
         }

         if( isEOF == FALSE )
           debugf("%4d File(%s) not TAR format\n", __LINE__, file->getCName());
       #endif

       return NULL;
     }
   }

   // Validate/compute header checksum
   if( strlen(head->chksum) != 6 || head->chksum[7] != ' ' )
   {
     IFHCDM(
       debugf("%4d File(%s) checksum format\n", __LINE__, file->getCName());
     )
     return NULL;
   }

   long checksum= toOctal(head->chksum);
   if( checksum < 0 )
   {
     IFHCDM(
       debugf("%4d File(%s) checksum(%s)\n", __LINE__,
              file->getCName(), head->chksum);
     )
     return NULL;
   }

   memset(head->chksum, ' ', sizeof(head->chksum)); // Replace with blanks
   unsigned long matchsum= 0;
   for(int i= 0; i<sizeof(GNU_HEADER); i++)
     matchsum += *((unsigned char*)(&buffer) + i);

   matchsum &= 0x00007fff;
   if( matchsum != checksum )
   {
     IFHCDM( printf("%4d using signed checksum\n", __LINE__); )

     for(int i= 0; i<sizeof(GNU_HEADER); i++)
       matchsum += *((signed char*)(&buffer) + i);

     matchsum &= 0x00007fff;
     if( matchsum != checksum )
     {
       IFHCDM(
         debugf("%4d File(%s) checksum error\n", __LINE__, file->getCName());
       )
       return NULL;
     }
   }

   name= head->name;
   mode= toOctal(head->mode);
   length= toOctal(head->size);
   time= toOctal(head->mtime);
   origin += BLOCK_SIZE;            // Set file origin

   switch( head->typeflag )         // Process file type
   {
     case DIRTYPE:                  // Directory
       mode |= S_IFDIR;             // Indicate directory
       break;

     case LNKTYPE:                  // Link
     case CHRTYPE:                  // Character special
     case BLKTYPE:                  // Block special
     case LONGTYPE:                 // (GNU) Long name descriptor
     case FIFOTYPE:                 // FIFO special
     case XHDTYPE:                  // Extended header, refers to next file
     case XGLTYPE:                  // Global extended header
//     throwf("%4d DiskArchive NOT CODED YET", __LINE__); // Treat as regular

//   case SYMTYPE:                  // Reserved
//   case CONTTYPE:                 // Reserved
//   case REGTYPE:                  // Regular file
//   case AREGTYPE:                 // Regular file (deprecated)
     default:                       // Default
       mode |= S_IFREG;             // Indicate regular file
       break;
   }

// debugf("Name: %s\n", head->name);
// debugf("Mode: %s\n", head->mode);
// debugf("Size: %s\n", head->size);
// debugf("Time: %s\n", head->mtime);
// debugf("Type: %c %d\n", head->typeflag, head->typeflag);

   return head->name;
}

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::next
//
// Function-
//       Select the next item
//
//----------------------------------------------------------------------------
const char*                         // The item name
   DiskArchive::next( void )        // Select the next item
{
   return index(object + 1);
}

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::read
//
// Function-
//       Read from current item.
//
//----------------------------------------------------------------------------
unsigned int                        // The number of bytes read
   DiskArchive::read(               // Read from current item
     void*             addr,        // Into this buffer address
     unsigned int      size)        // For this length
{
   // Do not read past end of file
   if( (offset + size) > length )
     size= length - offset;

   // Read from file
   unsigned L= file->read(addr, size);

   offset += L;

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       DiskArchive::setOffset
//
// Function-
//       Set position within current item.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   DiskArchive::setOffset(          // Set position
     int64_t           offset)      // Offset
{
   if( offset < 0 || offset > length )
     return (-1);

   this->offset= offset;
   return file->setOffset(origin + offset);
}

