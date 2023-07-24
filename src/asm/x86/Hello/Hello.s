##----------------------------------------------------------------------------
##
##       Copyright (C) 2010 Frank Eskesen.
##
##       This file is free content, distributed under creative commons CC0,
##       explicitly released into the Public Domain.
##       (See accompanying html file LICENSE.ZERO or the original contained
##       within https://creativecommons.org/publicdomain/zero/1.0/legalcode)
##
##----------------------------------------------------------------------------
##
## Title-
##       Hello.s
##
## Purpose-
##       Sample assembler program.
##
## Notes-
##       Generated from Source.cpp using:
##           export OPTIMIZE=-S
##           make Source.o
##
##----------------------------------------------------------------------------
        .file   "Source.cpp"
        .def    ___main;        .scl    2;      .type   32;     .endef
        .section .rdata,"dr"
LC0:
        .ascii "Hello ASM World\12\0"
        .text
        .align 2
.globl _main
        .def    _main;  .scl    2;      .type   32;     .endef
_main:
        pushl   %ebp
        movl    %esp, %ebp
        subl    $8, %esp
        andl    $-16, %esp
        movl    $0, %eax
        movl    %eax, -4(%ebp)
        movl    -4(%ebp), %eax
        call    __alloca
        call    ___main
        movl    $LC0, (%esp)
        call    _printf
        movl    $0, %eax
        leave
        ret
        .def    _printf;        .scl    2;      .type   32;     .endef
