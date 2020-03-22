//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       plot.h
//
// Purpose-
//       Plot functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef PLOT_H_INCLUDED
#define PLOT_H_INCLUDED

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifndef PLOTF
  #define PLOTF f
#endif

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static FILE*         plotFile= NULL; // File pointer

static double        plotMin;       // Maximum value
static double        plotMax;       // Minimum value
static double        plotScale;     // Scale factor
static double        scale= 0.0;    // Scale override value

//----------------------------------------------------------------------------
//
// Subroutine-
//       prePlot
//
// Purpose-
//       Prepare to plot.
//
//----------------------------------------------------------------------------
void
   prePlot( void )                  // Prepare to plot
{
   double            delta;         // The step factor
   double            x;             // The current x
   double            y;             // Function(x) resultant

   delta= (upper - lower) / steps;
   plotFile= fopen("plot", "w");
   if( plotFile == NULL )
   {
     fprintf(stderr, "File(plot): ");
     perror("Open failure");
     exit(EXIT_FAILURE);
   }

   plotMin= +999999.9;
   plotMax= -999999.9;
   for(x=lower; x<=upper; x += delta)
   {
     y= PLOTF(x);
     if( y > plotMax )
       plotMax= y;
     if( y < plotMin )
       plotMin= y;
   }
   plotScale= 1.0;
   if( (plotMax - plotMin) > 0.0 )
     plotScale= 80.0 / (plotMax - plotMin);

   if( scale > 0.0 )
     plotScale= scale;

   fprintf(plotFile, "Scale(%12.6f) Min(%12.6f) Max(%12.6f)\n",
                     plotScale, plotMin, plotMax);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       plot
//
// Purpose-
//       Plot a value.
//
//----------------------------------------------------------------------------
inline void
   plot(                            // Plot
     double            x)           // X Value
{
   unsigned            m;
   double              y= PLOTF(x);

   int                 i;

   fprintf(plotFile, "x(%12.6f) y(%12.6f)", x, y);
   m= (unsigned)((y-plotMin)*plotScale);
   for(i=0; i<m; i++)
     fprintf(plotFile, " ");
   fprintf(plotFile, "*\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       endPlot
//
// Purpose-
//       Close plot file.
//
//----------------------------------------------------------------------------
void
   endPlot( void )                  // Close plot file
{
   fclose(plotFile);
}

#endif // PLOT_H_INCLUDED
