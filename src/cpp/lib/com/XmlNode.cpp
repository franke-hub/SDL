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
//       XmlNode.cpp
//
// Purpose-
//       XmlNode object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isspace()
#include <stdarg.h>                 // For botch()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>              // Used in debug method
#include <com/Buffer.h>
#include <com/Reader.h>
#include <com/Writer.h>

#include "com/XmlNode.h"
#include "com/XmlParser.h"
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifdef _OS_WIN
  #define vsnprintf _vsnprintf
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     typeName[]=
{  "ERROR"
,  "ENTITY"
,  "ATTR"
,  "ELEM"
,  "ROOT"
,  "TEXT"
,  "COMMENT"
,  "CDATA"
,  "DECL"
,  "DESC"
}; // typeName[]

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::~XmlNode
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   XmlNode::~XmlNode( void )        // Destructor
{
   detach();                        // Remove from parent node
   deleteAttrib();                  // Delete attribute nodes
   deleteChild();                   // Delete child nodes
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::XmlNode
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   XmlNode::XmlNode( void )         // Default constructor
:  List<XmlNode>::Link()
,  type(TYPE_RESET), parent(NULL), name(), data(), attrib(), child()
{
   setPrev(NULL);
   setNext(NULL);
}

   XmlNode::XmlNode(                // Constructor
     int               type,        // Node type
     const std::string&name,        // Node name
     const std::string&data)        // Node value
:  List<XmlNode>::Link()
,  type(type), parent(NULL), name(name), data(data), attrib(), child()
{
   setPrev(NULL);
   setNext(NULL);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::debug
//
// Purpose-
//       Object method: Debugging display
//
//----------------------------------------------------------------------------
void
   XmlNode::debug(                  // Debugging display
     int               level)       // Nesting level
{
   XmlParser           parser;      // EMPTY parser

   parser.debug(level, this);
}

void
   XmlNode::debug( void )           // Debugging display
{
   debug(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::deleteAttrib
//
// Purpose-
//       Object method: Delete attribute nodes
//
//----------------------------------------------------------------------------
void
   XmlNode::deleteAttrib( void )    // Delete attribute nodes
{
   for(;;)
   {
     XmlNode* node= (XmlNode*)attrib.remq(); // Delete attribute nodes
     if( node == NULL )
       break;

     node->parent= NULL;
     delete node;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::deleteChild
//
// Purpose-
//       Object method: Delete child nodes
//
//----------------------------------------------------------------------------
void
   XmlNode::deleteChild( void )     // Delete child nodex
{
   for(;;)
   {
     XmlNode* node= (XmlNode*)child.remq(); // Delete child nodes
     if( node == NULL )
       break;

     node->parent= NULL;
     delete node;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::detach
//
// Purpose-
//       Object method: Remove this node from parent node
//
//----------------------------------------------------------------------------
void
   XmlNode::detach( void )          // Remove this node from parent node
{
   if( parent != NULL )
   {
     if( parent->child.isOnList(this) )
       parent->child.remove(this, this);
     else if( parent->attrib.isOnList(this) )
       parent->attrib.remove(this, this);

     parent= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getAttribCount
//
// Purpose-
//       Get the attribute node count
//
//----------------------------------------------------------------------------
int
   XmlNode::getAttribCount( void ) const
{
   int count= 0;
   XmlNode* node= attrib.getHead();
   while( node != NULL )
   {
     count++;
     node= node->getNext();
   }

   return count;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getAttrib
//
// Purpose-
//       Get an attribute node
//
//----------------------------------------------------------------------------
XmlNode*
   XmlNode::getAttrib(
     int               index) const
{
   int count= 0;
   XmlNode* node= attrib.getHead();
   while( node != NULL )
   {
     if( count == index )
       return node;

     count++;
     node= node->getNext();
   }

   return NULL;
}

XmlNode*
   XmlNode::getAttrib(
     const string&       name) const
{
   XmlNode* node= attrib.getHead();
   while( node != NULL )
   {
     if( node->name == name )
       return node;

     node= node->getNext();
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getChildCount
//
// Purpose-
//       Get the child node count
//
//----------------------------------------------------------------------------
int
   XmlNode::getChildCount( void ) const
{
   int count= 0;
   XmlNode* node= child.getHead();
   while( node != NULL )
   {
     count++;
     node= node->getNext();
   }

   return count;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getChild
//
// Purpose-
//       Get a child node
//
//----------------------------------------------------------------------------
XmlNode*
   XmlNode::getChild(
     int               index) const
{
   int count= 0;
   XmlNode* node= child.getHead();
   while( node != NULL )
   {
     if( count == index )
       return node;

     count++;
     node= node->getNext();
   }

   return NULL;
}

XmlNode*
   XmlNode::getChild(
     const string&     name) const
{
   XmlNode* node= child.getHead();
   while( node != NULL )
   {
     if( node->name == name )
       return node;

     node= node->getNext();
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getName
//
// Purpose-
//       Extract associated name.
//
//----------------------------------------------------------------------------
string                              // The associated name
   XmlNode::getName( void ) const   // Get associated name
{
   return name;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getParent
//
// Purpose-
//       Extract associated parent
//
//----------------------------------------------------------------------------
XmlNode*                            // The associated parent node
   XmlNode::getParent( void ) const // Get associated parent node
{
   return parent;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getType
//
// Purpose-
//       Extract associated node type.
//
//----------------------------------------------------------------------------
int                                 // The associated node type
   XmlNode::getType( void ) const   // Get associated node type
{
   return type;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::getValue
//
// Purpose-
//       Extract associated node value.
//
//----------------------------------------------------------------------------
string                              // The associated node value
   XmlNode::getValue( void ) const  // Get associated node value
{
   return data;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::setValue
//
// Purpose-
//       Replace associated node value.
//
//----------------------------------------------------------------------------
void
   XmlNode::setValue(               // Set associated node value
     string            value)       // The replacement value
{
   data= value;
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::insert
//
// Purpose-
//       Object method: Insert a node
//
//----------------------------------------------------------------------------
void
   XmlNode::insert(                 // Insert
     XmlNode*          node)        // This node
{
   node->parent= this;

   if( node->type == TYPE_ATTR )
     attrib.fifo(node);
   else
     child.fifo(node);
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::insertBefore
//
// Purpose-
//       Object method: Insert a node (ordered)
//
//----------------------------------------------------------------------------
void
   XmlNode::insertBefore(           // Insert
     XmlNode*          node,        // This node
     int               index)       // BEFORE this node
{
   node->parent= this;

   if( node->type == TYPE_ATTR )
   {
     XmlNode* link= getAttrib(index);
     if( link != NULL )
       link= link->getPrev();

     attrib.insert(link, node, node);
   }
   else
   {
     XmlNode* link= getChild(index);
     if( link != NULL )
       link= link->getPrev();

     child.insert(link, node, node);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       XmlNode::type2name
//
// Purpose-
//       Convert type number to name. (Used in debug.)
//
//----------------------------------------------------------------------------
const char*                         // The associated name
   XmlNode::type2name(              // Get associated name
     int               type)        // For this type
{
   const char* result= "????";      // Resultant
   if( type >= 0 && type < TYPE_COUNT )
     result= typeName[type];

   return result;
}

