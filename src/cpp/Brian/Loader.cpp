//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Loader.cpp
//
// Purpose-
//       Attempt to load a command and a service just by existing.
//
// Last change date-
//       2024/10/01
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace debugging

#include "Common.h"                 // For Common::shutdown
#include "Command.h"                // For Command (base class)
#include "Service.h"                // For Service (base class)

#include "Thing.h"                  // For (debugging) Thing

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Global constructors/destructors
// Order:
//   GlobalDestructor::constructor
//   SecondDestructor::constructor
//     :
//   SecondDestructor::destructor
//   GlobalDestructor::destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;
static struct GlobalDestructor {
   GlobalDestructor( void )
{  if( HCDM ) debugf("Loader::GlobalDestructor!\n"); }

   ~GlobalDestructor( void )
{  if( HCDM ) debugf("Loader::GlobalDestructor~\n");

   global_destructor_invoked= true;
}
}  globalDestructor;
}  // Anonymous namespace

namespace {                         // Anonymous namespace
static int             second_destructor_invoked= false;
static struct SecondDestructor {
   SecondDestructor( void )
{  if( HCDM ) debugf("Loader::SecondDestructor!\n"); }

   ~SecondDestructor( void )
{  if( HCDM ) debugf("Loader::SecondDestructor~\n");

   second_destructor_invoked= true;
}
}  secondDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Static class-
//       Command_list
//
// Purpose-
//       List commands or services
//
//----------------------------------------------------------------------------
static class Command_list : public Command {
public:
   Command_list() : Command("list")
{  }

virtual Command::resultant          // Resultant
   work(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   std::string arg1("command");
   if( argc > 1 )
     arg1= argv[1];

   if( arg1 != "service" && arg1 != "services" ) {
     printf("Commands:\n");
     Command::Map_t* map= Command::get_map();

     size_t column= 0;                // Output column
     for(Command::MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
       std::string s= mi->first;
       if( column + s.size() > 78 ) {
         printf("\n");
         column= 0;
       }

       if( column != 0 ) {
         printf(", ");
         column += 2;
       }
       printf("%s", s.c_str());

       column += s.size();
     }
   } else {
     printf("Services:\n");
     Service::Map_t* map= Service::get_map();

     size_t column= 0;                // Output column
     for(Service::MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
       std::string s= mi->first;
       if( column + s.size() > 78 ) {
         printf("\n");
         column= 0;
       }

       if( column != 0 ) {
         printf(", ");
         column += 2;
       }
       printf("%s", s.c_str());

       column += s.size();
     }
   }
   printf("\n");

   return nullptr;
}
} command_list; // static class Command_list

//----------------------------------------------------------------------------
//
// Static class-
//       Command_quit
//
// Purpose-
//       Terminate processing
//
//----------------------------------------------------------------------------
static class Command_quit : public Command {
public:
   Command_quit() : Command("quit")
{  }

virtual Command::resultant          // Resultant
   work(int, char**)                // Handle Command
{  Common::get()->shutdown(); return nullptr; }
} command_quit; // static class Command_quit

//----------------------------------------------------------------------------
//
// Static class-
//       Command_junk
//
// Purpose-
//       Return something
//
//----------------------------------------------------------------------------
static class Command_junk : public Command {
public:
   Command_junk() : Command("junk")
{  }

virtual Command::resultant          // Resultant
   work(int, char**)                // Handle Command
{
   debugf("junk junk junk junk. Yeah!\n");

   return std::make_shared<Thing>(); // See if it auto-magically disappears
}
} command_junk; // static class Command_junk
