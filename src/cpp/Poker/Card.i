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
//       Card.i
//
// Purpose-
//       Card inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef CARD_I_INCLUDED
#define CARD_I_INCLUDED

//----------------------------------------------------------------------------
// BaseCard::Accessor methods
//----------------------------------------------------------------------------
void
   BaseCard::display( void ) const  // Get VISIBLE attribute
{
   char                string[SIZE_NAME];

   printf("%s\n", toShortString(string));
}

int                                 // The VISIBLE attribute
   BaseCard::getVisible( void ) const // Get VISIBLE attribute
{
   return visible;
}

void
   BaseCard::setVisible(            // Set VISIBLE attribute
     int               visible)     // The new VISIBLE attribute
{
   this->visible= (visible != 0);
}

//----------------------------------------------------------------------------
// BaseCard::Methods
//----------------------------------------------------------------------------
void
   BaseCard::hide( void )           // Set VISIBLE attribute FALSE
{
   this->visible= FALSE;
}

void
   BaseCard::show( void )           // Set VISIBLE attribute TRUE
{
   this->visible= TRUE;
}

//----------------------------------------------------------------------------
// Card::Accessor methods
//----------------------------------------------------------------------------
Card::Rank                          // The Rank
   Card::getRank( void ) const      // Get Rank
{
   return rank;
}

Card::Suit                          // The Suit
   Card::getSuit( void ) const      // Get Suit
{
   return suit;
}

#endif // CARD_I_INCLUDED
