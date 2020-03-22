//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DateParser.h
//
// Purpose-
//       Date Generator/Parser.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef DATEPARSER_H_INCLUDED
#define DATEPARSER_H_INCLUDED

#include <sys/types.h>              // For time_t
#include <string>                   // For std::string

//----------------------------------------------------------------------------
//
// Class-
//       DateParser
//
// Purpose-
//       Date Generator/Parser.
//
//----------------------------------------------------------------------------
class DateParser                    // Date Generator/Parser
{
//----------------------------------------------------------------------------
// DateParser::Constructor/Destructor
//----------------------------------------------------------------------------
protected:                          // (Only contains static methods)
   ~DateParser( void );             // Destructor
   DateParser( void );              // Default constructor

//----------------------------------------------------------------------------
// DateParser::Methods
//----------------------------------------------------------------------------
public:
static std::string                  // RFC1123 date string
   generate(                        // Generate date string
     time_t            source);     // The source date

static time_t                       // Resultant date
   parse(                           // Parse date string
     const char*       string);     // The date string
}; // class DateParser

#endif  // DATEPARSER_H_INCLUDED
