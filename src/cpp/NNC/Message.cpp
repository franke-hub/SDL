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
//       Message.cpp
//
// Purpose-
//       Message object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include <com/Reader.h>
#include "Message.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "MESSAGE " // Source file name

//----------------------------------------------------------------------------
// Static data areas
//----------------------------------------------------------------------------
Message::MessageLink Message::undefinedIndex(
       Message::ID_UndefinedIndex,
       "$FL Compiler error, message'$01' index'$02' undefined");

Message::MessageLink Message::undefinedMessage(
       Message::ID_UndefinedMessage,
       "$FL Compiler error, message'$00' undefined");

static MessageCallback
                     defaultCallback; // Default callback handler

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::invalidMessageFile
//
// Purpose-
//       Write error message and exit.
//
//----------------------------------------------------------------------------
static void
   invalidMessageFile(              // Error abort
     const char*       name,        // The message file reader name
     LineReader&       reader,      // The message file reader
     const char*       message)     // Error message
{
   fprintf(stderr, "Message file(%s) line(%ld) %s\n",
                   name, reader.getLine(), message);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::isCommentStart
//
// Purpose-
//       See whether a character begins a comment.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE if comment start
   isCommentStart(                  // Is character a comment start?
     int               C)           // The test character
{
   return( C == '/' || C == '#' );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::isWhiteSpace
//
// Purpose-
//       See whether a character is a white space character.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE if white space
   isWhiteSpace(                    // Is character white space?
     int               C)           // The test character
{
   return( C == ' ' || C == '\n' || C == '\t' );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::skipWhiteSpace
//
// Purpose-
//       Skip white space in LineReader.
//
//----------------------------------------------------------------------------
static int                          // The next non white space character
   skipWhiteSpace(                  // Skip white space
     LineReader&       reader)      // The message file LineReader
{
   return reader.skipBlank();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::skipToNewLine
//
// Purpose-
//       Skip the remainder of the line.
//
//----------------------------------------------------------------------------
static void
   skipToNewLine(                   // Skip the remainder of the line
     LineReader&       reader)      // The message file LineReader
{
   reader.skipLine();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::MessageLink::~MessageLink
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Message::MessageLink::~MessageLink( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::MessageLink::MessageLink
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Message::MessageLink::MessageLink( // Initializing constructor
     unsigned int      ident,       // Message identifer
     const char*       text)        // Message text
:  ident(ident)
,  text(text)
{
}

   Message::MessageLink::MessageLink( void ) // Default constructor
:  ident(0)
,  text(NULL)
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::~Message
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Message::~Message( void )        // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::Message
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Message::Message( void )         // Constructor
:  subpool()
,  callback(&defaultCallback)
{
   int                 i;

   // Initialize the hash table
   for(i=0; i<HASHSIZE; i++)
     new (&messageList[i]) List<MessageLink>();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::hash
//
// Purpose-
//       Hash function.
//
//----------------------------------------------------------------------------
unsigned int                        // Hash(number)
   Message::hash(                   // Get hash index
     unsigned int      ident)       // Message identifier
{
   return ident & (HASHSIZE-1);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::locate
//
// Purpose-
//       Locate a message in the message table.
//
//----------------------------------------------------------------------------
const Message::MessageLink*         // -> MessageLink
   Message::locate(                 // Locate a message
     unsigned int      ident) const // Message identifier
{
   const MessageLink*  ptrLink;     // -> MessageLink

   for(ptrLink= (MessageLink*)messageList[hash(ident)].getHead();
       ptrLink != NULL;
       ptrLink= (MessageLink*)ptrLink->getNext() )
   {
     if( ptrLink->getIdent() == ident )
       break;
   }

   return ptrLink;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::load
//
// Purpose-
//       Load a message file
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Message::load(                   // Constructor
     const char*       fileName)    // Filename of message table
{
   LineReader          reader;      // Message file LineReader
   const MessageLink*  oldLink;     // -> MessageLink
   MessageLink*        ptrLink;     // -> MessageLink
   char*               ptrText;     // -> Message text

   unsigned int        ident;       // Message identifier
   char                text[1024];  // Message text

   int                 C;           // Current character
   unsigned            i;

   if( reader.open(fileName) != 0 )
   {
     fprintf(stderr, "Could not open message file(%s)\n", fileName);
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Load the message table
   for(;;)
   {
     C= skipWhiteSpace(reader);
     if( isCommentStart(C) )
     {
       skipToNewLine(reader);
       continue;
     }

     if( C == EOF )
       break;

     if( C < '0' || C > '9' )
       invalidMessageFile(fileName, reader, "Invalid message number");

     ident= 0;
     for(;;)
     {
       if( C < '0' || C > '9' )
         break;

       i= C - '0';
       ident *= 10;
       ident += i;

       C= reader.get();
     }

     for(;;)
     {
       if( isWhiteSpace(C) )
         C= skipWhiteSpace(reader);
       if( isCommentStart(C) )
       {
         skipToNewLine(reader);
         C= reader.get();
         continue;
       }

       if( C == ',' )
         break;

       invalidMessageFile(fileName, reader, "Missing comma after number");
     }

     C= skipWhiteSpace(reader);
     for(;;)
     {
       if( isWhiteSpace(C) )
         C= skipWhiteSpace(reader);
       if( isCommentStart(C) )
       {
         skipToNewLine(reader);
         C= skipWhiteSpace(reader);
         continue;
       }

       if( C == '"' )
         break;

       invalidMessageFile(fileName, reader, "Missing quote");
     }

     i= 0;
     for(;;)
     {
       C= reader.get();

       if( C == '"' )
       {
         for(;;)
         {
           C= skipWhiteSpace(reader);
           if( isCommentStart(C) )
           {
             skipToNewLine(reader);
             continue;
           }
           break;
         }

         if( C == '"' )
           continue;

         if( C == ',' || C == EOF )
           break;

         invalidMessageFile(fileName, reader, "Missing comma after string");
       }

       if( C == '\n' )
         invalidMessageFile(fileName, reader, "Incomplete text");

       if( C == '\0' )
         invalidMessageFile(fileName, reader, "NULL in text");

       if( C == '\\' )
       {
         C= reader.get();
         if( C == 'n' )
           C= '\n';
         else if( C == '"' )
           C= '\"';
         else if( C == 't' )
           C= '\t';
         else if( C == '\\' )
           C= '\\';
         else
           invalidMessageFile(fileName, reader, "Invalid \\sequence");
       }

       if( i >= sizeof(text)-2 )
         invalidMessageFile(fileName, reader, "Text too long");

       text[i++]= C;
     }
     text[i++]= '\0';

     oldLink= locate(ident);
     if( oldLink != NULL )
     {
       fprintf(stderr, "Message file(%s) line(%ld) Duplicate identifier\n",
                       fileName, (long)reader.getLine());
       fprintf(stderr, "Ident(%d) Prior(%s) New(%s)\n",
                       ident, oldLink->getText(), text);
       exit(EXIT_FAILURE);
     }

     ptrText= (char*)subpool.allocate(i);
     if( ptrText == NULL )
       invalidMessageFile(fileName, reader, "Storage shortage");
     strcpy(ptrText, text);

     ptrLink= (MessageLink*)subpool.allocate(sizeof(MessageLink));
     if( ptrLink == NULL )
       invalidMessageFile(fileName, reader, "Storage shortage");
     new(ptrLink) MessageLink(ident, ptrText);

     messageList[hash(ident)].lifo(ptrLink);

//// debugf("%s %4d: N(%d) T(%s)\n", __SOURCE__, __LINE__, ident, text);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::message
//
// Purpose-
//       Write a message.
//
//----------------------------------------------------------------------------
void
   Message::message(                // Write a message
     unsigned int      ident,       // Message identifier
     unsigned int      argc,        // Argument count
     va_list           argv)        // Argument array
{
   const MessageLink*  ptrLink;     // -> MessageLink
   const char*         ptrText;     // -> Message text
   char                msgid[8];    // (Broken) Message identifier
   char                msgix[8];    // (Broken) Message index

   va_list             varg;        // Argument element

   char*               ptrC;        // -> String
   int                 C;           // Current character
   unsigned            X;           // Current index
   int                 i;
   unsigned            j;

   ptrLink= locate(ident);          // Locate the message
   if( ptrLink == NULL )
   {
     ptrLink= locate(ID_UndefinedMessage);
     if( ptrLink == NULL )
       ptrLink= &undefinedMessage;
   }

   //-------------------------------------------------------------------------
   // Print message text
   //-------------------------------------------------------------------------
   ptrText= ptrLink->getText();
   for(i=0; ptrText[i] != '\0'; i++)
   {
     C= ptrText[i];                 // Get next character
     if( C != '$' )                 // If standard character
       fputc(C, stdout);            // Write it out
     else                           // If special character
     {
       i++;                         // Skip it
       C= ptrText[i];
       if( C == '$' )               // If the '$' is what's wanted
         fputc(C, stdout);          // Write it out
       else if( C >= '0' && C <= '9' ) // If positional parameter
       {
         X= C-'0';                  // Get numeric value
         i++;                       // Get next character
         C= ptrText[i];
         if( C < '0' || C > '9' )   // If not numeric
           break;                   // Terminate message

         X= (X * 10) + (C-'0');
         if( X == 0 )               // If message-id required
           fprintf(stdout, "%.4d", ident);

         else if( X <= argc )       // If message index is valid
         {
           va_copy(varg, argv);
           ptrC= NULL;
           for(j=0; j<X; j++)
             ptrC= va_arg(varg, char*);
           fprintf(stdout, "%s", ptrC);
           va_end(varg);
         }

         else                       // Invalid message index
         {
           fputc('\n', stdout);
           sprintf(msgid, "%.4d", ident);
           sprintf(msgix, "%.4d", X);
           if( ident != ID_UndefinedIndex && ident != ID_UndefinedMessage )
             message(ID_UndefinedIndex, 2, msgid, msgix);
           return;
         }
       }
       else if( C == 'F' )          // If file control parameter
       {
         i++;                       // Get next character
         C= ptrText[i];

         callback->set();
         fprintf(stdout, "%s:", callback->getName() );
         if( C == 'L' || C == 'C' )
         {
           fprintf(stdout, "%ld:", (long)callback->getLine() );
           if( C != 'L' )
             fprintf(stdout, "%ld:", (long)callback->getColumn() );
         }
       }
     }
   }

   fputc('\n', stdout);             // End of message
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::message
//
// Purpose-
//       Write a message
//
//----------------------------------------------------------------------------
void
   Message::message(                // Write a message
     unsigned int      ident,       // Message identifier
     unsigned int      argc,        // Argument count
                       ...)         // Argument array
{
   va_list             argv;        // Argument element

   va_start(argv, argc);
   message(ident, argc, argv);
   va_end(argv);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Message::setCallback
//
// Purpose-
//       Set the callback handler
//
//----------------------------------------------------------------------------
void
   Message::setCallback(            // Set the callback handler
     MessageCallback*  callback)    // The handler to set
{
   if( callback == NULL )           // If not specified
     callback= &defaultCallback;    // Use the default callback

   this->callback= callback;        // Set the handler
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       MessageCallback::~MessageCallback
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   MessageCallback::~MessageCallback( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       MessageCallback::MessageCallback
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   MessageCallback::MessageCallback( void ) // Constructor
:  lineNumber((unsigned)(-1))
,  column(unsigned(-1))
{
   strcpy(fileName, "*Filename not defined*");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       MessageCallback::set
//
// Purpose-
//       Set name components.
//
//----------------------------------------------------------------------------
void
   MessageCallback::set( void )     // Set name components
{
}

