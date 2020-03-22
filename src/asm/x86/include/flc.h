/*----------------------------------------------------------------------------
**
**       Copyright (C) 2010 Frank Eskesen.
**
**       This file is free content, distributed under the MIT license.
**       (See accompanying file LICENSE.MIT or the original contained
**       within https://opensource.org/licenses/MIT)
**
**--------------------------------------------------------------------------*/
/*                                                                          */
/* Title-                                                                   */
/*       flc.h                                                              */
/*                                                                          */
/* Purpose-                                                                 */
/*       Fixed Low Core definitions for X86.                                */
/*                                                                          */
/* Last change date-                                                        */
/*       2010/01/01                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/
#ifndef X86FLC_H_INCLUDED
#define X86FLC_H_INCLUDED

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* Struct-                                                                  */
/*       flc                                                                */
/*                                                                          */
/* Purpose-                                                                 */
/*       Fixed Low Core definitions.                                        */
/*                                                                          */
/*--------------------------------------------------------------------------*/
struct flc {                        /* Fixed Low Core                       */
/*--------------------------------------------------------------------------*/
/* Low core vectors                                                         */
/*--------------------------------------------------------------------------*/
   char                _000000[0x000400]; /* Undefined                      */

/*--------------------------------------------------------------------------*/
/* BIOS Data Areas                                                          */
/*--------------------------------------------------------------------------*/
   uint16              com1Port;    /* 0x000400 COM1 port base address      */
   uint16              com2Port;    /* 0x000402 COM2 port base address      */
   uint16              com3Port;    /* 0x000404 COM3 port base address      */
   uint16              com4Port;    /* 0x000406 COM4 port base address      */

   uint16              lpr1Port;    /* 0x000408 LPR1 port base address      */
   uint16              lpr2Port;    /* 0x00040a LPR2 port base address      */
   uint16              lpr3Port;    /* 0x00040c LPR3 port base address      */
   uint16              lpr4Port;    /* 0x00040e LPR4 port base address      */

   uint16              hw0410;      /* 0x000410 Installed hardware          */
#define HW0410_LPRN        0x0000c0 /* + Number of LPR adaptors             */
#define HW0410_MODEM       0x00002e /* + Internal modem present             */
#define HW0410_COMN        0x00000e /* + Number of COM adaptors             */
#define HW0410_FDN         0x00c000 /* + Number of floppy disks             */
#define HW0410_VMODE       0x003000 /* + Video mode                         */
#define HW0410_VMODE0      0x000000 /* + Video mode 0 (undefined)           */
#define HW0410_VMODE1      0x001000 /* + Video mode 1 (40x25 color)         */
#define HW0410_VMODE2      0x002000 /* + Video mode 0 (80x25 color)         */
#define HW0410_VMODE3      0x003000 /* + Video mode 0 (80x25 mono)          */
#define HW0410_MOUSE       0x000400 /* + Mouse present                      */
#define HW0410_COPROC      0x000200 /* + Math coprocessor present           */
#define HW0410_FDBOOT      0x000100 /* + IPL diskette                       */
#define HW0410_RESERVED    0x000811 /* + Reserved                           */

   uint8               hw0412;      /* 0x000412 Installed hardware          */
#define HW0412_POST        0x0000ff /* + Power-on self-test status          */
#define HW0412_RTC         0x000002 /* + Real Time Clock installed          */
#define HW0412_REMAP       0x000001 /* + Memory remapped                    */

   uint16              hw0413;      /* 0x000413 Memory size, in KB          */
#define FLC_MEMSIZE    hw0413

   uint16              _000415;     /* 0x000415 Reserved                    */

   uint8               hw0417;      /* 0x000417 Keyboard control            */
#define FLC_KEYBOARD0  hw0417

   uint8               hw0418;      /* 0x000418 Keyboard control            */
#define FLC_KEYBOARD1  hw0418

   uint8               hw0419;      /* 0x000419 Alternate keypad entry      */
   uint16              hw041a;      /* 0x00041a Keyboard head pointer       */
#define FLC_KEYHEAD    hw041a

   uint16              hw041c;      /* 0x00041c Keyboard tail pointer       */
#define FLC_KEYTAIL    hw041c

   uint8               hw041e[32];  /* 0x00041e Keyboard buffer             */
#define FLC_KEYBUFF    hw041e

