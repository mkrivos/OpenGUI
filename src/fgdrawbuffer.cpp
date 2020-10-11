/*
   Copyright (C) 1996,2004  Marian Krivos

   nezmar@atlas.sk

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   bitmap.cc - bitmap engine
 */

#include <stdarg.h>

#include "fgbase.h"
#include "_fastgl.h"


#ifdef FG_NAMESPACE
namespace fgl {
#endif

int FGDrawBuffer::ID_counter = 1;		// incremental ID of all graph. objects
int FGDrawBuffer::oldx, FGDrawBuffer::oldy;
const char *FGDrawBuffer::empty_string="";

/**
	Performs image rectangle copy from one FGDrawBuffer object to other with specified
	ROP and clipping.
	@return false if no drawing was performed, or true if all ran OK.
	@param xdst x coord into the destination block
	@param ydst y coord into the destination block
	@param xsrc x coord into the source block
	@param ysrc y coord into the source block
	@param ww a width of copied block (maybe bigger than src or dest block)
	@param hh a height of copied block (maybe bigger than src or dest block)
	@param src a source object that will be copied
	@param opcia a bit oriented value that describes the operational mode of procedure:
-	BITBLIT_COPY - copy only
-	BITBLIT_WITH_OPERATOR - copy with operator
-	BITBLIT_WITH_ALPHA - use the source alpha value
-	BITBLIT_WITH_CLIPPING - perform clipping before
-	BITBLIT_WITH_DECREMENT - for use with overlapping blocks
	@see Defines for more about one.
*/
int FGAPI FGDrawBuffer::bitblit(int xdst, int ydst, int xsrc, int ysrc, int ww, int hh, FGDrawBuffer *src, int opcia)
{
	int ppp = state._ppop, oldt = fgstate.color_key;
	int _ww=src->w, _hh=src->h;

	if (src->image == 0 || src->type == BMP_NONE || image==0)
		return 0;

	xsrc += src->xoff;
	ysrc += src->yoff;
	xdst += xoff;
	ydst += yoff;

	// compute the tile size
	if (src->clip(xsrc, ysrc, _ww, _hh)==0) return 0;
	if (ww<1 || hh<1) return 0;

	// test for transparency ... and force one
	if (src->GetAlpha())
		opcia = (src->GetAlpha() | BITBLIT_WITH_OPERATOR);

	if (src->GetColorKey() != DEFAULT_COLOR_KEY)
	{
		ppp = _GCOLORKEY;
		fgstate.color_key = src->GetColorKey();
		opcia |= BITBLIT_WITH_OPERATOR;
	}
	for (int i = 0; i < hh;)
	{
		for (int j = 0; j < ww;)
		{
			int _x=xdst+j,
				_y=ydst+i,
				_w=j+_ww>ww?ww-j:_ww,
				_h=i+_hh>hh?hh-i:_hh;

			if (clip(_x, _y, _w, _h))
				RamToRam(xsrc+(_x-(xdst+j)), ysrc+(_y-(ydst+i)), src->w, src->h,
					_x, _y, w, h,
					_w, _h, src->image, image, opcia, ppp);
			j += _ww;
		}
		i += _hh;
	}
	fgstate.color_key = oldt;
	return 1;
}

FGDrawBuffer::FGDrawBuffer(int ws, int hs, const char *nm, ObjectType typ,  FGPixel i, FGPixel p)
{
	_init(ws, hs, typ, i, p, nm);
}

/**
	Create a DRAWBUFFER object.
	Parameter 'ww' and 'hh' are width and height of rectangle, 'buf'
	is pointed to the image data taht will be copied into created object.
*/
FGDrawBuffer::FGDrawBuffer(int ww, int hh, FGPixel * buf)
{
	_init(ww, hh, BMP_MEM, 0);
	Resize(0, 0);
	memcpy(image, buf, areasize(w,h));
}

/**
	Creates the object with specified size, type, color layout or already existing pixel data.
*/
FGDrawBuffer::FGDrawBuffer(int ww, int hh, ObjectType t, int color, FGPixel * buf)
{
	_init(ww, hh, t, color);
	if (ww > 0 && hh > 0)
	{
		if (!buf)
		{
			Resize(0, 0);
			clear(color);
		}
		else image = buf;
	}
	else image = 0;
}

/**
	Release object's members.
*/
FGDrawBuffer::~FGDrawBuffer()
{
	if (name && name != empty_string) delete []name;
	if (type && image && type!=FGFRAMEBUFFER && type != BMP_IMAGE)
	{
		free(image);
	}
}

FGDrawBuffer::FGDrawBuffer(FGDrawBuffer& old)
{
	*this = old;
	id = ID_counter++;
	atomic = FGMutex();
	if (old.name && old.name != empty_string)
	{
		assert(name	 = new char[strlen(old.name)+1]);
		strcpy(name, old.name);
	}
	else
	{
		name = (char *)empty_string;
	}
	if (old.image && old.type!=FGFRAMEBUFFER && old.type != BMP_IMAGE)
	{
		image = (FGPixel *) malloc(areasize(old.w, old.h));
		memcpy(image, old.image, areasize(old.w, old.h));
	}
}

void FGAPI FGDrawBuffer::_init(int ww, int hh, ObjectType t, FGPixel ii,  FGPixel pp, const char *nm)
{
	lock();
	id = ID_counter++;
	if (nm)
	{
		assert(name	 = new char[strlen(nm)+1]);
		strcpy(name,nm);
	}
	else
	{
		name = (char *)empty_string;
	}
	state_saved = false;
	image = 0;
	state.alpha = DEFAULT_ALPHA_VALUE;
	state.colorkey = DEFAULT_COLOR_KEY;
	w = ww;
	h = hh;
	xoff=yoff=0;
	wwrk=ww;
	hwrk=hh;
	type = t;
	status = WHIDEN|WLOCKED;
	state._ink = ii;
	state._paper = pp;
	state._ppop = _GSET;
	set_font(FONT0816);
	unlock();
}

void FGAPI FGDrawBuffer::modify_point(FGPixel *ptr, FGPixel ink)
{
	switch (state._ppop)
	{
		default:
		case 0:
			*ptr = ink;
			break;
		case 1:					// xor
			*ptr^= ink;
			break;
		case 2:					// and
			*ptr &= ink;
			break;
		case 3:					// or
			*ptr |= ink;
			break;
		case 4:					// plus
			*ptr += ink;
			break;
		case 5:					// minus
			*ptr -= ink;
			break;
		case 6:					// not
			*ptr ^= 0xffffff;
			break;
		case _GREPLACE_GE16:			// replace
			if (*ptr>=16) *ptr = ink;
			break;
		case _GTRANSPARENT:					// alpha transparency
			if (*ptr != 0) *ptr = ink;
			break;
		case _GCOLORKEY:					// color key transparency
			if (*ptr != fgstate.color_key) *ptr = ink;
			break;
		case _GREPLACE_LE32:
			if (*ptr < 32) *ptr = ink;
			break;
		case _GREPLACE_LE48:
			if (*ptr < 48) *ptr = ink;
			break;
		case _GREPLACE_LE64:
			if (*ptr < 64) *ptr = ink;
			break;
		case _GREPLACE_LE80:
			if (*ptr < 80) *ptr = ink;
			break;
		case _GREPLACE_LESS:
			if (*ptr < ink) *ptr = ink;
			break;
	}
}

/**
	Draw a horizontal line from [x:y] with size 'c' and color 'ink'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::hline(int x, int y, int c, FGPixel ink)
{
	FGPixel *ptr;
	int	j,stuff=1;

	x += xoff;
	y += yoff;

	if (clip(x,y,c,stuff) == 0)
		return 0;

	ptr	= image	+ w * y + x;

	switch(state._ppop)
	{
		default:
		case 0:	 // set
			FGmemset(ptr, ink, c);
			break;
		case 1:	 // xor
			for	(j = c;	j; j--)
			{
				*ptr++ ^= ink;
			}
			break;
		case 2:	 // and
			for	(j = c;	j; j--)
			{
				*ptr++ &= ink;
			}
			break;
		case 3:	 // or
			for	(j = c;	j; j--)
			{
				*ptr++ |= ink;
			}
			break;
		case 4:	 // plus
			for	(j = c;	j; j--)
			{
				*ptr++ += ink;
			}
			break;
		case 5:	 // minus
			for	(j = c;	j; j--)
			{
				*ptr++ -= ink;
			}
			break;
		case 6:	 // not
			for	(j = c;	j; j--)
			{
				*ptr++ ^= 0x00ffffff;
			}
			break;
		case _GREPLACE_GE16:	 // replace
			for	(j = c;	j; j--)
			{
				if (*(ptr) >= 16)
					*ptr = ink;
				ptr++;
			}
			break;
		case _GTRANSPARENT:
			for	(j = c;	j; j--)
			{
				if (*ptr != 0)
					*ptr = ink;
				ptr++;
			}
			break;
		case _GCOLORKEY:	 // replace
			for	(j = c;	j; j--)
			{
				if (*ptr != fgstate.color_key)
					*ptr = ink;
				ptr++;
			}
			break;
		case _GREPLACE_LE32:	 // replace2
			for	(j = c;	j; j--)
			{
				if (*(ptr) < 32) *ptr = ink;
				ptr++;
			}
			break;
		case _GREPLACE_LE48:	 // replace2
			for	(j = c;	j; j--)
			{
				if (*(ptr) < 48) *ptr = ink;
				ptr++;
			}
			break;
		case _GREPLACE_LE64:	 // replace2
			for	(j = c;	j; j--)
			{
				if (*(ptr) < 64) *ptr = ink;
				ptr++;
			}
			break;
		case _GREPLACE_LE80:	 // replace2
			for	(j = c;	j; j--)
			{
				if (*(ptr) < 80) *ptr = ink;
				ptr++;
			}
			break;
		case _GREPLACE_LESS:	 // replace4
			for	(j = c;	j; j--)
			{
				if (*(ptr) < ink) *ptr = ink;
				ptr++;
			}
			break;
	}
	return 1;
}

/**
	Draw a vertical line from [x:y] with size 'c' and color 'ink'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::vline(int x, int y, int c, FGPixel ink)
{
	FGPixel *ptr;
	int	i, stuff=1;

	x += xoff;
	y += yoff;

	if (clip(x,y,stuff,c) == 0)
		return 0;

	ptr	= image	+ w * y + x;

	for	(i = c;	i; i--)
	{
		modify_point(ptr, ink);
		ptr	+= w;
	}
	return 1;
}

/**
	Draw a pixel at position [x:y] with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinates are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::putpixel(int x, int y, unsigned color)
{
	x += xoff;
	y += yoff;

	if (y <	yoff 
	    || x < xoff 
	    || y >= (hwrk+yoff) 
	    || x >= (wwrk+xoff))
		return 0;
	modify_point(image + w * y + x, color == UNDEFINED_COLOR ? state._ink : color);
	return 1;
}

// internal for circle draw to RAM
void   FGAPI FGDrawBuffer::_symetry(int xs, int ys, int x,	int	y, FGPixel color)
{
	putpixel(xs + x, ys + y, color);
	putpixel(xs - x, ys + y, color);
	putpixel(xs + x, ys - y, color);
	putpixel(xs - x, ys - y, color);
	putpixel(xs + y, ys + x, color);
	putpixel(xs - y, ys + x, color);
	putpixel(xs + y, ys - x, color);
	putpixel(xs - y, ys - x, color);
}

// internal for circle fill to RAM
void	FGAPI FGAPI FGDrawBuffer::_symetry2(int xs, int ys, int x, int y, FGPixel color)
{
	hline(xs-x,ys+y,x+x+1, color);
	hline(xs-x,ys-y,x+x+1, color);
	hline(xs-y,ys+x,y+y+1, color);
	hline(xs-y,ys-x,y+y+1, color);
}

/**
	Draw a circle shape at position [x:y] with radius and with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::circle(int	xs,	int	ys,	int	r, unsigned color)
{
	int	x =	0;
	int	y =	r;
	int	p =	3 -	2 *	r;

	if ((ys	+ r) < 0 || (xs +	r) < 0
		||	(ys	- r) >=	h || (xs -	r) >= w
		||	r <	1) return 0;

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

	while (x < y)
	{
		_symetry(xs, ys, x,	y, color);
		if (p <	0)
			p += 4 * (x++) + 6;
		else
			p += 4 * ((x++)	- (y--)) + 10;
	}
	if (x == y)
		_symetry(xs, ys, x,	y, color);
	return 1;
}

/**
	Fill a circle shape at position [x:y] with radius and with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::fcircle(int	xs,	int	ys,	int	r, unsigned color)
{
	int	x =	0;
	int	y =	r;
	int	p =	3 -	2 *	r;

	if ((ys	+ r) < 0 || (xs +	r) < 0
		||	(ys	- r) >=	h || (xs -	r) >= w
		||	r <	1) return 0;

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

	while (x < y)
	{
		_symetry2(xs, ys, x, y, color);
		if (p <	0)
			p += 4 * (x++) + 6;
		else
			p += 4 * ((x++)	- (y--)) + 10;
	}
	if (x == y)
		_symetry2(xs, ys, x, y, color);
	return 1;
}

/**
	Draw an ellipse shape at position [x:y] with radius 'rx' & 'ry'and with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::ellipse(int	x,	int	y,	int	rx, int ry, unsigned color)
{
	if ((y	+ ry) < 0 || (x + rx) < 0 || (y - ry) >= h || (x	- rx) >= w || rx<1 || ry<1)
		return 0;
	int ix, iy;
	int h, i, j, k;
	int oh, oi, oj, ok;

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

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
				putpixel(x + h, y + k, color);
				if (h)
					putpixel(x - h, y + k, color);
				if (k)
				{
					putpixel(x + h, y - k, color);
					if (h)
						putpixel(x - h, y - k, color);
				}
			}

			if (((i != oi) || (j != oj)) && (h < i))
			{
				putpixel(x + i, y + j, color);
				if (i)
					putpixel(x - i, y + j, color);
				if (j)
				{
					putpixel(x + i, y - j, color);
					if (i)
						putpixel(x - i, y - j, color);
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
				putpixel(x + j, y + i, color);
				if (j)
					putpixel(x - j, y + i, color);
				if (i)
				{
					putpixel(x + j, y - i, color);
					if (j)
						putpixel(x - j, y - i, color);
				}
			}

			if (((k != ok) || (h != oh)) && (h < oi))
			{
				putpixel(x + k, y + h, color);
				if (k)
					putpixel(x - k, y + h, color);
				if (h)
				{
					putpixel(x + k, y - h, color);
					if (k)
						putpixel(x - k, y - h, color);
				}
			}

			ix = ix + iy / ry;
			iy = iy - ix / ry;

		}
		while (i > h);
	}
	return 1;
}

/**
	Fill an ellipse shape at position [x:y] with radius 'rx' & 'ry'and with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::fellipse(int	x,	int	y,	int	rx, int ry, unsigned color)
{
	if ((y	+ ry) < 0 || (x + rx) < 0 || (y - ry) >= h || (x	- rx) >= w || rx<1 || ry<1)
		return 0;
	int ix, iy;
	int a, b, c, d;
	int da, db, dc, dd;
	int na, nb, nc, nd;

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

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
				hline(x - b, y + c, b * 2, color);
				if (c)
					hline(x - b, y - c, b * 2, color);
				dc = c;
			}

			if ((d < dd) && (d > dc))
			{
				hline(x - a, y + d, a * 2, color);
				hline(x - a, y - d, a * 2, color);
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
				hline(x - d, y + a, d * 2, color);
				if (a)
					hline(x - d, y - a, d * 2, color);
				da = a;
			}

			if ((b < db) && (b > da))
			{
				hline(x - c, y + b, c * 2, color);
				hline(x - c, y - b, c * 2, color);
				db = b;
			}

		}
		while (b > a);
	}
	return 1;
}

/**
Draws a circular arc with centre x, y and radius r, in an anticlockwise direction
starting from the angle a and ending when it reaches b. These values are specified
in radian format, with M_PI*2 equal to a full circle.
Zero is to the right of the centre point, and larger values rotate anticlockwise from there.
@param x x coordinate of the arc centre
@param y y coordinate of the arc centre
@param ang1 starting angle
@param ang2 ending angle
@param r radius in pixels
@param color color
*/
int FGAPI FGDrawBuffer::arc(int x, int y, double ang1, double ang2, int r, unsigned color)
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
	int carry_qv=0;

