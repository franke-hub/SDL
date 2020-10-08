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
//       HtmlNode.cpp
//
// Purpose-
//       HtmlNode implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include "Common.h"
#include "HtmlNode.h"
#include "HtmlNodeVisitor.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNode::~HtmlNode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HtmlNode::~HtmlNode( void )      // Destructor
{
   HtmlNode*           head;        // The first child HtmlNode
   HtmlNode*           next;        // The current child HtmlNode

   head= getChild();
   while( head != NULL )            // Delete child HtmlNodes
   {
     next= head;
     head= head->getPeer();
     delete next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNode::HtmlNode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HtmlNode::HtmlNode( void )       // Default constructor
:  parent(NULL)
,  peer(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNode::Accessors
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
HtmlNode*                           // The associated attribute HtmlNode
   HtmlNode::getAttr(               // Get attribute
     std::string       attr) const  // For this attribute name
{
   (void)attr; return NULL;         // (Unused parameter)
}

std::string                         // Resultant
   HtmlNode::getData( void ) const  // Get data
{
   return "";
}

std::string                         // Resultant
   HtmlNode::getName( void ) const  // Get name
{
   return "";
}

HtmlNode*                           // The first child HtmlNode
   HtmlNode::getChild( void ) const // Get first child HtmlNode
{
   return NULL;
}

HtmlNode::Type                      // The HtmlNode Type
   HtmlNode::getType( void ) const  // Get HtmlNode Type
{
   return TYPE_NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       HtmlNode::visit
//
// Purpose-
//       Visit this HtmlNode, then conditionally the child HtmlNodes.
//
//----------------------------------------------------------------------------
void
   HtmlNode::visit(                 // Visit this HtmlNode
     HtmlNodeVisitor&  visitor)     // With this HtmlNodeVisitor
{
   HtmlNode*           node;        // Working -> HtmlNode
   int                 rc;          // Return code

   rc= visitor.visit(this);         // Visit this HtmlNode
   node= getChild();                // Visit all child HtmlNode
   if( rc == 0 && node != NULL )
   {
     node->visit(visitor);          // Visit the child, pushing the stack
     node= node->getPeer();         // Visit all the child's peer HtmlNodes
     while( node != NULL )
     {
       node->visit(visitor);
       node= node->getPeer();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ElemHtmlNode::~ElemHtmlNode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ElemHtmlNode::~ElemHtmlNode( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ElemHtmlNode::ElemHtmlNode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ElemHtmlNode::ElemHtmlNode(      // Constructor
     std::string       name)        // Element name
:  HtmlNode()
,  name(name)
,  head(NULL)
,  tail(NULL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       ElemHtmlNode::Accessors
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
HtmlNode*                           // The associated AttrHtmlNode
   ElemHtmlNode::getAttr(           // Get attribute HtmlNode
     std::string       name) const  // With this name
{
   HtmlNode*           node= head;

   while( node != NULL )
   {
     if( node->getType() != TYPE_ATTR )
     {
       node= NULL;
       break;
     }

     if( node->getName() == name )
       break;

     node= node->getPeer();
   }

   return node;
}

std::string                         // The ELEMENT name
   ElemHtmlNode::getName( void ) const // Get ELEMENT name
{
   return name;
}

HtmlNode*                           // The first child HtmlNode
   ElemHtmlNode::getChild( void ) const // Get first child HtmlNode
{
   return head;
}

HtmlNode::Type                      // The HtmlNode Type
   ElemHtmlNode::getType( void ) const // Get HtmlNode Type
{
   return TYPE_ELEM;
}

//----------------------------------------------------------------------------
//
// Method-
//       ElemHtmlNode::insertChild
//
// Purpose-
//       Add a child HtmlNode
//
//----------------------------------------------------------------------------
void
   ElemHtmlNode::insertChild(       // Insert child HtmlNode
     HtmlNode*         child)       // The new child
{
   if( child->getParent() != NULL )
   {
     logf("HtmlNode::insert(%p)\n", child);
     throw "DuplicateParentException";
   }

   child->parent= this;
   child->peer= NULL;

   if( tail == NULL )
     this->head= child;
   else
     tail->peer= child;
   tail= child;
}

//----------------------------------------------------------------------------
//
// Method-
//       AttrHtmlNode::~AttrHtmlNode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   AttrHtmlNode::~AttrHtmlNode( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AttrHtmlNode::AttrHtmlNode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   AttrHtmlNode::AttrHtmlNode(      // Constructor
     std::string       name,        // Attribute name
     std::string       value)       // Attribute value
:  HtmlNode()
,  name(name)
,  data(value)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       AttrHtmlNode::Accessors
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
std::string                         // The ATTRIBUTE value
   AttrHtmlNode::getData( void ) const // Get ATTRIBUTE value
{
   return data;
}

std::string                         // The ATTRIBUTE name
   AttrHtmlNode::getName( void ) const // Get ATTRIBUTE name
{
   return name;
}

HtmlNode::Type                      // The HtmlNode Type
   AttrHtmlNode::getType( void ) const // Get HtmlNode Type
{
   return TYPE_ATTR;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextHtmlNode::~TextHtmlNode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   TextHtmlNode::~TextHtmlNode( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       TextHtmlNode::TextHtmlNode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   TextHtmlNode::TextHtmlNode(      // Default constructor
     std::string       text)        // The text
:  HtmlNode()
,  data(text)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       TextHtmlNode::Accessors
//
// Purpose-
//       Accessors.
//
//----------------------------------------------------------------------------
std::string                         // The text
   TextHtmlNode::getData( void ) const // Get text
{
   return data;
}

HtmlNode::Type                      // The HtmlNode Type
   TextHtmlNode::getType( void ) const // Get HtmlNode Type
{
   return TYPE_TEXT;
}

