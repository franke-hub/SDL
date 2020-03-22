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
//       FileName.h
//
// Purpose-
//       Parse a filename into its components.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef FILENAME_H_INCLUDED
#define FILENAME_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       FileName
//
// Purpose-
//       Get information about a potential file name.
//
// Notes-
//       The special file names "." and ".." do not contain an extension.
//       A file name of the form .xxxx (with no other '.') has no extension.
//
//       The FileName object has no restrictions on path or name lengths.
//       The only method which references the file system is resolve().
//       Method resolve requires all protions of the filename fit within
//       the stdio.h FILENAME_MAX limit.
//
//       The FileName object generally requires exception handling since
//       some object methods throw a "Storage shortage" exception.
//       The FileName static methods provide equivalent function and
//       none throw exceptions. However, some static methods require the
//       result fit within the stdio.h FILENAME_MAX limit.
//
// Method compare-
//       This yields the same result as strcmp or stricmp, depending upon
//       whether the operating system differentiates file name case.
//
// Method resolve-
//       Returns NULL if successful and "<text" if an exception is detected,
//       or result if the fileDesc name could only be partially resolved.
//       (Note that this is the inverse of most other methods.)
//       On error, the object method sets the TEMPORARY to the invalid part
//       of the fileDesc (or throws "Storage shortage".)
//
// Unix examples-
//       FileName("foo.x") from current directory "/u/user/temp", where
//       /u -> /home
//       /home/user/temp -> /local/user/temp
//       /local/user/temp/foo.x -> bar.y
//
//       Before resolve:
//       getFileName(): "foo.x"
//       getPathOnly(): [temporary] ""
//       getNamePart(): "foo.x"
//       getNameOnly(): [temporary] "foo"
//       getExtension(): ".x"
//
//       After resolve:
//       getFileName(): "/local/user/temp/bar.y"
//       getPathOnly(): [temporary] "/local/user/temp/"
//       getNamePart(): "bar.y"
//       getNameOnly(): [temporary] "bar"
//       getExtension(): ".y"
//
// Win examples-
//       FileName("foo.x") from directory "D:\temp", where
//       C: Current directory "C:\"
//       D: Current directory "D:\temp"
//
//       Before resolve:
//       getFileName(): "foo.x"
//       getPathOnly(): [temporary] ""
//       getNamePart(): "foo.x"
//       getNameOnly(): [temporary] "foo"
//       getExtension(): ".x"
//
//       After resolve:
//       getFileName(): "D:\temp\foo.x"
//       getPathOnly(): [temporary] "D:\temp\"
//       getNamePart(): "foo.x"
//       getNameOnly(): [temporary] "foo"
//       getExtension(): ".x"
//
// Win examples-
//       FileName("c:foo.x") from directory "D:\temp", where
//       C: Current directory "C:\"
//       D: Current directory "D:\temp"
//
//       Before resolve:
//       getFileName(): "c:foo.x"
//       getPathOnly(): [temporary] "c:"
//       getNamePart(): "foo.x"
//       getNameOnly(): [temporary] "foo"
//       getExtension(): ".x"
//
//       After resolve:
//       getFileName(): "C:\foo.x"
//       getPathOnly(): [temporary] "C:\"
//       getNamePart(): "foo.x"
//       getNameOnly(): [temporary] "foo"
//       getExtension(): ".x"
//
//----------------------------------------------------------------------------
class FileName {
//----------------------------------------------------------------------------
// FileName::Attributes
//----------------------------------------------------------------------------
protected:
char*                  fileDesc;    // The path/file name string
char*                  fileTemp;    // Temporary resultant

//----------------------------------------------------------------------------
// FileName::Constructors
//----------------------------------------------------------------------------
public:
   ~FileName( void ) throw();       // Default destructor
   FileName( void ) throw();        // Default constructor

   FileName(                        // Construct, setFileDesc
     const char*       fileDesc);   // The absolute path/file name

