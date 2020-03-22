//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       tlc.cpp
//
// Purpose-
//       Threaded Language Compiler, i.e. Forth
//
// Last change date-
//       2020/01/10
//
//----------------------------------------------------------------------------
#include "tlc.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAIN "tlc_main.hpp"

#if false
#include "tlc_main.hpp"
#include "tlc_test.hpp"
#include "tlc_oops.hpp"
#endif

// Debugging controls
static const int       USE_DEBUG= false; // Activate debugging code?
static const size_t    MAX_I= 512;  // Maximum instruction counter (if !0)
static const size_t    MAX_L= 8;    // Maximum Level counter (if !0)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define CODE_SIZE 0x00010000        // Number of Code Stack elements
#define DATA_SIZE 0x00100000        // Number of Data Stack elements

//----------------------------------------------------------------------------
// Debugging information
//----------------------------------------------------------------------------
static size_t          I_counter= 0; // Instruction counter
static size_t          L_counter= 0; // CNEXT depth level

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Stack<Word>     code;        // The Code Stack
static Stack<Data>     data;        // The Data Stack
static WordMap         wordMap;     // The Symbol Table

static Word            i_addr;      // The current instruction address
static Word            i_word;      // The current instruction
static Word            x_addr;      // The current EXIT address

static int             operational; // TRUE while operational

