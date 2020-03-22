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
//       TemplateException.h
//
// Purpose-
//       OBSOLETE: Now used as template example.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       The original classes written so that the class name is available at
//       compile time. This seems to be more work than is needed, but the
//       resulting code is still useful as a template usage demo. The current
//       Exception class has other simplifications that have not been carried
//       forward into this demonstration class.
//
//----------------------------------------------------------------------------
#ifndef TEMPLATE_EXCEPTION_H_INCLUDED
#define TEMPLATE_EXCEPTION_H_INCLUDED

#include <exception>
#include <typeinfo>
#include <boost/core/demangle.hpp>

//----------------------------------------------------------------------------
//
// Class-
//       BasicTemplateException
//
// Purpose-
//       The Exception base class.
//
//----------------------------------------------------------------------------
template<class T>
class BasicTemplateException : public std::exception {
   // Attributes
   protected:
   std::string         what_str;    // The what message string

   // Class name wrapper
   protected:
   static inline std::string
     def_what(std::string name, std::string desc) {
       std::string result= name;
       result += "(";
       result += desc;
       result += ")";
       return result;
   }

   inline std::string
     def_what(std::string desc) {
       const char* mangled= typeid(*this).name();
       std::string name= boost::core::demangle(mangled);
       return def_what(name, desc);
   }

   public:
   virtual const char*
     what( void ) const throw() {
       return what_str.c_str();
   }

   // Constructors
   public:
   BasicTemplateException()
   : std::exception()
   , what_str(def_what("")) {}

   BasicTemplateException(std::string desc)
   : std::exception()
   , what_str(def_what(desc)) {}
};

class TemplateException : public BasicTemplateException<Exception> {
   using BasicTemplateException<Exception>::BasicTemplateException;

#if 1
   // Constructors (Needed so that the class is TemplateException,
   // and not BasicTemplateException<TemplateException>.)
   public:
   TemplateException()
   : BasicTemplateException()\
   { what_str= def_what(""); }

   TemplateException(std::string desc)
   : BasicTemplateException()
   { what_str= def_what(desc); }
#endif
};

//----------------------------------------------------------------------------
//
// Class-
//       TemplateRuntimeException<T>
//       KeyError
//       NullPointerException
//
// Purpose-
//       The Built-in Exception classes.
//
//----------------------------------------------------------------------------
template<class T>
class TemplateRuntimeException : public TemplateException {
   using TemplateException::TemplateException;

   // Constructors
   public:
   TemplateRuntimeException()
   : TemplateException()
   { what_str= def_what(""); }

   TemplateRuntimeException(std::string desc)
   : TemplateException()
   { what_str= def_what(desc); }

   TemplateRuntimeException(const char* desc)
   : TemplateException()
   { what_str= def_what(std::string(desc)); }
};

class TemplateKeyError : public TemplateRuntimeException<TemplateKeyError> {
   using TemplateRuntimeException<TemplateKeyError>::TemplateRuntimeException;
};

class TemplateNullPointerException : public TemplateRuntimeException<TemplateNullPointerException> {
   using TemplateRuntimeException<TemplateNullPointerException>::TemplateRuntimeException;
};

#endif // TEMPLATE_EXCEPTION_H_INCLUDED
