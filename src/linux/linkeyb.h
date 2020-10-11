/*

   LibGII internals

   Copyright (C) 1998 Andreas Beck	[becka@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _GGI_INTERNAL_GII_H
#define _GGI_INTERNAL_GII_H

#define _BUILDING_LIBGII

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>

#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef __rtems__
#include <rtems/kd.h>
#include <rtems/keyboard.h>
#else
#include <sys/kd.h>
#include <sys/vt.h>
#include <linux/tty.h>
#include <linux/keyboard.h>
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

typedef signed char		sint8;
typedef unsigned char		uint8;

typedef signed short		sint16;
typedef unsigned short		uint16;

typedef signed int		sint32;
typedef unsigned int		uint32;

typedef	signed int		ggi_sint;
typedef unsigned int		ggi_uint;

#define GGI_LITTLE_ENDIAN		1
#define GGI_OK		  0	/* All is well */
#define GGI_ENOMEM	-20	/* Out of memory */
#define GGI_ENOFILE	-21	/* File does not exist */
#define GGI_ENODEVICE	-22	/* Input/Output device can not be opened */
#define GGI_EARGREQ	-23	/* Required argument missing */
#define GGI_EARGINVAL	-24	/* Invalid argument(s) */
#define GGI_ENOTALLOC	-25	/* Trying to use or deallocate a resource that
				   was not previously allocated */
#define GGI_EFATAL	-26	/* Fatal error - the state of the target of
				   the operation is undefined */
#define GGI_EBADFILE	-27	/* Error reading (config) file */
#define GGI_ENOSPACE	-28	/* Out of space */

#define GGI_ELOCKBUSY	-30	/* Lock is in use */
#define GGI_ENOTFOUND	-31	/* The requested object was not found */
#define GGI_EEXCLUSIVE	-32	/* Tried to get non-exclusive access to object
				   which only supports exclusive access */

#define GGI_EEVUNKNOWN	-40	/* Unknown event sent to input source */
#define GGI_EEVNOTARGET	-41	/* No apropriate target for sent event */
#define GGI_EEVOVERFLOW	-42	/* Overflow when queuing event */

#define GGI_EUNKNOWN	-99	/* Unknown error */

#define GII_KEY(typ,val)	(((typ) << 8) | (val))
#define GII_KTYP(x)		((x) >> 8)
#define GII_KVAL(x)		((x) & 0xff)
#define GII_UNICODE(x)		((x) < 0xE000 || (x) >= 0xF900)

/* types */

#define GII_KT_LATIN1	0x00
#define GII_KT_SPEC	0xE0
#define GII_KT_FN	0xE1
#define GII_KT_PAD	0xE2
#define GII_KT_MOD	0xE3
#define GII_KT_DEAD	0xE4

/* modifiers */

#define GII_KM_SHIFT	0x00
#define GII_KM_CTRL	0x01
#define GII_KM_ALT	0x02
#define GII_KM_META	0x03
#define GII_KM_SUPER	0x04
#define GII_KM_HYPER	0x05
#define GII_KM_ALTGR	0x06
#define GII_KM_CAPS	0x07
#define GII_KM_NUM	0x08
#define GII_KM_SCROLL	0x09

#define GII_KM_MASK	0x0f
#define GII_KM_RIGHT	0x40	/* additive */
#define GII_KM_LOCK	0x80	/*  - " -   (only used for labels) */

/* Bits in the "modifiers" field */

#define GII_MOD_SHIFT	(1 << GII_KM_SHIFT)
#define GII_MOD_CTRL	(1 << GII_KM_CTRL)
#define GII_MOD_ALT	(1 << GII_KM_ALT)
#define GII_MOD_META	(1 << GII_KM_META)
#define GII_MOD_SUPER	(1 << GII_KM_SUPER)
#define GII_MOD_HYPER	(1 << GII_KM_HYPER)
#define GII_MOD_ALTGR	(1 << GII_KM_ALTGR)
#define GII_MOD_CAPS	(1 << GII_KM_CAPS)
#define GII_MOD_NUM	(1 << GII_KM_NUM)
#define GII_MOD_SCROLL	(1 << GII_KM_SCROLL)

