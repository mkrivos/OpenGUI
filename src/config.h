/*
    OpenGUI - Drawing & Windowing library

    Copyright (C) 1996,2004  Marian Krivos

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    nezmar@atlas.sk
*/

/*
 comment out to disable MTRR extension
 */
//#define			NO_MTRR

/*
 comment out to disable absolete code (old ListBox, etc.)
 */
//#define			NOABSOLETE

/*
 comment out for apropriate BitsPerPixel mode: 8, 15, 16 or 32
 */

//#define 			CLOSE_STDOUT
#define 			HW_ACCELERATOR		1

//
// enable PenMount DMC9512 controller (touch pad)
//
//#define  PENMOUNT

//
// enable the mouse move emulation with keyboard keys
//
//#define  FG_MOUSEKEYS
#ifdef	FG_MOUSEKEYS
//
// these keys are used for apropriate functions
//
#define		FG_MOUSEKEYS_UP			CTRL_UP
#define		FG_MOUSEKEYS_DOWN		CTRL_DOWN
#define		FG_MOUSEKEYS_LEFT		CTRL_LEFT
#define		FG_MOUSEKEYS_RIGHT 		CTRL_RIGHT
#define		FG_MOUSEKEYS_BUTTON1	' '
#define		FG_MOUSEKEYS_BUTTON2	CR
#endif

//#define DEBUG

#ifdef __rtems__
#define				NO_MTRR
#undef				HW_ACCELERATOR
#endif

// disable for index colors or QNX OS
#if (!defined(BPP15) && !defined(BPP16) && !defined(BPP32))
#undef				FG_JPEG
#undef				FG_PNG
#endif

