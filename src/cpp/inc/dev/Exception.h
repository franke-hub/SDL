//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Exception.h
//
// Purpose-
//       HTTP exceptions
//
// Last change date-
//       2022/02/15
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_EXCEPTION_H_INCLUDED
#define _PUB_HTTP_EXCEPTION_H_INCLUDED

#include <stdexcept>                // For std::exception, ...
#include <string>                   // For std::string

namespace pub::http {
//----------------------------------------------------------------------------
//
// Class-
//       http::exception
//         http::io_exception
//           http::io_eof
//           http::io_error
//         http::stream_error
//
// Purpose-
//       Define HTTP exceptions
//
//----------------------------------------------------------------------------
class exception : public std::exception {
public:
std::string            mess;

virtual
   ~exception() = default;
   exception(std::string mess)
:  std::exception(), mess(mess) {}

   exception(const exception&) = default;
   exception(exception&&) = default;
exception& operator=(const exception&) = default;
exception& operator=(exception&&) = default;

virtual const char*
   what() const throw()
{  return mess.c_str(); }
}; // class exception

class io_exception : public exception {
public: using exception::exception;
}; // class io_exception

class io_eof : public io_exception {
public: using io_exception::io_exception;
}; // class io_eof

class io_error : public io_exception {
public: using io_exception::io_exception;
}; // class io_error

class stream_error : public exception {
public: using exception::exception;
}; // class stream_error
}  // namespace pub::http
#endif // _PUB__HTTPEXCEPTION_H_INCLUDED
