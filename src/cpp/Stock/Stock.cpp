//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stock.cpp
//
// Purpose-
//       Stock data analyzer.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/Julian.h>
#include <com/Reader.h>
#include <com/ParseINI.h>
#include <com/Random.h>
#include <com/Reader.h>
#include <com/Writer.h>

#include "Fanin.h"
#include "Neuron.h"
#include "Stock.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "STOCK   " // Source file, for debugging

//----------------------------------------------------------------------------
// Inlines
//----------------------------------------------------------------------------
#include "Fanin.i"
#include "Neuron.i"

//----------------------------------------------------------------------------
//
// NEURAL NET (Global)
//
//----------------------------------------------------------------------------
NeuronValue*         inpArrayN= NULL; // The input value array
Neuron*              l3ArrayN=  NULL; // The L3 array
Neuron*              l2ArrayN=  NULL; // The L2 array
Neuron*              l1ArrayN=  NULL; // The L1 array
Neuron*              outArrayN= NULL; // The output array

Fanin**              l3ArrayF=  NULL; // The  L3 -> INP mesh
Fanin**              l2ArrayF=  NULL; // The  L2 ->  L3 mesh
Fanin**              l1ArrayF=  NULL; // The  L1 ->  L2 mesh
Fanin**              outArrayF= NULL; // The OUT ->  L1 mesh

//----------------------------------------------------------------------------
//
// EVALUATION CONTROLS
//
//----------------------------------------------------------------------------
static Unit          unit[DIM_UNIT];// The evaluation units
static Plex          plex(DIM_UNIT);// The evaluation group

//----------------------------------------------------------------------------
//
// HISTORY DATA (Global)
//
//----------------------------------------------------------------------------
unsigned             histIndex0;    // Minimum history index
unsigned             histIndexN;    // Maximum history index
unsigned             histIndexU;    // Number of index points to use
int                  histJulian[DIM_HIST+1]; // History Date data
double               histPrice[DIM_HIST]; // History Price data
double               histVolume[DIM_HIST]; // History Volume data

//----------------------------------------------------------------------------
//
// FUNCTIONAL CONTROLS AND PARAMETERS
//
//----------------------------------------------------------------------------
Global               global;        // Global area

//----------------------------------------------------------------------------
//
// CONSTANTS
//
//----------------------------------------------------------------------------
static int           const doy[2][12]= {
   {   0,  31,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334}, // Standard
   {   0,  31,  60,  91, 121, 152, 182, 213, 244, 274, 305, 335}  // Leap year
   };

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int           swHist= FALSE; // TRUE if history display wanted

