//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Reader.h
//
// Purpose-
//       Define the Reader object.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Reader defines a buffer and a set of input access methods.
//       MediaReader implements Reader for any Media.
//       FileReader implements Reader using an internal FileMedia.
//       LineReader extends FileReader, adding a line and column counter.
//
// Exceptions-
//       (const char*) "InvalidStateException"
//       (const char*) "NullPointerException"
//
//----------------------------------------------------------------------------
#ifndef READER_H_INCLUDED
#define READER_H_INCLUDED

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

#ifndef MEDIA_H_INCLUDED
#include "Media.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Reader
//
// Purpose-
//       Reader defines a buffer and a set of input access methods.
//
//----------------------------------------------------------------------------
class Reader : public virtual MediaType {   // Reader
//----------------------------------------------------------------------------
// Reader::Attributes
//----------------------------------------------------------------------------
protected:
Byte*                  buffer;      // The buffer
Size_t                 length;      // The length of the buffer
Size_t                 size;        // The number of valid bytes
Size_t                 used;        // The number of used bytes

//----------------------------------------------------------------------------
// Reader::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Reader( void );                 // Destructor
   Reader( void );                  // Default constructor
   Reader(                          // Value constructor
     Size_t            size);       // The size of the buffer

private:                            // Bitwise copy is prohibited
   Reader(const Reader&);           // Disallowed copy constructor
Reader&
   operator=(const Reader&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Reader::Pure virtual methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the Reader
     const char*       name,        // The Reader name
     const char*       mode= Media::MODE_READ) = 0; // The open mode

virtual int                         // Return code (0 OK)
   close( void ) = 0;               // Close the Reader

virtual int                         // Return code (0 OK)
   flush( void ) = 0;               // Flush the Reader

protected:
virtual int                         // Return code (0 OK)
   input( void ) = 0;               // Read input

//----------------------------------------------------------------------------
// Reader::Accessor methods
//----------------------------------------------------------------------------
public:
virtual Size_t                      // The available buffer length
   getAvail( void );                // Get available buffer length

virtual Size_t                      // The buffer length
   getLength( void );               // Get buffer length

virtual void
   reset( void );                   // Reset the Reader

virtual void
   resize(                          // Resize the Reader
     Size_t            size);       // The new length

//----------------------------------------------------------------------------
// Reader::Methods
//----------------------------------------------------------------------------
public:
virtual void
   fill( void );                    // Fill the Reader

virtual int                         // The next input character
   get( void );                     // Get the next character

virtual int                         // The prior input character
   prior( void ) const;             // Get the prior character

virtual const Byte*                 // The input data
   pull(                            // Pull input data
     Size_t            size);       // Of this length

virtual Size_t                      // The number of data bytes read
   read(                            // Read input data
     Byte*             addr,        // To this address
     Size_t            size);       // For this length

virtual int                         // The next blank character
   findBlank( void );               // Skip to the next blank character

virtual int                         // The next non-blank character
   skipBlank( void );               // Skip to the next non-blank character

virtual int                         // The line delimiter
   readLine(                        // Read a line (ignoring '\r' characters)
     Byte*             addr,        // To this address
     Size_t            size);       // Of this maximum length

virtual int                         // The line delimiter
   skipLine( void );                // Skip a line (ignoring '\r' characters)
}; // class Reader

