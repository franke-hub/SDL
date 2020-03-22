//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Controls.h
//
// Purpose-
//       Controls description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef CONTROLS_H_INCLUDED
#define CONTROLS_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       PokerControls
//
// Purpose-
//       The informaion used to evaluate a poker hand.
//
//----------------------------------------------------------------------------
struct PokerControls                // PokerControls
{
//----------------------------------------------------------------------------
// PokerControls::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum Model
{  MODEL_HUMAN                      // HUMAN input
,  MODEL_RANDOM                     // Random
,  MODEL_CONSERVATIVE               // Conservative
,  MODEL_NEUTRAL                    // Neutral
,  MODEL_AGGRESSIVE                 // Aggressive
}; // enum Model

//----------------------------------------------------------------------------
// PokerControls::Constructors
//----------------------------------------------------------------------------
   ~PokerControls( void ) {}        // Destructor
   PokerControls( void )            // Constructor
:  edge(0.0)
,  model(MODEL_RANDOM)
,  bluffing(FALSE)
{
}

//----------------------------------------------------------------------------
// PokerControls::Attributes
//----------------------------------------------------------------------------
   double              edge;        // The PokerPlayer's edge
   Model               model;       // The PokerPlayer's betting model
   int                 bluffing;    // TRUE iff bluffing

//----------------------------------------------------------------------------
// PokerControls::Methods
//----------------------------------------------------------------------------
void
   display( void ) const;           // Display the PokerControls
}; // struct PokerControls

#endif // CONTROLS_H_INCLUDED
