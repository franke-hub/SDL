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
//       Media.h
//
// Purpose-
//       Describe the Media Interface.
//
// Last change date-
//       2007/01/01
//
// Exceptions-
//       const char* "InvalidArgumentException"
//       const char* "MediaIOException"
//       const char* "MediaStateException"
//
//       END_OF_FILE state does not cause an exception.
//       An END_OF_MEDIA state that cannot be corrected causes a
//       "MediaIOExecption" for flush() and close().
//
//----------------------------------------------------------------------------
#ifndef MEDIA_H_INCLUDED
#define MEDIA_H_INCLUDED

//----------------------------------------------------------------------------
//
// Definition class-
//       MediaType
//
// Purpose-
//       Define the types used by Media objects.
//
//----------------------------------------------------------------------------
class MediaType {                   // Media types
//----------------------------------------------------------------------------
// MediaType::Attributes
//----------------------------------------------------------------------------
public:                             // Mode definitions
static const char*     MODE_READ;   // Read mode
static const char*     MODE_WRITE;  // Write (create/truncate) mode
static const char*     MODE_CREATE; // Write (create) mode
static const char*     MODE_APPEND; // Write (append) mode
static const char*     MODE_INOUT;  // Read+write mode
static const char*     MODE_OUTIN;  // Write+read mode
static const char*     MODE_CREATE_IN; // Create+read mode
static const char*     MODE_APPEND_IN; // Append+read mode

//----------------------------------------------------------------------------
// MediaTypes::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef char           Byte;        // A data byte
typedef unsigned long  Size_t;      // A data length

enum State                          // The Media State
{  STATE_RESET                      // RESET
,  STATE_INPUT                      // Open for input
,  STATE_OUTPUT                     // Open for output
,  STATE_INOUT                      // Open for input and output
,  STATE_OUTIN                      // Open for output and input
,  STATE_EOF                        // End of input
,  STATE_EOM                        // End of media
,  STATE_ERROR                      // Error state
};

enum RC                             // Special open/get/put return code/char
// General pupose return codes or characters
{  RC_NORMAL= 0                     // No error encountered
,  RC_MEDIA_FAULT= (-14)            // Media error encountered
,  RC_SYSTEM= (-15)                 // System error encountered

// Get() return characters
,  RC_EOF= (-1)                     // End of file
,  RC_EOM= RC_EOF                   // End of media
,  RC_NULL= (-2)                    // Non-blocking and no data available
,  RC_SKIP= (-3)                    // Some data skipped (and lost)
,  RC_USER= (-4)                    // User error

// Open() return codes
,  RC_CREATE= (-1)                  // open(create), but file exists
}; // enum RC
}; // class MediaType

//----------------------------------------------------------------------------
//
// Interface Class-
//       Media
//
// Purpose-
//       Media defines a set of input/output interfaces.
//
//----------------------------------------------------------------------------
class Media : public MediaType {    // Media Interface
//----------------------------------------------------------------------------
//
// Method-
//       Media::getState
//
// Purpose-
//       Get (but do not modify) the State.
//
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const = 0;      // Get current State

//----------------------------------------------------------------------------
//
// Method-
//       Media::open
//
// Purpose-
//       Start using the Media.
//
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   open(                            // Start using the Media
     const char*       name,        // The name
     const char*       mode) = 0;   // The mode

//----------------------------------------------------------------------------
//
// Method-
//       Media::close
//
// Purpose-
//       Finish using the Media.
//
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   close( void ) = 0;               // Finish using the Media

//----------------------------------------------------------------------------
//
// Method-
//       Media::flush
//
// Purpose-
//       Flush the Media.
//
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   flush( void ) = 0;               // Flush the Media

//----------------------------------------------------------------------------
//
// Method-
//       Media::read
//
// Purpose-
//       Read from the Media.
//
//----------------------------------------------------------------------------
public:
virtual Size_t                      // Number of bytes read
   read(                            // Read from the Media
     Byte*             addr,        // Data address
     Size_t            size) = 0;   // Data length

//----------------------------------------------------------------------------
//
// Method-
//       Media::write
//
// Purpose-
//       Write onto the Media
//
//----------------------------------------------------------------------------
virtual Size_t                      // Number of bytes written
   write(                           // Write onto the Media
     const Byte*       addr,        // Data address
     Size_t            size) = 0;   // Data length
}; // class Media

