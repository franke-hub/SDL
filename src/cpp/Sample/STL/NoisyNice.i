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
//       NoisyNice.i
//
// Purpose-
//       Implements NoisyNice.h.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NOISYNICE_I_INCLUDED
#define NOISYNICE_I_INCLUDED

#include "Main.h"

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::~NoisyNice
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NoisyNice::~NoisyNice( void )    // Destructor
{
   wtlc(LevelInfo, "NoisyNice(%p)::~NoisyNice() %d\n", this, serialNum);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::NoisyNice
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   NoisyNice::NoisyNice( void )     // Default constructor
:  Nice()
{
   wtlc(LevelInfo, "NoisyNice(%p)::NoisyNice() %d\n", this, serialNum);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::NoisyNice(const NoisyNice&)
//
// Purpose-
//       Copy constructor.
//
//----------------------------------------------------------------------------
   NoisyNice::NoisyNice(            // Copy constructor
     const NoisyNice&  source)      // Source NoisyNice&
{
   wtlc(LevelInfo, "NoisyNice(%p)::NoisyNice(%p) %d(%d)\n", this, &source,
                   serialNum, source.serialNum);
   Nice::operator=(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::operator=(const NoisyNice&)
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
NoisyNice&                          // Resultant
   NoisyNice::operator=(            // Assignment operator
     const NoisyNice&  source)      // Source NoisyNice&
{
   wtlc(LevelInfo, "NoisyNice(%p)::operator=(%p) %d=%d\n", this, &source,
                   serialNum, source.serialNum);
   Nice::operator=(source);
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::operator<(const NoisyNice&) const
//
// Purpose-
//       Less-than comparison operator.
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   NoisyNice::operator<(            // Less than operator
     const NoisyNice&  source) const// Source NoisyNice&
{
   wtlc(LevelInfo, "NoisyNice(%p)::operator<(%p) %d<%d\n", this, &source,
                   serialNum, source.serialNum);
   return Nice::operator<(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::operator==(const NoisyNice&) const
//
// Purpose-
//       Equality comparison
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   NoisyNice::operator==(           // Equality operator
     const NoisyNice&  source) const // Source NoisyNice&
{
   wtlc(LevelInfo, "NoisyNice(%p)::operator==(%p) %d==%d\n", this, &source,
                   serialNum, source.serialNum);
   return Nice::operator==(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::operator!=(const NoisyNice&) const
//
// Purpose-
//       Inequality comparison
//
//----------------------------------------------------------------------------
int                                 // Resultant (TRUE || FALSE)
   NoisyNice::operator!=(           // Inequality operator
     const NoisyNice&  source) const // Source NoisyNice&
{
   wtlc(LevelInfo, "NoisyNice(%p)::operator!=(%p) %d!=%d\n", this, &source,
                   serialNum, source.serialNum);
   return Nice::operator!=(source);
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::s( void ) const
//
// Purpose-
//       An equality preserving function.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   NoisyNice::s( void ) const       // An equality preserving function
{
   wtlc(LevelInfo, "NoisyNice(%p)::s() %d\n", this, serialNum);
   return Nice::s();
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyNice::i( void )
//
// Purpose-
//       Change the "equality" of the NoisyNice.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   NoisyNice::i( void )             // Change equality
{
   int                 serialNum= this->serialNum;

   Nice::i();
   wtlc(LevelInfo, "NoisyNice(%p)::s() %d=%d\n",
                   this, this->serialNum, serialNum);
   return Nice::s();
}

#endif // NOISYNICE_I_INCLUDED
