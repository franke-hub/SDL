//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Options.cpp
//
// Purpose-
//       Implement http/Options.h
//
// Last change date-
//       2022/03/06
//
//----------------------------------------------------------------------------
#include <algorithm>                // For std::swap
#include <memory>                   // For std::shared_ptr
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string

#include <assert.h>                 // For assert()
#include <errno.h>                  // For errno
#include <stdio.h>                  // For fprintf()
#include <stdint.h>                 // For integer types
#include <arpa/inet.h>              // For inet_ntop()

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception

#include "pub/http/Options.h"       // For pub::http::Options, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// enum (NOT USED)
// {  HCDM= false                      // Hard Core Debug Mode?
// ,  VERBOSE= 0                       // Verbosity, higher is more verbose
// }; // enum

static const Options::const_iterator
                       static_end;  // A static Options::end()

//----------------------------------------------------------------------------
//
// Method-
//       Options::Options
//       Options::~Options
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Options::Options( void )
:  opts()
{  }

   Options::Options(const Options& from)
:  opts()
{  append(from); }

   Options::Options(Options&& from)
:  opts()
{
   if( from.opts.get_head() ) {
     opts.insert(nullptr, from.opts.get_head(), from.opts.get_tail());
     from.reset();
   }
}

   Options::~Options( void )
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       Options::operator=
//
// Purpose-
//       Assignment operators
//
//----------------------------------------------------------------------------
Options&                            // (Always *this)
   Options::operator=(              // Assignment copy operator
     const Options&    from)        // (Copy from)
{  opts.reset(); append(from); return *this; }

Options&                            // (Always *this)
   Options::operator=(              // Assignment move operator
     Options&&         from)        // (Move from)
{  opts.reset(); append(from); from.reset(); return *this; }

