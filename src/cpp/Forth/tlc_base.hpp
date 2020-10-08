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
//       tlc_base.hpp
//
// Purpose-
//       TLC built-in functions
//
// Last change date-
//       2020/10/04
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Exposed data areas
//----------------------------------------------------------------------------
static Word            BASE[]= {DEF_VAR, Word(10)}; // The numeric base

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const char*     cvttab= "0123456789abcdefghijklmnopqrstuvwxyz";
static const char*     CVTTAB= "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//----------------------------------------------------------------------------
// CABS: lhs :: ABS(lhs)
//----------------------------------------------------------------------------
static void CABS(void) {
   if( data.top() < 0 )
     data.push(-data.pop());
}
static Word TABS[] = {(Word)CABS};

//----------------------------------------------------------------------------
// CADD: lhs rhs :: lhs + rhs
//----------------------------------------------------------------------------
static void CADD(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs + rhs);
}
static Word TADD[] = {(Word)CADD};

//----------------------------------------------------------------------------
// CAND: lhs rhs :: lhs & rhs
//----------------------------------------------------------------------------
static void CAND(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs & rhs);
}
static Word TAND[] = {(Word)CAND};

//----------------------------------------------------------------------------
// CCLS: lhs rhs :: strcmp(lhs,rhs)
//----------------------------------------------------------------------------
static void CCLS(void) {
   const char* rhs= (const char*)data.pop();
   const char* lhs= (const char*)data.pop();
   data.push(strcmp(lhs,rhs));
}
static Word TCLS[] = {(Word)CCLS};

//----------------------------------------------------------------------------
// CDEBUG: Write debugging message
//----------------------------------------------------------------------------
static void CDEBUG( void ) {
   debug_op("DEBUG");
}
static Word TDEBUG[] = {(Word)CDEBUG};

//----------------------------------------------------------------------------
// CDEBUG_DUMP: Dump the stacks
//----------------------------------------------------------------------------
static void CDEBUG_DUMP( void ) {
   debug_op("DDUMP");

   debug_dump();
}
static Word TDEBUG_DUMP[] = {(Word)CDEBUG_DUMP};

//----------------------------------------------------------------------------
// CDEBUG_IMMW: Debug a thread list immediate
//----------------------------------------------------------------------------
static void CDEBUG_IMMW( void ) {
   debug_op("DIMMW");

   next_word(i_addr);
   char* ADDR= *(char**)i_addr;
   debug_list((void**)ADDR);
}
static Word TDEBUG_IMMW[] = {(Word)CDEBUG_IMMW};

//----------------------------------------------------------------------------
// CDEBUG_THIS: Debug this thread list
//----------------------------------------------------------------------------
static void CDEBUG_THIS( void ) {
   debug_op("DTHIS");

   debug_this();
}
static Word TDEBUG_THIS[] = {(Word)CDEBUG_THIS};

//----------------------------------------------------------------------------
// CDEC: Decrement top stack element. (Not to be confused with TH_DEC)
//----------------------------------------------------------------------------
static void CDEC(void) {
   Data item= data.pop();
   data.push(--item);
}
static Word TDEC[] = {(Word)CDEC};

//----------------------------------------------------------------------------
// CDIV: lhs rhs :: lhs / rhs
//----------------------------------------------------------------------------
static void CDIV(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs / rhs);
}
static Word TDIV[] = {(Word)CDIV};

//----------------------------------------------------------------------------
// CDIVR: lhs rhs :: (lhs / rhs) (lhs % rhs)
//----------------------------------------------------------------------------
static void CDIVR(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs / rhs);
   data.push(lhs % rhs);
}
static Word TDIVR[] = {(Word)CDIVR};

