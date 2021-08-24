//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       extract.cpp
//
// Purpose-
//       Extract encoded files from a set of files.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/define.h>
#include <com/Buffer.h>
#include <com/Debug.h>
#include <com/istring.h>
#include <com/params.h>
#include <com/FileInfo.h>
#include <com/Media.h>
#include <com/Parser.h>
#include <com/Reader.h>
#include <com/Unconditional.h>
#include <com/Writer.h>

#include "Codec.h"
#include "Base64Codec.h"
#include "LineParser.h"
#include "UuCodeCodec.h"
#include "YncodeCodec.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define INP_SIZE              32768 // Input buffer size
#define NAME_SIZE              1024 // Maximum size of a filename
#define PROPSIZE                512 // Property size

enum FSM                            // Finite state machine
{  FSM_PROP                         // PROP state
,  FSM_DATA                         // DATA state
,  FSM_EXIT                         // EXIT state
}; // enum FSM

#define count localCount            // Avoids conflict with library routine
#define index localIndex            // Avoids conflict with library routine

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
//
// Struct-
//       Segment
//
// Purpose-
//       Describe a file segment.
//
//----------------------------------------------------------------------------
struct Segment {                    // File Segment
//----------------------------------------------------------------------------
// Segment::Constructors
//----------------------------------------------------------------------------
   ~Segment( void );                // Destructor
   Segment( void );                 // Default constructor

//----------------------------------------------------------------------------
// Segment: Attributes
//----------------------------------------------------------------------------
   Segment*            next;        // Chain pointer
   unsigned            index;       // Index number
   TempBuffer          temp;        // Segment data
}; // struct Segment

//----------------------------------------------------------------------------
//
// Struct-
//       Content
//
// Purpose-
//       Describe a file's content.
//
//----------------------------------------------------------------------------
struct Content {                    // File content
//----------------------------------------------------------------------------
// Content::Enumerations and typedefs
//----------------------------------------------------------------------------
enum Code                           // Encoding
{  CodeRESET                        // Encoding not specified
,  Code64                           // Base 64 Encoding
,  CodeUU                           // UU Encoding
,  CodeYN                           // yEnc Encoding
,  CodeEMPTY                        // Content has been extracted
}; // enum Code

//----------------------------------------------------------------------------
// Content::Constructors
//----------------------------------------------------------------------------
   ~Content( void );                // Destructor
   Content( void );                 // Default constructor

//----------------------------------------------------------------------------
// Content: Methods
//----------------------------------------------------------------------------
int                                 // TRUE if all Segments are present
   isComplete( void ) const;        // Are all Segments present?

Segment*                            // -> Segment
   open(                            // Open Segment
     unsigned          index);      // Segment index

void
   close(                           // Close Segment
     Segment*          segment);    // -> Segment

void
   decode(                          // Decode a file
     Reader&           inp);        // Input unit

void
   empty( void );                   // Empty the Content

void
   extract( void );                 // Extract the Content

//----------------------------------------------------------------------------
// Content: Attributes
//----------------------------------------------------------------------------
   Content*            next;        // Chain pointer
   char*               name;        // The file's name
   int                 code;        // Encoding
   unsigned            count;       // Number of Segments
   unsigned            size;        // For CODEYN, file size
   Segment*            head;        // First Segment
}; // struct Content

//----------------------------------------------------------------------------
//
// Struct-
//       Multipart
//
// Purpose-
//       Describe multipart content.
//
//----------------------------------------------------------------------------
struct Multipart {                  // Multipart content
//----------------------------------------------------------------------------
// Multipart::Enumerations and typedefs
//----------------------------------------------------------------------------
enum Code                           // Content-Transfer-Encoding
{  CodeRESET                        // Not specified
,  Code7                            // 7bit Encoding
,  Code8                            // 8bit Encoding
,  Code64                           // base64 Encoding
,  CodeQuotePrint                   // quoted/printable
,  CodeBinary                       // binary
}; // enum Code

enum Cset                           // charset
{  CsetRESET                        // Not specified
,  CsetASCII                        // us-ascii
,  CsetISO_8859_1                   // ISO-8859-1
,  CsetMacintosh                    // Macintosh
,  CsetUTF8                         // utf-8
}; // enum Cset

enum Type                           // Content-Type
{  TypeRESET                        // Not specified
,  TypeHTML                         // text/html
,  TypeIMAGEGIF                     // image/gif
,  TypeMULTIALT                     // multipart/alternative
,  TypeMULTIMIX                     // multipart/mixed
,  TypeMULTIREL                     // multipart/related
,  TypeMULTI_1ST= TypeMULTIALT      // First multipart type
,  TypeMULTI_LST= TypeMULTIREL      // Last multipart type
,  TypeOCTET                        // application/octet-stream
,  TypeTEXT                         // text/plain
,  TypeUNKNOWN                      // unknown/unknown
}; // enum Type

//----------------------------------------------------------------------------
// Multipart::Constants for parameterization
//----------------------------------------------------------------------------
static const char*     codeName[];  // NULL delimited code name array
static const char*     csetName[];  // NULL delimited cset name array
static const char*     typeName[];  // NULL delimited type name array

//----------------------------------------------------------------------------
// Multipart::Constructors
//----------------------------------------------------------------------------
   ~Multipart( void );              // Destructor
   Multipart( void );               // Default constructor

//----------------------------------------------------------------------------
// Multipart::Methods
//----------------------------------------------------------------------------
int                                 // New mode
   extract( void );                 // Extract multipart content

int                                 // New mode
   extractData( void );             // Extract Multipart data

int                                 // New mode
   extractProp( void );             // Extract Multipart properties

const char*                         // Return message (NULL OK)
   parseProp(                       // Parse a Multipart Property line
     char*             inpLine);    // The line to parse

const char*                         // Return message (NULL OK)
   parseType(                       // Parse the "Content-Type:" Property line
     char*             inpLine);    // The line to parse

//----------------------------------------------------------------------------
// Multipart: Attributes
//----------------------------------------------------------------------------
   char*               boundary;    // boundary=
   char*               name;        // name=
   int                 code;        // Content-Transfer-Encoding:
// int                 cset;        // charset=
   int                 type;        // Content-Type:
}; // struct Multipart

