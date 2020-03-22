//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       LineParser.h
//
// Purpose-
//       Parse an input control line.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
#ifndef LINEPARSER_H_INCLUDED
#define LINEPARSER_H_INCLUDED

#include <stdint.h>
#include <string.h>

#include <com/Parser.h>

//----------------------------------------------------------------------------
//
// Class-
//       LineParser
//
// Purpose-
//       Parse an input control line.
//
//----------------------------------------------------------------------------
class LineParser                    // LineParser
{
//----------------------------------------------------------------------------
// LineParser::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~LineParser( void )              // Destructor
{
}

inline
   LineParser( void )               // Default constructor
:  string(NULL)
{
}

inline
   LineParser(                      // String constructor
     const char*       string)      // Source string
:  string(string)
{
}

//----------------------------------------------------------------------------
// LineParser::Methods
//----------------------------------------------------------------------------
public:
inline const char*                  // -> String (after " parm=")
   find(                            // Locate parameter
     const char*       parm)        // Parameter (form " parm=")
{
   const char* result= strstr(string, parm);
   if( result != NULL )
     result += strlen(parm);

   return result;
}

inline uint32_t                     // Parameter value
   getDec32(                        // Get decimal value
     const char*       parm)        // Parameter (form " parm=")
{
   int32_t             result= 0;

   const char* C= find(parm);
   if( C != NULL )
   {
     Parser parser(C);
     result= parser.toDec32();
   }

   return result;
}

inline uint64_t                     // Parameter value
   getDec64(                        // Get decimal value
     const char*       parm)        // Parameter (form " parm=")
{
   int64_t             result= 0;

   const char* C= find(parm);
   if( C != NULL )
   {
     Parser parser(C);
     result= parser.toDec64();
   }

   return result;
}

inline uint32_t                     // Parameter value
   getHex32(                        // Get hexidecimal value
     const char*       parm)        // Parameter (form " parm=")
{
   int32_t             result= 0;

   const char* C= find(parm);
   if( C != NULL )
   {
     Parser parser(C);
     result= parser.toHex32();
   }

   return result;
}


inline bool                         // TRUE iff parameter present
   isPresent(                       // Is parameter present?
     const char*       parm)        // Parameter (form " parm=")
{
   return (strstr(string, parm) != NULL);
}

inline void
   set(                             // Set the current parse string
     const char*       parm)        // The current parse string
{
   string= parm;
}

//----------------------------------------------------------------------------
// LineParser::Attributes
//----------------------------------------------------------------------------
protected:
   const char*         string;      // The parse string
}; // class LineParser

#endif // LINEPARSER_H_INCLUDED
