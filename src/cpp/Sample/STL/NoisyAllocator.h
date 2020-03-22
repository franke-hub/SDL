//----------------------------------------------------------------------------
//
//       Copyright (c) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NoisyAllocator.h
//
// Purpose-
//       Describes an "allocator" class as defined by the STL.
//
// Last change date-
//       2017/01/01
//
// Implementation notes-
//       This class logs all of its operations.
//
//----------------------------------------------------------------------------
#ifndef NOISYALLOCATOR_H_INCLUDED
#define NOISYALLOCATOR_H_INCLUDED

#ifndef MAIN_H_INCLUDED
#include "Main.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       NoisyAllocator
//
// Purpose-
//       A noisy allocator class.
//
//----------------------------------------------------------------------------
template<typename _Tp>
class NoisyAllocator
{
//----------------------------------------------------------------------------
// NoisyAllocator::Attributes
//----------------------------------------------------------------------------
public:
   typedef size_t      size_type;
   typedef ptrdiff_t   difference_type;
   typedef _Tp*        pointer;
   typedef const _Tp*  const_pointer;
   typedef _Tp&        reference;
   typedef const _Tp&  const_reference;
   typedef _Tp         value_type;

   template<typename _Tp1>
     struct rebind
     { typedef NoisyAllocator<_Tp1> other; };

//----------------------------------------------------------------------------
// NoisyAllocator::Constructors
//----------------------------------------------------------------------------
public:
   ~NoisyAllocator( void ) throw()
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::~NoisyAllocator()\n", this);
}

   NoisyAllocator( void ) throw()
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::NoisyAllocator()\n", this);
}

   NoisyAllocator(const NoisyAllocator& source) throw()
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::NoisyAllocator(%p)\n", this, &source);
}

   template<typename _Tp1>
   NoisyAllocator(const NoisyAllocator<_Tp1>&) throw()
   { wtlc(LevelInfo, "NoisyAllocator<>(%p)::NoisyAllocator(*)\n", this); }

//----------------------------------------------------------------------------
// NoisyAllocator::Methods
//----------------------------------------------------------------------------
public:
pointer
   address(reference __r) const     // Convert reference to pointer
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::address(%p)\n", this, &__r);
   return &__r;
}

const_pointer
   address(const_reference __r) const // Convert const_reference to const_pointer
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::address(const %p)\n", this, &__r);
   return &__r;
}

pointer
   allocate(size_type __n, const void* = 0)
{
   pointer result= static_cast<pointer>(::operator new(__n * sizeof(_Tp)));

   wtlc(LevelInfo, "%p= NoisyAllocator(%p)::allocate(%ld,*)\n",
                   result, this, (long)__n);
   return result;
}

void
   deallocate(pointer __p, size_type)
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::deallocate(%p,*)\n", this, __p);

   ::operator delete(__p);
}

size_type
   max_size() const throw()
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::max_size()\n", this);
   return size_t(-1) / sizeof(_Tp);
}

void
   construct(pointer __p, const _Tp& __val)
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::construct(%p,%p)\n",
                   this, __p, &__val);
   ::new(__p) _Tp(__val);
}

void
   destroy(pointer __p)
{
   wtlc(LevelInfo, "NoisyAllocator(%p)::destroy(%p)\n", this, __p);
   __p->~_Tp();
}
}; // class NoisyAllocator

//----------------------------------------------------------------------------
// Global operators
//----------------------------------------------------------------------------
template<typename _Tp>
inline bool
   operator==(const NoisyAllocator<_Tp>&, const NoisyAllocator<_Tp>&)
   { return true; }
  
template<typename _Tp>
inline bool
   operator!=(const NoisyAllocator<_Tp>&, const NoisyAllocator<_Tp>&)
   { return false; }

#endif // NOISYALLOCATOR_H_INCLUDED
