//----------------------------------------------------------------------------
//
//       Copyright (c) 2013 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       BlackBox.cpp
//
// Purpose-
//       BlackBox object implementation.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>
#include <com/Unconditional.h>

#include "BlackBox.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const double    SECONDS_PER_HOUR= 3600; // Number of seconds per hour

//----------------------------------------------------------------------------
//
// Method-
//       BlackBox::~BlackBox
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   BlackBox::~BlackBox( void )      // Destructor
{
   if( history != NULL )
   {
     for(int row= 0; row<rows; row++)
       free(history[row]);

     free(history);
     history= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BlackBox::BlackBox
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   BlackBox::BlackBox(              // Constructor
     int               rows,        // Number of history rows
     int               cols)        // Number of history columns
:  history(NULL)
,  cols(cols)
,  rows(rows)
{
   int                 col;         // Current column
   int                 row;         // Current row

   history= (double**)must_malloc(rows * sizeof(double*));
   for(row= 0; row<rows; row++)
     history[row]= NULL;

   for(row= 0; row<rows; row++)
   {
     history[row]= (double*)malloc(cols * sizeof(double));
     if( history[row] == NULL )
       throwf("No row storage(%ld)\n", (long)cols * sizeof(double*));

     for(col= 0; col<cols; col++)
       history[row][col]= 0.0;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BlackBox::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   BlackBox::debug(                 // Debugging display
     const char*       text)        // Associated text
{
   int                 col;         // Current column
   int                 row;         // Current row

   debugf("BlackBox(%p)::debug(%s)\n", this, text);
   debugf("..%8d rows\n", rows);
   debugf("..%8d cols\n", cols);

   for(row= 0; row<rows; row++)
   {
     debugf("[%3d]", row);
     for(col= 0; col<cols; col++)
       debugf(" %10.3f", history[row][col]);

     debugf("\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BlackBox::setRow
//
// Purpose-
//       Update history
//
//----------------------------------------------------------------------------
void
   BlackBox::setRow(                // Update history
     const double*     update)      // The history row
{
   double* rowZero= history[0];

   for(int col= 0; col<cols; col++) // Update the new row
     rowZero[col]= update[col];

   for(int row= 1; row<rows; row++)
     history[row-1]= history[row];

   history[rows-1]= rowZero;
}

