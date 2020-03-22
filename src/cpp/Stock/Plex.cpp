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
//       Plex.cpp
//
// Purpose-
//       Stock evaluation group.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <com/Checksum.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Interval.h>
#include <com/Julian.h>
#include <com/nativeio.h>
#include <com/Network.h>
#include <com/Random.h>
#include <com/syslib.h>

#include "Dasd.h"
#include "Stock.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "PLEX    " // Source file name

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If defined, hard-core debug mode
#undef  HCDM                        // If defined, hard-core debug mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DIM_BACKUP                4 // The number of backup files

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

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
// Method-
//       Plex::~Plex
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Plex::~Plex( void )              // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Plex::Plex
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Plex::Plex(                      // Constructor
     unsigned int    elements)      // The number of Units
:  DarwinPlex(elements)
,  minGeneration(0)
,  maxGeneration(ULONG_MAX)
,  checkChange(FALSE)
,  checkMutate(FALSE)
,  checkRank(FALSE)
,  checkRule(FALSE)
,  someNormal(FALSE)
{
   #ifdef HCDM
     debugf("Plex(%p)::Plex()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Plex::debugDump
//
// Purpose-
//       Debugging dump.
//
//----------------------------------------------------------------------------
void
   Plex::debugDump( void ) const    // Debugging dump
{
   Unit*             ptrUnit;       // -> Unit

   int               i, j;

   //-------------------------------------------------------------------------
   // Dump the Rules
   //-------------------------------------------------------------------------
   tracef("Plex::debugDump()\n");
// tracef("%.8llx= RNG.getSeed()\n", RNG.getSeed());
   tracef("%.8x= savedSeed\n", global.savedSeed);
   for(i= 0; i<DIM_UNIT; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     tracef("[%2d] c(%10ld) s(%10ld) lt(%6ld) f(%6ld) ",
            i, ptrUnit->cash, ptrUnit->stock,
            ptrUnit->lastTransfer, ptrUnit->fee);

     for(j=0; j<DIM_OUT; j++)
       tracef("%8e ", ptrUnit->outs[j]);
     for(j=0; j<FANIN_COUNT; j++)
       tracef("%.4x ", ptrUnit->rule[j] & 0x0000ffff);
     tracef("\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Plex::evaluate
//
// Purpose-
//       Evaluate the group
//
//----------------------------------------------------------------------------
void
   Plex::evaluate( void )           // Evaluate and sort the group
{
   int               i;

   #ifdef HCDM
     debugf("Plex(%p)::evaluate() started\n", this);
   #endif

   DarwinPlex::evaluate();

   // Evaluation makes the unit valid
   for(i=0; i<used; i++)
   {
     unit[i]->isValid= 1;
   }

   #ifdef HCDM
     debugf("Plex(%p)::evaluate() complete\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       ::diskWrite
//
// Purpose-
//       Write onto disk, check for errors.
//
//----------------------------------------------------------------------------
static void
   diskWrite(                       // Read from disk
     int             handle,        // Handle
     void*           buffer,        // Buffer address
     unsigned        length)        // Buffer length
{
   unsigned          L;             // Length read

   L= write(handle, buffer, length);
   if( L == length )
     return;

   fprintf(stderr, "File write error(%d)", errno);
   perror("");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Plex::backup
//
// Purpose-
//       Save the Units to disk
//
//----------------------------------------------------------------------------
void
   Plex::backup( void )             // Save the Units to disk
{
   char              fileName[32];  // Output file name
   Checksum64        checksum;      // Rule checksum
   Julian            tod;           // Time of day
   DasdNew           header;        // Output header
   int               handle;        // File Handle
   int               keep;          // Number of units kept
   Network::Byte     outsum[sizeof(uint64_t)]; // Output checksum
   Unit*             ptrUnit;       // -> Unit

   int               i, j;

   //-------------------------------------------------------------------------
   // Validate current parameters
   //-------------------------------------------------------------------------
   if( used != DIM_UNIT )
   {
     fprintf(stderr, "Used(%d) != DIM_UNIT(%d)\n", used, DIM_UNIT);
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Informational Header
   //-------------------------------------------------------------------------
   tod= Julian::current();          // Set the current time
   outGeneration= (outGeneration+1)%DIM_BACKUP;
   sprintf(fileName, "Backup.%.3ld", (long)outGeneration);
   debugf("Date(%10lu) Time(%10lu) Writing(%s)...", (long)tod.getDate(),
          (long)getTime(tod), fileName);

   //-------------------------------------------------------------------------
   // Format the header
   //-------------------------------------------------------------------------
   memset((char*)&header, 0, sizeof(header));
   strcpy(header.cbid, PLEX_CBID);
   header.releaseId= DasdNew::ReleaseId;
   header.versionId= DasdNew::VersionId;
   header.julianDay= (long)tod.getDate();
   header.julianTod= (long)getTime(tod);

   header.l3ArraySize= DasdNew::L3ArraySize;
   header.l2ArraySize= DasdNew::L2ArraySize;
   header.l1ArraySize= DasdNew::L1ArraySize;
   header.l0ArraySize= DasdNew::L0ArraySize;

   header.randSeed= global.savedSeed;
   header.generation= generation;

   header.unitCount= DIM_UNIT;
   header.usedCount= DIM_USED;
   header.cullCount= getCull();
   header.outsCount= DIM_OUT;

   header.index0= histIndex0;
   header.indexN= histIndexN;
   header.julian0= histJulian[histIndex0];
   header.julianN= histJulian[histIndexN-1];

   for(i= 0; i<DIM_UNIT; i++)
   {
     ptrUnit= (Unit*)getUnit(i)->castConcrete();
     header.unit[i].evaluation=   ptrUnit->evaluation;
     header.unit[i].cash=         ptrUnit->cash;
     header.unit[i].stock=        ptrUnit->stock;
     header.unit[i].lastTransfer= ptrUnit->lastTransfer;
     header.unit[i].fee=          ptrUnit->fee;
     for(j=0; j<DIM_OUT; j++)
     {
       header.unit[i].outs[j]=    ptrUnit->outs[j];
     }
   }

   //-------------------------------------------------------------------------
   // Write the header
   //-------------------------------------------------------------------------
   checksum.reset();
   handle= open(fileName,           // Open the file
                O_WRONLY|O_BINARY|O_TRUNC|O_CREAT,// (in write-only binary mode)
                S_IREAD|S_IWRITE);  // (with full write access)
   if( handle < 0 )                 // If we cannot open the output file
   {
     fprintf(stderr, "Open(%s), errno(%d)", fileName, errno);
     perror(":");
     exit(EXIT_FAILURE);
   }

   checksum.accumulate((char*)&header, sizeof(header));
   diskWrite(handle, &header, sizeof(header));

   //-------------------------------------------------------------------------
   // Write the Rules
   //-------------------------------------------------------------------------
   keep= DIM_UNIT-header.cullCount;
   for(i= 0; i<keep; i++)
   {
     ptrUnit= (Unit*)getUnit(i)->castConcrete();
     checksum.accumulate((char*)ptrUnit->rule, RULE_SIZE);
     diskWrite(handle, ptrUnit->rule, RULE_SIZE);
   }

   //-------------------------------------------------------------------------
   // Write the trailing checksum, close the file
   //-------------------------------------------------------------------------
   Network::store64(checksum.getValue(), outsum);
   diskWrite(handle, outsum, sizeof(outsum));
   if( close(handle) != 0 )         // If close failure
   {
     fprintf(stderr, "Close(%s), errno(%d)", fileName, errno);
     perror(":");
     exit(EXIT_FAILURE);
   }
   debugf("done\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       ::diskRead
//
// Purpose-
//       Read from disk, check for errors.
//
//----------------------------------------------------------------------------
static void
   diskRead(                        // Read from disk
     int             handle,        // Handle
     void*           buffer,        // Buffer address
     unsigned        length)        // Buffer length
{
   unsigned          L;             // Length read

   L= read(handle, buffer, length);
   if( L == length )
     return;

   if( L == 0 )
   {
     fprintf(stderr, "File incomplete\n");
     exit(EXIT_FAILURE);
   }

   fprintf(stderr, "File read error(%d)", errno);
   perror("");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Plex::newFormat
//
// Purpose-
//       Restore the Units from disk - new format
//
//----------------------------------------------------------------------------
void
   Plex::newFormat(                 // Restore the Units from disk
     int             handle)        // Handle
{
   Checksum64        checksum;      // Working Checksum
   int               diff;          // Number of differing rules
   DasdNew           header;        // Header
   Network::Byte     inpsum[sizeof(uint64_t)]; // Checksum input area
   int               keep;          // The number of units on disk
   const char*       oldRule;       // -> Rule[0]
   Unit*             ptrUnit;       // -> Unit
   unsigned          saveIndex0;    // Saved histIndex0
   uint64_t          validator;     // Checksum validator

   int               i, j;

   //-------------------------------------------------------------------------
   // Read and verify the header
   //-------------------------------------------------------------------------
   checksum.reset();

   diskRead(handle, &header, sizeof(header));
   checksum.accumulate((char*)&header, sizeof(header));

   if( strcmp(header.cbid, PLEX_CBID) != 0
       || header.releaseId != DasdNew::ReleaseId
       || header.versionId != DasdNew::VersionId
       || header.unitCount != DIM_UNIT
       || header.l3ArraySize != DasdNew::L3ArraySize
       || header.l2ArraySize != DasdNew::L2ArraySize
       || header.l1ArraySize != DasdNew::L1ArraySize
       || header.l0ArraySize != DasdNew::L0ArraySize)
   {
     fprintf(stderr, "Invalid Header Data\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Load the Rules
   //-------------------------------------------------------------------------
   keep= DIM_UNIT - header.cullCount;
   for(i= 0; i<keep; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     ptrUnit->evaluation=   header.unit[i].evaluation;
     ptrUnit->cash=         header.unit[i].cash;
     ptrUnit->stock=        header.unit[i].stock;
     ptrUnit->lastTransfer= header.unit[i].lastTransfer;
     ptrUnit->fee=          header.unit[i].fee;
     for(j=0; j<DIM_OUT; j++)
     {
       ptrUnit->outs[j]=    header.unit[i].outs[j];
     }

     diskRead(handle, ptrUnit->rule, RULE_SIZE);
     checksum.accumulate((char*)ptrUnit->rule, RULE_SIZE);
   }
   for(i= keep; i<DIM_UNIT; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     ptrUnit->random();
     ptrUnit->evaluation= 0;
   }
   if( global.seedControl == 0 )    // If not random seed
     RNG.setSeed(header.randSeed);  // Use saved seed
   generate();                      // Next generation
   generation= header.generation;

   //-------------------------------------------------------------------------
   // Read the trailing checksum
   //-------------------------------------------------------------------------
   diskRead(handle, inpsum, sizeof(inpsum));
   validator= Network::load64(inpsum);
   if( checksum.getValue() != validator )
   {
     fprintf(stderr, "Data checksum\n");
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // See if rules differ
   //-------------------------------------------------------------------------
   ptrUnit= (Unit*)(getUnit(0)->castConcrete());
   oldRule= (char*)ptrUnit->rule;
   diff= 0;
   for(i= 0; i<keep; i++)
   {
     ptrUnit= (Unit*)(getUnit(i)->castConcrete());
     oldRule= (char*)ptrUnit->rule;
     for(j= i+1; j<keep; j++)
     {
       ptrUnit= (Unit*)(getUnit(j)->castConcrete());
       if( memcmp(oldRule, ptrUnit->rule, RULE_SIZE) != 0 )
       {
         diff++;
         break;
       }
     }
   }
   debugf("\n");
   if( diff == 0 )
     debugf("!! WARNING !! All Rules are the same\n");

   else
     debugf("%d of %d Rules differ\n", diff+1, keep);

   //-------------------------------------------------------------------------
   // Check for incremental evaluation
   //-------------------------------------------------------------------------
   if( global.revalControl != 0 )
     debugf("Re-evaluation forced\n");
   else if( header.index0 == histIndex0
       && header.indexN == histIndexN
       && histJulian[histIndex0] == header.julian0
       && histJulian[histIndexN-1] == header.julianN )
   {
     debugf("Continuing evaluation, %d units already valid\n", keep);
     for(i= 0; i<keep; i++)
     {
       ptrUnit= (Unit*)(getUnit(i)->castConcrete());
       ptrUnit->isValid= 1;
     }
   }

   else if( header.index0 == histIndex0
       && histIndexN > header.indexN
       && histJulian[histIndex0] == header.julian0
       && histJulian[header.indexN-1] == header.julianN )
   {
     debugf("Incremental evaluation\n");
     saveIndex0= histIndex0;
     histIndex0= header.indexN;
     for(i= 0; i<keep; i++)
     {
       ptrUnit= (Unit*)(getUnit(i)->castConcrete());
       ptrUnit->isValid= 1;         // Indicate incremental evaluation
       ptrUnit->evaluation= ptrUnit->evaluate();
     }
     histIndex0= saveIndex0;
   }
   else
     debugf("Re-evaluation required\n");

   debugf("Training mode...\n");
   debugf("\n");
}

#include "OldPlex.cpp"

//----------------------------------------------------------------------------
//
// Method-
//       Plex::restore
//
// Purpose-
//       Restore the Units from disk
//
//----------------------------------------------------------------------------
void
   Plex::restore( void )            // Restore the Units from disk
{
   char              fileName[32];  // Input file name
   DasdHeader        header[DIM_BACKUP]; // Input header
   int               handle[DIM_BACKUP]; // File Handle
   Unit*             ptrUnit;       // -> Unit

   int               i;

   //-------------------------------------------------------------------------
   // Validate initial parameters
   //-------------------------------------------------------------------------
   if( used != DIM_UNIT )
   {
     fprintf(stderr, "Used(%d) != DIM_UNIT(%d)\n", used, DIM_UNIT);
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Open the files
   //-------------------------------------------------------------------------
   for(i=0; i<DIM_BACKUP; i++)
   {
     sprintf(fileName, "Backup.%.3d", i);
     handle[i]= open(fileName,      // Open the file
                     O_RDONLY|O_BINARY); // (in read-only binary mode)
   }

   for(i=0; i<DIM_BACKUP; i++)
   {
     if( handle[i] >= 0 )
       break;
   }

   if( i == DIM_BACKUP )            // If no files open
   {
     fprintf(stderr, "\n\n");
     fprintf(stderr, "No restore file!\n");

     for(i= 0; i<DIM_UNIT; i++)
     {
       ptrUnit= (Unit*)(getUnit(i)->castConcrete());
       ptrUnit->random();           // Load random data
     }

     outGeneration= 0;
     return;
   }

   //-------------------------------------------------------------------------
   // Read the headers
   //-------------------------------------------------------------------------
   for(i=0; i<DIM_BACKUP; i++)
   {
     header[i].julianDay= 0;
     header[i].julianTod= 0;
     if( handle[i] < 0 )
     {
       fprintf(stderr, "File(Backup.%.3d) Failed to open\n", i);
       continue;
     }

     diskRead(handle[i], &header[i], sizeof(header[i]));
     printf("File(Backup.%.3d) Date(%10u) Time(%10u)\n", i,
            header[i].julianDay, header[i].julianTod);
   }

   //-------------------------------------------------------------------------
   // Pick the latest one
   //-------------------------------------------------------------------------
   outGeneration= 0;
   for(i=1; i<DIM_BACKUP; i++)
   {
     if( header[outGeneration].julianDay > header[i].julianDay )
       continue;
     if( header[outGeneration].julianDay < header[i].julianDay )
     {
       outGeneration= i;
       continue;
     }

     // Dates are equal, check times
     if( header[outGeneration].julianTod > header[i].julianTod )
       continue;
     if( header[outGeneration].julianTod < header[i].julianTod )
       outGeneration= i;
   }

   i= outGeneration;
   sprintf(fileName, "Backup.%.3d", i);
   debugf("\n");
   debugf("File(%s) Date(%10u) Time(%10u) selected\n", fileName,
          header[i].julianDay, header[i].julianTod);

   //-------------------------------------------------------------------------
   // Load the Rules
   //-------------------------------------------------------------------------
   lseek(handle[i], 0, SEEK_SET);   // Reposition to beginning of file

   if( header[i].versionId == DasdNew::VersionId
       && header[i].releaseId == DasdNew::ReleaseId )
     newFormat(handle[i]);
   else if( header[i].versionId == DasdOld::VersionId
            && header[i].releaseId == DasdOld::ReleaseId )
   {
     debugf(".. Old format file!!\n");
     oldFormat(handle[i]);
   }
   else
   {
     fprintf(stderr, "Version(%d) Release(%x) not supported\n",
                     header[i].versionId, header[i].releaseId);
     exit(EXIT_FAILURE);
   }

   //-------------------------------------------------------------------------
   // Close the files
   //-------------------------------------------------------------------------
   for(i= 0; i<DIM_BACKUP; i++)
     close(handle[i]);
}

