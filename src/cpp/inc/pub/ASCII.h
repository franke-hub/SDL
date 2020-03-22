//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/ASCII.h
//
// Purpose-
//       Enumerate the ASCII character set.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       This is only intended for use as a reference.
//
//----------------------------------------------------------------------------
#ifndef _PUB_ASCII_H_INCLUDED
#define _PUB_ASCII_H_INCLUDED

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE::detail::unused {
//----------------------------------------------------------------------------
//
// Enum-
//       ASCII
//
// Purpose-
//       Enumerate the ASCII character set.
//
//----------------------------------------------------------------------------
enum ASCII                          // ASCII enumeration
{  NUL=                0x00         // '\0' NULL character
,  SOH=                0x01         // Start Of Heading
,  STX=                0x02         // Start of TeXt
,  ETX=                0x03         // End of TeXt
,  EOT=                0x04         // End Of Transmission
,  ENQ=                0x05         // ENQuery
,  ACK=                0x06         // ACKnowledge
,  BEL=                0x07         // '\a' Alarm (BELl)
,  BS=                 0x08         // '\b' BackSpace
,  HT=                 0x09         // '\t' Horizontal Tab
,  LF=                 0x0A         // '\n' Line Feed
,  VT=                 0x0B         // '\v' Vertical Tab
,  FF=                 0x0C         // '\f' Form Feed
,  CR=                 0x0D         // '\r' Carriage Return
,  SO=                 0x0E         // Shift Out
,  SI=                 0x0F         // Shift In
,  DLE=                0x10         // Data Link Escape
,  DC1=                0x11         // Device Control 1
,  DC2=                0x12         // Device Control 2
,  DC3=                0x13         // Device Control 3
,  DC4=                0x14         // Device Control 4
,  NAK=                0x15         // Negative AcKnowledge
,  SYN=                0x16         // SYNchronize
,  ETB=                0x17         // End of Transmission Block
,  CAN=                0x18         // CANcel
,  EM=                 0x19         // End of Media
,  SUB=                0x1A         // SUBstitute
,  ESC=                0x1B         // ESCape
,  FS=                 0x1C         // Field Separator
,  GS=                 0x1D         // Group Separator
,  RS=                 0x1E         // Record Separator
,  US=                 0x1F         // Unit Separator
,  SP=                 0x20         // SPace

,  Space=              ' '          // 0x20
,  ExclamationPoint=   '!'          // 0x21
,  QuotationMark=      '"'          // 0x22
,  PoundSign=          '#'          // 0x23
,  DollarSign=         '$'          // 0x24
,  PercentSign=        '%'          // 0x25
,  Ampersand=          '&'          // 0x26
,  Quote=              '\''         // 0x27
,  LeftParenthesis=    '('          // 0x28
,  RightParenthesis=   ')'          // 0x29
,  Asterisk=           '*'          // 0x2A
,  PlusSign=           '+'          // 0x2B
,  Comma=              ','          // 0x2C
,  Hyphen=             '-'          // 0x2D
,  Period=             '.'          // 0x2E
,  RightSlash=         '/'          // 0x2F

,  _0_=                '0'          // 0x30
,  _1_=                '1'          // 0x31
,  _2_=                '2'          // 0x32
,  _3_=                '3'          // 0x33
,  _4_=                '4'          // 0x34
,  _5_=                '5'          // 0x35
,  _6_=                '6'          // 0x36
,  _7_=                '7'          // 0x37
,  _8_=                '8'          // 0x38
,  _9_=                '9'          // 0x39

,  Colon=              ':'          // 0x3A
,  Semicolon=          ';'          // 0x3B
,  LeftAngle=          '<'          // 0x3C
,  EqualSign=          '='          // 0x3D
,  RightAngle=         '>'          // 0x3E
,  QuestionMark=       '?'          // 0x3F
,  AtSign=             '@'          // 0x40

,  _A_=                'A'          // 0x41
,  _B_=                'B'          // 0x42
,  _C_=                'C'          // 0x43
,  _D_=                'D'          // 0x44
,  _E_=                'E'          // 0x45
,  _F_=                'F'          // 0x46
,  _G_=                'G'          // 0x47
,  _H_=                'H'          // 0x48
,  _I_=                'I'          // 0x49
,  _J_=                'J'          // 0x4A
,  _K_=                'K'          // 0x4B
,  _L_=                'L'          // 0x4C
,  _M_=                'M'          // 0x4D
,  _N_=                'N'          // 0x4E
,  _O_=                'O'          // 0x4F
,  _P_=                'P'          // 0x50
,  _Q_=                'Q'          // 0x51
,  _R_=                'R'          // 0x52
,  _S_=                'S'          // 0x53
,  _T_=                'T'          // 0x54
,  _U_=                'U'          // 0x55
,  _V_=                'V'          // 0x56
,  _W_=                'W'          // 0x57
,  _X_=                'X'          // 0x58
,  _Y_=                'Y'          // 0x59
,  _Z_=                'Z'          // 0x5A

,  LeftBracket=        '['          // 0x5B
,  LeftSlash=          '\\'         // 0x5C
,  RightBracket=       ']'          // 0x5D
,  Carat=              '^'          // 0x5E
,  Underscore=         '_'          // 0x5F
,  GraveAccent=        '`'          // 0x60

,  _a_=                'a'          // 0x61
,  _b_=                'b'          // 0x62
,  _c_=                'c'          // 0x63
,  _d_=                'd'          // 0x64
,  _e_=                'e'          // 0x65
,  _f_=                'f'          // 0x66
,  _g_=                'g'          // 0x67
,  _h_=                'h'          // 0x68
,  _i_=                'i'          // 0x69
,  _j_=                'j'          // 0x6A
,  _k_=                'k'          // 0x6B
,  _l_=                'l'          // 0x6C
,  _m_=                'm'          // 0x6D
,  _n_=                'n'          // 0x6E
,  _o_=                'o'          // 0x6F
,  _p_=                'p'          // 0x70
,  _q_=                'q'          // 0x71
,  _r_=                'r'          // 0x72
,  _s_=                's'          // 0x73
,  _t_=                't'          // 0x74
,  _u_=                'u'          // 0x75
,  _v_=                'v'          // 0x76
,  _w_=                'w'          // 0x77
,  _x_=                'x'          // 0x78
,  _y_=                'y'          // 0x79
,  _z_=                'z'          // 0x7A

,  LeftBrace=          '{'          // 0x7B
,  Bar=                '|'          // 0x7C
,  RightBrace=         '}'          // 0x7D
,  Tilde=              '~'          // 0x7E
,  DEL=                0x7F         // DELete

// Values 0x80 through 0xFF are not defined by ASCII.
// See https://www.ascii-codes.com/ for differing code page implementations.
}; // enum ASCII
}  // namespace _PUB_NAMESPACE::detail::unused

#endif // _PUB_ASCII_H_INCLUDED