//----------------------------------------------------------------------------
//
// Class-
//       MediaReader
//
// Purpose-
//       Implement Reader for any Media.
//
//----------------------------------------------------------------------------
class MediaReader : public Reader { // MediaReader
//----------------------------------------------------------------------------
// MediaReader::Attributes
//----------------------------------------------------------------------------
protected:
Media*                 media;       // The Media
State                  state;       // The open State

//----------------------------------------------------------------------------
// MediaReader::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~MediaReader( void );            // Destructor
   MediaReader( void );             // Default constructor
   MediaReader(                     // Value constructor
     Size_t            size);       // The size of the MediaReader

private:                            // Bitwise copy is prohibited
   MediaReader(const MediaReader&); // Disallowed copy constructor
MediaReader&
   operator=(const MediaReader&);   // Disallowed assignment operator

//----------------------------------------------------------------------------
// MediaReader::Methods
//----------------------------------------------------------------------------
public:
virtual void
   attach(                          // Attach Media to MediaReader
     Media&            media);      // The new Media

virtual void
   detach( void );                  // Detach Media

//----------------------------------------------------------------------------
// MediaReader::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the MediaReader
     const char*       name,        // The MediaReader name
     const char*       mode= Media::MODE_READ); // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the MediaReader

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the MediaReader

protected:
virtual int                         // Return code (0 OK)
   input( void );                   // Read input
}; // class MediaReader

//----------------------------------------------------------------------------
//
// Class-
//       FileReader
//
// Purpose-
//       Implement Reader using an internal FileMedia.
//
//----------------------------------------------------------------------------
class FileReader : public Reader {  // FileReader
//----------------------------------------------------------------------------
// FileReader::Attributes
//----------------------------------------------------------------------------
protected:
FileMedia              media;       // The FileMedia
State                  state;       // The open State

//----------------------------------------------------------------------------
// FileReader::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FileReader( void );             // Destructor
   FileReader( void );              // Default constructor
   FileReader(                      // Value constructor
     const char*       name);       // The Media name

private:                            // Bitwise copy is prohibited
   FileReader(const FileReader&);   // Disallowed copy constructor
FileReader&
   operator=(const FileReader&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// FileReader::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the FileReader
     const char*       name,        // The FileReader name
     const char*       mode= Media::MODE_READ); // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the FileReader

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the FileReader

protected:
virtual int                         // Return code (0 OK)
   input( void );                   // Read input
}; // class FileReader

//----------------------------------------------------------------------------
//
// Class-
//       LineReader
//
// Purpose-
//       Extend FileReader, adding a line and column counter.
//
// Notes-
//       The line and column counters are valid only when get() is the ONLY
//       method used to retrieve data. Methods findBlank(), skipBlank(),
//       readLine() and skipLine() can be used since they use get().
//
//       Line and column numbers begin at 1. The first '\n' character in the
//       file occurs in line 1.
//
//       Methods getLine() and getColumn() return the line and column number
//       after the LAST get(). When a '\n' is encountered, the line number is
//       incremented and the column number is set to 0.
//
//       The '\r' (carriage return character) is never returned, but does set
//       the column number to 0. The '\n' (new line character) is returned
//       when detected.
//
//----------------------------------------------------------------------------
class LineReader : public FileReader { // LineReader
//----------------------------------------------------------------------------
// LineReader::Attributes
//----------------------------------------------------------------------------
protected:
unsigned long          line;        // The current line number
unsigned long          column;      // The current column number

//----------------------------------------------------------------------------
// LineReader::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~LineReader( void );             // Destructor
   LineReader( void );              // Default constructor
   LineReader(                      // Value constructor
     const char*       name);       // The Media name

private:                            // Bitwise copy is prohibited
   LineReader(const LineReader&);   // Disallowed copy constructor
LineReader&
   operator=(const LineReader&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// LineReader::Accessor methods
//----------------------------------------------------------------------------
public:
virtual unsigned long               // The file column
   getColumn( void ) const;         // Get file column

virtual unsigned long               // The file line
   getLine( void ) const;           // Get file line

//----------------------------------------------------------------------------
// LineReader::Implementation methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   open(                            // Open the FileReader
     const char*       name,        // The FileReader name
     const char*       mode= Media::MODE_READ); // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the FileReader

//----------------------------------------------------------------------------
// LineReader::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // The next character
   get( void );                     // Read the next character
}; // class LineReader

#endif // READER_H_INCLUDED
