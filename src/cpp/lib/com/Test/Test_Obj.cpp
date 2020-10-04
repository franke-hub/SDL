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
//       Test_Obj.cpp
//
// Purpose-
//       Test Object implementation.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>

#include "com/Object.h"
#include "com/Vector.h"

using namespace std;                // For cout, endl

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static unsigned        errorCount= 0; // Error counter

//----------------------------------------------------------------------------
//
// Class-
//       Noisy
//
// Purpose-
//       Noisy String
//
//----------------------------------------------------------------------------
class Noisy : public String {
public:
virtual
   ~Noisy( void )
{  debugf("Noisy(%p)::~Noisy() %s\n", this, c_str()); }

   Noisy(unsigned data) : String("%d", data)
{  debugf("Noisy(%p)::Noisy(%d)\n", this, data); }
}; // class Noisy

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   {{{{
     printf("Scope one...\n");
     Ref<Object> one= new Object();
     Ref<Object> two= new Object();
     two.set(new Object());
     cout << two << endl;
     printf("...Scope one\n");
   }}}}

   {{{{
     printf("Scope two...\n");
     Ref<Object> one= new String("one");
     Ref<String> two= new String("two");
     two.set(new String("333"));
     cout << two << endl;
     printf("...Scope two\n");
   }}}}

   {{{{
     printf("Scope three..\n");
     Ref< Vector<Noisy> > one= new Vector<Noisy>();
     for(int i= 1; i<1000; i++)
       one->insert(new Noisy(i));
     printf("...Scope three\n");
   }}}}

   {{{{
     printf("Error checks..\n");

     // Verify Ref<T>= Ref<Object> throws exception
     Ref<String> s= new String("s");
     Ref<Object> o;

     // Verify Ref<T>= Ref<Object> (of type Ref<T>) Works
     o= new String("t");
     try {
       s= o;
       printf("%4d As expected, no exception\n", __LINE__);
     } catch(const char* X) {
       errorCount++;
       printf("%4d ERROR: exception(%s)\n", __LINE__, X);
     }

     o= new Object();
     try {
       s= o;

       errorCount++;
       printf("%4d ERROR: exception not thrown\n", __LINE__);
     } catch(const char* X) {
       printf("%4d As expected, exception(%s)\n", __LINE__, X);
     }
   }}}}

   // Insure that no allocated Objects remain
   if( Object::getObjectCounter() == 0 ) // If no allocated Objects remain
     printf("%4d As expected, Object::objectCount == 0\n", __LINE__);
   else                           // If Objects remain
   {
     errorCount++;
     printf("ERROR: Object::objectCount(%d) != 0\n", Object::getObjectCounter());
   }

   printf("...Error checks\n");

   // Testing complete
   if( errorCount == 0 )
     printf("NO errors detected\n");
   else if( errorCount == 1 )
     printf("1 error detected\n");
   else
     printf("%u errors detected\n", errorCount);

   return 0;
}

