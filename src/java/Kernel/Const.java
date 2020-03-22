//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Const.java
//
// Purpose-
//       Constants.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Class-
//       Const
//
// Purpose-
//       Constants.
//
//----------------------------------------------------------------------------
public class Const
{
//----------------------------------------------------------------------------
// Const.attributes
//----------------------------------------------------------------------------
static final boolean   debugging= true; // Debugging attribute
static final boolean   hcdm= false; // Hard-Core Debug Mode
static final boolean   scdm= false; // Soft-Core Debug Mode

// Global constants
static final int       wordsPerPage= 1024; // Number of Words/Page
static final int       rosOrigin= 0x7FFF0000; // Read/only Storage origin

// Instructions with no modifiers
static final Instruction       nopInstruction=  new Instruction();
static final Instruction       backInstruction= new BackInstruction();
static final Instruction       dumpInstruction= new DumpInstruction();
static final Instruction       haltInstruction= new HaltInstruction();
static final Instruction       showInstruction= new ShowInstruction();
static final Instruction       stelInstruction= new StelInstruction();
static final Instruction       stxlInstruction= new StxlInstruction();
} // Class Const

