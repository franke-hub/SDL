//----------------------------------------------------------------------------
//
//       Copyright (c) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_constructor.cpp
//
// Purpose-
//       Figure out when and why "Class_name value();" fails.
//
// Last change date-
//       2019/02/16
//
// When and why-
//       "Class_name variable();" MUST ALWAYS BE interpreted as:
//       function variable() returning Class_name, per C++ standard.
//       (Called "most vexing parse" ambiguity resolution (Scott Meyers).)
//
//       "Class_name variable{};"
//
//----------------------------------------------------------------------------
#include <exception>
#include <string>

#include <assert.h>
#include <stdio.h>

#include <pub/Debug.h>
using namespace pub::debugging;

//----------------------------------------------------------------------------
//
// Class-
//       Has_default_constructor
//
// Purpose-
//       Define class with a default constructor
//
//----------------------------------------------------------------------------
class Has_default_constructor {
protected:
std::string            validator;

public:
   Has_default_constructor( void )
:  validator("is valid") {}

bool
   is_valid( void )
{  return (validator == "is valid"); }
}; // class Has_default_constructor

//----------------------------------------------------------------------------
//
// Class-
//       No_default_constructor
//
// Purpose-
//       Define class without a default constructor
//
//----------------------------------------------------------------------------
class No_default_constructor {
protected:
std::string            validator= "is valid";

public:
bool
   is_valid( void )
{  return (validator == "is valid"); }
}; // class No_default_constructor

//----------------------------------------------------------------------------
//
// Class-
//       Has_constructor
//
// Purpose-
//       Define class with a constructor
//
//----------------------------------------------------------------------------
class Has_constructor {
protected:
std::string            validator;

public:
   Has_constructor(std::string S)
:  validator(S) {}

bool
   is_valid( void )
{  return (validator == "is valid"); }
}; // class Has_constructor

//----------------------------------------------------------------------------
//
// Class-
//       Has_defaultable_constructor
//
// Purpose-
//       Define class with a defaultable constructor
//
//----------------------------------------------------------------------------
class Has_defaultable_constructor {
protected:
std::string            validator;

public:
   Has_defaultable_constructor(std::string S= "is valid")
:  validator(S) {}

bool
   is_valid( void )
{  return (validator == "is valid"); }
}; // class Has_defaultable_constructor

//----------------------------------------------------------------------------
// The 'dirty' command: For quick and dirty testing
//----------------------------------------------------------------------------
static inline int                   // Return code
   dirty( void )                    // The 'dirty' command
{
   int errorCount= 0;               // Number of errors encountered

   debugf("dirty...\n");

   Has_default_constructor thing1;
// Has_default_constructor thing2(); // thing2 is a function returning a Has_default_constructor
// Has_default_constructor thing2(){}; // (Invalid syntax)
// Has_default_constructor thing2{}(); // (Invalid syntax)
   Has_default_constructor thing2{}; // (not a function)

   No_default_constructor thing3;
// No_default_constructor thing4(); // thing4 is a function returning a No_default_constructor
   No_default_constructor thing4{}; // (What you might expect)

// Has_constructor thing5(nullptr); // EXCEPTION: std::string(nullptr)
   Has_constructor thing6("is valid");
// Has_constructor thing7;          // Constructor required!
   Has_constructor thing8();        // thing8 is a function returning a Has_constructor

// Has_defaultable_constructor thing9(nullptr); // EXCEPTION: std::string(nullptr)
   Has_defaultable_constructor thingA("is valid");
   Has_defaultable_constructor thingB;
// Has_defaultable_constructor thingC(); // thingC is a function returning a Has_defaultable_constructor
// Has_defaultable_constructor thingD{"NOT valid"}; // Valid syntax, wrong result
   Has_defaultable_constructor thingE{"is valid"}; // Valid syntax, valid result

   assert( thing1.is_valid() );
   assert( thing2.is_valid() );     // thing2 is corrected
   assert( thing3.is_valid() );
   assert( thing4.is_valid() );     // thing4 is corrected
// assert( thing5.is_valid() );     // thing5 is NOT valid
   assert( thing6.is_valid() );
// assert( thing7.is_valid() );     // thing7 constructor required
// assert( thing8.is_valid() );     // thing8 is a function, not a variable
// assert( thing9.is_valid() );     // thing9 is NOT valid
   assert( thingA.is_valid() );
   assert( thingB.is_valid() );
// assert( thingC.is_valid() );     // thingC is a function, not a variable
// assert( thingD.is_valid() );     // thingE is NOT valid
   assert( thingE.is_valid() );

   printf("...dirty\n");
   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline entry
     int             argc,          // Parameter count
     const char*     argv[])        // Parameter vector
{
   int errorCount= 0;               // Number of errors detected

   try {
     errorCount += dirty();
   } catch(std::exception X) {
     errorCount++;
     debugf("Exception what(%s)\n", X.what());
   }

   if( errorCount == 0 )
     debugf("NO errors\n");
   else if( errorCount == 1 )
     debugf("1 error\n");
   else
     debugf("%d errors\n", errorCount);

   return (errorCount != 0);
}

