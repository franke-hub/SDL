//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
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
//       2022/04/05
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_WRAPPER_H_INCLUDED
#define _LIBPUB_WRAPPER_H_INCLUDED

#include <functional>               // For std::function
#include <string>                   // For std::string
#include <getopt.h>                 // For opt* controls

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
extern int             opt_hcdm;    // Hard Core Debug Mode? [default: false]
extern int             opt_verbose; // Debugging verbosity   [default: 0]

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
     struct option*    OPTS= nullptr; // The getopt_long parameter: longopts
     size_t            OPNO= 0;     // The number of options + 1

     // User callback functions
     void_t            info_f;      // Information exit handler
     main_t            init_f;      // Initialization handler
     main_t            main_f;      // The generic program
     parm_t            parm_f;      // The parameter handler
     void_t            term_f;      // Termination handler

   public:
     static int        opt_hcdm;    // Hard Core Debug Mode?
     static int        opt_verbose; // Verbosity, higher is more verbose

     ~Wrapper();                    // Destructor
     Wrapper(option* O= nullptr);   // Default/option list constructor

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
       @brief Lambda function: Extends initialization.

       The lambda function returns 0 normally and non-zero to terminate.
     ************************************************************************/
     void on_init(main_t f)         // (Default does nothing, returns 0)
     { init_f= f; }

     /************************************************************************
       @brief Lambda function: Defines the code to be run.

       The lambda function returns the main program return code.
     ************************************************************************/
     void on_main(main_t f)         // (Default does nothing, returns 0)
     { main_f= f; }

     /************************************************************************
       @brief Lambda function: Handle user-defined parameters.

       The lambda function returns 0 normally and non-zero on failure.
       An error message should be written describing the failure condition.
     ************************************************************************/
     void on_parm(parm_t f)         // (Default does nothing, returns 0)
     { parm_f= f; }

     /************************************************************************
       @brief Lambda function: Extends termination.
     ************************************************************************/
     void on_term(void_t f)         // (Default does nothing)
     { term_f= f; }

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
     static int  atoi(const char* V); // Convert V to integer

     /************************************************************************
       Write an informational parameter description message, then exit.
     ************************************************************************/
     void info( void ) const;       // Handle parameter error(s)

     /************************************************************************
       @brief Convert parameter to integer.
       @param N The parameter name.
       @param V The string to convert. Non-numeric characters are invalid.
       @return The integer representation of `V`

       Invokes atoi(V). If an error occurs an error message is written to
       stderr and and the program exits, invoking on_info().
     ************************************************************************/
            int  ptoi(const char* V, const char* N= nullptr);

     /************************************************************************
       Write a completion status message.
     ************************************************************************/
     static void report_errors(int error_count); // Report error count

     /************************************************************************
       @brief The program driver

       @param argc The argument count.
       @param argv The argument array.

       @return The main program return code.

       @code
         // Parameter analysis
         int rc= parm(argc, argv)
           (For each user option: rc |= parm_f(name, value))
         if( rc ) {
           (fprintf(stderr, list of built-in parameters)
           info()
         }

         // Initialization
         rc= init_f(argc, argv)
         if( rc )
           std::exit(rc)
         (local initialization)

         // Operation
         rc= main_f(argc, argv)

         // Termination
         term_f()
         (local termination)
         return rc
       @endcode
     ************************************************************************/
     int run(int argc, char* argv[]);

   protected:
     /************************************************************************
       Generic program sections
     ************************************************************************/
     int  parm(int argc, char* argv[]); // Handle parameters
//   void info( void ) const;       // Handle parameter error(s) (public)
     int  init(int argc, char* argv[]); // Handle initialization
     int  main(int argc, char* argv[]); // Main function
     void term( void );             // Termination cleanup
}; // class Wrapper
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_WRAPPER_H_INCLUDED
