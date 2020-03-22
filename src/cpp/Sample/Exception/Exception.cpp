//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Exception.cpp
//
// Purpose-
//       Test std::exception
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       This demonstrates how NOT to use Exceptions.
//       Or, why catch(exception&) rather than catch(exception) is always
//       preferred, and usually necessary.
//
//----------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <exception>

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Debugging display of an exception.
//
//----------------------------------------------------------------------------
void
   debug(                           // Debugging
     const exception   x)           // Exception
{
   // This down-casts the exception
   printf("debug(%p) what(%s)\n", &x, x.what());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debugRef
//
// Purpose-
//       Debugging display of an exception.
//
//----------------------------------------------------------------------------
void
   debugRef(                        // Debugging
     const exception&  x)           // Exception reference
{
   printf("debugRef(%p) what(%s)\n", &x, x.what());
}

//----------------------------------------------------------------------------
//
// Class-
//       MyBadException
//
// Purpose-
//       Define a really bad exception class.
//
//----------------------------------------------------------------------------
class MyBadException : public std::bad_exception
{
private:
   unsigned            constructCount;
   unsigned            destructCount;

public:
virtual
   ~MyBadException( void ) throw();
   MyBadException( void ) throw();

   MyBadException(const exception& source); // Copy constructor
MyBadException&
   operator=(const MyBadException& source); // Assignment operator
MyBadException&
   operator=(const exception& source); // Assignment operator

virtual const char*
   what() const throw();
}; // class MyBadException

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const MyBadException
                       t_t_t_thats_all_folks;

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
//
// Purpose-
//       Indicate that something bad happened.
//
//----------------------------------------------------------------------------
static void
   shouldNotOccur(                  // Something bad happened
     int               line,        // At this line number
     const char*       mess = NULL) // Associated message
{
   if( mess == NULL )
     mess= "(See source code)";

   fprintf(stdout, "%4d Exception: %s\n", line, mess);
   throw t_t_t_thats_all_folks;
}

//----------------------------------------------------------------------------
//
// Method-
//       MyBadException::~MyBadException
//
// Purpose-
//       Destructor (nop).
//
//----------------------------------------------------------------------------
   MyBadException::~MyBadException( void ) throw()
{
   printf("%4d MyBadException(%p)::~MyBadException()\n", __LINE__, this);

   destructCount++;
   printf("cCount(%d) dCount(%d)\n", constructCount, destructCount);
}
   MyBadException::MyBadException( void ) throw()
:  std::bad_exception()
,  constructCount(0)
,  destructCount(0)
{
   printf("%4d MyBadException(%p)::MyBadException()\n", __LINE__, this);

   constructCount++;
   printf("cCount(%d) dCount(%d)\n", constructCount, destructCount);
}

   MyBadException::MyBadException(const exception& source) // Copy constructor
:  std::bad_exception()
,  constructCount(0)
,  destructCount(0)
{
   printf("%4d MyBadException(%p)::MyBadException(const exception& %p)\n",
          __LINE__, this, &source);

   constructCount++;
   printf("cCount(%d) dCount(%d)\n", constructCount, destructCount);
}

//----------------------------------------------------------------------------
//
// Method-
//       MyBadException::operator=
//
// Purpose-
//       Assignment operators.
//
//----------------------------------------------------------------------------
MyBadException&
   MyBadException::operator=(const exception& source) // Assignment operator
{
   printf("%4d MyBadException(%p)::operator=(const exception %p)\n",
          __LINE__, this, &source);

   if( dynamic_cast<const MyBadException*>(&source) != NULL )
   {
     printf("isa(MyBadException)\n");
     constructCount= dynamic_cast<const MyBadException*>(&source)->constructCount;
     destructCount= dynamic_cast<const MyBadException*>(&source)->destructCount;
   }
   printf("cCount(%d) dCount(%d)\n", constructCount, destructCount);
   return *this;
}

MyBadException&
   MyBadException::operator=(const MyBadException& source) // Assignment operator
{
   printf("%4d MyBadException(%p)::operator=(const MyBadException %p)\n",
          __LINE__, this, &source);

   constructCount= source.constructCount;
   destructCount= source.destructCount;
   printf("cCount(%d) dCount(%d)\n", constructCount, destructCount);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       MyBadException::what
//
// Purpose-
//       Describe the exception
//
//----------------------------------------------------------------------------
const char*
   MyBadException::what() const throw()
{
// printf("%4d MyBadException(%p)::what()\n", __LINE__, this);
   return "Proper what message";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test00
//
// Purpose-
//       Catch(exception x)
//
//----------------------------------------------------------------------------
int                                 // Error count
   test00( void )                   // Testcase
{
   int                 errorCount= 0;
   int                 error;

   printf("\n");
   printf("test00 catch(exception)\n");
   error= FALSE;
   try {
     shouldNotOccur(__LINE__, "Normal exception");
   } catch(exception x) {           // This down-casts the exception
     error= TRUE;
     printf("test00 Caught: exception(%p) what(%s)\n", &x, x.what());
   }

   if( !error )
   {
     errorCount++;
     printf("test00 Expected exception not caught\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test01
//
// Purpose-
//       Catch(exception& x)
//
//----------------------------------------------------------------------------
int                                 // Error count
   test01( void )                   // Testcase
{
   int                 errorCount= 0;
   int                 error;

   printf("\n");
   printf("test01 catch(exception&)\n");
   error= FALSE;
   try {
     shouldNotOccur(__LINE__, "Normal exception");
   } catch(exception& x) {
     error= TRUE;
     printf("test01 Caught: exception&(%p) what(%s)\n", &x, x.what());
   }

   if( !error )
   {
     errorCount++;
     printf("test01 Expected exception not caught\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test02
//
// Purpose-
//       Display exception
//
//----------------------------------------------------------------------------
int                                 // Error count
   test02( void )                   // Testcase
{
   int                 errorCount= 0;

   printf("\n");
   printf("test02 catch(exception&)\n");

   debug(t_t_t_thats_all_folks);
   debugRef(t_t_t_thats_all_folks);

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test99
//
// Purpose-
//       Throw an exception.
//
//----------------------------------------------------------------------------
int                                 // Error count
   test99( void )                   // Testcase
{
   int                 errorCount= 0;

   printf("\n");
   printf("test99\n");
   shouldNotOccur(__LINE__, "test99 called");

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   int                 errorCount= 0;

   std::set_terminate (__gnu_cxx::__verbose_terminate_handler);
   try {
     errorCount += test00();
     errorCount += test01();
     errorCount += test02();
//// errorCount += test99();
   } catch(exception& x) {
     errorCount++;
     printf("******** Unexpected exception ********\n");
     printf("******** %s\n", x.what());
   } catch(...) {
     errorCount++;
     printf("******** Unexpected exception ********\n");
   }

   printf("\n");
   printf("Error count: %d\n", errorCount);
   if( errorCount != 0 )
     errorCount= 1;

   return errorCount;
}

