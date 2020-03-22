//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       XmlParser.h
//
// Purpose-
//       Describe the XML Parser.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       The XmlParser is neither XML compliant nor DOM compliant.
//         Only Ascii codespace characters are used.
//         Some XML declaratives (notably <!ENTITY>) are not supported.
//         Some DOM methods are missing, some are renamed.
//
//       The getText() method returns normalized text. Leading and trailing
//       whitespace is removed and inner whitespace results in a single
//       blank character. Entity values replace entity names.
//
//       The getValue() method returns normalized data. Leading and trailing
//       quotes are removed. Inner whitespace is preserved. Entity values
//       replace entity names.
//
//----------------------------------------------------------------------------
#ifndef XMLPARSER_H_INCLUDED
#define XMLPARSER_H_INCLUDED

#ifndef XMLNODE_H_INCLUDED
#include "XmlNode.h"                // (Includes other prerequisites)
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Reader;
class Writer;

//----------------------------------------------------------------------------
//
// Class-
//       XmlParser
//
// Purpose-
//       A generic XML Parser.
//
//----------------------------------------------------------------------------
class XmlParser {                   // The XML Parser
friend class XmlNode;               // (Uses debug(int,const XmlNode*))

//----------------------------------------------------------------------------
// XmlParser::Attributes
//----------------------------------------------------------------------------
protected:
XmlNode*               desc;        // The root descriptor XmlNode
XmlNode*               root;        // The root element XmlNode
XmlList                entity;      // The Entity list
char                   pmet[4];     // (For getEntity temporary)
char*                  temp;        // (for getEntity temporary)

//----------------------------------------------------------------------------
// XmlParser::Constructors
//----------------------------------------------------------------------------
public:
   ~XmlParser( void );              // Default destructor
   XmlParser( void );               // Default constructor

private:                            // Bitwise copy prohibited
   XmlParser(const XmlParser&);
XmlParser&
   operator=(const XmlParser&);

//----------------------------------------------------------------------------
// XmlParser::Internal methods
//----------------------------------------------------------------------------
protected:
void
   debug(                           // Debugging display
     int               level,       // For this nesting level
     const XmlNode*    node) const; // And this node

static XmlNode*                     // Resultant XmlNode
   genAttr(                         // Generate attribute node
     XmlNode*          parent,      // With this parent
     Reader&           reader);     // Using this Reader

static XmlNode*                     // Resultant XmlNode
   genData(                         // Generate CDATA/comment node
     XmlNode*          parent,      // With this parent
     Reader&           reader);     // Using this Reader

static XmlNode*                     // Resultant XmlNode
   genDesc(                         // Generate descriptive node
     XmlNode*          parent,      // With this parent
     Reader&           reader);     // Using this Reader

static XmlNode*                     // Resultant XmlNode
   genNode(                         // Generate element node
     XmlNode*          parent,      // With this parent
     Reader&           reader);     // Using this Reader

static void
   output(                          // Output the node tree
     XmlNode*          here,        // Starting from here
     Writer&           writer);     // Onto this Writer

static std::string                  // Resultant node tree
   toString(                        // Output the node tree
     XmlNode*          here);       // Starting from here

//----------------------------------------------------------------------------
// XmlParser::Accessors
//----------------------------------------------------------------------------
public:
XmlNode*                            // The associated descriptor node
   getDesc( void ) const;           // Get associated descriptor node

// getEntity resultant only valid until next XmlParser method call
const char*                         // The associated entity value
   getEntity(                       // Get associated entity value
     const std::string&entity) const; // For this entity name

void
   setEntity(                       // Set associated entity value
     const std::string&name,        // For this entity name
     const char*       value);      // To this value (NULL to remove)

XmlNode*                            // The associated root node
   getRoot( void ) const;           // Get associated root node

//----------------------------------------------------------------------------
// XmlParser::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

std::string                         // Evaluated string
   evaluate(                        // Replace entities
     const std::string&source) const; // Within this string

std::string                         // Associated text
   getText(                         // Get normalized text
     const XmlNode*    node) const; // For this XmlNode

std::string                         // Associated value
   getValue(                        // Get normalized value
     const XmlNode*    node) const; // For this XmlNode

void
   output(                          // Output the node tree
     Writer&           writer) const; // Onto this Writer

XmlNode*                            // Resultant (allocated) root node
   parse(                           // Extract next complete node tree
     Reader&           reader);     // From this Reader

XmlNode*                            // Resultant (allocated) root node
   parse(                           // Extract next complete node tree
     const std::string&input);      // From this string

void
   reset( void );                   // Reset the XmlParser

std::string                         // Resultant string
   toString( void ) const;          // Output the node tree
}; // class XmlParser

#endif // XMLPARSER_H_INCLUDED
