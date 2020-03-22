//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wildchar.h
//
// Purpose-
//       Define the Wildchar object.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       This object is used to extend character comparisons so that wild
//       card characters can be used.  A wild card character is equivalent
//       to itself and any of its replacement characters.
//
//       Wild character replacement is not recursive.  The wild card list
//       must include all replacement characters.  Wild characters in the
//       wild card list are not expanded.
//
//       The NUL character, '\0', cannot have a replacement character.
//
//       The resultant of any compare operation is:
//         <0 iff source < target
//         =0 iff source == target
//         >0 iff source > target
//       For non-zero resultants, the sign is based on the original comparison
//       rather than any wild card expansion comparison.
//
//----------------------------------------------------------------------------
#ifndef WILDCHAR_H_INCLUDED
#define WILDCHAR_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Wildchar
//
// Purpose-
//       Character comparisons using wild card characters.
//
//----------------------------------------------------------------------------
class Wildchar {                    // Wildchar
//----------------------------------------------------------------------------
// Wildchar::Attributes
//----------------------------------------------------------------------------
protected:
const char*            wildlist[256]; // Wild character list

//----------------------------------------------------------------------------
// Wildchar::Constructors
//----------------------------------------------------------------------------
public:
   ~Wildchar( void );               // Destructor
   Wildchar( void );                // Default constructor

private:                            // Bitwise copy is prohibited
   Wildchar(const Wildchar&);       // Disallowed copy constructor
Wildchar&
   operator=(const Wildchar&);      // Disallowed assignment operator

//----------------------------------------------------------------------------
// Wildchar::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Resultant
   get(                             // Extract the wild card list
     int               wild);       // For this character

const char*                         // Resultant (prior wild card list)
   set(                             // Replace the wild card list
     int               wild,        // For this character
     const char*       list);       // Using this list

int                                 // Resultant <0, =0, >0)
   compare(                         // Compare characters using wild cards
     int               source,      // Source character
     int               target);     // Target character

int                                 // Resultant <0, =0, >0)
   compare(                         // Compare strings using wild cards
     const char*       source,      // Source string
     const char*       target);     // Target string

int                                 // Resultant <0, =0, >0)
   compare(                         // Compare memory using wild cards
     const char*       source,      // Source data area
     const char*       target,      // Target data area
     unsigned          length);     // Data area length

char*                               // Resultant
   strstr(                          // strstr with wild card replacement
     const char*       string,      // Source string
     const char*       substr);     // Substring
}; // class Wildchar

#endif // WILDCHAR_H_INCLUDED
