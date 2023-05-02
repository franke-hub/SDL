//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       FileName.cpp
//
// Purpose-
//       Extract information about a file name.
//
// Last change date-
//       2020/10/03
//
// Needs work-
//       TODO: Handle // and //computer_name/ prefix in Windows.
//       Note: Windows: dir \\computer_name always fails.
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <com/define.h>
#include <com/istring.h>
#include <com/Debug.h>
#include <com/Unconditional.h>

#include "com/FileName.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif
#include <com/ifmacro.h>            // Must follow define/undef HCDM

#ifndef MAX_SYMLINK
#define MAX_SYMLINK 128             // The maximum link recursion count
#endif

#define USE_PASSWD_HOME_GETENV 1
#define USE_PASSWD_HOME_GETUID 2

#define USE_PASSWD_HOME USE_PASSWD_HOME_GETENV

//----------------------------------------------------------------------------
// _OS_WIN
#if   defined(_OS_WIN)
#include <ctype.h>                  // toupper
#include <direct.h>                 // _getdcwd, _getdrive

const char*            PATH_SEPARATOR= "\\"; // _OS_WIN Path separator

#ifndef S_ISLNK
#define S_ISLNK(x) FALSE
#endif

#define lstat stat                  // No links in windows

//----------------------------------------------------------------------------
// _OS_BSD
#elif defined(_OS_BSD)
#include <pwd.h>                    // getpwnam

const char*            PATH_SEPARATOR= "/";  // _OS_BSD Path separator

