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
//       EiDBLoader.h
//
// Purpose-
//       Describe the Exon/Intron DataBase Loader.
//
// Last change date-
//       2003/03/16
//
// Classes-
         class EiDBLoader;          // Exon/Intron DataBase Loader
//
//----------------------------------------------------------------------------
#ifndef EIDBLOADER_H_INCLUDED
#define EIDBLOADER_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Accumulator;
class EiDB;
class Extractor;

//----------------------------------------------------------------------------
//
// Class-
//       EiDBLoader
//
// Purpose-
//       Describe the Exon/Intron DataBase.
//
//----------------------------------------------------------------------------
class EiDBLoader                    // Exon/Intron DataBase Loader
{
//----------------------------------------------------------------------------
// EiDBLoader::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum LOADMODE                       // Load mode
{
   MODE_LEFTRIGHT,                  // Left to right
   MODE_RIGHTLEFT                   // Right to left
}; // enum LOADMODE

//----------------------------------------------------------------------------
// EiDBLoader::Constructors
//----------------------------------------------------------------------------
public:
   ~EiDBLoader( void );             // Default destructor
   EiDBLoader( void );              // Default constructor

private:                            // Bitwise copy prohibited
   EiDBLoader(const EiDBLoader&);
   EiDBLoader& operator=(const EiDBLoader&);

//----------------------------------------------------------------------------
// EiDBLoader::methods
//----------------------------------------------------------------------------
public:
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
   setMaxSize(                      // Set maximum column size
     int               size);       // To this value

void
   setMinSize(                      // Set minimum column size
     int               size);       // To this value

int                                 // Return code, (0 OK)
   load(                            // Load the EiDB
     EiDB&             eidb,        // The EiDB to load
     Accumulator&      accumulator, // The Accumulator
     Extractor&        extractor,   // The Extractor
     LOADMODE          loadMode= MODE_LEFTRIGHT); // Load Mode

//----------------------------------------------------------------------------
// EiDBLoader::Attributes
//----------------------------------------------------------------------------
protected:
   int                 ignoreFirst; // Ignore first item?
   int                 ignoreLast;  // Ignore last item?
   int                 ignoreOnly;  // Ignore only specified?
   unsigned            maxSize;     // Sizeof(largest allowed line)
   unsigned            minSize;     // Sizeof(smallest allowed line)
}; // class EiDBLoader

#endif // EIDBLOADER_H_INCLUDED
