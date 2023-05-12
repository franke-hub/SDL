//----------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wrapper.h
//
// Purpose-
//       Generic program wrapper.
//
// Last change date-
//       2023/05/04
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_WRAPPER_H_INCLUDED
#define _LIBPUB_WRAPPER_H_INCLUDED

#include <functional>               // For std::function
#include <string>                   // For std::string
#include <getopt.h>                 // For opt* controls

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

//----------------------------------------------------------------------------
// Built-in options (External, in default namespace)
//----------------------------------------------------------------------------
extern int             opt_hcdm;    // Hard Core Debug Mode? [default: false]
extern int             opt_verbose; // Debugging verbosity   [default: 0]

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Debug;                        // _LIBPUB_NAMESPACE::Debug

//----------------------------------------------------------------------------
//
// Class-
//       Wrapper
//
// Purpose-
//       Generic program wrapper
//
//----------------------------------------------------------------------------
/*****************************************************************************
  @class Wrapper Wrapper Wrapper.h "pub/Wrapper.h"

  A generic program wrapper.
*****************************************************************************/
class Wrapper                       // Generic program wrapper
{  protected:
     typedef const char                                 CC;
     typedef std::function<int(int, char*[])>           main_t;
     typedef std::function<int(std::string, CC*)>       parm_t;
     typedef std::function<void()>                      void_t;

     // The (possibly extended) option list
     char*             OSTR= nullptr; // The getopt_long parameter: optstring
     struct option*    OPTS= nullptr; // The getopt_long parameter: longopts
     size_t            OPNO= 0;     // The number of options + 1

     // User callback functions
     void_t            info_f;      // Information exit handler
     main_t            init_f;      // Initialization handler
     main_t            main_f;      // The generic program
     parm_t            parm_f;      // The parameter handler
     void_t            term_f;      // Termination handler

     int               opt_index= 0; // The current option index

   public:
     std::string       program;     // The program name

     ~Wrapper();                    // Destructor
     Wrapper(option* O= nullptr, const char* P= nullptr); // Constructor

     void debug(const char* info="") const; // Debug this object

     //-----------------------------------------------------------------------
     // Set function handlers
     /************************************************************************
       @brief Lambda function: Called if any parameter error occurs.

       The lambda function should write its parameter descriptions to stderr,
       e.g. `fprintf(stderr, "  --parm\tSample parameter description\n");`
       then adding any positional parameter descriptions, e.g.
       `fprintf(stderr, "\nPositional parameters:\n");`
       `fprintf(stderr, "integer Number of iterations\n"); // Example`
       `fprintf(stderr, "integer Number of threads\n"); // Example`

        @remark Positional parameters are handled in the on_main lambda.
     ************************************************************************/
     void on_info(void_t f)         // (Default does nothing)
     { info_f= f; }

     /************************************************************************
       @brief Define lambda function: Extends initialization.

       The lambda function
       @returns 0 Normal, the on_main lambda is invoked.
                Any other return code becomes the run method's return code,
                skipping the on_main lambda but invoking the on_term lambda.

       Called after the on_parm lambda and before the on_main lambda.
     ************************************************************************/
     void on_init(main_t f)         // (Default does nothing, returns 0)
     { init_f= f; }

     /************************************************************************
       @brief Lambda function: Defines the code to be run.

       The lambda function
       @returns The the run method's return code.
     ************************************************************************/
     void on_main(main_t f)         // (Default does nothing, returns 0)
     { main_f= f; }

     /************************************************************************
       @brief Lambda function: Handle user-defined parameters.

       The lambda function
       @returns 0 Normal. The on_init lambda is invoked next.
                Any other return code indicates a terminating error.
                After all parameters are examined, the on_info method
                is invoked and the program exits with a return code of 1.

       If a parameter error is detected, an error message should be written
       describing the failure condition.
     ************************************************************************/
     void on_parm(parm_t f)         // (Default does nothing, returns 0)
     { parm_f= f; }

     /************************************************************************
       @brief Lambda function: Extends termination.
     ************************************************************************/
     void on_term(void_t f)         // (Default does nothing)
     { term_f= f; }

