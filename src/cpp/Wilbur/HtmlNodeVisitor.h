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
//       HtmlNodeVisitor.h
//
// Purpose-
//       HtmlNodeVisitor base class.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef HTMLNODEVISITOR_H_INCLUDED
#define HTMLNODEVISITOR_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HtmlNode;

//----------------------------------------------------------------------------
//
// Class-
//       HtmlNodeVisitor
//
// Purpose-
//       HtmlNode visitor base class.
//
//----------------------------------------------------------------------------
class HtmlNodeVisitor {             // HtmlNode Visitor
//----------------------------------------------------------------------------
// HtmlNodeVisitor::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HtmlNodeVisitor( void );        // Destructor
   HtmlNodeVisitor( void );         // Default constructor

private:                            // Disallowed
   HtmlNodeVisitor(const HtmlNodeVisitor& source); // Copy constructor
HtmlNodeVisitor& operator=(const HtmlNodeVisitor& source); // Assignment operator

//----------------------------------------------------------------------------
// HtmlNodeVisitor::Methods
//----------------------------------------------------------------------------
public:
/**
* This method should be overridden in a derived class. The base method
* does nothing, and does not visit child nodes.
*
**/
virtual int                         // Return code (0 to visit child nodes)
   visit(                           // Visit
     HtmlNode*         node);       // This HtmlNode
}; // class HtmlNodeVisitor

#endif // HTMLNODEVISITOR_H_INCLUDED
