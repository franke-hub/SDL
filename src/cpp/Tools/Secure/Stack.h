//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stack.h
//
// Purpose-
//       Stack manipulation utilities.
//
// Last change date-
//       2007/01/01                 Version 2, Release 1
//
//----------------------------------------------------------------------------
#ifndef STACK_H_INCLUDED
#define STACK_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       StackBase
//
// Purpose-
//       Stack base class descriptor
//
//----------------------------------------------------------------------------
class StackBase {                   // Stack base class descriptor
//----------------------------------------------------------------------------
// StackBase::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~StackBase( void );              // Destructor
   StackBase( void );               // Constructor

private:                            // Bitwise copy is prohibited
   StackBase(const StackBase&);             // Disallowed copy constructor
   StackBase& operator=(const StackBase&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// StackBase::Methods
//----------------------------------------------------------------------------
public:
void
   reset( void );                   // Reset the stack

//----------------------------------------------------------------------------
// Stack::Attributes
//----------------------------------------------------------------------------
protected:
   int                 top;         // Top of stack index
   int                 bot;         // End of stack index
}; // class StackBase

//----------------------------------------------------------------------------
//
// Class-
//       Stack
//
// Purpose-
//       Stack descriptor
//
//----------------------------------------------------------------------------
template<class T, int depth>
class Stack : public StackBase {    // Stack descriptor
//----------------------------------------------------------------------------
// Stack::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Stack( void );                  // Destructor
   Stack( void );                   // Constructor

private:                            // Bitwise copy is prohibited
   Stack(const Stack&);             // Disallowed copy constructor
   Stack& operator=(const Stack&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Stack::Methods
//----------------------------------------------------------------------------
public:
void
   coherencyDebug( void ) const;    // Debugging tool

void
   fifo(                            // Push onto Stack, FIFO ordering
     T                 element);    // The element to push

void
   lifo(                            // Push onto Stack, LIFO ordering
     T                 element);    // The element to push

T                                   // Resultant element
   pull( void );                    // Pull from Stack

//----------------------------------------------------------------------------
// Stack::Attributes
//----------------------------------------------------------------------------
private:
   T                   array[depth];// The stack array
}; // class Stack

#endif // STACK_H_INCLUDED