/* function keys */

#define GIIK_F0			GII_KEY(GII_KT_FN, 0)
#define GIIK_F1			GII_KEY(GII_KT_FN, 1)
#define GIIK_F2			GII_KEY(GII_KT_FN, 2)
#define GIIK_F3			GII_KEY(GII_KT_FN, 3)
#define GIIK_F4			GII_KEY(GII_KT_FN, 4)
#define GIIK_F5			GII_KEY(GII_KT_FN, 5)
#define GIIK_F6			GII_KEY(GII_KT_FN, 6)
#define GIIK_F7			GII_KEY(GII_KT_FN, 7)
#define GIIK_F8			GII_KEY(GII_KT_FN, 8)
#define GIIK_F9			GII_KEY(GII_KT_FN, 9)
#define GIIK_F10		GII_KEY(GII_KT_FN, 10)
#define GIIK_F11		GII_KEY(GII_KT_FN, 11)
#define GIIK_F12		GII_KEY(GII_KT_FN, 12)
#define GIIK_F13		GII_KEY(GII_KT_FN, 13)
#define GIIK_F14		GII_KEY(GII_KT_FN, 14)
#define GIIK_F15		GII_KEY(GII_KT_FN, 15)
#define GIIK_F16		GII_KEY(GII_KT_FN, 16)
#define GIIK_F17		GII_KEY(GII_KT_FN, 17)
#define GIIK_F18		GII_KEY(GII_KT_FN, 18)
#define GIIK_F19		GII_KEY(GII_KT_FN, 19)
#define GIIK_F20		GII_KEY(GII_KT_FN, 20)
#define GIIK_F21		GII_KEY(GII_KT_FN, 21)
#define GIIK_F22		GII_KEY(GII_KT_FN, 22)
#define GIIK_F23		GII_KEY(GII_KT_FN, 23)
#define GIIK_F24		GII_KEY(GII_KT_FN, 24)
#define GIIK_F25		GII_KEY(GII_KT_FN, 25)
#define GIIK_F26		GII_KEY(GII_KT_FN, 26)
#define GIIK_F27		GII_KEY(GII_KT_FN, 27)
#define GIIK_F28		GII_KEY(GII_KT_FN, 28)
#define GIIK_F29		GII_KEY(GII_KT_FN, 29)
#define GIIK_F30		GII_KEY(GII_KT_FN, 30)
#define GIIK_F31		GII_KEY(GII_KT_FN, 31)
#define GIIK_F32		GII_KEY(GII_KT_FN, 32)
#define GIIK_F33		GII_KEY(GII_KT_FN, 33)
#define GIIK_F34		GII_KEY(GII_KT_FN, 34)
#define GIIK_F35		GII_KEY(GII_KT_FN, 35)
#define GIIK_F36		GII_KEY(GII_KT_FN, 36)
#define GIIK_F37		GII_KEY(GII_KT_FN, 37)
#define GIIK_F38		GII_KEY(GII_KT_FN, 38)
#define GIIK_F39		GII_KEY(GII_KT_FN, 39)
#define GIIK_F40		GII_KEY(GII_KT_FN, 40)
#define GIIK_F41		GII_KEY(GII_KT_FN, 41)
#define GIIK_F42		GII_KEY(GII_KT_FN, 42)
#define GIIK_F43		GII_KEY(GII_KT_FN, 43)
#define GIIK_F44		GII_KEY(GII_KT_FN, 44)
#define GIIK_F45		GII_KEY(GII_KT_FN, 45)
#define GIIK_F46		GII_KEY(GII_KT_FN, 46)
#define GIIK_F47		GII_KEY(GII_KT_FN, 47)
#define GIIK_F48		GII_KEY(GII_KT_FN, 48)
#define GIIK_F49		GII_KEY(GII_KT_FN, 49)
#define GIIK_F50		GII_KEY(GII_KT_FN, 50)
#define GIIK_F51		GII_KEY(GII_KT_FN, 51)
#define GIIK_F52		GII_KEY(GII_KT_FN, 52)
#define GIIK_F53		GII_KEY(GII_KT_FN, 53)
#define GIIK_F54		GII_KEY(GII_KT_FN, 54)
#define GIIK_F55		GII_KEY(GII_KT_FN, 55)
#define GIIK_F56		GII_KEY(GII_KT_FN, 56)
#define GIIK_F57		GII_KEY(GII_KT_FN, 57)
#define GIIK_F58		GII_KEY(GII_KT_FN, 58)
#define GIIK_F59		GII_KEY(GII_KT_FN, 59)
#define GIIK_F60		GII_KEY(GII_KT_FN, 60)
#define GIIK_F61		GII_KEY(GII_KT_FN, 61)
#define GIIK_F62		GII_KEY(GII_KT_FN, 62)
#define GIIK_F63		GII_KEY(GII_KT_FN, 63)
#define GIIK_F64		GII_KEY(GII_KT_FN, 64)

