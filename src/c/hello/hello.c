//----------------------------------------------------------------------------
//
//       Copyright (C) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the "un-license,"
//       explicitly released into the Public Domain.
//       (See accompanying file LICENSE.UNLICENSE or the original
//       contained within http://unlicense.org)
//
//----------------------------------------------------------------------------
//
// Title-
//       hello.c
//
// Purpose-
//       Hello world, using a library of do-nothing functions.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

//----------------------------------------------------------------------------
// Function prototypes
//----------------------------------------------------------------------------
extern int test00( void );
extern int test01( void );
extern int test02( void );
extern int test03( void );

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
   unsigned int      errorCount= 0;

   errorCount += test00();
   errorCount += test01();
   errorCount += test02();
   errorCount += test03();

   printf("Hello world\n");

   return errorCount;
}

