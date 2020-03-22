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
//       Buffer.h
//
// Purpose-
//       Define the Buffer object.
//
// Last change date-
//       2007/01/01
//
// Notes-
//       Buffer combines the Reader and Writer objects.
//       MediaBuffer implements Buffer for any Media.
//       FileBuffer implements Buffer using an internal FileMedia.
//       TempBuffer implements Buffer using an internal TempMedia.
//
// Exceptions-
//       (const char*) "InvalidStateException"
//       (const char*) "NullPointerException"
//       (const char*) "OutputException"
//
//----------------------------------------------------------------------------
#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

#ifndef READER_H_INCLUDED
#include "Reader.h"
#endif

#ifndef WRITER_H_INCLUDED
#include "Writer.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Buffer
//
// Purpose-
//       Buffer combines the Reader and Writer objects.
//
//----------------------------------------------------------------------------
class Buffer : public Reader, public Writer { // Buffer
//----------------------------------------------------------------------------
// Buffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Buffer( void );                 // Destructor
   Buffer( void );                  // Default constructor
   Buffer(                          // Value constructor
     Size_t            size);       // The size of the buffer

private:                            // Bitwise copy is prohibited
   Buffer(const Buffer&);           // Disallowed copy constructor
Buffer&
   operator=(const Buffer&);        // Disallowed assignment operator

//----------------------------------------------------------------------------
// Buffer::Pure virtual methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the Buffer
     const char*       name,        // The Buffer name
     const char*       mode) = 0;   // The open mode

virtual int                         // Return code (0 OK)
   close( void ) = 0;               // Close the Buffer

virtual int                         // Return code (0 OK)
   flush( void ) = 0;               // Flush the Buffer

protected:
virtual int                         // Return code (0 OK)
   input( void ) = 0;               // Read input

virtual int                         // Return code (0 OK)
   output( void ) = 0;              // Write output

//----------------------------------------------------------------------------
// Buffer::Accessor methods
//----------------------------------------------------------------------------
public:
virtual Size_t                      // The available buffer length
   getAvail( void );                // Get available buffer length

virtual Size_t                      // The buffer length
   getLength( void );               // Get buffer length

virtual void
   reset( void );                   // Reset the Buffer

virtual void
   resize(                          // Resize the Buffer
     Size_t            size);       // The new length
}; // class Buffer

//----------------------------------------------------------------------------
//
// Class-
//       MediaBuffer
//
// Purpose-
//       Implement Buffer for any Media.
//
//----------------------------------------------------------------------------
class MediaBuffer : public Buffer { // MediaBuffer
//----------------------------------------------------------------------------
// MediaBuffer::Attributes
//----------------------------------------------------------------------------
protected:
Media*                 media;       // The Media
State                  state;       // The open State

//----------------------------------------------------------------------------
// MediaBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~MediaBuffer( void );            // Destructor
   MediaBuffer( void );             // Default constructor
   MediaBuffer(                     // Value constructor
     Size_t            size);       // The size of the MediaBuffer

private:                            // Bitwise copy is prohibited
   MediaBuffer(const MediaBuffer&); // Disallowed copy constructor
MediaBuffer&
   operator=(const MediaBuffer&);   // Disallowed assignment operator

//----------------------------------------------------------------------------
// MediaBuffer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   attach(                          // Attach Media to MediaBuffer
     Media&            media);      // The new Media

virtual void
   detach( void );                  // Detach Media

//----------------------------------------------------------------------------
// MediaBuffer::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the MediaBuffer
     const char*       name,        // The MediaBuffer name
     const char*       mode);       // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the MediaBuffer

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the MediaBuffer

protected:
virtual int                         // Return code (0 OK)
   input( void );                   // Read input

virtual int                         // Return code (0 OK)
   output( void );                  // Write output
}; // class MediaBuffer

//----------------------------------------------------------------------------
//
// Class-
//       FileBuffer
//
// Purpose-
//       Implement Buffer using an internal FileMedia.
//
//----------------------------------------------------------------------------
class FileBuffer : public Buffer {  // FileBuffer
//----------------------------------------------------------------------------
// FileBuffer::Attributes
//----------------------------------------------------------------------------
protected:
FileMedia              media;       // The Media
State                  state;       // The open State

//----------------------------------------------------------------------------
// FileBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FileBuffer( void );             // Destructor
   FileBuffer( void );              // Default constructor

private:                            // Bitwise copy is prohibited
   FileBuffer(const FileBuffer&);   // Disallowed copy constructor
FileBuffer&
   operator=(const FileBuffer&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// FileBuffer::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the FileBuffer
     const char*       name,        // The FileBuffer name
     const char*       mode);       // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the FileBuffer

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the FileBuffer

protected:
virtual int                         // Return code (0 OK)
   input( void );                   // Read input

virtual int                         // Return code (0 OK)
   output( void );                  // Write output
}; // class FileBuffer

//----------------------------------------------------------------------------
//
// Class-
//       TempBuffer
//
// Purpose-
//       Implement Buffer using an internal TempMedia.
//
//----------------------------------------------------------------------------
class TempBuffer : public Buffer {  // TempBuffer
//----------------------------------------------------------------------------
// TempBuffer::Attributes
//----------------------------------------------------------------------------
protected:
TempMedia              media;       // The Media
State                  state;       // The open State

//----------------------------------------------------------------------------
// TempBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TempBuffer( void );             // Destructor
   TempBuffer( void );              // Default constructor

private:                            // Bitwise copy is prohibited
   TempBuffer(const TempBuffer&);   // Disallowed copy constructor
TempBuffer&
   operator=(const TempBuffer&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// TempBuffer::Implementation methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Open the TempBuffer
     const char*       name,        // The TempBuffer name
     const char*       mode);       // The open mode

virtual int                         // Return code (0 OK)
   close( void );                   // Close the TempBuffer

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the TempBuffer

virtual void
   truncate( void );                // Truncate the TempBuffer

protected:
virtual int                         // Return code (0 OK)
   input( void );                   // Read input

virtual int                         // Return code (0 OK)
   output( void );                  // Write output
}; // class TempBuffer

#endif // BUFFER_H_INCLUDED
