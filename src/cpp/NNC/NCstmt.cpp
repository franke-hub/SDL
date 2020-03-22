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
//       NCstmt.cpp
//
// Purpose-
//       Neural Net Compiler: Statement parser
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include "hcdm.h"
#include "NC_com.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCSTMT  " // Source file, for debugging

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"                 // Diagnostic macros

//----------------------------------------------------------------------------
//
// Subroutine-
//       CTL_DEBUG
//
// Purpose-
//       Handle a debug statement.
//
//----------------------------------------------------------------------------
static void
   ctl_debug(                       // Process DEBUG statement
     const char*       inpbuf)      // Current statement buffer
{
#if 1
   if( inpbuf[6] != ';' )
     NCmess(NC_msg::ID_SynGeneric, 0);
#endif

#if 0
   trace_mode(TRACEMODE_INTENSIVE);
#endif

#if 0
   if( NC_COM.pass == NC_com::Pass1 )
     return;
#endif

#if 0
   if( NC_COM.pass == NC_com::Pass2 )
     VPSABORT("DEBUG CHECKSTOP");
#endif

#if 0
   HCDM= TRUE;
   errorf("#debug activated\n");
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       CTL_DEFINE
//
// Purpose-
//       Handle a define statement.
//
//----------------------------------------------------------------------------
static void
   ctl_define(                      // Process define statement
     const char*       inpbuf)      // Current statement buffer
{
   char                symName[NC_com::WORK_SIZE];

   NC_FixedSymbol      symbol;      // Symbol
   NC_opFixed*         value;       // Initial value

   int                 stmtix;      // Current statement index

   //-------------------------------------------------------------------------
   // Extract the symbol name
   //-------------------------------------------------------------------------
   stmtix= 8;                       // Offset of first character
   stmtix= ncnextw(inpbuf, stmtix, symName); // Extract next word
   if( symName[0] == '\0' )         // If nothing extracted
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }

   //-------------------------------------------------------------------------
   // Extract the symbol value
   //-------------------------------------------------------------------------
   value= NC_opFixed::generate(inpbuf, stmtix); // Extract the value
   if( value == NULL )              // If extraction fault
     return;
   value->operate();                // Compute the immediate value
   symbol.value= value->getFixed();
   delete value;

   //-------------------------------------------------------------------------
   // Load the symbol table entry
   //-------------------------------------------------------------------------
   NC_COM.ist.insert(NC_sym::TypeFixed, NC_COM.begroup, symName, &symbol);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       CTL_INCLUDE
//
// Purpose-
//       Handle an include statement.
//
//----------------------------------------------------------------------------
static int
   ctl_include(                     // Process include statement
     const char*       inpbuf,      // Current buffer
     char*             outbuf)      // Include file name
{
   char                d;           // Closing delimiter
   const char*         s;           // Pointer to string

   int                 i;           // General index variable

   //-------------------------------------------------------------------------
   // Extract the include name
   //-------------------------------------------------------------------------
   s= &inpbuf[9];                   // Address the '\"' character
   if( *s == '\"' )
     d= '\"';
   else if( *s == '<' )
     d= '>';
   else                             // If invalid statement
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return(FALSE);
   }

   s++;                             // Skip the '\"' character
   i=0;                             // Initialize
   while(i<255)                     // Copy the filename
   {
     if( *s == d )
       break;
     if( *s == '\0' )
     {
       NCmess(NC_msg::ID_SynGeneric, 0);
       return(FALSE);
     }

     outbuf[i++]= *s;
     s++;
   }

   if( i == 0 )
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return(FALSE);
   }
   if( i >= 255 )
   {
     outbuf[255]= '\0';
     NCmess(NC_msg::ID_SynStringTooLong, 1, outbuf);
     return(FALSE);
   }

   outbuf[i]= '\0';
   return(TRUE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       CONTROL
//
// Purpose-
//       Handle a control statement.
//
//----------------------------------------------------------------------------
static void
   control(                         // Process control statements
     const char*       inpbuf)      // Current buffer
{
   char                w[256];      // Work area
   int                 rc;          // Called routine return code

   if( memcmp(inpbuf, "#debug", 6) == 0 ) // If this is a debug statement
   {
     ctl_debug(inpbuf);             // Debugging hook
   }

   else if( memcmp(inpbuf, "#define ", 8) == 0 ) // If this is a define
                                    // statement
   {
     ctl_define(inpbuf);            // Add the element to the table
   }

   else if( memcmp(inpbuf, "#include ", 9) == 0 ) // If this is an include
                                    // statement
   {
     rc= ctl_include(inpbuf, w);    // Determine include filename
     if( rc )                       // If the filename is valid
       ncincl(&w[0]);               // Include the file
   }

   else                             // If invalid control statement
   {
     NCmess(NC_msg::ID_SynGeneric, 0);
     return;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NCSTMT
//
// Purpose-
//       Parse a statement.
//
//----------------------------------------------------------------------------
extern void
   ncstmt(                          // Parse a statement
     const char*       inpbuf)      // Current buffer
{
   int                 stmtix;      // Statement index

   //-------------------------------------------------------------------------
   // Trace
   //-------------------------------------------------------------------------
   if( HCDM )
     tracef("%s\n", inpbuf);

   if( __BRINGUP__ )
     printf("%s\n", inpbuf);

   //-------------------------------------------------------------------------
   // NULL statement
   //-------------------------------------------------------------------------
   if( inpbuf[0] == '\0' )          // If NULL statement
     return;                        // Statement complete

   //-------------------------------------------------------------------------
   // CONTROL statement
   //-------------------------------------------------------------------------
   if( inpbuf[0] == '#' )           // If control statement
   {
     control(inpbuf);               // Process control statement
     return;                        // and return
   }

   //-------------------------------------------------------------------------
   // FUNCTION statement
   //-------------------------------------------------------------------------
   stmtix= ncnextw(inpbuf, 0, NC_COM.word0);
                                    // Extract statement word
   makeUPPER(NC_COM.word0);         // Convert to upper case

   if( strcmp(NC_COM.word0, "FANIN") == 0 ) // If FANIN statement
     nc__fan(inpbuf, stmtix);       // Process FANIN

   else if( strcmp(NC_COM.word0, "NEURON") == 0 ) // If NEURON statement
     nc__neu(inpbuf, stmtix);       // Process NEURON

   else if( strcmp(NC_COM.word0, "CONSTANT") == 0 ) // If CONSTANT statement
     nc__con(inpbuf, stmtix);       // Process CONSTANT

   else if( strcmp(NC_COM.word0, "END") == 0 ) // If END statement
     nc__end(inpbuf, stmtix);       // Process END

   else if( strcmp(NC_COM.word0, "BEGIN") == 0 ) // If BEGIN statement
     nc__beg(inpbuf, stmtix);       // Process BEGIN

   else if( strcmp(NC_COM.word0, "DO") == 0 ) // If DO statement
     nc__do(inpbuf, stmtix);        // Process DO

   else if( strcmp(NC_COM.word0, "ENTRY") == 0 ) // If ENTRY statement
     nc__ent(inpbuf, stmtix);       // Process ENTRY

   else                             // If invalid statement
     NCmess(NC_msg::ID_SynGeneric, 0); // Write error message
}