/* special keys */

#define GIIK_VOID		GII_KEY(GII_KT_SPEC, 0)

#define GIIK_Enter		GIIUC_Return
#define GIIK_Delete		GIIUC_Delete

#define GIIK_Break		GII_KEY(GII_KT_SPEC, 5)

#define GIIK_ScrollForw		GII_KEY(GII_KT_SPEC, 10)
#define GIIK_ScrollBack		GII_KEY(GII_KT_SPEC, 11)

#define GIIK_Boot		GII_KEY(GII_KT_SPEC, 12)
#define GIIK_Compose		GII_KEY(GII_KT_SPEC, 14)
#define GIIK_SAK		GII_KEY(GII_KT_SPEC, 15)

#define GIIK_Undo		GII_KEY(GII_KT_SPEC, 23)
#define GIIK_Redo		GII_KEY(GII_KT_SPEC, 24)
#define GIIK_Menu		GII_KEY(GII_KT_SPEC, 25)
#define GIIK_Cancel		GII_KEY(GII_KT_SPEC, 26)
#define GIIK_PrintScreen	GII_KEY(GII_KT_SPEC, 27)
#define GIIK_Execute		GII_KEY(GII_KT_SPEC, 28)
#define GIIK_Find		GII_KEY(GII_KT_SPEC, 30)
#define GIIK_Begin		GII_KEY(GII_KT_SPEC, 31)
#define GIIK_Clear		GII_KEY(GII_KT_SPEC, 32)
#define GIIK_Insert		GII_KEY(GII_KT_SPEC, 34)
#define GIIK_Select		GII_KEY(GII_KT_SPEC, 35)
#define GIIK_Macro	 	GII_KEY(GII_KT_SPEC, 38)
#define GIIK_Help		GII_KEY(GII_KT_SPEC, 39)
#define GIIK_Do			GII_KEY(GII_KT_SPEC, 40)
#define GIIK_Pause	 	GII_KEY(GII_KT_SPEC, 41)
#define GIIK_Stop		GIIK_Pause
#define GIIK_SysRq		GII_KEY(GII_KT_SPEC, 42)
#define GIIK_ModeSwitch		GII_KEY(GII_KT_SPEC, 43)

#define GIIK_Up			GII_KEY(GII_KT_SPEC, 50)
#define GIIK_Down		GII_KEY(GII_KT_SPEC, 51)
#define GIIK_Left		GII_KEY(GII_KT_SPEC, 52)
#define GIIK_Right		GII_KEY(GII_KT_SPEC, 53)
#define GIIK_Prior		GII_KEY(GII_KT_SPEC, 54)
#define GIIK_PageUp		GIIK_Prior
#define GIIK_Next		GII_KEY(GII_KT_SPEC, 55)
#define GIIK_PageDown		GIIK_Next
#define GIIK_Home		GII_KEY(GII_KT_SPEC, 56)
#define GIIK_End		GII_KEY(GII_KT_SPEC, 57)

/* keys on the numeric keypad */

