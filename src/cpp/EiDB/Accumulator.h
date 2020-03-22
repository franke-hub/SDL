//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Accumulator.h
//
// Purpose-
//       Describe the Exon/Intron DataBase Accumulator.
//
// Last change date-
//       2003/03/16
//
// Classes-
         class Accumulator;         // Exon/Intron DataBase Accumulator
         class DataAccumulator;     // Exon/Intron DataBase data Accumulator
         class LabelAccumulator;    // Exon/Intron DataBase label Acumulator
//
//----------------------------------------------------------------------------
#ifndef ACCUMULATOR_H_INCLUDED
#define ACCUMULATOR_H_INCLUDED

#include <stdio.h>

//----------------------------------------------------------------------------
//
// Class-
//       Accumulator
//
// Purpose-
//       Describe the Exon/Intron DataBase Accumulator.
//
//----------------------------------------------------------------------------
class Accumulator                   // Exon/Intron DataBase Accumulator
{
//----------------------------------------------------------------------------
// Accumulator::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Accumulator( void );            // Default destructor
   Accumulator( void );             // Default constructor

private:                            // Bitwise copy prohibited
   Accumulator(const Accumulator&);
   Accumulator& operator=(const Accumulator&);

//----------------------------------------------------------------------------
// Accumulator::methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   open(                            // Open the Accumulator
     const char*       fileName);   // Using this file

virtual char*                       // The next accumulator line
   load( void ) = 0;                // Load accumuator line

unsigned                            // The current line number
   getLineNumber( void ) const;     // Get current line number

unsigned                            // The current warning counter
   getWarnings( void ) const;       // Get current warning counter

int                                 // Return code (0 OK)
   close( void );                   // Close the Accumualtor

protected:
char*                               // Resultant
   read(                            // Read a line
     char*             addr,        // Data address
     unsigned          size);       // Data length

//----------------------------------------------------------------------------
// Accumulator::Attributes
//----------------------------------------------------------------------------
protected:
   char*               text;        // Accumulator line
   FILE*               handle;      // File handle
   const char*         fileName;    // File name
   unsigned            lineNumber;  // Line number
   unsigned            warnings;    // Warning counter
}; // class Accumulator

//----------------------------------------------------------------------------
//
// Class-
//       DataAccumulator
//
// Purpose-
//       Describe the Exon/Intron DataBase data Accumulator.
//
//----------------------------------------------------------------------------
class DataAccumulator : public Accumulator // Exon/Intron DataBase data Accumulator
{
//----------------------------------------------------------------------------
// DataAccumulator::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DataAccumulator( void );        // Default destructor
   DataAccumulator( void );         // Default constructor

private:                            // Bitwise copy prohibited
   DataAccumulator(const DataAccumulator&);
   DataAccumulator& operator=(const DataAccumulator&);

//----------------------------------------------------------------------------
// DataAccumulator::methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next accumulator line
   load( void );                    // Load accumuator line

//----------------------------------------------------------------------------
// DataAccumulator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class DataAccumulator

//----------------------------------------------------------------------------
//
// Class-
//       LabelAccumulator
//
// Purpose-
//       Describe the Exon/Intron DataBase label Accumulator.
//
//----------------------------------------------------------------------------
class LabelAccumulator : public Accumulator // Exon/Intron DataBase label Acumulator
{
//----------------------------------------------------------------------------
// LabelAccumulator::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~LabelAccumulator( void );       // Default destructor
   LabelAccumulator( void );        // Default constructor

private:                            // Bitwise copy prohibited
   LabelAccumulator(const LabelAccumulator&);
   LabelAccumulator& operator=(const LabelAccumulator&);

//----------------------------------------------------------------------------
// LabelAccumulator::methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next accumulator line
   load( void );                    // Load accumuator line

//----------------------------------------------------------------------------
// LabelAccumulator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class LabelAccumulator

//----------------------------------------------------------------------------
//
// Class-
//       ExonAccumulator
//
// Purpose-
//       Describe the Exon/Intron DataBase Exon Accumulator.
//       This Accumulator should be used with the default (base) Extractor.
//       It accumulates all Exon data into a single item.
//
//----------------------------------------------------------------------------
class ExonAccumulator : public Accumulator // Exon/Intron DataBase Exon Accumulator
{
//----------------------------------------------------------------------------
// ExonAccumulator::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ExonAccumulator( void );        // Default destructor
   ExonAccumulator( void );         // Default constructor

private:                            // Bitwise copy prohibited
   ExonAccumulator(const ExonAccumulator&);
   ExonAccumulator& operator=(const ExonAccumulator&);

//----------------------------------------------------------------------------
// ExonAccumulator::methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next accumulator line
   load( void );                    // Load accumuator line

//----------------------------------------------------------------------------
// ExonAccumulator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class ExonAccumulator

//----------------------------------------------------------------------------
//
// Class-
//       IntronAccumulator
//
// Purpose-
//       Describe the Exon/Intron DataBase Intron Accumulator.
//       This Accumulator should be used with the default (base) Extractor.
//       It accumulates all Intron data into a single item.
//
//----------------------------------------------------------------------------
class IntronAccumulator : public Accumulator // Exon/Intron DataBase Intron Accumulator
{
//----------------------------------------------------------------------------
// IntronAccumulator::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~IntronAccumulator( void );      // Default destructor
   IntronAccumulator( void );       // Default constructor

private:                            // Bitwise copy prohibited
   IntronAccumulator(const IntronAccumulator&);
   IntronAccumulator& operator=(const IntronAccumulator&);

//----------------------------------------------------------------------------
// IntronAccumulator::methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next accumulator line
   load( void );                    // Load accumuator line

//----------------------------------------------------------------------------
// IntronAccumulator::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class IntronAccumulator

#endif // ACCUMULATOR_H_INCLUDED