     /************************************************************************
       @brief The program driver

       @param argc The argument count.
       @param argv The argument array.

       @return The main program return code.

       @code
         // Parameter analysis, exits if --help specified or error detected
         parm(argc, argv)
           (parm_f(parameter, argument) called for each user argument)
           (If --help specified or an error was detected) {
             (fprintf(stderr, list of built-in parameters)
             info_f()
             exit('error detected' ? 1 : 0)
           }

         // Initialization and operation
         (local initialization: create trace table and debugging file)
         rc= init_f(argc, argv)
         if( rc == 0 )
           rc= main_f(argc, argv)   // Program operation

         // Termination
         term_f()
         (local termination)
         return rc
       @endcode
     ************************************************************************/
     int run(int argc, char* argv[]);

     //-----------------------------------------------------------------------
     // Utilities
     /************************************************************************
       @brief Convert string to integer.
       @param V The string to convert. Non-numeric characters are invalid.
       @returns The integer representation of `V`
       @retval errno\
                0: Normal, no error detected.\
           EINVAL: Invalid character or empty string.\
           ERANGE: Value outside of integer range.
     ************************************************************************/
     static int atoi(const char* V); // Convert V to integer

     /************************************************************************
       @brief Convert string to long integer.
       @param V The string to convert. Non-numeric characters are invalid.
       @returns The integer representation of `V`
       @retval errno\
                0: Normal, no error detected.\
           EINVAL: Invalid character or empty string.\
           ERANGE: Value outside of integer range.
     ************************************************************************/
     static long atol(const char* V); // Convert V to long integer

     /************************************************************************
       @brief Create debugging output file
       @param name The debugging file name, defaulted to Debug default
       @param mode The debugging file mode, defaulted to Debug default
       @param head The debugging Debug::Heading flags, defaulted to none
       @returns The Debug object, set as Debug default

       Implementation note: Debug mode set to Debug::MODE_INTENSIVE if the
       external variable opt_hcdm is non-zero.
     ************************************************************************/
     static Debug* init_debug(
       const char*     name= nullptr,
       const char*     mode= nullptr,
       int             head= 0);

     /************************************************************************
       @brief Create memory mapped trace file.
       @param name The memory mapped file name
       @param size The memory mapped file length
       @returns The initialized memory mapped trace file address, which may
         be slightly different than Trace::table.
     ************************************************************************/
     static void* init_trace(const char* name, int size);

     /************************************************************************
       @brief Convert parameter to integer.
       @param N The parameter name.
       @param V The string to convert. Non-numeric characters are invalid.
       @return The integer representation of `V`

       Invokes atoi(V). If an error occurs an error message is written to
       stderr and and the program exits, invoking on_info().
     ************************************************************************/
     int ptoi(const char* V, const char* N= nullptr);

     /************************************************************************
       Write a completion status message.
     ************************************************************************/
     static void report_errors(int error_count); // Report error count

     /************************************************************************
       @brief Terminate debugging
       @param debug The Debug object returned by init_debug
     ************************************************************************/
     static void term_debug(Debug* debug);

     /************************************************************************
       @brief Terminate memory mapped trace
       @param table The Trace object returned by init_trace.
       @param size  The Trace object length passed to init_trace.
     ************************************************************************/
     static void term_trace(void* table, int size);

     //-----------------------------------------------------------------------
     // Generic program sections
     //-----------------------------------------------------------------------
     [[noreturn]]
     void info( void ) const;       // Handle parameter error(s) (public)

   protected:
     void parm(int argc, char* argv[]); // Handle parameters
     int  init(int argc, char* argv[]); // Handle initialization
     int  main(int argc, char* argv[]); // Main function
     void term( void );             // Termination cleanup

     //-----------------------------------------------------------------------
     // Internal methods
     //-----------------------------------------------------------------------
     ssize_t option1(int);          // Get switch option index
     option* option2(const char*);  // Get long option descriptor
}; // class Wrapper
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_WRAPPER_H_INCLUDED
