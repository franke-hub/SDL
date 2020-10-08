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
//       Main.cpp
//
// Purpose-
//       Mainline control.
//
// Last change date-
//       2007/01/01
//
// Working notes-
//         1 M  Neurons
//       100    Dendrites/Neuron
//       200 MB Budget
//
//        16 MB Neurons   (16B/Neuron)
//       200 MB Dendrites (100/Neuron, 2/Dendrite)
//         1 MB BitMap    (1/Neuron)
//         1 MB Code
//       ---
//       218 MB
//
// Working notes, problem:
//     Critical values:
//             Number of cycles that a Neuron is ON
//             Number of inputs required to set a Neuron
//
//     If too low, the number found rapidly drops to zero and stays zero.
//     If too high, the number found in step i+1 is the number Reset in i.
//     (There is no variation.)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Bit.h>
#include <com/Interval.h>
#include <com/Random.h>
#include <com/Unconditional.h>

#include "Allocator.h"              // (Compile-only test)
#include "Dendrite.h"
#include "Master.h"
#include "Neuron.h"
#include "Synapse.h"
#include "SynapseBundle.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Master          masterData;  // Master data area

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Master*                master= &masterData; // -> Master data area

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
// Function prototypes
//----------------------------------------------------------------------------
extern void parm(int,char*[]);      // Parameter analysis

