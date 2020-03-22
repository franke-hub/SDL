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
//       testPoorly.cpp
//
// Purpose-
//       Test improper usage.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <string>

#include "Main.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#if defined(HCDM) && !defined(SCDM)
  #define SCDM
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Poorly
//
// Purpose-
//       This Poorly defined object contains std::string objects, references
//       and pointers. The compiler isn't smart enough to detect that, when
//       this object is constructed from a temporary object, it's bad to have
//       that temporary go out of scope.
//
//----------------------------------------------------------------------------
class Poorly {
public:
string                 obj;         // A string object
const string&          ref;         // A string reference
const string*          ptr;         // A string pointer

   ~Poorly( void ) {}               // Destructor
   Poorly(                          // Constructor
     const string&     reference)   // With reference
:  obj(reference)                   // Copy
,  ref(reference)                   // Copy address
,  ptr(&reference)                  // Copy address
{
   #ifdef HCDM
     cout << __LINE__ << " Poorly(" << this << ")::"
          << "Poorly('" << &reference << "," << reference << "')"
          << "\n.obj(" << &obj << ") '" <<  obj << "'"
          << "\n.ref(" << &ref << ") '" <<  ref << "'"
          << "\n.ptr(" <<  ptr << ") '" << *ptr << "'"
          << "\n" ;
     debug(cout);
   #endif
}

void
   check(                           // Check this object
     const char*       s)           // Against this string
{
   #ifdef HCDM
     cout << __LINE__ << " Poorly(" << this << ")::check(" << s << ")\n";
   #endif

   verify( obj == ref );
   verify( obj == *ptr );

   verify( strcmp(obj.c_str(), s) == 0 );
   verify( strcmp(ref.c_str(), s) == 0 );
   verify( strcmp(ptr->c_str(), s) == 0 );
}

ostream&
   debug(                           // Debug this object
     ostream&          out) const   // On this stream
{
   #ifdef HCDM
     cout << __LINE__ << " Poorly(" << this << ")::debug()\n";
   #endif

   #if defined(SCDM)
     out
         <<   "{" << "Poorly@" << this
         << "\n," << "obj{" << (void*)obj .c_str() << ",'" <<  obj << "'}"
         << "\n," << "ref{" << (void*)ref .c_str() << ",'" <<  ref << "'}"
         << "\n," << "ptr{" << (void*)ptr->c_str() << ",'" << *ptr << "'}"
         << "}\n" ;
   #endif
   return print(out);
}

ostream&
   print(                           // Display this object
     ostream&          out) const   // On this stream
{
   #ifdef HCDM
     cout << __LINE__ << " Poorly(" << this << ")::print()\n";
   #endif

   #if defined(SCDM)
     return out
         <<   "{" << "{" << &obj << ",'" <<  obj << "'}"
         << "\n," << "{" << &ref << ",'" <<  ref << "'}"
         << "\n," << "{" <<  ptr << ",'" << *ptr << "'}"
         << "}\n\n"
         ;
   #else
      return out;
   #endif
}
}; // class Poorly

ostream& operator<<(
           ostream&      stream,
           const Poorly& Poorly) {return Poorly.print(stream);}

ostream& operator+=(
           ostream&      stream,
           const Poorly& Poorly) {return Poorly.debug(stream);}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testObject
//
// Purpose-
//       Return Poorly object.
//
//----------------------------------------------------------------------------
static Poorly*                      // A Poorly object
   testObject(                      // Return Poorly object
     const char*       inpstr)      // From this string
{
   if( inpstr == NULL )
   {
     #ifdef HCDM
       cout << __LINE__ << " testPoorly() testObject(NULL)\n";
     #endif

     string temp= "poorly string";

     // We are about to build a poorly defined object.  Since the string
     // parameter is constructed in this routine's autodata (stack) area,
     // and the Poorly object refers to that autodata string, the references
     // and pointers in the Poorly object are not valid. Since the reference is
     // hidden inside the object, the error is not detected by the compiler.
     // Logically, this is the same as:
     //   string& bad(void) {string foo="foo"; return foo;}
     // in which the error is detected.
     return new Poorly(temp);
   }

   #ifdef HCDM
     cout << __LINE__ << " testPoorly() testObject( " << inpstr << ")\n";
   #endif

   // The resultant Poorly [defined object] works but the associated string
   // object is never deallocated. This is bad too.
   string* temp= new string(inpstr);
   return new Poorly(*temp);

   // If we try to get around the problem with:
   //   string* temp= new string(inpstr);
   //   Poorly* poorly= new Poorly(*temp);
   //   delete temp;
   // we wind up referring and pointing to the temp string that we just
   // deleted. Now we're not pointing into the stack, we're pointing at
   // free memory. Bad, bad, bad.
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testB0
//
// Purpose-
//       Demonstrate object usage error
//
//----------------------------------------------------------------------------
static void
   testB0( void )                   // Demo object usage error
{
   Logger::log("testPoorly::testB0()\n");

   cout << "\nGOOD string\n";
   Poorly* poorly= testObject("poorly string");
   trash();
   poorly->check("poorly string");
   poorly->debug(cout);
   delete poorly;

   //-------------------------------------------------------------------------
   // This can result in an exception that cannot be handled.
   cout << "\nBAD string\n";
   poorly= NULL;
   try {
     poorly= testObject(NULL);
     trash();
     poorly->check("poorly string");
     poorly->debug(cout);
   } catch(...) {
     cout << "EXCEPTION CAUGHT" << endl;
   }

   delete poorly;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testPoorly
//
// Purpose-
//       Test <string>
//
//----------------------------------------------------------------------------
extern void
   testPoorly( void )
{
   wtlc(LevelStd, "testPoorly()\n");

   // Demonstrate bug
   if( bugLevel > 0 )
     testB0();
}