	rr1 = r;
	rr2 = x;
	rr3 = y;

	/* evaluate the start point and the end point */
	px = (int) (rr2 + rr1 * cos(ang1));
	py = (int) (rr3 - rr1 * sin(ang1));
	ex = (int) (rr2 + rr1 * cos(ang2));
	ey = (int) (rr3 - rr1 * sin(ang2));

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

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
         carry_qv=1;
         qe+=4;
    } }

	while (!done)
	{
		putpixel(px, py, color);

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
					q++;
                    if (carry_qv) {carry_qv=0;qe-=4;}
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
					q++;
                    if (carry_qv) {carry_qv=0;qe-=4;}
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
					q++;
                    if (carry_qv) {carry_qv=0;qe-=4;}
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

					q = 1;
                    if (carry_qv) {carry_qv=0;qe-=4;}
					px1 = px;
					py1 = py - 1;
					px2 = px - 1;
					py2 = py - 1;
					px3 = px - 1;
					py3 = py;
				}
				break;

			default:
				return 1;
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
	putpixel(px, py, color);
	return 1;
}


static void CartesFromPolar(float azimut, float range, int& coord_x, int& coord_y)
{
		double _x,_y, pi=M_PI;

		double dAz=azimut*pi/180.0;        /* azimut v radianoch     */

		_x=(double)range*sin(dAz);
		_y=(double)range*cos(dAz);

		if(dAz > (pi/2.0) &&  dAz < (1.5*pi))
		{
				if(_y>0)  coord_y = int(-_y+0.5);
				else coord_y = int(_y+0.5);
		}
		else
		{
				if(_y>0)  coord_y = int(_y+0.5);
				else coord_y = int(-_y+0.5);
		}

		if(dAz > pi && dAz <= (2.0*pi))
		{
				if(_x>0)  coord_x = int(-_x+0.5);
				else coord_x = int(_x+0.5);
		}
		else
		{
				if(_x>0)  coord_x = int(_x+0.5);
				else coord_x = int(-_x+0.5);
		}
}