//----------------------------------------------------------------------------
//
// Subroutine-
//       next_word
//
// Purpose-
//       Update a Word by its length
//
//----------------------------------------------------------------------------
static inline void
   next_word(                       // Update a Word by its length
     Word&             word)        // The Word to update
{  word= (Word)((char*)word + sizeof(Word)); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_dump
//
// Purpose-
//       Dump the stacks
//
//----------------------------------------------------------------------------
static void debug_dump( void )
{
   size_t used= code.used;
   debugf("[%6zd] Code:\n", used);
   while( used > 0 )
   {
     Word word= code[used-1];
     debugf("[%6d] %p\n", int(used - code.used), word);
     used--;
   }

   used= data.used;
   debugf("\n[%6zd] Data:\n", used);
   while( used > 0 )
   {
     Data item= data[used-1];
     debugf("[%6d] 0x%lx,%ld\n", int(used - data.used), item, item);
     used--;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_list
//
// Purpose-
//       Display a thread
//
//----------------------------------------------------------------------------
void
   debug_list(                      // Display a thread
     void**            list)        // The list of instructions
{
   Word                word= nullptr; // Working Word

   Word addr= Word(list);
   do
   {
     word= *(Word*)addr;
     if( word == nullptr )
     {
       if( addr == x_addr )
       {
         debugf("<EXIT> [%p] %p\n", addr, word);
         return;
       }

       while( word == nullptr && (Data(addr) & 0x0000001f) )
       {
         debugf("ALIGN  [%p] %p\n", addr, word);
         word= *(Word*)addr;
         next_word(addr);
       }
     }

     debugf(">>>>>  [%p] %p\n", addr, word);
     next_word(addr);
   } while(word);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_op
//
// Purpose-
//       Write standard operation debug message
//
//----------------------------------------------------------------------------
static void
   debug_op(                        // Standard operation debug message
     const char*       op)          // The operation
{
   debugf("%-6s [%p] %p C[%zd](%p) D[%zd](0x%zx,%zd)\n", op, i_addr, i_word,
          code.used, code.top(), data.used, data.top(), data.top());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_this
//
// Purpose-
//       Display the current thread content
//
//----------------------------------------------------------------------------
void
   debug_this( void )               // Display current thread
{  debug_list((void**)(Word*)i_addr); };

//----------------------------------------------------------------------------
//
// Subroutine-
//       set_iaddr
//
// Purpose-
//       Set the new instruction address (while running an instruction)
//
// Implemenation notes-
//       The new instruction address is updated AFTER the instrucion runs,
//       so the actual instruction address set is decremented first.
//
//----------------------------------------------------------------------------
static inline void
   set_iaddr(                       // Set the instruction address
     Word              addr)        // The next instruction to run
{
   i_addr= addr;                    // The desired instruction address
   i_addr= (Word)((char*)i_addr - sizeof(Word));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       _CON
//
// Purpose-
//       Define constant subroutine.
//
// Implementation notes-
//       Pushes variable content to top of Data Stack
//
// Usage-
//       Define a variable using:
//         Word VAR_NAME[] = {DEF_CON, constant_value};
//
//----------------------------------------------------------------------------
static void _CON(void) {
   Word word= i_word;
   next_word(word);
   Data item= *(Data*)word;
   if( USE_DEBUG )
     debugf("+++++  [%p]=(0x%zx,%zd)\n", word, item, item);
   data.push(item);
}
static Word DEF_CON = (Word)_CON;

//----------------------------------------------------------------------------
//
// Subroutine-
//       _SUB
//
// Purpose-
//       Define thread subroutine
//
// Usage-
//       Use this for the first item in a subroutine.
//
//----------------------------------------------------------------------------
static void _SUB(void) {
   code.push(i_addr);
   i_addr= i_word;
}
static Word DEF_SUB = (Word)_SUB;

//----------------------------------------------------------------------------
//
// Subroutine-
//       _VAR
//
// Purpose-
//       Define variable subroutine.
//
// Implementation notes-
//       Pushes *ADDRESS OF* variable to top of Data Stack
//
// Usage-
//       Define a variable using:
//         Word VAR_NAME[] = {DEF_VAR, initial_value};
//
//----------------------------------------------------------------------------
static void _VAR(void) {
   Word word= i_word;
   next_word(word);

   if( USE_DEBUG  ) {
     Data data= *(Data*)word;
     debugf("+++++  [%p]=(0x%zx,%zd)\n", word, data, data);
   }

   data.push(Data(word));
}
static Word DEF_VAR = (Word)_VAR;

//----------------------------------------------------------------------------
// CEXIT: Return from DEF_SUB subroutine
//----------------------------------------------------------------------------
static void CEXIT(void) {
   i_addr= code.pop();
}
static Word TEXIT[] = {Word(CEXIT)};

//----------------------------------------------------------------------------
// CGOTO: Threaded GOTO
//----------------------------------------------------------------------------
static void CGOTO(void) {
   next_word(i_addr);
   set_iaddr(*(Word*)i_addr);
}
static Word TGOTO[] = {(Word)CGOTO};

//----------------------------------------------------------------------------
// CNEXT: Threaded operation mode instruction processing loop
//   Called with next instruction to execute on data stack
//----------------------------------------------------------------------------
static void CNEXT(void)
{
   Word              PROGRAM[2]; // The program to execute
   Word              s_iaddr= i_addr; // The current i_addr
   Word              s_xaddr= x_addr; // The current x_addr

   if( USE_DEBUG )                  // If debugging activated
   {
     debugf(">>>>>> %p %p CNEXT(%p)\n", i_addr, x_addr, Word(data.top()));
//// debug_dump(); // DO NOT REMOVE // HARD CORE DEBUG MODE

     L_counter++;                   // Update level counter
     if( L_counter > MAX_L )
       throw Exception(to_string("MAX_L(%ld) exceeded", MAX_L));
   }

   PROGRAM[0]= Word(data.pop());    // Get THE instruction
   PROGRAM[1]= nullptr;             // The exit program
   i_addr= PROGRAM + 0;             // Set instruction address
   x_addr= PROGRAM + 1;             // Set exit address
   while( operational )
   {
     if( USE_DEBUG )                // If debugging activated
     {
       I_counter++;                 // Update instruction counter
       if( I_counter > MAX_I )
         throw Exception(to_string("MAX_I(%ld) exceeded", MAX_I));
     }

     IFCHECK(
       if( i_addr == nullptr )
       {
         debugf("ERROR: i_addr == nullptr\n");
         break;
       }
     )

     i_word= *(Word*)i_addr;
     if( i_word == nullptr )        // If empty program
     {
       if( i_addr == x_addr )       // EXIT if exit address reached
         break;

       while( i_word == nullptr && (Data(i_addr) & 0x0000001f) )
       {
         if( USE_DEBUG ) debugf("ALIGN  [%p] %p\n", i_addr, i_word);
         next_word(i_addr);
         i_word= *(Word*)i_addr;
       }
     }

     if( USE_DEBUG ) debug_op("CNEXT");

     IFCHECK(
       if( i_word == nullptr )      // Exit if nullpointer
       {
         if( !USE_DEBUG ) debug_op("CNEXT");
         debugf("ERROR: HALT detected\n");
         break;
       }

       if( *(Code*)i_word == nullptr ) // Exit if malformed
       {
         if( !USE_DEBUG ) debug_op("CNEXT");
         debugf("ERROR: ZERO detected\n");
         break;
       }
     )

     (*(Code*)i_word)();
     next_word(i_addr);
   }

   if( USE_DEBUG )                  // If debugging activated
   {
     debugf("<<<<<< %p %p CNEXT(%p)\n", s_iaddr, s_xaddr, PROGRAM[0]);
     L_counter--;
   }

   x_addr= s_xaddr;                 // Restore exit address
   i_addr= s_iaddr;                 // Restore instruction address
}
static Word TNEXT[] = {Word(CNEXT)};

//----------------------------------------------------------------------------
// CRESET: Initialize/Reset the environment
//----------------------------------------------------------------------------
static void CRESET(void) {
   // Initialize the NULL program
   // Code stack initialized to: (nullptr)
   code.used= 0;
   i_addr= Word(0xdeadbeef000000fe);
   x_addr= Word(0xdeadbeef000000ef);
   code.push(nullptr);

   // Allow the user to accidentally pop the top stack. (Hey, it happens)
   // Data stack initialized to: (-7) (-6) (-5) (-4) (-3) (-2) (-1) (0)
   data.used= 0;
   for(int i= 0; i<8; i++)
     data.push(i-7);

   operational= true;
}

//----------------------------------------------------------------------------
// Threaded code
//----------------------------------------------------------------------------
#include MAIN

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
   code.item= (Word*)malloc(sizeof(Code) * CODE_SIZE);
   code.size= CODE_SIZE;

   data.item= (Data*)malloc(sizeof(Data) * DATA_SIZE);
   data.size= DATA_SIZE;

   if( data.item == nullptr || data.item == nullptr )
     throwf("Storage shortage");

   pub::Console::start();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate.
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   if( code.item ) free(code.item);
   if( data.item ) free(data.item);

   pub::Console::stop();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static inline void
   info( void )                     // Informational exit
{  } // NOT CODED YET

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static inline void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{  } // NOT CODED YET

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   parm(argc, argv);                // Parameter analysis
   init();

   try {
     CC_MAIN();                     // Operate TLC
   } catch( pub::Exception& X) {
     debugf("catch %s\n", std::string(X).c_str());
   } catch( std::exception& X) {
     debugf("std::exception.what(%s)\n", X.what());
   } catch( const char* X) {
     debugf("catch(const char*(%s))\n", X);
   } catch(...) {
     debugf("Exception(...)\n");
   }

   term();
   if( USE_DEBUG ) debugf("TLC completed\n");

   return 0;
}