#define GIIK_P0			GII_KEY(GII_KT_PAD, '0')
#define GIIK_P1			GII_KEY(GII_KT_PAD, '1')
#define GIIK_P2			GII_KEY(GII_KT_PAD, '2')
#define GIIK_P3			GII_KEY(GII_KT_PAD, '3')
#define GIIK_P4			GII_KEY(GII_KT_PAD, '4')
#define GIIK_P5			GII_KEY(GII_KT_PAD, '5')
#define GIIK_P6			GII_KEY(GII_KT_PAD, '6')
#define GIIK_P7			GII_KEY(GII_KT_PAD, '7')
#define GIIK_P8			GII_KEY(GII_KT_PAD, '8')
#define GIIK_P9			GII_KEY(GII_KT_PAD, '9')
#define GIIK_PA			GII_KEY(GII_KT_PAD, 'A')
#define GIIK_PB			GII_KEY(GII_KT_PAD, 'B')
#define GIIK_PC			GII_KEY(GII_KT_PAD, 'C')
#define GIIK_PD			GII_KEY(GII_KT_PAD, 'D')
#define GIIK_PE			GII_KEY(GII_KT_PAD, 'E')
#define GIIK_PF			GII_KEY(GII_KT_PAD, 'F')

#define GIIK_PPlus		GII_KEY(GII_KT_PAD, '+')
#define GIIK_PMinus		GII_KEY(GII_KT_PAD, '-')
#define GIIK_PSlash		GII_KEY(GII_KT_PAD, '/')
#define GIIK_PAsterisk		GII_KEY(GII_KT_PAD, '*')
#define GIIK_PStar		GIIK_PAsterisk
#define GIIK_PEqual		GII_KEY(GII_KT_PAD, '=')
#define GIIK_PSeparator		GII_KEY(GII_KT_PAD, ',')
#define GIIK_PDecimal		GII_KEY(GII_KT_PAD, '.')
#define GIIK_PParenLeft		GII_KEY(GII_KT_PAD, '(')
#define GIIK_PParenRight	GII_KEY(GII_KT_PAD, ')')
#define GIIK_PSpace		GII_KEY(GII_KT_PAD, ' ')
#define GIIK_PEnter		GII_KEY(GII_KT_PAD, '\r')
#define GIIK_PTab		GII_KEY(GII_KT_PAD, '\t')

#define GIIK_PPlusMinus		GII_KEY(GII_KT_PAD, 0x80)
#define GIIK_PBegin		GII_KEY(GII_KT_PAD, 0x81)

#define GIIK_PF1		GII_KEY(GII_KT_PAD, 0x91)
#define GIIK_PF2		GII_KEY(GII_KT_PAD, 0x92)
#define GIIK_PF3		GII_KEY(GII_KT_PAD, 0x93)
#define GIIK_PF4		GII_KEY(GII_KT_PAD, 0x94)
#define GIIK_PF5		GII_KEY(GII_KT_PAD, 0x95)
#define GIIK_PF6		GII_KEY(GII_KT_PAD, 0x96)
#define GIIK_PF7		GII_KEY(GII_KT_PAD, 0x97)
#define GIIK_PF8		GII_KEY(GII_KT_PAD, 0x98)
#define GIIK_PF9		GII_KEY(GII_KT_PAD, 0x99)

/* modifier keys */

#define GIIK_Shift		GII_KEY(GII_KT_MOD, GII_KM_SHIFT)
#define GIIK_Ctrl		GII_KEY(GII_KT_MOD, GII_KM_CTRL)
#define GIIK_Alt		GII_KEY(GII_KT_MOD, GII_KM_ALT)
#define GIIK_Meta		GII_KEY(GII_KT_MOD, GII_KM_META)
#define GIIK_Super		GII_KEY(GII_KT_MOD, GII_KM_SUPER)
#define GIIK_Hyper		GII_KEY(GII_KT_MOD, GII_KM_HYPER)
#define GIIK_AltGr		GII_KEY(GII_KT_MOD, GII_KM_ALTGR)
#define GIIK_Caps		GII_KEY(GII_KT_MOD, GII_KM_CAPS)
#define GIIK_Num		GII_KEY(GII_KT_MOD, GII_KM_NUM)
#define GIIK_Scroll		GII_KEY(GII_KT_MOD, GII_KM_SCROLL)

/* modifier _labels_ */

