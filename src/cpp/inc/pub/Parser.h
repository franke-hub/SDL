//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Parser.h
//
// Purpose-
//       Simple parameter file parser.
//
// Last change date-
//       2020/12/30
//
// Notes-
//       An parameter file consists of section name declarations:
//         [section name]                       {; comment to end of line}
//
//       and parameter name/value declarations:
//         parameter name {= {parameter value}} {; comment to end of line}
//
//       (The default parameter value is "")
//       Leading and trailing blanks are removed from both the parameter
//       name and value. Quotations allow special characters (including
//       leading and trailing blanks) in the name or value.
//
//       Spaces are significant in section names.
//
//       Comment lines begin with semicolons.
//
//----------------------------------------------------------------------------
#ifndef _PUB_PARSER_H_INCLUDED
#define _PUB_PARSER_H_INCLUDED

#include <string>                   // For std::string
#include <pub/List.h>               // For pub::List

namespace pub {
//----------------------------------------------------------------------------
//
// Class-
//       pub::Parser
//
// Purpose-
//       External Parameter controls.
//
//----------------------------------------------------------------------------
class Parser {
//----------------------------------------------------------------------------
// Parser::Structures
//----------------------------------------------------------------------------
protected:
struct Parameter : public pub::List<Parameter>::Link {
std::string            parm_name;   // The Parameter's name
std::string            parm_value;  // The Parameter's value
};

struct Section : public pub::List<Section>::Link {
std::string            sect_name;   // This Section's name
pub::List<Parameter>   parm_list;   // The Parameter list
};

//----------------------------------------------------------------------------
// Parser::Attributes
//----------------------------------------------------------------------------
protected:
long                   error_count; // Parse error count
std::string            file_name;   // The file name
List<Section>          sect_list;   // The Section List

//----------------------------------------------------------------------------
// pub::Parser::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   Parser(                          // Default constructor
     const char*       file_name= nullptr) // File name
{  open(file_name); }

   Parser(                          // Default constructor
     std::string       file_name)   // File name string
{  open(file_name); }

   ~Parser( void )                  // Destructor
{  close(); }

void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
// Internal methods
//----------------------------------------------------------------------------
protected:
void error(size_t, const char*);    // Handle error

//----------------------------------------------------------------------------
//
// Method-
//       pub::Parser::open
//
// Purpose-
//       (Re)load the parameter file
//
// Return code values-
//       =0 Successful, no parsing errors.
//       >0 Parse error count (Messages written to stderr)
//       <0 Failure reading file
//
// Implementation notes-
//       A nullptr file_name resets the Parser and always returns 0.
//
//----------------------------------------------------------------------------
public:
long                                // Return code, error_count
   open(                            // Open the parameter file
     const char*       file_name= nullptr); // The parameter file name

long                                // Return code, error_count
   open(                            // Open the parameter file
     std::string       file_name)   // The parameter file name
{  return open(file_name.c_str()); }

//----------------------------------------------------------------------------
//
// Method-
//       pub::Parser::close
//
// Purpose-
//       Reset the Parser.
//
//----------------------------------------------------------------------------
void
   close( void );                   // Close the parameter file

//----------------------------------------------------------------------------
//
// Method-
//       pub::Parser::get_next
//
// Purpose-
//       Get next section or parameter name (for iteration.)
//
// Implementation notes-
//       Users MUST NOT modify (or free) the returned value.
//
//----------------------------------------------------------------------------
const char*                         // The next section name
   get_next(                        // Get the next section name
     const char*       sect) const; // The current section name

const char*                         // The next parameter name
   get_next(                        // Get the next parameter name
     const char*       sect,        // The current section name
     const char*       parm) const; // The current parameter name

//----------------------------------------------------------------------------
//
// Method-
//       pub::Parser::get_value
//
// Purpose-
//       Extract a parameter value.
//
// Implementation notes-
//       Users MUST NOT modify (or free) the returned parameter value.
//
//----------------------------------------------------------------------------
const char*                         // The parameter value
   get_value(                       // Get parameter value
     const char*       sect,        // The section name (nullptr allowed)
     const char*       parm) const; // The parameter name

const char*                         // The parameter value
   get_value(                       // Get parameter value
     std::string       sect,        // The section name
     std::string       parm) const  // The parameter name
{  return get_value(sect.c_str(), parm.c_str()); }
}; // class Parser
}  // namespace PUB
#endif // PUB_PARSER_H_INCLUDED