/**
Draws a circular arc with centre x, y and radius r, in an clockwise direction
starting from the angle a and ending when it reaches b. These values are specified
in degrees, with 360 equal to a full circle. The zero angle is at the 12 o'clock.
Zero is to the right of the centre point, and larger values rotate clockwise from there.
@param x x coordinate of the arc centre
@param y y coordinate of the arc centre
@param ang1 starting angle
@param ang2 ending angle
@param r radius in pixels
@param color color
*/
int FGAPI FGDrawBuffer::arc2(int x, int y, int ang1, int ang2, int r, unsigned color)
{
#define NORMALIZE_ANGLE(a) a = a%360
	int _x,_y;

	NORMALIZE_ANGLE(ang1);
	NORMALIZE_ANGLE(ang2);

	CartesFromPolar(ang1, r, _x, _y);

	moveto(x+_x, y+ -_y);
	do
	{
		ang1++;
		NORMALIZE_ANGLE(ang1);
		CartesFromPolar(ang1, r, _x, _y);
		lineto(x+_x, y+ -_y, color);
	} while (ang1 != ang2);

	return true;
}

#define swap(a,b)           {a^=b; b^=a; a^=b;}
#define	MIN(a,b)	( (a) < (b) ? (a) : (b) )
#define MAX(a,b)	( (a) > (b) ? (a) : (b) )

