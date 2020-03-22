//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dirty.cpp
//
// Purpose-
//       Quick and dirty test.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <map>
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include "Command.h"

//----------------------------------------------------------------------------
// class Debug_Command_h: Used to debug Command.h
//
// Problem: used const char* as key. The keys are not identical unless they
//   are the SAME pointer. Pointing to a key that has the same content does
//   not make it identical. (Fixed in Command.h by adding a Comparator.)
//----------------------------------------------------------------------------
#define DCH Debug_Command_h
class DCH {
public:
typedef std::string    Name;
typedef const char*    Value;
typedef std::map<Name, Value> Map;
static Map             map;

static std::string
   map_string( void )
{
   std::string result;

   for(const auto& pair: map) {
     if( result.length() != 0 ) result += ", ";
     result += pair.first;
     result += ":";
     // result += std::to_string(pair.second); // When Value is an int
     result += pair.second;
   }

   return result;
}

// The actual debugging code
static int run(int argc, const char** argv) {
   std::cout << "Map: " << map_string() << std::endl;
   if( argc > 1 ) {
     // const char* one = "one";
     // const char* val = argv[1];
     std::string one = "one";
     std::string val = argv[1];
     printf("one(%s) val(%s)\n", one.c_str(), val.c_str());
     printf("one: %s= map[%s]\n", map[one], one.c_str());
     printf("one: %p= map[%s]\n", map[one], one.c_str());
     printf("val: %p= map[%s]\n", map[val], val.c_str());
     // printf("%d= strcmp(%s,%s)\n", strcmp(one,val), one, val);
   }

   return 0;
}
}; // class DCH

DCH::Map               DCH::map;    // The DCH::map instance

namespace Private {
static class STATIC_CONSTRUCTOR {
public:
   STATIC_CONSTRUCTOR( void )
{  DCH::map[std::string("one")] = "111";
   DCH::map[std::string("two")] = "222";
   DCH::map[std::string("three")] = "333";
}
} instance; // class STATIC_CONSTRUCTOR
} // namespace Private

INSTALL_COMMAND_NAME("command_h", command_h, DCH::run)
#undef DCH

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
   int               result= 0;     // Function resultant

   try {
     if( argc > 1 ) {
        argc -= 1;
        argv = &argv[1];
     } else {
        argc = 1;
        static const char* temp[] = {"list"};
        argv = temp;
     }

     result= Command::command()->run(argc, argv);
   } catch(const char* x) {
     fprintf(stderr, "catch(const char*(%s))\n", x);
     result= 2;
   } catch(std::exception& x) {
     fprintf(stderr, "catch(exception.what(%s))\n", x.what());
     result= 2;
   } catch(...) {
     fprintf(stderr, "catch(...)\n");
     result= 2;
   }

   if( result != 0 )
     printf("result(%d)\n", result);

   return result;
}