//----------------------------------------------------------------------------
//
// Struct-
//       Plainpart
//
// Purpose-
//       Describe plain part content.
//
//----------------------------------------------------------------------------
struct Plainpart {                  // Plainpart content
//----------------------------------------------------------------------------
// Plainpart::Enumerations and typedefs
//----------------------------------------------------------------------------
enum FileState                      // Subject state
{  FILE_1of1                        // Simple subject
,  FILE_1ofM                        // First of multiple
,  FILE_NofM                        // Middle of multiple
,  FILE_MofM                        // Last of multiple
}; // enum FileState

//----------------------------------------------------------------------------
// Plainpart::Constructors
//----------------------------------------------------------------------------
   ~Plainpart( void );              // Destructor
   Plainpart( void );               // Default constructor

//----------------------------------------------------------------------------
// Plainpart::Methods
//----------------------------------------------------------------------------
int                                 // New mode
   extract( void );                 // Extract Plainpart content

void
   parseSubj(                       // Parse the "Subject:" Property line
     char*             inpLine);    // The line to parse (MODIFIED!)

//----------------------------------------------------------------------------
// Plainpart: Attributes
//----------------------------------------------------------------------------
   char*               name;        // File name
   int                 count;       // Number of elements in file
   int                 index;       // This file's element number
   int                 state;       // File state
}; // struct Plainpart

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static Content*        outs= NULL;  // The sorted list of output files
static FileReader      reader;      // Working FileReader
static char*           fileName;    // Working file name
static char*           inpLine;     // Input line
static char*           inpProp;     // Input property (continuation) line
static int             todaysMajor= 0; // Today's major number
static int             todaysMinor= 0; // Today's minor number

static int             sw_allowany; // Allow any filename?
static int             sw_allowdup; // Allow duplicates?
static int             sw_unnamed;  // Allow unnamed?
static int             sw_verbose;  // Verbose mode?

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
const char*            NULL_DELIMITER= "\"No delimiter\"";

static const char*     xName[] =    // Valid extension types
   { ".asx"
   , ".avi"
   , ".gif"
// , ".htm"
// , ".html"
   , ".jpg"
   , ".jpeg"
   , ".mpg"
   , ".mpeg"
   , ".wmv"
// , ".zip"
   };

// Property lists
enum Property                       // Valid property types
{  PROP_ARTICLE
,  PROP_CONTENT
,  PROP_FROM
,  PROP_SUBJECT
}; // enum Property

static const char*     const propName[] = // Valid property names
   { "Article:"
   , "Content-Type:"
   , "From:"
   , "Subject:"
   };

static int             propSize[] = // Length(property names)
   { 0                              // Article
   , 0                              // Content-Type
   , 0                              // From
   , 0                              // Subject
   };

static char            propData[][PROPSIZE] = // Property values area
   { {""}                           // Article
   , {""}                           // Content-Type
   , {""}                           // From
   , {""}                           // Subject
   };

// propValue[i]= propData[i] if present, NULL if not
static char*           propValue[] =// Property values
   { NULL                           // Article
   , NULL                           // Content-Type
   , NULL                           // From
   , NULL                           // Subject
   };

// Multipart properties list
enum MultipartProperty              // Valid property types
{  MPRO_TYPE
,  MPRO_BASE
,  MPRO_CODE
,  MPRO_NAME
,  MPRO_IDENT
,  MPRO_LENGTH
,  MPRO_LOCATION
}; // enum Property

static const char*     const mproName[] = // Valid Mulitpart property names
   { "Content-Type:"
   , "Content-Base:"
   , "Content-Transfer-Encoding:"
   , "Content-Disposition:"
   , "Content-ID:"
   , "Content-Length:"
   , "Content-Location:"
   };

const char*            Multipart::codeName[]=
   { "<none>"
   , "7bit"
   , "8bit"
   , "base64"
   , "quoted-printable"
   , "binary"
   , NULL};

const char*            Multipart::csetName[]=
   { "<none>"
   , "us-ascii"
   , "ISO-8859-1"
   , "Macintosh"
   , "utf-8"
   , NULL};

const char*            Multipart::typeName[]=
   { "<none>"
   , "text/html"
   , "image/gif"
   , "multipart/alternative"
   , "multipart/mixed"
   , "multipart/related"
   , "application/octet-stream"
   , "text/plain"
   , "unknown/unknown"
   , NULL};