void FGDrawBuffer::__line(int a1, int b1, int a2, int b2, unsigned color)
{
	int xend, yend, dx, dy, c1, c2, step;
	register int p, x, y;

	dx = abs(a2 - a1);
	dy = abs(b2 - b1);

	if (dx > 32000)
		return;					// bulgar const

	if (dy > 32000)
		return;

	if (color == UNDEFINED_COLOR)
		color = state._ink;

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

		putpixel(x, y, color);
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
			putpixel(x, y, color);
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

		putpixel(x, y, color);

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
			putpixel(x, y, color);
		}
	}
}

/**
	Draw a line from position [x:y] to the position [a:b] with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::line(int x, int y, int a, int b, unsigned color)
{
	int cl=1;
	oldx = a;
	oldy = b;
	cl = ClipLine(x,y,a,b,0,0,w,h);
	if (cl)	__line(x,y,a,b,color); // null if not clipped
	return cl;
}

static FGMutex TextMutex;

/**
	Draw a text with the current object font, background 'p' and foreground 'i' from the position [x:y].

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::text(int x, int y, const char *txt, unsigned i, unsigned p)
{
	FGPixel *dst, *dst2;
	int alloc=0;

	TextMutex.Lock();

	x+=xoff;
	y+=yoff;

	int	width =	textwidth((char *)txt);
	int _x=x, _y=y,ww=GetFontW(), bufw=width+3;
	int height = GetFontH();

	if (clip(x,y,width,height)==0 || width<2)
	{
		TextMutex.Unlock();
		return 0;
	}

	if (i == UNDEFINED_COLOR)
	    i = state._ink;
	if (p == UNDEFINED_COLOR)
		p = state._paper;

	_x = x-_x;
	_y = y-_y;

	unsigned  bytes = (bufw)*height;

	if (bytes < FGTEXTBUFSZ)
		dst	= fgstate.fgimagebuffer;
	else
	{
		alloc = 1;
		dst = (FGPixel *) malloc(bytes*sizeof(FGPixel));
	}

	dst2 = dst;

	if (state._ppop == _GCOLORKEY)
	{
		p = fgstate.color_key;
	}
//	set_mmx();
	while (*txt)
	{
		variable_record *fontvar = FGFontManager::IsVariableFont(state._font);
		FGPixel *font = FGFontManager::GetFontImage(state._font);

		if (fontvar)
		{
			ww = fontvar->w[*(unsigned char *)txt];
		}
		CharOutClip(dst,
			fontvar ? fontvar->off[*(unsigned char *)txt++] + font:
				*(unsigned char *)txt++ * (GetFontW()*GetFontH()) + font,
			bufw,
			(ww+3)&~3,
			GetFontH(), i, p);
		dst	+= ww;
	}
//	reset_mmx();
	RamToRam(_x, _y, bufw, GetFontH(), x, y, w, h, width, height, dst2, image, BITBLIT_WITH_OPERATOR, state._ppop);
	if (alloc==1)
		free(dst2);

	TextMutex.Unlock();


	return 1;
}

/**
	Draw a patterned line from position [a1:b1] to the position [a2:b2].

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::pline(int a1, int b1, int a2, int b2, FGPattern *pat)
{
	int xend, yend, dx, dy, c1, c2, step;
	register int p, x, y;
	int cl;
	FGPixel iscolor;

	oldx = a2;
	oldy = a2;

	cl = ClipLine(a1,b1,a2,b2,0,0,w,h);

	if (cl==0) return 0;

	dx = abs(a2 - a1);
	dy = abs(b2 - b1);

	if (dx > 32000)
		return 0;			// bulgar const

	if (dy > 32000)
		return 0;

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
			putpixel(x, y, iscolor);
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
				putpixel(x, y, iscolor);
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
			putpixel(x, y, iscolor);

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
				putpixel(x, y, iscolor);
		}
	}
	return cl;
}

/**
	Draw a rectangle from position [x:y] with size [w:h] with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::rect(int x, int y, int w, int h, unsigned color)
{
	if (color == UNDEFINED_COLOR)
		color = state._ink;

	hline(x, y,	w,color);
	hline(x, y + h - 1,	w,color);
	vline(x, y,	h,color);
	vline(x	+ w	- 1, y,	h,color);
	return 1;
}

/**
	Fill a rectangle from position [x:y] with size [w:h] with 'color'.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::box(int x, int y, int ww, int hh, unsigned color)
{
	x += xoff;
	y += yoff;

	if (clip(x, y, ww, hh) == 0) return 0;

	if (color == UNDEFINED_COLOR)
	    color = state._ink;
	L1Box(CalcAddr(x,y), ww, hh, color, w, state._ppop);
	return 1;
}

/**
	Fill convex polygon with 'n' vertices. The each vertice is
	a pair of two integers X and Y. Parameter 'pt' is an array
	of these pairs. FUnction return the Rectangle of the polygon
	boundary.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
FGRect FGAPI FGDrawBuffer::fpolygon(const FGPointArray& vertices, unsigned color)
{
	FGRect rect;
	edge *edges=edge_array, *ep;
	scan *scans=scan_array, *sp, *points, *segments;
	int	xmin, xmax,	ymin, ymax;
	int	ypos, nedges, mem = 0;

	int n = vertices.vertices;
	FGPoint* pt = vertices.array;
	if (color == UNDEFINED_COLOR)
	    color = state._ink;

	if ((n > 1)	&&
	  (pt[0].x == pt[n - 1].x) && (pt[0].y == pt[n - 1].y))
	{
		n--;
	}
	if (n <	1)
	{
		return rect;
	}
	if (n>128)
	{
		edges =	(edge *) malloc(sizeof(edge) * (n +	2));
		scans =	(scan *) malloc(sizeof(scan) * (n +	8));
		mem = 1;
	}
	if (edges && scans)
	{
		int	prevx =	xmin = xmax	= pt[0].x;
		int	prevy =	ymin = ymax	= pt[0].y;

		nedges = 0;
		ep = edges;
		while (--n >= 0)
		{
			if (pt[n].y >=	prevy)
			{
				ep->e.x	= prevx;
				ep->e.y	= prevy;
				ep->e.xlast	= prevx	= pt[n].x;
				ep->e.ylast	= prevy	= pt[n].y;
			}
			else
			{
				ep->e.xlast	= prevx;
				ep->e.ylast	= prevy;
				ep->e.x	= prevx	= pt[n].x;
				ep->e.y	= prevy	= pt[n].y;
			}
			if ((ep->e.y > (hwrk+yoff)) || (ep->e.ylast <	(0)))
				continue;
			{
				if (ep->e.y	< 0)
				{
					ep->e.x	+= (((int) (((long)	(((int)	((ep->e.xlast -	ep->e.x)) << 1)) * (long) ((0	- ep->e.y))) / (long) ((ep->e.ylast	- ep->e.y))) + (((int) ((ep->e.xlast - ep->e.x)) ^ (int) ((0 - ep->e.y)) ^ (int) ((ep->e.ylast - ep->e.y))) >> ((sizeof(int) * 8)	- 1)) +	1) >> 1);
					ep->e.y	= 0;;
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
					(&ep->e)->xstep	= (-1);
					(&ep->e)->dx = (-(&ep->e)->dx);
				}
				else
				{
					(&ep->e)->xstep	= 1;
				}
				if ((&ep->e)->dx > (&ep->e)->dy)
				{
					(&ep->e)->xmajor = 1;
					(&ep->e)->error	= (&ep->e)->dx >> 1;
				}
				else
				{
					(&ep->e)->xmajor = 0;
					(&ep->e)->error	= ((&ep->e)->dy	- ((1 -	(&ep->e)->xstep) >>	1))	>> 1;
				}
			};
			ep->status = inactive;
			nedges++;
			ep++;
		}
		rect.x = xmin;
		rect.y = ymin;
		rect.w = xmax-xmin;
		rect.h = ymax-ymin;
		if ((nedges	> 0) &&	(xmin <= wwrk+xoff)	&& (xmax >=	0))
		{
			if (xmin < 0)
				xmin = 0;
			if (ymin < 0)
				ymin = 0;
			if (xmax > (wwrk+xoff))
				xmax = (wwrk+xoff);
			if (ymax > (hwrk+yoff))
				ymax = (hwrk+yoff);

			for	(ypos =	ymin; ypos <= ymax;	ypos++)
			{
				sp = scans;
				points = 0;
				segments = 0;
				for	(n = nedges, ep	= edges; --n >=	0; ep++)
				{
					switch (ep->status)
					{
						case inactive:
							if (ep->e.y	!= ypos)
								break;
							if (ep->e.dy ==	0)
							{
								ep->status = passed;
								xmin = ep->e.x;
								xmax = ep->e.xlast;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int	_swap_tmpval_ =	(xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
								{
									scan *prev = 0;
									scan *work = segments;
									int	overlap	= 0;

									while (work	!= 0)
									{
										if ((work->x1 <= xmax) && (xmin	<= work->x2))
										{
											overlap	= 1;
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
							xmin = xmax	= ep->e.x;
							if (ep->e.ylast	== ypos)
							{
								ep->status = passed;
								xmax = ep->e.xlast;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int	_swap_tmpval_ =	(xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
								{
									scan *prev = 0;
									scan *work = points;

									while (work	!= 0)
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
							else if	(ep->e.xmajor)
							{
								for	(;;)
								{
									(&ep->e)->x	+= (&ep->e)->xstep;
									if (((&ep->e)->error -=	(&ep->e)->dy) <	0)
									{
										(&ep->e)->error	+= (&ep->e)->dx;
										break;
									}
								};
								xmax = ep->e.x - ep->e.xstep;
								{
									if ((int) (xmin) > (int) (xmax))
									{
										int	_swap_tmpval_ =	(xmin);

										(xmin) = (xmax);
										(xmax) = _swap_tmpval_;
									}
								};
							}
							else
							{
								{
									if (((&ep->e)->error -=	(&ep->e)->dx) <	0)
									{
										(&ep->e)->x	+= (&ep->e)->xstep;
										(&ep->e)->error	+= (&ep->e)->dy;
									}
								};
							}
							{
								scan *prev = 0;
								scan *work = points;

								while (work	!= 0)
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
						int	overlap	= 0;

						while (work	!= 0)
						{
							if ((work->x1 <= xmax) && (xmin	<= work->x2))
							{
								overlap	= 1;
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
				while (segments	!= 0)
				{
					xmin = segments->x1;
					xmax = segments->x2;
					segments = segments->next;
					{
						if (xmin > wwrk+xoff)
						{
							continue;
						}
						if (xmax < 0)
						{
							continue;
						}
						if (xmin < 0)
						{
							xmin = 0;;
						}
						if (xmax > wwrk+xoff)
						{
							xmax = wwrk+xoff;
						}
					}
					hline(xmin, ypos, xmax - xmin +	1, color);
				}
			}
		}
	}
	if (mem)
	{
		free(edges);
		free(scans);
	}
	return rect;
}

/**
	Draw convex polygon with 'n' vertices. The each vertice is
	a pair of two integers X and Y. Parameter 'pt' is an array
	of these pairs. The used color is current object's foreground color.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/

/**
	Draw convex polygon with 'n' vertices. The each vertice is
	a pair of two integers X and Y. Parameter 'pt' is an array
	of these pairs.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
void FGAPI FGDrawBuffer::polygon(const FGPointArray& vertices, unsigned color)
{
	int doClose=0;
	int n = vertices.vertices;
	FGPoint* pt = vertices.array;

	if (color == UNDEFINED_COLOR)
		color = state._ink;

	if (n <= 1)	return;
	if (n == 2) doClose	= 1;

	moveto(pt[0].x, pt[0].y);

	for	(int i = 1;	i <	n; i++)
	{
		lineto(pt[i].x, pt[i].y, color);
	}

	if (doClose || (pt[0].x!=pt[n-1].x && pt[0].y != pt[n-1].y))
		lineto(pt[0].x, pt[0].y, color);
}

/**
	Draws a bezier spline using the four control points specified in the points array.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
void FGAPI FGDrawBuffer::spline(FGPoint points[4], unsigned color)
{
	#define NPTS   64

	FGPoint pts[NPTS];

	if (color == UNDEFINED_COLOR)
	    color = state._ink;

	calc_spline(points, NPTS, pts);
	moveto(pts[0].x, pts[0].y);

	for (int i=1; i<NPTS; i++)
	{
		lineto(pts[i].x, pts[i].y, color);
	}
}

/**
	Draw a line from the latest PEN position to the position [x:y]

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGAPI FGDrawBuffer::lineto(int x, int y, unsigned color)
{
	int rc = line(oldx, oldy, x, y, color);
	if (color == UNDEFINED_COLOR)
		color = state._ink;
	if (state._ppop == _GXOR)
		putpixel(x, y, color);
	return rc;
}

/**
	Draw a text via standard "C" behaviour with the current object font & colors from the position [x:y].

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function return false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
*/
int FGDrawBuffer::printf(int xx, int yy, const char *format,...)
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
	int len = strlen(s);
	if (s[len-1] == '\n')
	{
		s[--len] = 0;
	}
	text(xx,yy,s);
	return len;
}