//----------------------------------------------------------------------------
// _OS_ERR
#else
#error "Invalid OS"
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     STORAGE_SHORTAGE= "Storage shortage"; // Exception

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::catstr
//
// Purpose-
//       Force character by character string copy. (Overlap allowed)
//
//----------------------------------------------------------------------------
static void
   catstr(                          // Concatenate strings
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       append)      // The append string
{
   result += strlen(result);

   while( *append != '\0' )
   {
     *result= *append;
     result++;
     append++;
   }

   *result= '\0';
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::apppendString
//
// Purpose-
//       Append to the result buffer.
//
//----------------------------------------------------------------------------
static const char*                  // NULL iff valid
   appendString(                    // Append to string
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       append)      // The append string
   throw()                          // No exceptions
{
   if( (strlen(result) + strlen(append)) >= FILENAME_MAX )
     return "<FILENAME_MAX";

   catstr(result, append);
   return NULL;
}

static inline const char*           // NULL iff valid
   appendString(                    // Append to string
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       append,      // The append string
     size_t            length)      // The append string length
   throw()                          // No exceptions
{
   size_t resultLength= strlen(result);
   if( (resultLength + length) >= FILENAME_MAX )
     return "<FILENAME_MAX";

   result += resultLength;
   memcpy(result, append, length);
   result[length]= '\0';

   return NULL;
}

//----------------------------------------------------------------------------
//
// Macro-
//       COMPARE
//
// Purpose-
//       Is a character a path separator?
//
//----------------------------------------------------------------------------
#if defined(_OS_WIN) || defined(_OS_CYGWIN)
   #define COMPARE stricmp
#else
   #define COMPARE strcmp
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::isPathSep
//
// Purpose-
//       Is a character a path separator?
//
//----------------------------------------------------------------------------
static inline int                   // TRUE iff path separator
   isPathSep(                       // Is character a path separator?
     char              C)           // The test character
{
   #ifdef _OS_WIN
     if( C == '\\' )
       return TRUE;
   #endif

   if( C == '/' )
     return TRUE;

   return FALSE;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::getNextPathDelimiterIndex
//
// Purpose-
//       Get the next path delimiter index from a string
//
//----------------------------------------------------------------------------
static inline ssize_t               // The next delimiter index
   getNextPathDelimiterIndex(       // Get next delimiter index
     const char*       source,      // From this string
     size_t            offset)      // Starting at this offset
{
   if( offset >= strlen(source) )
     return (-1);

   const char* bsdC= strchr(source+offset, '/');

   #ifdef _OS_WIN
     const char* winC= strchr(source+offset, '\\');

     if( bsdC == NULL )
       bsdC= winC;
     else
     {
       if( winC != NULL && winC < bsdC )
         bsdC= winC;
     }
   #endif

   if( bsdC == NULL )
     return (-1);

   return bsdC - source;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::repairName
//
// Purpose-
//       Repair a file name (WIN only)
//
//----------------------------------------------------------------------------
static inline void
   repairName(                      // Repair file name
     char*             result)      // Repair a file name
{
   #ifdef _OS_WIN
     // Convert '/' delimters to '\\'
     for(int i=0; result[i] != '\0'; i++)
     {
       if( result[i] == '/' )
         result[i]= '\\';
     }
   #else
     (void)result;                  // (Unused parameter in NOP function)
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::readlink
//
// Purpose-
//       WINDOWS readlink method.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
static inline int                   // Length read, -1 if error
   readlink(                        // Read a link
     const char*       linkName,    // The name of the link
     char*             buffer,      // Result buffer
     size_t            length)      // Result buffer length
{
   IFHCDM( tracef("%d ShouldNotOccur readlink(%s)\n", __LINE__, linkName); )

   return (-1);                     // HOW DID WE GET HERE?
}
#endif // _OS_WIN

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::resetString
//
// Purpose-
//       Reset a result string.
//
//----------------------------------------------------------------------------
static inline void
   resetString(                     // Reset a result string
     char*             result)      // Resultant of length > FILENAME_MAX
   throw()                          // No exceptions
{
   result[0]= '\0';
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::getPrefixIndex
//
// Purpose-
//       Get the prefix index from a string
//
// Prereq-
//       isPathSep(), getNextPathDelimiterIndex()
//
//----------------------------------------------------------------------------
static inline ssize_t               // The first delimiter index
   getPrefixIndex(                  // Get first delimiter index
     const char*       source)      // From this string
{
   #if defined(_OS_WIN) || defined(_OS_CYGWIN)
     const int length= strlen(source);
     if( isPathSep(source[0]) && isPathSep(source[1]) )
     {
       ssize_t offset= getNextPathDelimiterIndex(source, 2);
       if( offset < 0 )
         offset= length;

       return offset;
     }
   #endif

   #ifdef _OS_WIN
     if( length > 1 && source[1] == ':' )
       return 1;

     if( length > 3 && source[3] == ':' )
       return 3;
   #endif

   #ifdef _OS_LINUX
     (void)source;                  // (Unused parameter)
   #endif

   return (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::~FileName
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   FileName::~FileName( void )      // Default destructor
   throw()                          // No exceptions
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::FileName( void )
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   FileName::FileName( void )       // Default constructor
   throw()                          // No exceptions
:  fileDesc(NULL)
,  fileTemp(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::FileName(const char*)
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   FileName::FileName(              // Constructor
     const char*     fileName)      // The path/file name string
:  fileDesc(NULL)
,  fileTemp(NULL)
{
   if( reset(fileName) == NULL )
     throw STORAGE_SHORTAGE;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::FileName(const char*, const char*)
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   FileName::FileName(              // Construct, setFileName
     const char*       filePath,    // The file path
     const char*       fileName)    // The file name
:  fileDesc(NULL)
,  fileTemp(NULL)
{
   if( reset(filePath, fileName) == NULL )
     throw STORAGE_SHORTAGE;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::FileName(const FileName&)
//
// Purpose-
//       Copy constructor
//
//----------------------------------------------------------------------------
   FileName::FileName(              // Constructor
     const FileName& source)        // The source FileName object
:  fileDesc(NULL)
,  fileTemp(NULL)
{
   if( source.fileDesc != NULL )
     fileDesc= Unconditional::strdup(source.fileDesc);

   if( source.fileTemp != NULL )
     fileTemp= Unconditional::strdup(source.fileTemp);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::operator=(const FileName&)
//
// Purpose-
//       Assignment operator constructor
//
//----------------------------------------------------------------------------
FileName&                           // Resultant (*this)
   FileName::operator=(             // Assignment operator
     const FileName& source)        // The source FileName object
{
   if( fileDesc != NULL )
   {
     free(fileDesc);
     fileDesc= NULL;
   }

   if( fileTemp != NULL )
   {
     free(fileTemp);
     fileTemp= NULL;
   }

   if( source.fileDesc != NULL )
     fileDesc= Unconditional::strdup(source.fileDesc);

   if( source.fileTemp != NULL )
     fileTemp= Unconditional::strdup(source.fileTemp);

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getExtension
//
// Purpose-
//       Extract the extension from a complete file name.
//
//----------------------------------------------------------------------------
char*                               // result, or NULL iff error
   FileName::getExtension(          // Get extension portion of a filename
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file name string
   throw()                          // No exceptions
{
   result[0]= '\0';
   const char* CC= getExtension(fileDesc);
   if( strlen(CC) >= FILENAME_MAX )
     return NULL;

   strcpy(result, CC);
   return result;
}

const char*                         // The extension portion of the filename
   FileName::getExtension(          // Get extension portion of a filename
     const char*       fileDesc)    // The path/file name string
   throw()                          // No exceptions
{
   const int           length= strlen(fileDesc);

   const char* result= fileDesc + length - 1;
   while( result >= fileDesc )      // Search for '.' or '/' delimiter
   {
     if( *result == '.' )           // If delimiter found
     {
       if( result == fileDesc )     // Name ".xxxx" has no extension
         return fileDesc + length;

       //---------------------------------------------------------------------
       // The special file names "." and ".." do not have extensions
       const char* name= getNamePart(fileDesc);
       if( strcmp(name, ".") == 0 || strcmp(name, "..") == 0 )
         return fileDesc + length;

       return result;
     }

     if( isPathSep(*result) )
       break;

     result--;
   }

   return fileDesc + length;
}

const char*                         // The extension portion of the fileDesc
   FileName::getExtension( void ) const // Get extension portion of the fileDesc
   throw()                          // No exceptions
{
   const char* result= fileDesc;
   if( result != NULL )
     result= getExtension(result);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getFileName
//
// Purpose-
//       Extract the fully qualified file name
//
//----------------------------------------------------------------------------
const char*                         // The path/file.name descriptor
   FileName::getFileName( void ) const // Get path/file.name descriptor
   throw()                          // No exceptions
{
   return fileDesc;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getNameOnly
//
// Purpose-
//       Extract the name portion of the file descriptor (w/o extension)
//
//----------------------------------------------------------------------------
char*                               // result, or NULL iff error
   FileName::getNameOnly(           // Get file name (w/o extension)
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file.name string
   throw()                          // No exceptions
{
   result[0]= '\0';
   fileDesc= getNamePart(fileDesc);
   if( strlen(fileDesc) >= FILENAME_MAX )
     return NULL;

   strcpy(result, fileDesc);
   const char* nameEnds= getExtension(result);
   if( nameEnds != NULL )
     result[nameEnds-result]= '\0';

   return result;
}

char*                               // result, or NULL iff error
   FileName::getNameOnly(           // Get file name (w/o extension)
     char*             result)      // Resultant of length > FILENAME_MAX
   throw()                          // No exceptions
{
   result[0]= '\0';
   if( fileDesc == NULL )
     return NULL;

   return getNameOnly(result, fileDesc);
}

const char*                         // The file name (w/o extension)
   FileName::getNameOnly( void )    // Get file name (w/o extension)
{
   if( fileTemp != NULL )
   {
     free(fileTemp);
     fileTemp= NULL;
   }

   const char* fileName= getNamePart();
   if( fileName != NULL )
   {
     const char* nameEnds= getExtension(fileName);
     if( nameEnds == NULL || nameEnds == fileName )
       nameEnds= fileName + strlen(fileName);

     size_t length= nameEnds - fileName;
     fileTemp= (char*)Unconditional::malloc(length+1);
     memcpy(fileTemp, fileName, length);
     fileTemp[length]= '\0';
   }

   return fileTemp;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getNamePart
//
// Purpose-
//       Extract the file name from a complete file name.
//
// Notes-
//       This name may be "" but is never NULL.
//
//----------------------------------------------------------------------------
char*                               // result, or NULL iff error
   FileName::getNamePart(           // Get name portion of a filename
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file name string
   throw()                          // No exceptions
{
   result[0]= '\0';
   const char* CC= getNamePart(fileDesc);
   if( strlen(CC) >= FILENAME_MAX )
     return NULL;

   strcpy(result, CC);
   return result;
}

const char*                         // The name portion of the filename
   FileName::getNamePart(           // Get name portion of a filename
     const char*       fileDesc)    // The path/file name string
   throw()                          // No exceptions
{
   const int           length= strlen(fileDesc);

   int index= length;
   int minIndex= getPrefixIndex(fileDesc);
   while( --index > minIndex )
   {
     if( isPathSep(fileDesc[index]) )
       break;
   }

   if( index < minIndex )
     return fileDesc + length;

   return fileDesc + index + 1;
}

const char*                         // The name portion of the filedesc
   FileName::getNamePart( void ) const // Get name portion of a filedesc
   throw()                          // No exceptions
{
   const char* result= fileDesc;
   if( result != NULL )
     result= getNamePart(result);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getPathOnly
//
// Purpose-
//       Extract the path portion of the file descriptor (w/o file name)
//
//----------------------------------------------------------------------------
char*                               // The path portion of fileDesc
   FileName::getPathOnly(           // Get path portion of fileDesc
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file.name string
   throw()                          // No exceptions
{
   const char* fileName= getNamePart(fileDesc);
   int length= (fileName - fileDesc);
   if( length >= FILENAME_MAX )
   {
     result[0]= '\0';
     return NULL;
   }

   memcpy(result, fileDesc, length);
   result[length]= '\0';
   return result;
}

char*                               // The path portion of fileDesc
   FileName::getPathOnly(           // Get path portion of fileDesc
     char*             result)      // Resultant of length > FILENAME_MAX
   throw()                          // No exceptions
{
   result[0]= '\0';
   if( fileDesc == NULL )
     return NULL;

   return getPathOnly(result, fileDesc);
}

const char*                         // The path portion of fileDesc
   FileName::getPathOnly( void )    // Get path portion of fileDesc
{
   if( fileTemp != NULL )
   {
     free(fileTemp);
     fileTemp= NULL;
   }

   if( fileDesc != NULL )
   {
     const char* fileName= getNamePart(fileDesc);
     size_t length= (fileName - fileDesc);
     fileTemp= (char*)Unconditional::malloc(length+1);
     memcpy(fileTemp, fileDesc, length);
     fileTemp[length]= '\0';
   }

   return fileTemp;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getPathSeparator
//
// Purpose-
//       Get the path separator string
//
//----------------------------------------------------------------------------
const char*                         // The path separator string
   FileName::getPathSeparator( void ) // Get path separator string
   throw()                          // No exceptions
{
   return PATH_SEPARATOR;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::getTemporary
//
// Purpose-
//       Retrive the TEMPORARY
//
//----------------------------------------------------------------------------
const char*                         // The TEMPORARY
   FileName::getTemporary( void ) const // Get the TEMPORARY
   throw()                          // No exceptions
{
   return fileTemp;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::append
//
// Purpose-
//       Append a string to the file name.
//
//----------------------------------------------------------------------------
const char*                         // Resultant file name, or NULL
   FileName::append(                // Append to the file name
     const char*       string)      // The string to append
   throw()                          // No exceptions
{
   if( fileDesc == NULL )
     fileDesc= strdup(string);
   else
   {
     size_t length= strlen(fileDesc) + strlen(string);
     char* fileName= (char*)malloc(length + 1);
     if( fileName != NULL )
     {
       strcpy(fileName, fileDesc);
       catstr(fileName, string);
       free(fileDesc);
       fileDesc= fileName;
     }
   }

   return fileDesc;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::appendPath
//
// Purpose-
//       Append a string to the file name as if it were a path name.
//
//----------------------------------------------------------------------------
const char*                         // Resultant file name, or NULL
   FileName::appendPath(            // Append to the file name as if path
     const char*       string)      // The string to append
   throw()                          // No exceptions
{
   if( fileDesc == NULL )
     fileDesc= strdup(string);
   else
   {
     //-----------------------------------------------------------------------
     // Handle the case where fileDesc ends with a '/'
     const char* SEP= PATH_SEPARATOR;
     size_t length= strlen(fileDesc);
     if( length > 0 )
     {
       if( isPathSep(fileDesc[length-1]) )
         SEP= "";
     }

     if( isPathSep(*string) )       // Avoid duplicate separators
       string++;

     length += strlen(SEP) + strlen(string);
     char* fileName= (char*)malloc(length + 1);
     if( fileName != NULL )
     {
       strcpy(fileName, fileDesc);
       catstr(fileName, SEP);
       catstr(fileName, string);
       free(fileDesc);
       fileDesc= fileName;
     }
   }

   return fileDesc;
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::compare
//
// Purpose-
//       Compare file descriptor names.
//
//----------------------------------------------------------------------------
int                                 // Result: (<0, =0, >0)
   FileName::compare(               // Compare file descriptor names
     const char*       L,           // The comparitor name
     const char*       R)           // The comprihend name
   throw()                          // No exceptions
{
   return COMPARE(L, R);
}

int                                 // Result: (<0, =0, >0)
   FileName::compare(               // Compare file descriptor names
     const char*       R) const     // The comprihend name
   throw()                          // No exceptions
{
   return COMPARE(fileDesc, R);
}

int                                 // Result: (<0, =0, >0)
   FileName::compare(               // Compare file descriptor names
     const FileName&   R) const     // The comprihend
   throw()                          // No exceptions
{
   return COMPARE(fileDesc, R.fileDesc);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::concat
//
// Purpose-
//       Concatenate path and file names
//
//----------------------------------------------------------------------------
char*                               // result, or NULL iff length error
   FileName::concat(                // Get extension portion of a filename
     char*             result,      // Resultant
     unsigned long     length,      // Resultant length
     const char*       filePath,    // The path name string
     const char*       fileName)    // The file name string
   throw()                          // No exceptions
{
   result[0]= '\0';
   if( filePath == NULL )
   {
     if( strlen(fileName) >= length )
       return NULL;

     strcpy(result, fileName);
   }
   else
   {
     //-----------------------------------------------------------------------
     // We must handle the case where filePath ends with a '/'
     const char* SEP= PATH_SEPARATOR;
     size_t size= strlen(filePath);
     if( size > 0 )
     {
       if( isPathSep(filePath[size-1]) )
         SEP= "";
     }

     if( isPathSep(*fileName) )
       fileName++;

     size += strlen(SEP) + strlen(fileName);
     if( size >= length )
       return NULL;

     strcpy(result, filePath);
     catstr(result, SEP);
     catstr(result, fileName);
   }

   return result;
}

char*                               // result, or NULL iff length error
   FileName::concat(                // Get extension portion of a filename
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       filePath,    // The path name string
     const char*       fileName)    // The file name string
   throw()                          // No exceptions
{
   return concat(result, FILENAME_MAX, filePath, fileName);
}

//----------------------------------------------------------------------------
//
// Method-
//       FileName::reset
//
// Purpose-
//       Reset the FileName object, deleting fileDesc and fileTemp
//
//----------------------------------------------------------------------------
void
   FileName::reset( void )          // Reset the FileName object
   throw()                          // No exceptions
{
   if( fileTemp != NULL )
   {
     free(fileTemp);
     fileTemp= NULL;
   }

   if( fileDesc != NULL )
   {
     free(fileDesc);
     fileDesc= NULL;
   }
}

const char*                         // The path/file name, or NULL
   FileName::reset(                 // Reset the FileName
     const char*       fileName)    // The absolute path/file name
   throw()                          // No exceptions
{
   reset();

   fileDesc= strdup(fileName);
   return fileDesc;
}

const char*                         // The path/file name, or NULL
   FileName::reset(                 // Reset the FileName
     const char*       filePath,    // The absolute path name (NULL for current)
     const char*       fileName)    // The relative file name
   throw()                          // No exceptions
{
   char                workPath[4096]; // For current directory

   reset();
   if( filePath == NULL )
   {
     if( getcwd(workPath, sizeof(workPath)) == NULL )
       return NULL;

     filePath= workPath;
   }

   //-------------------------------------------------------------------------
   // We must handle the case where filePath ends with a '/'
   const char* SEP= PATH_SEPARATOR;
   size_t size= strlen(filePath);
   if( size > 0 )
   {
     if( isPathSep(filePath[size-1]) )
       SEP= "";
   }

   if( isPathSep(*fileName) )
     fileName++;

   size += strlen(SEP) + strlen(fileName);
   fileDesc= (char*)malloc(size+1);
   if( fileDesc != NULL )
   {
     strcpy(fileDesc, filePath);
     catstr(fileDesc, SEP);
     catstr(fileDesc, fileName);
   }

   return fileDesc;
}

#ifdef _OS_WIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       ::namePrefix
//
// Purpose-
//       Handle file name prefix (WIN version)
//
//----------------------------------------------------------------------------
static const char*                  // NULL iff successful
   namePrefix(                      // Handle file name prefix
     char*             result,      // Resultant, length > FILENAME_MAX
     int&              minIndex,    // Resultant minimum index
     const char*       source)      // The source file name
{
   char                buffer[FILENAME_MAX+1];// Temporary buffer
   const char*         CC;          // Working const char*
   char*               CV;          // Working char*
   const size_t        length= strlen(source); // strlen(source);
   struct stat         statBuff;    // Stat buffer
   unsigned            unit= 0;     // The absolute unit index (invalid)
   ssize_t             x= 0;        // Current source index

   int                 i;

   //-------------------------------------------------------------------------
   // Initialize minIndex resultant
   minIndex= 0;                     // Set minimum index

   //-------------------------------------------------------------------------
   // Handle Unit specifier
   strcpy(result, "*:");            // Default unit identifier (overwritten)
   if( length > 1                   // If a device qualifier exists
       && source[1] == ':' )
   {
     if( toupper(source[0]) < 'A' || toupper(source[0]) > 'Z' )
       return "<Drive";

     result[0]= toupper(source[0]);
     unit= result[0] - 'A' + 1;
     if( _getdcwd(unit, buffer, sizeof(buffer)) == NULL )
       return "<_getdcwd";
     minIndex= x= 2;
   }
   else if( isPathSep(source[0]) && isPathSep(source[1]) ) // If net qualifier
   {
     resetString(result);
     x= getNextPathDelimiterIndex(source, 2);
     if( x < 0 )
       x= length;

     if( (CC= appendString(result, source, x)) != NULL )
       return CC;

     repairName(result);
     int rc= lstat(result, &statBuff); // Get file information
     IFHCDM( tracef("%d= lstat(%s)\n", rc, result); )
     if( rc != 0 )                // If failure
       return "<lstat";

     minIndex= x;
     if( minIndex == length )
       return NULL;
   }
   else if( length > 3              // If a device qualifier exists
       && source[3] == ':' )
   {
     minIndex= 4;
     resetString(result);
     return appendString(result, source);
   }
   else                             // If no unit specifier
   {
     unit= _getdrive();             // Get the default unit index
     result[0]= 'A' + unit - 1;
     minIndex= 2;
   }

   //-------------------------------------------------------------------------
   // Handle working directory
   if( isPathSep(source[x]) )
     x++;
   else
   {
     if( (CV= _getdcwd(unit, buffer, sizeof(buffer))) == NULL )
       return "<_getdcwd";

     if( buffer[strlen(buffer)-1] == '\\' )
       buffer[strlen(buffer)-1]= '\0';

     if( (CC= appendString(result, &buffer[2])) != NULL )
       return CC;
   }

   // Concatenate the path and the file
   CC= appendString(result, "\\");
   if( CC == NULL )
     CC= appendString(result, &source[x]);
   if( CC != NULL )
     return CC;

   return NULL;
}

#else // !_OS_WIN
//----------------------------------------------------------------------------
//
// Subroutine-
//       ::namePrefix
//
// Purpose-
//       Handle file name prefix (BSD version)
//
//----------------------------------------------------------------------------
static const char*                  // NULL iff successful
   namePrefix(                      // Handle file name prefix
     char*             result,      // Resultant, length > FILENAME_MAX
     int&              minIndex,    // Resultant minimum index
     const char*       source)      // The source file name
{
   char                buffer[FILENAME_MAX+1];// Temporary buffer
   const char*         CC;          // Working const char*
   const char*         CV;          // Working const char*
   const int           length= strlen(source);
   int                 x= 0;        // Current source index

   //-------------------------------------------------------------------------
   // Initialize minIndex resultant
   minIndex= 0;                     // Set minimum index

   //-------------------------------------------------------------------------
   // Handle working directory
   resetString(result);

   #ifdef _OS_CYGWIN
     struct stat       statBuff;    // Stat buffer

     if( source[0] == '/' && source[1] == '/' )
     {
       x= getNextPathDelimiterIndex(source, 2);
       if( x < 0 )
         x= length;

       if( (CC= appendString(result, source, x)) != NULL )
         return CC;

       int rc= lstat(result, &statBuff);// Get file information
       IFHCDM( tracef("%d= lstat(%s)\n", rc, result); )
       if( rc != 0 )                // If failure
         return "<lstat";

       minIndex= x;
       if( minIndex == length )
         return NULL;
     }
   #endif

   if( source[x] == '/' )
   {
     if( (CC= appendString(result, &source[x])) != NULL )
       return CC;
   }
   #ifdef HCDM
     else if( x != 0 )
     {
       tracef("%d ShouldNotOccur x(%d) source(%s)\n", __LINE__, x, source);
       throw "Should not occur";
     }
   #endif
   else if( source[0] == '~' )
   {
     int x= 1;
     #if( USE_PASSWD_HOME == USE_PASSWD_HOME_GETENV )
       struct passwd  passBuff;
     #endif
     struct passwd* ptrPasswd= NULL;
     if( source[1] == '/' )
     {
       #if( USE_PASSWD_HOME == USE_PASSWD_HOME_GETENV )
         ptrPasswd= &passBuff;
         ptrPasswd->pw_dir= getenv("HOME");
         IFHCDM( tracef("%s= getenv(HOME)\n", ptrPasswd->pw_dir); )

         CC= "<getenv";             // In case error
         if( ptrPasswd->pw_dir == NULL )
           ptrPasswd= NULL;
       #elif( USE_PASSWD_HOME == USE_PASSWD_HOME_GETUID )
         ptrPasswd= getpwuid(geteuid());
         IFHCDM( tracef("%p= getpwuid(%ld)\n", ptrPasswd, (long)geteuid()); )

         CC= "<getpwuid";           // In case error
       #endif
     }
     else
     {
       CV= strchr(source+1, '/');
       if( CV == NULL )
         CV= (char*)source + length;

       x= (CV - source);
       resetString(buffer);
       if( (CC= appendString(buffer, source + 1, x-1)) != NULL )
         return CC;

       ptrPasswd= getpwnam(buffer);
       IFHCDM( tracef("%p= getpwnam(%s)\n", ptrPasswd, buffer); )

       CC= "<getpwnam";             // In case error
     }

     if( ptrPasswd == NULL )
     {
       appendString(result, source, x);
       return CC;
     }

     if( (CC= appendString(result, ptrPasswd->pw_dir)) != NULL )
       return CC;

     if( (CC= appendString(result, &source[x])) != NULL )
       return CC;
   }
   else
   {
     CV= getcwd(buffer, sizeof(buffer));
     if( CV == NULL )
       return "<getcwd";

     CC= appendString(result, buffer);
     if( CC == NULL )
       CC= appendString(result, "/");
     if( CC == NULL )
       CC= appendString(result, source);
     if( CC != NULL )
       return CC;
   }

   return NULL;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       FileName::resolve
//
// Purpose-
//       Resolve a fileDesc, adding home directory and removing links
//
//----------------------------------------------------------------------------
const char*                         // NULL iff valid
   FileName::resolve(               // Resolve path/file name
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // Input fileName
   throw()                          // No exceptions
{
   char                buffer[FILENAME_MAX+1];// Temporary buffer
   char                source[FILENAME_MAX+1]; // Working source filename
   char                target[FILENAME_MAX+1]; // Working target filename

   const char*         CC;          // Working const char*
   int                 minIndex;    // The minimum index
   unsigned            symCount;    // Symbolic link counter
   struct stat         statBuff;    // Stat buffer

   int                 rc;

   IFHCDM( tracef("%d FileName::resolve(*,%s)\n", __LINE__, fileDesc); )

   //-------------------------------------------------------------------------
   // As a side-effect, on error we set result to the partial fileName
   resetString(result);

   if( fileDesc == NULL )
     return "<NULL";

   if( strlen(fileDesc) >= FILENAME_MAX )
     return "<FILENAME_MAX";

   //-------------------------------------------------------------------------
   // Handle working directory
   if( (CC= namePrefix(result, minIndex, fileDesc)) != NULL )
     return CC;

   strcpy(source, result);
   repairName(source);
   resetString(result);

   //-------------------------------------------------------------------------
   // Verify path structure removing symbolic links
   int ddIndex= (-1);               // .. index
   int loIndex= minIndex + 1;       // Last '/' index (+1)
   int hiIndex;                     // Next '/' after loIndex
   int length;                      // source.length()
   for(symCount= 0;;)
   {
     IFHCDM( tracef("%4d Source(%s) %d\n", __LINE__, source, symCount); )

     for(;;)                        // Search for links
     {
       length= strlen(source);
       if( loIndex >= length )
       {
         strcpy(result, source);
//       repairName(result);
         return NULL;
       }

       hiIndex= getNextPathDelimiterIndex(source,loIndex);
       if( hiIndex < 0 )
         hiIndex= length;

       #if defined(HCDM) && FALSE
         tracef("HCDM(%d) ddX(%d) loX(%d) hiX(%d) len(%d) '%s'\n", __LINE__,
                ddIndex, loIndex, hiIndex, length, source);
       #endif

       //---------------------------------------------------------------------
       // Handle special filenames "." and ".."
       if( (hiIndex-loIndex) == 1
           && source[loIndex] == '.' )
       {
         source[loIndex]= '\0';
         if( hiIndex < length )
         {
           hiIndex++;
           appendString(source, &source[hiIndex]);
         }

         //-------------------------------------------------------------------
         // Restart after /. removal
         IFHCDM( tracef("%4d Source(%s) %d [/.]\n", __LINE__, source, symCount); )

         continue;
       }

       if( (hiIndex-loIndex) == 2
           && source[loIndex] == '.' && source[loIndex+1] == '.' )
       {
         if( ddIndex < minIndex )
           return "</../ exception";

         source[ddIndex]= '\0';
         if( hiIndex < length )
         {
           hiIndex++;
           appendString(source, &source[hiIndex]);
         }

         //-------------------------------------------------------------------
         // Restart after /.. removal
         IFHCDM( tracef("%4d Source(%s) %d [/..]\n", __LINE__, source, symCount); )

         loIndex= ddIndex--;
         while( (--ddIndex) >= 0 )
         {
           if( isPathSep(source[ddIndex]) )
           {
             ddIndex++;
             break;
           }
         }
         continue;
       }

       //---------------------------------------------------------------------
       // Partial name must be a link or a valid path
       memcpy(result, source, hiIndex);
       result[hiIndex]= '\0';
//     repairName(result);
       rc= lstat(result, &statBuff);// Get file information
       IFHCDM( tracef("%d= lstat(%s)\n", rc, result); )
       if( rc != 0 )                // If failure
       {
         if( hiIndex >= length )    // The last name need not exist
           return NULL;

         return result;
       }

       if( S_ISLNK(statBuff.st_mode) ) // If this is a link
         break;

       ddIndex= loIndex;
       loIndex= hiIndex + 1;
     }

     //-----------------------------------------------------------------------
     // The name is a link name
     symCount++;
     if( symCount >= MAX_SYMLINK )
       break;

     rc= readlink(result, buffer, sizeof(buffer) - 1);
     IFHCDM(
       buffer[(rc < 0) ? 0 : rc]= '\0';
       tracef("%d= readlink(%s)='%s'\n", rc, result, buffer);
     )
     if( rc < 0 )                   // If failure
       return "<readlink";
     buffer[rc]= '\0';

     if( isPathSep(buffer[0]) )     // If restart
     {
       strcpy(target, buffer);
       if( (CC= appendString(target, &source[hiIndex])) != NULL )
         return CC;

       if( (CC= namePrefix(result, minIndex, target)) != NULL )
         return CC;

       strcpy(target, result);
       resetString(result);

       ddIndex= (-1);               // .. index
       loIndex= minIndex + 1;       // Last '/' index (+1)
     }
     else
     {
       strcpy(target, source);
       target[loIndex]= '\0';
       CC= appendString(target, buffer);
       if( CC == NULL )
         CC= appendString(target, &source[hiIndex]);

       if( CC != NULL )
         return CC;
     }

     strcpy(source, target);
   }

   return "<MAX_SYMLINK";
}

const char*                         // NULL iff successful
   FileName::resolve(               // Resolve file descriptor, removing links
     char*             result)      // Resultant of length > FILENAME_MAX
   throw()                          // No exceptions
{
   return resolve(result, fileDesc);
}

const char*                         // NULL iff successful
   FileName::resolve( void )        // Resolve file descriptor, removing links
{
   char                buffer[FILENAME_MAX+1]; // Working buffer

   //-------------------------------------------------------------------------
   // As a side-effect, on error we set fileTemp to the partial fileName
   if( fileTemp != NULL )
   {
     free(fileTemp);
     fileTemp= NULL;
   }

   const char* result= resolve(buffer, fileDesc);
   if( result == NULL )
   {
     char* CV= Unconditional::strdup(buffer);
     free(fileDesc);
     fileDesc= CV;
   }
   else if( buffer[0] != '\0' )
   {
     fileTemp= Unconditional::strdup(buffer);
     if( result == buffer )
       result= fileTemp;
   }

   return result;
}