//----------------------------------------------------------------------------
// CDOT: lhs :: {} Top stack value printed
//----------------------------------------------------------------------------
static void CDOT(void)
{
static const bool      fancy_dec= true; // Fancy decimal? (thousands)
static const bool      fancy_hex= true; // Fancy hexidecimal?

   unsigned char       buff[256];   // Output buffer
   unsigned char       work[256];   // Work buffer
   size_t              value= (size_t )BASE[1]; // Get working base
   unsigned int        base= value & 0x000000FF; // (Max is actually 36)

   value= data.pop();               // Get the current value (unsigned)
   int is_negative= false;          // Need a minus sign?
   if( base == 10 && (Data)value < 0 ) // Only base 10 is signed
   {
     is_negative= true;
     value= -value;
   }

   unsigned int digits= 0;          // (For fancy_dec, number of digits)
   unsigned int size= 0;            // Number of data bytes
   while( value != 0 )              // Extract the value
   {
     // Fancy-dancy option to have decimal thousands separators
     if( fancy_dec )
     {
       if( base == 10 && digits > 0 && (digits % 3) == 0 )
         work[size++]= ',';

       digits++;
     }

     work[size++]= CVTTAB[value % base];
     value /= base;
   }
   if( size == 0 )
     work[size++]= '0';

   // HEX: Special case: expand size to 16 or 32
   if( fancy_hex && base == 16 )
   {
     if( size >= 8 )
     {
       while( size < 16 )
         work[size++]= '0';
     } else {
       while( size < 8 )
         work[size++]= '0';
     }

     // Fancy-dancy option to have hex leading 0x
     work[size++]= 'x';
     work[size++]= '0';
   }

   if( is_negative )
     work[size++]= '-';

   // Reverse the work buffer into the output buffer
   for(unsigned i= 0; i<size; i++)
   {
     int j= size - i - 1;
     buff[i]= work[j];
   }
   buff[size]= '\0';

   printf("%s ", buff);
}
static Word TDOT[] = {(Word)CDOT};

//----------------------------------------------------------------------------
// CDUP: rhs :: rhs rhs
//----------------------------------------------------------------------------
static void CDUP(void) {
   data.push(data.top());
}
static Word TDUP[] = {(Word)CDUP};

//----------------------------------------------------------------------------
//
// Subroutine-
//       CGET
//
// Purpose-
//       Get input string
//
// Inputs-
//       (maximum length) (@string)
//
// Outputs-
//       (actual length)
//
// Implementation notes-
//       No longer actively used (or maintained.)
//
//----------------------------------------------------------------------------
static void
   CGET( void )                     // Get input string
{
   char* const       S= (char*)data.pop();
   Data              L= data.pop();
debugf("<<CGET S(%p) L(%ld)\n", S, L);

   char* C= fgets(S, L, stdin);
   L= 0;                            // Default, zero length
   if( C != nullptr )
   {
     L= strlen(S);
     if( L > 0 && S[L-1] == '\n' )
       S[--L]= '\0';
   }
   else
     operational= false;            // QUIT if EOF

pub::utility::dump(stdout, S, L+1);
debugf(">>CGET S(%s) L(%ld)\n", S, L);
   data.push(L);
}
static Word TGET[] = {(Word)CGET};