void FGAPI FGDrawBuffer::Resize(int dx, int dy)
{
	// test to new new size
	if (w + dx<1 || h + dy<1)
		return;
	if (dx==0 && dy==0 && image)
		return;
	lock();
	if (image) free(image);
	w += dx;
	h += dy;
	if (w*h)
		assert((image = (FGPixel *) malloc(areasize(w, h)))!=0);
	wwrk += dx;
	hwrk += dy;
	unlock();
}

/**
	Change the name of the object.
*/
void FGAPI FGDrawBuffer::SetName(const char *s)
{
	lock();
	if (name != empty_string) delete [] name;
	name = new char[strlen(s)+1];
	strcpy(name,s);
	unlock();
}

/**
 * Resize the image with ratio 'rx' and 'ry'.
 */
void FGAPI FGDrawBuffer::stretch(double rx, double ry)
{
	stretch((int)(w*rx), (int)(h*ry));
}

/**
 * Resize the image to the new size.
 */
void FGAPI FGDrawBuffer::stretch(int _w, int _h)
{
#ifndef INDEX_COLORS
		int xpos = 0, ypos = 0;
		unsigned pix;
		float r, g, b;
		int r1, g1, b1;
		int r2, g2, b2;
		int r3, g3, b3;
		int r4, g4, b4;
		int r0, g0, b0;
		float ratio_x;
		float ratio_y;
		float x1, y1;
		float xerr, yerr;
		float xfloat, yfloat;

		if (_w == w && _h == h)
			return;
		if (_w <1 || _h < 1)
			return;

		lock();

		FGPixel *dest = (FGPixel *)malloc(areasize(_w, _h)), *dest_pom=dest;

		ratio_x = (float)w / (float)_w;
		ratio_y = (float)h / (float)_h;

		for (ypos = 0; ypos < _h; ypos++)
				for (xpos = 0; xpos <  _w; xpos++) {
						xfloat = (xpos) * ratio_x;
						yfloat = (ypos) * ratio_y;

						x1 = (int)xfloat;
						y1 = (int)yfloat;

						xerr = 1.0 - (xfloat - (float)x1);
						yerr = 1.0 - (yfloat - (float)y1);
#define _getpixel(x,y,r,g,b) { pix = getpixel((int)x, (int)y); r=kRedComponent(pix); g=kGreenComponent(pix); b=kBlueComponent(pix); }
						_getpixel(x1, y1, r1, g1, b1);
						_getpixel(x1 + 1, y1, r2, g2, b2);
						_getpixel(x1 + 1, y1 + 1, r3, g3, b3);
						_getpixel(x1, y1 + 1, r4, g4, b4);
						r = (float)r1 * xerr +
							(float)r2 * (1.0 - xerr) +
							(float)r3 * (1.0 - xerr) +
							(float)r4 * xerr;

						r += (float)r1 * yerr +
							 (float)r2 * yerr +
							 (float)r3 * (1.0 - yerr) +
							 (float)r4 * (1.0 - yerr);

						r0 = int(r * 0.25);

						g = (float)g1 * xerr +
							(float)g2 * (1.0 - xerr) +
							(float)g3 * (1.0 - xerr) +
							(float)g4 * xerr;

						g += (float)g1 * yerr +
							 (float)g2 * yerr +
							 (float)g3 * (1.0 - yerr) +
							 (float)g4 * (1.0 - yerr);

						g0 = int(g * 0.25);

						b = (float)b1 * xerr +
							(float)b2 * (1.0 - xerr) +
							(float)b3 * (1.0 - xerr) +
							(float)b4 * xerr;

						b += (float)b1 * yerr +
							 (float)b2 * yerr +
							 (float)b3 * (1.0 - yerr) +
							 (float)b4 * (1.0 - yerr);

						b0 = int(b * 0.25);

#ifndef BPP32
							*dest_pom++ = (((r0<<8)&0xf800) | ((g0<<3)&0x7e0) | ((b0>>3)&0x1f));
#else
							*dest_pom++ = ((r0<<16) | (g0<<8) | b0);
#endif
		}
	unlock();
	Resize(_w-w, _h-h);
	lock();
	free(image);
	image = dest;
	unlock();
#endif
}

