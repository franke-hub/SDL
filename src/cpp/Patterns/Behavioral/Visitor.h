//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Visitor.h
//
// Purpose-
//       Represent an operation to be performed on the elements of an object
//       structure.  Visitor lets you define a new operation without changing
//       the classes on which it operates.
//
// Last change date-
//       2017/01/01
//
// Also known as-
//       Policy
//
//----------------------------------------------------------------------------
#ifndef VISITOR_H_INCLUDED
#define VISITOR_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class AbstractNode;
class AbstractVisitor;

//----------------------------------------------------------------------------
//
// Class-
//       AbstractNode
//
// Purpose-
//       Define the AbstractNode object.
//
//----------------------------------------------------------------------------
class AbstractNode : public Object
{
//----------------------------------------------------------------------------
// AbstractNode::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractNode( void );
   AbstractNode( void );

//----------------------------------------------------------------------------
// AbstractNode::Methods
//----------------------------------------------------------------------------
virtual void
   accept(
     AbstractVisitor*  visitor) = 0;

//----------------------------------------------------------------------------
// AbstractNode::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractNode

//----------------------------------------------------------------------------
//
// Class-
//       AssignmentNode
//
// Purpose-
//       Define the AssignmentNode object.
//
//----------------------------------------------------------------------------
class AssignmentNode : public AbstractNode
{
//----------------------------------------------------------------------------
// AssignmentNode::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AssignmentNode( void );
   AssignmentNode( void );

//----------------------------------------------------------------------------
// AssignmentNode::Methods
//----------------------------------------------------------------------------
virtual void
   accept(
     AbstractVisitor*  visitor)
{
   visitor->assign(this);
}

//----------------------------------------------------------------------------
// AssignmentNode::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AssignmentNode

//----------------------------------------------------------------------------
//
// Class-
//       ReferenceNode
//
// Purpose-
//       Define the ReferenceNode object.
//
//----------------------------------------------------------------------------
class ReferenceNode : public AbstractNode
{
//----------------------------------------------------------------------------
// ReferenceNode::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~ReferenceNode( void );
   ReferenceNode( void );

//----------------------------------------------------------------------------
// ReferenceNode::Methods
//----------------------------------------------------------------------------
virtual void
   accept(
     AbstractVisitor*  visitor)
{
   visitor->refers(this);
}

//----------------------------------------------------------------------------
// ReferenceNode::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class ReferenceNode

//----------------------------------------------------------------------------
//
// Class-
//       AbstractVisitor
//
// Purpose-
//       Define the AbstractVisitor object.
//
//----------------------------------------------------------------------------
class AbstractVisitor : public Object
{
//----------------------------------------------------------------------------
// AbstractVisitor::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractVisitor( void );
   AbstractVisitor( void );

//----------------------------------------------------------------------------
// AbstractVisitor::Methods
//----------------------------------------------------------------------------
virtual void
   assign(
     AbstractNode*     node) = 0;

virtual void
   refers(
     AbstractNode*     node) = 0;

//----------------------------------------------------------------------------
// AbstractVisitor::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class AbstractVisitor

#endif  // VISITOR_H_INCLUDED
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//
// Class-
//       SampleClient
//
// Purpose-
//       Sample usage.
//
//----------------------------------------------------------------------------
class SampleClient : public Object
{
void*
   run( void )
{
   AbstractVisitor*    visitor= new ConcreteVisitor();

   visitor.accept(new ReferenceNode());
   visitor.accept(new AssignmentNode());
} // void run
} // class SampleClient

