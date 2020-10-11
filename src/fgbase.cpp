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

   base.cc - base graphics support routines
*/

#include <stdarg.h>
#include <errno.h>

#include "fgbase.h"
#include "fgcolor.h"
#include "_fastgl.h"

#include <sys/timeb.h>

#define swap(a,b)           {a^=b; b^=a; a^=b;}

#ifdef FG_NAMESPACE
namespace fgl
{
#endif

FGInternalState fgstate;

static FGPixel __p1[2] =
{0, 1};
static FGPixel __p2[9] =
{1, 0, 0, 1, 1, 1, 1, 0, 0};
static FGPixel __p3[15] =
{0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1};
static FGPixel __p4[8] =
{0, 0, 0, 0, 1, 1, 1, 1};

FGPattern PatternDot(__p1, 2, 1), PatternSlashDot(__p2, 9, 1), PatternSlashDotSlash(__p3, 15, 1),
		PatternSlash(__p4, 8, 1);

//! draw clipped to screen line
shapper_t  vector_draw_line = 0;

//! draw clipped to screen recty - HW helper = 0;
clipper_t   vector_clip_rect = 0;

//! draw clipped to screen box
shapper_t  vector_fill_box = 0;

//! draw clipped to screen plot
plotter_t  vector_draw_point = 0;

//! set 8 bit palette (shift needed for 6-bit palette)
paletter_t vector_palette = 0;

//! bitblit copy
blitter_t   vector_blit_copy = L1RamToVideo8;
blitter2_t   vector_blit_op  = L1RamToVideo2;
blitter_a_t vector_blit_a    = L1RamToVideoA;

//! clipping co-ordinates
int clip_x_min=0, clip_y_min=0, clip_x_max=1, clip_y_max=1;

//! offset VRAM in VideoSegment (non-zero in LINUX only)
FGPixel *videobase = 0;

//! size of the screen
int X_width=1, Y_width=1, X_virtual=1, Y_virtual=1;

int sound_enabled = true;

char __fgl_version[] = FG_VERSION;

edge            edge_array[128+2];
scan            scan_array[128+8];

void __fg_error(const char* s,int	flag)
{
	printf("OpenGUI ERROR: %s\n", s);
	if (flag)
		abort();
}

/**
	Returns the X axis resolution (visible)
	@ingroup Misc
*/
unsigned FGAPI GetXRes(void)
{
	return X_width;
}

/**
	Returns the Y axis resolution (visible)
	@ingroup Misc
*/
unsigned FGAPI GetYRes(void)
{
	return Y_width;
}

/**
	Returns the VRAM array address. YOu can use this address for direct videomemory access.
	@ingroup Misc
*/
FGPixel * FGAPI GetFrameBuffer(void)
{
	return videobase;
}

/**
	Returns the number of bits used per pixels.
	I.e. 8 for 256 color modes
	15, 16 or 32 for non-indexed modes
	@ingroup Misc
*/
int FGAPI get_colordepth(void)
{
	return FASTGL_BPP;
}

const char* GetVer(void)
{
	return __fgl_version;
}

/**
	Set transparency color for global drawing functions
*/
void FGAPI set_transpcolor(FGPixel c)
{
	fgstate.color_key = c;
}

FGPixel FGPattern::next(void)
{
	if (pos >= len)
		pos = 0;
	if (masked)
	{
		return data[pos++];
	}
	return data[pos++];
}

#define	FFB_CLIP_TOP	1
#define	FFB_CLIP_BOTTOM	2
#define	FFB_CLIP_LEFT	4
#define	FFB_CLIP_RIGHT	8

static unsigned inline comp_outcode(int x, int y, int clip_left, int clip_top, int clip_right, int clip_bottom)
{
	unsigned	code = 0;

	if (y < clip_top)
		code |= FFB_CLIP_TOP;
	if (y > clip_bottom)
		code |= FFB_CLIP_BOTTOM;
	if (x > clip_right)
		code |= FFB_CLIP_RIGHT;
	if (x < clip_left)
		code |= FFB_CLIP_LEFT;

	return code;
}

static inline int muldiv64(int m1, int m2, int d)
{
	return int((double) m1 * (double) m2 / ((double) d));
}

static inline int gl_regioncode (int x, int y, int __clipx1, int __clipy1, int __clipx2, int __clipy2)
{
	int result = 0;

	if (x < __clipx1)
	result |= 1;
	else if (x > __clipx2)
	result |= 2;
	if (y < __clipy1)
	result |= 4;
	else if (y > __clipy2)
	result |= 8;
	return result;
}

/**
 * Modified Cohen-Sutherland line clipping algorithm.
 * Unlike "stock" Cohen-Sutherland algorithm, produces clipped coordinates
 * suitable for rendering Bresenham lines.
 *
 * Returns 0 if line completely clipped out (reject) else returns 1
 @ingroup Misc
 */
int ClipLine(int& x1, int& y1, int& x2, int& y2,
	int __clipx1, int __clipy1, int __clipx2, int __clipy2)
{
	for (;;)
	{
		int r1 = gl_regioncode (x1, y1, __clipx1, __clipy1, __clipx2, __clipy2);
		int r2 = gl_regioncode (x2, y2, __clipx1, __clipy1, __clipx2, __clipy2);

		if (!(r1 | r2))
			break;		/* completely inside */

		if (r1 & r2)
			return 0;		/* completely outside */

		if (r1 == 0)
		{
			swap (x1, x2);	/* make sure first */
			swap (y1, y2);	/* point is outside */
			r1 = r2;
		}
		if (r1 & 1)
		{	/* left */
			y1 += muldiv64 (__clipx1 - x1, y2 - y1, x2 - x1);
			x1 = __clipx1;
		}
		else if (r1 & 2)
		{	/* right */
			y1 += muldiv64 (__clipx2 - x1, y2 - y1, x2 - x1);
			x1 = __clipx2;
		} else if (r1 & 4)
		{	/* top */
			x1 += muldiv64 (__clipy1 - y1, x2 - x1, y2 - y1);
			y1 = __clipy1;
		} else if (r1 & 8)
		{	/* bottom */
			x1 += muldiv64 (__clipy2 - y1, x2 - x1, y2 - y1);
			y1 = __clipy2;
		}
	}
	return 1;
}

/**
Draws point directly on the screen.
*/
void draw_point(int x, int y, FGPixel color, unsigned ppop)
{
	vector_draw_point(videobase,x,y,X_virtual,color, ppop);
}

//! internal for fill circle in VRAM
static void FGAPI _vsymetry2(int xs, int ys, int x, int y, FGPixel ink, unsigned ppop)
{
	draw_line(xs + x, ys + y, xs - x, ys + y, ink, ppop);
	draw_line(xs + x, ys - y, xs - x, ys - y, ink, ppop);
	draw_line(xs + y, ys + x, xs - y, ys + x, ink, ppop);
	draw_line(xs + y, ys - x, xs - y, ys - x, ink, ppop);
}

//! fill circle directly on the screen
void FGAPI fill_circle(int xs, int ys, int r, FGPixel ink, unsigned ppop)
{
	int x = 0;
	int y = r;
	int p = 3 - 2 * r;

	if ((ys + r) < clip_y_min || (xs + r) < clip_x_min || (ys - r) >= clip_y_max || (xs - r) >= clip_x_max || r < 1)
		return;

	while (x < y)
	{
		_vsymetry2(xs, ys, x, y, ink, ppop);
		if (p < 0)
			p += 4 * (x++) + 6;
		else
			p += 4 * ((x++) - (y--)) + 10;
	}
	if (x == y)
		_vsymetry2(xs, ys, x, y, ink, ppop);
}

//! internal for draw circle to the screen
static void FGAPI _vsymetry(int xs, int ys, int x, int y, FGPixel ink, unsigned ppop)
{
	vector_draw_point(videobase,xs + x, ys + y, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs - x, ys + y, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs + x, ys - y, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs - x, ys - y, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs + y, ys + x, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs - y, ys + x, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs + y, ys - x, X_virtual, ink, ppop);
	vector_draw_point(videobase,xs - y, ys - x, X_virtual, ink, ppop);
}

//! draw circle to the screen
void FGAPI draw_circle(int xs, int ys, int r, FGPixel ink, unsigned ppop)
{
	int x = 0;
	int y = r;
	int p = 3 - 2 * r;

	if ((ys + r) < clip_y_min || (xs + r) < clip_x_min || (ys - r) >= clip_y_max || (xs - r) >= clip_x_max || r < 1)
		return;

	while (x < y)
	{
		_vsymetry(xs, ys, x, y, ink, ppop);
		if (p < 0)
			p += 4 * (x++) + 6;
		else
			p += 4 * ((x++) - (y--)) + 10;
	}
	if (x == y)
		_vsymetry(xs, ys, x, y, ink, ppop);
}

//! this is very important procedure - its must be fast
//! put buffer from RAM to VRAM with redraw mode
void RamToVideo(FGPixel * Image, int x, int y, int xm, int ym, int w, int h, int cx_max, int cy_max, int opcia, unsigned ppop)
{
	FGPixel *src, *dst;

	if (y < 0 || x < 0 || y >= cy_max || x >= cx_max || w < 1 || h < 1)
		return;
	if ((ym + h) < clip_y_min || (xm + w) < clip_x_min || ym >= clip_y_max || xm >= clip_x_max)
		return;

	if ((x + w) > cx_max)
		w -= ((x + w) - cx_max);
	if ((y + h) > cy_max)
		h -= ((y + h) - cy_max);

	if ((xm + w) > clip_x_max)
		w -= ((xm + w) - clip_x_max);
	if (xm < clip_x_min)
	{
		w += (xm - clip_x_min);
		x += clip_x_min - xm;
		xm = clip_x_min;
	}

	if ((ym + h) > clip_y_max)
	{
		h -= ((ym + h) - clip_y_max);
	}
	if (ym < clip_y_min)
	{
		h += (ym - clip_y_min);
		y += clip_y_min - ym;
		ym = clip_y_min;
	}

	if (w < 1 || h < 1)
		return;

	if (xm < 0)
		xm = 0;
	if (ym < 0)
		ym = 0;
	src = Image + y * cx_max + x;
	dst = (FGPixel *) (ym * X_virtual + xm + videobase);

	switch (opcia & 0xF00)
	{
		case BITBLIT_COPY:
			vector_blit_copy(src, dst, w, h, cx_max, X_virtual);
			break;
		case BITBLIT_WITH_OPERATOR:
			vector_blit_op(src, dst, w, h, cx_max, X_virtual, ppop);
			break;
		case BITBLIT_WITH_ALPHA:
			vector_blit_a(src, dst, w, h, cx_max, X_virtual, opcia & 0xFF, ppop);
			break;
	}
#if defined(X11_DRIVER) || defined(_WIN32)
	UpdateRect(xm,ym,xm,ym,w,h);
#endif
}

//! this very important procedure - its must be fast
//! put buffer from VIDEORAM to RAM with redraw mode
void VideoToRam(FGPixel * Image, int x, int y, int xm, int ym, int w, int h, int cx_max, int cy_max)
{
	FGPixel *src, *dst;

	if ((y + h) < clip_y_min || (x + w) < clip_x_min || y >= clip_y_max || x >= clip_x_max || w < 1 || h < 1)
		return;
	if (ym < 0 || xm < 0 || ym >= cy_max || xm >= cx_max)
		return;

	if ((x + w) > clip_x_max)
		w -= ((x + w) - clip_x_max);
	if (x < clip_x_min)
	{
		w += (x - clip_x_min);
		xm += clip_x_min - x;
		x = clip_x_min;
	}
	if ((y + h) > clip_y_max)
		h -= ((y + h) - clip_y_max);
	if (y < clip_y_min)
	{
		h += (y - clip_y_min);
		ym += clip_y_min - y;
		y = clip_y_min;
	}

	if ((xm + w) > cx_max)
		w -= ((xm + w) - cx_max);
	if ((ym + h) > cy_max)
		h -= ((ym + h) - cy_max);

	if (w < 1 || h < 1)
		return;
	dst = Image + ym * cx_max + xm;
	src = (FGPixel *) (y * X_virtual + x + videobase);
	L1VideoToRam8(src, dst, w, h, X_virtual, cx_max);
}

//! this is a very important procedure - its must be very fast
//! put buffer from RAM to RAM with redraw mode
//! The rectangles must be clipped !!!
void RamToRam(int x, int y, int step_src, int ymax, int xdst, int ydst, int step_dst, int ymaxdst, int w, int h, FGPixel * from, FGPixel * to, unsigned alpha, unsigned ppop)
{
	FGPixel *src, *dst;

	if (alpha & BITBLIT_WITH_CLIPPING)	// clip right now!
	{
		if (w < 1 || h < 1)
			return;
		if (y >= ymax || x >= step_src)
			return;
		if (ydst >= ymaxdst || xdst >= step_dst)
			return;
		if (y + h < 0 || x + h < 0)
			return;
		if (ydst + h < 0 || xdst + w < 0)
			return;

		if ((x + w) > step_src)
			w -= ((x + w) - step_src);
		if ((y + h) > ymax)
			h -= ((y + h) - ymax);
		if (xdst < 0)
		{
			w += xdst;
			xdst = 0;
		}
		if (ydst < 0)
		{
			h += ydst;
			ydst = 0;
		}
		if ((xdst + w) > step_dst)
			w -= ((xdst + w) - step_dst);
		if ((ydst + h) > ymaxdst)
			h -= ((ydst + h) - ymaxdst);
	}
	if (w < 1 || h < 1)
		return;

	if (alpha & BITBLIT_WITH_DECREMENT)
	{
		dst = to + ((ydst+h-1) * step_dst + xdst+w-1);
		src = from + ((y+h-1) * step_src + x+w-1);
		L1RamToRamd(src, dst, w, h, step_src, step_dst);
	}
	else
	{
		dst = to + ydst * step_dst + xdst;
		src = from + y * step_src + x;

		if (alpha & 255)
			L1RamToRamA(src, dst, w, h, step_src, step_dst, alpha & 255, ppop);
		else
		{
			if (alpha & BITBLIT_WITH_OPERATOR)
				L1RamToRamPpop(src, dst, w, h, step_src, step_dst, ppop);
			else
				L1RamToRam(src, dst, w, h, step_src, step_dst);
		}
	}
}

#if defined(_MSC_VER) || defined(__BORLANDC__)
FGPixel LoadColor(FGPixel *p)
{
		__asm
		{
		   mov eax, [p]
			cmp [bpp],1
			jz   LC8
			cmp [bpp],2
			jz   LC16
			jmp  LC32
LC8:			mov		al,[eax]
				mov		ah,al
				push	ax
				push	ax
				pop		eax
				jmp end
LC16:			mov		ax,[eax]
			push	ax
				push	ax
				pop		eax
				jmp end
LC32:			mov		eax,[eax]
end:

		};
}

void FGAPI CharOutClip(FGPixel *dst, FGPixel *src, int step, int w, int h, FGPixel ink, FGPixel paper)
{
		__asm {
				cmp		byte ptr [w], 8
				jnz		_CharOutClip12
				cmp		byte ptr bpp, 1
				jnz		_CharOutClip12
				mov		edx,[src]
				mov		eax,[dst]
				push	edi
				push	esi
				push	ebx
				push	ecx
				push	ebp
				cld
				mov		edi,eax
				mov		esi,edx
				mov		ecx,[h]
				xchg	eax,ebx
				mov		al,byte ptr [ink]
				mov		ah,al
				push	ax
				push	ax
				pop		eax
				xchg	eax,ebx
				xchg	eax,edx
				mov		al,byte ptr [paper]
				mov		ah,al
				push	ax
				push	ax
				pop		eax
				xchg	eax,edx
L127:			push	ecx
				push	edi
				push	ebp
				lodsd
				mov		ebp,eax
				and		eax,ebx
				push	eax
				push	ebp
				pop		eax
				pop		ebp
				not		eax
				and		eax,edx
				or		eax,ebp
				stosd
				lodsd
				mov		ebp,eax
				and		eax,ebx
				xchg	eax,ebp
				not		eax
				and		eax,edx
				or		eax,ebp
				stosd
				pop		ebp
				pop		edi
				add		edi,[step]
				pop		ecx
				dec		ecx
				jnz		L127
L127_1:			pop		ebp
				pop		ecx
				pop		ebx
				pop		esi
				pop		edi
				jmp		endd

_CharOutClip12:	mov		edx,[src]
				mov		eax,[dst]
				push	edi
				push	esi
				push	ebx
				push	ecx
				push	ebp
				mov		cl,byte ptr [bpp]
				shr		cl,1
				shl		dword ptr [step],cl
				cld
				mov		edi,eax
				mov		esi,edx
				xchg	eax,ebx
				lea		eax,[ink] // otocene
				push 	eax
				call 	LoadColor
				xchg	eax,ebx
				xchg	eax,edx
				lea		eax,[paper]
				push 	eax
				call 	LoadColor
				add 	esp,2*4
				xchg	eax,edx
				mov		ecx,[h]
				mov		eax,[w]
				push	edx
				mul		word ptr [bpp]
				pop		edx
				shr		al,2
				mov		ch,al

L12712:			push	ecx
				push	edi
				push	ebp
L12713:			lodsd
				mov		ebp,eax
				and		eax,ebx
				push	eax
				push	ebp
				pop		eax
				pop		ebp
				not		eax
				and		eax,edx
				or		eax,ebp
				stosd
				dec		ch
				jnz		L12713
				pop		ebp
				pop		edi
				pop		ecx
				add		edi,[step]
				dec		cl
				jnz		L12712

				pop		ebp
				pop		ecx
				pop		ebx
				pop		esi
				pop		edi
endd:
		};
}
#endif

#if defined(__GNUC__) || defined(__ICC) || defined(__SUNPRO_CC)
void CharOutClip(FGPixel *dst, FGPixel *src, int step, int fontw, int fonth, FGPixel ink, FGPixel paper)
{
	for(int i=0;i<fonth;i++)
		switch(fontw>>2)
	{
		case 8:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 7:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 6:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 5:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 4:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 3:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 2:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
		case 1:
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			*dst++ = *src++?ink:paper;
			dst += step-fontw;
	}
}
#endif

static FGMutex TextMutex;

//! draw text directly on the SCREEN
/**
	@param x x position from top left corner
	@param y y position from top left corner
	@param txt 7bit ascii text string (null terminated)
	@param ink foreground color
	@param paper background color
	@param ppop the ROP
	@param f font taht will be used
	@return the next X position behind the printed string
*/
int FGAPI text_out(int x, int y, char *txt, FGPixel ink, FGPixel paper, unsigned ppop, int f)
{
	FGPixel *dst, *dst2;
	int alloc=0;

	TextMutex.Lock();

	// nastav font
	if (FGFontManager::get_font_ptr(f) == 0)
		f = FONT0816; // fallback to standard font

	int	width =	FGFontManager::textwidth(f,txt);
	int ww = FGFontManager::GetW(f);

	if ((y + FGFontManager::GetH(f)) < clip_y_min
		|| (x + width) < clip_x_min
		|| y >= clip_y_max
		|| x >= clip_x_max)
	{
		TextMutex.Unlock();
		return x;
	}

	unsigned  bytes = (width+ww)*FGFontManager::GetH(f);

	if (bytes < FGTEXTBUFSZ)
		dst	= fgstate.fgimagebuffer;
	else
	{
		alloc = 1;
		dst = (FGPixel *) malloc(bytes*sizeof(FGPixel));
	}

	dst2 = dst;

	if (ppop == _GCOLORKEY)
	{
		paper = fgstate.color_key;
	}
//	set_mmx();
	while (*txt)
	{
		variable_record *v = FGFontManager::IsVariableFont(f);
		if (v)
		{
			ww = v->w[*(unsigned char *)txt];
		}
		CharOutClip(dst,
			v ? v->off[*(unsigned char *)txt++] + FGFontManager::GetFontImage(f):
				*(unsigned char *)txt++ * (FGFontManager::GetW(f)*FGFontManager::GetH(f)) + FGFontManager::GetFontImage(f),
			width+FGFontManager::GetW(f),
			(ww+3)&~3,
			FGFontManager::GetH(f),
			ink,
			paper);
		dst	+= ww;
	}
//	reset_mmx();
	RamToVideo(dst2, 0, 0, x, y, width, FGFontManager::GetH(f), width+FGFontManager::GetW(f), FGFontManager::GetH(f), BITBLIT_WITH_OPERATOR, ppop);
	if (alloc) free(dst2);

	TextMutex.Unlock();

	return x + width;
}

//! fill box directly on the screen
void fill_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
{
	if ((y + h) < clip_y_min || (x + w) < clip_x_min || y >= clip_y_max || x >= clip_x_max || w < 1 || h < 1)
		return;
	if ((x + w) > clip_x_max)
		w -= ((x + w) - clip_x_max);
	if ((y + h) > clip_y_max)
		h -= ((y + h) - clip_y_max);
	if (x < clip_x_min)
	{
		w += (x - clip_x_min);
		x = clip_x_min;
	}
	if (y < clip_y_min)
	{
		h += (y - clip_y_min);
		y = clip_y_min;
	}
	if (w < 1 || h < 1)
		return;
	vector_fill_box(x, y, w, h, ink, ppop);
#if defined(X11_DRIVER) || defined(_WIN32)
	UpdateRect(x,y,x,y,w,h);
#endif
}

//! draw box directly on the screen
void draw_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
{
	draw_line(x, y, x + w - 1, y, ink, ppop);
	draw_line(x, y + h - 1, x + w - 1, y + h - 1, ink, ppop);
	draw_line(x, y, x, y + h - 1, ink, ppop);
	draw_line(x + w - 1, y, x + w - 1, y + h - 1, ink, ppop);
}

//! set clipping for screen (no window!)
void FGAPI set_clip_rect(int w, int h, int x, int y)
{
	clip_x_min = x;
	clip_y_min = y;
	if (clip_x_max < 0)
		clip_x_min = 0;
	if (clip_y_max < 0)
		clip_y_min = 0;
	clip_x_max = x + w;
	clip_y_max = y + h;
	if (clip_x_max > X_width)
		clip_x_max = X_width;
	if (clip_y_max > Y_width)
		clip_y_max = Y_width;

	// for polygon routines
	fgstate.ps.gc_xoffset = 0;
	fgstate.ps.gc_yoffset = 0;
	fgstate.ps.gc_xcliplo = clip_x_min;
	fgstate.ps.gc_ycliplo = clip_y_min;
	fgstate.ps.gc_xcliphi = clip_x_max - 1;
	fgstate.ps.gc_ycliphi = clip_y_max - 1;

	fgstate.ps.line = vector_draw_line;
	fgstate.ps.scan = draw_hline;
	if (vector_clip_rect)
		vector_clip_rect(clip_x_min, clip_y_min, clip_x_max, clip_y_max);
}

//! put block from RAM to VIDEORAMRAM with current draw mode
void FGAPI put_block(int x, int y, int w, int h, FGPixel * p, unsigned ppop)
{
	if (ppop == _GSET)
		RamToVideo(p, 0, 0, x, y, w, h, w, h, BITBLIT_COPY, ppop);
	else
		RamToVideo(p, 0, 0, x, y, w, h, w, h, BITBLIT_WITH_OPERATOR, ppop);
}

//! get block from VIDEORAM to RAM
void FGAPI get_block(int x, int y, int w, int h, FGPixel * p)
{
	VideoToRam(p, x, y, 0, 0, w, h, w, h);
}

//! return size of buffer for block
unsigned int FGAPI areasize(int w, int h)
{
	return w * h * bpp;
}

void clear_frame_buffer(FGPixel color)
{
	fill_box(0, 0, X_virtual, Y_width, color, _GSET);
#ifdef _WIN32
	delay(5);	// empiric constant for VSYNC
#endif
}

//! A traditional printf to the screen + position
int gprintf(FGPixel ink, FGPixel paper, int x, int y, const char *format, ...)
{
	char s[256];
	va_list arglist;
	va_start(arglist, format);
#ifndef _MSC_VER
	vsnprintf(s, 255, format, arglist);
#else
	vsprintf(s, format, arglist);
#endif
	va_end(arglist);
	text_out(x, y, s, ink, paper, _GSET);
	return strlen(s);
}

/**
 ** SCANCNVX.C ---- scan fill a convex polygon
 **
 ** Copyright (c) 1995 Csaba Biegl, 820 Stirrup Dr, Nashville, TN 37221
 ** [e-mail: csaba@vuse.vanderbilt.edu] See "doc/copying.cb" for details.
 **/
void do_fill_polygon(const FGPointArray* poly, PolygonStruct * current, FGPixel ink, unsigned ppop)
{
	int n = poly->vertices;
	FGPoint *pt = poly->array;

	edge *edges = edge_array, *ep;
	scan *scans = scan_array, *sp, *points, *segments;
	int xmin, xmax, ymin, ymax;
	int ypos, nedges, mem=0;

	if ((n > 1) && (pt[0].x == pt[n - 1].x) && (pt[0].y == pt[n - 1].y))
	{
		n--;
	}
	if (n < 1)
	{
		return;
	}
	if (n > 128)
	{
		edges = (edge *) malloc(sizeof(edge) * (n + 2));
		scans = (scan *) malloc(sizeof(scan) * (n + 8));
		mem = 1;
	}
	if (edges && scans)
	{
		int prevx = xmin = xmax = pt[0].x;
		int prevy = ymin = ymax = pt[0].y;

		nedges = 0;
		ep = edges;
		while (--n >= 0)
		{
			if (pt[n].y >= prevy)
			{
				ep->e.x = prevx;
				ep->e.y = prevy;
				ep->e.xlast = prevx = pt[n].x;
				ep->e.ylast = prevy = pt[n].y;
			}
			else
			{
				ep->e.xlast = prevx;
				ep->e.ylast = prevy;
				ep->e.x = prevx = pt[n].x;
				ep->e.y = prevy = pt[n].y;
			}
			if ((ep->e.y > (current->gc_ycliphi)) || (ep->e.ylast < (current->gc_ycliplo)))
				continue;
			{
				if (ep->e.y < current->gc_ycliplo)
				{
					ep->e.x += (((int) (((long) (((int) ((ep->e.xlast - ep->e.x)) << 1)) * (long) ((current->gc_ycliplo - ep->e.y))) / (long) ((ep->e.ylast - ep->e.y))) + (((int) ((ep->e.xlast - ep->e.x)) ^ (int) ((current->gc_ycliplo - ep->e.y)) ^ (int) ((ep->e.ylast - ep->e.y))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

					ep->e.y = current->gc_ycliplo;;
				}
			};
			if (ymin > ep->e.y)
				ymin = ep->e.y;
			if (ymax < ep->e.ylast)
				ymax = ep->e.ylast;
			if (xmin > ep->e.x)
				xmin = ep->e.x;
			if (xmax < ep->e.xlast)
				xmax = ep->e.xlast;
			{
				(&ep->e)->dy = (&ep->e)->ylast - (&ep->e)->y;
				(&ep->e)->dx = (&ep->e)->xlast - (&ep->e)->x;
				if ((&ep->e)->dx < 0)
				{
					(&ep->e)->xstep = (-1);
					(&ep->e)->dx = (-(&ep->e)->dx);
				}
				else
				{
					(&ep->e)->xstep = 1;
				}
				if ((&ep->e)->dx > (&ep->e)->dy)
				{
					(&ep->e)->xmajor = 1;
					(&ep->e)->error = (&ep->e)->dx >> 1;
				}
				else
				{
					(&ep->e)->xmajor = 0;
					(&ep->e)->error = ((&ep->e)->dy - ((1 - (&ep->e)->xstep) >> 1)) >> 1;
				}
			};
			ep->status = inactive;
			nedges++;
			ep++;
		}
		if ((nedges > 0) && (xmin <= (current->gc_xcliphi)) && (xmax >= (current->gc_xcliplo)))
		{
			if (xmin < (current->gc_xcliplo))
				xmin = (current->gc_xcliplo);
			if (ymin < (current->gc_ycliplo))
				ymin = (current->gc_ycliplo);
			if (xmax > (current->gc_xcliphi))
				xmax = (current->gc_xcliphi);
			if (ymax > (current->gc_ycliphi))
				ymax = (current->gc_ycliphi);

			for (ypos = ymin; ypos <= ymax; ypos++)
			{
				sp = scans;
				points = 0;
				segments = 0;
				for (n = nedges, ep = edges; --n >= 0; ep++)
				{
					switch (ep->status)
					{
						case inactive:
							if (ep->e.y != ypos)
								break;
							if (ep->e.dy == 0)
							{
								ep->status = passed;
								xmin = ep->e.x;
								xmax = ep->e.xlast;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int _swap_tmpval_ = (xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
								{
									scan *prev = 0;
									scan *work = segments;
									int overlap = 0;

									while (work != 0)
									{
										if ((work->x1 <= xmax) && (xmin <= work->x2))
										{
											overlap = 1;
											if (xmin < work->x1)
												work->x1 = xmin;
											if (xmax > work->x2)
											{
												prev = work;
												while ((work = work->next) != 0)
												{
													if (work->x1 > xmax)
														break;
													if (work->x2 > xmax)
														xmax = work->x2;
												}
												prev->x2 = xmax;
												prev->next = work;
											}
											break;
										}
										if (work->x1 > xmax)
											break;
										prev = work;
										work = work->next;
									}
									if (!overlap)
									{
										sp->x1 = xmin;
										sp->x2 = xmax;
										sp->next = work;
										if (prev)
											prev->next = sp;
										else
											segments = sp;
									}
								};
								sp++;
								break;
							}
							ep->status = active;
						case active:
							xmin = xmax = ep->e.x;
							if (ep->e.ylast == ypos)
							{
								ep->status = passed;
								xmax = ep->e.xlast;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int _swap_tmpval_ = (xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
								{
									scan *prev = 0;
									scan *work = points;

									while (work != 0)
									{
										if (work->x1 > xmin)
											break;
										prev = work;
										work = work->next;
									}
									sp->x1 = xmin;
									sp->x2 = xmax;
									sp->next = work;
									if (prev)
										prev->next = sp;
									else
										points = sp;
								};
								sp++;
							}
							else if (ep->e.xmajor)
							{
								for (;;)
								{
									(&ep->e)->x += (&ep->e)->xstep;
									if (((&ep->e)->error -= (&ep->e)->dy) < 0)
									{
										(&ep->e)->error += (&ep->e)->dx;
										break;
									}
								};
								xmax = ep->e.x - ep->e.xstep;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int _swap_tmpval_ = (xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
							}
							else
							{
								{
									if (((&ep->e)->error -= (&ep->e)->dx) < 0)
									{
										(&ep->e)->x += (&ep->e)->xstep;
										(&ep->e)->error += (&ep->e)->dy;
									}
								};
							}
							{
								scan *prev = 0;
								scan *work = points;

								while (work != 0)
								{
									if (work->x1 > xmin)
										break;
									prev = work;
									work = work->next;
								}
								sp->x1 = xmin;
								sp->x2 = xmax;
								sp->next = work;
								if (prev)
									prev->next = sp;
								else
									points = sp;
							};
							sp++;
							break;
						default:
							break;
					}
				}
				while (points != 0)
				{
					scan *nextpt = points->next;

					if (!nextpt)
						break;
					xmin = points->x1;
					xmax = nextpt->x2;
					points = nextpt->next;
					{
						scan *prev = 0;
						scan *work = segments;
						int overlap = 0;

						while (work != 0)
						{
							if ((work->x1 <= xmax) && (xmin <= work->x2))
							{
								overlap = 1;
								if (xmin < work->x1)
									work->x1 = xmin;
								if (xmax > work->x2)
								{
									prev = work;
									while ((work = work->next) != 0)
									{
										if (work->x1 > xmax)
											break;
										if (work->x2 > xmax)
											xmax = work->x2;
									}
									prev->x2 = xmax;
									prev->next = work;
								}
								break;
							}
							if (work->x1 > xmax)
								break;
							prev = work;
							work = work->next;
						}
						if (!overlap)
						{
							nextpt->x1 = xmin;
							nextpt->x2 = xmax;
							nextpt->next = work;
							if (prev)
								prev->next = nextpt;
							else
								segments = nextpt;
						}
					};
				}
				while (segments != 0)
				{
					xmin = segments->x1;
					xmax = segments->x2;
					segments = segments->next;
					{
						if (xmin > current->gc_xcliphi)
						{
							continue;
						}
						if (xmax < current->gc_xcliplo)
						{
							continue;
						}
						if (xmin < current->gc_xcliplo)
						{
							xmin = current->gc_xcliplo;;
						}
						if (xmax > current->gc_xcliphi)
						{
							xmax = current->gc_xcliphi;;
						}
					};
					(*current->scan) (
										 (xmin + current->gc_xoffset),
										 (ypos + current->gc_yoffset),
										 (xmax - xmin + 1),ink,ppop);
				}
			}
		}
	}
	if (mem)
	{
		free(edges);
		free(scans);
	}
}

void do_polygon(const FGPointArray* poly, PolygonStruct * current, FGPixel ink, unsigned ppop)
{
	int n = poly->vertices;
	FGPoint *pt = poly->array;

	int i, px, py, x1, y1, x2, y2, doClose = 1;

	if (n <= 0)
		return;

	if (n == 1)
		doClose = 1;

	x1 = x2 = pt[0].x;
	y1 = y2 = pt[0].y;

	for (i = 1; i < n; i++)
	{
		int pptx = pt[i].x;
		int ppty = pt[i].x;

		if (x1 > pptx)
			x1 = pptx;
		if (x2 < pptx)
			x2 = pptx;
		if (y1 > ppty)
			y1 = ppty;
		if (y2 < ppty)
			y2 = ppty;
	}

	px = pt[n - 1].x;
	py = pt[n - 1].y;

	for (i = 0; i < n; i++)
	{
		x1 = px;
		y1 = py;
		x2 = px = pt[i].x;
		y2 = py = pt[i].y;

		if (i | doClose)
		{
			if (y1 > y2)
			{
				{
					int _swap_tmpval_ = (x1);

					(x1) = (x2);
					(x2) = _swap_tmpval_;
				};
				{
					int _swap_tmpval_ = (y1);

					(y1) = (y2);
					(y2) = _swap_tmpval_;
				};
			}
			{
				if (x1 < x2)
				{
					if (x2 < current->gc_xcliplo)
					{
						continue;
					}
					if (x1 > current->gc_xcliphi)
					{
						continue;
					}
					{
						if (x1 < current->gc_xcliplo)
						{
							y1 += (((int) (((long) (((int) ((y2 - y1)) << 1)) * (long) (current->gc_xcliplo - x1)) / (long) ((x2 - x1))) + (((int) ((y2 - y1)) ^ (int) (current->gc_xcliplo - x1) ^ (int) ((x2 - x1))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							x1 = current->gc_xcliplo;
						}
					};
					{
						if (x2 > current->gc_xcliphi)
						{
							y2 -= (((int) (((long) (((int) ((y2 - y1)) << 1)) * (long) (x2 - current->gc_xcliphi)) / (long) ((x2 - x1))) + (((int) ((y2 - y1)) ^ (int) (x2 - current->gc_xcliphi) ^ (int) ((x2 - x1))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							x2 = current->gc_xcliphi;
						}
					};
				}
				else
				{
					if (x1 < current->gc_xcliplo)
					{
						continue;
					}
					if (x2 > current->gc_xcliphi)
					{
						continue;
					}
					{
						if (x2 < current->gc_xcliplo)
						{
							y2 += (((int) (((long) (((int) ((y1 - y2)) << 1)) * (long) (current->gc_xcliplo - x2)) / (long) ((x1 - x2))) + (((int) ((y1 - y2)) ^ (int) (current->gc_xcliplo - x2) ^ (int) ((x1 - x2))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							x2 = current->gc_xcliplo;
						}
					};
					{
						if (x1 > current->gc_xcliphi)
						{
							y1 -= (((int) (((long) (((int) ((y1 - y2)) << 1)) * (long) (x1 - current->gc_xcliphi)) / (long) ((x1 - x2))) + (((int) ((y1 - y2)) ^ (int) (x1 - current->gc_xcliphi) ^ (int) ((x1 - x2))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							x1 = current->gc_xcliphi;
						}
					};
				}
				if (y1 < y2)
				{
					if (y2 < current->gc_ycliplo)
					{
						continue;
					}
					if (y1 > current->gc_ycliphi)
					{
						continue;
					}
					{
						if (y1 < current->gc_ycliplo)
						{
							x1 += (((int) (((long) (((int) ((x2 - x1)) << 1)) * (long) (current->gc_ycliplo - y1)) / (long) ((y2 - y1))) + (((int) ((x2 - x1)) ^ (int) (current->gc_ycliplo - y1) ^ (int) ((y2 - y1))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							y1 = current->gc_ycliplo;
						}
					};
					{
						if (y2 > current->gc_ycliphi)
						{
							x2 -= (((int) (((long) (((int) ((x2 - x1)) << 1)) * (long) (y2 - current->gc_ycliphi)) / (long) ((y2 - y1))) + (((int) ((x2 - x1)) ^ (int) (y2 - current->gc_ycliphi) ^ (int) ((y2 - y1))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							y2 = current->gc_ycliphi;
						}
					};
				}
				else
				{
					if (y1 < current->gc_ycliplo)
					{
						continue;
					}
					if (y2 > current->gc_ycliphi)
					{
						continue;
					}
					{
						if (y2 < current->gc_ycliplo)
						{
							x2 += (((int) (((long) (((int) ((x1 - x2)) << 1)) * (long) (current->gc_ycliplo - y2)) / (long) ((y1 - y2))) + (((int) ((x1 - x2)) ^ (int) (current->gc_ycliplo - y2) ^ (int) ((y1 - y2))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							y2 = current->gc_ycliplo;
						}
					};
					{
						if (y1 > current->gc_ycliphi)
						{
							x1 -= (((int) (((long) (((int) ((x1 - x2)) << 1)) * (long) (y1 - current->gc_ycliphi)) / (long) ((y1 - y2))) + (((int) ((x1 - x2)) ^ (int) (y1 - current->gc_ycliphi) ^ (int) ((y1 - y2))) >> ((sizeof(int) * 8) - 1)) + 1) >> 1);

							y1 = current->gc_ycliphi;
						}
					};
				}
			};
			(*current->line) (
								 (x1 + current->gc_xoffset),
								 (y1 + current->gc_yoffset),
								 (x2 + current->gc_xoffset),
								 (y2 + current->gc_yoffset),
								 ink, ppop);
		}
	}
}

// fill polygon
void FGAPI fill_convex(const FGPointArray* poly, FGPixel ink, unsigned ppop)
{
	do_fill_polygon(poly, &fgstate.ps, ink,ppop);
}

// draw polygon
void FGAPI draw_convex(const FGPointArray* poly, FGPixel ink, unsigned ppop)
{
	do_polygon(poly, &fgstate.ps,ink,ppop);
}

/** do_ellipse:
 *  Helper function for the ellipse drawing routines. Calculates the points
 *  in an ellipse of radius rx and ry around point x, y, and calls the
 *  specified routine for each one. The output proc will be passed first a
 *  copy of the bmp parameter, then the x, y point, then a copy of the d
 *  parameter (so putpixel() can be used as the callback).
 */
void FGAPI draw_ellipse(int x, int y, int rx, int ry, FGPixel ink, unsigned ppop)
{
	int ix, iy;
	int h, i, j, k;
	int oh, oi, oj, ok;

	if ((y + ry) < clip_y_min || (x + rx) < clip_x_min || (y - ry) >= clip_y_max || (x - rx) >= clip_x_max || rx < 1 || ry < 1)
		return;

	if (rx < 1)
		rx = 1;

	if (ry < 1)
		ry = 1;

	h = i = j = k = 0xFFFF;

	if (rx > ry)
	{
		ix = 0;
		iy = rx * 64;

		do
		{
			oh = h;
			oi = i;
			oj = j;
			ok = k;

			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * ry) / rx;
			k = (i * ry) / rx;

			if (((h != oh) || (k != ok)) && (h < oi))
			{
				vector_draw_point(videobase, x + h, y + k, X_virtual, ink, ppop );
				if (h)
					vector_draw_point(videobase, x - h, y + k, X_virtual, ink, ppop );
				if (k)
				{
					vector_draw_point(videobase, x + h, y - k, X_virtual, ink, ppop );
					if (h)
						vector_draw_point(videobase, x - h, y - k, X_virtual, ink, ppop );
				}
			}

			if (((i != oi) || (j != oj)) && (h < i))
			{
				vector_draw_point(videobase, x + i, y + j, X_virtual, ink, ppop );
				if (i)
					vector_draw_point(videobase, x - i, y + j, X_virtual, ink, ppop );
				if (j)
				{
					vector_draw_point(videobase, x + i, y - j, X_virtual, ink, ppop );
					if (i)
						vector_draw_point(videobase, x - i, y - j, X_virtual, ink, ppop );
				}
			}

			ix = ix + iy / rx;
			iy = iy - ix / rx;

		}
		while (i > h);
	}
	else
	{
		ix = 0;
		iy = ry * 64;

		do
		{
			oh = h;
			oi = i;
			oj = j;
			ok = k;

			h = (ix + 32) >> 6;
			i = (iy + 32) >> 6;
			j = (h * rx) / ry;
			k = (i * rx) / ry;

			if (((j != oj) || (i != oi)) && (h < i))
			{
				vector_draw_point(videobase, x + j, y + i, X_virtual, ink, ppop );
				if (j)
					vector_draw_point(videobase, x - j, y + i, X_virtual, ink, ppop );
				if (i)
				{
					vector_draw_point(videobase, x + j, y - i, X_virtual, ink, ppop );
					if (j)
						vector_draw_point(videobase, x - j, y - i, X_virtual, ink, ppop );
				}
			}

			if (((k != ok) || (h != oh)) && (h < oi))
			{
				vector_draw_point(videobase, x + k, y + h, X_virtual, ink, ppop );
				if (k)
					vector_draw_point(videobase, x - k, y + h, X_virtual, ink, ppop );
				if (h)
				{
					vector_draw_point(videobase, x + k, y - h, X_virtual, ink, ppop );
					if (k)
						vector_draw_point(videobase, x - k, y - h, X_virtual, ink, ppop );
				}
			}

			ix = ix + iy / ry;
			iy = iy - ix / ry;

		}
		while (i > h);
	}
}


/** ellipse fill:
 *  Draws a filled ellipse.
 */
void FGAPI fill_ellipse(int x, int y, int rx, int ry, FGPixel ink, unsigned ppop)
{
	int ix, iy;
	int a, b, c, d;
	int da, db, dc, dd;
	int na, nb, nc, nd;

	if ((y + ry) < clip_y_min || (x + rx) < clip_x_min || (y - ry) >= clip_y_max || (x - rx) >= clip_x_max || rx < 1 || ry < 1)
		return;

	if (rx > ry)
	{
		dc = -1;
		dd = 0xFFFF;
		ix = 0;
		iy = rx * 64;
		na = 0;
		nb = (iy + 32) >> 6;
		nc = 0;
		nd = (nb * ry) / rx;

		do
		{
			a = na;
			b = nb;
			c = nc;
			d = nd;

			ix = ix + (iy / rx);
			iy = iy - (ix / rx);
			na = (ix + 32) >> 6;
			nb = (iy + 32) >> 6;
			nc = (na * ry) / rx;
			nd = (nb * ry) / rx;

			if ((c > dc) && (c < dd))
			{
				draw_hline(x - b, y + c, b * 2,ink,ppop);
				if (c)
					draw_hline(x - b, y - c, b * 2,ink,ppop);
				dc = c;
			}

			if ((d < dd) && (d > dc))
			{
				draw_hline(x - a, y + d, a * 2,ink,ppop);
				draw_hline(x - a, y - d, a * 2,ink,ppop);
				dd = d;
			}

		}
		while (b > a);
	}
	else
	{
		da = -1;
		db = 0xFFFF;
		ix = 0;
		iy = ry * 64;
		na = 0;
		nb = (iy + 32) >> 6;
		nc = 0;
		nd = (nb * rx) / ry;

		do
		{
			a = na;
			b = nb;
			c = nc;
			d = nd;

			ix = ix + (iy / ry);
			iy = iy - (ix / ry);
			na = (ix + 32) >> 6;
			nb = (iy + 32) >> 6;
			nc = (na * rx) / ry;
			nd = (nb * rx) / ry;

			if ((a > da) && (a < db))
			{
				draw_hline(x - d, y + a, d * 2,ink,ppop);
				if (a)
					draw_hline(x - d, y - a, d * 2,ink,ppop);
				da = a;
			}

			if ((b < db) && (b > da))
			{
				draw_hline(x - c, y + b, c * 2,ink,ppop);
				draw_hline(x - c, y - b, c * 2,ink,ppop);
				db = b;
			}

		}
		while (b > a);
	}
}

// ***********************************************************************
/** Calculates a weighted average between x1 and x2.
 */
static inline float FGAPI bez_split(float mu, float x1, float x2)
{
	return (1.0 - mu) * x1 + mu * x2;
}


/**
	Calculates a point on a bezier curve.
 */
static FGPoint FGAPI bezval(float mu, FGPoint coor[4] )
{
	float work[8];
	int i;
	int j;

	for (i = 0; i < 4; i++)
	{
		work[i] = (float) coor[i].x;
		work[i+4] = (float) coor[i].y;
	}

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3 - j; i++)
		{
			work[i] = bez_split(mu, work[i], work[i + 1]);
			work[i+4] = bez_split(mu, work[i+4], work[i + 1 + 4]);
		}
	}

	return FGPoint(work[0], work[4]);
/*
	FGPoint work;
	int i;
	int j;

	for (j = 0; j < 3; j++)
	{
		for (i = 0; i < 3 - j; i++)
		{
			work.x = coor[i].x;
			work.x = bez_split(mu, work.x, coor[i + 1].x);
			work.y = coor[i].y;
			work.y = bez_split(mu, work.y, coor[i + 1].y);
		}
	}
	return work;
*/
}

/**
Calculates a series of npts values along a bezier spline, storing them in
the output x and y arrays. The bezier curve is specified by the four x/y
control points in the points array: points[0] and points[1] contain
the coordinates of the first control point, points[2] and points[3]
are the second point, etc. Control points 0 and 3 are the ends of the spline,
and points 1 and 2 are guides. The curve probably won't pass through points 1 and 2,
but they affect the shape of the curve between points 0 and 3 (the lines p0-p1 and p2-p3
are tangents to the spline). The easiest way to think of it is that the curve starts at p0,
heading in the direction of p1, but curves round so that it arrives at p3
from the direction of p2. In addition to their role as graphics primitives,
spline curves can be useful for constructing smooth paths around a series of control points.
*/
void calc_spline(FGPoint points[4], int npts, FGPoint output[])
{
	double denom;

	for (int i = 0; i < npts; i++)
	{
		denom = (double) i / ((double) npts - 1.0);
		output[i] = bezval(denom, points);
//		output[i].y = (int) bezval(denom, &points[i].y);
	}
}


/**
	Draws a bezier spline using the four control points specified in the points array.
*/
void FGAPI draw_spline(FGPoint points[4], FGPixel ink, unsigned ppop)
{
#define NPTS	  64

	FGPoint pts[NPTS];

	calc_spline(points, NPTS, pts);

	for (int i = 1; i < NPTS; i++)
	{
		vector_draw_line(pts[i - 1].x, pts[i - 1].y, pts[i].x, pts[i].y, ink, ppop);

		if (ppop == _GXOR)
			vector_draw_point(videobase, pts[i].x, pts[i].y, X_virtual, ink, ppop);
	}
}

#ifndef __BORLANDC__
// for compatibility
int stricmp(const char *s1, const char *s2)
{
	while (tolower(*s1) == tolower(*s2))
	{
		if (*s1 == 0)
			return 0;
		s1++;
		s2++;
	}
	return (int) tolower(*s1) - (int) tolower(*s2);
}
#endif

int strnicmp(const char *s1, const char *s2, size_t n)
{

	if (n == 0)
		return 0;
	do
	{
		if (tolower(*s1) != tolower(*s2++))
			return (int) tolower(*s1) - (int) tolower(*--s2);
		if (*s1++ == 0)
			break;
	}
	while (--n != 0);
	return 0;
}

char *strupr(register char *s)
{
	register int c;
	char *s2 = s;

	while ((c = *s) != 0)
	{
		if (islower(c))
			*s = toupper(c);
		s++;
	}
	return s2;
}

char *strlwr(char *_s)
{
	char *rv = _s;

	while (*_s)
	{
		*_s = tolower(*_s);
		_s++;
	}
	return rv;
}


/** Helper function for the arc function. Calculates the points in an arc
 *  of radius r around point x, y, going anticlockwise from fixed point
 *  binary angle ang1 to ang2, and calls the specified routine for each one.
 *  The output proc will be passed first a copy of the bmp parameter, then
 *  the x, y point, then a copy of the d parameter (so putpixel() can be
 *  used as the callback).
 */
void FGAPI draw_arc(int x, int y, double ang1, double ang2, int r, FGPixel ink, unsigned ppop)
{
	int px, py;
	int ex, ey;
	int px1, px2, px3;
	int py1, py2, py3;
	int d1, d2, d3;
	int ax, ay;
	int q, qe;
	double tg_cur, tg_end;
	int done = 0;
	double rr1, rr2, rr3;
	int rr = (r * r);
	int prenes_qv=0;

	rr1 = r;
	rr2 = x;
	rr3 = y;

	/* evaluate the start point and the end point */
	px = (int) (rr2 + rr1 * cos(ang1));
	py = (int) (rr3 - rr1 * sin(ang1));
	ex = (int) (rr2 + rr1 * cos(ang2));
	ey = (int) (rr3 - rr1 * sin(ang2));

	/* start quadrant */
	if (px >= x)
	{
		if (py <= y)
			q = 1;				/* quadrant 1 */
		else
			q = 4;				/* quadrant 4 */
	}
	else
	{
		if (py < y)
			q = 2;				/* quadrant 2 */
		else
			q = 3;				/* quadrant 3 */
	}

	/* end quadrant */
	if (ex >= x)
	{
		if (ey <= y)
			qe = 1;				/* quadrant 1 */
		else
			qe = 4;				/* quadrant 4 */
	}
	else
	{
		if (ey < y)
			qe = 2;				/* quadrant 2 */
		else
			qe = 3;				/* quadrant 3 */
	}

#define loc_tg(_y, _x)  (_x-x) ? (double)(_y-y)/(_x-x) : (double)(_y-y)

	tg_end = loc_tg(ey, ex);

	if (q==qe)  // su v rovnakom kvadrante
	  { if (ang1 > ang2) // kreslenie cez 4 kv
		{
		 prenes_qv=1; qe+=4;
	  } }

	while (!done)
	{
		vector_draw_point(videobase,px, py, X_virtual, ink, ppop );

		/* from here, we have only 3 possible direction of movement, eg.
		 * for the first quadrant:
		 *
		 *    OOOOOOOOO
		 *    OOOOOOOOO
		 *    OOOOOO21O
		 *    OOOOOO3*O
		 */

		/* evaluate the 3 possible points */
		switch (q)
		{

			case 1:
				px1 = px;
				py1 = py - 1;
				px2 = px - 1;
				py2 = py - 1;
				px3 = px - 1;
				py3 = py;

				/* 2nd quadrant check */
				if (px != x)
				{
					break;
				}
				else
				{
					/* we were in the end quadrant, changing is illegal. Exit. */
					if (qe == q)
						done = 1;
					q++; if (prenes_qv) {prenes_qv=0;qe-=4;}
				}
				/* fall through */

			case 2:
				px1 = px - 1;
				py1 = py;
				px2 = px - 1;
				py2 = py + 1;
				px3 = px;
				py3 = py + 1;

				/* 3rd quadrant check */
				if (py != y)
				{
					break;
				}
				else
				{
					/* we were in the end quadrant, changing is illegal. Exit. */
					if (qe == q)
						done = 1;
					q++; if (prenes_qv) {prenes_qv=0;qe-=4;}
				}
				/* fall through */

			case 3:
				px1 = px;
				py1 = py + 1;
				px2 = px + 1;
				py2 = py + 1;
				px3 = px + 1;
				py3 = py;

				/* 4th quadrant check */
				if (px != x)
				{
					break;
				}
				else
				{
					/* we were in the end quadrant, changing is illegal. Exit. */
					if (qe == q)
						done = 1;
					q++;if (prenes_qv) {prenes_qv=0;qe-=4;}
				}
				/* fall through */

			case 4:
				px1 = px + 1;
				py1 = py;
				px2 = px + 1;
				py2 = py - 1;
				px3 = px;
				py3 = py - 1;

				/* 1st quadrant check */
				if (py == y)
				{
					/* we were in the end quadrant, changing is illegal. Exit. */
					if (qe == q)
						done = 1;

					q = 1;if (prenes_qv) {prenes_qv=0;qe-=4;}
					px1 = px;
					py1 = py - 1;
					px2 = px - 1;
					py2 = py - 1;
					px3 = px - 1;
					py3 = py;
				}
				break;

			default:
				return;
		}

		/* now, we must decide which of the 3 points is the right point.
		 * We evaluate the distance from center and, then, choose the
		 * nearest point.
		 */
		ax = x - px1;
		ay = y - py1;
		rr1 = ax * ax + ay * ay;

		ax = x - px2;
		ay = y - py2;
		rr2 = ax * ax + ay * ay;

		ax = x - px3;
		ay = y - py3;
		rr3 = ax * ax + ay * ay;

		/* difference from the main radius */
		if (rr1 > rr)
			d1 = (int) (rr1 - rr);
		else
			d1 = (int) (rr - rr1);
		if (rr2 > rr)
			d2 = (int) (rr2 - rr);
		else
			d2 = (int) (rr - rr2);
		if (rr3 > rr)
			d3 = (int) (rr3 - rr);
		else
			d3 = (int) (rr - rr3);

		/* what is the minimum? */
		if (d1 <= d2)
		{
			px = px1;
			py = py1;
		}
		else if (d2 <= d3)
		{
			px = px2;
			py = py2;
		}
		else
		{
			px = px3;
			py = py3;
		}

		/* are we in the final quadrant? */
		if (qe == q)
		{
			tg_cur = loc_tg(py, px);

			/* is the arc finished? */
			switch (q)
			{

				case 1:
					/* end quadrant = 1? */
					if (tg_cur <= tg_end)
						done = 1;
					break;

				case 2:
					/* end quadrant = 2? */
					if (tg_cur <= tg_end)
						done = 1;
					break;

				case 3:
					/* end quadrant = 3? */
					if (tg_cur <= tg_end)
						done = 1;
					break;

				case 4:
					/* end quadrant = 4? */
					if (tg_cur <= tg_end)
						done = 1;
					break;
			}
		}
	}

	/* draw the last evaluated point */
	vector_draw_point(videobase,px, py,X_virtual, ink, ppop);
}

void AlphaBlit(FGPixel * dst_data, FGPixel * src_data, int len, int alpha, unsigned ppop);

#if FASTGL_BPP==32
#define ALPHA_BLIT(src_pix, dst_pix) res = ((src_pix & 0xFF00FF) - (dst_pix & 0xFF00FF)) * alpha / 256 + dst_pix; 	src_pix &= 0xFF00;		dst_pix &= 0xFF00;		gc = (src_pix - dst_pix) * alpha / 256 + dst_pix; res &= 0xFF00FF; gc &= 0xFF00; *dst_data = gc | res;
#endif

#if FASTGL_BPP==16 || FASTGL_BPP==15
//#define ALPHA_BLIT res = ((src_pix & 0xF81F) - (dst_pix & 0xF81F)) * alpha / 256 + dst_pix; 	src_pix &= 0x7E0;		dst_pix &= 0x7E0;		gc = (src_pix - dst_pix) * alpha / 256 + dst_pix; res &= 0xF81F; gc &= 0x7E0; *dst_data = gc | res;
#define ALPHA_BLIT(c, l) rc = kRedComponent(c); \
				gc = kGreenComponent(c); \
				bc = kBlueComponent(c);  \
				rl = kRedComponent(l); \
				gl = kGreenComponent(l); \
				bl = kBlueComponent(l); \
				rc = (rc - rl) * alpha / 256 + rl; \
				gc = (gc - gl) * alpha / 256 + gl; \
				bc = (bc - bl) * alpha / 256 + bl; \
				*dst_data = FGMakeColor(rc,gc,bc);
#endif

//#if FASTGL_BPP==8
//#define ALPHA_BLIT res = g = src_pix; *dst_data = g | res;
//#endif

inline void AlphaBlit(FGPixel * dst_data, FGPixel * src_data, int len, int alpha, unsigned ppop)
{
#if FASTGL_BPP != 8
	unsigned dst_pix, src_pix;
	unsigned rc, gc, bc;
	unsigned rl, gl, bl;
	unsigned res;

	if (ppop != _GTRANSPARENT)
		while (len--)
		{
			src_pix = *src_data++;
			dst_pix = *dst_data;
			ALPHA_BLIT(src_pix, dst_pix)

			dst_data++;
		}
	else
		while (len--)
		{
			if ((src_pix = *src_data++) != (unsigned)fgstate.color_key)
			{
				dst_pix = *dst_data;
				ALPHA_BLIT(src_pix, dst_pix)
			}
			dst_data++;
		}
#endif
}

void L1RamToVideoA(FGPixel * src, FGPixel * dst, int w, int h, int offsrc, int offdst, int alpha, unsigned ppop)
{
	while (h--)
	{
		AlphaBlit(dst, src, w, alpha, ppop);
		dst += offdst;
		src += offsrc;
	}
}

void L1RamToRamA(FGPixel * src, FGPixel * dst, int w, int h, int offsrc, int offdst, int alpha, unsigned ppop)
{
	while (h--)
	{
		AlphaBlit(dst, src, w, alpha, ppop);
		dst += offdst;
		src += offsrc;
	}
}

void FGAPI moveto(int x, int y)
{
	fgstate._oldx = x;
	fgstate._oldy = y;
}

int FGAPI drawto_line(int x, int y, FGPixel ink, unsigned ppop)
{
	int rc = vector_draw_line(fgstate._oldx, fgstate._oldy, x, y, ink, ppop);
	moveto(x,y);
	return rc;
}

unsigned long FGAPI FGClock(void)
{
	static int first = 1, res;
	static struct timeb tbuf, tcur, tlast;
	if (first)
	{
		first = 0;
		ftime(&tbuf);
	}
	ftime(&tcur);
	tlast = tcur;

	res = (tcur.time - tbuf.time) * 1000 + (tcur.millitm - tbuf.millitm);
	// test for time change
	if (res<0)
	{
		tbuf = tcur;
		res = 0;
	}
	return res;
}

//
// patterned line
//
static void __pline(plotter_t draw, int a1, int b1, int a2, int b2, FGPattern * pat, FGPixel ink, unsigned ppop)
{
	int xend, yend, dx, dy, c1, c2, step;
	register int p, x, y;
	FGPixel iscolor;

	dx = abs(a2 - a1);
	dy = abs(b2 - b1);

	if (dx > 32000)
		return;					// bulgar const

	if (dy > 32000)
		return;

	if (pat->reset)
		pat->pos = 0;

	if (dx > dy)
	{							/* slope < 1 => step in x direction */
		x = MIN(a1, a2);
		xend = MAX(a1, a2);
		if (x == a1)
		{
			y = b1;
			step = ((b2 - y) < 0 ? -1 : 1);
		}
		else
		{
			y = b2;
			step = ((b1 - y) < 0 ? -1 : 1);
		}

		p = 2 * dy - dx;
		c1 = 2 * dy;
		c2 = 2 * (dy - dx);

		iscolor=pat->next();
		if (iscolor)
			draw(videobase, x, y, X_virtual, iscolor, ppop);
		while (x < xend)
		{
			x++;
			if (p < 0)
			{
				p += c1;
			}
			else
			{
				y += step;
				p += c2;
			}
			iscolor=pat->next();
			if (iscolor)
				draw(videobase, x, y, X_virtual, iscolor, ppop);
		}
	}
	else
	{							/* slope > 1 => step in y direction */
		y = MIN(b1, b2);
		yend = MAX(b1, b2);
		if (y == b1)
		{
			x = a1;
			step = ((a2 - x) < 0 ? -1 : 1);
		}
		else
		{
			x = a2;
			step = ((a1 - x) < 0 ? -1 : 1);
		}

		p = 2 * dx - dy;
		c1 = 2 * dx;
		c2 = 2 * (dx - dy);

		iscolor=pat->next();
		if (iscolor)
			draw(videobase, x, y, X_virtual, iscolor, ppop);

		while (y < yend)
		{
			y++;
			if (p < 0)
			{
				p += c1;
			}
			else
			{
				x += step;
				p += c2;
			}
			iscolor=pat->next();
			if (iscolor)
				draw(videobase, x, y, X_virtual, iscolor, ppop);
		}
	}
}

//
// globalny draw_line() na obrazovku
//
int __draw_line(int a1, int b1, int a2, int b2, FGPixel color, unsigned ppop)
{
	int xend, yend, dx, dy, c1, c2, step;
	register int p, x=0, y=0;
	int cl;

	cl = ClipLine(a1,b1,a2,b2,clip_x_min, clip_y_min, clip_x_max, clip_y_max);

	if (cl==0) return 0;

	moveto(x,y);
	dx = abs(a2 - a1);
	dy = abs(b2 - b1);

	if (dx > 32000)
		return 0;					// bulgar const

	if (dy > 32000)
		return 0;

	if (dx > dy)
	{							/* slope < 1 => step in x direction */
		x = MIN(a1, a2);
		xend = MAX(a1, a2);
		if (x == a1)
		{
			y = b1;
			step = ((b2 - y) < 0 ? -1 : 1);
		}
		else
		{
			y = b2;
			step = ((b1 - y) < 0 ? -1 : 1);
		}

		p = 2 * dy - dx;
		c1 = 2 * dy;
		c2 = 2 * (dy - dx);

		draw_point(x, y, color, ppop);
		while (x < xend)
		{
			x++;
			if (p < 0)
			{
				p += c1;
			}
			else
			{
				y += step;
				p += c2;
			}
			draw_point(x, y, color, ppop);
		}
	}
	else
	{							/* slope > 1 => step in y direction */
		y = MIN(b1, b2);
		yend = MAX(b1, b2);
		if (y == b1)
		{
			x = a1;
			step = ((a2 - x) < 0 ? -1 : 1);
		}
		else
		{
			x = a2;
			step = ((a1 - x) < 0 ? -1 : 1);
		}

		p = 2 * dx - dy;
		c1 = 2 * dx;
		c2 = 2 * (dx - dy);

		draw_point(x, y, color, ppop);
		while (y < yend)
		{
			y++;
			if (p < 0)
			{
				p += c1;
			}
			else
			{
				x += step;
				p += c2;
			}
			draw_point(x, y, color, ppop);
		}
	}
	return cl;
}

void FGAPI draw_pattern_line(int x1, int y1, int x2, int y2, FGPattern * p, unsigned ppop)
{
	__pline(vector_draw_point, x1, y1, fgstate.xold = x2, fgstate.yold = y2, p, 0, ppop);
}

void FGAPI drawto_pattern_line(int x2, int y2, FGPattern * p, unsigned ppop)
{
	__pline(vector_draw_point, fgstate.xold, fgstate.yold, x2, y2, p, 0, ppop);
	fgstate.xold = x2;
	fgstate.yold = y2;
}

void FGAPI draw_pattern_box(int x, int y, int a, int b, FGPattern *pat, unsigned ppop)
{
	int tmp = pat->reset;
	pat->reset=1;
	draw_pattern_line(x, y, x+a-1, y, pat,ppop);
	pat->reset=0;
	draw_pattern_line(x+a-1, y+1, x+a-1, y+b-1, pat,ppop);
	draw_pattern_line(x+a-2, y+b-1, x, y+b-1, pat,ppop);
	draw_pattern_line(x, y+b-2, x, y+1, pat,ppop);
	pat->reset = tmp;
}

void outpb(unsigned port, unsigned val)
{
#ifdef __BORLANDC__
	asm
	{
		mov   edx,dword ptr [port]
		movzx eax,byte  ptr [val]
		out dx,al
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("outb %%al,%%dx": :"d" (port), "a" (val));
#endif
}

void outpw(unsigned port, unsigned val)
{
#ifdef __BORLANDC__
	asm
	{
		mov   edx,dword ptr [port]
		movzx eax,word  ptr [val]
		out dx,ax
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("outw %%ax,%%dx": :"d" (port), "a" (val));
#endif
}

void outpl(unsigned port, unsigned val)
{
#ifdef __BORLANDC__
	asm
	{
		mov edx,dword ptr [port]
		mov eax,dword ptr [val]
		out dx,eax
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("outl %%eax,%%dx": :"d" (port), "a" (val));
#endif
}

unsigned char inpb(unsigned port)
{
	unsigned char input;
#ifdef __BORLANDC__
	asm
	{
		xor eax,eax
		mov edx,dword ptr [port]
		in  al,dx
		mov [input],al
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("inb %%dx,%%al\n"
							: "=a" (input)
							: "d" (port));
#endif
	return input;
}

unsigned short inpw(unsigned port)
{
	unsigned short input;
#ifdef __BORLANDC__
	asm
	{
		xor eax,eax
		mov edx,dword ptr [port]
		in  ax,dx
		mov [input],ax
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("inw %%dx,%%ax\n"
							: "=a" (input)
							: "d" (port));
#endif
	return input;
}

unsigned int inpl(unsigned port)
{
	unsigned int input;
#ifdef __BORLANDC__
	asm
	{
		xor eax,eax
		mov edx,dword ptr [port]
		in  eax,dx
		mov [input],eax
	}
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm__ __volatile__ ("inl %%dx,%%eax\n"
							: "=a" (input)
							: "d" (port));
#endif
	return input;
}

static inline void FGAPI modify_point(FGPixel *ptr, FGPixel c, unsigned ppop)
{
	switch (ppop)
	{
		default:
		case 0:
			*ptr =	c;
			break;
		case 1:					// xor
			*ptr^= c;
			break;
		case 2:					// and
			*ptr &= c;
			break;
		case 3:					// or
			*ptr |= c;
			break;
		case 4:					// plus
			*ptr += c;
			break;
		case 5:					// minus
			*ptr -= c;
			break;
		case 6:					// not
			*ptr ^= 0x00ffffff;
			break;
		case _GREPLACE_GE16:					// replace
			if (*ptr>=16) *ptr = c;
			break;
		case _GTRANSPARENT:					// alpha transparency
			if (c != 0) *ptr = c;
			break;
		case _GCOLORKEY:					// color key transparency
			if ( c!= fgstate.color_key) *ptr = c;
			break;
		case _GREPLACE_LE32:					// replace
			if (*ptr < 32) *ptr = c;
			break;
		case _GREPLACE_LE48:					// replace
			if (*ptr < 48) *ptr = c;
			break;
		case _GREPLACE_LE64:					// replace
			if (*ptr < 64) *ptr = c;
			break;
		case _GREPLACE_LE80:					// replace
			if (*ptr < 80) *ptr = c;
			break;
		case _GREPLACE_LESS:					// replace
			if (*ptr < c) *ptr = c;
			break;
	}
}

//
// NOTE! this runs with pixels not bytes !!!
//
inline void FGmemset2(FGPixel * to, FGPixel data, unsigned c, unsigned ppop)
{
	while (c--)
		modify_point(to++, data, ppop);
}

//
// NOTE! this runs with pixels not bytes !!!
//
//inline
void FGmemcpy(FGPixel * to, FGPixel* from, unsigned c)
{
#ifdef BPP32xy
	if (fgstate.__fgl_mmx && c>16)
	{
		#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
		asm("emms");

		asm volatile ("rep\n\t stosl\n"
			: "=D" (__d0), "=c" (__d1)
		:"c" (c), "D"(to), "a"(data) : "memory");

		asm("emms");
		#endif

		#ifdef __BORLANDC__
		c >>= 1;
		__asm
		{
			emms
			mov ecx,[c]
			xor ebx,ebx
			mov esi,[from]
			mov edi,[to]
L0:
			movdqu xmm0, dqword ptr[esi+ebx]
			movdqu dqword ptr[edi+ebx],xmm0
/*
			mov eax,[esi+ebx*4]
			mov [edi+ebx*4],eax
*/
			add ebx,8
			dec ecx
			test ecx,ecx
			jnz L0
			emms
		}
		#endif
	}
	else
#endif
	if (c>64)
	{
		memcpy(to, from, sizeof(FGPixel)*c );
	}
	else
	{
		while (c--)
			*to++ = *from++;
	}
}

//
// NOTE! this runs with pixels not bytes !!!
//
inline void FGmemcpyd(FGPixel * to, FGPixel* from, unsigned c)
{
	while (c--)
		*to-- = *from--;
}

//
// NOTE! this runs with pixels not bytes !!!
//
inline void FGmemcpy2(FGPixel * to, FGPixel* from, unsigned c, unsigned ppop)
{
	while (c--)
		modify_point(to++, *from++, ppop);
}

void L1RamToRam(FGPixel *from,	FGPixel *to, int w, int h, int step_src, int step_dst)
{
	for(int i=0; i<h; i++)
	{
		FGmemcpy(to,from,w);
		from += step_src;
		to += step_dst;
	}
}

void L1RamToRamd(FGPixel *from,	FGPixel *to, int w, int h, int step_src, int step_dst)
{
	for(int i=0; i<h; i++)
	{
		FGmemcpyd(to,from,w);
		from -= step_src;
		to -= step_dst;
	}
}

void L1RamToRamPpop(FGPixel *from,	FGPixel *to, int w, int h, int step_src, int step_dst, unsigned ppop)
{
	for(int i=0; i<h; i++)
	{
		FGmemcpy2(to,from,w,ppop);
		from += step_src;
		to += step_dst;
	}
}

//! fill box in buffer without drawmode
void L1Box(FGPixel	*ptr, int w, int h, FGPixel color, int xoffset, unsigned ppop)
{
	if (ppop == _GSET)
	{
		for(;h>0;h--)
		{
			FGmemset(ptr, color, w);
			ptr += xoffset;
		}
	}
	else
	{
		for(;h>0;h--)
		{
			FGmemset2(ptr, color, w, ppop); // !!!!!!!!!!!!!!
			ptr += xoffset;
		}
	}
}

//! copy block from video to RAM
void L1VideoToRam8(FGPixel	*from, FGPixel	*to, int w, int h, int off1, int off2)
{
	for(;h>0;h--)
	{
		FGmemcpy(to, from, w); // !!!!!!!!!!!!!!
		from += off1;
		to += off2;
	}
}

void L1RamToVideo8(FGPixel	*from, FGPixel *to, int w,	int h, int off1, int off2)
{
	for(;h>0;h--)
	{
		FGmemcpy(to, from, w); // !!!!!!!!!!!!!!
		from += off1;
		to += off2;
	}
}

void L1RamToVideo2(FGPixel	*from, FGPixel *to, int w,	int h, int off1, int off2, unsigned ppop)
{
	for(;h>0;h--)
	{
		FGmemcpy2(to, from, w, ppop); // !!!!!!!!!!!!!!
		from += off1;
		to += off2;
	}
}

void draw_hline(int x, int y, int len, FGPixel ink, unsigned ppop)
{
	draw_line(x,y,x+len,y,ink,ppop);
}

void draw_vline(int x, int y, int len, FGPixel ink, unsigned ppop)
{
	draw_line(x,y,x,y+len,ink,ppop);
}

int __fill_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
{
	L1Box(videobase+(X_virtual*y+x),w,h,ink,X_virtual, ppop);
	return 1;
}

void __draw_point(FGPixel *ptr, int x, int y, int ww, FGPixel c, unsigned ppop)
{
	if (y <	clip_y_min || x < clip_x_min || y >= clip_y_max || x >= clip_x_max) return;
	modify_point(ptr + ww * y + x, c, ppop);
}

int get_point(int x, int y)
{
	if (y <	clip_y_min || x < clip_x_min || y >= clip_y_max || x >= clip_x_max) return 256;
	return *(videobase + X_virtual * y + x);
}

#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
//
// NOTE! this runs with pixels not bytes !!!
//
void FGmemset(FGPixel * to, FGPixel data, unsigned c)
{
	register volatile unsigned long int __d0, __d1;
	__asm__("cld");
#ifdef INDEX_COLORS
	asm volatile ("imull $0x01010101,%%eax\n\t rep\n\t stosb\n"
		: "=D" (__d0), "=c" (__d1)
	:"c" (c), "D"(to), "a"(data) : "memory");
#endif
#ifdef DIRECT_COLORS
	asm volatile ("imull $0x00010001,%%eax\n\t rep\n\t stosw\n"
		: "=D" (__d0), "=c" (__d1)
	:"c" (c), "D"(to), "a"(data) : "memory");
#endif
#ifdef TRUE_COLORS
	asm volatile ("rep\n\t stosl\n"
		: "=D" (__d0), "=c" (__d1)
	:"c" (c), "D"(to), "a"(data) : "memory");
#endif
}

#elif __BORLANDC__ // borland

//
// NOTE! this runs with pixels not bytes !!!
//
void FGmemset(FGPixel * to, FGPixel data, unsigned c)
{
	asm cld;
	asm push ecx
	asm push edi
#ifdef INDEX_COLORS
	int cc;

	while (int (to) & 3)
	{
		if (--c & 0x80000000)
			goto exi;
		*to++ = data;
	}
	cc = c >> 2;
	if (cc < 4)					// if less than 16 bytes
	{
		asm mov edi,[to]
		asm mov al,[data]
		asm mov ecx,[c]
		asm rep stosb;
	}
	else
	{
		asm
		{
			mov 	al,[data]
			mov 	ah,al
			push 	ax
			push	ax
			pop 	eax
			mov 	edi,[to]
			mov 	ecx,[cc]
			rep 	stosd
		}

		if (c &= 3)
		{
			asm mov al,[data]
			asm mov ecx,[c]
			asm	and ecx,3
			asm rep stosb
		}
	}
#endif
#ifdef DIRECT_COLORS
	asm mov edi,[to]
	asm mov ax,[data]
	asm mov ecx,[c]
	asm rep stosw
#endif
#ifdef TRUE_COLORS
	asm mov edi,[to]
	asm mov eax,[data]
	asm mov ecx,[c]
	asm rep stosd
#endif
exi:
	asm pop edi
	asm pop ecx
}

#else

void FGmemset(FGPixel * to, FGPixel data, unsigned c)
{
#ifdef INDEX_COLORS
	memset(to,data,c);
#else
	while (c--)	*to++ = data;
#endif
}
#endif // other compilers

#ifdef FG_NAMESPACE
}
#endif

/**
* Nasty hack to get working Intel C++ 7.1 with FEDORA core 1
*/
#ifdef __ICC
extern "C" const unsigned short int *__ctype_b;
static unsigned short int pole[384];
const unsigned short int *__ctype_b=pole;
#endif