/**
	Fill a triangle with these three vertices.

	Coordinate [0:0] is top-left corner of the screen. INK is color
	index in 8-bit videomodes, or absolute color in 16/24 bit modes.
	Raster Operaration is the current value of ENUM_PPOP. The default
	is _GSET (i.e. simply copy) See set_ppop() for more details.
	Coordinate are clipped if needed before drawing.
	Function returns false if no drawing (i.e. object has been
	clipped-out completelly). The object XY-offset is added
	before drawing.
	@param x1 x position of first vertice
	@param y1 y position of first vertice
	@param x2 x position of second vertice
	@param y2 y position of second vertice
	@param x3 x position of third vertice
	@param y3 y position of third vertice
	@param color foreground color
*/
FGRect FGAPI FGDrawBuffer::ftriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned color)
{
	FGPointArray points(3, &x1);
	return fpolygon(points, color);
}

void FGAPI FGDrawBuffer::turn_bitmap(int cnt, FGPixel *from, FGPixel *to)
{
	FGPixel *p = new FGPixel[w];

	for (int i = 0; i < cnt; i++)
	{
		memmove(p, to - (i * w), w*sizeof(FGPixel));
		memmove(to - (i * w), from + (i * w), w*sizeof(FGPixel));
		memmove(from + (i * w), p, w*sizeof(FGPixel));
	}
	delete p;
}