#define GIIK_ShiftL		(GIIK_Shift  | 0)
#define GIIK_ShiftR		(GIIK_Shift  | GII_KM_RIGHT)
#define GIIK_CtrlL		(GIIK_Ctrl   | 0)
#define GIIK_CtrlR		(GIIK_Ctrl   | GII_KM_RIGHT)
#define GIIK_AltL		(GIIK_Alt    | 0)
#define GIIK_AltR		(GIIK_Alt    | GII_KM_RIGHT)
#define GIIK_MetaL		(GIIK_Meta   | 0)
#define GIIK_MetaR		(GIIK_Meta   | GII_KM_RIGHT)
#define GIIK_SuperL		(GIIK_Super  | 0)
#define GIIK_SuperR		(GIIK_Super  | GII_KM_RIGHT)
#define GIIK_HyperL		(GIIK_Hyper  | 0)
#define GIIK_HyperR		(GIIK_Hyper  | GII_KM_RIGHT)

#define GIIK_ShiftLock		(GIIK_Shift  | GII_KM_LOCK)
#define GIIK_CtrlLock		(GIIK_Ctrl   | GII_KM_LOCK)
#define GIIK_AltLock		(GIIK_Alt    | GII_KM_LOCK)
#define GIIK_MetaLock		(GIIK_Meta   | GII_KM_LOCK)
#define GIIK_SuperLock		(GIIK_Super  | GII_KM_LOCK)
#define GIIK_HyperLock		(GIIK_Hyper  | GII_KM_LOCK)
#define GIIK_AltGrLock		(GIIK_AltGr  | GII_KM_LOCK)
#define GIIK_CapsLock		(GIIK_Caps   | GII_KM_LOCK)
#define GIIK_NumLock		(GIIK_Num    | GII_KM_LOCK)
#define GIIK_ScrollLock		(GIIK_Scroll | GII_KM_LOCK)

/* miscellaneous */

#define GIIK_NIL   	0xffff	/* used to indicate "not mapped yet" */
#define GII_BUTTON_NIL	0xff	/* used for pseudo keys (composed ones) */

/* ASCII keys */
#define GIIUC_Nul		0x00
#define GIIUC_BackSpace		0x08
#define GIIUC_Tab		0x09
#define GIIUC_Linefeed		0x0a
#define GIIUC_Return		0x0d
#define GIIUC_Escape		0x1b
#define GIIUC_Delete		0x7f

