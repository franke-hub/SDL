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
//       XmlNode.h
//
// Purpose-
//       Describe an XML Node.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       The XmlNode is neither XML compliant nor DOM compliant.
//         Only Ascii codespace characters are used.
//         Some XML declaratives (notably <!ENTITY>) are not supported.
//         Some DOM methods are missing, some are renamed.
//
//----------------------------------------------------------------------------
#ifndef XMLNODE_H_INCLUDED
#define XMLNODE_H_INCLUDED

#include <string>

#ifndef XMLLIST_H_INCLUDED
#include "XmlList.h"                // (Also includes List.h, Link.h)
#endif

//----------------------------------------------------------------------------
//
// Class-
//       XmlNode
//
// Purpose-
//       A generic XML Node.
//
//----------------------------------------------------------------------------
class XmlNode : public List<XmlNode>::Link { // Generic XML Node
//----------------------------------------------------------------------------
// XmlNode::Attributes
//----------------------------------------------------------------------------
protected:
   int                 type;        // XmlNode type
   XmlNode*            parent;      // Parent XmlNode

   std::string         name;        // Associated node name
   std::string         data;        // Associated node value
   XmlList             attrib;      // List of attribute XmlNode elements
   XmlList             child;       // List of child XmlNode elements

//----------------------------------------------------------------------------
// XmlNode::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum TYPE                           // Node types
{  TYPE_RESET                       // RESET, type not set
,  TYPE_ENTITY                      // Entity node
,  TYPE_ATTR                        // Attribute node
,  TYPE_ELEM                        // Element node
,  TYPE_ROOT                        // Root element node
,  TYPE_TEXT                        // Text element node
,  TYPE_COMMENT                     // Comment node     <!-- ... -->
,  TYPE_CDATA                       // CData node       <![CDATA[ ... ]]>
,  TYPE_DECL                        // Declarative node <! ... >
,  TYPE_DESC                        // Descriptive node <? ... ?>
,  TYPE_COUNT                       // Number of Node types
}; // enum TYPE

//----------------------------------------------------------------------------
// XmlNode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~XmlNode( void );                // Default destructor
   XmlNode( void );                 // Default constructor

   XmlNode(                         // Constructor
     int               type,        // Node type
     const std::string&name,        // Node name
     const std::string&data = "");  // Node value

private:                            // Bitwise copy prohibited
   XmlNode(const XmlNode&);
XmlNode&
   operator=(const XmlNode&);

//----------------------------------------------------------------------------
// XmlNode::Internal methods
//----------------------------------------------------------------------------
protected:
void
   debug(                           // Debugging node display
     int               level);      // For nesting level

//----------------------------------------------------------------------------
// XmlNode::Static methods
//----------------------------------------------------------------------------
public:
static const char*                  // The associated name
   type2name(                       // Get associated name
     int               type);       // For this type

//----------------------------------------------------------------------------
// XmlNode::Accessors
//----------------------------------------------------------------------------
public:
int                                 // The number of attribute nodes
   getAttribCount( void ) const;    // Get number of attribute nodes

XmlNode*                            // The associated attribute
   getAttrib(                       // Get associated attribute
     int               index= 0) const;// For this attribute index

XmlNode*                            // The associated attribute node
   getAttrib(                       // Get associated attribute node
     const std::string&name) const; // With this name

int                                 // The number of child nodes
   getChildCount( void ) const;     // Get number of child nodes

XmlNode*                            // The associated child node
   getChild(                        // Get associated child node
     int               index= 0) const;// For this child index

XmlNode*                            // The first associated child
   getChild(                        // Get first associated child
     const std::string&name) const; // With this name

std::string                         // The associated name
   getName( void ) const;           // Get associated name

XmlNode*                            // The associated parent node
   getParent( void ) const;         // Get associated parent node

int                                 // The node type
   getType( void ) const;           // Get node type

std::string                         // The associated value
   getValue( void ) const;          // Get associated value

void
   setValue(                        // Set associated value
     std::string       value);      // The replacement value

//----------------------------------------------------------------------------
// XmlNode::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void );                   // Debugging node display

void
   deleteAttrib( void );            // Delete ALL attribute nodes

void
   deleteChild( void );             // Delete ALL child nodes

void
   detach( void );                  // Detach this node from parent node

void
   insert(                          // Insert (at end of list)
     XmlNode*          node);       // This (child or attribute) node

void
   insertBefore(                    // Insert
     XmlNode*          node,        // This (child or attribute) node
     int               index);      // BEFORE this node index
}; // class XmlNode

#endif // XMLNODE_H_INCLUDED