   FileName(                        // Construct, setFileDesc
     const char*       filePath,    // The absolute path name (NULL for current)
     const char*       fileName);   // The relative file name

public:                             // Copy and assignment ARE allowed
   FileName(const FileName&);       // Bitwise copy allowed

FileName&
   operator=(const FileName&);      // Bitwise assignment allowed

//----------------------------------------------------------------------------
// FileName::special accessors (using TEMPORARY)
//   These methods allocate storage (using malloc) to contain the resultant.
//   This allocated storage is maintained internally within the FileName
//   object, and is kept until another special accessor method is called.
//
//   The static methods with the same name use the supplied TEMPORARY result.
//----------------------------------------------------------------------------
public:
// STATIC: Stores Name (only) in result
static char*                        // result, or NULL iff error
   getNameOnly(                     // Get the file name (w/o extension)
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file.name string
   throw();                         // No exceptions

// OBJECT: Stores Name (only) in result
char*                               // result, or NULL iff error
   getNameOnly(                     // Get the file name (w/o extension)
     char*             result)      // Resultant of length > FILENAME_MAX
   throw();                         // No exceptions

// OBJECT: Stores Name (only) in TEMPORARY
const char*                         // The file name (w/o extension)
   getNameOnly( void );             // Get file name (w/o extension)

// STATIC: Stores Path (only) in result
static char*                        // result, or NULL iff error
   getPathOnly(                     // Get path name (w/o file.name)
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file.name string
   throw();                         // No exceptions

// OBJECT: Stores Path (only) in result
char*                               // result, or NULL iff error
   getPathOnly(                     // Get path name (w/o file.name)
     char*             result)      // Resultant of length > FILENAME_MAX
   throw();                         // No exceptions

// OBJECT: Stores Path (only) in TEMPORARY
const char*                         // The path name (w/o file.name)
   getPathOnly( void );             // Get path name (w/o file.name)

// OBJECT: Returns TEMPORARY, for example after resolve()
const char*                         // The TEMPORARY
   getTemporary( void ) const       // Get TEMPORARY
   throw();                         // No exceptions

// STATIC: Stores working path name in result
static const char*                  // NULL iff successful
   resolve(                         // Resolve file name, removing links
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file name string
   throw();                         // No exceptions

// OBJECT: Stores working path name in result
const char*                         // NULL iff successful
   resolve(                         // Resolve file name, removing links
     char*             result)      // Resultant of length > FILENAME_MAX
   throw();                         // No exceptions

// OBJECT: Stores working path name in TEMPORARY
const char*                         // NULL iff successful
   resolve( void );                 // Resolve file name, removing links

//----------------------------------------------------------------------------
// FileName::accessors
//----------------------------------------------------------------------------
public:
static char*                        // result, or NULL iff error
   getExtension(                    // Get extension portion of fileDesc
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file.name string
   throw();                         // No exceptions

static const char*                  // The extension portion of fileDesc
   getExtension(                    // Get extension portion of a name
     const char*       fileDesc)    // The path/file.name string
   throw();                         // No exceptions

const char*                         // The extension portion of fileDesc
   getExtension( void ) const       // Get extension portion of the name
   throw();                         // No exceptions

const char*                         // The complete path/file.name
   getFileName( void ) const        // Get complete path/file.name
   throw();                         // No exceptions

static char*                        // result, or NULL iff error
   getNamePart(                     // Get filename portion of a name
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       fileDesc)    // The path/file name string
   throw();                         // No exceptions

static const char*                  // The file.name portion of fileDesc
   getNamePart(                     // Get file.name portion of a name
     const char*       fileDesc)    // The path/file.name string
   throw();                         // No exceptions

const char*                         // The file.name portion of fileDesc
   getNamePart( void ) const        // Get file.name portion of the name
   throw();                         // No exceptions

static const char*                  // The path separator character
   getPathSeparator( void )         // Get path separator character
   throw();                         // No exceptions

//----------------------------------------------------------------------------
// FileName::methods
//----------------------------------------------------------------------------
public:
const char*                         // The full path/file name, or NULL
   append(                          // Append to the file name
     const char*       string)      // The string to append
   throw();                         // No exceptions

const char*                         // The full path/file name, or NULL
   appendPath(                      // Append to the file name, as if path
     const char*       string)      // The string to append
   throw();                         // No exceptions

static int                          // Result: (<0, =0, >0)
   compare(                         // Compare file descriptor names
     const char*       L,           // Comparitor fileDesc name
     const char*       R)           // Comprihend fileDesc name
   throw();                         // No exceptions

int                                 // Result: (<0, =0, >0)
   compare(                         // Compare with this fileDesc name
     const char*       R) const     // Comprahend
   throw();                         // No exceptions

int                                 // Result: (<0, =0, >0)
   compare(                         // Compare with chis fileDesc name
     const FileName&   R) const     // Comprahend
   throw();                         // No exceptions

static char*                        // result, or NULL iff length error
   concat(                          // Concatenate
     char*             result,      // Resultant
     unsigned long     length,      // Resultant length
     const char*       filePath,    // The path name string
     const char*       fileName)    // The file name string
   throw();                         // No exceptions

static char*                        // result, or NULL iff length error
   concat(                          // Concatenate
     char*             result,      // Resultant of length > FILENAME_MAX
     const char*       filePath,    // The path name string
     const char*       fileName)    // The file name string
   throw();                         // No exceptions

void
   reset( void )                    // RESET (empty) the FileName object
   throw();                         // No exceptions

const char*                         // The complete path/file name, or NULL
   reset(                           // Reset the FileName
     const char*       fileDesc)    // The absolute path/file name
   throw();                         // No exceptions

const char*                         // The complete path/file name, or NULL
   reset(                           // Reset the FileName
     const char*       filePath,    // The absolute path name
     const char*       fileName)    // The file name
   throw();                         // No exceptions
}; // class FileName

#endif // FILENAME_H_INCLUDED