#define GIIUC_Space		0x20
#define GIIUC_Exclamation	0x21
#define GIIUC_Exclam		GIIUC_Exclamation
#define GIIUC_DoubleQuote	0x22
#define GIIUC_QuoteDbl		GIIUC_DoubleQuote
#define GIIUC_NumberSign	0x23
#define GIIUC_Hash		GIIUC_NumberSign
#define GIIUC_Dollar		0x24
#define GIIUC_Percent		0x25
#define GIIUC_Ampersand		0x26
#define GIIUC_Apostrophe	0x27
#define GIIUC_ParenLeft		0x28
#define GIIUC_ParenRight	0x29
#define GIIUC_Asterisk		0x2a
#define GIIUC_Star		GIIUC_Asterisk
#define GIIUC_Plus		0x2b
#define GIIUC_Comma		0x2c
#define GIIUC_Minus		0x2d
#define GIIUC_Period		0x2e
#define GIIUC_Slash		0x2f
#define GIIUC_0			0x30
#define GIIUC_1			0x31
#define GIIUC_2			0x32
#define GIIUC_3			0x33
#define GIIUC_4			0x34
#define GIIUC_5			0x35
#define GIIUC_6			0x36
#define GIIUC_7			0x37
#define GIIUC_8			0x38
#define GIIUC_9			0x39
#define GIIUC_Colon		0x3a
#define GIIUC_Semicolon		0x3b
#define GIIUC_Less		0x3c
#define GIIUC_Equal		0x3d
#define GIIUC_Greater		0x3e
#define GIIUC_Question		0x3f
#define GIIUC_At		0x40
#define GIIUC_A			0x41
#define GIIUC_B			0x42
#define GIIUC_C			0x43
#define GIIUC_D			0x44
#define GIIUC_E			0x45
#define GIIUC_F			0x46
#define GIIUC_G			0x47
#define GIIUC_H			0x48
#define GIIUC_I			0x49
#define GIIUC_J			0x4a
#define GIIUC_K			0x4b
#define GIIUC_L			0x4c
#define GIIUC_M			0x4d
#define GIIUC_N			0x4e
#define GIIUC_O			0x4f
#define GIIUC_P			0x50
#define GIIUC_Q			0x51
#define GIIUC_R			0x52
#define GIIUC_S			0x53
#define GIIUC_T			0x54
#define GIIUC_U			0x55
#define GIIUC_V			0x56
#define GIIUC_W			0x57
#define GIIUC_X			0x58
#define GIIUC_Y			0x59
#define GIIUC_Z			0x5a
#define GIIUC_BracketLeft	0x5b
#define GIIUC_BackSlash		0x5c
#define GIIUC_BracketRight	0x5d
#define GIIUC_Circumflex	0x5e
#define GIIUC_Hat		GIIUC_Circumflex
#define GIIUC_Underscore	0x5f
#define GIIUC_Grave		0x60
#define GIIUC_a			0x61
#define GIIUC_b			0x62
#define GIIUC_c			0x63
#define GIIUC_d			0x64
#define GIIUC_e			0x65
#define GIIUC_f			0x66
#define GIIUC_g			0x67
#define GIIUC_h			0x68
#define GIIUC_i			0x69
#define GIIUC_j			0x6a
#define GIIUC_k			0x6b
#define GIIUC_l			0x6c
#define GIIUC_m			0x6d
#define GIIUC_n			0x6e
#define GIIUC_o			0x6f
#define GIIUC_p			0x70
#define GIIUC_q			0x71
#define GIIUC_r			0x72
#define GIIUC_s			0x73
#define GIIUC_t			0x74
#define GIIUC_u			0x75
#define GIIUC_v			0x76
#define GIIUC_w			0x77
#define GIIUC_x			0x78
#define GIIUC_y			0x79
#define GIIUC_z			0x7a
#define GIIUC_BraceLeft		0x7b
#define GIIUC_Bar		0x7c
#define GIIUC_Pipe		GIIUC_Bar
#define GIIUC_BraceRight	0x7d
#define GIIUC_Tilde		0x7e

