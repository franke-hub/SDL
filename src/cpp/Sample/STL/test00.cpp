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
//       test00.cpp
//
// Purpose-
//       Demonstrate function operator usage.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <cstddef>
#include <iostream>

#include "Main.h"
#include "Function.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       myOP
//
// Purpose-
//       Define an operator structure.
//
//----------------------------------------------------------------------------
struct myOP
{
   bool operator()(const char* l, const char* r) const
   {
     printf("myOP(%s,%s)\n", l, r);

     return strcmp(l, r) < 0;
   }
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       testOp<typename _TYP, typename _Compare>
//
// Purpose-
//       Demonstrate function operator usage.
//
//----------------------------------------------------------------------------
template<typename _TYP, typename _Compare>
static bool                         // Resultant
   testOp(                          // Use function operator
     _TYP              lhs,         // Left Hand Side
     _TYP              rhs)         // Right Hand Side
{
   _Compare            op;          // Compare operator

   return op(lhs,rhs);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test00
//
// Purpose-
//       Demonstrate function operator usage.
//
//----------------------------------------------------------------------------
extern void
   test00( void )
{
   wtlc(LevelStd, "test00()\n");

   {{{{
     wtlc(LevelInfo, "test00() usage\n");

     gtSTR             gtTEST;
     ltSTR             ltTEST;

     verify( (gtTEST("A", "B") == FALSE) );
     verify( (gtTEST("B", "A") == TRUE) );

     verify( (ltTEST("A", "B") == TRUE) );
     verify( (ltTEST("B", "A") == FALSE) );

     if( getLogLevel() < LevelStd )
     {
       struct combo {
         gtSTR gtTEST;
         ltSTR ltTEST;
         myOP  myTEST;
       } combo;
       myOP myTEST;

       printf("%4ld sizeof(gtSTR)\n", (long)sizeof(gtTEST));
       printf("%4ld sizeof(ltSTR)\n", (long)sizeof(ltTEST));
       printf("%4ld sizeof(myOP)\n",  (long)sizeof(myTEST));
       printf("%4ld sizeof(combo)\n", (long)sizeof(combo));

       verify( (myTEST("A", "B") == TRUE) );
       verify( (myTEST("B", "A") == FALSE) );
       verify( (combo.gtTEST("A","B") == FALSE) );
       verify( (combo.ltTEST("A","B") == TRUE) );
       verify( (combo.myTEST("A","B") == TRUE) );
     }
   }}}}

   {{{{
     wtlc(LevelInfo, "test00() template\n");

     verify( (testOp<const char*, gtSTR>("A", "B") == FALSE) );
     verify( (testOp<const char*, gtSTR>("B", "A") == TRUE) );

     verify( (testOp<const char*, ltSTR>("A", "B") == TRUE) );
     verify( (testOp<const char*, ltSTR>("B", "A") == FALSE) );
   }}}}
}

