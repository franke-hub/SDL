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
//       http/Options.h
//
// Purpose-
//       HTTP Options.
//
// Last change date-
//       2022/02/20
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_OPTIONS_H_INCLUDED
#define _PUB_HTTP_OPTIONS_H_INCLUDED

#include <string>                   // For std::string
#include <strings.h>                // For strcasecmp

#include <pub/List.h>               // For pub::List

namespace pub::http {
//----------------------------------------------------------------------------
//
// Class-
//       Options
//
// Purpose-
//       HTTP request/response options
//
//----------------------------------------------------------------------------
class Options {                     // Http request/response options
//----------------------------------------------------------------------------
// Options::Static constants
//----------------------------------------------------------------------------
public:
// HTTP Option keys
typedef const char CC;              // (Shorthand)
static constexpr CC*   HTTP_HEADER_HOST= "HOST";
static constexpr CC*   HTTP_HEADER_LENGTH= "Content-Length";
static constexpr CC*   HTTP_HEADER_TYPE= "Content-Type";

// HTTP Constants
static constexpr CC*   HTTP_METHOD_CONNECT= "CONNECT";
static constexpr CC*   HTTP_METHOD_DELETE= "DELETE";
static constexpr CC*   HTTP_METHOD_GET= "GET";
static constexpr CC*   HTTP_METHOD_HEAD= "HEAD";
static constexpr CC*   HTTP_METHOD_OPTIONS= "OPTIONS";
static constexpr CC*   HTTP_METHOD_POST= "POST";
static constexpr CC*   HTTP_METHOD_PUT= "PUT";
static constexpr CC*   HTTP_METHOD_TRACE= "TRACE";

static constexpr CC*   HTTP_OPT_PROTOCOL= "PROTOCOL"; // Protocol specifier
static constexpr CC*   HTTP_PROTOCOL_H0= "HTTP/1.0";  // HTTP/1.0 (clear text)
static constexpr CC*   HTTP_PROTOCOL_H1= "HTTP/1.1";  // HTTP/1.1 (clear text)
static constexpr CC*   HTTP_PROTOCOL_H2= "HTTP/2";    // HTTP/2   (clear text)
static constexpr CC*   HTTP_PROTOCOL_S0= "HTTPS/1.0"; // HTTPS/1.0 (encrypted)
static constexpr CC*   HTTP_PROTOCOL_S1= "HTTPS/1.1"; // HTTPS/1.1 (encrypted)
static constexpr CC*   HTTP_PROTOCOL_S2= "HTTPS/2";   // HTTPS/2   (encrypted)

//----------------------------------------------------------------------------
// Options::Typedefs
//----------------------------------------------------------------------------
typedef std::string    string;

//----------------------------------------------------------------------------
// Options::Option
//----------------------------------------------------------------------------
class Option : public pub::List<Option>::Link { // Option descriptor
public:
const string           first;       // The Option name
string                 second;      // The Option value

   ~Option( void ) = default;       // Destructor
   Option(                          // Constructor
     const string&     name,        // The Option name
     const string&     value);      // The Option value
}; // class Option

//----------------------------------------------------------------------------
// Options::const_iterator
//----------------------------------------------------------------------------
class const_iterator : public std::forward_iterator_tag {
Option*                item= nullptr; // The current Option
public:
   ~const_iterator( void ) = default; // Destructor
   const_iterator( void ) = default;  // Default (end) constructor
   const_iterator(const const_iterator&); // Copy constructor
   const_iterator(const pub::List<Option>&); // Constructor

bool operator==(const const_iterator&) const; // Operator ==
bool operator!=(const const_iterator&) const; // Operator !=

const Option& operator*( void ) const;  // Dereference operator
const Option* operator->( void ) const; // Dereference operator
const_iterator& operator++( void ); // Prefix operator
const_iterator  operator++( int );  // Postfix operator

void swap(const_iterator&);         // (Swappable)
}; // class const_iterator

//----------------------------------------------------------------------------
// Options::iterator
//----------------------------------------------------------------------------
class iterator : public std::forward_iterator_tag {
Option*                item= nullptr; // The current Option
public:
   ~iterator( void ) = default;     // Destructor
   iterator( void ) = default;      // Default (end) constructor
   iterator(const const_iterator&); // Copy constructor
   iterator(const pub::List<Option>&); // Constructor

bool operator==(const const_iterator&) const; // Operator ==
bool operator!=(const const_iterator&) const; // Operator !=

Option& operator*( void ) const;    // Dereference operator
Option* operator->( void ) const;   // Dereference operator
iterator& operator++( void );       // Prefix operator
iterator  operator++( int );        // Postfix operator

void swap(iterator&);               // (Swappable)
}; // class iterator

//----------------------------------------------------------------------------
// Options::Attributes
//----------------------------------------------------------------------------
protected:
pub::List<Option>      opts;        // The Option list

//----------------------------------------------------------------------------
// Options::Destructor/Constructor
//----------------------------------------------------------------------------
public:
   ~Options( void );                // Destructor
   Options( void );                 // Default constructor
   Options(const Options& from);    // Copy constructor
   Options(Options&& from);         // Move constructor

//----------------------------------------------------------------------------
// Options::Operators
//----------------------------------------------------------------------------
Options&                            // (Always *this)
   operator=(const Options& opts);  // Assignment copy operator

Options&                            // (Always *this)
   operator=(Options&& opts);       // Assignment move operator

//----------------------------------------------------------------------------
// Options::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Options::Methods
//----------------------------------------------------------------------------
void
   append(const Options& opts);     // Append Options

const_iterator                      // The begin iterator
   begin( void ) const;             // Get begin iterator

const_iterator                      // The end iterator
   end( void ) const;               // Get end iterator

bool                                // (Indicates Option replaced)
   insert(                          // Insert
     const string&     name,        // Option name
     const string&     value);      // Option value

const char*                         // The Option value
   locate(                          // Get Option value
     const char*       name) const; // For this Option name

const char*                         // The Option value
   locate(                          // Get Option value
     const string&     name) const  // For this Option name
{  return locate(name.c_str()); }

const string                        // The Option value
   locate(                          // Get Option value
     const string&     name,        // For this Option name
     const string&     value) const; // And this default value

bool                                // (Indicates Option removed)
   remove(                          // Remove
     const char*       name);       // This Option name

bool                                // (Indicates Option removed)
   remove(                          // Remove
     const string&     name)        // This Option name
{  return remove(name.c_str()); }

void
   reset( void );                   // Reset Options

string&                             // The (settable) Option value
   operator[](                      // Get Option value
     const char*       name);       // For this Option name

string&                             // The (settable) Option value
   operator[](                      // Get Option value
     const string&     name)        // For this Option name
{  return operator[](name.c_str()); }
}; // class Options
}  // namespace pub::http
#endif // _PUB_HTTP_OPTIONS_H_INCLUDED