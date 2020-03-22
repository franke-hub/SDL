//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ScanCode.h
//
// Purpose-
//       Enumerate scan codes for the IBM PC.
//
// Last change date-
//       2007/01/01
//
// See also-
//       ASCII.h, KeyCode.h
//
//----------------------------------------------------------------------------
#ifndef SCANCODE_H_INCLUDED
#define SCANCODE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       ScanCode
//
// Purpose-
//       Enumerate the "IBM compatible" keyboard scan codes.
//
//----------------------------------------------------------------------------
class ScanCode {                    // IBM compatible keyboard scan codes
public:
enum                                // IBM compatible keyboard scan codes
{  ESC=                0x01         // ESCape key
,  _1_=                0x02         // 1 ! key
,  _2_=                0x03         // 2 @ key
,  _3_=                0x04         // 3 # key
,  _4_=                0x05         // 4 $ key
,  _5_=                0x06         // 5 % key
,  _6_=                0x07         // 6 ^ key
,  _7_=                0x08         // 7 & key
,  _8_=                0x09         // 8 * key
,  _9_=                0x0A         // 9 ( key
,  _0_=                0x0B         // 0 ) key
,  Hyphen=             0x0C         // - _ key
,  EqualSign=          0x0D         // = + key
,  Backspace=          0x0E         // Backspace key
,  TAB=                0x0F         // TAB key
,  _q_=                0x10         // q Q key
,  _w_=                0x11         // w W key
,  _e_=                0x12         // e E key
,  _r_=                0x13         // r R key
,  _t_=                0x14         // t T key
,  _y_=                0x15         // y Y key
,  _u_=                0x16         // u U key
,  _i_=                0x17         // i I key
,  _o_=                0x18         // o O key
,  _p_=                0x19         // p P key
,  LeftBracket=        0x1A         // [ { key
,  RightBracket=       0x1B         // ] } key
,  ENTER=              0x1C         // ENTER key
,  LeftControl=        0x1D         // Left control key
,  RightControl=       0x1D         // Right control key
,  _a_=                0x1E         // a A key
,  _s_=                0x1F         // s S key
,  _d_=                0x20         // d D key
,  _f_=                0x21         // f F key
,  _g_=                0x22         // g G key
,  _h_=                0x23         // h H key
,  _j_=                0x24         // j J key
,  _k_=                0x25         // k K key
,  _l_=                0x26         // l L key
,  Semicolon=          0x27         // ; : key
,  Quote=              0x28         // ' " key
,  GraveAccent=        0x29         // ` ~ key
,  LeftShift=          0x2A         // Left shift key
,  LeftSlash=          0x2B         // \ | key
,  _z_=                0x2C         // z Z key
,  _x_=                0x2D         // x X key
,  _c_=                0x2E         // c C key
,  _v_=                0x2F         // v V key
,  _b_=                0x30         // b B key
,  _n_=                0x31         // n N key
,  _m_=                0x32         // m M key
,  Comma=              0x33         // , < key
,  Period=             0x34         // . > key
,  RightSlash=         0x35         // / ? key
,  RightShift=         0x36         // Right shift key
,  PrintScreen=        0x37         // Print screen key
,  ALT=                0x38         // ALT key
,  Space=              0x39         // Space key
,  CapsLock=           0x3A         // Caps lock key
,  F01=                0x3B         // F01 key
,  F02=                0x3C         // F02 key
,  F03=                0x3D         // F03 key
,  F04=                0x3E         // F04 key
,  F05=                0x3F         // F05 key
,  F06=                0x40         // F06 key
,  F07=                0x41         // F07 key
,  F08=                0x42         // F08 key
,  F09=                0x43         // F09 key
,  F10=                0x44         // F10 key
,  F11=                0x57         // F11 key
,  F12=                0x58         // F12 key
,  NumericLock=        0x45         // Numeric lock key
,  ScrollLock=         0x46         // Scroll lock key
,  Home=               0x47         // Home key
,  CursorUp=           0x48         // Cursor up key
,  PageUp=             0x49         // Page up key
,  GrayMinus=          0x4A         // Numeric pad minus key
,  CursorLeft=         0x4B         // Cursor left key
,  Center=             0x4C         // Cursor center key
,  CursorRight=        0x4D         // Cursor right key
,  GrayPlus=           0x4E         // Numeric pad plus key
,  End=                0x4F         // End key
,  CursorDown=         0x50         // Cursor down key
,  PageDown=           0x51         // Page down key
,  Insert=             0x52         // Insert key
,  Delete=             0x53         // Delete key
}; // enum
}; // class ScanCode

#endif // SCANCODE_H_INCLUDED
