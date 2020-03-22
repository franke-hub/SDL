//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Node.h
//
// Purpose-
//       Describe a generic Node.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <string>
#include <com/List.h>

//----------------------------------------------------------------------------
//
// Class-
//       Node
//
// Purpose-
//       A generic Node.
//
//----------------------------------------------------------------------------
class Node : public List<Node>::Link { // Generic Node
//----------------------------------------------------------------------------
// Node::Attributes
//----------------------------------------------------------------------------
protected:
   int                 type;        // Node type
   Node*               parent;      // Parent Node
   List<Node>          child;       // List of child Node elements

//----------------------------------------------------------------------------
// Node::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Node( void );                   // Default destructor
   Node( void );                    // Default constructor

private:                            // Bitwise copy prohibited
   Node(const Node&);
Node&
   operator=(const Node&);

//----------------------------------------------------------------------------
// Node::Internal methods
//----------------------------------------------------------------------------
protected:
void
   debug(                           // Debugging node display
     int               level);      // For nesting level

void
   deleteChild( void );             // Delete ALL child nodes

//----------------------------------------------------------------------------
// Node::Accessors
//----------------------------------------------------------------------------
public:
int                                 // The number of attribute nodes
   getAttribCount( void ) const;    // Get number of attribute nodes

Node*                               // The associated attribute
   getAttrib(                       // Get associated attribute
     int               index= 0) const; // For this attribute index

Node*                               // The associated attribute node
   getAttrib(                       // Get associated attribute node
     const std::string&name) const; // With this name

int                                 // The number of child nodes
   getChildCount( void ) const;     // Get number of child nodes

Node*                               // The associated child node
   getChild(                        // Get associated child node
     int               index= 0) const; // For this child index

Node*                               // The first associated child
   getChild(                        // Get first associated child
     const std::string&name) const; // With this name

std::string                         // The associated name
   getName( void ) const;           // Get associated name

Node*                               // The associated parent node
   getParent( void ) const;         // Get associated parent node

int                                 // The node type
   getType( void ) const;           // Get node type

//----------------------------------------------------------------------------
// Node::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void );                   // Debugging node display

void
   insert(                          // Insert (at end of list)
     Node*             node);       // This (child or attribute) node

void
   insertBefore(                    // Insert
     Node*             node,        // This child node
     int               index);      // BEFORE this node index

void
   remove( void );                  // Remove this node from parent node
}; // class Node

#endif // NODE_H_INCLUDED