   uint8               hw043e;      /* 0x00043e Recalibrate status          */
   uint8               hw043f;      /* 0x00043f Motor status                */
   uint8               hw0440;      /* 0x000440 Motor off counter           */
   uint8               hw0441;      /* 0x000441 Last diskette drive status  */
   uint8               hw0442[7];   /* 0x000442 Diskette controller status  */
   uint8               hw0449;      /* 0x000449 Display mode set            */
   uint16              hw044a;      /* 0x00044a Number of columns           */
   uint16              hw044c;      /* 0x00044c Length of regen buffer      */
   uint16              hw044e;      /* 0x00044e Start address in regen buff */
   uint16              hw0450;      /* 0x000450 Cursor position page 1      */
   uint16              hw0452;      /* 0x000452 Cursor position page 2      */
   uint16              hw0454;      /* 0x000454 Cursor position page 3      */
   uint16              hw0456;      /* 0x000456 Cursor position page 4      */
   uint16              hw0458;      /* 0x000458 Cursor position page 5      */
   uint16              hw045a;      /* 0x00045a Cursor position page 6      */
   uint16              hw045c;      /* 0x00045c Cursor position page 7      */
   uint16              hw045e;      /* 0x00045e Cursor position page 8      */
   uint16              hw0460;      /* 0x000460 Cursor type                 */
   uint8               hw0462;      /* 0x000462 Display page                */
   uint16              hw0463;      /* 0x000463 Display controller base addr*/
   uint8               hw0465;      /* 0x000465 Current 3x8 register setting*/
   uint8               hw0466;      /* 0x000466 Current 3x9 register setting*/
   uint32              hw0467;      /* 0x000467 Reset code (on reset)       */
   uint8               hw046b;      /* 0x00046b Reserved                    */
   uint32              hw046c;      /* 0x00046c Timer counter               */
   uint8               hw0470;      /* 0x000470 Timer overflow counter      */
   uint8               hw0471;      /* 0x000471 Break key state             */
   uint16              hw0472;      /* 0x000472 Reset flag                  */
   uint8               hw0474;      /* 0x000474 Last fixed disk status      */
   uint8               hw0475;      /* 0x000475 Number of fixed disks       */
   uint8               hw0476;      /* 0x000476 Reserved                    */
   uint8               hw0477;      /* 0x000477 Reserved                    */
   uint8               hw0478;      /* 0x000478 LPR1 time-out value         */
   uint8               hw0479;      /* 0x000479 LPR2 time-out value         */
   uint8               hw047a;      /* 0x00047a LPR3 time-out value         */
   uint8               hw047b;      /* 0x00047b LPR4 time-out value         */
   uint8               hw047c;      /* 0x00047c COM1 time-out value         */
   uint8               hw047d;      /* 0x00047d COM2 time-out value         */
   uint8               hw047e;      /* 0x00047e COM3 time-out value         */
   uint8               hw047f;      /* 0x00047f COM4 time-out value         */
   uint16              hw0480;      /* 0x000480 Keyboard buff head offset   */
   uint16              hw0482;      /* 0x000482 Keyboard buff tail offset   */
   uint8               hw0484;      /* 0x000484 Number of screen rows       */
   uint16              hw0485;      /* 0x000485 Character height (bytes)    */
   uint8               hw0487;      /* 0x000487 Video control states        */
   uint8               hw0488;      /* 0x000488 Video control states        */
   uint8               hw0489;      /* 0x000489 Reserved                    */
   uint8               hw048a;      /* 0x00048a Reserved                    */
   uint8               hw048b;      /* 0x00048b Media control               */
   uint8               hw048c;      /* 0x00048c Fixed disk controller status*/
   uint8               hw048d;      /* 0x00048d " error status              */
   uint8               hw048e;      /* 0x00048e " interrupt control         */
   uint8               hw048f;      /* 0x00048f Reserved                    */
   uint8               hw0490[8];   /* 0x000490 Diskette control            */
   uint8               hw0498[16];  /* 0x000498 RTC data areas              */
   uint8               hw04a8[4];   /* 0x0004a8 Video control               */
   uint8               hw04ac[4];   /* 0x0004ac Reserved                    */
   uint8               hw04b0[16];  /* 0x0004b0 Reserved                    */
   uint8               hw04c0[16];  /* 0x0004c0 Reserved                    */
   uint8               hw04d0[16];  /* 0x0004d0 Reserved                    */
   uint8               hw04e0[16];  /* 0x0004e0 Reserved                    */
   uint8               hw04f0[16];  /* 0x0004f0 Reserved                    */
   uint8               hw0500;      /* 0x000500 Print screen status         */
   uint8               hw0501[255]; /* 0x000501 Reserved                    */
   uint8               hw0600[256]; /* 0x000600 Reserved                    */
   uint8               hw0700[256]; /* 0x000700 Reserved                    */
   uint8               hw0800[256*16]; /* 0x000800 Reserved                 */
   uint8               hw1000[256*32]; /* 0x001000 Reserved                 */
   uint8               hw2000[256*32]; /* 0x002000 Reserved                 */
   uint8               hw3000[256*32]; /* 0x003000 Reserved                 */
   uint8               hw4000[256*32]; /* 0x004000 Reserved                 */
   uint8               hw5000[256*32]; /* 0x005000 Reserved                 */
   uint8               hw6000[256*32]; /* 0x006000 Reserved                 */
   uint8               hw7000[256*30]; /* 0x007000 Reserved                 */

/*--------------------------------------------------------------------------*/
/* Boot area                                                                */
/*--------------------------------------------------------------------------*/
#define FLC_BOOT       hw7c00       /* Boot entry point                     */
   char                hw7c00[512]; /* Boot record 0                        */
   char                hw7e00[512]; /* Boot record 1                        */

/*--------------------------------------------------------------------------*/
/* User control area                                                        */
/*--------------------------------------------------------------------------*/
   char                hw8000[0x008000]; /* User control area               */
}; /* struct flc */

#endif /* X86FLC_H_INCLUDED */