//----------------------------------------------------------------------------
//
// Subroutine-
//       isBlank
//
// Purpose-
//       Is character a blank?
//
//----------------------------------------------------------------------------
static inline int                   // TRUE iff blank
   isBlank(                         // Is character a blank
     int               C)           // The character in question
{
   if( C == ' '
       || C == '\t'
       || C == '\r'
       || C == '\n' )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trim
//
// Purpose-
//       Remove leading and trailing blanks from a string.
//
//----------------------------------------------------------------------------
static const char*                  // String origin
   trim(                            // Trim leading/trailing blanks
     char*             string)      // From this string
{
   int x= strlen(string);
   x--;
   while( x > 0 && isBlank(string[x]) )
   {
     string[x]= '\0';
     x--;
   }

   while( isBlank(*string) )
     string++;

   return string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       uniqueFilename
//
// Purpose-
//       Choose a unique filename.
//       (Input: oldname.ext, output: oldname-DD-yymmdd-nnnn.ext)
//
//----------------------------------------------------------------------------
static void
   uniqueFilename(                  // Generate unique file name
     char*             target,      // The resultant unique file name
     const char*       source)      // The source filename string
{
   const char*         ptrC;        // -> Working string
   const char*         ptrX;        // -> File name extention
   int                 L;

   strcpy(target, source);          // Copy the original name

   // Locate the filename extension
   for(ptrC= source, ptrX= source; *ptrC != '\0'; ptrC++ )
   {
     if( *ptrC == '.' )
       ptrX= ptrC + 1;
   }

   // Append the unique suffix
   L= strlen(target);
   while( L > 0 )
   {
     L--;
     if( target[L] == '.' )
       break;
   }

   if( L > (NAME_SIZE - 32) )
     L= NAME_SIZE - 32;
   target += L;

   L= sprintf(target, "-DD-%.6d-%.4d.", todaysMajor, ++todaysMinor);
   target += L;

   // Append the file type
   if( strlen(ptrX) > 8 )
   {
     memcpy(target, ptrX, 8);
     *(target+8)= '\0';
   }
   else
     strcpy(target, ptrX);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getContent
//
// Purpose-
//       Locate the appropriate Content.
//
//----------------------------------------------------------------------------
static Content*                     // -> Content
   getContent(                      // Locate Content
     const char*       fileName,    // Associated fileName
     unsigned          count)       // Number of Segments
{
   Content*            content;     // Resultant

   for(content= outs; content != NULL; content= content->next)
   {
     if( strcmp(content->name, fileName) == 0 )
     {
       if( count != content->count )
         fprintf(stdout, "%4d content(%s) old(%d) new(%d) count\n", __LINE__,
                         content->name, content->count, count);

       return content;
     }
   }

   content= new Content();
   content->name= strdup(fileName);
   content->count= count;
   content->next= outs;
   outs= content;
   return content;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isBeginLine
//
// Purpose-
//       Determine whether a line is a begin line.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE iff begin line
   isBeginLine(                     // Is this a begin line
     const char*       inpLine)     // The line
{
   return (  memicmp("begin ", inpLine, 6) == 0
             && inpLine[6] >= '0' && inpLine[6] <= '7'
             && inpLine[7] >= '0' && inpLine[7] <= '7'
             && inpLine[8] >= '0' && inpLine[8] <= '7'
             && inpLine[9] == ' ' );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       isDelimiterLine
//
// Purpose-
//       Determine whether a line is a delimiter line.
//
//----------------------------------------------------------------------------
static int                          // TRUE iff delimiter line
   isDelimiterLine(                 // Is this a delimiter line
     const char*       inpLine,     // The line
     const char*       delimit)     // The delimiter
{
   // Look for property delimiter
   for(unsigned i= 0; i<ELEMENTS(propName); i++)
   {
     if( memicmp(propName[i], inpLine, propSize[i]) == 0 )
       return TRUE;
   }

   // Look for special delimiter
   if( memcmp("--", inpLine, 2) == 0 ) // If delimiter
   {
     if( memcmp(delimit, inpLine+2, strlen(delimit)) == 0 )
       return TRUE;

     if( sw_verbose )
       printf("%4d: Delimiter:\n....:  Found(%s)\n....: Expect(--%s)\n",
              __LINE__, inpLine, delimit);
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       readLine
//
// Purpose-
//       Read a reader line.
//
//----------------------------------------------------------------------------
static int                          // Return code
   readLine(                        // Read a reader line
     char*             into)        // Data address (Length INP_SIZE)
{
   int rc= reader.readLine(into, INP_SIZE);
   #ifdef HCDM
     printf("LINE: '%s'\n", into);
   #endif

   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       validExt
//
// Purpose-
//       Test for a valid extension
//
//----------------------------------------------------------------------------
static int                          // TRUE if extension is valid
   validExt(                        // Validate extension
     const char*       ext)         // The extension
{
   for(unsigned i= 0; i<ELEMENTS(xName); i++)
   {
     if( stricmp(ext, xName[i]) == 0 )
       return TRUE;
   }

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       validFilenameChar
//
// Purpose-
//       Test for a valid file name character
//
//----------------------------------------------------------------------------
static int                          // TRUE if character is valid
   validFilenameChar(               // Validate file name character
     int               C)           // The character
{
   if( !isprint(C) )
     return FALSE;

   if( C == '/' )
     return FALSE;

   if( C == ':' )
     return FALSE;

   if( C == '\\' )
     return FALSE;

   if( C == '~' )
     return FALSE;

   if( C == '<' )
     return FALSE;

   if( C == '>' )
     return FALSE;

   if( C == '|' )
     return FALSE;

   if( C == '*' )
     return FALSE;

   if( C == '?' )
     return FALSE;

   if( C == '\'' )
     return FALSE;

   if( C == '\"' )
     return FALSE;

   if( C == '`' )
     return FALSE;

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseFilename
//
// Purpose-
//       Parse a filename, putting result in fileName
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   parseFilename(                   // Parse file name
     const char*       string)      // The filename string
{
   int                 isNumeric;   // TRUE if name all numeric
   int                 x;           // Line index
   int                 xExt;        // Line index (for .extention)

   x= strlen(string);
   x--;
   while( x > 0 && string[x] != '.' )
     x--;
   xExt= x;

   if( !validExt(&string[xExt]) )
   {
     if( ! sw_allowany )
     {
       fprintf(stderr, "%4d: Invalid extension: %s\n", __LINE__, string);
       return "Extension";
     }

     fprintf(stderr, "%4d: Allowed extension: %s\n", __LINE__, string);
   }

   isNumeric= TRUE;
   x--;
   while( x >= 0 && validFilenameChar(string[x]) )
   {
     if( string[x] < '0' || string[x] > '9' )
       if( string[x] != ' ' )
         isNumeric= FALSE;
     x--;
   }

   while( string[x+1] == ' ' )      // Remove leading blanks
     x++;

   if( isNumeric )                  // Make numeric names unique
     uniqueFilename(fileName, string+x+1);
   else
     strcpy(fileName, string+x+1);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Find next non-whitespace in string
//
//----------------------------------------------------------------------------
static const char*                  // Next non-whitespace character
   skipBlank(                       // Find next non-whitespace character
     const char*       text)        // In this string
{
   Parser parser(text);
   return parser.skipSpace();
}

//----------------------------------------------------------------------------
//
// Method-
//       Segment::~Segment
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Segment::~Segment( void )        // Destructor
{
   #ifdef HCDM
     printf("Segment(%p)::~Segment()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Segment::Segment
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Segment::Segment( void )         // Default constructor
:  next(NULL)
,  index(0)
,  temp()
{
   #ifdef HCDM
     printf("Segment(%p)::Segment()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::~Content
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Content::~Content( void )        // Destructor
{
   #ifdef HCDM
     printf("Content(%p)::~Content()\n", this);
   #endif

   empty();
   name= Unconditional::replace(name, NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::Content
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Content::Content( void )         // Default constructor
:  next(NULL)
,  name(NULL)
,  code(CodeRESET)
,  count(0)
,  size(0)
,  head(NULL)
{
   #ifdef HCDM
     printf("Content(%p)::Content()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::isComplete
//
// Purpose-
//       Test for completeness
//
//----------------------------------------------------------------------------
int                                 // TRUE if all Segments are present
   Content::isComplete( void ) const// Are all Segments present?
{
   Segment*            segment;     // -> Current Segment
   unsigned            index;       // Current file index

   index= 0;
   for(segment= head; segment != NULL; segment= segment->next)
   {
     index++;
     if( segment->index != index )
       return FALSE;
   }
   if( count != index )
     return FALSE;

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Content::open
//
// Purpose-
//       Open a new Segment.
//
//----------------------------------------------------------------------------
Segment*                            // -> Segment
   Content::open(                   // Open Segment
     unsigned          index)       // Segment index
{
   Segment*            segment;     // Resultant
   Segment*            ptrS;        // -> Segment

   segment= NULL;
   if( code != CodeEMPTY )
   {
     ptrS= head;
     while( ptrS != NULL )
     {
       if( ptrS->index == index )
         break;

       ptrS= ptrS->next;
     }

     if( ptrS == NULL )
     {
       segment= new Segment();
       segment->index= index;
       segment->temp.open(name, Media::MODE_WRITE);

       ptrS= head;
       if( ptrS == NULL || ptrS->index > index )
       {
         segment->next= ptrS;
         head= segment;
       }
       else
       {
         while( ptrS->next != NULL && ptrS->next->index < index )
           ptrS= ptrS->next;

         segment->next= ptrS->next;
         ptrS->next= segment;
       }
     }
   }

   #ifdef HCDM
     printf("%p= Content(%p)::open(%d)\n", segment, this, index);
   #endif

   return segment;
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::close
//
// Purpose-
//       Close a Segment.
//
//----------------------------------------------------------------------------
void
   Content::close(                  // Close a Segment
     Segment*          segment)     // -> Segment
{
   #ifdef HCDM
     printf("Content(%p)::close(%p)\n", this, segment);
   #endif

   if( segment == NULL )
     return;

   segment->temp.close();

   if( !isComplete() )
     return;

   extract();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Content::decode
//
// Purpose-
//       Decode a file.
//
//----------------------------------------------------------------------------
void
   Content::decode(                 // Decode a file
     Reader&           reader)      // From this Reader
{
   Base64Codec         codec64;     // 64 Codec
   UuCodeCodec         codecUU;     // UU Codec
   YncodeCodec         codecYN;     // yEnc Codec
   FileInfo            info(name);  // File information
   FileWriter          writer;      // Writer
   const char*         status;      // Status
   char                string[64];  // Working string

   int                 rc;

   if( info.exists() )
   {
     if( !sw_allowdup )             // If duplicates not allowed
     {
       fprintf(stderr, "%4d: File(%s) rejected: No -D\n", __LINE__, name);
       return;
     }

     fprintf(stdout, "%4d: File(%s) Exists\n", __LINE__, name);
     uniqueFilename(fileName, name);
     name= Unconditional::replace(name, fileName);
     assert( name != NULL );
     fprintf(stdout, "%4d: ===>(%s)\n", __LINE__, name);
   }

   rc= reader.open(name);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d: SNO: File(%s) rdr open failure(%d)\n",
                     __LINE__, name, rc);
     return;
   }

   rc= writer.open(name);
   if( rc != 0 )
   {
     fprintf(stderr, "%4d: SNO: File(%s) wtr open failure(%d)\n",
                     __LINE__, name, rc);
     reader.close();
     return;
   }

   status= "UU";
   switch( code )
   {
     case Code64:
       status= "64";
       rc= codec64.decode(reader, writer);
       break;

     case CodeUU:
       rc= codecUU.decode(reader, writer);
       break;

     case CodeYN:
       status= "YN";
       rc= codecYN.decode(reader, writer);
       break;

     default:
       fprintf(stderr, "%4d: File(%s) code(%d), assumed UU\n",
                       __LINE__, name, code);
       rc= codecUU.decode(reader, writer);
       break;
   }

   reader.close();
   writer.close();
   if( rc != 0 )
   {
     sprintf(string, "Failed(%d), kept", rc);
     info.reset(name);
     if( info.getFileSize() == 0 )
     {
       sprintf(string, "Failed(%d), removed (empty)", rc);
       unlink(name);
     }
     status= string;
   }

   fprintf(stdout, "%4d: File(%s) Decode: %s\n", __LINE__, name, status);
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::empty
//
// Purpose-
//       Empty the Content
//
//----------------------------------------------------------------------------
void
   Content::empty( void )           // Empty the Content
{
   Segment*            ptrS;

   while( head != NULL )
   {
     ptrS= head;
     head= ptrS->next;
     delete ptrS;
   }

   code= CodeEMPTY;
}

//----------------------------------------------------------------------------
//
// Method-
//       Content::extract
//
// Purpose-
//       Extract the Content.
//
//----------------------------------------------------------------------------
void
   Content::extract( void )         // Extract the Content
{
   Segment*            segment;     // -> Current Segment
   TempBuffer          temp;        // Temporary
   unsigned            index;       // Current file index
   int                 C;           // Current character

   #ifdef HCDM
     printf("Content(%p)::extract()\n", this);
   #endif

   if( code == CodeEMPTY )
     return;

   index= 0;
   temp.open(name, Media::MODE_WRITE);
   for(segment= head; segment != NULL; segment= segment->next)
   {
     index++;
     if( segment->index != index )
     {
       if( index > segment->index )
       {
         fprintf(stdout, "%4d: File(%s) Extra(%d) ignored\n", __LINE__,
                         name, segment->index);
         index= segment->index;
         continue;
       }

       if( (segment->index-1) == index )
         fprintf(stdout, "%4d: File(%s) Missing(%d)\n", __LINE__,
                         name, index);
       else
         fprintf(stdout, "%4d: File(%s) Missing(%d..%d)\n", __LINE__,
                         name, index, segment->index-1);

       index= segment->index;
     }

     segment->temp.open(name, Media::MODE_READ);
     for(;;)
     {
       C= segment->temp.get();
       if( C == EOF )
         break;

       temp.put(C);
     }
     segment->temp.close();
     segment->temp.truncate();
   }
   if( count != index )
     fprintf(stdout, "%4d: File(%s) Missing(%d..%d)\n", __LINE__,
                     name, index+1, count);

   temp.close();
   decode(temp);
   empty();
   fprintf(stdout, "\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::~Multipart
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Multipart::~Multipart( void )    // Destructor
{
   #ifdef HCDM
     printf("Multipart(%p)::~Multipart()\n", this);
   #endif

   boundary= Unconditional::replace(boundary, NULL);
   name= Unconditional::replace(name, NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::Multipart
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Multipart::Multipart( void )     // Default constructor
:  boundary(NULL)
,  name(NULL)
,  code(CodeRESET)
,  type(TypeRESET)
{
   #ifdef HCDM
     printf("Multipart(%p)::Multipart()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::extract
//
// Purpose-
//       Extract Multipart data.
//
//----------------------------------------------------------------------------
int                                 // New mode
   Multipart::extract( void )       // Extract Multipart data
{
   const char*         delimit;     // Delimiter line
   int                 mode;        // Current mode

   int                 rc;

   #ifdef HCDM
     printf("Multipart(%p)::extract()\n", this);
   #endif

   //-------------------------------------------------------------------------
   // Find first delimiter line
   //-------------------------------------------------------------------------
   delimit= boundary;
   if( delimit == NULL )
     delimit= NULL_DELIMITER;

   for(;;)                          // Find the start delimiter
   {
     rc= readLine(inpLine);         // Read a file line
     if( rc < 0 )
       return FSM_EXIT;

     if( isDelimiterLine(inpLine, delimit) )
       break;
   }
   if( inpLine[0] != '-' )
     return FSM_PROP;

   //-------------------------------------------------------------------------
   // Extract multipart
   //-------------------------------------------------------------------------
   mode= FSM_DATA;
   while( mode == FSM_DATA )
   {
     name= Unconditional::replace(name, NULL);

     inpLine[0]= '\0';              // No default source
     mode= extractProp();           // Extract properties
     if( mode == FSM_DATA )
       mode= extractData();
   }

   return mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::extractData
//
// Purpose-
//       Extract Multipart data.
//
//----------------------------------------------------------------------------
int                                 // New mode
   Multipart::extractData( void )   // Extract Multipart data
{
   Content*            content;     // -> Current Content
   const char*         delimit;     // Delimiter line
   int                 mode;        // Resultant mode
   Segment*            segment;     // -> Current Segment
   char                string[32];  // Working file name

   int                 rc;

   #ifdef HCDM
     printf("Multipart(%p)::extractData()\n", this);
   #endif

   //-------------------------------------------------------------------------
   // Parse the data lines
   //-------------------------------------------------------------------------
   delimit= boundary;
   if( delimit == NULL )
     delimit= NULL_DELIMITER;

   content= NULL;
   segment= NULL;
   if( code == Code64 )
   {
     if( name == NULL )
     {
       if( type == TypeRESET )
         fprintf(stderr, "%4d: Rejected: TypeRESET\n", __LINE__);

       else if( !sw_unnamed )
         fprintf(stderr, "%4d: Rejected: No name, No -U\n", __LINE__);

       else
       {
         if( type == TypeIMAGEGIF )
           uniqueFilename(fileName, "unnamed.gif");
         else if( type == TypeOCTET || type == TypeUNKNOWN)
           uniqueFilename(fileName, "unnamed.jpg");
         else
         {
           sprintf(string, "type%d unnamed.jpg", type);
           uniqueFilename(fileName, string);
         }

         fprintf(stdout, "%4d: File(%s) %s\n", __LINE__,
                         fileName, typeName[type]);
         name= Unconditional::replace(name, fileName);
       }
     }

     if( name != NULL )
     {
       content= getContent(name, 1);
       segment= content->open(1);
       if( segment == NULL )
       {
         fprintf(stdout, "%4d: File(%s) dup\n", __LINE__, name);
         uniqueFilename(fileName, name);
         name= Unconditional::replace(name, fileName);
         content= getContent(name, 1);
         segment= content->open(1);
         assert( segment != NULL );
       }

       fprintf(stdout, "%4d: File(%s) base64\n", __LINE__, name);
       content->code= Content::Code64;
     }
   }

   mode= FSM_PROP;                  // Default return mode
   for(;;)                          // Find the start delimiter
   {
     rc= readLine(inpLine);         // Read a file line
     if( rc < 0 )
     {
       mode= FSM_EXIT;
       break;
     }

     if( inpLine[0] == '\0' )
       continue;

     // Look for special lines
     if( isDelimiterLine(inpLine, delimit) )
     {
       if( inpLine[0] == '-' )
       {
         mode= FSM_DATA;

         int L= strlen(inpLine);
         if( inpLine[L-1] == '-'
             && inpLine[L-2] == '-' )
           mode= FSM_PROP;
       }
       break;
     }

     if( segment != NULL )          // If handling this data
       segment->temp.printf("%s\n", inpLine);
   }

   if( content != NULL )
     content->close(segment);

   return mode;
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::extractProp
//
// Purpose-
//       Extract Multipart properties.
//
// Notes-
//       inpLine may contain a property line.
//
//----------------------------------------------------------------------------
int                                 // New mode
   Multipart::extractProp( void )   // Extract Multipart properties
{
// const char*         delimit;     // Delimiter line
   char*               oldLine;     // -> Prior data area
   char*               ptrLine;     // -> Input data area

   int                 rc;

   #ifdef HCDM
     printf("Multipart(%p)::extractProp()\n", this);
   #endif

   //-------------------------------------------------------------------------
   // Parse the property lines
   //-------------------------------------------------------------------------
   oldLine= inpLine;                // Prior input line
   ptrLine= inpProp;                // Active input line
   for(;;)                          // Handle property lines
   {
     rc= readLine(ptrLine);         // Read a file line
     if( rc < 0 )
       return FSM_EXIT;

     if( ptrLine[0] == '\0' )
     {
       parseProp(oldLine);
       break;
     }

     if( ptrLine[0] == ' ' || ptrLine[0] == '\t' ) // If continuation
     {
       if( (strlen(oldLine) + strlen(ptrLine)) < INP_SIZE )
         strcat(oldLine, ptrLine);
       else
         fprintf(stderr, "%4d: Line overflow: '%s'\n", __LINE__, ptrLine);
     }
     else
     {
       parseProp(oldLine);

       oldLine= ptrLine;
       if( oldLine == inpLine )
         ptrLine= inpProp;
       else
         ptrLine= inpLine;
     }
   }

   return FSM_DATA;
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::parseProp
//
// Purpose-
//       Parse a Multipart property line.
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Multipart::parseProp(            // Parse a Multipart Property line
     char*             inpLine)     // The line to parse
{

   const char*         result;      // Return message

   char*               ptrC;        // Working -> char
   int                 index;       // Property name index
   const char*         string;      // String remainder

   unsigned            i;

   #ifdef HCDM
     printf("Multipart(%p)::parseProp(%s)\n", this, inpLine);
   #endif

   if( inpLine[0] == '\0' )
     return NULL;

   index= (-1);
   for(i= 0; i<ELEMENTS(mproName); i++)
   {
     if( memicmp(mproName[i], inpLine, strlen(mproName[i])) == 0 )
     {
       index= i;
       break;
     }
   }
   if( index < 0 )
   {
     fprintf(stderr, "%4d: Unknown property: %s\n", __LINE__, inpLine);
     return "Unknown property";
   }

   result= NULL;
   switch(index)
   {
     case MPRO_TYPE:
       result= parseType(inpLine+strlen(mproName[MPRO_TYPE]));
       break;

     case MPRO_CODE:
       string= skipBlank(inpLine+strlen(mproName[MPRO_CODE]));
       result= "Unknown encoding";
       for(i= 1; codeName[i] != NULL; i++)
       {
         if( stricmp(string, codeName[i]) == 0 )
         {
           code= i;
           result= NULL;
           break;
         }
       }

       if( result != NULL )
         fprintf(stderr, "%4d: Unknown encoding: %s\n", __LINE__, inpLine);
       break;

     case MPRO_NAME:
       if( name != NULL )
         break;

       string= skipBlank(inpLine+strlen(mproName[MPRO_NAME]));
       ptrC= (char*)strstr(string, "name=");
       if( ptrC == NULL )
         break;

       string= ptrC+5;
       if( *string == '\"' )
       {
         string++;

         ptrC= (char*)strchr(string, '\"');
         if( ptrC == NULL )
         {
           fprintf(stderr, "%4d: Malformed name: %s\n", __LINE__, inpLine);
           result= "Format error";
           break;
         }
         *ptrC= '\0';
       }
       if( parseFilename(string) == NULL )
         name= Unconditional::replace(name, fileName);
       else
         code= CodeRESET;
       break;

     case MPRO_BASE:
     case MPRO_IDENT:
     case MPRO_LENGTH:
     case MPRO_LOCATION:
       break;

     default:
       fprintf(stderr, "%4d: Should not occur(%d)\n", __LINE__, index);
       result= "Should not occur";
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Multipart::parsetype
//
// Purpose-
//       Parse the "Content-Type:" property
//
//----------------------------------------------------------------------------
const char*                         // Return message (NULL OK)
   Multipart::parseType(            // Parse the "Content-Type:" Property line
     char*             inpLine)     // The line to parse
{
   char*               inpBoundary; // Input boundary
   char*               ptrC;        // Working -> char
   char*               string;      // propValue[PROP_CONTENT] remainder

   #ifdef HCDM
     printf("Multipart(%p)::parseType(%s)\n", this, inpLine);
   #endif

   inpBoundary= boundary;
   boundary= NULL;
   name= Unconditional::replace(name, NULL);

   string= (char*)skipBlank(inpLine);
   ptrC= strchr(string, ';');
   if( ptrC == NULL )
   {
     ptrC= string + strlen(string);
     ptrC[1]= '\0';                 // (We look at this string later)
   }
   *ptrC= '\0';
   for(int i= 1; typeName[i] != NULL; i++)
   {
     if( stricmp(string, typeName[i]) == 0 )
     {
       type= i;
       break;
     }
   }
   if( type == TypeRESET )
     fprintf(stderr, "%4d: Content-Type: %s\n", __LINE__, string);

   // Process 'boundary=' or 'name=', if present
   for(;;)
   {
     string= (char*)skipBlank(ptrC + 1);

     // Handle boundary
     if( memcmp("boundary=", string, 9) == 0 )
     {
       ptrC= strchr(string+10, '\"');
       if( *(string+9) != '\"' || ptrC == NULL )
         fprintf(stderr, "%4d: Content-type: %s\n", __LINE__, string);

       else
       {
         *ptrC= '\0';
         boundary= Unconditional::replace(boundary, string+10);
         continue;
       }
     }

     // Handle name
     if( memcmp("name=", string, 5) == 0 )
     {
       ptrC= strchr(string+6, '\"');
       if( *(string+5) != '\"' || ptrC == NULL )
         fprintf(stderr, "%4d: Content-type: %s\n", __LINE__, string);

       else
       {
         *ptrC= '\0';
         if( parseFilename(string+6) == NULL )
           name= Unconditional::replace(name, fileName);
         else
           code= CodeRESET;
         continue;
       }
     }

     // Ignored element
     ptrC= strchr(string, ';');
     if( ptrC == NULL )
       break;
   }

   #ifdef HCDM
     printf("%4d: name(%s) old(%p='%s') new(%p='%s')\n", __LINE__,
            name, inpBoundary, inpBoundary, boundary, boundary);
   #endif

   if( inpBoundary != NULL && boundary != NULL )
   {
     #ifdef HCDM
       printf("%4d: Push: '%s'\n=========>: '%s'\n", __LINE__,
              inpBoundary, boundary);
     #endif

//// There are no properties allowed after a boundary property
//// ::inpLine[0]= '\0';              // No default source
//// extractProp();                   // Extract remaining properties
     extract();                       // Process next level

     #ifdef HCDM
       printf("%4d:  Pop: '%s'\n=========>: '%s'\n", __LINE__,
              boundary, inpBoundary);
     #endif

     name= Unconditional::replace(name, NULL);
     code= CodeRESET;
     type= TypeRESET;
   }

   Unconditional::replace(boundary, NULL);
   boundary= inpBoundary;

   #ifdef HCDM
     printf("%4d: name(%s) old(%p='%s') new(%p='%s')\n", __LINE__,
            name, inpBoundary, inpBoundary, boundary, boundary);
   #endif

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Plainpart::~Plainpart
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Plainpart::~Plainpart( void )    // Destructor
{
   #ifdef HCDM
     printf("Plainpart(%p)::~Plainpart()\n", this);
   #endif

   name= Unconditional::replace(name, NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       Plainpart::Plainpart
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Plainpart::Plainpart( void )     // Default constructor
:  name(NULL)
,  count(0)
,  index(0)
,  state(FILE_1of1)
{
   #ifdef HCDM
     printf("Plainpart(%p)::Plainpart()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Plainpart::extract
//
// Purpose-
//       Extract Plainpart data.
//
//----------------------------------------------------------------------------
int                                 // New mode
   Plainpart::extract( void )       // Extract Plainpart data
{
   Content*            content;     // -> Current Content
   Segment*            segment;     // -> Current Segment

   int                 rc;

   #ifdef HCDM
     printf("Plainpart(%p)::extract()\n", this);
   #endif

   //-------------------------------------------------------------------------
   // Parse data lines
   //-------------------------------------------------------------------------
   parseSubj(propValue[PROP_SUBJECT]); // Parse the "Subject:" Line
   content= NULL;
   segment= NULL;
   if( state != FILE_1of1 )         // If part of split file
   {
     content= getContent(name, count);
     segment= content->open(index);
     if( segment == NULL )
     {
       state= FILE_1of1;
       fprintf(stdout, "%4d: File(%s) (%d/%d) dup\n", __LINE__,
                       name, index, count);
     }
     else
       fprintf(stdout, "%4d: File(%s) (%d/%d)\n", __LINE__,
                       name, index, count);
   }

   //-------------------------------------------------------------------------
   // Parse data lines
   //-------------------------------------------------------------------------
   for(;;)
   {
     rc= readLine(inpLine);
     if( rc < 0 )
       break;

     // Handle empty line
     if( inpLine[0] == '\0' )
       continue;

     if( content == NULL )
     {
       // Handle "begin" line
       if( isBeginLine(inpLine) )
       {
         if( state == FILE_1of1 )
         {
           if( parseFilename(trim(inpLine+10)) == NULL )
           {
             name= Unconditional::replace(name, fileName);
             content= getContent(name, count);
             segment= content->open(index);
             if( segment == NULL )
             {
               fprintf(stdout, "%4d: File(%s) dup\n", __LINE__, name);
               uniqueFilename(fileName, name);
               name= Unconditional::replace(name, fileName);
               content= getContent(name, 1);
               segment= content->open(1);
               assert( segment != NULL );
             }

             content->code= Content::CodeUU;
             fprintf(stdout, "%4d: File(%s) begin\n", __LINE__, name);
           }
         }

         continue;
       }

       // Handle "=ybegin" line
       if( TRUE && memicmp(inpLine, "=ybegin ", 8) == 0 )
       {
         LineParser lp(inpLine);

         const char* C= lp.find(" name=");
         if( C == NULL )
           continue;
         name= Unconditional::replace(name, C);

         int total= 1;
         C= lp.find(" total=");
         if( C != NULL)
           total= atol(C);
         content= getContent(name, total);

         index= 1;
         C= lp.find(" part=");
         if( C != NULL )
           index= atol(C);

         segment= content->open(index);
         if( segment == NULL )
         {
           content= NULL;
           continue;
         }

         C= lp.find(" size=");
         if( C != NULL )
         {
           unsigned long size= atol(C);
           if( content->size == 0 )
             content->size= size;
         }

         if( index == 1 )
           fprintf(stdout, "%4d: File(%s) begin\n", __LINE__, name);

         content->code= Content::CodeYN;
         continue;
       }
     }

     // Handle special lines
     if( content != NULL && content->code == Content::CodeYN )
     {
       if( memicmp(inpLine, "=yend ", 6) == 0 )
       {
         content->close(segment);
         content= NULL;
         segment= NULL;
         continue;
       }

       // Handle "=ypart" line
       if( memicmp(inpLine, "=ypart ", 7) == 0 )
         continue;
     }
     else
     {
       // Handle begin line
       if( isBeginLine(inpLine) )
       {
         if( content != NULL && state == FILE_1ofM )
         {
           content->code= Content::CodeUU;
           fprintf(stdout, "%4d: File(%s) begin\n", __LINE__, name);
         }

         continue;
       }

       // Handle "end" line
       if( stricmp("end", inpLine) == 0 )
       {
         if( state != FILE_1of1 && state != FILE_MofM )
         {
           if( content != NULL )
             fprintf(stdout, "%4d: File(%s) Unexpected end\n", __LINE__,
                             content->name);
           else
             fprintf(stdout, "%4d: File(%s) Unexpected end\n", __LINE__,
                             "<no-content>");
         }

         if( content != NULL )
           content->close(segment);
         content= NULL;
         segment= NULL;

         continue;
       }
     }

     // Handle delimiter line
     if( isDelimiterLine(inpLine, NULL_DELIMITER) )
     {
       if( content != NULL )
         content->close(segment);

       return FSM_PROP;
     }

     // Load the data line
     if( segment != NULL )          // If handling this data
     {
       if( inpLine[0] != '.' || inpLine[1] != '.' )
         segment->temp.printf("%s\n", inpLine);
       else
         segment->temp.printf("%s\n", inpLine+1);
     }
   }

   // Error or end of file
   return FSM_EXIT;
}

//----------------------------------------------------------------------------
//
// Method-
//       Plainpart::parseSubj
//
// Purpose-
//       Parse the "Subject:" property
//       Used to get the filename for multipart files.
//       (For single part files, a begin statement is required.)
//
// Valid forms-
//       *- filename.ext (index/count)*
//       *- filename.ext [index/count]*
//       filename.ext (index/count)*
//       filename.ext [index/count]*
//
//----------------------------------------------------------------------------
void
   Plainpart::parseSubj(            // Parse the "Subject:" Property line
     char*             inpLine)     // The line to parse (MODIFIED!)
{
   int                 cDelim;      // Closing delimiter
   int                 found;       // TRUE iff index/count sequence found
   int                 mDelim;      // Middle  delimiter
   Parser              parser;      // Parser
   char*               string;      // Working subject line
   int                 x;           // Line index

   #if defined(HCDM) && TRUE
     printf("Plainpart(%p)::parseSubj(%s)\n", this, inpLine);
   #endif

   state= FILE_1of1;                // Set default values
   count= index= 1;

   string= inpLine;
   if( string == NULL )
     return;

   // Find the last non-blank character in the line
   x= strlen(string);
   while( x > 0 && string[x-1] == ' ' )
   {
     x--;
   }
   if( x == 0 )
     return;
   string[x]= '\0';

   // Search backward looking for a (index/count) or [index/count]
   found= FALSE;
   mDelim= '\0';
   cDelim= '\0';
   while( x > 0 && !found )
   {
     x--;
     switch(string[x])
     {
       case ')':
         cDelim= ')';
         mDelim= '\0';
         break;

       case ']':
         cDelim= ']';
         mDelim= '\0';
         break;

       case '/':
         if( cDelim != '\0' )
           mDelim= '/';
         break;

       case '(':
         if( x > 0
             && string[x-1] == ' '
             && cDelim == ')'
             && mDelim == '/' )
         {
           found= TRUE;
           break;
         }

         cDelim= '\0';
         mDelim= '\0';
         break;

       case '[':
         if( x > 0
             && string[x-1] == ' '
             && cDelim == ']'
             && mDelim == '/' )
         {
           found= TRUE;
           break;
         }

         cDelim= '\0';
         mDelim= '\0';
         break;

       default:
         break;
     }
   }

   if( !found )
     return;

   // Parse the index and count
   parser.setString(&string[x]);
   parser.next();                   // Skip over opening delimiter
   index= parser.toDec();           // Get index
   if( parser.current() != mDelim )
   {
     fprintf(stderr, "%4d delmiter('%c') not('/'): %s\n", __LINE__,
                     parser.current(), string);
     return;
   }

   parser.next();                   // Skip over middle delimiter
   count= parser.toDec();           // Get count
   if( parser.current() != cDelim )
   {
     fprintf(stderr, "%4d delimiter('%c') not('%c'): %s\n", __LINE__,
                     parser.current(), cDelim, string);
     return;
   }

   if( index == 0 )
     return;

   if( index == 1 && count == 1 )
     return;

   if( index < 1 || count <= 1 || index > count )
   {
     fprintf(stderr, "%4d (%d/%d) in: %s\n", __LINE__, index, count, string);
     return;
   }

   // Index and count accepted - remove remainder
   while( x > 0 && string[x-1] == ' ' )
   {
     x--;
   }
   string[x]= '\0';
   if( x == 0 )
     return;

   // Find the file name extension (which must preceed the counts)
   while( x > 0 && string[x-1] != '.' )
     x--;

   if( x == 0 )
     return;

   x--;
   if( !validExt(&string[x]) )
     return;

   // We have accepted this file name, index and count
   // Get filename origin
   while( x >= 0 && validFilenameChar(string[x]) && string[x] != '-' )
     x--;

   while( string[x+1] == ' ' )
     x++;

   name= Unconditional::replace(name, &string[x+1]);

   if( index == 1 )
     state= FILE_1ofM;

   else if( index == count )
     state= FILE_MofM;

   else
     state= FILE_NofM;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "extract {options} filename ...\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename ...\n");
   fprintf(stderr, "  The list of files to extract\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "\t-A\tAllow all file names\n");
   fprintf(stderr, "\t-D\tAllow duplicate file extraction\n");
   fprintf(stderr, "\t-U\tAllow Unnamed file extraction\n");
   fprintf(stderr, "\t-V\tVerbose mode\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 count;       // The number of files to compare
   int                 error;       // Error encountered indicator
// int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   count= 0;
   error= FALSE;
// verify= 0;

   sw_allowany= FALSE;
   sw_allowdup= FALSE;
   sw_unnamed= FALSE;
   sw_verbose= FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

//     if( swname("verify", argp) ) // Verify?
//       verify= swatob("verify", argp);

       if( swname("A", argp) )      // Allow any file name?
         sw_allowany= swatob("A", argp);

       else if( swname("D", argp) )      // Allow duplicates?
         sw_allowdup= swatob("D", argp);

       else if( swname("U", argp) ) // Allow unnamed?
         sw_unnamed= swatob("U", argp);

       else if( swname("V", argp) ) // Verbose mode?
         sw_verbose= swatob("V", argp);

       else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       count++;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( count < 1 )                  // If too few files specified
   {
     error= TRUE;
     fprintf(stderr, "No filename specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   struct tm*          ptrTM;       // -> Time struct
   time_t              tod;         // Time of day

   #ifdef HCDM
     printf("%4d: init()\n", __LINE__);
   #endif

   // Allocate input lines
   fileName= (char*)Unconditional::malloc(INP_SIZE);
   inpLine= (char*)Unconditional::malloc(INP_SIZE);
   inpProp= (char*)Unconditional::malloc(INP_SIZE);

   // Initialize propSize array
   for(unsigned i= 0; i<ELEMENTS(propSize); i++)
     propSize[i]= strlen(propName[i]);

   // Get today's number
   tod= time(NULL);
   ptrTM= gmtime(&tod);
   todaysMajor= (ptrTM->tm_year+1900) % 100;
   todaysMajor= todaysMajor * 100 + ptrTM->tm_mon + 1;
   todaysMajor= todaysMajor * 100 + ptrTM->tm_mday;
   todaysMinor= 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Write the extracted and sorted output files.
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Write the output files
{
   Content*            content;     // -> Current Content

   #ifdef HCDM
     printf("%4d: term()\n", __LINE__);
   #endif

   while( outs != NULL )
   {
     content= outs;
     content->extract();
     outs= content->next;
     delete content;
   }

   free(inpProp);
   free(inpLine);
   free(fileName);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       resetProp
//
// Purpose-
//       Reset the properties
//
//----------------------------------------------------------------------------
static void
   resetProp( void )                // Reset properties
{
   for(unsigned i= 0; i<ELEMENTS(propValue); i++)
     propValue[i]= NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extractData
//
// Purpose-
//       Extract data, which may be multipart or plainpart
//
//----------------------------------------------------------------------------
static int                          // New mode
   extractData( void )              // Extract data
{
   int                 mode;        // Current mode
   Multipart           multi;       // Current Multipart
   Plainpart           plain;       // Current Plainpart

   #ifdef HCDM
     printf("%4d: extractData()\n", __LINE__);
   #endif

   // Handle Multipart data
   if( propValue[PROP_CONTENT] != NULL )
   {
     multi.parseType(propValue[PROP_CONTENT]);
     if( multi.type >= Multipart::TypeMULTI_1ST && multi.type <= Multipart::TypeMULTI_LST )
       multi.extract();
   }

   // Handle Plain data
   mode= FSM_DATA;
   while( mode == FSM_DATA )
     mode= plain.extract();

   return mode;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parseProp
//
// Purpose-
//       Parse an outer property line.
//
//----------------------------------------------------------------------------
static const char*                  // Return message (NULL OK)
   parseProp(                       // Parse a plain Property line
     const char*       inpLine)     // The line to parse
{
   #ifdef HCDM
     printf("%4d: parseProp(%s)\n", __LINE__, inpLine);
   #endif

   for(unsigned i= 0; i<ELEMENTS(propName); i++)
   {
     if( memicmp(propName[i], inpLine, propSize[i]) == 0 )
     {
       if( strlen(inpLine + propSize[i]) < PROPSIZE )
       {
         strcpy(propData[i], &inpLine[propSize[i]+1]);
         propValue[i]= propData[i];
         return NULL;
       }
     }
   }

   return "Unknown";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extractProp
//
// Purpose-
//       Extract properties.
//
// Notes-
//       inpLine may contain a property line.
//
//----------------------------------------------------------------------------
static int                          // New mode
   extractProp( void )              // Extract properties
{
   char*               oldLine;     // -> Prior data area
   char*               ptrLine;     // -> Input data area

   int                 rc;

   #ifdef HCDM
     printf("%4d: extractProp()\n", __LINE__);
   #endif

   //-------------------------------------------------------------------------
   // Parse the property lines
   //-------------------------------------------------------------------------
   oldLine= inpLine;                // Prior input line
   ptrLine= inpProp;                // Active input line
   for(;;)                          // Find the start delimiter
   {
     rc= readLine(ptrLine);         // Read a file line
     if( rc < 0 )
       break;

     if( ptrLine[0] == '\0' )
     {
       parseProp(oldLine);
       return FSM_DATA;
     }

     if( ptrLine[0] == ' ' || ptrLine[0] == '\t' ) // If continuation
     {
       if( (strlen(oldLine) + strlen(ptrLine)) < INP_SIZE )
         strcat(oldLine, ptrLine);
       else
         fprintf(stderr, "%4d: Line overflow: '%s'\n", __LINE__, ptrLine);
     }
     else
     {
       parseProp(oldLine);

       oldLine= ptrLine;
       if( oldLine == inpLine )
         ptrLine= inpProp;
       else
         ptrLine= inpLine;
     }
   }

   // Error or end of file
   return FSM_EXIT;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       extract
//
// Purpose-
//       Extract data from file.
//
//----------------------------------------------------------------------------
static int                          // Return code
   extract(                         // Extract data from file
     const char*       fileName)    // -> FileName to extract
{
   int                 mode;        // Current mode
   int                 rc;

   #ifdef HCDM
     printf("%4d: extract(%s)\n", __LINE__, fileName);
   #endif

   //-------------------------------------------------------------------------
   // Open the file
   //-------------------------------------------------------------------------
   rc= reader.open(fileName);       // Open the file
   if( rc != 0 )                    // Open the file
   {
     fprintf(stderr, "%d= reader.open(%s)", rc, fileName);
     perror("open failed");
     return (-1);
   }

   //-------------------------------------------------------------------------
   // Scan the file
   //-------------------------------------------------------------------------
   inpLine[0]= '\0';                // First time, no default source
   mode= FSM_PROP;                  // Reset properties
   resetProp();
   while( mode != FSM_EXIT )        // Scan the file
   {
     switch(mode)
     {
       case FSM_PROP:
         mode= extractProp();       // Extract properties
         break;

       case FSM_DATA:
         mode= extractData();       // Extract data
         resetProp();
         break;

       default:
         break;
     }
   }

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   reader.close();
   return 0;                        // Decode complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 rc;          // Called routine's return code
   int                 returncd;    // This routine's return code

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);
   init();

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   returncd= 0;
   try {
     for(int i=1; i<argc; i++)
     {
       if( argv[i][0] == '-' )      // If this parameter is in switch format
         continue;

       rc= extract(argv[i]);        // Copy the file to the content buffer
       if( rc != 0 )                // If failure
         returncd= 1;               // Indicate it
     }
   } catch(const char* E) {
     fprintf(stderr, "Exception(const char* '%s')\n", E);
   } catch(...) {
     fprintf(stderr, "Exception(...)\n");
   }

   //-------------------------------------------------------------------------
   // Write the output files
   //-------------------------------------------------------------------------
   term();

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}

