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
//       Command.h
//
// Purpose-
//       Command dictionary class.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef COMMAND_H_INCLUDED
#define COMMAND_H_INCLUDED

#include <map>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>

#include "Exception.h"

//----------------------------------------------------------------------------
// Insure FALSE, TRUE, and USE_COMPARATOR are defined
//----------------------------------------------------------------------------
#define USE_COMPARATOR TRUE

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Command
//
// Purpose-
//       Command descriptor.
//
//----------------------------------------------------------------------------
class Command {                     // Command descriptor
//----------------------------------------------------------------------------
// Command::Attributes
//----------------------------------------------------------------------------
public:                             // Enumerations and typedefs
typedef int (*Function)(int argc, const char* argv[]);
#if( USE_COMPARATOR )
   struct LessComparator {
     bool operator()(const char* L, const char* R) const
          { return strcmp(L, R) < 0; }
   }; // struct LessComparator

   typedef std::map<const char*, Function, LessComparator> Map;
#else
   typedef std::map<std::string, Function> Map;
#endif

protected:
Map                    dict;        // The Command dictionary
static Command*        _command;    // A built-in command instance

//----------------------------------------------------------------------------
// Command::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Command( void ) {}             // Destructor
inline
   Command( void ) {}              // Constructor

//----------------------------------------------------------------------------
// Command::Accessor methods
//----------------------------------------------------------------------------
public:
static inline Command*              // The built-in command instance
   command( void )                  // Get built-in command instance
{  if( _command == nullptr ) _command= new Command();
   return _command;
}

inline const Function               // The function
   get(                             // Get dictionary entry
     const char*       name)        // The entry name
{
// printf("Command(%p).get(%s)\n", this, name);
   Function result= nullptr;
   #if( USE_COMPARATOR )
     if( dict.find(name) != dict.end() )
       result = dict[name];
   #else
     std::string key(name);
     if( dict.find(key) != dict.end() )
       result = dict[key];
   #endif
   if( result == nullptr ) {
     std::cerr << "Valid commands: " << list() << std::endl;
     throw KeyError(name);
   }

   return result;
}

inline void
   set(                             // Set dictionary for
     const char*       name,        // This entry name to
     const Function    func)        // This associated Function
{
// printf("Command(%p).set(%s)\n", this, name);

   #if( USE_COMPARATOR )
     dict[name] = func;
   #else
     dict[std::string(name)] = func;
   #endif
}

//----------------------------------------------------------------------------
// Command::Methods
//----------------------------------------------------------------------------
public:
inline std::string
   list( void ) const
{  std::string result;
// printf("Command(%p).list()\n", this);

#undef _USE_COMMAND_FUNCTION_DISPLAY // #define or #undef here (only)
#ifdef _USE_COMMAND_FUNCTION_DISPLAY
   char buffer[64];
   for(const auto& pair: dict) {
     if( result.length() != 0 ) result += ", ";
     result += pair.first;
     result += ":";
     std::sprintf(buffer, "%p", pair.second);
     result += buffer;
   }
#else
   for(const auto& pair: dict) {
     if( result.length() != 0 ) result += ", ";
     result += pair.first;
   }
#endif
#undef _USE_COMMAND_FUNCTION_DISPLAY

   return result;
}

inline int                          // The function's or summary return code
   main(                            // Run the command, or all the commands
     int               argc,        // With this argument count
     const char*       argv[],      // And these arguments
     bool              verbose= true) // Default, use verbose mode
{
   int               result= 0;     // Function resultant

   try {
     if( argc <= 1 ) {              // If the no specific command specified
       bool need_blank= false;
       for(const auto& pair: dict) {
         const char* name= pair.first;
         const char* temp[]= {name, nullptr};
         if( need_blank ) std::cout << std::endl;
         std::cout << "Running: " << name << std::endl;
         need_blank= true;
         int rc= pair.second(1, temp);
         if( rc != 0 )
           std::cout << "rc(" << rc << ")" << std::endl;

         result= std::max(result, rc);
       }
     } else {                       // If a single command name
       argc -= 1;
       argv = &argv[1];
       result= this->run(argc, argv);
     }
   } catch(Exception& x) {
     std::cerr << "catch(" << x.get_class_name() << ").what(" << x.what()
               << ")) " << x.get_class_what() << std::endl;
     result= 2;
   } catch(std::exception& x) {
     std::cerr << "catch(std::exception.what(" << x.what() << "))" << std::endl;
     result= 2;
   } catch(const char* x) {
     std::cerr << "catch(const char*(" << x << "))" << std::endl;
     result= 2;
   } catch(...) {
     std::cerr << "catch(...)" << std::endl;
     result= 2;
   }

   if( verbose )
     std::cout << "result(" << result << ")" << std::endl;
   return result;
}

inline int                          // The function's return code
   run(                             // Run the command
     int               argc,        // With this argument count
     const char*       argv[])      // And these arguments
{
// printf("Command(%p).run(%s)\n", this, argv[0]);
   return get(argv[0])(argc, argv);
}
}; // class Command

//----------------------------------------------------------------------------
// Define Common::_command, once and only once
//----------------------------------------------------------------------------
#ifndef _DEFINE_COMMAND_COMMON
#define _DEFINE_COMMAND_COMMON 1
#endif

#if _DEFINE_COMMAND_COMMON
Command*               Command::_command= nullptr; // The built-in Command instance
#endif

#undef  _DEFINE_COMMAND_COMMON
#define _DEFINE_COMMAND_COMMON 0

//----------------------------------------------------------------------------
//
// Macro-
//       INSTALL_COMMAND
//
// Purpose-
//       Create Command::command installer
//
//----------------------------------------------------------------------------
#define DEFAULT_COMMAND_AT (*::Command::command())
#define INSTALL_COMMAND_AT DEFAULT_COMMAND_AT

#define INSTALL_COMMAND_NAME(name, space, function) \
   namespace Command_built_in::space { \
     static class STATIC_CONSTRUCTOR { \
       public: inline STATIC_CONSTRUCTOR( void ) \
       { INSTALL_COMMAND_AT.set(name, ::function); } \
     } instance; \
   } // namespace Command_built_in::space

#define INSTALL_COMMAND(name) INSTALL_COMMAND_NAME(#name, name, name)

#endif // COMMAND_H_INCLUDED
