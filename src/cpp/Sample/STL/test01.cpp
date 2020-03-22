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
//       test01.cpp
//
// Purpose-
//       Test "Nice.h"
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "Main.h"
#include "Nice.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       testNice
//
// Purpose-
//       Test "Nice.h"
//
//----------------------------------------------------------------------------
static void
   testNice( void )                 // Test "Nice.h"
{
   Nice                left;
   Nice                middle(left);// Copy constructor
   Nice                right;

   verify( left < right );
   verify( middle < right );
   verify( left != right );
   verify( right != left );
   verify( right != middle );
   verify( middle != right );
   verify( middle == left );
   verify( left == middle );

   // Assignment
   right= left;
   verify( left == right );
   verify( middle == right );
   verify( right == middle );
   verify( !(left < right) );
   verify( !(left != right) );

   // Equality preserving
   left.s();
   middle.s();
   right.s();
   verify( left == right );
   verify( middle == right );
   verify( right == middle );
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test01
//
// Purpose-
//       Test "Nice.h"
//
//----------------------------------------------------------------------------
extern void
   test01( void )
{
   wtlc(LevelStd, "test01()\n");

   testNice();
}

