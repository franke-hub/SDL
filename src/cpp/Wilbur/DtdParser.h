//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DtdParser.h
//
// Purpose-
//       DTD (Document Type Definition) Parser.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef DTDPARSER_H_INCLUDED
#define DTDPARSER_H_INCLUDED

#include <map>
#include <string>

#ifndef TEXTBUFFER_H_INCLUDED
#include "TextBuffer.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DataSource;

//----------------------------------------------------------------------------
//
// Class-
//       DtdParser
//
// Purpose-
//       DTD Parser.
//
//----------------------------------------------------------------------------
class DtdParser                     // DTD Parser
{
//----------------------------------------------------------------------------
// DtdParser::Attributes
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, std::string>
                       NvPair;      // Name/Value pair
typedef NvPair::iterator
                       NvIter;      // Name/Value pair iterator

NvPair                 attlistMap;  // The ATTLIST map
NvPair                 elementMap;  // The ELEMENT map
NvPair                 entityMap;   // The ENTITY map
NvPair                 publicMap;   // The PUBLIC map
TextBuffer             errorReport; // Error report buffer

//----------------------------------------------------------------------------
// DtdParser::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DtdParser( void );              // Destructor
   DtdParser( void );               // Default constructor

//----------------------------------------------------------------------------
// DtdParser::Accessors
//----------------------------------------------------------------------------
public:
inline std::string                  // The associated ATTLIST entry
   getATTLIST(                      // Get ATTLIST entry
     std::string       entry)       // For this entry
{
   NvIter              i= attlistMap.find(entry);
   if( i == attlistMap.end() )
     return "";

   return i->second;
}

inline std::string                  // The associated ELEMENT entry
   getELEMENT(                      // Get ELEMENT entry
     std::string       entry)       // For this entry
{
   NvIter              i= elementMap.find(entry);
   if( i == elementMap.end() )
     return "";

   return i->second;
}

inline std::string                  // The associated ENTITY entry
   getENTITY(                       // Get ENTITY entry
     std::string       entry)       // For this entry
{
   NvIter              i= entityMap.find(entry);
   if( i == entityMap.end() )
     return "";

   return i->second;
}

inline void
   setENTITY(                       // Set ENTITY entry
     std::string       entry,       // For this entry
     std::string       value)       // To this value
{
   // Duplicates silently ignored
   if( entityMap.find(entry) == entityMap.end() )
     entityMap[entry]= value;
}

inline std::string                  // The associated PUBLIC entity
   getPUBLIC(                       // Get PUBLIC entity
     std::string       entry)       // For this entry
{
   NvIter i= publicMap.find(entry);
   if( i == publicMap.end() )
     return "";

   return i->second;
}

inline void
   setPUBLIC(                       // Set PUBLIC entry
     std::string       entry,       // For this entry
     std::string       value)       // To this value
{
   // Duplicates silently ignored
   if( publicMap.find(entry) == publicMap.end() )
     publicMap[entry]= value;
}

inline const char*                  // The error report
   getREPORT( void )                // Get error report
{
   if( errorReport.size() == 0 )
     return errorText(0);

   return errorReport.toChar();
}

//----------------------------------------------------------------------------
// DtdParser::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Write debugging messages

static const char*                  // Error string
   errorText(                       // Convert into error string
     int                code);      // This parse return code

int                                 // Return code (0 OK)
   parse(                           // Parse
     DataSource&       data);       // Using this DataSource

void
   reset( void );                   // Reset the DtdParser

protected:
int                                 // Return code (0 OK)
   includeSOURCE(                   // Handle include
     DataSource&       file,        // Using this DataSource
     DataSource&       data);       // Using this data

int                                 // Return code (0 OK)
   includeSTMT(                     // Handle statement
     DataSource&       data,        // Using this DataSource
     char*             stmt);       // Using this string

protected:
void
   error(                           // Append error message
     int               code,        // Error code
     DataSource&       data);       // For this DataSource
}; // class DtdParser

#endif // DTDPARSER_H_INCLUDED
