//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Generate.cpp
//
// Purpose-
//       Generate raw terraform data.
//
// Last change date-
//       2017/01/01
//
// Parameters-
//       NONE IMPLEMENTED.
//
// Input-
//       stdin: A set of position vectors.
//       Each position vector is specified as an X, Y, and Z position.
//
// Output-
//       stdout: The interpolated height values
//       Note: Output is [Y][X] and NOT [X][Y].
//
// Counts-
//       [0] Normal
//       [1] Orthogonal Vector Height 0
//       [2] Duality
//       [3] Exact match -or- Singularity
//
// Logic-
//       Each location is interpolated between the nearest three position
//       vectors which surround the position.
//       If the location is at a position vector, it's used.
//
//       If there aren't three positions, interpolation suffices.
//
//----------------------------------------------------------------------------
#include <iostream>                 // C++ iostream
#include <list>                     // C++ list
using namespace std;

#include <stdarg.h>                 // For va_list
#include <math.h>                   // For sqrt
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <sys/stat.h>               // For struct stat

#include "Position.h"
#include "XY.h"
#include "XYZ.h"

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef DEBUG_X
#define DEBUG_X (-1)                // Debug X value (normally -1)
#endif

#ifndef DEBUG_Y
#define DEBUG_Y (-1)                // Debug Y value (normally -1)
#endif

#ifndef SMOOTHIES
#define SMOOTHIES 4                 // The number of smoothing iterations
#endif

//----------------------------------------------------------------------------
// Constant for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#if( FALSE )                        // TRUE for gdb debugging
  #define INPUT_FILE "data/inp.west12" // Specify input file
#endif

#ifdef HCDM
  #define URHERE() debug("%4d URHERE\n", __LINE__)
#else
  #define URHERE() ((void)0)
#endif

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const float     EPSILON = 1e-10; // A small value
static const float     MIN_DISTANCE= 1.0; // Minimum allowed separation

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             counts[4];   // Operation counters
static float           height[256][256]; // The 256x256 height array
static float           update[256][256]; // The 256x256 height array

