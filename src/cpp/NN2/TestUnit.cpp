//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestUnit.cpp
//
// Purpose-
//       Configuration unit test.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include "com/Debug.h"
#include "com/Interval.h"
#include "com/Logger.h"
#include "com/Random.h"

#include "Neuron.h"
#include "Network.h"
#include "NetMiddle.h"
#include "NetRoot.h"
#include "NetVideo.h"
#include "NetCIFAR10.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define CIFAR10_SOURCE "data_batch_1.bin" // Default file name
#define DEFAULT_CYCLES 16           // Default cycle count

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     fileName= nullptr; // Source file name
static int             cycleCount= DEFAULT_CYCLES; // Iteration count
static struct timespec delay= {0, 250000000}; // Iteration delay

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "Usage: %s <options> {Image-set}\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "--cycles=n\tSet run cycle count\n");
   fprintf(stderr, "-v\tVerify parameters\n");
   exit(EXIT_FAILURE);
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
   bool              error;         // TRUE if error encountered
   bool              verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= false;                    // Default, no error found
   verify= false;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( argv[j][1] == '-' ) {    // --Parameter
         if( strcmp(argv[j], "--help") == 0 )
           error= true;
         else if( memcmp(argv[j]+2, "cycles=", 7) == 0 )
           cycleCount= atoi(argv[j]+9);
         else {
           error= true;
           fprintf(stderr, "Invalid control '%s'\n", argv[j]);
         }
       } else {                     // Switch list
         for(i=1; argv[j][i] != '\0'; i++)
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'h':              // -h (help)
               error= true;
               break;

             case 'v':              // -v (verify)
               verify= true;
               break;

             default:               // If invalid switch
               error= true;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else if( fileName != nullptr ) {
       error= true;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
     else
       fileName= argv[j];
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( fileName == nullptr )
     fileName= CIFAR10_SOURCE;      // Default source file name

   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "Cycle count: %d\n", cycleCount);
     fprintf(stderr, "File name: '%s'\n", fileName);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_unit
//
// Purpose-
//       Configuration unit test.
//
//----------------------------------------------------------------------------
static inline int                   // Error count
   test_unit( void )                // Configuration unit test
{  debugf("test_unit: Configuration unit test.\n");

   int error_count = 0;

{{{{
   NN::Root            root(3);     // The Root instance
   NN::VideoSourceCIFAR10
                       source(fileName); // CIFAR10 VideoSource

// // This Layer is not needed. It's here to verify the basic Layer
// NN::Layer*          outer= new NN::Layer(root);
// root.insert_layer(outer);

// NN::MiddleLayer*    middle= new NN::MiddleLayer(*outer);
   NN::MiddleLayer*    middle= new NN::MiddleLayer(root);
   middle->insert_layer(new NN::VideoInpCIFAR10(root, source));
   middle->insert_layer(new NN::FaninpNeuron(16, *middle));
   middle->insert_layer(new NN::FanoutNeuron(16, *middle));
   middle->insert_layer(new NN::FanoutNeuron(16, *middle));
   middle->insert_layer(new NN::FanoutNeuron(16, *middle));
   middle->insert_layer(new NN::FanoutNeuron(16, *middle));
   middle->insert_layer(new NN::OutBuffer(64, *middle));
// middle->insert_layer(new NN::VideoOut(8,8));
// outer->insert_layer(middle);     // Insert the (only) inner layer
   root.insert_layer(middle);       // Insert the (only) inner layer

   debugf("\n\nBUILD >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
   root.build(0);                   // Build the Root object

   bool retry= true;                // Update the build until complete
   for(int i= 0; retry; i++) {
     retry= false;
     debugf("\nBUILD_UPDATE(%d)>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", i);
     retry |= root.build_update(i);
   }

   debugf("\n\nBUILD_DEBUG >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
   root.build_debug();              // Debug the build

   for(int i= 0; i<cycleCount; i++) {
     debugf("\n\nCYCLE[%2d] >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n", i);
     root.update();
     if( true )
       middle->debug();
//// root.debug();

     nanosleep(&delay, nullptr);
   }

   root.debug();
   debugf(">>>> Running destructors\n");
}}}}

   debugf("Error count: %d\n", error_count);
   return error_count;
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int error_count= 0;

   Logger logger("debug.out");
   parm(argc, argv);

   try {
     error_count += test_unit();
   } catch(NN::NetworkException& X) {
     debugf("%s.what(%s)\n", X.get_class_name().c_str(), X.what());
     error_count += 1;
   } catch(std::exception& X) {
     debugf("std::exception.what(%s)\n", X.what());
     error_count += 1;
   } catch(const char* X) {
     debugf("catch(const char*(%s))\n", X);
     error_count += 1;
   } catch(...) {
     debugf("catch(...)\n");
     error_count += 1;
   }

   debugf("%d Error%s encountered\n", error_count, error_count == 1 ? "" : "s");
   return error_count == 0 ? 0 : 1;
}