//----------------------------------------------------------------------------
// CIFEQZ: (TOP) :: {} If (TOP) == 0, process GOTO function
//----------------------------------------------------------------------------
static void CIFEQZ(void) {
   Data item= data.pop();
   next_word(i_addr);
   if( item == 0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFEQZ[] = {(Word)CIFEQZ};

//----------------------------------------------------------------------------
// CIFGEZ: (TOP) :: {} If (TOP) >= 0, process GOTO function
//----------------------------------------------------------------------------
static void CIFGEZ(void) {
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs >= 0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFGEZ[] = {(Word)CIFGEZ};

//----------------------------------------------------------------------------
// CIFGTZ: If (TOP) >  0, process GOTO function
//----------------------------------------------------------------------------
static void CIFGTZ(void) {
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs >  0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFGTZ[] = {(Word)CIFGTZ};

//----------------------------------------------------------------------------
// CIFLEZ: If (TOP) <= 0, process GOTO function
//----------------------------------------------------------------------------
static void CIFLEZ(void) {
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs <= 0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFLEZ[] = {(Word)CIFLEZ};

//----------------------------------------------------------------------------
// CIFLTZ: If (TOP) <  0, process GOTO function
//----------------------------------------------------------------------------
static void CIFLTZ(void) {
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs <  0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFLTZ[] = {(Word)CIFLTZ};

//----------------------------------------------------------------------------
// CIFNEZ: If (TOP) != 0, process GOTO function
//----------------------------------------------------------------------------
static void CIFNEZ(void) {
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs != 0 )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFNEZ[] = {(Word)CIFNEZ};

//----------------------------------------------------------------------------
// CIFEQ: If (TOP-1) == (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFEQ(void) {
   Data rhs= data.pop(); // (TOP)
   Data lhs= data.pop(); // (TOP-1)
   next_word(i_addr);
   if( lhs == rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFEQ[] = {(Word)CIFEQ};

//----------------------------------------------------------------------------
// CIFGE: If (TOP-1) >= (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFGE(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs >= rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFGE[] = {(Word)CIFGE};

//----------------------------------------------------------------------------
// CIFGT: If (TOP-1) >  (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFGT(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs >  rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFGT[] = {(Word)CIFGT};

//----------------------------------------------------------------------------
// CIFLE: If (TOP-1) <= (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFLE(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs <= rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFLE[] = {(Word)CIFLE};

//----------------------------------------------------------------------------
// CIFLT: If (TOP-1) <  (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFLT(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs <  rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFLT[] = {(Word)CIFLT};

//----------------------------------------------------------------------------
// CIFNE: If (TOP-1) != (TOP), process GOTO function
//----------------------------------------------------------------------------
static void CIFNE(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   next_word(i_addr);
   if( lhs != rhs )
     set_iaddr(*(Word*)i_addr);
}
static Word TIFNE[] = {(Word)CIFNE};

//----------------------------------------------------------------------------
// CIMMW: Push the next program Word onto the Stack
//----------------------------------------------------------------------------
static void CIMMW(void) {
   next_word(i_addr);
   Word word= *(Word*)i_addr;
   data.push(Data(word));
}
static Word TIMMW[] = {(Word)CIMMW};

//----------------------------------------------------------------------------
// CINC: Increment top stack element
//----------------------------------------------------------------------------
static void CINC(void) {
   Data item= data.pop();
   data.push(++item);
}
static Word TINC[] = {(Word)CINC};

//----------------------------------------------------------------------------
// CMAX: rhs= pop, lhs=pop; push(max(lhs,rhs))
//----------------------------------------------------------------------------
static void CMAX(void) {
   Data rhs= data.pop();
   if( rhs > data.top() )
   {
     data.pop();
     data.push(rhs);
   }
}
static Word TMAX[] = {(Word)CMAX};

//----------------------------------------------------------------------------
// CMIN: rhs= pop, lhs=pop; push(min(lhs,rhs))
//----------------------------------------------------------------------------
static void CMIN(void) {
   Data rhs= data.pop();
   if( rhs < data.top() )
   {
     data.pop();
     data.push(rhs);
   }
}
static Word TMIN[] = {(Word)CMIN};

//----------------------------------------------------------------------------
// CMOD: rhs= pop, lhs=pop; push(lhs % rhs)
//----------------------------------------------------------------------------
static void CMOD(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs % rhs);
}
static Word TMOD[] = {(Word)CMOD};

//----------------------------------------------------------------------------
// CMUL: rhs= pop, lhs=pop; push(lhs * rhs)
//----------------------------------------------------------------------------
static void CMUL(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs * rhs);
}
static Word TMUL[] = {(Word)CMUL};

//----------------------------------------------------------------------------
// CNEG: lhs= pop; push(-lhs)
//----------------------------------------------------------------------------
static void CNEG(void) {
   Data lhs= data.pop();
   data.push(-lhs);
}
static Word TNEG[] = {(Word)CNEG};

//----------------------------------------------------------------------------
// CNOP: No operation
//----------------------------------------------------------------------------
static void CNOP(void) {
}
static Word TNOP[] = {(Word)CNOP};

//----------------------------------------------------------------------------
// CNOT: lhs= pop; push(!lhs)
//----------------------------------------------------------------------------
static void CNOT(void) {
   Data lhs= data.pop();
   data.push(!lhs);
}
static Word TNOT[] = {(Word)CNOT};

//----------------------------------------------------------------------------
// COR: rhs= pop, lhs=pop; push(lhs | rhs)
//----------------------------------------------------------------------------
static void COR(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs | rhs);
}
static Word TOR[] = {(Word)COR};

//----------------------------------------------------------------------------
// COUTC: Write character to console. ('\n' converted to "\r\n")
//----------------------------------------------------------------------------
static void COUTC(void) {
   Data out= data.pop();
   if( out == '\r' )
     return;

   if( out == '\n' )
     putchar('\r');
   putchar(out);
}
static Word TOUTC[] = {(Word)COUTC};

//----------------------------------------------------------------------------
// COVER: Copy the next to the top element to the top of the Stack
//----------------------------------------------------------------------------
static void COVER(void) {
   Data one= data.pop();
   Data two= data.top();
   data.push(one);
   data.push(two);
}
static Word TOVER[] = {(Word)COVER};

//----------------------------------------------------------------------------
// CPEEKC: addr :: *(unsigned char*)addr
//----------------------------------------------------------------------------
static void CPEEKC(void) {
   Word addr= Word(data.pop());
   IFCHECK(
     if( addr == nullptr )
     {
       debugf("ERROR: nullptr PEEK detected\n");
       operational= false;
       return;
     }
   )

   Data item= *(unsigned char*)addr;
   data.push(item);
}
static Word TPEEKC[] = {(Word)CPEEKC};

//----------------------------------------------------------------------------
// CPEEKW: addr :: (*addr)
//----------------------------------------------------------------------------
static void CPEEKW(void) {
   Word addr= Word(data.pop());
   IFCHECK(
     if( addr == nullptr )
     {
       debugf("ERROR: nullptr PEEK detected\n");
       operational= false;
       return;
     }
   )

   data.push(Data(*(Word*)addr));
}
static Word TPEEKW[] = {(Word)CPEEKW};

//----------------------------------------------------------------------------
// CPOKEC: value addr :: {} *(char)addr= value
//----------------------------------------------------------------------------
static void CPOKEC(void) {
   Word addr= Word(data.pop());
   Data item= data.pop();
   IFCHECK(
     if( addr == nullptr )
     {
       debugf("ERROR: nullptr POKE detected\n");
       operational= false;
       return;
     }
   )

   *(unsigned char*)addr= item;
}
static Word TPOKEC[] = {(Word)CPOKEC};

//----------------------------------------------------------------------------
// CPOKEW: value addr :: {} *addr= item
//----------------------------------------------------------------------------
static void CPOKEW(void) {
   Word addr= Word(data.pop());
   Data item= data.pop();
   IFCHECK(
     if( addr == nullptr )
     {
       debugf("ERROR: nullptr POKE detected\n");
       operational= false;
       return;
     }
   )

   *(Data*)addr= item;
}
static Word TPOKEW[] = {(Word)CPOKEW};

//----------------------------------------------------------------------------
//
// Subroutine-
//       TPUTI
//
// Purpose-
//       Write standard C-string
//
// Input-
//       Next word contains string address
//
//----------------------------------------------------------------------------
static void
   CPUTI( void )                    // Write output string (immediate)
{
   next_word(i_addr);
   unsigned char* S= *(unsigned char**)i_addr;

   printf("%s", S);
}
static Word TPUTI[] = {(Word)CPUTI};

//----------------------------------------------------------------------------
//
// Subroutine-
//       TPUTS
//
// Purpose-
//       Write standard C-string
//
// Input-
//       Top contains string address
//
//----------------------------------------------------------------------------
static void
   CPUTS( void )                    // Write output string
{
   unsigned char*    S= *(unsigned char**)data.pop();

   printf("%s", S);
}
static Word TPUTS[] = {(Word)CPUTS};

//----------------------------------------------------------------------------
// CQUIT: Terminate operation
//----------------------------------------------------------------------------
static void CQUIT(void) {
   operational= false;
}
static Word TQUIT[] = {(Word)CQUIT};

//----------------------------------------------------------------------------
// CSUB: rhs= pop, lhs=pop; push(lhs - rhs)
//----------------------------------------------------------------------------
static void CSUB(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs - rhs);
}
static Word TSUB[] = {(Word)CSUB};

//----------------------------------------------------------------------------
// CSWAP: Swap the top two Stack elements
//----------------------------------------------------------------------------
static void CSWAP(void) {
   Data one= data.pop();
   Data two= data.pop();
   data.push(one);
   data.push(two);
}
static Word TSWAP[] = {(Word)CSWAP};

//----------------------------------------------------------------------------
// TPOP: Pop (ignore) the top Data Stack item
//----------------------------------------------------------------------------
static void CPOP(void) {
   data.pop();
}
static Word TPOP[] = {Word(CPOP)};

//----------------------------------------------------------------------------
// CXOR: rhs= pop, lhs=pop; push(lhs ^ rhs)
//----------------------------------------------------------------------------
static void CXOR(void) {
   Data rhs= data.pop();
   Data lhs= data.pop();
   data.push(lhs ^ rhs);
}
static Word TXOR[] = {(Word)CXOR};
