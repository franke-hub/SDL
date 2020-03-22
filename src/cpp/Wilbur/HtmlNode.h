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
//       HtmlNode.h
//
// Purpose-
//       HtmlNode base class.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef HTMLNODE_H_INCLUDED
#define HTMLNODE_H_INCLUDED

#include <string>

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HtmlNodeVisitor;
class ElemHtmlNode;

//----------------------------------------------------------------------------
//
// Class-
//       HtmlNode
//
// Purpose-
//       HtmlNode base class.
//
// Usage notes-
//       ~HtmlNode() deletes all child HtmlNodes.
//
//----------------------------------------------------------------------------
class HtmlNode {                    // HtmlNode base class
//----------------------------------------------------------------------------
// HtmlNode::Attributes
//----------------------------------------------------------------------------
friend class ElemHtmlNode;
public:
enum Type                           // The HtmlNode Type
{  TYPE_NULL                        // Undefined Type
,  TYPE_ELEM                        // ELEMENT HtmlNode
,  TYPE_ATTR                        // ATTRIBUTE HtmlNode
,  TYPE_TEXT                        // TEXT HtmlNode
}; // enum Type

protected:
ElemHtmlNode*          parent;      // -> Parent HtmlNode
HtmlNode*              peer;        // -> Peer HtmlNode

//----------------------------------------------------------------------------
// HtmlNode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HtmlNode( void );               // Destructor
   HtmlNode( void );                // Default constructor

private:                            // Disallowed
   HtmlNode(const HtmlNode&);       // Copy constructor
HtmlNode& operator=(const HtmlNode&); // Assignment operator

//----------------------------------------------------------------------------
// HtmlNode::Accessors
//----------------------------------------------------------------------------
public:
virtual HtmlNode*                   // The associated attribute HtmlNode
   getAttr(                         // Get attribute HtmlNode
     std::string       attr) const; // For this attribute name

virtual std::string                 // The data string
   getData( void ) const;           // Get data string

virtual std::string                 // The name string
   getName( void ) const;           // Get name string

virtual HtmlNode*                   // The first child HtmlNode
   getChild( void ) const;          // Get first child HtmlNode

ElemHtmlNode*                       // The parent HtmlNode
   getParent( void ) const          // Get parent HtmlNode
{
   return parent;
}

HtmlNode*                           // The next peer HtmlNode
   getPeer( void ) const            // Get next peer HtmlNode
{
   return peer;
}

virtual Type                        // The HtmlNode Type
   getType( void ) const;           // Get HtmlNode Type

//----------------------------------------------------------------------------
// HtmlNode::Methods
//----------------------------------------------------------------------------
public:
/**
* Visit the HtmlNode subtree.
*
* For each node, the node is visited first. The return code from that visit
* determines whether child nodes are visited. If so, each child node begins
* a new visit subtree.
*
**/
void
   visit(                           // Visit the HtmlNode tree
     HtmlNodeVisitor&  visitor);    // Using this Visitor
}; // class HtmlNode

//----------------------------------------------------------------------------
//
// Class-
//       ElemHtmlNode
//
// Purpose-
//       ELEMENT HtmlNode.
//
//----------------------------------------------------------------------------
class ElemHtmlNode : public HtmlNode { // ELEMENT HtmlNode
//----------------------------------------------------------------------------
// ElemHtmlNode::Attributes
//----------------------------------------------------------------------------
public:
std::string            name;        // The ELEMENT name
HtmlNode*              head;        // -> Head child HtmlNode
HtmlNode*              tail;        // -> Tail child HtmlNode

//----------------------------------------------------------------------------
// ElemHtmlNode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ElemHtmlNode( void );           // Destructor
   ElemHtmlNode(                    // Constructor
     std::string       name);       // Element name

private:                            // Disallowed
   ElemHtmlNode(const ElemHtmlNode&); // Copy constructor
ElemHtmlNode& operator=(const ElemHtmlNode&); // Assignment operator

//----------------------------------------------------------------------------
// ElemHtmlNode::Accessors
//----------------------------------------------------------------------------
public:
virtual HtmlNode*                   // The associated attribute HtmlNode
   getAttr(                         // Get attribute HtmlNode
     std::string       attr) const; // For this attribute name

virtual std::string                 // The name string
   getName( void ) const;           // Get name string

virtual HtmlNode*                   // The first child HtmlNode
   getChild( void ) const;          // Get first child HtmlNode

virtual Type                        // The HtmlNode Type
   getType( void ) const;           // Get HtmlNode Type

//----------------------------------------------------------------------------
// ElemHtmlNode::Methods
//----------------------------------------------------------------------------
public:
/**
* Insert a child element.
*
* The child element is added at the end of the child node list.
*
**/
void
   insertChild(                     // Insert child ElementHtmlNode
     HtmlNode*         child);      // The HtmlNode to insert
}; // class ElemHtmlNode

//----------------------------------------------------------------------------
//
// Class-
//       AttrHtmlNode
//
// Purpose-
//       ATTRIBUTE HtmlNode.
//
//----------------------------------------------------------------------------
class AttrHtmlNode : public HtmlNode { // ATTRIBUTE HtmlNode
//----------------------------------------------------------------------------
// AttrHtmlNode::Attributes
//----------------------------------------------------------------------------
public:
std::string            name;        // The ATTRIBUTE name
std::string            data;        // The ATTRIBUTE value

//----------------------------------------------------------------------------
// AttrHtmlNode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~AttrHtmlNode( void );           // Destructor
   AttrHtmlNode(                    // Constructor
     std::string       name,        // Attribute name
     std::string       data);       // Attribute value

private:                            // Disallowed
   AttrHtmlNode(const AttrHtmlNode&); // Copy constructor
AttrHtmlNode& operator=(const AttrHtmlNode&); // Assignment operator

//----------------------------------------------------------------------------
// AttrHtmlNode::Accessors
//----------------------------------------------------------------------------
public:
virtual std::string                 // The name string
   getName( void ) const;           // Get name string

virtual std::string                 // The data string
   getData( void ) const;           // Get data string

virtual Type                        // The HtmlNode Type
   getType( void ) const;           // Get HtmlNode Type
}; // class AttrHtmlNode

//----------------------------------------------------------------------------
//
// Class-
//       TextHtmlNode
//
// Purpose-
//       TEXT HtmlNode.
//
//----------------------------------------------------------------------------
class TextHtmlNode : public HtmlNode { // TEXT HtmlNode
//----------------------------------------------------------------------------
// TextHtmlNode::Attributes
//----------------------------------------------------------------------------
public:
std::string            data;        // The TEXT

//----------------------------------------------------------------------------
// TextHtmlNode::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TextHtmlNode( void );           // Destructor
   TextHtmlNode(                    // Constructor
     std::string       text);       // The text

private:                            // Disallowed
   TextHtmlNode(const TextHtmlNode&); // Copy constructor
TextHtmlNode& operator=(const TextHtmlNode&); // Assignment operator

//----------------------------------------------------------------------------
// TextHtmlNode::Accessors
//----------------------------------------------------------------------------
public:
virtual std::string                 // The data string
   getData( void ) const;           // Get data string

virtual Type                        // The HtmlNode Type
   getType( void ) const;           // Get HtmlNode Type
}; // class TextHtmlNode

#endif // HTMLNODE_H_INCLUDED
