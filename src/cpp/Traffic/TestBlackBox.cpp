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
//       TestBlackBox.cpp
//
// Purpose-
//       BlackBox object unit test.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <com/Debug.h>

#include "BlackBox.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
#define ROWS 13
#define COLS 11

static BlackBox        history(ROWS,COLS); // BlackBox history

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
const double           verifier[ROWS][COLS]=
{  {  1.01,  1.02,  1.03,  1.04,  1.05,  1.06,  1.07,  1.08,  1.09,  1.10,  1.11}
,  {  2.01,  2.02,  2.03,  2.04,  2.05,  2.06,  2.07,  2.08,  2.09,  2.10,  2.11}
,  {  3.01,  3.02,  3.03,  3.04,  3.05,  3.06,  3.07,  3.08,  3.09,  3.10,  3.11}
,  {  4.01,  4.02,  4.03,  4.04,  4.05,  4.06,  4.07,  4.08,  4.09,  4.10,  4.11}
,  {  5.01,  5.02,  5.03,  5.04,  5.05,  5.06,  5.07,  5.08,  5.09,  5.10,  5.11}
,  {  6.01,  6.02,  6.03,  6.04,  6.05,  6.06,  6.07,  6.08,  6.09,  6.10,  6.11}
,  {  7.01,  7.02,  7.03,  7.04,  7.05,  7.06,  7.07,  7.08,  7.09,  7.10,  7.11}
,  {  8.01,  8.02,  8.03,  8.04,  8.05,  8.06,  8.07,  8.08,  8.09,  8.10,  8.11}
,  {  9.01,  9.02,  9.03,  9.04,  9.05,  9.06,  9.07,  9.08,  9.09,  9.10,  9.11}
,  { 10.01, 10.02, 10.03, 10.04, 10.05, 10.06, 10.07, 10.08, 10.09, 10.10, 10.11}
,  { 11.01, 11.02, 11.03, 11.04, 11.05, 11.06, 11.07, 11.08, 11.09, 11.10, 11.11}
,  { 12.01, 12.02, 12.03, 12.04, 12.05, 12.06, 12.07, 12.08, 12.09, 12.10, 12.11}
,  { 13.01, 13.02, 13.03, 13.04, 13.05, 13.06, 13.07, 13.08, 13.09, 13.10, 13.11}
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize BlackBox data
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize BlackBox data
{
   for(int i= 1; i<=ROWS; i++)
     history.setRow(verifier[i-1]);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Verify BlackBox data
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   test( void )                     // Verify BlackBox data
{
   int                 errorCount= 0; // Number of errors encountered

   for(int i= 1; i<=ROWS; i++)
   {
     const double* row= history.getRow(i-1);

     for(int j= 1; j<=COLS; j++)
     {
       if( row[j-1] != verifier[i-1][j-1] )
       {
         debugf("[%2d][%2d] Expected(%10.4f) Got(%10.4f)\n", i, j,
                verifier[i-1][j-1], row[j-1]);
         errorCount++;
       }
     }
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   init();

   int rc= test();
   if( rc == 0 )
     debugf("TestBlackBox COMPLETE, NO errors\n");
   else
   {
     debugf("TestBlackBox FAILURE, %d error%s\n", rc, rc > 1 ? "s" : "");
     rc= 1;
   }

// history.debug();
   return rc;
}

