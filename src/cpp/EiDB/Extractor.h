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
//       Extractor.h
//
// Purpose-
//       Define a mechanism for splitting a sequence into component items.
//
// Last change date-
//       2003/03/16
//
// Classes-
         class Extractor;           // Extractor
         class ExonExtractor;       // Exon extractor
         class IntronExtractor;     // Intron extractor
//
//----------------------------------------------------------------------------
#ifndef EXTRACTOR_H_INCLUDED
#define EXTRACTOR_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Extractor
//
// Purpose-
//       Describe a generic extractor.
//
//----------------------------------------------------------------------------
class Extractor                     // Extractor
{
//----------------------------------------------------------------------------
// Extractor::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Extractor( void );              // Default destructor
   Extractor( void );               // Default constructor

private:                            // Bitwise copy prohibited
   Extractor(const Extractor&);
   Extractor& operator=(const Extractor&);

//----------------------------------------------------------------------------
// Extractor::Methods
//----------------------------------------------------------------------------
public:
unsigned                            // The number of warnings detected
   getWarnings( void ) const;       // Get number of warnings detected

void
   setIgnoreFirst(                  // Set ignore first item control
     int               mode);       // To this value

void
   setIgnoreLast(                   // Set ignore last item control
     int               mode);       // To this value

void
   setIgnoreOnly(                   // Set ignore only item control
     int               mode);       // To this value

void
   load(                            // Load the gene
     char*             gene);       // The sequence to load
                                    // Will be used and modified

char*                               // The next Item
   next(                            // Extract the next Item
     unsigned          line);       // Input line number (for messages)

//----------------------------------------------------------------------------
// Extractor::Internal methods
//----------------------------------------------------------------------------
protected:
virtual char*                       // The next Item
   getNext(                         // Extract the next Item
     unsigned          line);       // Input line number (for messages)

//----------------------------------------------------------------------------
// Extractor::Attributes
//----------------------------------------------------------------------------
protected:
   char*               gene;        // Full sequence
   int                 geneIsFirst; // TRUE if this is the first Item
   int                 ignoreFirst; // TRUE if we should ignore the first Item
   int                 ignoreLast;  // TRUE if we should ignore the last Item
   int                 ignoreOnly;  // TRUE to invert ignoreFirst, ignoreLast
   unsigned            genewarns;   // Number of warnings on this gene
   unsigned            warnings;    // Number of warnings detected
}; // class Extractor

//----------------------------------------------------------------------------
//
// Class-
//       AtgExtractor
//
// Purpose-
//       Describe the exon extractor.
//       (Beginning at the first ATG sequence.)
//
//----------------------------------------------------------------------------
class AtgExtractor : public Extractor // Exon extractor
{
//----------------------------------------------------------------------------
// AtgExtractor::Constructors
//----------------------------------------------------------------------------
public:
   ~AtgExtractor( void );           // Default destructor
   AtgExtractor(                    // Constructor
     int               sw_wild);    // TRUE iff wildcharacter matching applies

private:                            // Bitwise copy prohibited
   AtgExtractor(const AtgExtractor&);
   AtgExtractor& operator=(const AtgExtractor&);

//----------------------------------------------------------------------------
// AtgExtractor::Methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next Exon
   getNext(                         // Extract the next Exon
     unsigned          line);       // Input line number (for messages)

//----------------------------------------------------------------------------
// AtgExtractor::Attributes
//----------------------------------------------------------------------------
protected:
   int                 sw_wild;     // Wild character matching?
}; // class AtgExtractor

//----------------------------------------------------------------------------
//
// Class-
//       ExonExtractor
//
// Purpose-
//       Describe the exon extractor.
//
//----------------------------------------------------------------------------
class ExonExtractor : public Extractor // Exon extractor
{
//----------------------------------------------------------------------------
// ExonExtractor::Constructors
//----------------------------------------------------------------------------
public:
   ~ExonExtractor( void );          // Default destructor
   ExonExtractor( void );           // Default constructor

private:                            // Bitwise copy prohibited
   ExonExtractor(const ExonExtractor&);
   ExonExtractor& operator=(const ExonExtractor&);

//----------------------------------------------------------------------------
// ExonExtractor::Methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next Exon
   getNext(                         // Extract the next Exon
     unsigned          line);       // Input line number (for messages)

//----------------------------------------------------------------------------
// ExonExtractor::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class ExonExtractor

//----------------------------------------------------------------------------
//
// Class-
//       IntronExtractor
//
// Purpose-
//       Describe the Intron extractor.
//
//----------------------------------------------------------------------------
class IntronExtractor : public Extractor // Intron extractor
{
//----------------------------------------------------------------------------
// IntronExtractor::Constructors
//----------------------------------------------------------------------------
public:
   ~IntronExtractor( void );        // Default destructor
   IntronExtractor( void );         // Default constructor

private:                            // Bitwise copy prohibited
   IntronExtractor(const IntronExtractor&);
   IntronExtractor& operator=(const IntronExtractor&);

//----------------------------------------------------------------------------
// IntronExtractor::Methods
//----------------------------------------------------------------------------
public:
virtual char*                       // The next Intron
   getNext(                         // Extract the next Intron
     unsigned          line);       // Input line number (for messages)

//----------------------------------------------------------------------------
// IntronExtractor::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class IntronExtractor

#endif // EXTRACTOR_H_INCLUDED
