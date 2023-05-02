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
//       Test_lex.CPP
//
// Purpose-
//       Test complex class.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include <com/complex.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TEST_LEX" // Source file, for debugging

//----------------------------------------------------------------------------
// Static Constants
//----------------------------------------------------------------------------
#define OPASSIGN                  0 // Operator =
#define OPADD                     1 // Operator +
#define OPSUB                     2 // Operator -
#define OPMUL                     3 // Operator *
#define OPDIV                     4 // Operator /
#define OPPLUS                    5 // Operator +=
#define OPIFEQ                    6 // Operator ==
#define OPIFNE                    7 // Operator !=
#define OPMAX                     8 // Number of defined operators

static const char*           opname[OPMAX]= {
   "=",
   "+",
   "-",
   "*",
   "/",
   "+=",
   "==",
   "!="
   };

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Show complex "result= f1 op f2"
//
//----------------------------------------------------------------------------
void
   show(
     int             const op,      // Operator
     complex*        const o1,      // Operand 1
     complex*        const o2)      // Operand 2
{
   int               vb;            // Boolean Resultant
   complex           vc;            // Complex Resultant
   char              c1[64];        // Character string
   char              c2[64];        // Character string
   char              cr[64];        // Character string

   switch (op)                      // Process operator
   {
     case OPASSIGN:                 // =
       vc= (*o1);
       vc.a(cr);
       o1->a(c1);
       printf("%s= %s\n", cr, c1);
       return;

     case OPADD:                    // +
       vc= (*o1) + (*o2);
       break;

     case OPSUB:                    // -
       vc= (*o1) - (*o2);
       break;

     case OPMUL:                    // *
       vc= (*o1) * (*o2);
       break;

     case OPDIV:                    // /
       vc= (*o1) / (*o2);
       break;

     case OPPLUS:                   // +=
       vc= (*o1);
       vc += (*o2);
       break;

     case OPIFEQ:                   // ==
       vb= (*o1) == (*o2);
       o1->a(c1);
       o2->a(c2);
       printf("%d= %s %s %s\n", vb, c1, opname[op], c2);
       return;

     case OPIFNE:                   // !=
       vb= (*o1) != (*o2);
       o1->a(c1);
       o2->a(c2);
       printf("%d= %s %s %s\n", vb, c1, opname[op], c2);
       return;

     default:                       // Invalid
       printf("Invalid opcode(%d)\n", op);
       return;
   }

   vc.a(cr);
   o1->a(c1);
   o2->a(c2);
   printf("%s= %s %s %s\n", cr, c1, opname[op], c2);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Show complex "result= f1 op f2"
//
//----------------------------------------------------------------------------
void
   show(
     int             const op,      // Operator
     complex         o1,            // Operand 1
     complex         o2)            // Operand 2
{
   int               vb;            // Boolean Resultant
   complex           vc;            // Complex Resultant
   char              c1[64];        // Character string
   char              c2[64];        // Character string
   char              cr[64];        // Character string

   switch (op)                      // Process operator
   {
     case OPASSIGN:                 // =
       vc= o1;
       vc.a(cr);
       o1.a(c1);
       printf("%s= %s\n", cr, c1);
       return;

     case OPADD:                    // +
       vc= o1 + o2;
       break;

     case OPSUB:                    // -
       vc= o1 - o2;
       break;

     case OPMUL:                    // *
       vc= o1 * o2;
       break;

     case OPDIV:                    // /
       vc= o1 / o2;
       break;

     case OPPLUS:                   // +=
       vc= o1;
       vc += o2;
       break;

     case OPIFEQ:                   // ==
       vb= o1 == o2;
       o1.a(c1);
       o2.a(c2);
       printf("%d= %s %s %s\n", vb, c1, opname[op], c2);
       return;

     case OPIFNE:                   // !=
       vb= o1 != o2;
       o1.a(c1);
       o2.a(c2);
       printf("%d= %s %s %s\n", vb, c1, opname[op], c2);
       return;

     default:                       // Invalid
       printf("Invalid opcode(%d)\n", op);
       return;
   }

   vc.a(cr);
   o1.a(c1);
   o2.a(c2);
   printf("%s= %s %s %s\n", cr, c1, opname[op], c2);
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
   complex           r, f1, f2;     // Complex values

   //-------------------------------------------------------------------------
   // Complex
   //-------------------------------------------------------------------------
   printf("\n");
   printf("Arithmetic operator tests:\n");
   f1= complex(-1,2);
   f2= complex( 3,4);

   show(OPASSIGN, &f1, &f1);
   show(OPASSIGN, &f2, &f2);

   printf("\n");
   show(OPADD, &f1, &f2);
   show(OPSUB, &f1, &f2);
   show(OPMUL, &f1, &f2);
   show(OPDIV, &f1, &f2);
   show(OPPLUS, &f1, &f2);

   printf("\n");
   show(OPADD, complex(1.0,2.0), complex(2.0));
   show(OPSUB, complex(1.0,2.0), complex(2.0));
   show(OPMUL, complex(1.0,2.0), complex(2.0));
   show(OPDIV, complex(1.0,2.0), complex(2.0));
   show(OPPLUS, complex(1.0,2.0), complex(2.0));

   printf("\n");
   show(OPADD, complex(2.0), complex(1.0,2.0));
   show(OPSUB, complex(2.0), complex(1.0,2.0));
   show(OPMUL, complex(2.0), complex(1.0,2.0));
   show(OPDIV, complex(2.0), complex(1.0,2.0));
   show(OPPLUS, complex(2.0), complex(1.0,2.0));

   printf("\n");
   printf("Division tests:\n");
   show(OPDIV, complex(+1.0,+0.0), complex(+2.0,+0.0));
   show(OPDIV, complex(+1.0,+0.0), complex(-2.0,+0.0));
   show(OPDIV, complex(-1.0,+0.0), complex(+2.0,+0.0));
   show(OPDIV, complex(-1.0,+0.0), complex(-2.0,+0.0));

   printf("\n");
   show(OPDIV, complex(+1.0,+0.0), complex(+0.0,+2.0));
   show(OPDIV, complex(+1.0,+0.0), complex(+0.0,-2.0));
   show(OPDIV, complex(-1.0,+0.0), complex(+0.0,+2.0));
   show(OPDIV, complex(-1.0,+0.0), complex(+0.0,-2.0));

   printf("\n");
   show(OPDIV, complex(+0.0,+1.0), complex(+2.0,+0.0));
   show(OPDIV, complex(+0.0,+1.0), complex(-2.0,+0.0));
   show(OPDIV, complex(+0.0,-1.0), complex(+2.0,+0.0));
   show(OPDIV, complex(+0.0,-1.0), complex(-2.0,+0.0));

   printf("\n");
   show(OPDIV, complex(+0.0,+1.0), complex(+0.0,+2.0));
   show(OPDIV, complex(+0.0,+1.0), complex(+0.0,-2.0));
   show(OPDIV, complex(+0.0,-1.0), complex(+0.0,+2.0));
   show(OPDIV, complex(+0.0,-1.0), complex(+0.0,-2.0));

   printf("\n");
   printf("Logical operator tests:\n");
   f1= complex(1.0,1.0);
   f2= f1 + complex(1.0);
   f2= complex(1.0) + f2;
   show(OPIFEQ, &f1, &f2);
   show(OPIFNE, &f1, &f2);

   f1= complex(1.2,1.2);
   f2= f1;
   show(OPIFEQ, &f1, &f2);
   show(OPIFNE, &f1, &f2);

   f1= complex(1,1) + complex(2,2);
   f2= complex(2,2) + complex(1,1);
   if (f1 == f2)
     printf("OK: f1=(1,1)+(2,2); f2=(2,2)+(1,1); (f1 == f2)\n");
   else
     printf("NG: f1=(1,1)+(2,2); f2=(2,2)+(1,1); (f1 != f2)\n");

#if 1
   f1= 0;
   if (f1=complex(1,1)+complex(2,2), f2=complex(2,2)+complex(1,1), f1 == f2)
     printf("OK: (f1=(1,1)+(2,2), f2=(2,2)+(1,1), f1 == f2)\n");
   else
     printf("NG: (f1=(1,1)+(2,2), f2=(2,2)+(1,1), f1 != f2)\n");
#endif

#if 1
   if (complex(1,1) + complex(2,2) == complex(2,2) + complex(1,1))
     printf("OK: (1,1)+(2,2)==(2,2)+(1,1)\n");
   else
     printf("NG: (1,1)+(2,2)!=(2,2)+(1,1)\n");
#endif

#if 1
   if ((complex(1,1) + complex(2,2)) == (complex(2,2) + complex(1,1)))
     printf("OK: ((1,1)+(2,2))==((2,2)+(1,1))\n");
   else
     printf("NG: ((1,1)+(2,2))!=((2,2)+(1,1))\n");
#endif

#if 1
   if (((complex(1,1) + complex(2,2)) == (complex(2,2) + complex(1,1))))
     printf("OK: (((1,1)+(2,2))==((2,2)+(1,1)))\n");
   else
     printf("NG: (((1,1)+(2,2))!=((2,2)+(1,1)))\n");
#endif

#if 1
   if (complex(complex(1,1) + complex(2,2)) == complex(complex(2,2) + complex(1,1)))
     printf("OK: complex((1,1)+(2,2))==complex((2,2)+(1,1))\n");
   else
     printf("NG: complex((1,1)+(2,2))!=complex((2,2)+(1,1))\n");
#endif

   if (complex(2,1) + complex(2,-1) == complex(4,0))
     printf("OK: (2,1)+(2,-1)==(4,0)\n");
   else
     printf("NG: (2,1)+(2,-1)!=(4,0)\n");

   if (complex(2,1) + complex(2,-1) == complex(4,1))
     printf("NG: (2,1)+(2,-1)==(4,1)\n");
   else
     printf("OK: (2,1)+(2,-1)!=(4,1)\n");

   return 0;
}

