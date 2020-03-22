//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Message.h
//
// Purpose-
//       Define the Message(table) object.
//
// Last change date-
//       2007/01/01
//
// Format-
//       Blank lines and lines beginning with '//' are ignored.
//       Format: Number, "String" {...}, ...
//         Number: The message number
//         String: The message string, concatenated if multiple.
//                 '\n' replaced with newline
//                 '\\' replaced with single '\'
//
//       $00 : The original message identifier.
//       $01..$99 : positional parameters (1..99)
//       $FN : File Name
//       $FL : File Name Line
//       $FC : File Name Line Column
//
//----------------------------------------------------------------------------
#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <stdarg.h>

#ifndef LIST_INCLUDED
#include "com/List.h"
#endif

#ifndef SUBPOOL_H_INCLUDED
#include "com/Subpool.h"
#endif

#ifndef CALLBACK_H_INCLUDED
#include "Callback.h"
#endif

//----------------------------------------------------------------------------
// Forward References
//----------------------------------------------------------------------------
class MessageCallback;

//----------------------------------------------------------------------------
//
// Class-
//       Message
//
// Purpose-
//       The Message object contains messages.
//
//----------------------------------------------------------------------------
class Message {                     // Message table
//----------------------------------------------------------------------------
// class Message::MessageLink
//----------------------------------------------------------------------------
public:
struct MessageLink : public List<MessageLink>::Link { // Message Link
//----------------------------------------------------------------------------
// Message::MessageLink::Attributes
//----------------------------------------------------------------------------
private:
unsigned int           ident;       // Message identifier
const char*            text;        // Message text

//----------------------------------------------------------------------------
// Message::MessageLink::Constructors
//----------------------------------------------------------------------------
public:
  ~MessageLink( void );             // Destructor

  MessageLink( void );              // Default constructor
  MessageLink(                      // Initializing constructor
    unsigned int       ident,       // Message identifer
    const char*        text);       // Message text

//----------------------------------------------------------------------------
// Message::MessageLink::Accessor methods
//----------------------------------------------------------------------------
public:
inline unsigned int                 // The numeric message identifier
   getIdent( void ) const;          // Get the message identifier

inline const char*                  // The (unformatted) message text
   getText( void ) const;           // Get the message text
}; // struct Message::MessageLink

//----------------------------------------------------------------------------
// Message::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Ident                          // Message identifier (alias)
{
   ID_UndefinedIndex=          9998,// Undefined message index
   ID_UndefinedMessage=        9999,// Undefined message message
}; // enum Ident

enum
{
   HASHSIZE=                     32 // Number of hash classes
}; // enum (generic)

//----------------------------------------------------------------------------
// Message::Attributes
//----------------------------------------------------------------------------
private:
Subpool                subpool;     // Storage subpool
MessageCallback*       callback;    // Callback routine
List<MessageLink>      messageList[HASHSIZE]; // Message list

static MessageLink     undefinedIndex;   // Undefined index built-in
static MessageLink     undefinedMessage; // Undefined message built-in

//----------------------------------------------------------------------------
// Message::Constructors
//----------------------------------------------------------------------------
public:
   ~Message( void );                // Destructor

   Message( void );                 // Constructor

private:                            // Bitwise copy is prohibited
   Message(const Message&);         // Disallowed copy constructor
   Message& operator=(const Message&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Message::Private methods
//----------------------------------------------------------------------------
private:
static unsigned int                 // Hash(number)
   hash(                            // Get hash index
     unsigned int      ident);      // Message identifier

//----------------------------------------------------------------------------
// Message::Accessor methods
//----------------------------------------------------------------------------
public:
void
   setCallback(                     // Set the associated Callback
     MessageCallback*  callback);   // The associated Callback

//----------------------------------------------------------------------------
// Message::Public methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   load(                            // Load message table
     const char*       fileName);   // Associated file name

const MessageLink*                  // -> MessageLink
   locate(                          // Locate a message
     unsigned int      ident) const;// Message identifier

void
   message(                         // Write a message
     unsigned int      ident,       // Message identifier
     unsigned int      argc,        // Argument count
     va_list           argv);       // Argument array

void
   message(                         // Write a message
     unsigned int      ident,       // Message identifier
     unsigned int      argc,        // Argument count
                       ...);        // Argument array
}; // class Message

//----------------------------------------------------------------------------
//
// Class-
//       MessageCallback
//
// Purpose-
//       The MessageCallback object returns a fileName, lineNumber and column.
//
//----------------------------------------------------------------------------
class MessageCallback : public Callback { // Message callback
//----------------------------------------------------------------------------
// MessageCallback::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~MessageCallback( void );        // Destructor
   MessageCallback( void );         // Constructor

//----------------------------------------------------------------------------
// MessageCallback::Methods
//----------------------------------------------------------------------------
public:
inline const char*                  // The file name
   getName( void ) const;           // Extract the file name

inline unsigned                     // The line number
   getLine( void ) const;           // Extract the line number

inline unsigned                     // The column
   getColumn( void ) const;         // Extract the column

virtual void
   set( void );                     // Set name components

//----------------------------------------------------------------------------
// MessageCallback::Attributes
//----------------------------------------------------------------------------
protected:
   char                fileName[512]; // File name
   unsigned            lineNumber;  // Current line number
   unsigned            column;      // Current column
}; // class MessageCallback

#include "Message.i"

#endif // MESSAGE_H_INCLUDED
