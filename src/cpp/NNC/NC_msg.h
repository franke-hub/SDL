//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NC_msg.h
//
// Purpose-
//       (NC) Neural Net Compiler: Message control
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_MSG_H_INCLUDED
#define NC_MSG_H_INCLUDED

#ifndef MESSAGE_H_INCLUDED
#include <Message.h>
#endif

//----------------------------------------------------------------------------
//
// Class-
//       NC_msg
//
// Purpose-
//       Define the message table.
//
//----------------------------------------------------------------------------
class NC_msg : public Message       // Message table
{
//----------------------------------------------------------------------------
// NC_msg::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum MessageId                      // Message identifiers
{
   ID_VersionId=                  0,// NNC Version number
   ID_Pass1=                      1,// Compiling $01
   ID_Pass2=                      2,// Pass[2]
   ID_Pass3=                      3,// Pass[3]
   ID_Pass4=                      4,// Pass[4]

   ID_WarnNo=                    90,// $01 Warnings
   ID_ErrsNo=                    91,// $01 Errors
   ID_SevsNo=                    92,// $01 Severe errors
   ID_TermNo=                    93,// $01 Terminating errors

   ID_IOROpen=                 3001,// Cannot open $01
   ID_IORStorage=              3002,// Cannot read $01 -- Storage shortage
   ID_IORFault=                3003,// I/O error reading $01

   ID_IOWOpen=                 3011,// Cannot open $01

   ID_VPSOpen=                 3091,// VPSCOLD initialization failure, ...
   ID_VPSFault=                3092,// I/O error (VPS fault)

   ID_NeuNoName=               3101,// Neuron stmt, no name clause
   ID_NeuDupClause=            3102,// Neuron stmt, duplicate $01 clause
   ID_NeuInvalid=              3103,// Neuron stmt, invalid type

   ID_FanNoRead=               3201,// Fanin stmt, no read clause
   ID_FanDupClause=            3202,// Fanin stmt, duplicate $01 clause
   ID_FanTarget=               3203,// Neuron $01 not defined

   ID_EndMissing=              3801,// Begin group incomplete
   ID_BegDupClause=            3802,// Begin stmt, duplicate $01 clause
   ID_EndWithoutBeg=           3803,// End not in same file as begin
   ID_InfWithoutFile=          3810,// INFO specified without FILE clause
   ID_InfChanged=              3811,// Reuse of FILE but INFO changed

   ID_ForInfinite=             3821,// Do group iterator zero

   ID_EntMissing=              3880,// No entry statement
   ID_EntDuplicate=            3881,// $FL Duplicate entry statement

   ID_SynStmtTooLong=          3900,// Statement too long
   ID_SynStringTooLong=        3901,// String $01 too long
   ID_SynFileNameTooLong=      3902,// Filename $01 too long
   ID_SynWordTooLong=          3903,// Word $01 too long
   ID_SynSymbolTooLong=        3904,// Symbol $01 too long
   ID_SynInfoTooLong=          3905,// Fileinfo $01 too long
   ID_SynStringEnd=            3998,// Unterminated string
   ID_SynGeneric=              3999,// Syntax error

   ID_SymNameMissing=          6000,// Symbol name missing
   ID_SymName=                 6001,// Name $01 not acceptable as symbol
   ID_SymNotFound=             6002,// Symbol $01 not found
   ID_SymDuplicate=            6003,// Symbol $01 is a duplicate
   ID_SymStorage=              6004,// Symbol table full

   ID_SeqNoBegin=              6101,// No prior begin statement
   ID_SeqNoBeginFile=          6102,// No prior begin file[] clause
   ID_SeqNoNeuron=             6103,// No prior neuron statement

   ID_DimTooManyDim=           6201,// Too many dimensions
   ID_DimTooManyElements=      6202,// Too many elements
   ID_DimValue=                6203,// Invalid value for dimension
   ID_DimRange=                6204,// Dimension out of range
   ID_DimMismatch=             6205,// Mismatched neuron dimensionality
   ID_DimMismatchEntry=        6205,// Mismatched neuron dimensionality
   ID_DimMismatchSource=       6205,// Mismatched neuron dimensionality
   ID_DimMismatchTarget=       6205,// Mismatched neuron dimensionality

   ID_FixComplex=              7001,// Expression too complex
   ID_FixFileSpace=            7002,// File space full
   ID_FixQualifierCount=       7003,// Too many qualifiers

   ID_StgInitial=              8001,// Storage shortage - cannot initialize
   ID_StgSkipStmt=             8002,// Storage shortage - statement skipped
   ID_StgFatal=                8099,// Storage shortage - cannot continue

   ID_BugFileLine=             9900,// Compiler error: $01.$02
   ID_BugNotCoded=             9901,// Compiler error: $01 not coded yet
   ID_BugPassDiff=             9902,// Symbol($01) value differs in pass II
}; // enum MessageId

enum MessageLevel                   // Message level
{
   ML_Info,                         // Informational
   ML_Warn,                         // Warning
   ML_Error,                        // Error
   ML_Severe,                       // Severe error
   ML_Terminating                   // Terminating
}; // enum MessageId

//----------------------------------------------------------------------------
// NC_msg::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   ~NC_msg( void ) {}               // Destructor
   NC_msg( void );                  // Constructor

//----------------------------------------------------------------------------
// NC_msg::Methods
//----------------------------------------------------------------------------
public:
void
   internalError(                   // Write internal error message
     const char*       fileName,    // File name
     int               lineNumber); // Line number

void
   message(                         // Write error message
     MessageId         msgno,       // Message number
     int               argc,        // Argument count
                       ...);        // Argument array

//----------------------------------------------------------------------------
// NC_msg::Attributes
//----------------------------------------------------------------------------
public:
   MessageLevel        stopLevel;   // Fatal message level
   MessageLevel        highLevel;   // Highest message level
   MessageLevel        showLevel;   // Display message level

   unsigned            infoCount;   // The number of info messages
   unsigned            warnCount;   // The number of warn messages
   unsigned            errsCount;   // The number of error messages
   unsigned            sevsCount;   // The number of severe messages
   unsigned            termCount;   // The number of terminating messages
}; // class NC_msg

//----------------------------------------------------------------------------
//
// Class-
//       Pass1MessageCallback
//
// Purpose-
//       Define a callback routine for use during pass I.
//
//----------------------------------------------------------------------------
class Pass1MessageCallback : public MessageCallback
{
//----------------------------------------------------------------------------
// Pass1MessageCallback::Constructors/destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Pass1MessageCallback( void );   // Destructor
   Pass1MessageCallback( void );    // Constructor

//----------------------------------------------------------------------------
// Pass1MessageCallback::Methods
//----------------------------------------------------------------------------
public:
virtual void
   set( void );                     // Set name components
}; // class Pass1MessageCallback

//----------------------------------------------------------------------------
//
// Class-
//       Pass2MessageCallback
//
// Purpose-
//       Define a callback routine for use during other passes.
//
//----------------------------------------------------------------------------
class Pass2MessageCallback : public MessageCallback
{
//----------------------------------------------------------------------------
// Pass2MessageCallback::Constructors/destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Pass2MessageCallback( void );   // Destructor
   Pass2MessageCallback( void );    // Constructor

//----------------------------------------------------------------------------
// Pass2MessageCallback::Methods
//----------------------------------------------------------------------------
public:
virtual void
   set( void );                     // Set name components
}; // class Pass2MessageCallback

#endif // NC_MSG_H_INCLUDED