void FGAPI FGDrawBuffer::rotate(ENUM_ROTATE_DIRECTION direction)
{
	FGPixel *src, *dst, *dst2, pom;
	int foo;

	lock();
	switch(direction)
	{
		case ROTATE_90_CW:
			dst = dst2 = (FGPixel *) malloc(areasize(w,h));
			for(int j=0;j<w;j++)
			{
				for(int i=h-1;i>=0;i--)
				{
					*dst++ = *CalcAddr(j,i);
				}
			}
swapsize:
			foo = w;
			w   = h;
			h   = foo;
			foo = xoff;
			xoff= yoff;
			yoff= foo;
			foo = wwrk;
			wwrk= hwrk;
			hwrk= foo;
			free(image);
			image = dst2;
			break;
		case ROTATE_90_CCW:
			dst = dst2 = (FGPixel *) malloc(areasize(w,h));
			for(int j=w-1;j>=0;j--)
			{
				for(int i=0;i<h;i++)
				{
					*dst++ = *CalcAddr(j,i);
				}
			}
			goto swapsize;
		case ROTATE_180:
			src = CalcAddr(0, 0);
			dst = CalcAddr(w-1, h-1);
			for(int j=(w*h)/2;j>=0;j--)
			{
				pom    = *src;
				*src++ = *dst;
				*dst-- = pom;
			}
			break;
		case ROTATE_HORIZONTAL:
			for(int i=0;i<h;i++)
			{
				src = CalcAddr(0,i);
				dst = src + w-1;
				for(int j=0;j<w/2;j++)
				{
					pom = *src;
					*src++ = *dst;
					*dst-- = pom;
				}
			}
			break;
		case ROTATE_VERTICAL:
			turn_bitmap(h / 2, image, image + w * (h - 1));
			break;
	}
	unlock();
}

void FGAPI FGDrawBuffer::clear(int color)
{
	FGmemset(image, color, w*h);
}

#ifdef FG_NAMESPACE
}
#endif