//----------------------------------------------------------------------------
//
// Subroutine-
//       findBlank
//
// Purpose-
//       Find next blank
//
//----------------------------------------------------------------------------
static inline void
   findBlank(                       // Find blanks in reader file
     LineReader&     reader)        // The Reader
{
   reader.findBlank();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getTime
//
// Purpose-
//       Get Julian second of day.
//
//----------------------------------------------------------------------------
static double                       // Second of day
   getTime(                         // Get Julian second of day
     const Julian&   julian)        // From this Julian
{
   double tod= julian.getTime();    // The Julian second (fractional)
   uint64_t second= (uint64_t)tod;  // The Julian second (truncated)
   tod -= (double)second;           // The fractional day
   tod *= (double)Julian::SECONDS_PER_DAY;
   return tod;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipBlank
//
// Purpose-
//       Skip blanks
//
//----------------------------------------------------------------------------
static int                          // Next non-blank
   skipBlank(                       // Skip blanks in reader file
     LineReader&     reader)        // The Reader
{
   return reader.skipBlank();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       skipLine
//
// Purpose-
//       Position at beginning of next line.
//
//----------------------------------------------------------------------------
static int                          // Delimiter
   skipLine(                        // Skip remainder of line
     LineReader&     reader)        // The Reader
{
   return reader.skipLine();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       badData
//
// Purpose-
//       Handle bad data in file.
//
//----------------------------------------------------------------------------
static void
   badData(                         // Convert string to number
     const char*     name,          // The file name
     LineReader&     reader,        // The Reader
     int             date,          // The date
     const char*     message)       // Error message
{
   fprintf(stderr, "File(%s) Line(%ld): %ld %s\n",
                   name, (long)reader.getLine(),
                   (long)date, message);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       toNumber
//
// Purpose-
//       Extract numeric value from file.
//
//----------------------------------------------------------------------------
static int                          // Resultant number
   toNumber(                        // Convert string to number
     LineReader&     reader)        // The Reader
{
   int               result;
   int               c;

   c= reader.prior();

   result= 0;
   while( c != ' ' )
   {
     if( c == '.' )
     {
       c= reader.get();
       continue;
     }

     result= result * 10 + (c - '0');
     c= reader.get();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::julian
//
// Purpose-
//       Calculate Julian day.
//
//----------------------------------------------------------------------------
static int                          // The Julian day
   julian(                          // Julian day
     int             inpYear,       // Year
     int             month,         // Month
     int             day)
{
   unsigned          resultant;
   unsigned          leap;
   unsigned          century;
   unsigned          year= inpYear + 4712;

   if( year <= 0 || month <= 0 || day <= 0
       || month > 12 || day > 31)
     printf("%d %d %d\n", year, month, day);

   assert( month > 0 && month <= 12 );
   assert( day  >  0 && day   <= 31 );

   resultant = 365 * year + year/4;

   leap= 0;
   if( (year%4) == 0 )
   {
     resultant--;
     leap= 1;
   }

   resultant += doy[leap][month-1] + day;
   if( resultant <= 2361221 )       // If date before Sept 2, 1752
     return resultant;

   year= inpYear - 300;
   if( month <= 2 )
     year--;
   century= year / 100;
   return resultant - (century*3)/4 - 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadParameters
//
// Purpose-
//       Load the parameter data.
//
//----------------------------------------------------------------------------
static void
   loadParameters( void )           // Initialize the Parameter data
{
   ParseINI          ini;           // The initialization file

   const char*       parm;          // Parameter pointer
   double            cullProb;      // Cull probability
   double            changeProb;    // Change probability
   double            mutateProb;    // Mutation probability

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   cullProb=                   0.5; // 50 percent
   mutateProb=                 0.0; //  0 percent
   changeProb=              0.0001; // 1/100 percent
   histIndex0=                   0; // No origin offset
   histIndexN=                   0; // No explicit count
   histIndexU=                   0; // No ending offset

   global.transferFee=        1500; // $15.00
   global.interestRate=       0.05; //  5 percent per year
   global.initialBalance= 10000000; // $100,000.00
   global.minimumBalance=    30000; //     $300.00
   global.revalControl=          1; // "re-evaluate"+
   global.seedControl=           1; // "randomize"+
   global.traceControl=          0; // "trace"-

   //-------------------------------------------------------------------------
   // Load the parameters
   //-------------------------------------------------------------------------
   ini.open("pgm.ini");             // Load the initialization file

   parm= ini.getValue("Controls", "cullProbability");
   if( parm != NULL )
     cullProb= atof(parm);

   parm= ini.getValue("Controls", "mutateProbability");
   if( parm != NULL )
     mutateProb= atof(parm);

   parm= ini.getValue("Controls", "changeProbability");
   if( parm != NULL )
     changeProb= atof(parm);

   parm= ini.getValue("Controls", "initialBalance");
   if( parm != NULL )
     global.initialBalance= atol(parm);

   parm= ini.getValue("Controls", "interestRate");
   if( parm != NULL )
     global.interestRate= atof(parm);

   parm= ini.getValue("Controls", "minimumBalance");
   if( parm != NULL )
     global.minimumBalance= atol(parm);

   parm= ini.getValue("Controls", "transferFee");
   if( parm != NULL )
     global.transferFee= atol(parm);

   parm= ini.getValue("Debugging","randomize");
   if( parm != NULL )
     global.seedControl= atol(parm);

   parm= ini.getValue("Debugging", "re-evaluate");
   if( parm != NULL )
     global.revalControl= atol(parm);

   parm= ini.getValue("Debugging","trace");
   if( parm != NULL )
     global.traceControl= atol(parm);

   parm= ini.getValue("History",  "minIndex");
   if( parm != NULL )
     histIndex0= atol(parm);

   parm= ini.getValue("History",  "maxIndex");
   if( parm != NULL )
     histIndexN= atol(parm);

   parm= ini.getValue("History",  "useIndex");
   if( parm != NULL )
     histIndexU= atol(parm);

   ini.close();

   //-------------------------------------------------------------------------
   // Verify parameters
   //-------------------------------------------------------------------------
   debugf("\n");
   debugf("%10.4f = Controls.cullProbability\n", cullProb);
   debugf("%10.4f = Controls.changeProbability*100.0\n", changeProb*100.0);
   debugf("%10.4f = Controls.mutateProbability\n", mutateProb);
   debugf("%10.4f = Controls.interestRate\n", global.interestRate);
   debugf("%10u = Controls.initialBalance\n", global.initialBalance);
   debugf("%10u = Controls.minimumBalance\n", global.minimumBalance);
   debugf("%10u = Controls.transferFee\n", global.transferFee);
   debugf("%10u = Debugging.randomize\n", global.seedControl);
   debugf("%10u = Debugging.re-evaluate\n", global.revalControl);
   debugf("%10u = Debugging.trace\n",       global.traceControl);
   debugf("%10u = History.minIndex\n", histIndex0);
   debugf("%10u = History.maxIndex\n", histIndexN);
   debugf("%10u = History.useIndex\n", histIndexU);

   global.dailyInterest= global.interestRate / 365.25;
   plex.probCull= cullProb;
   plex.probMute= mutateProb;
   global.changeProb= changeProb;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       normalize
//
// Purpose-
//       Normalize a value
//
//----------------------------------------------------------------------------
static double                       // Resultant
   normalize(                       // Normalize a value
     double          value,         // Vnnormalized value
     double          minValue,      // Minimum value
     double          maxValue)      // Maximum value
{
   return (value - minValue)/(maxValue-minValue);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       loadHistory
//
// Purpose-
//       Load the history file.
//
//----------------------------------------------------------------------------
static void
   loadHistory( void )              // Initialize the history data
{
   FileWriter        histOut;       // History output file
   LineReader        histFile("pgm.inp"); // History data file
   int               c;             // Current character

   unsigned          index;         // History index
   int               date;          // Date
   int               yyyy, mm, dd;  // Date components
   int               dow;           // Day of week

   int               rc;

   //-------------------------------------------------------------------------
   // Load the history file
   //-------------------------------------------------------------------------
   rc= histFile.open("pgm.inp", LineReader::MODE_READ);
   if( rc != 0 )
   {
     fprintf(stderr, "Unable to open history file\n");
     exit(EXIT_FAILURE);
   }

   debugf("\n");
   debugf("Loading history files...");
   if( swHist )
     histOut.open("PGM.OUT", FileWriter::MODE_WRITE);

   skipLine(histFile);
   for(index=0; index<DIM_HIST; index++)
   {
     date= toNumber(histFile);

     yyyy= date / 10000;
     mm=   date % 10000;
     dd=   mm   % 100;
     mm=   mm   / 100;

     histJulian[index]= julian(yyyy,mm,dd);
     if( index > 1 && histJulian[index] <= histJulian[index-1] )
     {
       badData("pgm.inp", histFile, date, "Date out of order");
     }
     dow= histJulian[index] % 7;
     if( dow > 4 )
     {
       badData("pgm.inp", histFile, date, "Market open on weekend");
     }

     skipBlank(histFile);
     histPrice[index]= normalize(toNumber(histFile),
                                 MINPRICEVALUE, MAXPRICEVALUE);

     skipBlank(histFile);
     histVolume[index]= normalize(toNumber(histFile),
                                  MINVOLUMEVALUE, MAXVOLUMEVALUE);

     if( swHist )
     {
       histOut.printf("[%5d] %8d %.6g %.6g\n",
                      index, date,
                      histPrice[index], histVolume[index]);
     }

     c= skipLine(histFile);
     if( c == EOF )
       break;
   }
   if( index == DIM_HIST )
   {
     fprintf(stderr, "DIM_HIST too small\n");
     exit(EXIT_FAILURE);
   }
   printf("done(%d)\n", index);

   // Set the next day in the history, an input parameter
   if( (histJulian[index]%7) == 4 )
     histJulian[index]= histJulian[index-1]+3;
   else
     histJulian[index]= histJulian[index-1]+1;

   #if 0
     for(i=0; i<index; i++)
     {
       printf("[%5d] (%10d) %8f %8f\n",
              i, histJulian[i], histPrice[i], histVolume[i]);
     }
     exit(EXIT_SUCCESS);
   #endif

   if( histIndexN > 0 )
   {
     histIndexN= index - histIndexN;
     if( histIndexU > 0 )
     {
       if( histIndex0 > 0 )
       {
         fprintf(stderr, "Cannot specify minIndex, maxIndex and useIndex\n");
         exit(EXIT_FAILURE);
       }
       histIndex0= histIndexN - histIndexU;
     }
     else
       histIndex0= histIndex0 + Unit::minIndex;
   }
   else
   {
     histIndex0= histIndex0 + Unit::minIndex;
     histIndexN= index;
     if( histIndexU > 0 )
       histIndexN= histIndex0 + histIndexU;
   }

   if( histIndex0 > index )
     histIndex0= index;
   if( histIndexN > index )
     histIndexN= index;

   debugf("History: [0..%u[ [%u]..[%u] ]..%u]\n",
          Unit::minIndex, histIndex0, histIndexN, index);
   if( histIndex0 >= histIndexN )
   {
     fprintf(stderr, "!! No evaluation points\n");
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initFaninArray
//
// Purpose-
//       Initialize the Fanin array.
//
//----------------------------------------------------------------------------
static void
   initFaninArray( void )           // Initialize the Fanin array
{
   int               i;
   int               j;

   //-------------------------------------------------------------------------
   // Allocate the Fanin array
   //-------------------------------------------------------------------------
   l3ArrayF= new Fanin*[DIM_L3];
   for(i= 0; i<DIM_L3; i++)
     l3ArrayF[i]= new Fanin[DIM_INP];

   l2ArrayF= new Fanin*[DIM_L2];
   for(i= 0; i<DIM_L2; i++)
     l2ArrayF[i]= new Fanin[DIM_L3];

   l1ArrayF= new Fanin*[DIM_L1];
   for(i= 0; i<DIM_L1; i++)
     l1ArrayF[i]= new Fanin[DIM_L2];

   outArrayF= new Fanin*[DIM_OUT];
   for(i= 0; i<DIM_OUT; i++)
     outArrayF[i]= new Fanin[DIM_L1];

   //-------------------------------------------------------------------------
   // Initialize the Fanin array
   //-------------------------------------------------------------------------
   printf("\n");
   printf("Initialize Fanin pointers...");
   for(i=0; i<DIM_L3; i++)
   {
     l3ArrayN[i].setFanin(DIM_USED, l3ArrayF[i]);
     for(j=0; j<DIM_USED; j++)
     {
       l3ArrayF[i][j].set(&inpArrayN[j], 1.0);
     }
   }

   for(i=0; i<DIM_L2; i++)
   {
     l2ArrayN[i].setFanin(DIM_L3, l2ArrayF[i]);
     for(j=0; j<DIM_L3; j++)
     {
       l2ArrayF[i][j].set(&l3ArrayN[j], 1.0);
     }
   }

   for(i=0; i<DIM_L1; i++)
   {
     l1ArrayN[i].setFanin(DIM_L2, l1ArrayF[i]);
     for(j=0; j<DIM_L2; j++)
     {
       l1ArrayF[i][j].set(&l2ArrayN[j], 1.0);
     }
   }

   for(i=0; i<DIM_OUT; i++)
   {
     outArrayN[i].setFanin(DIM_L1, outArrayF[i]);
     for(j=0; j<DIM_L1; j++)
     {
       outArrayF[i][j].set(&l1ArrayN[j], 1.0);
     }
   }
   printf("done\n");

   //-------------------------------------------------------------------------
   // Restore the Fanin array
   //-------------------------------------------------------------------------
   for(i=0; i<DIM_UNIT; i++)
   {
     plex.setUnit(&unit[i]);
   }
   plex.restore();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initNeuronArray
//
// Purpose-
//       Initialize the Neuron array.
//
//----------------------------------------------------------------------------
static void
   initNeuronArray( void )          // Initialize the Neuron array
{
   //-------------------------------------------------------------------------
   // Allocate the Neuron array
   //-------------------------------------------------------------------------
   inpArrayN= new NeuronValue[DIM_INP];
   l3ArrayN=  new Neuron[DIM_L3];
   l2ArrayN=  new Neuron[DIM_L2];
   l1ArrayN=  new Neuron[DIM_L1];
   outArrayN= new Neuron[DIM_OUT];
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Test sigmoid function.
//
//----------------------------------------------------------------------------
static void
   test( void )                     // Test
{
#if 0
   NeuronValue       sourceN;

   Neuron            targetN;
   Fanin             targetF(&sourceN, 1.0);

   Value             value;
   Value             result;

   targetN.setFanin(1, &targetF);

   for(value= -10.0; value<=10.0; value+=1.0)
   {
     Neuron::globalClock++;
     sourceN.setValue(value);
     result= targetN.resolve();

     printf("%f= sigmoid(%f)\n", result, value);
   }

   printf("%ld Reads\n", Neuron::globalCount);
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize.
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   RNG.randomize();                 // Default, random seed
   loadParameters();                // Load the parameter file
   loadHistory();                   // Load the history file
   initNeuronArray();               // Initialize the Neuron array
   initFaninArray();                // Initialize the Fanin array
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               error;         // TRUE if error encountered
   int               i, j;          // General index variables

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   swHist= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
       {
         switch(argv[j][i])         // Examine the switch
         {
           case 'h':                // -h (history)
             swHist= TRUE;
             break;
           default:                 // If invalid switch
             error= TRUE;
             fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
             break;
         }
       }

       continue;
     }
     else
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter '%s'\n", argv[j]);
     }
   }

   if( error )
     exit(EXIT_FAILURE);
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
int                                 // Return coe
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   Unit*             ptrUnit;
   Julian            tod;

   int               i;

   //-------------------------------------------------------------------------
   // Version
   //-------------------------------------------------------------------------
   debugf("Neural Net analyzer, Date(%s) Time(%s)\n",
          __DATE__, __TIME__);

   //-------------------------------------------------------------------------
   // Sizes
   //-------------------------------------------------------------------------
   #if 0
     printf("%10ld= Per Fanin\n",   (long)sizeof(Fanin));
     printf("%10ld= Per Neuron\n",  (long)sizeof(Neuron));
     printf("%10ld= Per Unit\n",    (long)sizeof(Unit));

     printf("\n");
     printf("%10ld= Fanins\n",  (long)sizeof(Fanin)  * FANIN_COUNT);
     printf("%10ld= Neurons\n", (long)sizeof(Neuron) * NEURON_COUNT);
     printf("%10ld= Units\n",   (long)sizeof(Unit)   * DIM_UNIT);
     printf("---------- ----------\n");
     printf("%10ld= Total\n",   (long)sizeof(Unit)   * DIM_UNIT);
   #endif

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   parm(argc, argv);
   init();
   test();

   //-------------------------------------------------------------------------
   // Train
   //-------------------------------------------------------------------------
   tod= Julian::current();          // Set the current time
   debugf("\n");
   debugf("Date(%10lu) Time(%10lu)\n", (long)tod.getDate(), (long)getTime(tod));
   for(;;)
   {
     plex.evaluate();

     debugf("\nGeneration(%lu)\n", (unsigned long)plex.getGeneration());
     for(i=0; i<DIM_UNIT; i++)
     {
       ptrUnit= (Unit*)plex.getUnit(i)->castConcrete();
       debugf("[%2d] (%4lu) ", i, plex.getGeneration() - ptrUnit->generation);
       ptrUnit->showRule();
     }

     global.savedSeed= RNG.get();
     plex.generate();
     plex.backup();
     debugFlush();
   }

   return 0;
}

