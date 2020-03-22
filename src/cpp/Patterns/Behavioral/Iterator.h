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
//       Iterator.h
//
// Purpose-
//       Provide an independent mechanism for enumeration.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef ITERATOR_H_INCLUDED
#define ITERATOR_H_INCLUDED
#include ".Object/Object.h"

//----------------------------------------------------------------------------
//
// Class-
//       AbstractIterator
//
// Purpose-
//       Define the AbstractIterator object.
//
//----------------------------------------------------------------------------
class AbstractIterator : public Object
{
//----------------------------------------------------------------------------
// AbstractIterator::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~AbstractIterator( void );

protected:
   AbstractIterator( void ):

//----------------------------------------------------------------------------
// AbstractIterator::Methods
//----------------------------------------------------------------------------
public:
virtual Object*
   first( void ) = 0;

virtual Object*
   next( void ) = 0;

virtual Object*
   current( void ) const = 0;

virtual int
   isValid( void ) const = 0;

//----------------------------------------------------------------------------
// AbstractIterator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class AbstractIterator

//----------------------------------------------------------------------------
//
// Class-
//       Iteratee
//
// Purpose-
//       Define an Iteratee object.
//
//----------------------------------------------------------------------------
class Iteratee : public Object
{
//----------------------------------------------------------------------------
// Iteratee::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Iteratee( void );
   Iteratee( void ):

//----------------------------------------------------------------------------
// Iteratee::Methods
//----------------------------------------------------------------------------
public:
virtual AbstractIterator*
   iterate( void );

//----------------------------------------------------------------------------
// Iteratee::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class Iteratee

#endif  // ITERATOR_H_INCLUDED
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
   Iteratee*           iteratee= new Iteratee();

   AbstractIterator* iterator= iteratee.iterate();
   for(Object* ptrO= iterator.first();
       iterator.isValid();
       ptrO= iterator.next())
   {
   }
   delete iterator;

   delete iteratee;
} // void run
} // class SampleClient

