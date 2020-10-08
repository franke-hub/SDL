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
//       snap.cpp
//
// Purpose-
//       Sample snap standalone function. Demo prints ascii characters.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       snap
//
// Purpose-
//       Memory snap dump, copy as needed.
//
//----------------------------------------------------------------------------
#define ROUNDING 16
static const char* hextab = "0123456789abcdef";
static void snap(const void* _addr, long _size) {
    char               buff[ROUNDING+1];
    const char*        addr= (const char*)((size_t)_addr & (~(ROUNDING-1)));
    long               size= _size + ((const char*)_addr - addr);

    size += (ROUNDING-1);
    size &= ~(ROUNDING-1);

    printf("addr: %.8zx, size: %.8lx\n", (size_t)_addr, _size);
    buff[ROUNDING] = '\0';
    while( size > 0 ) {
        int i,j;
        memcpy(buff, addr, ROUNDING);
        printf("%.8zx", (size_t)addr);
        for(i= 0; i<4; i++) {
            printf(" ");
            for(j= 0; j<4; j++) {
                int x = i*4 + j;
                int c = buff[x] & 0x00ff;
                printf("%c%c", hextab[c>>4], hextab[c&0x000f]);
            }
        }

        for(i = 0; i<ROUNDING; i++) {
            int c = buff[i] & 0x00ff;
            if( c < 0x0020 || c >= 0x007f )
                buff[i] = '.';
        }

        printf("  *%s*\n", buff);
        addr += ROUNDING;
        size -= ROUNDING;
    }
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
int                                 // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   const char*         test =
                           " !\"#$%&\'()*+,-./"
                           "0123456789"
                           ":;<=>?@"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "[\\]^_`"
                           "abcdefghijklmnopqrstuvwxyz"
                           "{|}~\x7f\a\b\t\v\r\n"
                           "This is a test. It is only a test.\n"
                           "Certainly there is no need to PANIC!";

   snap(test, strlen(test)+1);
   snap(test+1, strlen(test));
   return 0;
}

