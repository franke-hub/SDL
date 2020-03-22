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
//       NC_sys.h
//
// Purpose-
//       (NC) Neural Net Compiler: Global Entry Points
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_SYS_H_INCLUDED
#define NC_SYS_H_INCLUDED

#ifndef NCTYPE_H_INCLUDED
#include "NCtype.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class NC_opFixed;
class NC_opFloat;
class NC_opNeuronAddr;
struct NC_ifd;
struct NC_ofd;

//----------------------------------------------------------------------------
// External routines
//----------------------------------------------------------------------------
extern int
   ncalloc(                         // Allocate group space
     NN::FPO*          ptrfpo,      // -> Resultant address
     NN::FileId        fileno,      // File identifier
     NN::PartId        partno,      // Partition identifier
     long              count);      // Number of elements to allocate

extern int
   ncexpr(                          // Evaluate expression
     const char*       inpbuf,      // Current buffer
     int               inpndx,      // Current buffer index
     NN::Value*        outval);     // Expression value

extern int                          // Resultant buffer index
   ncGenFixed(                      // Generate a fixed expression
     const char*       inpbuf,      // Current buffer
     int               inpndx,      // Current buffer index
     NC_opFixed**      expr);       // Resultant expression

extern void
   ncincl(                          // Include source file
     const char*       filenm);     // Pointer to source file name

extern int
   ncload(                          // Load the next statement
     NC_ifd*           ptrifd);     // Pointer to file descriptor

extern void
   ncstmt(                          // Parse the next statement
     const char*       inpbuf);     // Current buffer

extern void
   ncparm(                          // NEURON parameter control
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

extern int
   ncnextw(                         // Extract the next word
     const char*       inpbuf,      // Current buffer
     int               inpndx,      // Current buffer index
     char*             waccum);     // Word accumulator, 256 bytes

extern int
   ncskipb(                         // Skip over blanks
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern int
   ncstring(                        // Extract a string
     const char*       inpbuf,      // Input statement buffer
     int               inpndx,      // Input statement index
     char*             s,           // Output string buffer
     int               l);          // Output string length

//----------------------------------------------------------------------------
// External routines
//----------------------------------------------------------------------------
extern void
   nc__beg(                         // Statment: BEGIN
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__con(                         // Statment: CONSTANT
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__do(                          // Statment: DO
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__end(                         // Statment: END
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__ent(                         // Statment: ENTRY
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__fan(                         // Statment: FANIN
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

extern void
   nc__neu(                         // Statment: NEURON
     const char*       inpbuf,      // Current buffer
     int               inpndx);     // Current buffer index

#endif // NC_SYS_H_INCLUDED
