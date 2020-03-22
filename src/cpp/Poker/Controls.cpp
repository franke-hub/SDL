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
//       Controls.cpp
//
// Purpose-
//       Controls implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include "Define.h"
#include "Controls.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       PokerControls::display
//
// Purpose-
//       Display the PokerControls.
//
//----------------------------------------------------------------------------
void
   PokerControls::display( void ) const // Display the PokerControls
{
   const char*         model;       // The betting model

   printf("PokerControls(%p)::display()\n", this);
   model= "INVALID";
   switch(this->model)
   {
     case MODEL_HUMAN:
       model= "HUMAN";
       break;

     case MODEL_RANDOM:
       model= "Random";
       break;

     case MODEL_CONSERVATIVE:
       model= "Conservative";
       break;

     case MODEL_NEUTRAL:
       model= "Neutral";
       break;

     case MODEL_AGGRESSIVE:
       model= "Aggressive";
       break;

     default:
       break;
   }

   printf("edge(%7.4f) model(%s) bluff(%d)\n", edge, model, bluffing);
}

