/*
   OpenGUI - Drawing & Windowing library

   Copyright (C) 1996,2005  Marian Krivos

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

   base.h - base graphics support headers
*/

#ifndef __BASE_H
#define __BASE_H

//#define FG_NAMESPACE		// uncoment this line if you want namespace support

#include "fgversion.h"
#include "fginternal.h"
#include "fgcolor.h"
#include "fgshape.h"

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

#ifdef FG_NAMESPACE
/**
	@brief Global library namespace.
	On now namespace support in the library is fully implemented, but disabled.
	The switch to 'fgl::' namespace will be in near future.
	You can modify your code to support OpenGUI namespace right now by
	uncommenting #define FG_NAMESPACE line in 'base.h' headers;
*/
namespace fgl {
#else
#define fgl
#endif

#define UNDEFINED_USER_DATA	((void *)(-1L))

typedef void (*blitter_t) (FGPixel *, FGPixel *, int, int, int, int);
typedef void (*blitter2_t) (FGPixel *, FGPixel *, int, int, int, int, unsigned);
typedef void (*blitter_a_t) (FGPixel *, FGPixel *, int, int, int, int, int, unsigned ppop);
typedef int  (*shapper_t) (int, int, int, int, FGPixel ink, unsigned ppop);
typedef void (*clipper_t) (int, int, int, int);
typedef void (*plotter_t) (FGPixel *ptr, int x, int y, int ww, FGPixel ink, unsigned ppop);
typedef void (*paletter_t) (unsigned, unsigned);
typedef int  (*mouser_t) (int, int, int, int&);

#define	CR		13
#define	ESC		27
#define	TAB		9
#define	LF		10

/**
	The ROPs. Intended for a drawing functions.
	@ingroup Enums
*/
enum ENUM_PPOP
{
	_GSET,				// 0
	_GXOR,
	_GAND,
	_GOR,
	_GPLUS,
	_GMINUS,
	_GNOT,
	_GREPLACE_GE16,     // 7
	_GTRANSPARENT,      // 8
	_GCOLORKEY,         // 9
	_GREPLACE_LE32,      // 10
	_GREPLACE_LE48,      // 10
	_GREPLACE_LE64,      // 10
	_GREPLACE_LE80,      // 10
	_GREPLACE_LESS      // 11
};

/**
	Constants for apropriate videomodes.
	@ingroup Enums
*/
enum ENUM_VIDEOMODE {
	GTEXT,
	G320x200,
	G640x480,
	G800x600,
	G1024x768,
	G1280x1024,
	G1600x1200,
	GCUSTOM
};

/**
	Constants for bitmap rotations.
	@ingroup Enums
	@see FGDrawBuffer::rotate()
*/
enum ENUM_ROTATE_DIRECTION
{
	ROTATE_90_CW,
	ROTATE_90_CCW,
	ROTATE_180,
	ROTATE_HORIZONTAL,
	ROTATE_VERTICAL
};

#define	FG_BUTTON_NONE			0
#define	FG_BUTTON_LEFT			1
#define	FG_BUTTON_RIGHT			2
#define	FG_BUTTON_MIDDLE		4
#define	FG_BUTTON_WHEEL_UP		8
#define	FG_BUTTON_WHEEL_DOWN	16

/**
	For Buffered animation.
	@ingroup Enums
*/
enum ENUM_BUFFERING
{
	FG_DOUBLEBUFFER=2,
	FG_TRIPLEBUFFER,
	FG_QUADBUFFER
};

/**
	Flags for use with FGDrawBuffer::bitblit()
	@addtogroup Defines
	@{
*/
//! do only raw copy (no OR, AND etc.)
const unsigned BITBLIT_COPY	=			0x0000;
//! do copy with current dest. operator (_GSET, _GOR ...), see set_ppop()
const unsigned BITBLIT_WITH_OPERATOR =	0x0200;
//! do copy and assume the source alpha value (transparency)
const unsigned BITBLIT_WITH_ALPHA =		0x0400;
//! clips the coordinates before drawing (use if unsure)
const unsigned BITBLIT_WITH_CLIPPING =	0x1000;
//! to copy the overlapped blocks
const unsigned BITBLIT_WITH_DECREMENT =	0x2000;

/**
	@}
*/

/**
	The structure describes a pattern for the patterned line drawing.
*/
struct FGPattern
{
	FGPixel *data;
	int len, masked, reset, pos;
	FGPixel next(void);

