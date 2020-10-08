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
//       NCmain.cpp
//
// Purpose-
//       Neural Net Compiler.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <exception>
#include <iostream>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <com/Debug.h>
#include <com/define.h>
#include <com/syslib.h>

#include <Message.h>
#include "NC_com.h"
#include "NC_op.h"
#include "NC_sym.h"
#include "NC_sys.h"

#include "NN_com.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NCMAIN  " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define DEFAULT_GROUP "*"           // Default group symbol name

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define __BRINGUP__           FALSE // Bringup diagnostics?
#include "NCdiag.h"

//----------------------------------------------------------------------------
//
// Class-
//       NC_MessageCallback
//
// Purpose-
//       MessageCallback
//
//----------------------------------------------------------------------------
class NC_MessageCallback : public MessageCallback
{
//----------------------------------------------------------------------------
// NC_MessageCallback::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~NC_MessageCallback( void ) {}   // Destructor
   NC_MessageCallback( void );      // Constructor

//----------------------------------------------------------------------------
// NC_MessageCallback::Methods
//----------------------------------------------------------------------------
public:
virtual void
   set( void );                     // Set filename components

//----------------------------------------------------------------------------
// NC_MessageCallback::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class NC_MessageCallback

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
NN_com*                nn_com;      // Pointer to global area
int                    HCDM;        // Hard-Core Debug mode

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static NC_MessageCallback
                       callback;    // Our callback

//----------------------------------------------------------------------------
//
// Method-
//       NC_MessageCallback::NC_MessageCallback
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   NC_MessageCallback::NC_MessageCallback( void )
:  MessageCallback()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_MessageCallback::set
//
// Purpose-
//       Set filename components.
//
//----------------------------------------------------------------------------
void
   NC_MessageCallback::set( void )  // Set filename components
{
   if( NC_COM.pass == NC_com::Pass0 )
   {
     if( NC_COM.srcfile == NULL )
       strcpy(fileName, "*UndefinedFile*");
     else
     {
       strcpy(fileName, NC_COM.srcfile->filenm);
       lineNumber= NC_COM.srcfile->lineno;
       column= NC_COM.srcfile->column;
     }
   }


   if( NC_COM.debug == NULL )
     return;

   if( NC_COM.debug->ifd == NULL )
     strcpy(fileName, "*UndefinedFile*");
   else
     strcpy(fileName, NC_COM.debug->ifd->filenm);

   lineNumber= NC_COM.debug->lineNumber;
   column= NC_COM.debug->column;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getzero
//
// Purpose-
//       Allocate zeroed storage, abort if not available.
//
//----------------------------------------------------------------------------
static void*
   getzero(                         // MALLOC, must-complete
     int32_t           length)      // The allocation length
{
   void*               retaddr;     // The return address

   retaddr= malloc(length);         // Allocate the storage
   if( retaddr == NULL )            // If storage not available
   {
     NCmess(NC_msg::ID_StgInitial, 0);
     exit(EXIT_FAILURE);
   }
   memset(retaddr, 0, length);

   return(retaddr);                 // Exit, function complete
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       finish
//
// Purpose-
//       Write completion message
//
//----------------------------------------------------------------------------
static void
   finish(                          // Completion message
     NC_msg::MessageId ident,       // Message identifier
     int               count)       // Error count
{
   char                cc[16];      // Character count

   sprintf(cc, "%5d", count);       // Format number
   NCmess(ident, 1, cc);            // Show error severity
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wrap
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
static int                          // Return code
   wrap(                            // Neuron control program
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   NN::FileId          fileId;      // Working fileId
   FileName            fileName;    // FileName object
   SymtabIterator      iterator;    // Symbol table iterator
   NN::Offset          offset;      // Working offset
   NC_op*              op;          // Current operator
   Neuron*             ptrN;        // -> Neuron
   NC_ofd*             ptrofd;      // Pointer to file descriptor
   NC_NeuronSymbol*    symbol;      // Symbol table entry
   NN::FPO             fpo;         // File/Part/Offset

   unsigned            i;
   int                 rc;

   //-------------------------------------------------------------------------
   // Global storage allocation
   //-------------------------------------------------------------------------
   new NC_com();                    // Build the NC common object
   nn_com= new NN_com();            // Build the NN common object

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   ncparm(argc, argv);              // Parameter analysis

   //-------------------------------------------------------------------------
   // Global storage initialization
   //-------------------------------------------------------------------------
   NC_COM.dummyDebug= NC_opDebug::generate();
   NC_COM.message.load("nnc.msg");

   NC_COM.stmtbuff= (char*)getzero(NC_COM.max_stmt);
   NC_COM.exprbuff= (char*)getzero(NC_COM.max_stmt);
   NC_COM.obj_no= 1;

   NC_COM.grpstak.reset();          // Initialize the begin stack
   NC_COM.srclist.reset();          // Initialize the source list
   NC_COM.srcstak.reset();          // Initialize the source stack
   NC_COM.objlist.reset();          // Initialize the object list

   NC_COM.message.setCallback(&callback);

   NC_COM.begroup= (NC_BeGroupSymbol*)
                       NC_COM.ist.Symtab::insert(NULL, DEFAULT_GROUP);
   NC_COM.grpstak.lifo(NC_COM.begroup); // This block is now active
   NC_COM.begroup->type= NC_sym::TypeBeGroup;
   NC_COM.begroup->current_G= NC_COM.begroup;

   //-------------------------------------------------------------------------
   // Initialize traceing
   //-------------------------------------------------------------------------
   if( NC_debug > 1000 )            // If intensive mode
   {
     debugSetIntensiveMode();       // Intensive debugging mode
     NC_debug -= 1000;              // Restore debug level
   }

   //-------------------------------------------------------------------------
   // Initialize PGS
   //-------------------------------------------------------------------------
   NC_COM.vps_framemask= NC_COM.vps_framesize - 1;
   rc= NN_COM.pgs.cold(             // Initialize VPS
           NC_COM.outname,          // The control file's name
           NC_COM.vps_framesize,    // Framesize
           0);                      // Default Frameno
   if( rc != 0 )                    // If initialization failed
   {
     sprintf(NC_COM.word0, "%d", rc);
     sprintf(NC_COM.word1, "%d", NC_COM.vps_framesize);
     sprintf(NC_COM.word2, "%d", NC_COM.vps_fileno + 1);
     sprintf(NC_COM.word3, "%d", NC_COM.vps_partno + 1);
     NCmess(NC_msg::ID_VPSOpen, 5, NC_COM.word0,
                                   NC_COM.outname,
                                   NC_COM.word1,
                                   NC_COM.word2,
                                   NC_COM.word3);
     exit(EXIT_FAILURE);
   }

// vpsinfo(0, "NeuralNet datafile V1.0 R1.0");

   //-------------------------------------------------------------------------
   // Set default output file
   //-------------------------------------------------------------------------
   ptrofd= (NC_ofd*)malloc(sizeof(NC_ofd));
   if( ptrofd == NULL )
   {
     NCmess(NC_msg::ID_StgInitial, 0);
     exit(EXIT_FAILURE);
   }
   memset(ptrofd, 0, sizeof(NC_ofd));

   fileName.append(NC_COM.inpname);
   fileName.getNameOnly(ptrofd->fname, NC_COM.inpname);
   strcat(ptrofd->fname, ".000");
   ptrofd->fileno= NN_COM.pgs.insFile(ptrofd->fname); // Activate the file

   NC_COM.objlist.lifo(ptrofd);     // Add the OFD to the list of OFDs
   NC_COM.objfile= ptrofd;          // This file is now active
   NC_COM.begroup->ofd= ptrofd;     // This file is now active

   //-------------------------------------------------------------------------
   // Pass[0]
   //-------------------------------------------------------------------------
   BRINGF(( "Pass[0]\n" ));
   NC_COM.pass= NC_com::Pass0;      // Indicate Pass[0]
   NCmess(NC_msg::ID_VersionId, 0); // Startup statement
   NCmess(NC_msg::ID_Pass1, 1, NC_COM.inpname); // Source file

   ncincl(NC_COM.inpname);          // PASS[0]
   if( NC_COM.message.highLevel > NC_COM.message.stopLevel )
     goto terminate;
   if( NC_COM.initial_N == FALSE )  // If no entry statement
   {
     NCmess(NC_msg::ID_EntMissing, 0);
     goto terminate;
   }

   //-------------------------------------------------------------------------
   // Pass[1]
   //-------------------------------------------------------------------------
   BRINGF(( "Pass[1]\n" ));
   NC_COM.pass= NC_com::Pass1;      // Indicate PASS[1]

   for(op= (NC_op*)NC_COM.pass1.getHead();
           op != NULL;
           op= (NC_op*)op->getNext())
   {
     BRINGUP(cout << *op;)
     op->operate();
   }
   if( NC_COM.message.highLevel > NC_COM.message.stopLevel )
     goto terminate;

   // Allocate the space for the Neurons
   for(iterator.begin(NC_COM.xst); iterator.isValid(); iterator.next())
   {
     symbol= (NC_NeuronSymbol*)iterator.current();
     if( symbol->type != NC_sym::TypeNeuron )
       continue;

     rc= ncalloc(&fpo,              // Allocate neuron space
                 symbol->addr.f,    // From assigned file
                 NN::PartNeuron,    // Allocate neuron space
                 symbol->count);    // Number of elements
     if( rc != 0 )
       exit(EXIT_FAILURE);

     BRINGF(( "Neuron(%.2lx:%.8lx)*[%3ld] '%s'\n", (long)fpo.f, (long)fpo.o,
              (long)symbol->count, NC_COM.xst.getSymbolName(symbol) ));

     symbol->addr.f= fpo.f;
     symbol->addr.o= fpo.o;
   }

   //-------------------------------------------------------------------------
   // Pass[2]
   //-------------------------------------------------------------------------
   BRINGF(( "Pass[2]\n" ));
   NCmess(NC_msg::ID_Pass2, 0);     // Pass[2] message
   NC_COM.pass= NC_com::Pass2;

   for(op= (NC_op*)NC_COM.pass2.getHead();
           op != NULL;
           op= (NC_op*)op->getNext())
   {
     BRINGUP(cout << *op;)
     op->operate();
   }
   if( NC_COM.message.highLevel > NC_COM.message.stopLevel )
     goto terminate;

   //-------------------------------------------------------------------------
   // Pass[3]
   //-------------------------------------------------------------------------
   BRINGF(( "Pass[3]\n" ));
   NC_COM.pass= NC_com::Pass3;

   for(op= (NC_op*)NC_COM.passN.getHead();
           op != NULL;
           op= (NC_op*)op->getNext())
   {
     BRINGUP(cout << *op; cout.flush();)
     op->operate();
   }
   if( NC_COM.message.highLevel > NC_COM.message.stopLevel )
     goto terminate;

   // Allocate the space for the Fanins
   for(iterator.begin(NC_COM.xst); iterator.isValid(); iterator.next())
   {
     symbol= (NC_NeuronSymbol*)iterator.current();
     if( symbol->type != NC_sym::TypeNeuron )
       continue;

     fileId= symbol->addr.f;
     offset= symbol->addr.o;
     for(i=0; i<symbol->count; i++) // Initialize the Fanin address
     {
       ptrN= (Neuron*)nnuchg(fileId, NN::PartNeuron, offset);
       if( ptrN == NULL )           // If VPS error
       {
         NCmess(NC_msg::ID_VPSFault, 0);
         break;
       }

       if( ptrN->cbid != Neuron::CBID )
       {
         printf("..Symbol(%s) A(%.2x:%.8lx.%.8lx) CBID(%.4X)\n",
                NC_COM.xst.getSymbolName(symbol), symbol->addr.f,
                long(symbol->addr.o>>32), long(symbol->addr.o), ptrN->cbid);
         NCfault(__SOURCE__, __LINE__);
       }

       else if( ptrN->faninCount > 0 )
       {
         rc= ncalloc(&fpo,          // Allocate Fanin space
                     fileId,        // From assigned file
                     NN::PartFanin, // Allocate Fanin space
                     ptrN->faninCount); // Number of elements
         if( rc != 0 )
           exit(EXIT_FAILURE);

         BRINGF(( "Neuron(%.2lx:%.8lx) Fanin(%.8lx)*%ld\n",
                  (long)fileId, (long)offset, (long)fpo.o,
                  (long)ptrN->faninCount ));

         ptrN->faninVaddr= fpo.o;
         ptrN->faninCount= 0;       // Reset for next pass
       }

       nnurel(fileId, NN::PartNeuron, offset);
       offset += sizeof(Neuron);
     }
   }

   //-------------------------------------------------------------------------
   // Pass[4]
   //-------------------------------------------------------------------------
   BRINGUP(cout << "Pass[4]\n"; cout.flush();)
   NC_COM.pass= NC_com::Pass4;

   for(op= (NC_op*)NC_COM.passN.getHead();
           op != NULL;
           op= (NC_op*)op->getNext())
   {
     op->operate();
   }
   if( NC_COM.message.highLevel > NC_COM.message.stopLevel )
     goto terminate;

   //-------------------------------------------------------------------------
   // Pass[5]
   //-------------------------------------------------------------------------
   BRINGUP(cout << "Pass[5]\n"; cout.flush();)
   if( NC_COM.sw_symtab )           // If symbol table is wanted
   {
     printf("Neurons by address\n");
     NC_COM.xst.displayByAddr();    // Display the symbol table

     printf("\n");
     printf("Neurons by name\n");
     NC_COM.xst.displayByName();    // Display the symbol table
   }

   //-------------------------------------------------------------------------
   // Terminate VPS
   //-------------------------------------------------------------------------
terminate:
   NN_COM.pgs.term();               // Terminate VPS

   //-------------------------------------------------------------------------
   // Write error level message
   //-------------------------------------------------------------------------
   if( NC_COM.message.highLevel >= NC_msg::ML_Warn )
     finish(NC_msg::ID_WarnNo, NC_COM.message.warnCount);
   if( NC_COM.message.highLevel >= NC_msg::ML_Error )
     finish(NC_msg::ID_ErrsNo, NC_COM.message.errsCount);
   if( NC_COM.message.highLevel >= NC_msg::ML_Severe )
     finish(NC_msg::ID_SevsNo, NC_COM.message.sevsCount);
   if( NC_COM.message.highLevel >= NC_msg::ML_Terminating )
     finish(NC_msg::ID_TermNo, NC_COM.message.termCount);

   if( NC_COM.message.highLevel >= NC_msg::ML_Error )
     return 1;

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
   main(                            // Neuron control program
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 result= 1;   // Return code (failure)

   try {
     result= wrap(argc, argv);
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(std::exception& X) {
     printf("catch(exception.what(%s))\n", X.what());
   } catch(...) {
     printf("Exception(...)\n");
   }

   return result;
}

