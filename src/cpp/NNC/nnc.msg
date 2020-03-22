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
//       nnc.msg
//
// Purpose-
//       Message Table (US English version)
//
// Last change date-
//       2007/01/01
//
// Debugging tests (normally commented out)-
//       12345, "Bringup test message"
//              "\nFC($FC)" "\nFL($FL)" "\nFN($FN)"
//              "\n00($00)" "\n01($01)" "\n02($02)" "\n99($99)",
         12345, "Another bringup test message",
//       aaaaa, "invalid number"
//            , "invalid number"
//       12345  "missing comma after number"
//       12345, "missing comma after string" 12345
//       12345, "incomplete message
//       12345, "invalid \p sequence"
//       12345, , "missing quote"

//----------------------------------------------------------------------------
// Informational messages
   0, "I: Neural Net Compiler (NNC) Version 1.0",
   1, "I: Compiling '$01'",
   2, "I: Pass[2]",
   3, "I: Pass[3]",
   4, "I: Pass[4]",

  90, "I: $01 warnings detected",
  91, "I: $01 errors detected",
  92, "I: $01 serious errors detected",
  93, "I: $01 terminating errors detected",

//----------------------------------------------------------------------------
// Messages used to debug the message handler
 997, "I: $FN '$01'",
 998, "I: $FL '$01'",
 999, "I: $FC '$01'",

1000, "W: Sample warning message",
2000, "E: Sample error message",
3000, "S: Sample serious message",

//----------------------------------------------------------------------------
// Input file errors
3001, "S: Cannot open input file '$01'",
3002, "S: Cannot read file '$01' - Storage shortage",
3003, "S: I/O error reading '$01'",

//----------------------------------------------------------------------------
// Output file errors
3011, "S: Cannot open output file '$01'",

//----------------------------------------------------------------------------
// PGS file errors
3091, "S: PGSCOLD initialization failure, RC=$01\n"
      ">>Control_file($02)\n"
      ">>Framesize($03) Fileno($04) Partno($05)",
3092, "S: $FL I/O error (PGS fault)",


//----------------------------------------------------------------------------
// NEURON statement
3101, "S: $FL Neuron statement has no name clause",
3102, "S: $FL Multiple $01 clauses",
3103, "S: $FL Invalid type '$01'",

//----------------------------------------------------------------------------
// FANIN statement
3201, "S: $FL Missing $01 clause",
3202, "S: $FL Multiple $01 clauses",
3203, "S: $FL Neuron '$01' not defined",

//----------------------------------------------------------------------------
// BEGIN statement
3801, "S: $01 Begin group incomplete",
3802, "S: $FL Multiple $01 clauses",
3803, "S: $FL End without begin",
3810, "S: $FL INFO specified without FILE clause",
3811, "S: $FL Reuse of FILE but INFO changed",

//----------------------------------------------------------------------------
// DO statement
3821, "S: $01 Do iterator zero",

//----------------------------------------------------------------------------
// ENTRY statement
3880, "S: No entry statement",
3881, "S: $FL Duplicate entry statement",

//----------------------------------------------------------------------------
// General syntax and length errors
3900, "S: $FL Statement too long",
3901, "S: $FL String '$01' too long",
3902, "S: $FL Filename '$01' too long",
3903, "S: $FL Word '$01' too long",
3904, "S: $FL Symbol '$01' too long",
3905, "S: $FL Fileinfo '$01' too long",
3998, "S: $FL Unterminated string",
3999, "S: $FL Syntax error",

//----------------------------------------------------------------------------
// Invalid symbols
6000, "S: $FL Symbol name missing",
6001, "S: $FL Name '$01' not acceptable as symbol",
6002, "S: $FL Symbol '$01' not found",
6003, "S: $FL Symbol '$01' is a duplicate",
6004, "S: $FL Symbol table full",

//----------------------------------------------------------------------------
// Invalid statement sequence
6101, "S: $FL No prior begin statement",
6102, "S: $FL No prior begin file[] clause",
6103, "S: $FL No prior neuron statement",

//----------------------------------------------------------------------------
// Invalid dimensions
6201, "S: $FL Too many dimensions",
6202, "S: $FL Too many elements",
6203, "S: $FL Invalid value for dimension",
6204, "S: $FL Dimension out of range",
6205, "S: $FL Mismatched neuron dimensionality",

//----------------------------------------------------------------------------
// Fixed space shortage
7001, "S: $FL Expression too complex",
7002, "S: $FL File space full",
7003, "S: Symbol($01) has too many qualifiers",

//----------------------------------------------------------------------------
// Storage shortage errors
8001, "S: Storage shortage - cannot initialize",
8002, "S: $FL Storage shortage - statement skipped",
8099, "X: $FL Storage shortage - cannot continue",

//----------------------------------------------------------------------------
// Compiler errors
9900, "X: $FL Compiler error: $01.$02", // File, identifier
9901, "X: $FL Compiler error: $01 not coded yet",
9902, "X: $FL Symbol($01) value differs in pass II from pass I",

//----------------------------------------------------------------------------
// Message errors
9998, "X: $FL Compiler error, message'$01' index'$02' undefined",
9999, "X: $FL Compiler error, message'$00' undefined",

