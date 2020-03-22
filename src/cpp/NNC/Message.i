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
//       Message.i
//
// Purpose-
//       Message inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MESSAGE_I_INCLUDED
#define MESSAGE_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Message::MessageLink::getIdent
//
// Purpose-
//       Extract the message identifier.
//
//----------------------------------------------------------------------------
unsigned int                        // The messge identifier
   Message::MessageLink::getIdent( void ) const // Extract the identifier
{
   return ident;
}

//----------------------------------------------------------------------------
//
// Method-
//       Message::MessageLink::getText
//
// Purpose-
//       Extract the (unformatted) message text.
//
//----------------------------------------------------------------------------
const char*                         // The (unformatted) messge text
   Message::MessageLink::getText( void ) const // Extract the text
{
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       MessageCallback::getName
//
// Purpose-
//       Extract the file name.
//
//----------------------------------------------------------------------------
const char*                        // The file name
   MessageCallback::getName( void ) const // Extract the file name
{
   return fileName;
}

//----------------------------------------------------------------------------
//
// Method-
//       MessageCallback::getLine
//
// Purpose-
//       Extract the file line number.
//
//----------------------------------------------------------------------------
unsigned                            // The file line number
   MessageCallback::getLine( void ) const // Extract the file line number
{
   return lineNumber;
}

//----------------------------------------------------------------------------
//
// Method-
//       MessageCallback::getColumn
//
// Purpose-
//       Extract the file column.
//
//----------------------------------------------------------------------------
unsigned                            // The file column
   MessageCallback::getColumn( void ) const // Extract the file column
{
   return column;
}

#endif // MESSAGE_I_INCLUDED
