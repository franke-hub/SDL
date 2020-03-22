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
//       Writer.h
//
// Purpose-
//       Define the Writer object.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Writer defines a buffer and a set of output access methods.
//       MediaWriter implements Writer for any Media.
//       FileWriter implements Writer using an internal FileMedia.
//
// Exceptions-
//       (const char*) "InvalidStateException"
//       (const char*) "NullPointerException"
//       (const char*) "OutputException"
//
//----------------------------------------------------------------------------
#ifndef WRITER_H_INCLUDED
#define WRITER_H_INCLUDED

#include <stdarg.h>

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

#ifndef MEDIA_H_INCLUDED
#include "Media.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Writer
//
// Purpose-
//       Writer defines a buffer and a set of output access methods.
//
//----------------------------------------------------------------------------
class Writer : public virtual MediaType {   // Writer
//----------------------------------------------------------------------------
// Writer::Attributes
//----------------------------------------------------------------------------
protected:
Byte*                  buffer;      // The buffer
Size_t                 length;      // The length of the buffer
Size_t                 size;        // The number of valid bytes

//----------------------------------------------------------------------------
// Writer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Writer( void );                 // Destructor
   Writer( void );                  // Default constructor
   Writer(                          // Value constructor
     Size_t            size);       // The size of the buffer

private:                            // Bitwise copy is prohibited
   Writer(const Writer&);           // Disallowed copy constructor
Writer&
   operator=(const Writer&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Writer::Pure virtual methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the Writer
     const char*       name,        // The Writer name
     const char*       mode= Media::MODE_WRITE) = 0; // The open mode

virtual int                         // Return code (0 OK)
   close( void ) = 0;               // Close the Writer

virtual int                         // Return code (0 OK)
   flush( void ) = 0;               // Flush the Writer

protected:
virtual int                         // Return code (0 OK)
   output( void ) = 0;              // Write output

//----------------------------------------------------------------------------
// Writer::Accessor methods
//----------------------------------------------------------------------------
public:
virtual Size_t                      // The available buffer length
   getAvail( void );                // Get available buffer length

virtual Size_t                      // The buffer length
   getLength( void );               // Get buffer length

virtual void
   reset( void );                   // Reset the Writer

virtual void
   resize(                          // Resize the Writer
     Size_t            size);       // The new length

//----------------------------------------------------------------------------
// Writer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   vprintf(                         // Print into the Writer
     const char*       fmt,         // Format string
     va_list           argptr)      // VALIST
   _ATTRIBUTE_PRINTF(2,0);          // PRINTF arguments

virtual void
   printf(                          // Print into the Writer
     const char*       fmt,         // Format string
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(2,3);          // PRINTF arguments

virtual Byte*                       // -> Data area (to be filled)
   push(                            // Push into the Writer
     Size_t            size);       // For this length

virtual void
   put(                             // Put the next character
     int               C);          // The next character

virtual Size_t                      // The number of bytes written
   write(                           // Write data
     const Byte*       addr,        // From this address
     Size_t            size);       // For this length

virtual void
   writeLine(                       // Write a NULL-delimited line, then '\n'
     const Byte*       addr);       // From this address
}; // class Writer

//----------------------------------------------------------------------------
//
// Class-
//       MediaWriter
//
// Purpose-
//       Implement Writer for any Media.
//
//----------------------------------------------------------------------------
class MediaWriter : public Writer { // MediaWriter
//----------------------------------------------------------------------------
// MediaWriter::Attributes
//----------------------------------------------------------------------------
protected:
Media*                 media;       // The Media
State                  state;       // The open State

//----------------------------------------------------------------------------
// MediaWriter::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~MediaWriter( void );            // Destructor
   MediaWriter( void );             // Default constructor
   MediaWriter(                     // Value constructor
     Size_t            size);       // The size of the MediaWriter

private:                            // Bitwise copy is prohibited
   MediaWriter(const MediaWriter&); // Disallowed copy constructor
MediaWriter&
   operator=(const MediaWriter&);   // Disallowed assignment operator

//----------------------------------------------------------------------------
// MediaWriter::Methods
//----------------------------------------------------------------------------
public:
virtual void
   attach(                          // Attach Media to MediaWriter
     Media&            media);      // The new Media

virtual void
   detach( void );                  // Detach Media

//----------------------------------------------------------------------------
// MediaWriter::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the MediaWriter
     const char*       name,        // The MediaWriter name
     const char*       mode= Media::MODE_WRITE); // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the MediaWriter

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the MediaWriter

protected:
virtual int                         // Return code (0 OK)
   output( void );                  // Write output
}; // class MediaWriter

//----------------------------------------------------------------------------
//
// Class-
//       FileWriter
//
// Purpose-
//       Implement Writer using an internal FileMedia.
//
//----------------------------------------------------------------------------
class FileWriter : public Writer {  // FileWriter
//----------------------------------------------------------------------------
// FileWriter::Attributes
//----------------------------------------------------------------------------
protected:
FileMedia              media;       // The FileMedia
State                  state;       // The open State

//----------------------------------------------------------------------------
// FileWriter::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FileWriter( void );             // Destructor
   FileWriter( void );              // Default constructor
   FileWriter(                      // Value constructor
     const char*       name);       // The Media name

private:                            // Bitwise copy is prohibited
   FileWriter(const FileWriter&);   // Disallowed copy constructor
FileWriter&
   operator=(const FileWriter&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// FileWriter::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the FileWriter
     const char*       name,        // The FileWriter name
     const char*       mode= Media::MODE_WRITE); // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the FileWriter

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the FileWriter

protected:
virtual int                         // Return code (0 OK)
   output( void );                  // Write output
}; // class FileWriter

#endif // WRITER_H_INCLUDED