#define GIIUC_NoBreakSpace	0xa0
#define GIIUC_NBSP		GIIUC_NoBreakSpace
#define GIIUC_ExclamDown	0xa1
#define GIIUC_Cent		0xa2
#define GIIUC_Sterling		0xa3
#define GIIUC_Pound		GIIUC_Sterling
#define GIIUC_Currency		0xa4
#define GIIUC_Yen		0xa5
#define GIIUC_BrokenBar		0xa6
#define GIIUC_Section		0xa7
#define GIIUC_Diaeresis		0xa8
#define GIIUC_Umlaut		GIIUC_Diaeresis
#define GIIUC_Copyright		0xa9
#define GIIUC_OrdFeminine	0xaa
#define GIIUC_GuillemotLeft	0xab
#define GIIUC_NotSign		0xac
#define GIIUC_SoftHyphen	0xad
#define GIIUC_Registered	0xae
#define GIIUC_Macron		0xaf
#define GIIUC_Degree		0xb0
#define GIIUC_PlusMinus		0xb1
#define GIIUC_TwoSuperior	0xb2
#define GIIUC_ThreeSuperior	0xb3
#define GIIUC_Acute		0xb4
#define GIIUC_Mu		0xb5
#define GIIUC_Micro		GIIUC_Mu
#define GIIUC_Paragraph		0xb6
#define GIIUC_Pilcrow		GIIUC_Paragraph
#define GIIUC_PeriodCentered	0xb7
#define GIIUC_MiddleDot		GIIUC_PeriodCentered
#define GIIUC_Cedilla		0xb8
#define GIIUC_OneSuperior	0xb9
#define GIIUC_mKuline		0xba
#define GIIUC_GuillemotRight	0xbb
#define GIIUC_OneQuarter	0xbc
#define GIIUC_OneHalf		0xbd
#define GIIUC_ThreeQuarters	0xbe
#define GIIUC_QuestionDown	0xbf
#define GIIUC_Agrave		0xc0
#define GIIUC_Aacute		0xc1
#define GIIUC_Acircumflex	0xc2
#define GIIUC_Atilde		0xc3
#define GIIUC_Adiaeresis	0xc4
#define GIIUC_Aumlaut		GIIUC_Adiaeresis
#define GIIUC_Aring		0xc5
#define GIIUC_AE		0xc6
#define GIIUC_Ccedilla		0xc7
#define GIIUC_Egrave		0xc8
#define GIIUC_Eacute		0xc9
#define GIIUC_Ecircumflex	0xca
#define GIIUC_Ediaeresis	0xcb
#define GIIUC_Eumlaut		GIIUC_Ediaeresis
#define GIIUC_Igrave		0xcc
#define GIIUC_Iacute		0xcd
#define GIIUC_Icircumflex	0xce
#define GIIUC_Idiaeresis	0xcf
#define GIIUC_Iumlaut		GIIUC_Idiaeresis
#define GIIUC_ETH		0xd0
#define GIIUC_Ntilde		0xd1
#define GIIUC_Ograve		0xd2
#define GIIUC_Oacute		0xd3
#define GIIUC_Ocircumflex	0xd4
#define GIIUC_Otilde		0xd5
#define GIIUC_Odiaeresis	0xd6
#define GIIUC_Oumlaut		GIIUC_Odiaeresis
#define GIIUC_Multiply		0xd7
#define GIIUC_Ooblique		0xd8
#define GIIUC_Ugrave		0xd9
#define GIIUC_Uacute		0xda
#define GIIUC_Ucircumflex	0xdb
#define GIIUC_Udiaeresis	0xdc
#define GIIUC_Uumlaut		GIIUC_Udiaeresis
#define GIIUC_Yacute		0xdd
#define GIIUC_THORN		0xde
#define GIIUC_ssharp		0xdf
#define GIIUC_agrave		0xe0
#define GIIUC_aacute		0xe1
#define GIIUC_acircumflex	0xe2
#define GIIUC_atilde		0xe3
#define GIIUC_adiaeresis	0xe4
#define GIIUC_aumlaut		GIIUC_adiaeresis
#define GIIUC_aring		0xe5
#define GIIUC_ae		0xe6
#define GIIUC_ccedilla		0xe7
#define GIIUC_egrave		0xe8
#define GIIUC_eacute		0xe9
#define GIIUC_ecircumflex	0xea
#define GIIUC_ediaeresis	0xeb
#define GIIUC_eumlaut		GIIUC_ediaeresis
#define GIIUC_igrave		0xec
#define GIIUC_iacute		0xed
#define GIIUC_icircumflex	0xee
#define GIIUC_idiaeresis	0xef
#define GIIUC_iumlaut		GIIUC_idiaeresis
#define GIIUC_eth		0xf0
#define GIIUC_ntilde		0xf1
#define GIIUC_ograve		0xf2
#define GIIUC_oacute		0xf3
#define GIIUC_ocircumflex	0xf4
#define GIIUC_otilde		0xf5
#define GIIUC_odiaeresis	0xf6
#define GIIUC_oumlaut		GIIUC_odiaeresis
#define GIIUC_Division		0xf7
#define GIIUC_oslash		0xf8
#define GIIUC_ugrave		0xf9
#define GIIUC_uacute		0xfa
#define GIIUC_ucircumflex	0xfb
#define GIIUC_udiaeresis	0xfc
#define GIIUC_uumlaut		GIIUC_udiaeresis
#define GIIUC_yacute		0xfd
#define GIIUC_thorn		0xfe
#define GIIUC_ydiaeresis	0xff
#define GIIUC_yumlaut		GIIUC_ydiaeresis

/* Dead keys */
#define GIIK_DeadRing			GII_KEY(GII_KT_DEAD, 0x00)
#define GIIK_DeadCaron			GII_KEY(GII_KT_DEAD, 0x01)
#define GIIK_DeadOgonek			GII_KEY(GII_KT_DEAD, 0x02)
#define GIIK_DeadIota			GII_KEY(GII_KT_DEAD, 0x03)
#define GIIK_DeadDoubleAcute		GII_KEY(GII_KT_DEAD, 0x04)
#define GIIK_DeadBreve			GII_KEY(GII_KT_DEAD, 0x05)
#define GIIK_DeadAboveDot		GII_KEY(GII_KT_DEAD, 0x06)
#define GIIK_DeadBelowDot		GII_KEY(GII_KT_DEAD, 0x07)
#define GIIK_DeadVoicedSound		GII_KEY(GII_KT_DEAD, 0x08)
#define GIIK_DeadSemiVoicedSound	GII_KEY(GII_KT_DEAD, 0x09)