//----------------------------------------------------------------------------
//
// Class-
//       FileMedia
//
// Purpose-
//       Implement Media for an external storage file.
//
//----------------------------------------------------------------------------
class FileMedia : public Media {    // FileMedia object
//----------------------------------------------------------------------------
// FileMedia::Attributes
//----------------------------------------------------------------------------
protected:
State                  state;       // The current State
State                  openState;   // The State after open
void*                  handle;      // The access handle

//----------------------------------------------------------------------------
// FileMedia::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~FileMedia( void );              // Destructor
   FileMedia( void );               // Default constructor

private:                            // Bitwise copy prohibited
   FileMedia(const FileMedia&);     // Copy constructor
FileMedia&
   operator=(const FileMedia&);     // Assignment operator

//----------------------------------------------------------------------------
// FileMedia::Methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Start using the FileMedia
     const char*       name,        // The name
     const char*       mode);       // The mode

virtual int                         // Return code (0 OK)
   close( void );                   // Finish using the FileMedia

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the FileMedia

virtual Size_t                      // Number of bytes read
   read(                            // Read from the FileMedia
     Byte*             addr,        // Data address
     Size_t            size);       // Data length

virtual Size_t                      // Number of bytes written
   write(                           // Write onto the FileMedia
     const Byte*       addr,        // Data address
     Size_t            size);       // Data length
}; // class FileMedia

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Socket;

//----------------------------------------------------------------------------
//
// Class-
//       SockMedia
//
// Purpose-
//       Implement Media for Sockets.
//
// Notes-
//       Method open only performs state checking.
//       Method close closes the socket, also setting socket= NULL.
//
//----------------------------------------------------------------------------
class SockMedia : public Media {    // SockMedia object
//----------------------------------------------------------------------------
// SockMedia::Attributes
//----------------------------------------------------------------------------
protected:
State                  state;       // The open State
Socket*                socket;      // The Socket

//----------------------------------------------------------------------------
// SockMedia::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~SockMedia( void );              // Destructor
   SockMedia( void );               // Default constructor

   SockMedia(                       // Socket constructor
     Socket*           sock);       // -> Socket

private:                            // Bitwise copy prohibited
   SockMedia(const SockMedia&);     // Copy constructor
SockMedia&
   operator=(const SockMedia&);     // Assignment operator

//----------------------------------------------------------------------------
// SockMedia::Accessor methods
//----------------------------------------------------------------------------
public:
Socket*                             // The Socket
   getSocket( void ) const;         // Get Socket

void
   setSocket(                       // Set the Socket
     Socket*           socket);     // -> Socket

//----------------------------------------------------------------------------
// SockMedia::Methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Start using the SockMedia
     const char*       name,        // The name
     const char*       mode);       // The mode

virtual int                         // Return code (0 OK)
   close( void );                   // Finish using the SockMedia

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the SockMedia

virtual Size_t                      // Number of bytes read
   read(                            // Read from the SockMedia
     Byte*             addr,        // Data address
     Size_t            size);       // Data length

virtual Size_t                      // Number of bytes written
   write(                           // Write onto the SockMedia
     const Byte*       addr,        // Data address
     Size_t            size);       // Data length
}; // class SockMedia

//----------------------------------------------------------------------------
//
// Class-
//       TempMedia
//
// Purpose-
//       Implement Media for temporary storage (memory)
//
//----------------------------------------------------------------------------
class TempMedia : public Media {    // TempMedia object
//----------------------------------------------------------------------------
// TempMedia::Attributes
//----------------------------------------------------------------------------
protected:
State                  state;       // The current State
State                  openState;   // The State after open
void*                  head;        // First storage buffer
void*                  tail;        // Last storage buffer
Size_t                 size;        // Number of tail bytes used
void*                  busy;        // The current input storage buffer
Size_t                 used;        // Number of busy bytes used

//----------------------------------------------------------------------------
// TempMedia::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TempMedia( void );              // Destructor
   TempMedia( void );               // Default constructor

private:                            // Bitwise copy prohibited
   TempMedia(const TempMedia&);     // Copy constructor
TempMedia&
   operator=(const TempMedia&);     // Assignment operator

//----------------------------------------------------------------------------
// TempMedia::Methods
//----------------------------------------------------------------------------
public:
virtual State                       // The current State
   getState( void ) const;          // Get current State

virtual int                         // Return code (0 OK)
   open(                            // Start using the TempMedia
     const char*       name,        // The name
     const char*       mode);       // The mode

virtual int                         // Return code (0 OK)
   close( void );                   // Finish using the TempMedia

virtual int                         // Return code (0 OK)
   flush( void );                   // Flush the TempMedia

virtual Size_t                      // Number of bytes read
   read(                            // Read from the TempMedia
     Byte*             addr,        // Data address
     Size_t            size);       // Data length

virtual Size_t                      // Number of bytes written
   write(                           // Write onto the TempMedia
     const Byte*       addr,        // Data address
     Size_t            size);       // Data length

virtual void
   truncate( void );                // Delete all associated storage
}; // class TempMedia

#endif // MEDIA_H_INCLUDED
