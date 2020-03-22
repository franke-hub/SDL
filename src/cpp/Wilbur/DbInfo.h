//----------------------------------------------------------------------------
//
//       Copyright (c) 2011 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DbInfo.h
//
// Purpose-
//       The generic database information packet.
//
// Last change date-
//       2011/01/01
//
// Implementation notes-
//       setLink stores type and link in network format.
//       getLink fetches type and link from network format.
//
//----------------------------------------------------------------------------
#ifndef DBINFO_H_INCLUDED
#define DBINFO_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       DbInfo
//
// Purpose-
//       The generic database information packet.
//
// Identification-
//       PROG: src/cpp/Wilbur/DbInfo.cpp
//       THIS: src/cpp/Wilbur/DbInfo.h
//
//----------------------------------------------------------------------------
struct DbInfo {                     // The generic database information packet
enum FC                             // Function codes
{  FC_NOP                           // No-operation
,  FC_INFO                          // Information packet
,  FC_TODO                          // TODO packet
,  FC_COUNT                         // The number of function codes
}; // enum FC

enum FM                             // Function code modifiers (Generic)
{  FM_NOP                           // No-operation
,  FM_COUNT                         // The number of function code modifiers
}; // enum FM

enum FM_INFO                        // Function code modifiers (FC_INFO)
{  INFO_NOP                         // (Undefined)
,  INFO_HAS                         // Contains
,  INFO_ISA                         // Identity
,  INFO_COUNT                       // The number of INFO function code modifiers
}; // enum FM_INFO

enum FM_TODO                        // Function code modifiers (FC_TODO)
{  TODO_NOP                         // (Undefined, generic job)
,  TODO_DEL                         // Delete
,  TODO_TIME                        // Timed Delete
,  TODO_COUNT                       // The number of TODO function code modifiers
}; // enum FM_TODO

enum TYPE                           // Link type
{  TYPE_NOP                         //  0 Unused link
,  TYPE_TEXT                        //  1 Link to DbText
,  TYPE_FILE                        //  2 Link to DbFile
,  TYPE_HTTP                        //  3 Link to DbHttp
,  TYPE_LINK                        //  4 Generic link
,  TYPE_CODE                        //  5 Generic numeric code
,  TYPE_TIME                        //  6 Generic time
,  TYPE_COUNT                       //  9 Number of defined types
}; // enum TYPE

//----------------------------------------------------------------------------
// DbInfo::Attributes
//----------------------------------------------------------------------------
uint16_t               fc;          // Function code
uint16_t               fm;          // Function code modifier
uint16_t               type[6];     // Link type
uint64_t               link[6];     // Associated link

//----------------------------------------------------------------------------
// DbInfo::Constructors
//----------------------------------------------------------------------------
   ~DbInfo( void );                 // Destructor
   DbInfo(                          // Constructor
     int               fc= FC_NOP,  // Function code
     int               fm= FM_NOP); // Function code modifier

//----------------------------------------------------------------------------
// DbInfo::Methods
//----------------------------------------------------------------------------
uint64_t                            // The link value
   getLink(                         // Get link type, value
     int               index,       // For this link index
     int*              type= NULL,  // The link type  (NULL allowed)
     uint64_t*         link= NULL); // The link value (NULL allowed)

int                                 // The link type
   getType(                         // Get link type
     int               index);      // For this link index

void
   setLink(                         // Set link
     int               index,       // The link index
     int               type,        // The link type
     uint64_t          link);       // The link value
}; // struct DbInfo

#endif // DBINFO_H_INCLUDED