static list<Position*> pList;       // The list of Position* elements

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Write a debug message onto stderr
//
//----------------------------------------------------------------------------
static inline void
   debug(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   #ifdef HCDM
     va_list           argptr;      // Argument list pointer

     va_start(argptr, fmt);         // Initialize va_ functions
     vfprintf(stderr, fmt, argptr);
     va_end(argptr);                // Close va_ functions
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       Display position content.
//
//----------------------------------------------------------------------------
static inline void
   debug(                           // Write error message
     const Position&   p)           // The associated position
{
   debug("<%6.1f,%6.1f,%6.1f>", p.x, p.y, p.z);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write an error message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Display position content.
//
//----------------------------------------------------------------------------
static inline void
   error(                           // Write error message
     const Position&   p)           // The associated position
{
   error("<%6.1f,%6.1f,%6.1f>", p.x, p.y, p.z);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       randomize
//
// Purpose-
//       Randomize a result up to +/- RANDOMIZER
//
//----------------------------------------------------------------------------
#if( FALSE )
static float           RANDOMIZER;  // RANDOMIZER value
static inline float                 // Randomizer result
   randomize( void )                // Add noise to result
{
   int temp= rand() % 2001;         // Valuerange 0..2000
   temp -= 1000;                    // Valuerange -1000..1000
   float result= (float)(temp)/1000.0; // Valuerange -1.0..+1.0
   result *= RANDOMIZER;            // Valuerange -RANDOMIZER..+RANDOMIZER

   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       area
//
// Purpose-
//       Determine area of triangle.
//
//----------------------------------------------------------------------------
static inline float                 // Area of triangle (* 2.0)
   area(                            // Get Area of triangle
     const XYZ&        p1,          // The XYZ point 1
     const XYZ&        p2,          // The XYZ point 2
     const XYZ&        p3)          // The XYZ point 3
{
   return fabs(p1.x * p2.y
             + p2.x * p3.y
             + p3.x * p1.y
             - p1.x * p3.y
             - p3.x * p2.y
             - p2.x * p1.y);
}

static inline float                 // Area of triangle (* 2.0)
   area(                            // Get Area of triangle
     const XYZ&        p1,          // The XYZ point 1
     const XYZ&        p2,          // The XYZ point 2
     const XY&         p3)          // The XY point 3
{
   return fabs(p1.x * p2.y
             + p2.x * p3.y
             + p3.x * p1.y
             - p1.x * p3.y
             - p3.x * p2.y
             - p2.x * p1.y);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       atPosition
//
// Purpose-
//       Determine Position at XY location.
//
//----------------------------------------------------------------------------
static        Position*             // -> Position
   atPosition(                      // Get Position at XY location
     const int         x,           // The x locatiion
     const int         y)           // The y locatiion
{
   Position*           result= NULL;// Resultant

   list<Position*>::iterator i;
   for(i= pList.begin(); i != pList.end(); i++)
   {
     Position* ip= *i;

     if( ip->x == x && ip->y == y )
     {
       result= ip;
       break;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       minPosition
//
// Purpose-
//       Determine Position with minimum XY distance.
//
//----------------------------------------------------------------------------
static inline Position*             // -> Position
   minPosition(                     // Get Position with minimum XY distance
     const XY&         H,           // From here,
     const Position*   p1= NULL)    // Omitting this position
{
   Position*           result= NULL;// Resultant
   float               distance= 1000000.0; // Associated distance

   list<Position*>::iterator i;
   for(i= pList.begin(); i != pList.end(); i++)
   {
     Position* ip= *i;
     if( ip == p1 )
       continue;

     if( p1 != NULL )               // Must surround point
     {
       if( p1->x >= ip->x )
       {
         if( H.x >= p1->x && H.y >= p1->y
             && H.x >= ip->x && H.y >= ip->y )
           continue;
         if( H.x < p1->x && H.y < p1->y
             && H.x < ip->x && H.y < ip->y )
           continue;
       }
       else if( p1->x < ip->x )
       {
         if( H.x < p1->x && H.y >= p1->y
             && H.x < ip->x && H.y >= ip->y )
           continue;
         if( H.x >= p1->x && H.y < p1->y
             && H.x >= ip->x && H.y < ip->y )
           continue;
       }
     }

     float d= ip->separation(H);
     if( d < distance )
     {
       distance= d;
       result= ip;
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       minPosition
//
// Purpose-
//       Determine Position with minimum XY distance that surrounds the
//       target position.
//
//----------------------------------------------------------------------------
static inline Position*             // -> Position
   minPosition(                     // Get Position with minimum XY distance
     const XY&         H,           // From here,
     const Position*   p1,          // Omitting this position
     const Position*   p2)          // And this position
{
   Position*           result= NULL;// Resultant
   float               distance= 1000000.0; // Associated distance

   #if( FALSE )
     debug("\n");
     debug("minPosition(%f,%f) omit(", H.x, H.y);
     debug(*p1); debug(","); debug(*p2); debug(")\n");
   #endif

   list<Position*>::iterator i;
   for(i= pList.begin(); i != pList.end(); i++)
   {
     Position* p3= *i;
     if( p3 == p1 || p3 == p2 )
       continue;

     // Insure that H is within the triangle defined by <p1,p2,p3>
     float aTot= area(*p1, *p2, *p3);

     float a1= area(*p1, *p2, H);
     float a2= area(*p2, *p3, H);
     float a3= area(*p3, *p1, H);
     if( fabs(a1 + a2 + a3 - aTot) > EPSILON )
       continue;

     float d= p3->separation(H);
     if( H.x == DEBUG_X && H.y == DEBUG_Y )
     {
       error("[%d][%d] %f accepted ", DEBUG_X, DEBUG_Y, d);
       error(*p3); error("\n");
     }

     if( d < distance )
     {
       distance= d;
       result= p3;
     }
   }

   if( H.x == DEBUG_X && H.y == DEBUG_Y )
     error("%p= minPosition([%f,%f] Given [%f,%f] and [%f,%f]\n",
           result, H.x, H.y, p1->x, p1->y, p2->x, p2->y);

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getHeight
//
// Purpose-
//       Compute height for position.
//
// Implementation logic-
//       1) Determine nearest element.
//       2) Determine next nerest element.
//       3) Compute third nearest element.
//          The three elements must create a triangle that surrounds the
//          position. If this cannot be done, an approximation is used.
//
//----------------------------------------------------------------------------
static float                        // The associated height
   getHeight(                       // Compute height for position
     int               x,           // The X position
     int               y)           // The Y position
{
   float               result= 0.0; // Resultant

   int DEBUG= FALSE;
   if( x == DEBUG_X && y == DEBUG_Y )
     DEBUG= TRUE;

   XY xy((float)x, (float)y);       // We are here
   Position* p1= minPosition(xy);   // The closest point
   if( p1->x == xy.x && p1->y == xy.y ) // Use exact match
   {
     counts[3]++;
     if( DEBUG )
       error("%f= getHeight(%d,%d) by definition(", p1->z, x, y);

     return p1->z;
   }

   Position* p2= minPosition(xy, p1); // The next closest surrounding point
   if( p2 == NULL )                 // If singularity
   {
     result= p1->z;
     if( DEBUG )
     {
       error("%f= getHeight(%d,%d) singularity(", result, x, y);
       error(*p1); error(")\n");
     }
     counts[3]++;
     return result;
   }

   Position* p3= minPosition(xy, p1, p2); // The closest enclosing point
   if( p3 == NULL )                 // If duality
   {
     XYZ xyz((float)x, (float)y, (p1->z + p2->z)/2.0);
     Position ix= xyz.intersection(*p1, *p2);
     result= ix.z;
     if( DEBUG )
     {
       error("%f= getHeight(%d,%d) duality(", result, x, y);
       error(*p1); error(","); error(*p2); error(")\n");
     }
     counts[2]++;
     return result;
   }

#if TRUE
   //-------------------------------------------------------------------------
   // Determine the equation for the plane
   // The normal vector, Vn, is orthoginal to the plane and is the cross
   // product of any two lines on the plane.
   // Google: Plane equation from 3 points
   Position v1= *p2 - *p1;          // The vector from P1 to P2
   Position v2= *p3 - *p1;          // The vector from P1 to P3
   Position vN= v1.cross(v2);       // The normal vector (orthoginal)

   //-------------------------------------------------------------------------
   // The plane equations:
   // d = vN.dot(Q), for any Q on the plane
   // d = vN.x * Q.x + Vn.y * Q.y + Vn.Z * Q.z
   // vN.z * Q.z = d - (vN.x * Q.x) - (Vn.y * Q.y)
   //
   // Solving for Q.z
   // Q.z = (d - (vN.x * Q.x) - (vN.y * Q.y)) / vN.z
   float d= vN.dot(*p1);            // The plane constant
   result= d - vN.x*xy.x - vN.y*xy.y;
   if( vN.z == 0.0 )
   {
     // Since the orthoginal vector height is zero, we cannot use it.
     // We use the weighted point height instead.
     // Note that the point distance cannot be zero. (Checked above.)
     float d1= p1->separation(xy);
     float d2= p2->separation(xy);
     float d3= p3->separation(xy);

     float dT= d1+d2+d3;
     float w1= dT/d1;
     float w2= dT/d2;
     float w3= dT/d3;

     if( DEBUG )
     {
       error("d1(%f) d2(%f) d3(%f)\n", d1, d2, d3);
       error("w1(%f) w2(%f) w3(%f)\n", w1, w2, w3);
     }

     result= (p1->z*w1+p2->z*w2+p3->z*w3)/(w1+w2+w3);
     counts[1]++;
   }
   else
   {
     result /= vN.z;
     counts[0]++;
   }

   if( DEBUG )
   {
     error("\n[%d][%d] %f %d\n", x, y, result, (vN.z == 0.0));
     error("d(%f) %f %f\n", d, vN.dot(*p2), vN.dot(*p3));
     error("p1"); error(*p1); error("\n");
     error("p2"); error(*p2); error("\n");
     error("p3"); error(*p3); error("\n");
     error("v1"); error(v1); error("\n");
     error("v2"); error(v2); error("\n");
     error("vN"); error(vN); error("\n");
   }
#else
   //-------------------------------------------------------------------------
   // (Equivalent to above method)
   // Determine the equation for the plane Ax + By + Cz == D
   // http://paulbourke.net/geometry/planeeq/ (slightly modified)
   float A= p1->y * (p2->z - p3->z)
          + p2->y * (p3->z - p1->z)
          + p3->y * (p1->z - p2->z);

   float B= p1->z * (p2->x - p3->x)
          + p2->z * (p3->x - p1->x)
          + p3->z * (p1->x - p2->x);

   float C= p1->x * (p2->y - p3->y)
          + p2->x * (p3->y - p1->y)
          + p3->x * (p1->y - p2->y);

   float D= p1->x * (p2->y * p3->z - p3->y * p2->z)
          + p2->x * (p3->y * p1->z - p1->y * p3->z)
          + p3->x * (p1->y * p2->z - p2->y * p1->z);

   // Point xy is on the plane, thus
   result= D - A*xy.x - B*xy.y;
   if( C == 0.0 )
   {
     // Since the orthoginal vector height is zero, we cannot use it.
     // We use the weighted point height instead.
     // Note that the point distance cannot be zero. (Checked above.)
     float d1= p1->separation(xy);
     float d2= p2->separation(xy);
     float d3= p3->separation(xy);

     float dT= d1+d2+d3;
     float w1= dT/d1;
     float w2= dT/d2;
     float w3= dT/d3;

     if( DEBUG )
     {
       error("d1(%f) d2(%f) d3(%f)\n", d1, d2, d3);
       error("w1(%f) w2(%f) w3(%f)\n", w1, w2, w3);
     }

     result= (p1->z*w1+p2->z*w2+p3->z*w3)/(w1+w2+w3);
     counts[1]++;
   }
   else
   {
     result /= C;
     counts[0]++;
   }

   if( DEBUG )
   {
     error("\n[%d][%d] %f %d\n", x, y, result, (C == 0.0));
     error("p1"); error(*p1); error("\n");
     error("p2"); error(*p2); error("\n");
     error("p3"); error(*p3); error("\n");
     error("A(%f) B(%f) C(%f) D(%f)\n", A, B, C, D);
   }
#endif

   debug("%f= getHeight(%d,%d)\n", result, x, y);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       generate
//
// Purpose-
//       Generate the raw format output file.
//
//----------------------------------------------------------------------------
static void
   generate( void )                 // Generate the output file
{
   debug("generating()...\n");
   counts[0]= counts[1]= counts[2]= counts[3]= 0;

   if( FALSE )
   {
     getHeight(  3, 250);
     getHeight(250, 250);
     getHeight(  3,  50);
     getHeight(250,  50);

     return;
   }

   for(int y= 0; y<256; y++)
   {
     for(int x= 0; x<256; x++)
     {
       height[x][y]= getHeight(x, y);
     }
   }

   if( TRUE )
     error("Counts: %d, %d, %d, %d\n",
           counts[0], counts[1], counts[2], counts[3]);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Purpose-
//       Load the parameters and the position vectors.
//
//----------------------------------------------------------------------------
static void
   load( void )                     // Load the input data
{
   int                 rc;          // Return code
   float               x, y, z;     // X, Y, and Z position

   debug("loading()...\n");

   #ifdef INPUT_FILE
     FILE* input= fopen(INPUT_FILE, "rb");

   #else
     // Verify stdin
     struct stat buff;
     int fn= fileno(stdin);
     rc= fstat(fn, &buff);
     if( rc != 0 )
     {
       error("ERROR: %d= stat(STDIN), ", rc);
       perror("stat(STDIN)");
       exit(EXIT_FAILURE);
     }

#ifndef _OS_WIN
     if( S_ISCHR(buff.st_mode) )
     {
       error("ERROR: terminal input not supported\n");
       exit(EXIT_FAILURE);
     }
#endif

     FILE* input= stdin;
   #endif

   // Load the parameters
   for(;;)
   {
     rc= fscanf(input, " %f", &x);
     if( rc == EOF )
       break;

     if( rc == 1 )
       rc= fscanf(input, " %f", &y);
     if( rc == 1 )
       rc= fscanf(input, " %f", &z);
     if( rc != 1 )
     {
       error("input incomplete position\n");
       exit(EXIT_FAILURE);
     }

     pList.push_back(new Position(x,y,z));
   }

   // Check for duplicates
   int ERROR= FALSE;
   list<Position*>::iterator i, j;
   for(i= pList.begin(); i != pList.end(); i++)
   {
     Position* ip= *i;

     j= i;
     j++;
     for(; j != pList.end(); j++)
     {
       Position* jp= *j;
       if( ip->separation(*jp) < MIN_DISTANCE )
       {
         ERROR= TRUE;
         error("Points <%f,%f> and <%f,%f> closer than(%f)\n",
               ip->x, ip->y, jp->x, jp->y, MIN_DISTANCE);
       }
     }
   }

   // Must have at least one entry
   if( pList.begin() == pList.end() )
   {
     ERROR= TRUE;
     error("No data points specified\n");
   }

   // Check for range errors
   for(i= pList.begin(); i != pList.end(); i++)
   {
     Position* ip= *i;

     if( ip->x < 0.0 || ip->x > 256.0 )
     {
       ERROR= TRUE;
       error("Point <%f,%f,%f> out of X range)\n", ip->x, ip->y, ip->z);
     }

     if( ip->y < 0.0 || ip->y > 256.0 )
     {
       ERROR= TRUE;
       error("Point <%f,%f,%f> out of Y range)\n", ip->x, ip->y, ip->z);
     }
   }

   // Debugging display
   if( TRUE )
   {
     for(i= pList.begin(); i != pList.end(); i++)
     {
       Position* p= *i;
       error("%8.3f %8.3f %8.3f\n", p->x, p->y, p->z);
     }

     //ERROR= TRUE;
   }

   if( ERROR )
   {
     error("Input data rejected\n");
     exit(EXIT_FAILURE);
   }

   #ifdef INPUT_FILE
     fclose(input);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       upcopy
//
// Purpose-
//       Copy from or into the height array
//
//----------------------------------------------------------------------------
#define upcopy(target, source)      \
{                                   \
   for(int j= 0; j<256; j++)        \
   {                                \
     for(int i= 0; i<256; i++)      \
       target[i][j]= source[i][j];  \
   }                                \
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       smooth
//
// Purpose-
//       Smooth the height array.
//
//----------------------------------------------------------------------------
static void
   smooth( void )                   // Smooth the output array
{
   upcopy(update, height);          // Initialize the update array

   for(int iteration= 0; iteration<SMOOTHIES; iteration++)
   {
     for(int x= 1; x<255; x++)
     {
       // Smooth the borders
       if( atPosition(x,0) == NULL )
       {
         float avg= height[x-1][0]
                 // height[x-0][0]
                  + height[x+1][0]
                  ;
         avg= avg/2.0;
         update[x][0]= (height[x][0] + avg)/2.0;
         if( DEBUG_X == x && DEBUG_Y == 0 )
           error("%f = smooth(%f,%f,%f)\n", update[x][0], height[x-1][0], height[x][0], height[x+1][0]);
       }

       if( atPosition(x,255) == NULL && atPosition(x,256) == NULL )
       {
         float avg= height[x-1][255]
                 // height[x-0][255]
                  + height[x+1][255]
                  ;
         avg= avg/2.0;
         update[x][255]= (height[x][255] + avg)/2.0;
         if( DEBUG_X == x && DEBUG_Y == 255 )
           error("%f = smooth(%f,%f,%f)\n", update[x][255], height[x-1][255], height[x][255], height[x+1][255]);
       }

       if( atPosition(0,x) == NULL )
       {
         float avg= height[0][x-1]
                 // height[0][x-0]
                  + height[0][x+1]
                  ;
         avg= avg/2.0;
         update[0][x]= (height[0][x] + avg)/2.0;
         if( DEBUG_X == 0 && DEBUG_Y == x )
           error("%f = smooth(%f,%f,%f)\n", update[0][x], height[0][x-1], height[0][x], height[0][x+1]);
       }

       if( atPosition(255,x) == NULL && atPosition(256,x) == NULL )
       {
         float avg= height[255][x-1]
                 // height[255][x-0]
                  + height[255][x+1]
                  ;
         avg= avg/2.0;
         update[255][x]= (height[255][x] + avg)/2.0;
         if( DEBUG_X == 255 && DEBUG_Y == x )
           error("%f = smooth(%f,%f,%f)\n", update[255][x], height[255][x-1], height[255][x], height[255][x+1]);
       }

       // Smooth the interior
       for(int y= 1; y<255; y++)
       {
         if( atPosition(x,y) == NULL )
         {
           #if( FALSE )
             float avg= height[x-1][y-1]
                      + height[x-0][y-1]
                      + height[x+1][y-1]
                      + height[x-1][y-0]
                     // height[x-0][y-0]
                      + height[x+1][y-0]
                      + height[x-1][y+1]
                      + height[x-0][y+1]
                      + height[x+1][y+1]
                      ;
             avg= avg/8.0;
             update[x][y]= (height[x][y] + avg)/2.0;
           #else
             float avg= height[x-1][y-1]
                      + height[x-0][y-1]
                      + height[x+1][y-1]
                      + height[x-1][y-0]
                      + height[x-0][y-0]
                      + height[x+1][y-0]
                      + height[x-1][y+1]
                      + height[x-0][y+1]
                      + height[x+1][y+1]
                      ;
             update[x][y]= avg/9.0;
           #endif

           if( DEBUG_X == x && DEBUG_Y == y )
             error("%f = smooth(\n%f,%f,%f\n%f,%f,%f\n%f,%f,%f)\n" , update[x][y]
                  , height[x-1][y+1], height[x-0][y+1], height[x+1][y+1]
                  , height[x-1][y-0], height[x-0][y-0], height[x+1][y-0]
                  , height[x-1][y-1], height[x-0][y-1], height[x+1][y-1]);
         }
       }
     }

     upcopy(height, update);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       output
//
// Purpose-
//       Write the output array.
//
//----------------------------------------------------------------------------
static void
   output( void )                   // Write the output array
{
   for(int x= 0; x<256; x++)
   {
     for(int y= 0; y<256; y++)
     {
       if( y != 0 )
         printf(" ");

       printf("%f", height[y][x]);
     }

     printf("\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       verify
//
// Purpose-
//       Verify the output array
//
//----------------------------------------------------------------------------
static void
   verify( void )                   // Verify the output array
{
   if( TRUE )
   {
     list<Position*>::iterator ii;
     for(ii= pList.begin(); ii != pList.end(); ii++)
     {
       Position* ip= *ii;

       int x= (int)ip->x;
       int y= (int)ip->y;

       if( x >= 0 && x < 256 && y >= 0 && y <255
           && (fabs(ip->z - height[x][y])) > 0.1 )
         error("[%d][%d] %f != inp(%f)\n", x, y, height[x][y], ip->z);
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter informational display.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter analysis
{
   error("Generate: Generate raw output file\n"
         "\n"
         "Input: stdin (Terminal input not allowed)\n"
         "A list of x,y, and z coordinates.\n"
         "Output: stdout\n"
         "The set of 256x256 interpolated x, y, and z coordinates\n"
         "\n"
         );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   if( FALSE )
     info();
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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   parm(argc, argv);                // Parameter analysis

   load();                          // Load the data
   generate();                      // Generate the raw output
   smooth();                        // Smooth the raw output
   verify();                        // Verify the output array
   output();                        // Write the output data

   return EXIT_SUCCESS;
}

