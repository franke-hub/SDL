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
//       DarwinUnit.cpp
//
// Purpose-
//       DarwinUnit methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <com/Bit.h>
#include <com/define.h>
#include <com/Debug.h>
#include <com/Random.h>
#include <com/List.h>

#include "com/DarwinUnit.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char      className[]= "DarwinUnit"; // The class name
static const char      hextab[]= "0123456789ABCDEF"; // Value to character

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::~DarwinUnit
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DarwinUnit::~DarwinUnit( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::DarwinUnit
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DarwinUnit::DarwinUnit( void )   // Constructor
:  evaluation(0)
,  generation(0)

// Controls
,  changed(0)
,  mutated(0)
,  evolChange(0)
,  evolMutate(0)
,  isValid(0)
{
   #ifdef HCDM
     debugf("DarwinUnit(%p)::DarwinUnit()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::className
//
// Purpose-
//       Get the UNIQUE class name.
//
//----------------------------------------------------------------------------
const char*                         // -> ClassName
   DarwinUnit::className( void ) const // Get the UNIQUE class name
{
   return ::className;              // Return the class name
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::evolve
//
// Purpose-
//       Evolve a rule.
//
//----------------------------------------------------------------------------
void
   DarwinUnit::evolve(              // Evolve a rule
     unsigned int      size,        // The rule size
     char*             rule,        // Target rule
     const char*       father,      // Father rule
     const char*       mother)      // Mother rule
{
   unsigned int        bytes= RNG.get() % size;
   unsigned int        bits=  RNG.get() % 8;

   #ifdef HCDM
     debugf("DarwinUnit::evolve(%d,%p,%p,%p)\n", size, rule, father, mother);
   #endif

   if( bytes > 0 )
     memcpy(rule, father, bytes);

   rule[bytes]=  (father[bytes]         <<    bits)
              | ((mother[bytes] & 0xff) >> (8-bits));

   bytes++;
   if( bytes < size )
     memcpy(rule+bytes, mother+bytes, size-bytes);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::mutate
//
// Purpose-
//       Mutate a rule.
//
//----------------------------------------------------------------------------
void
   DarwinUnit::mutate(              // Mutate a rule
     unsigned int      size,        // The rule size (in bytes)
     char*             rule)        // The rule
{
   unsigned int        i;

   #ifdef HCDM
     debugf("DarwinUnit::mutate(%d,%p)\n", size, rule);
   #endif

   i= RNG.get() % (size*8);
   Bit::set(rule, i, !Bit::get(rule, i));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::toFile
//
// Purpose-
//       Write the rule on a file.
//
//----------------------------------------------------------------------------
void
   DarwinUnit::toFile(              // Write the rule
     FILE*             file,        // The output file
     unsigned int      size,        // The rule size
     const char*       rule)        // The rule
{
   unsigned int        bits;        // Size in bits

   unsigned int        i;

   bits= size*8;
   for(i=0; i<bits; i++)
   {
     if( (i%8) == 0 )
       fputc('|', file);

     fputc(hextab[Bit::get(rule, i)], file);
   }

   fputc('|', file);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinUnit::toString
//
// Purpose-
//       Convert the rule to a string.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   DarwinUnit::toString(            // Convert to string
     char*             resultant,   // The output string
     unsigned int      size,        // The rule size
     const char*       rule)        // The rule
{
   unsigned int        bits;        // Size in bits
   unsigned int        outx= 0;     // Resultant index

   unsigned int        i;

   bits= size*8;
   for(i=0; i<bits; i++)
   {
     if( (i%8) == 0 )
       resultant[outx++]= '|';

     resultant[outx++]= hextab[Bit::get(rule, i)];
   }

   resultant[outx++]= '|';
   resultant[outx++]= '\0';
   return resultant;
}

