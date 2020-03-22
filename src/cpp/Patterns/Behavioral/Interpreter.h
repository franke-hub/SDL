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
//       Interpreter.h
//
// Purpose-
//       Given a language, define a representation for its grammar along
//       with an interpreter that uses the representation to interpret
//       sentences in tha language.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef INTERPRETER_H_INCLUDED
#define INTERPRETER_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       Context
//
// Purpose-
//       Define a Context object.
//       The context contains the parser state.
//
//----------------------------------------------------------------------------
class Context : public Object
{
//----------------------------------------------------------------------------
// Context::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Context( void );
   Context( void );

   Context(const Context&);         // Copy constructor
   operator=(const Context&);       // Assignment operator

//----------------------------------------------------------------------------
// Context::Methods
//----------------------------------------------------------------------------
   // None defined

//----------------------------------------------------------------------------
// Context::Attributes
//----------------------------------------------------------------------------
   // None defined
}; // class Context

//----------------------------------------------------------------------------
//
// Class-
//       AbstractExpression
//
// Purpose-
//       Define the AbstractExpression object.
//
//----------------------------------------------------------------------------
class AbstractExpression : public Object
{
//----------------------------------------------------------------------------
// AbstractExpression::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractExpression( void );

protected:
   AbstractExpression( void ):

//----------------------------------------------------------------------------
// AbstractExpression::Methods
//----------------------------------------------------------------------------
public:
virtual Context*
   interpret(
     Context*          context) = 0;

//----------------------------------------------------------------------------
// AbstractExpression::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractExpression

//----------------------------------------------------------------------------
//
// Class-
//       TerminalExpression
//
// Purpose-
//       Define a TerminalExpression object.
//
//----------------------------------------------------------------------------
class TerminalExpression : public AbstractExpression
{
//----------------------------------------------------------------------------
// TerminalExpression::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~TerminalExpression( void );
   TerminalExpression( void );

//----------------------------------------------------------------------------
// TerminalExpression::Methods
//----------------------------------------------------------------------------
public:
virtual Context*
   interpret(
     Context*          context);

//----------------------------------------------------------------------------
// TerminalExpression::Attributes
//----------------------------------------------------------------------------
   unsigned            count;
   AbstractExpression**
                       expression;
}; // class TerminalExpression

//----------------------------------------------------------------------------
//
// Class-
//       IntermediateExpression
//
// Purpose-
//       Define a IntermediateExpression object.
//
//----------------------------------------------------------------------------
class IntermediateExpression : public AbstractExpression
{
//----------------------------------------------------------------------------
// IntermediateExpression::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~IntermediateExpression( void );
   IntermediateExpression(unsigned count, ...);

//----------------------------------------------------------------------------
// IntermediateExpression::Methods
//----------------------------------------------------------------------------
public:
virtual Context*
   interpret(
     Context*          context);

//----------------------------------------------------------------------------
// IntermediateExpression::Attributes
//----------------------------------------------------------------------------
   unsigned            count;
   AbstractExpression**
                       expression;
}; // class IntermediateExpression

#endif  // INTERPRETER_H_INCLUDED
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
   Context*            context= new Context()
   AbstractExpression* one= new TerminalExpression();
   AbstractExpression* two= new IntermediateExpression(2, one, one);

   two->interpret(context);
} // void run
} // class SampleClient

