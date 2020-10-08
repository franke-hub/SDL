//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       visible.cpp
//
// Purpose-
//       Demonstrate Argument Dependent Lookup, A.K.A ADL or Koenig lookup.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <iostream>                 // For std::cout, std::endl
#include <string>                   // For std::string
#include <utility>                  // For std::move

#define USE_COMPILE_ERROR1 false    // Demonstrate compile error #1?
#define USE_COMPILE_ERROR2 false    // Demonstrate compile error #2?

using std::cout;
using std::endl;

// tic int             default_argc= 2;
static const char*     default_argv[]= { "a", "parm" };

namespace ADL {
#if( USE_COMPILE_ERROR1 )
        typedef std::string  Name;  // Typedef is not enough
#else
        class Name : public std::string {
        public:
          Name(const char* name) : std::string(name) {}
        }; // class Name
#endif

    std::string foo(Name& name);
    std::string foo(Name& name)
    {   std::string out; out= "foo(" + name + ")";
//      return std::move(out);
        return out;
    }

    std::string bar(std::string& name);
    std::string bar(std::string& name)
    {   std::string out; out= "bar(" + name + ")";
//      return std::move(out);
        return out;
    }
} // namespace ADL

int main(int, char**) {
//  int          argc= default_argc;
    const char** argv= default_argv;

    ADL::Name name("jolly good name");
    cout << foo(name) << endl;      // Argument requires
    cout << bar(name) << endl;

#if( USE_COMPILE_ERROR2 )
    cout << foo(argv[1]) << endl;
    cout << bar(argv[1]) << endl;
#else                               // Parameter argv needs USE_COMPILE_ERROR2
    (void)argv;
#endif

    return 0;
}