//----------------------------------------------------------------------------
//
// Method-
//       Options::operator[]
//
// Purpose-
//       Indexing operators
//
//----------------------------------------------------------------------------
string&                             // The (settable) Option value
   Options::operator[](             // Get Option value
     const char*       name)        // For this Option name
{
   for(Option* opt= opts.get_head(); opt; opt= opt->get_next()) {
     if( strcasecmp(name, opt->first.c_str()) == 0 )
       return opt->second;
   }

   opts.fifo(new Option(name, ""));
   return opts.get_tail()->second;
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Options::debug(const char* info) const  // Debugging display
{  debugf("Options(%p)::debug(%s)\n", this, info);

   int index= 0;
   for(const_iterator it= begin(); it != end(); ++it) {
     debugf("[%2d] %s(%s)\n", index++, it->first.c_str(), it->second.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::append
//
// Purpose-
//       Append options
//
//----------------------------------------------------------------------------
void
   Options::append(                 // Append Options
     const Options&    opts)        // (The source Options)
{
   for(const_iterator it= opts.begin(); it != opts.end(); ++it)
     insert(it->first, it->second);
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::begin
//       Options::end
//
// Purpose-
//       Return begin iterator
//       Return end iterator
//
//----------------------------------------------------------------------------
Options::const_iterator             // The begin iterator
   Options::begin( void ) const     // Get begin iterator
{  return const_iterator(opts); }

Options::const_iterator             // The end iterator
   Options::end( void ) const       // Get end iterator
{  return const_iterator(); }

//----------------------------------------------------------------------------
//
// Method-
//       Options::insert
//       Options::locate
//       Options::remove
//       Options::reset
//
// Purpose-
//       Insert Option (at end) (Note: removes Option if present)
//       Locate Option
//       Remove Option
//       Reset Options
//
//----------------------------------------------------------------------------
bool                                // (Indicates Option replaced)
   Options::insert(                 // Insert
     const string&     name,        // Option name
     const string&     value)       // Option value
{  bool result= remove(name);
   opts.fifo(new Option(name, value));
   return result;
}

const char*                         // The Option value
   Options::locate(                 // Get Option value
     const char*       name) const  // For this Option name
{
   for(Option* opt= opts.get_head(); opt; opt= opt->get_next()) {
     if( strcasecmp(name, opt->first.c_str()) == 0 )
       return opt->second.c_str();
   }

   return nullptr;
}

const string                        // The Option value
   Options::locate(                 // Get Option value
     const string&     name,        // For this Option name
     const string&     value) const // And this default value
{
   const char* item= name.c_str();
#if 1  // (TEST ++ITERATOR)
   for(const_iterator it= begin(); it != static_end; ++it) {
     if( strcasecmp(item, it->first.c_str()) == 0 )
       return it->second.c_str();
   }
#elif 1 // (TEST ITERATOR++)
   for(const_iterator it= begin(); it != end(); it++) {
     if( strcasecmp(item, it->first.c_str()) == 0 )
       return it->second.c_str();
   }
#else
   for(Option* opt= opts.get_head(); opt; opt= opt->get_next()) {
     if( strcasecmp(item, opt->first.c_str()) == 0 )
       return opt->second.c_str();
   }
#endif

   return value;
}

bool                                // (Indicates Option removed)
   Options::remove(                 // Remove Option with
     const char*       name)        // This Option name
{
   for(Option* opt= opts.get_head(); opt; opt= opt->get_next()) {
     if( strcasecmp(name, opt->first.c_str()) == 0 ) {
       opts.remove(opt, opt);
       delete opt;
       return true;
     }
   }

   return false;
}

void
   Options::reset( void )           // Remove all Options
{
   Option* head= opts.remq();
   while( head ) {
     delete head;
     head= opts.remq();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::Option::Option
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Options::Option::Option(
     const string&     name,        // The Option name
     const string&     value)       // The Option value
:  first(name), second(value)
{  if( name == "" )
     throw std::invalid_argument("pub::http::Options::Option name == \"\"");
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::const_iterator::const_iterator
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   Options::const_iterator::const_iterator( // Copy constructor
     const const_iterator& from)
:  item(from.item) {}

   Options::const_iterator::const_iterator( // Constructor
     const List<Option>& from)
:  item(from.get_head()) {}

//----------------------------------------------------------------------------
//
// Method-
//       Options::const_iterator::operator==
//       Options::const_iterator::operator!=
//
// Purpose-
//       Comparison operators
//
//----------------------------------------------------------------------------
bool
   Options::const_iterator::operator==(
     const const_iterator& from) const
{  return item == from.item; }

bool
   Options::const_iterator::operator!=(
     const const_iterator& from) const
{  return item != from.item; }

//----------------------------------------------------------------------------
//
// Method-
//       Options::const_iterator::operator*
//       Options::const_iterator::operator->
//
// Purpose-
//       Dereference operators
//
//----------------------------------------------------------------------------
const Options::Option&
   Options::const_iterator::operator*( void ) const
{  if( item == nullptr )
     throw std::runtime_error("end()::operator*()");

   return *item;
}

const Options::Option*
   Options::const_iterator::operator->( void ) const
{  if( item == nullptr )
     throw std::runtime_error("end()::operator->()");

   return item;
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::const_iterator::operator++(void)
//       Options::const_iterator::operator++(int)
//
// Purpose-
//       Prefix operator++
//       Postfix operator++
//
//----------------------------------------------------------------------------
Options::const_iterator&
   Options::const_iterator::operator++( void ) // Prefix ++operator
{  if( item == nullptr )
     throw std::runtime_error("end()::operator++()");

   item= item->get_next();
   return *this;
}

Options::const_iterator
   Options::const_iterator::operator++( int ) // Postfix operator++
{
   const_iterator temp= *this;
   if( item )
     item= item->get_next();
   return temp;
}

//----------------------------------------------------------------------------
//
// Method-
//       Options::const_iterator::swap
//
// Purpose-
//       Swap iterators
//
//----------------------------------------------------------------------------
void
   Options::const_iterator::swap(const_iterator& that) // Swap iterators
{  std::swap(item, that.item); }
}  // namespace _LIBPUB_NAMESPACE::http