//----------------------------------------------------------------------------
//
// Subroutine-
//       bitString
//
// Purpose-
//       Return the associated bit string
//
//----------------------------------------------------------------------------
static inline char*                 // The associated bit string
   bitString(                       // Get associated bit string
     int               index)       // For this index
{
   static char result[16];          // The resultant string

   int mask= 0x00000080;            // Bit mask
   for(int i=0; i<8; i++)           // Set resultant string
   {
     result[i]= '0';
     if( (index&mask) != 0 )
       result[i]= '1';

     mask >>= 1;
   }
   result[8]= '\0';

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bitCount
//
// Purpose-
//       Return number of bits in a value
//
//----------------------------------------------------------------------------
static inline int                   // The number of bits
   bitCount(                        // Get number of bits in an index
     int               index)       // For this index
{
   int result= 0;
   while( index != 0 )
   {
     if( (index&1) != 0 )
       result++;

     index >>= 1;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       generateBitCount
//
// Purpose-
//       Generate the index to bit count array for Synapse.cpp
//
//----------------------------------------------------------------------------
static inline void
   generateBitCount( void )         // Generate the index to bit count array
{
   printf("{  0 //   0 00000000\n"); // (Hard-coded first line)
   for(int i= 1; i<256; i++)
   {
     printf(",  %d // %3d %s\n", bitCount(i), i, bitString(i));
   }
   printf("};\n");                  // (Hard-coded last line)
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       release
//
// Purpose-
//       Release storage, allowing NULL address.
//
//----------------------------------------------------------------------------
static inline void
   release(                         // Release storage
     void*             addr)        // Storage address
{
   if( addr != NULL )
     free(addr);
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
   unsigned            size;        // Allocation length
   unsigned            total;       // Allocation length

   unsigned            i;

   // Initialize
   total= 0;

   // Allocate and initialize Axon array
   size= (master->aCount+7)/8;
   printf("%4d: %10u Axon array\n", __LINE__, size);
   master->axon= (char*)must_malloc(size);
   memset(master->axon, 0, size);
   total += size;

   // Allocate and initialize Dendrite array
   size= master->dCount*sizeof(Dendrite);
   printf("%4d: %10u Dendrite array\n", __LINE__, size);
   master->dendrite= (Dendrite*)must_malloc(size);
   for(i= 0; i<master->dCount; i++)
     master->dendrite[i].next= RNG.get();
   total += size;

   // Allocate and initialize Neuron array
   size= master->nCount*sizeof(Neuron);
   printf("%4d: %10u Neuron array\n", __LINE__, size);
   master->neuron= (Neuron*)must_malloc(size);
   for(i= 0; i<master->nCount; i++)
   {
     master->neuron[i].cycle= RNG.get() % Neuron::maxCycle;
     master->neuron[i].prior= RNG.get() % 256;
   }
   total += size;

   // Summary display
   printf("%4d: %10u Total\n", __LINE__, total);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       updateAxon
//
// Purpose-
//       Update the Axon array.
//
//----------------------------------------------------------------------------
static int                          // Number of active axons
   updateAxon( void )               // Update the Axon array
{
   unsigned            const iCount= master->iCount;
   unsigned            minTrigger=   Neuron::minTrigger;
   unsigned            maxTrigger=   Neuron::maxTrigger;

   unsigned            cycle;
   int                 triggered;
   int                 total;

   unsigned            i;

   total= 0;
   for(i= 0; i<master->nCount; i++)
   {
     cycle= master->neuron[i].cycle;
     triggered= (cycle >= minTrigger && cycle < maxTrigger);
     Bit::set(master->axon, iCount+i, triggered);
     total += triggered;
   }

   return total;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       updateNeuron
//
// Purpose-
//       Update the Neuron array.
//
//----------------------------------------------------------------------------
static void
   updateNeuron(                    // Update the Neuron array
     unsigned          active)      // Number of active axons
{
   const char*         const aArray= master->axon;
   unsigned            const aCount= master->aCount;
   const Dendrite*     const dArray= master->dendrite;
   unsigned            const dCount= master->dPerN;
   unsigned            const nCount= master->nCount;
   Neuron*             const neuron= master->neuron;
   unsigned            const load=   Neuron::load;

   unsigned            tCount;      // Trigger counter
   unsigned            tTotal;      // Trigger counter
   unsigned            rTotal;      // Trigger counter
   unsigned            prior;       // Prior trigger counter
   unsigned            delta;       // Prior trigger counter delta

   unsigned            dFirst;      // First dendrite index
   unsigned            dFinal;      // Final dendrite index

   unsigned            aIndex;      // Working Axon index
   unsigned            dIndex;      // Working dendrite index
   unsigned            nIndex;      // Working Neuron index

   rTotal= 0;
   tTotal= 0;
   for(nIndex= 0; nIndex<nCount; nIndex++)
   {
     if( neuron[nIndex].cycle != 0 )
     {
       neuron[nIndex].cycle++;
       if( neuron[nIndex].cycle >= Neuron::maxCycle )
       {
         rTotal++;
         neuron[nIndex].cycle= 0;
// printf("%4d: Reset(%d)\n", __LINE__, nIndex);
       }
     }
     else
     {
       tCount= 0;
       dFirst= dCount * nIndex;
       dFinal= dFirst + dCount;
       aIndex= nIndex + 1;
       for(dIndex= dFirst; dIndex<dFinal; dIndex++)
       {
         while(aIndex >= aCount)
           aIndex -= aCount;

         tCount += Bit::get(aArray, aIndex);
         aIndex += dArray[dIndex].next;
       }

       if( tCount > 255 )
         tCount= 255;

       prior= neuron[nIndex].prior;
       if( tCount > prior )
         delta= tCount - prior;
       else
         delta= prior - tCount;

       if( delta < load )
         neuron[nIndex].prior++;    // This insures continuous activity
       else
       {
         tTotal++;
         neuron[nIndex].cycle= 1;
         neuron[nIndex].prior= tCount;
// if( master->cycle > 0 )
// printf("%4d: Found(%d)\n", __LINE__, nIndex);
       }
     }
   }

   printf("%4d: %8lu Active(%8u) Found(%8u) Reset(%8u)\n", __LINE__,
          master->cycle, active, tTotal, rTotal);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0000
//
// Purpose-
//       Axon/Dendrite/Neuron test
//
//----------------------------------------------------------------------------
static int                          // Return code
   test0000(                        // Axon/Dendrite/Neuron test
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 active;      // Number of active axons

   parm(argc, argv);
   init();
   printf("Init complete\n");

   for(master->cycle= 0; master->cycle<1000; master->cycle++)
   {
     active= updateAxon();
     updateNeuron(active);
   }
   printf("\n");

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0001
//
// Purpose-
//       Synapse test (Hand-verification)
//
//----------------------------------------------------------------------------
static int                          // Return code
   test0001(                        // Synapse test
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   (void)argc; (void)argv;          // (Parameters ignored)
   const int INPS= 1024;            // Number of inputs
   const int OUTS= INPS/4;          // Number of outputs

   Synapse synapse(INPS, OUTS);     // Synapse block

   if( true  )                      // Set/Clear the array
   {
     // Randomly set array
     srand(128);                    // Constant random seed
     for(int x= 0; x<((INPS*OUTS)/5); x++) // Set 1/5 of array
     {
       int i= (rand() % INPS);
       int o= (rand() % OUTS);
       synapse.enable(i, o);
     }
   }
   else
   {
     // Randomly clear array
     for(int i= 0; i<INPS; i++)       // Set the entire array
       for(int o= 0; o<OUTS; o++)
         synapse.enable(i, o);

     srand(128);                      // Constant random seed
     for(int x= 0; x<((INPS*OUTS)*4/5); x++) // Clear 4/5 of array
     {
       int i= (rand() % INPS);
       int o= (rand() % OUTS);
       synapse.disable(i, o);
     }
   }

   // Set triggers/leakages
   for(int n= 0; n<OUTS; n++)
   {
     int inputs= synapse.getBits(n);
     synapse.setTrig(n, inputs/3 + 1);
     synapse.setLeak(n, inputs/15);
   }

   // Randomly set 1/6 inputs
   unsigned char* inps= synapse.getInps();
   unsigned char* sets= synapse.getSets();
   unsigned char* leak= synapse.getLeak();
   unsigned char* trig= synapse.getTrig();
   unsigned char* rems= synapse.getRems();
   unsigned char* outs= synapse.getOuts();
   for(int i= 0; i<INPS; i++)
   {
     if( (rand() % 6) == 0 )
     {
       int byteIndex= (i >> 3);
       int bitsIndex= (i & 7);
       int mask= (0x00000080 >> bitsIndex);
       inps[byteIndex] |= mask;
     }
   }

   // Get outputs
   for(int x= 0; x<4; x++)
   {
     synapse.update();

     // Display outputs
     printf("Out[***]: ");
     for(int i= 0; i<(OUTS/8); i++)
       printf("%.2x", outs[i]);
     printf("\n");

     // Display inputs
     printf("Inp[***]: ");
     for(int i= 0; i<(INPS/8); i++)
       printf("%.2x", inps[i]);
     printf("\n");

     // Display sets and outputs
     sets= synapse.getSets();
     printf("Get[%.3d]: ", 0);
     for(int i=0; i<(INPS/8); i++)
       printf("%.2x", sets[i]&inps[i]);
     printf("= %3d\n", synapse.evaluate(0));

     for(int n= 0; n<OUTS; n++)
     {
       sets= synapse.getSets(n);

       printf("Set[%.3d]: ", n);
       for(int i=0; i<(INPS/8); i++)
         printf("%.2x", sets[i]);

       int byteIndex= (n >> 3);
       int bitsIndex= (n & 7);
       int mask= (0x00000080 >> bitsIndex);
       mask &= outs[byteIndex];
       printf("= %2d %2d %2d %2d [%s]\n", synapse.getBits(n),
              trig[n], leak[n], rems[n], (mask != 0 ? "set" : "clr"));
     }

     printf("\n\n");
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0002
//
// Purpose-
//       Synapse timing test
//
//----------------------------------------------------------------------------
static int                          // Return code
   test0002(                        // Synapse timing test
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   (void)argc; (void)argv;          // (Parameters ignored)
   const int INPS= 1024;            // Number of inputs
   const int OUTS= INPS/4;          // Number of outputs
   const int OtoI= (OUTS/8);        // Number of bytes to copy OUT to INP

   Synapse inp000(INPS, OUTS);      // Synapse blocks
   Synapse inp001(INPS, OUTS);
   Synapse inp002(INPS, OUTS);
   Synapse inp003(INPS, OUTS);
   Synapse out000(INPS, OUTS);

   // Randomly set array
   srand(128);                      // Constant random seed
   for(int x= 0; x<((INPS*OUTS)/5); x++) // Set 1/5 of array
   {
     int i= (rand() % INPS);
     int o= (rand() % OUTS);
     inp000.enable(i, o);
     inp001.enable(i, o);
     inp002.enable(i, o);
     inp003.enable(i, o);
     out000.enable(i, o);
   }

   // Set triggers/leakages
   for(int n= 0; n<OUTS; n++)
   {
     const int NUM= 100;
     const int DEM= 909;

     int inputs= out000.getBits(n);
     inp000.setTrig(n, inputs/3 + 1);
     inp000.setLeak(n, (inputs*NUM)/DEM + 0);
     inp001.setTrig(n, inputs/3 + 1);
     inp001.setLeak(n, (inputs*NUM)/DEM + 1);
     inp002.setTrig(n, inputs/3 + 1);
     inp002.setLeak(n, (inputs*NUM)/DEM + 0);
     inp003.setTrig(n, inputs/3 + 1);
     inp003.setLeak(n, (inputs*NUM)/DEM + 1);
     out000.setTrig(n, inputs/3 + 1);
     out000.setLeak(n, (inputs*NUM)/DEM + 0);
   }

   // Randomly set 1/9 inputs
   unsigned char* inps= inp000.getInps();
   unsigned char* sets= out000.getSets();
   unsigned char* leak= out000.getLeak();
   unsigned char* trig= out000.getTrig();
   unsigned char* rems= out000.getRems();
   unsigned char* outs= out000.getOuts();
   for(int i= 0; i<INPS; i++)
   {
     if( (rand() % 9) <= 0 )
     {
       int byteIndex= (i >> 3);
       int bitsIndex= (i & 7);
       int mask= (0x00000080 >> bitsIndex);
       inps[byteIndex] |= mask;
     }
   }

   memcpy(inp001.getInps(), inps, INPS/8);
   memcpy(inp002.getInps(), inps, INPS/8);
   memcpy(inp003.getInps(), inps, INPS/8);

   // Run timing test
   Interval interval;
   interval.start();

   int ITERATIONS= 10000;
   for(int x= 0; x<ITERATIONS; x++)
   {
     inp000.update();
     inp001.update();
     inp002.update();
     inp003.update();

     memcpy(out000.getInps()+0*OtoI, inp000.getOuts(), OtoI);
     memcpy(out000.getInps()+1*OtoI, inp001.getOuts(), OtoI);
     memcpy(out000.getInps()+2*OtoI, inp002.getOuts(), OtoI);
     memcpy(out000.getInps()+3*OtoI, inp003.getOuts(), OtoI);
     out000.update();
   }

   interval.stop();
// printf("%9.3f seconds\n", interval.toDouble());
   printf("%9.3f updates/second\n", 5.0*ITERATIONS/interval.toDouble());

   // Display outputs
   outs= out000.getOuts();
   printf("Out[***]: ");
   for(int i= 0; i<(OUTS/8); i++)
     printf("%.2x", outs[i]);
   printf("\n");

   // Display inputs
   inps= out000.getInps();
   printf("Inp[***]: ");
   for(int i= 0; i<(INPS/8); i++)
     printf("%.2x", inps[i]);
   printf("\n");

   // Display sets and outputs
   sets= out000.getSets();
   printf("Get[%.3d]: ", 0);
   for(int i=0; i<(INPS/8); i++)
     printf("%.2x", sets[i]&inps[i]);
   printf("= %3d\n", out000.evaluate(0));

   for(int n= 0; n<OUTS; n++)
   {
     sets= out000.getSets(n);

     printf("Set[%.3d]: ", n);
     for(int i=0; i<(INPS/8); i++)
       printf("%.2x", sets[i]);

     int byteIndex= (n >> 3);
     int bitsIndex= (n & 7);
     int mask= (0x00000080 >> bitsIndex);
     mask &= outs[byteIndex];
     printf("= %3d %2d %2d %2d [%s]\n", out000.getBits(n),
            trig[n], leak[n], rems[n], (mask != 0 ? "set" : "clr"));
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0003
//
// Purpose-
//       Synapse timing test, large number of Neurons
//
// Implementation notes-
//       With MEMCPY == 1, Synapses are either all ones or all zeros.
//       (The trigger, leakage, and input setting ratios are critical.)
//
//----------------------------------------------------------------------------
static int                          // Return code
   test0003(                        // Synapse timing test, large Neuron count
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
#undef  MEMCPY
#define MEMCPY 0                    // [[ Highly unstable ]]

   (void)argc; (void)argv;          // (Parameters ignored)
   const int SIZE= 3000;            // Number of blocks
   const int INPS= 1024;            // Number of inputs
   const int OUTS= INPS/4;          // Number of outputs
#if( MEMCPY )
   const int OtoI= (OUTS/8);        // Number of bytes to copy OUT to INP
#endif

   Synapse** synapse= NULL;         // Synapse table

   synapse= (Synapse**)malloc(sizeof(Synapse*)*SIZE);
   memset(synapse, 0, sizeof(Synapse*)*SIZE);
   for(int s= 0; s<SIZE; s++)
     synapse[s]= new Synapse(INPS, OUTS);

   // Randomly enable synapse[0] set array
   srand(128);                      // Constant random seed
   for(int n= 0; n<((INPS*OUTS)/5); n++) // Enable 1/5 of total array
   {
     int s= 0;                      // Allows next line to be commented out
//   for(s= 0; s<SIZE; s++)         // INVERSION of below
     {
       int i= (rand() % INPS);
       int o= (rand() % OUTS);
       synapse[s]->enable(i, o);
     }
   }

   // Set triggers/leakages
   for(int n= 0; n<OUTS; n++)
   {
     // With memcpy, overly critical: Resultant with 10/90 inputs set:
     //  00..00   ff..ff  00..00
     // 100/300, 100/300 101/300
     //  85/903,  85/904  85/904
     int inputs= synapse[0]->getBits(n);
     synapse[0]->setTrig(n, (inputs*100)/300 + 1 + (n&1));
     synapse[0]->setLeak(n, (inputs*85)/904 + 0);
   }

#if( MEMCPY )
   // Set triggers/leakages [EXPERIMENTAL]
   for(int n= 0; n<OUTS; n++)
   {                                                        //  00..00;  ff..ff
     int inputs= synapse[0]->getBits(n);                    // 0.14634; 0.14618
     synapse[0]->setTrig(n, (inputs* 132)/902 + 1 + (n&1)); // 132/902; 132/903
     synapse[0]->setLeak(n, 28 + (n&1));                    //      29;      28
   }

   // Set random weights [EXPERIMENTAL]
   for(int x= 0; x<INPS; x+=8)
   {
     for(int s= 0; s<SIZE; s++)
     {
       int weight= (rand() % 4) - 1;
       if( weight == 0 )
         weight= 1;

       synapse[s]->setWeight(x, weight);
     }
   }
#endif

   // Copy synapse data
   unsigned char* sets= synapse[0]->getSets();
   unsigned char* leak= synapse[0]->getLeak();
   unsigned char* trig= synapse[0]->getTrig();
   for(int s= 1; s<SIZE; s++)
   {
     memcpy(synapse[s]->getSets(), sets, (INPS*OUTS)/8); // INVERSION of above
     memcpy(synapse[s]->getLeak(), leak, OUTS);
     memcpy(synapse[s]->getTrig(), trig, OUTS);
   }

   // Randomly set 11/90 inputs
   for(int s= 0; s<SIZE; s++)
   {
     unsigned char* inps= synapse[s]->getInps();
     for(int i= 0; i<INPS; i++)
     {
       if( (rand() % 90) < 11 )
       {
         int byteIndex= (i >> 3);
         int bitsIndex= (i & 7);
         int mask= (0x00000080 >> bitsIndex);
         inps[byteIndex] |= mask;
       }
     }

#if( MEMCPY )
     synapse[s]->update();          // Initialize outputs
     synapse[s]->update();
#endif
   }

   // Run timing test
   Interval interval;
   interval.start();

#if( MEMCPY )
   int ITERATIONS= 10;
#else
   int ITERATIONS= 100;
#endif
   for(int iteration= 0; iteration<ITERATIONS; iteration++)
   {
     for(int s= 0; s<4; s++)
     {
#if( MEMCPY )
//     memcpy(synapse[s]->getInps()+0*OtoI, synapse[SIZE-1-s]->getOuts(), OtoI);
//     memcpy(synapse[s]->getInps()+1*OtoI, synapse[SIZE-2-s]->getOuts(), OtoI);
//     memcpy(synapse[s]->getInps()+2*OtoI, synapse[SIZE-3-s]->getOuts(), OtoI);
//     memcpy(synapse[s]->getInps()+3*OtoI, synapse[SIZE-4-s]->getOuts(), OtoI);

       // Randomly change first input group
       if( (iteration % 3) == 0 )   // Occasionally, notably first time
       {
         // Randomly set xx/yy inputs
         unsigned char* inps= synapse[s]->getInps();
         for(int i= 0; i<INPS; i++)
         {
           //
           //  00..00;  ff..ff
           //   6/900;   7/900
           if( (rand() % 900) <  7 )
           {
             int byteIndex= (i >> 3);
             int bitsIndex= (i & 7);
             int mask= (0x00000080 >> bitsIndex);
             inps[byteIndex] |= mask;
           }
         }
       }
#endif

       synapse[s]->update();
     }

     #if( 0 )                       // Debugging hook
       if( iteration == 0 )
       {
         printf("HCDM: Iteration(%d)\n", iteration);
         int s= 0;                  // Select synapse
         int n= 27;                 // Select neuron

         unsigned char* inps= synapse[s]->getInps();
         unsigned char* rems= synapse[s]->getRems();
         unsigned char* outs= synapse[s]->getOuts();

         // Display inputs
         printf("HCDM: Inp[%4d][***]: ", s);
         for(int i= 0; i<(INPS/8); i++)
           printf("%.2x", inps[i]);
         printf("\n");

         // Display sets
         sets= synapse[s]->getSets(n);
         printf("HCDM: Set[%4d][%3d]: ", s, n);
         for(int i= 0; i<(INPS/8); i++)
           printf("%.2x", sets[i]);

         int byteIndex= (n >> 3);
         int bitsIndex= (n & 7);
         int mask= (0x00000080 >> bitsIndex);
         mask &= outs[byteIndex];
         printf("= %3d %2d %2d %2d [%s]\n", synapse[s]->getBits(n),
                trig[n], leak[n], rems[n], (mask != 0 ? "set" : "clr"));

         // Display outputs
         printf("HCDM: Out[%4d][***]: ", s);
         for(int i= 0; i<(OUTS/8); i++)
           printf("%.2x", outs[i]);
         printf("\n");

         // Display outputs
         s= SIZE-1;
         outs= synapse[s]->getOuts();
         printf("HCDM: Out[%4d][***]: ", s);
         for(int i= 0; i<(OUTS/8); i++)
           printf("%.2x", outs[i]);
         printf("\n");
       }
     #endif

     for(int s= 4; s<SIZE; s++)
     {
#if( MEMCPY )
       memcpy(synapse[s]->getInps()+0*OtoI, synapse[s-4]->getOuts(), OtoI);
       memcpy(synapse[s]->getInps()+1*OtoI, synapse[s-3]->getOuts(), OtoI);
       memcpy(synapse[s]->getInps()+2*OtoI, synapse[s-2]->getOuts(), OtoI);
       memcpy(synapse[s]->getInps()+3*OtoI, synapse[s-1]->getOuts(), OtoI);
#endif

       synapse[s]->update();
     }
   }

   interval.stop();
// printf("%9.3f seconds\n", interval.toDouble());
   printf("%9.3f updates/second\n", (SIZE*ITERATIONS)/interval.toDouble());

   // Display outputs
   unsigned char* inps= synapse[SIZE-1]->getInps();
   unsigned char* rems= synapse[SIZE-1]->getRems();
   unsigned char* outs= synapse[SIZE-1]->getOuts();

   printf("Out[***]: ");
   for(int i= 0; i<(OUTS/8); i++)
     printf("%.2x", outs[i]);
   printf("\n");

   // Display inputs
   printf("Inp[***]: ");
   for(int i= 0; i<(INPS/8); i++)
     printf("%.2x", inps[i]);
   printf("\n");

   // Display sets and outputs
   sets= synapse[SIZE-1]->getSets();
   printf("Get[%.3d]: ", 0);
   for(int i=0; i<(INPS/8); i++)
     printf("%.2x", inps[i]&sets[i]);
   printf("= %3d\n", synapse[SIZE-1]->evaluate(0));

   for(int n= 0; n<OUTS; n++)
   {
     sets= synapse[SIZE-1]->getSets(n);

     printf("Set[%.3d]: ", n);
     for(int i=0; i<(INPS/8); i++)
       printf("%.2x", sets[i]);

     int byteIndex= (n >> 3);
     int bitsIndex= (n & 7);
     int mask= (0x00000080 >> bitsIndex);
     mask &= outs[byteIndex];
     printf("= %3d %2d %2d %2d [%s]\n", synapse[SIZE-1]->getBits(n),
            trig[n], leak[n], rems[n], (mask != 0 ? "set" : "clr"));
   }

   // Deallocate storage
   for(int x= 0; x<SIZE; x++)
     delete synapse[x];
   free(synapse);

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0004
//
// Purpose-
//       Synapse timing test, large Neuron count
//
// Implementation notes-
//       EXPERIMENTAL, MEMCPY ONLY
//
//----------------------------------------------------------------------------
static int                          // Return code
   test0004(                        // Synapse timing test, large Neuron count
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   (void)argc; (void)argv;          // (Parameters ignored)
   const int INPS= 1024;            // Number of inputs
   const int OUTS= INPS/4;          // Number of outputs

   unsigned            bx0;         // B[0] index
   unsigned            bx1;         // B[1] index
   unsigned            bx2;         // B[2] index
   unsigned            bx3;         // B[3] index

   Synapse*            sp0;         // B[0] Synapse*
   Synapse*            sp1;         // B[1] Synapse*
   Synapse*            sp2;         // B[2] Synapse*
   Synapse*            sp3;         // B[3] Synapse*

   unsigned int        inpX;        // Input index
   unsigned int        outX;        // Output index

   Synapse             base(INPS, OUTS); // Reference block
   SynapseBundle       b0(  32, INPS, OUTS);
   SynapseBundle       b1(1024, INPS, OUTS);
   SynapseBundle       b2(1024, INPS, OUTS);
   SynapseBundle       b3(  32, INPS, OUTS);
   const int SIZE= 32 + 1024 + 1024 + 32; // Number of Synapses

   // Randomly enable synapse[0] set array
   srand(128);                      // Constant random seed
   for(int n= 0; n<((INPS*OUTS)/5); n++) // Enable 1/5 of total array
   {
     int i= (rand() % INPS);
     int o= (rand() % OUTS);
     base.enable(i, o);
   }

   // Set triggers/leakages
   for(int n= 0; n<OUTS; n++)
   {
     // With memcpy, overly critical: Resultant with 10/90 inputs set:
     //  00..00   ff..ff  00..00
     // 100/300, 100/300 101/300
     //  85/903,  85/904  85/904
     int inputs= base.getBits(n);
     base.setTrig(n, (inputs*100)/300 + 1 + (n&1));
     base.setLeak(n, (inputs*85)/904 + 0);
   }

   // Copy synapse data
   unsigned char* sets= base.getSets();
   unsigned char* leak= base.getLeak();
   unsigned char* trig= base.getTrig();
   for(bx0= 0; bx0<b0.getBCount(); bx0++)
   {
     sp0= b0.getSynapse(bx0);
     memcpy(sp0->getSets(), sets, (INPS*OUTS)/8);
     memcpy(sp0->getLeak(), leak, OUTS);
     memcpy(sp0->getTrig(), trig, OUTS);
   }

   for(bx1= 0; bx1<b1.getBCount(); bx1++)
   {
     sp1= b1.getSynapse(bx1);
     memcpy(sp1->getSets(), sets, (INPS*OUTS)/8);
     memcpy(sp1->getLeak(), leak, OUTS);
     memcpy(sp1->getTrig(), trig, OUTS);
   }

   for(bx2= 0; bx2<b2.getBCount(); bx2++)
   {
     sp2= b2.getSynapse(bx2);
     memcpy(sp2->getSets(), sets, (INPS*OUTS)/8);
     memcpy(sp2->getLeak(), leak, OUTS);
     memcpy(sp2->getTrig(), trig, OUTS);
   }

   for(bx3= 0; bx3<b3.getBCount(); bx3++)
   {
     sp3= b3.getSynapse(bx3);
     memcpy(sp3->getSets(), sets, (INPS*OUTS)/8);
     memcpy(sp3->getLeak(), leak, OUTS);
     memcpy(sp3->getTrig(), trig, OUTS);
   }

   // Run timing test
   Interval interval;
   interval.start();

   int ITERATIONS= 10;            // 9= 00; 10=ff..00..ff; 11= ff
   for(int iteration= 0; iteration<ITERATIONS; iteration++)
   {
     // (Occasionally) set INPUTS
     if( (iteration % 3) == 0 )   // (Notably, set first time through)
     {
       for(bx0= 0; bx0<b0.getBCount(); bx0++)
       {
         // Randomly set xx/yy inputs
         unsigned char* inps= b0.getSynapse(bx0)->getInps();
         for(int i= 0; i<INPS; i++)
         {
           // When ITERATIONS == 10
           //   00..00;  ff..00..ff;  ff..ff
           //   42/900;      43/900;  44/900
           if( (rand() % 900) < 43 )
           {
             int byteIndex= (i >> 3);
             int bitsIndex= (i & 7);
             int mask= (0x00000080 >> bitsIndex);
             inps[byteIndex] |= mask;
           }
         }
       }
     }

     // INPUT=>b0
     b0.update();

     // b0=>b1
     bx0= 0;
     bx1= 0;
     sp0= b0.getSynapse(bx0++);
     outX= 0;
     while( bx1 < b1.getBCount() )
     {
       sp1= b1.getSynapse(bx1++);
       inpX= 0;

       unsigned char* outs= sp0->getOuts();
       unsigned char* inps= sp1->getInps();
       while( inpX < (INPS/8) )     // Copy OUTS to INPS
       {
         if( outX >= (OUTS/8) )
         {
           if( bx0 >= b0.getBCount() )
             bx0= 0;

           sp0= b0.getSynapse(bx0++);
           outs= sp0->getOuts();
           outX= 0;
         }
         inps[inpX++]= outs[outX++];
       }
     }

     b1.update();

     // b1=>b2
     bx1= 0;
     bx2= 0;
     sp1= b1.getSynapse(bx1++);
     outX= 0;
     while( bx2 < b2.getBCount() )
     {
       sp2= b2.getSynapse(bx2++);
       inpX= 0;

       unsigned char* outs= sp1->getOuts();
       unsigned char* inps= sp2->getInps();
       while( inpX < (INPS/8) )     // Copy OUTS to INPS
       {
         if( outX >= (OUTS/8) )
         {
           if( bx1 >= b1.getBCount() )
             bx1= 0;

           sp1= b1.getSynapse(bx1++);
           outs= sp1->getOuts();
           outX= 0;
         }
         inps[inpX++]= outs[outX++];
       }
     }

     b2.update();

     // b2=>b3
     bx2= 0;
     bx3= 0;
     sp2= b2.getSynapse(bx2++);
     outX= 0;
     while( bx3 < b3.getBCount() )
     {
       sp3= b3.getSynapse(bx3++);
       inpX= 0;

       unsigned char* outs= sp2->getOuts();
       unsigned char* inps= sp3->getInps();
       while( inpX < (INPS/8) )     // Copy OUTS to INPS
       {
         if( outX >= (OUTS/8) )
         {
           if( bx2 >= b2.getBCount() )
             bx2= 0;

           sp2= b2.getSynapse(bx2++);
           outs= sp2->getOuts();
           outX= 0;
         }
         inps[inpX++]= outs[outX++];
       }
     }

     b3.update();
   }

   interval.stop();
// printf("%9.3f seconds\n", interval.toDouble());
   printf("%9.3f updates/second\n", (SIZE*ITERATIONS)/interval.toDouble());

   // Display sample output
   sp3= b3.getSynapse(b3.getBCount()-1);
   unsigned char* inps= sp3->getInps();
   unsigned char* rems= sp3->getRems();
   unsigned char* outs= sp3->getOuts();

   printf("Out[***]: ");
   for(int i= 0; i<(OUTS/8); i++)
     printf("%.2x", outs[i]);
   printf("\n");

   // Display final inputs
   printf("Inp[***]: ");
   for(int i= 0; i<(INPS/8); i++)
     printf("%.2x", inps[i]);
   printf("\n");

   // Display final Synapse sets and outputs
   sets= sp3->getSets();
   printf("Get[%.3d]: ", 0);
   for(int i=0; i<(INPS/8); i++)
     printf("%.2x", inps[i]&sets[i]);
   printf("= %3d\n", sp3->evaluate(0));

   for(int n= 0; n<OUTS; n++)
   {
     sets= sp3->getSets(n);

     printf("Set[%.3d]: ", n);
     for(int i=0; i<(INPS/8); i++)
       printf("%.2x", sets[i]);

     int byteIndex= (n >> 3);
     int bitsIndex= (n & 7);
     int mask= (0x00000080 >> bitsIndex);
     mask &= outs[byteIndex];
     printf("= %3d %2d %2d %2d [%s]\n", sp3->getBits(n),
            trig[n], leak[n], rems[n], (mask != 0 ? "set" : "clr"));
   }

   return 0;
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
   int result= 0;

   int testID= 4;                   // Test selector
   switch( testID )
   {
     case 0:
       result= test0000(argc, argv); // Axon/Dendrite/Neuron test
       break;

     case 1:
       result= test0001(argc, argv); // Synapse test
       break;

     case 2:
       result= test0002(argc, argv); // Synapse timing test
       break;

     case 3:
       result= test0003(argc, argv); // Synapse timing test, large array
       break;

     case 4:
       result= test0004(argc, argv); // Synapse timing test, large array
       break;

     default:
       generateBitCount();          // Generate Synapse.cpp bitCount
       break;
   }

   return result;
}