#define GIIK_DeadAcute			GII_KEY(GII_KT_DEAD, GIIUC_Acute)
#define GIIK_DeadCedilla		GII_KEY(GII_KT_DEAD, GIIUC_Cedilla)
#define GIIK_DeadCircumflex		GII_KEY(GII_KT_DEAD, GIIUC_Circumflex)
#define GIIK_DeadDiaeresis		GII_KEY(GII_KT_DEAD, GIIUC_Diaeresis)
#define GIIK_DeadGrave			GII_KEY(GII_KT_DEAD, GIIUC_Grave)
#define GIIK_DeadTilde			GII_KEY(GII_KT_DEAD, GIIUC_Tilde)
#define GIIK_DeadMacron			GII_KEY(GII_KT_DEAD, GIIUC_Macron)


enum gii_event_type
{

	evNothing = 0,	/* event is not valid. (must be zero)	*/

	evKeyPress=5,	/* key has been pressed			*/
	evKeyRelease,	/* key has been released		*/
	evKeyRepeat	/* automatically repeated keypress	*/
};

#define   EVMASK        0

#define   emKeyPress      1
#define   emKeyRelease    2
#define   emKeyRepeat     4
#define   emKey		   (emKeyPress | emKeyRelease | emKeyRepeat)
#define   emKeyboard	   emKey,

typedef int gii_event_mask;

#undef EVMASK

/*	This information is reported with all events. Use the <any> field
**	in a gii_event structure to access these fields.
*/
struct gii_any_event
{
	uint8	size;		/* size of event in bytes	*/\
	uint8	type;		/* type of this event		*/
};

struct gii_key_event
{
	uint8	size;		/* size of event in bytes	*/\
	uint8	type;		/* type of this event		*/
	uint32	modifiers;	/* current modifiers in effect */
	uint32	sym;		/* meaning of key	*/
	uint32  label;		/* label on key		*/
	uint32  button;		/* button number	*/

};

union gii_event
{
	uint8			size;		/* size of this event	*/
	struct gii_any_event		any;		/* access COMMON_DATA	*/
	struct gii_key_event		key;		/* key press/release	*/
};

void _giiEventBlank(union gii_event *ev, int size);
int GII_get_key(void);
extern int GII_keyboard_init(int kfd);
extern int GII_close(void);
void _giiEvQueueAdd(union gii_event *ev);

typedef struct keyboard_hook
{
	int fd;
	int eof;

	int old_mode;
	struct termios old_termios;
	char old_kbled;

	uint8  keydown_buf[128];
	uint32 keydown_sym[128];
	uint32 keydown_label[128];

	uint32  modifiers;
	uint32  normalmod;
	uint32  lockedmod;
	uint32  lockedmod2;

	unsigned char	accent;
	struct kbdiacrs	accent_table;

	int	call_vtswitch;
	int	needctrl2switch;
	int	ctrlstate;
} linkbd_priv;

typedef struct gii_input {
	int 			maxfd;
	fd_set			fdset;
	uint32			flags;
	linkbd_priv 	*priv;
} gii_input;

#define LED2MASK(x)	(((x) & LED_CAP ? GII_MOD_CAPS   : 0) | \
			 ((x) & LED_NUM ? GII_MOD_NUM    : 0) | \
			 ((x) & LED_SCR ? GII_MOD_SCROLL : 0))

#define MASK2LED(x)	(((x) & GII_MOD_CAPS   ? LED_CAP : 0) | \
			 ((x) & GII_MOD_NUM    ? LED_NUM : 0) | \
			 ((x) & GII_MOD_SCROLL ? LED_SCR : 0))

#ifdef FG_NAMESPACE
}
#endif


#endif /* _GGI_INTERNAL_GII_H */
