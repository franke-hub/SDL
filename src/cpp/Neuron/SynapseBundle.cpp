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
//       SynapseBundle.cpp
//
// Purpose-
//       SynapseBundle object methods.
//
// Last change date-
//       2013/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Synapse.h"
#include "SynapseBundle.h"

//----------------------------------------------------------------------------
//
// Method-
//       SynapseBundle::~SynapseBundle
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   SynapseBundle::~SynapseBundle( void ) // Destructor
{
   if( bundle != NULL )
   {
     for(unsigned x= 0; x<bCount; x++)
     {
       delete bundle[x];
       bundle[x]= NULL;
     }

     free(bundle);
     bundle= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SynapseBundle::SynapseBundle
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   SynapseBundle::SynapseBundle(    // Constructor
     unsigned int      bCount,      // Number of bundles
     unsigned int      iCount,      // Number of input Axions
     unsigned int      oCount)      // Number of output Neurons
:  Object()
,  bCount(bCount),  iCount(iCount), oCount(oCount)
,  bundle(NULL)
{
   if( iCount == 0 || oCount == 0 || (iCount&7) != 0 || (oCount&7) != 0 )
     throw "SynapseBundle: Parameter error";

   bundle= (Synapse**)malloc(bCount*sizeof(Synapse*));
   if( bundle == NULL )
     throw "SynapseBundle: Storage shortage";
   memset(bundle, 0, bCount*sizeof(Synapse*)); // All entries NULL

   for(unsigned x= 0; x<bCount; x++)
   {
     bundle[x]= new Synapse(iCount, oCount);
     if( bundle[x] == NULL )
       throw "SynapseBundle: Storage shortage";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SynapseBundle::update
//
// Purpose-
//       Read inputs, write outputs
//
//----------------------------------------------------------------------------
void
   SynapseBundle::update( void )    // Read inputs, write outputs
{
   for(unsigned x= 0; x<bCount; x++)
     bundle[x]->update();
}