	FGPattern(FGPixel * pp, int l, int m, int r = 1):data(pp), len(l),
	  masked(m), reset(r), pos(0)
	{
	}
};

/**
	This object is provided for common communication between
	any two OpenGUI graphics objects. By default is this unconnected.
	When you want connect the two objects to sending signal from one
	object to other, simply call the Connect() method. In called object
	you must overload virtual methods OnSignal(). When signal
	from one to other object is emited, there is one optional parameter
	that is transmited. You can set this parameter with Connect() or
	late by calling SetParam().
	@see FGCONNECT
*/
class FGConnector
{
		void* 	parameter1;
		class 	FGConnector *sendTo;
	public:
		/**
		This is an trivial way to solve problem with unavailable related user defined
		parameter. You can use method SetParam() to any Window to assign some parameter
		of int type. Since you can read this parameter at other place by GetParam().
		If returned value is -1, value is not assigned.
		*/
		void	FGAPI SetParameter(void* a) { parameter1 = a; }
		/**
		This is an trivial way to solve problem with unavailable related user defined
		parameter. You can use method SetParam() to any Window to assign some parameter
		of int type. Since you can read this parameter at other place by GetParam().
		If returned value is -1, value is not assigned.
		*/
		void*	FGAPI GetParameter(void)  const { return parameter1; }

		FGConnector()
		{
			SetParameter( UNDEFINED_USER_DATA );
			sendTo = 0;
		}
		virtual ~FGConnector() {}

		/**
		Connects other object pointed by 'from' and set an optional value.
		@note There is allowed only one bindings between two objects at one time,
		'The last, is the winner'!
		@param from other object of this kind to connect with us
		@param par user value/event code/magic number
		*/
		void 	FGAPI Connect(FGConnector *from, void* par)
		{
			sendTo = from;
			SetParameter(par);
		}
		//! do nothing - override in your code
		virtual void OnSignal(FGConnector *sender, void* value)
		{
		}
		//! Try emit defined signal if there is any.
		void FGAPI RunSignal(void)
		{
			if (sendTo)
				sendTo->OnSignal(this, parameter1);
		}
};

/**
 MACRO to connect one object with other. See class FGConnector for more.
 @ingroup Defines
 @see FGConnector::Connect()
*/
#define		FGCONNECT(sender, THIS, PARAM1) { (sender)->Connect((THIS), (void *)(PARAM1)); }

/**
	Structure to provide user defined mouse cursor.
*/
struct FGMouseCursor
{
	FGPixel *bitmap;
	FGPixel *mask;
	int	xoff, yoff;
	unsigned w,h;

	FGMouseCursor(FGPixel *a, FGPixel *b, int c, int d, int e, int f)
		: bitmap(a), mask(b), xoff(c), yoff(d), w(e), h(f)
	{
	}
};

// -----------------------------------------------------------------------------

#ifdef FG_NAMESPACE
}
#endif

#include "fgfontmanager.h"
#include "fgconcurency.h"
#include "fgdrawbuffer.h"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

void delay(unsigned int);
#ifdef _WIN32
void sound(unsigned);
void nosound(void);
#endif

void outpb(unsigned, unsigned);
void outpw(unsigned, unsigned);
unsigned char inpb(unsigned);
unsigned short inpw(unsigned);
unsigned int inpl(unsigned);
void outpl(unsigned, unsigned);

extern FGPattern PatternDot , PatternSlashDot , PatternSlashDotSlash , PatternSlash ;

//unsigned long long _rdtsc(void);
#ifndef __BORLANDC__
char *strupr(register char *s);
int stricmp(const char *s1, const char *s2);
int strnicmp(const char *s1, const char *s2, size_t cnt);
char *strlwr(char *_s);
#endif
/**
	@addtogroup Graphics
	@{
*/
int  get_point(int x, int y);
void draw_hline(int x, int y, int w, FGPixel ink, unsigned ppop);
void draw_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop);
void fill_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop);
void draw_point(int x, int y, FGPixel color, unsigned ppop);
int  draw_line(int x, int y, int x1, int y1, FGPixel color, unsigned ppop);
#define draw_line(a,b,c,d,e,f) vector_draw_line(a,b,c,d,e,f)
int  FGAPI drawto_line(int x, int y, FGPixel ink, unsigned ppop);
void FGAPI draw_pattern_line(int x, int y, int x1, int y1, FGPattern *, unsigned ppop);
void FGAPI drawto_pattern_line(int x, int y, FGPattern *, unsigned ppop);
void FGAPI draw_pattern_box(int x, int y, int a, int b, FGPattern *pat, unsigned ppop);
void FGAPI draw_spline(FGPoint points[4], FGPixel ink, unsigned ppop);
void calc_spline(FGPoint points[], int npts, FGPoint output[]);
void FGAPI moveto(int x, int y);
unsigned int FGAPI areasize(int x, int y);

void FGAPI _set_default_palette(void);
void FGAPI _set_fgl_palette(void);
unsigned int FGAPI get_palette(unsigned int i);
void _palette(unsigned, unsigned int);
void FGAPI CreatePaletteEntry(int rc, int gc, int bc, int idx);

