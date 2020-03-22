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
//       testVector.cpp
//
// Purpose-
//       Test vector.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <vector>

#include "Main.h"
#include "Nice.h"
#include "NoisyAllocator.h"
#include "NoisyNice.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define DIM_ARRAY 4

//----------------------------------------------------------------------------
//
// Subroutine-
//       test00
//
// Purpose-
//       Simple vector test.
//
//----------------------------------------------------------------------------
static void
   test00( void )
{
   wtlc(LevelInfo, "testVector::test00()\n");

   int                 count;
   vector<Nice*>       niceVector;
   vector<Nice*>::iterator
                       nvI;

   int                 i;

   wtlc(LevelAll, "Load the Vector\n");
   for(i= 0; i<DIM_ARRAY; i++)
     niceVector.push_back(new Nice());

   wtlc(LevelAll, "Test the Vector\n");
   for(i= 1; i<DIM_ARRAY; i++)
   {
     if( !verify(niceVector[i-1]->s() < niceVector[i]->s()) )
     {
       debugf("[%2d] %6d\n", i-1, niceVector[i-1]->s());
       debugf("[%2d] %6d\n", i, niceVector[i]->s());
       break;
     }
   }

   wtlc(LevelAll, "Test the Vector::iterator\n");
   count= 0;
   int prior= 0;
   for(nvI= niceVector.begin(); nvI != niceVector.end(); nvI++)
   {
     wtlc(LevelAll, "[%2d] %6d\n", count, (*nvI)->s());
     verify( prior < (*nvI)->s() );
     prior= (*nvI)->s();
     count++;
   }
   verify( count == DIM_ARRAY );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test01
//
// Purpose-
//       A vector containing objects, not pointers.
//
//----------------------------------------------------------------------------
static void
   test01( void )
{
   wtlc(LevelInfo, "testVector::test01()\n");

   typedef NoisyAllocator<NoisyNice>
                       allocator;
   int                 count;
   vector<NoisyNice, allocator>
                       niceVector;
   vector<NoisyNice, allocator>::iterator
                       nvI;
   NoisyNice           source;

   int                 i;

   wtlc(LevelAll, "Load the Vector\n");
   for(i= 0; i<DIM_ARRAY; i++)
   {
     source.i();
     niceVector.push_back(source);
   }

   wtlc(LevelAll, "Test the Vector\n");
   wtlc(LevelAll, "[%2d] %6d\n", 0, niceVector[0].s());
   for(i= 1; i<DIM_ARRAY; i++)
   {
     wtlc(LevelAll, "[%2d] %6d\n", i, niceVector[i].s());
     if( !verify(niceVector[i-1].s() < niceVector[i].s()) )
     {
       debugf("[%2d] %6d\n", i-1, niceVector[i-1].s());
       debugf("[%2d] %6d\n", i, niceVector[i].s());
       break;
     }
   }

   wtlc(LevelAll, "Test the Vector::iterator\n");
   count= 0;
   int prior= 0;
   for(nvI= niceVector.begin(); nvI != niceVector.end(); nvI++)
   {
     wtlc(LevelAll, "[%2d] %6d\n", count, (*nvI).s());
     verify( prior < (*nvI).s() );
     prior= (*nvI).s();
     count++;
   }
   verify( count == DIM_ARRAY );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testVector
//
// Purpose-
//       Test <vector>
//
//----------------------------------------------------------------------------
extern void
   testVector( void )
{
   wtlc(LevelStd, "testVector()\n");

   test00();
   test01();
}

