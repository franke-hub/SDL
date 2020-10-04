//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Vector.cpp
//
// Purpose-
//       Vector Object implementation.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <com/Debug.h>

#include "com/Vector.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include "com/ifmacro.h"

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::~Vector
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Vector<Object>::~Vector( void )  // Destructor
{
   IFHCDM( printf("%4d Vector(%p)::~Vector()\n", __LINE__, this); )

   count= 0;
   used= 0;

   delete [] refs;
   refs= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::Vector
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Vector<Object>::Vector( void )   // Constructor
:  Cloneable()
,  count(32)
,  used(0)
,  refs(new Ref<Object>[count])
{
   IFHCDM( printf("%4d: Vector(%p)::Vector()\n", __LINE__, this); )
}

   Vector<Object>::Vector(          // Copy constructor
     const Vector<Object>&
                       source)      // Source Vector<Object>
:  Cloneable()
,  count(source.count)
,  used(source.used)
,  refs(new Ref<Object>[count])
{
   IFHCDM(
     printf("%4d: Vector(%p)::Vector(Vector& %p)\n", __LINE__, this, &source);
   )

   for(unsigned i= 0; i<used; i++)
     refs[i]= source.refs[i];
}

   Vector<Object>::Vector(          // Constructor
     unsigned          count)       // Number of elements
:  Cloneable()
,  count(count)
,  used(0)
,  refs(new Ref<Object>[count])
{
   IFHCDM( printf("%4d: Vector(%p)::Vector(%d)\n", __LINE__, this, count); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
Vector<Object>&                     // Updated reference
   Vector<Object>::operator=(       // Assignment operator
     const Vector<Object>&
                       source)      // Source Vector<Object>
{
   IFHCDM(
     printf("%4d: Vector(%p)::operator=(%p)\n", __LINE__, this, &source);
   )

   count= source.count;
   used= source.used;

   Ref<Object>* refs= new Ref<Object>[count];
   for(unsigned i= 0; i<used; i++)
     refs[i]= source.refs[i];

   delete [] this->refs;

   this->refs= refs;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::operator[]
//
// Purpose-
//       Extract element of Vector<Object>
//
//----------------------------------------------------------------------------
Object*                             // Pointer reference
   Vector<Object>::operator[](      // Operator []
     unsigned          index) const // Index
{
   if( index >= used)
     indexException(index);

   return refs[index].get();
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::indexException
//
// Purpose-
//       Throw indexException.
//
//----------------------------------------------------------------------------
void
   Vector<Object>::indexException(  // Throw indexException
     unsigned int      index) const // For this index value
{
   throwf("Vector(%p)::indexException(%d)", this, index);
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::Object methods
//
// Purpose-
//       Override base Object methods
//
//----------------------------------------------------------------------------
int                                 // Result(<0,=0,>0)
   Vector<Object>::compare(         // Compare to
     const Object&     object) const // This object
{
   const Vector<Object>* that= dynamic_cast<const Vector<Object>*>(&object);
   if( that == NULL )
     compareCastException("Vector");

   unsigned M= used;
   if( M < that->used )
     M= that->used;

   for(unsigned i= 0; i<M; i++)
   {
     Object* L= this->refs[i].get();
     Object* R= that->refs[i].get();
     if( L != R )                   // If Objects physically differ
     {
       if( L == NULL )
         return +1;

       if( R == NULL )
         return -1;

       int cc= L->compare(*R);
       if( cc != 0 )
         return cc;
     }
   }

   if( this->used != that->used )
     return int(that->used - this->used);

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Vector<Object>::insert
//
// Purpose-
//       Insert an Object to the element array
//
//----------------------------------------------------------------------------
unsigned                            // The new element index
   Vector<Object>::insert(          // Insert a new element
     Object*           object)      // -> Object
{
   if( used >= count )
   {
     count += count/10;
     if( count < (used+32) )
       count= used + 32;

     Ref<Object>* clone= new Ref<Object>[count];
     for(unsigned i= 0; i<used; i++)
       clone[i]= refs[i];

     delete [] refs;
     refs= clone;
   }

   refs[used++]= object;
   return used-1;
}

