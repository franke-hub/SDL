//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Robots.h
//
// Purpose-
//       Load and parse a "/robots.txt" file.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef ROBOTS_H_INCLUDED
#define ROBOTS_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DataSource;

//----------------------------------------------------------------------------
//
// Class-
//       Robots
//
// Purpose-
//       Load and parse a "/robots.txt" file.
//
//----------------------------------------------------------------------------
class Robots {                      // Robots.txt file parser
//----------------------------------------------------------------------------
// Robots::Attributes
//----------------------------------------------------------------------------
protected:
char**                 match;       // List of allow/forbid entries

double                 delay;       // Crawl delay
int                    visit;       // Visit time (hhmm*10000) + hhmm

//----------------------------------------------------------------------------
// Robots::Constructors
//----------------------------------------------------------------------------
public:
   ~Robots( void );                 // Destructor
   Robots( void );                  // Default constructor

   Robots(                          // Constructor
     const char*       client,      // Client (agent) name
     DataSource&       source);     // ROBOTS.TXT file

//----------------------------------------------------------------------------
// Robots::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

inline double                       // The crawl delay
   getDelay( void ) const           // Get crawl delay
{  return delay;
}

inline int                          // The visit times
   getVisit( void ) const           // Get visit times
{  return visit;
}

int                                 // TRUE iff allowed
   allowed(                         // Is access allowed?
     const char*       url) const;  // For this URL

int                                 // Return code (0 OK)
   open(                            // Initialize this object
     const char*       client,      // Client (agent) name
     DataSource&       source);     // ROBOTS.TXT file

void
   reset( void );                   // Reset (close) this object
}; // class Robots

#endif // ROBOTS_H_INCLUDED