int FGAPI text_out(int x, int y, char *txt, FGPixel ink, FGPixel paper, unsigned ppop, int f=FONT0816);
void FGAPI fill_convex(const FGPointArray* poly, FGPixel ink, unsigned ppop);
void FGAPI draw_convex(const FGPointArray* poly, FGPixel ink, unsigned ppop);
void FGAPI get_block(int x, int y, int a, int b, FGPixel * p);
void FGAPI put_block(int x, int y, int a, int b, FGPixel * p, unsigned ppop);
void FGAPI set_clip_rect(int w, int h, int x, int y);

void FGAPI fill_ellipse(int x, int y, int rx, int ry, FGPixel ink, unsigned ppop);
void FGAPI draw_ellipse(int x, int y, int rx, int ry, FGPixel ink, unsigned ppop);
void FGAPI draw_arc(int x, int y, double ang1, double ang2, int r, FGPixel ink, unsigned ppop);
void FGAPI draw_circle(int x, int y, int r, FGPixel ink, unsigned ppop);
void FGAPI fill_circle(int xs, int ys, int r, FGPixel ink, unsigned ppop);

void RamToVideo(FGPixel * Image, int x, int y, int xm, int ym, int w, int h, int cx_max, int cy_max, int opcia, unsigned ppop);
void VideoToRam(FGPixel * Image, int x, int y, int xm, int ym, int w, int h, int cx_max, int cy_max);
void RamToRam(int x, int y, int xmax, int ymax, int xdst, int ydst, int xmaxdst, int ymaxdst, int w, int h, FGPixel * from, FGPixel * to, unsigned alpha, unsigned ppop);
int gprintf(FGPixel ink, FGPixel paper, int x, int y, const char *format,...);
/** @} Graphics */

/**
	@addtogroup Colors
	@{
*/
int FGAPI GetFreeColors(void);
void FGAPI SetColorFuzzy(int a);
FGPixel FGAPI CreateColor(int, int, int, int);
void FGAPI DeleteColor(int);
void FGAPI set_transpcolor(FGPixel);
/** @} Colors */

void FGmemcpy(FGPixel * to, FGPixel * from, unsigned c);
void FGmemset(FGPixel * to, FGPixel data, unsigned c);
void FGmemcpy2(FGPixel * to, FGPixel * from, unsigned c, unsigned ppop);
void FGmemset2(FGPixel * to, FGPixel data, unsigned c, unsigned ppop);

/**
	Clears the whole screen.
	@ingroup Misc
	@param c background color
*/
void clear_frame_buffer(FGPixel c);
int graph_set_mode(int mode);
/**
	Change from one to other graphics mode with its resolution.
	@ingroup Misc
	@param ww the new X axis resolution
	@param hh the new Y axis resolution
	@return TRUE if OK
*/
int graph_change_mode(int ww, int hh);
void cleanup(void);
int FGAPI get_colordepth(void);
unsigned FGAPI GetXRes(void);
unsigned FGAPI GetYRes(void);
FGPixel * FGAPI GetFrameBuffer(void);

/**
	Returns absolute time in msec from the start of application. Useful for timing.
	@ingroup Misc
*/
unsigned long FGAPI FGClock(void);
void FGAPI Snd(int a, int b);
void FGAPI Puk(void);
/**
	Show the error dialog with defined string.
	@ingroup Misc
	@param s a string that will be shown
	@param flag 0 for warning or for fatal error (will exit after presskey). If you want MODAL dialog, you must add to the flag the value 2.
*/
extern "C" void	IError(const char	*s,	int	flag);
extern "C" void	__fg_error(const char	*s,	int	flag);

/**
	Returns the version string, by example: "4.1.0"
	@ingroup Misc
*/
const char* GetVer(void);
void UpdateRect(int x, int y, int xm, int ym, int w, int h);
int  GetPriv(void);

class FGDriver;

extern FGDriver *__fg_driver;
extern int sound_enabled;

// draw clipped to screen line
extern shapper_t   vector_draw_line;

// draw clipped to screen rect
extern clipper_t   vector_clip_rect;

// draw clipped to screen plot
extern plotter_t   vector_draw_point;

// draw clipped to screen box
extern shapper_t   vector_fill_box;

// set 8 bit palette (shift needed for 6-bit palette)
// palette_8 is accepted
extern paletter_t  vector_palette;

// bitblit copy
extern blitter_t   vector_blit_copy;
extern blitter2_t   vector_blit_op;
extern blitter_a_t vector_blit_a;

extern const FGMouseCursor IDC_NORMAL_SMALL;
extern const FGMouseCursor IDC_NORMAL_LARGE;
extern const FGMouseCursor *idc_normal;


#ifdef FG_NAMESPACE
}
#endif

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif


#define Pattern     FGPattern


#endif // base_h
