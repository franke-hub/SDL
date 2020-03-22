//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HtmlParser.h
//
// Purpose-
//       HTML Parser.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef HTMLPARSER_H_INCLUDED
#define HTMLPARSER_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DataSource;
class HtmlNode;

//----------------------------------------------------------------------------
//
// Class-
//       HtmlParser
//
// Purpose-
//       HTML Parser.
//
//----------------------------------------------------------------------------
class HtmlParser {                  // HTML Parser
//----------------------------------------------------------------------------
// HtmlParser::Attributes
//----------------------------------------------------------------------------
protected:
HtmlNode*              root;        // The root HtmlNode

//----------------------------------------------------------------------------
// HtmlParser::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HtmlParser( void );             // Destructor
   HtmlParser( void );              // Default constructor

//----------------------------------------------------------------------------
// HtmlParser::Accessors
//----------------------------------------------------------------------------
public:
HtmlNode*                           // The root HtmlNode
   getRoot( void )                  // Get root HtmlNode
{
   return root;
}

//----------------------------------------------------------------------------
// HtmlParser::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

int                                 // Return code (0 OK)
   parse(                           // Parse
     DataSource&       data);       // Using this DataSource

void
   reset( void );                   // Reset the HtmlParser
}; // class HtmlParser

#endif // HTMLPARSER_H_INCLUDED
