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
*/

#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __linux__
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>
#elif _MSC_VER
#include <direct.h>
#endif

#ifdef __BORLANDC__
#include <dir.h>
#endif

#include "fastgl.h"
#include "listbox.h"

#include "_fastgl.h"
#include "drivers.h"

#ifdef FG_TTF
#include "fgttf.h"
#endif

//
// Global definitions & declaration
//

#ifdef FG_NAMESPACE
namespace fgl {
#endif


WindowDatabase *entryPoint;
FGMutex FGWindow::WindowListMutex;

FGMenuWindow *FGMenuWindow::CurrentMenu=0;
GuiHwnd FGMenuWindow::Proc=0;

static FGWindow *_Note_Ptr, *close_adept;

static unsigned int CRCTab[256];
/*
static int alty[26] = {
ALT_A,ALT_B,ALT_C,ALT_D,ALT_E,ALT_F,ALT_G,ALT_H,
ALT_I,ALT_J,ALT_K,ALT_L,ALT_M,ALT_N,ALT_O,ALT_P,
ALT_Q,ALT_R,ALT_S,ALT_T,ALT_U,ALT_V,ALT_W,ALT_X,
ALT_Y,ALT_Z };
*/
static void InitCRC(void);

// -----------------------------------------------------------------------------

static char *skipstr(char *s)
{
	while (*s)
	{
		if (*s > ' ')
			return s;
		s++;
	}
	return s;
}

/**
	Internal implementation of Icon object
	@internal
*/
class IconWindow : public FGWindow
{
		FGWindow *original;
		static void Proc(FGEvent *p)
		{
			IconWindow *pp = (IconWindow *)p->wnd;
			if (p->GetType()==CLICKRIGHTEVENT)
				pp->original->WindowIconize();
			if (p->GetType()==KEYEVENT && p->GetKey()==ALT_F02)
				pp->original->WindowIconize();
			if (p->GetType()==WINDOWMOVEEVENT)
			{
				if (entryPoint) entryPoint->DatabaseSetIcon(p->GetX(), p->GetY(), pp->original->GetName());
				pp->original->SetIconPosition(p->GetX(), p->GetY());
			}
		}
	public:
		IconWindow(FGWindow *parent, int xs, int ys, FGDrawBuffer *ico)
			: FGWindow(0, xs, ys, ico?ico->GetW()+4:48, ico?ico->GetH()+4+10:48, "Icon", Proc, 0, CWHITE, WNOPICTO|WNODRAWFROMCONSTRUCTOR|WICONWINDOWTYPE)
		{
			icon = ico;
			if (icon==0)
				icon = new FGDrawBuffer(44, 44, BMP_MEM, CGRAYED);
			original = parent;
			original->WindowAttachIcon(icon);
			WindowBox(0,0,w,h,CBLUELIGHT);
			WindowBox(2,10,w-4,2,CWHITE);
			WindowSetWorkRect(2,2,w-4,h-4);
			set_ppop(_GSET);
			set_font(0);
			draw();
			WindowText(0,0,parent->GetName(), CWHITE, CBLUELIGHT);
		}
		virtual void draw(void)
		{
			if (icon)
			{
				WindowPutBitmap(0,10,0,0,w,h,icon);
			}
		}
};

// -----------------------------------------------------------------------------

void RootResize(int ww, int hh)
{
	FGWindow::WindowVirtualRoot(0,0,ww,hh);
}

//
// Internal
// kresli ikonky na titulok okna
void FGWindow::WindowPiktograms(int x, int y)
{
	int action= status&WMINIMIZE?1:0;
	action |= status&WNOPICTO?0:2;
	switch(action)
	{
		case 0:	// nothing
			break;
		case 3:	// all (minimize+close)
			RamToRam(0, 0, PICTOSIZE*2, PICTOSIZE, x, y, w,h, PICTOSIZE*2, PICTOSIZE, _internal_bitmap0, image, BITBLIT_WITH_CLIPPING, _GSET);
			break;
		case 1:	// (minimize)
			RamToRam(0, 0, PICTOSIZE*2, PICTOSIZE, x+PICTOSIZE, y, w,h, PICTOSIZE, PICTOSIZE, _internal_bitmap0, image, BITBLIT_WITH_CLIPPING, _GSET);
			break;
		case 2:	// (close)
			RamToRam(PICTOSIZE, 0, PICTOSIZE*2, PICTOSIZE, x+PICTOSIZE, y, w,h, PICTOSIZE, PICTOSIZE, _internal_bitmap0, image, BITBLIT_WITH_CLIPPING, _GSET);
			break;
	}
}

//! A traditional printf in window form
/**
Writes formatted output to the window at current position.
When the whole FGWindow is printed out, image automatically scroll-up.

The printf function:
	-	Accepts a series of arguments
	-	Applies to each argument a format specifier contained in the format string *format
	-	Outputs the formatted data (to the screen, a stream, stdout, or a string)

There must be enough arguments for the format. If there are not, the results will
be unpredictable and likely disastrous.  Excess arguments (more than required
by the format) are merely ignored.

The format string, present in each of the printf function calls, controls how each
function will convert, format, and print its arguments.
@note There must be enough arguments for the format; if not, the results will
be unpredictable and possibly disastrous. Excess arguments (more than required
by the format) are ignored.

The format string is a character string that contains two types of objects:
	-	Plain characters are copied verbatim to the output stream.
	-	Conversion specifications fetch arguments from the argument list and apply formatting to them.

Plain characters are simply copied verbatim to the output stream.
Conversion specifications fetch arguments from the argument list and apply formatting to them.

see 'man printf' for more details

@return On success, printf returns the number of bytes output. On error, printf returns EOF.
@param format formatting string as detailed above
*/
int FGWindow::printf(const char *format, ...)
{
	char s[256]="";
	va_list arglist;

	va_start(arglist, format);
#ifndef _MSC_VER
	vsnprintf(s, sizeof(s)-1, format, arglist);
#else
	vsprintf(s, format, arglist);
#endif
	va_end(arglist);
	if (*s) puts(s);
	return strlen(s);
}

/**
	A traditional printf in window form + position
	As printf() but at the exact position.
*/
int FGWindow::printf(int xx, int yy, const char *format, ...)
{
	char s[256];
	s[0] = 0;

	move(xx, yy);
	va_list arglist;

	va_start(arglist, format);
#ifndef _MSC_VER
	vsnprintf(s, sizeof(s)-1, format, arglist);
#else
	vsprintf(s, format, arglist);
#endif
	va_end(arglist);
	int len = strlen(s);
	if (len)
	{
		if (s[len-1] == '\n')
		{
			s[--len] = 0;
		}
		WindowText(xx,yy,s);
	}
	return len;
}

//! Put char to window at current print position
void FGAPI FGWindow::wputc(int c)
{
	static FGMutex TextOutput(recursiveMutEx);

	TextOutput.Lock();

	if (c == -1)
	{
		WindowRepaintUser(0, ypos, xpos, GetFontH());
		TextOutput.Unlock();
		return;
	}

	if (xpos == -1)
	{
		WindowScrollUp(0, GetFontH(), wwrk, hwrk - GetFontH() - hwrk % GetFontH(), GetFontH());
		xpos = 1;
	}

	if (xpos < (wwrk - GetFontW()))
	{
		if (c == '\n')
		{
			if (ypos > (hwrk - GetFontH() * 2))
			{
				WindowRepaintUser(0, ypos, xpos, GetFontH());
				xpos = -1;
			}
			else
			{
				WindowRepaintUser(0, ypos, xpos, GetFontH());
				ypos += GetFontH();
				xpos = 1;
			}
		}
		else
		{
			text(xpos, ypos, (char *) &c);
			xpos += GetFontW();
		}
	}
	else
	{
		xpos -= GetFontW();
		wputc('\n');
		wputc(c);
	}
	TextOutput.Unlock();
}

/**
 Internal.
 * return 0 if in window area
 * else 1 for title
 * 2 for close
 * 3 for minimize
 * 4 for resize
 */
int FGAPI FGWindow::TitleFind(FGEvent * e)
{
	if (!IsVisible()) return 0;

	if ((x <= e->GetX()) && ((x + w) >= e->GetX()))
	{
		if (((x + w - 13) <= e->GetX()) &&
			((y + h - 13 <= e->GetY()) && ((y + h) >= e->GetY())) && status & WSIZEABLE)
				return 4;
		if (status & WTITLED)
		{
			if ((y <= e->GetY()) && ((y + 3+TITLEH) >= e->GetY()))
			{
				if (!(status & WNOPICTO)) // close or close&minimize
				{
					if (e->GetX() >= (x + w - 3 - PICTOSIZE))
						return 2;
					if (e->GetX() >= (x + w - 3 - PICTOSIZE*2))
						if (status&WMINIMIZE) return 3;
						else return 0;
				}
				else if (status & WMINIMIZE)
				{
					if (e->GetX() >= (x + w - 3 - PICTOSIZE))
						return 3;
				}
				return 1;
			}
		}
		else if (status & WUNMOVED) return 0;
		else if ((y <= e->GetY()) && ((y + h) >= e->GetY()) && status&WICONWINDOWTYPE)
			return 1;
		else if ((y <= e->GetY()) && ((y + TITLEH+3) >= e->GetY()))
			return 1;
	}
	return 0;
}

//! redraw block unconditionally
void FGAPI FGWindow::WindowUpdateBlock(int x, int y, int w, int h)
{
	clip_shape(x,y,w,h);
	RepaintBlock(x,y,w,h);
}

/**
	redraw window intersection with original background
	dx,dy are deltas from original
*/
void FGAPI FGWindow::WindowUpdate(int dx, int dy, int all)
{
	if (all || abs(dx) > w || abs(dy) > h)
	{
		FGApp::intersect(this,x,y,w,h);
		return;
	}
	if (dx>0) // right side
	{
		FGApp::intersect(this,x,y,dx,h);
	}
	if (dy>0) // up side
	{
		FGApp::intersect(this,x,y,w,dy);
	}

	if (dx<0) // left side
	{
		FGApp::intersect(this,x+w+dx,y,-dx,h);
	}
	if (dy<0) // down side
	{
		FGApp::intersect(this,x,y+h+dy,w,-dy);
	}
}

#define area(a,b,c,d) *out++=a,*out++=b,*out++=c,*out++=d; //, debug==1?(Ladenie("a,b,c,d - %d,%d,%d,%d\n",a,b,c,d)):0;

int FGAPI FGWindow::Rozporcuj(int X1, int Y1, int X2, int Y2, int *in, int *out)
{
	int x1, y1, x2, y2;

	if (*in == -1) return 0;

	while (*in != -1)
	{
		x1 = *in++;
		y1 = *in++;
		x2 = *in++;
		y2 = *in++;

		if (x1 >= X1 && y1 >= Y1 && x2 <= X2 && y2 <= Y2)
			continue;				// cely prekryty

		if (x2 < X1 || y2 < Y1 || x1 > X2 || y1 > Y2)	// cely mimo
		{
			area(x1, y1, x2, y2);
			continue;
		}

		if (x1 <= X1 && x2 >= X1)
		{
			area(x1, y1, X1, y2);
			x1 = X1;
		}

		if (y1 <= Y1 && y2 >= Y1)
		{
			area(x1, y1, x2, Y1);
			y1 = Y1;
		}

		if (x2 >= X2 && x1 <= X2)
		{
			area(X2, y1, x2, y2);
		}

		if (y2 >= Y2 && y1 <= Y2)
		{
			area(x1, Y2, x2, y2);
		}
	}
	*out = -1;
	return 1;
}

//! actualize window at relative coors in it
void FGAPI FGWindow::WindowRepaint(int xr, int yr, int w, int h)
{
	int zoznam1[4096], zoznam2[4096], *p1, *p2, *p3;
	FGWindow *Colise;
	FGWindowRIterator This = FGApp::GetRIterator(this); // at the end
	FGWindowRIterator posledne = FGApp::Windows.rbegin(); // at the end

	WindowListMutex.Lock();

	if (status & WLOCKED)
	{
		status |= WDIRTY;
		goto exi;
	}

	// clipping
	if (clip_shape(xr,yr,w,h) == 0) goto exi;


	// test overlap
	Colise = FGApp::OdkryteOkno(This, posledne, x + xr, y + yr, w, h);

	switch (long(Colise))
	{
		case -1:
			status |= WDIRTY;
			goto exi;
		case 0:
			RepaintBlock(xr, yr, w, h);
			goto exi;
	}
	// chyba je v okne Colise a mozno aj v oknach medzi nimy
	posledne = FGApp::GetRIterator(Colise);
	p1 = zoznam1;
	p1[0] = x + xr;
	p1[2] = p1[0] + w;
	p1[1] = y + yr;
	p1[3] = p1[1] + h;
	p1[4] = -1;
	p2 = zoznam2;		// inicializacia poli
	p2[0] = -1;

	status |= WDIRTY;
	for (;;)
	{
		FGWindow* _posledne = *posledne;
		if (_posledne->IsVisible())
		{
			if (!Rozporcuj(_posledne->GetX(), _posledne->GetY(), _posledne->GetX() + _posledne->GetW(), _posledne->GetY() + _posledne->GetH(), p1, p2))
				goto exi; // ???
			p3 = p1;
			p1 = p2;
			p2 = p3;
		}
		posledne++;	// fixme:  was --,
		if ( posledne == This) // finito?
			break;
		if (long(FGApp::OdkryteOkno(This, posledne, x + xr, y + yr, w, h)) == -1)
			break;
	}
	while (*p1 != -1)
	{
		RepaintBlock(p1[0] - x, p1[1] - y, p1[2] - p1[0], p1[3] - p1[1]);
		p1 += 4;
	}
exi:
	WindowListMutex.Unlock();
}

//! redraws rectangle area in a user space of the FGWindow
void FGAPI FGWindow::WindowRepaintUser(int xr, int yr, int wr, int hr)
{
	xr += xoff;
	yr += yoff;
	if (clip(xr,yr,wr,hr))
	{
		WindowRepaint(xr,yr,wr,hr);
	}
}

//! internal
long FGAPI FGWindow::WindowStatus(long stat)
{
	int tmp;
	tmp = status;

	switch (stat)
	{
		case WHIDE:
			if (!IsVisible())
				break;
			status |= WHIDEN;
			WindowUpdate(0,0,1); // zmaze ho cele podkladom
			break;

		case WVISIBLE:
			if (IsVisible())
				break;
			status &= (~WHIDEN);
			if (!iconized)
			{
				RepaintBlock(0, 0, w, h);
				SetStatus(GetStatus() & ~(WDIRTY));
			}
			break;

		case WDEACTIVE:
			WindowDrawTitle(CTITLE);
			break;

		case WACTIVE:
			WindowDrawTitle(CTITLEACTIVE);
			break;
	}
	return tmp;
}

void  FGAPI FGWindow::SetName(const char *s)
{
	FGDrawBuffer::SetName(s);

	if ( this == FGApp::GetCurrentWindow() )
		WindowStatus(WACTIVE);
	else
		WindowStatus(WDEACTIVE);
}

//! switches the FGWindow to the front - it get focus.
void FGAPI FGWindow::WindowFocus(void)
{
	FGWindow *old = FGApp::GetCurrentWindow(), *novy = this;
	FGEvent a1(LOSTFOCUSEVENT), a2(GETFOCUSEVENT);

	if (iconized && iconized!=(FGWindow *)-1)
		novy = iconized;
	old->SendToWindow(&a1);

	// reload because change of current is possible
	old = FGApp::GetCurrentWindow();

	if (FGApp::NumberOfWindow() <= 1) // no more window
		return;
	if (novy == old) // no more to switch
		return;

	old->WindowStatus(WDEACTIVE);	// deactive old

	FGApp::SetCurrentWindow(novy);

	if ((novy->status & (WTOP|WBACK)) == 0)
	{
		FGApp::RemoveIterator(novy);	// remove Window from the list
		FGApp::AddWindowToList(novy, novy->status);       // add Window to the list
	}

	novy->WindowStatus(WACTIVE); // active new
	// fixme - podmienene (pre top_back)
	novy->WindowRepaint(0,0,w,h); // refresh all
	SendToWindow(&a2);
}

/**
	Print a text string to the status bar.
	@param x x position
	@param s null terminated string
	@param c foreground color
*/
void FGAPI FGWindow::WindowStatusBar(int x, const char *s, int c)
{
	int y;
	x += xoff+1;
	y  = h-STATUSBARH+4-(status&WFRAMED?BASEBORDER:0);
	SAVE_CLIPPING(this)
// hack!
	wwrk-=4;
	WindowText(x, y, s, c, statuscolor);
	RESTORE_CLIPPING(this)
}

/**
	Draws a filled triangle to the FGWindow.
	@param x1 x position of first vertice
	@param y1 y position of first vertice
	@param x2 x position of second vertice
	@param y2 y position of second vertice
	@param x3 x position of third vertice
	@param y3 y position of third vertice
	@param col foreground color
	@return the rectangle of triangle
	@see set_ppop()
*/
FGRect FGAPI FGWindow::WindowFillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, unsigned col)
{
	FGRect r;
	r = ftriangle(x1,y1,x2,y2,x3,y3, col);
	if (r.w) WindowRepaintUser(r.x, r.y, r.w, r.h);
	return r;
}

/**
	Draws a filled convex polygon in the FGWindow.
	@param vertices FGPointArray object contained vertices of polygon
	@param col foreground color
	@return the rectangle of polygon
	@see set_ppop() FGPointArray
*/
FGRect FGAPI FGWindow::WindowFillPolygon(const FGPointArray& vertices, unsigned col)
{
	StateLock();
	FGRect r;
	r = fpolygon(vertices, col);
	if (r.w) WindowRepaintUser(r.x, r.y, r.w, r.h);
	StateUnlock();
	return r;
}

/**
	Draws a convex polygon in the FGWindow.
	@param vertices FGPointArray object contained vertices of polygon
	@param col foreground color
	@return the rectangle of polygon
	@see set_ppop() FGPointArray
*/
void FGAPI FGWindow::WindowDrawPolygon(const FGPointArray& vertices, unsigned col)
{
	StateLock();
	polygon(vertices, col);
	WindowRepaintUser(0,0,wwrk,hwrk);
	StateUnlock();
}

/**
Draws a bezier spline using the four control points specified in the points array.
The bezier curve is specified by the four x/y
control points in the points array: points[0] and points[1] contain
the coordinates of the first control point, points[2] and points[3]
are the second point, etc. FGControl points 0 and 3 are the ends of the spline,
and points 1 and 2 are guides. The curve probably won't pass through points 1 and 2,
but they affect the shape of the curve between points 0 and 3 (the lines p0-p1 and p2-p3
are tangents to the spline). The easiest way to think of it is that the curve starts at p0,
heading in the direction of p1, but curves round so that it arrives at p3
from the direction of p2. In addition to their role as graphics primitives,
spline curves can be useful for constructing smooth paths around a series of control points.
*/
void FGAPI FGWindow::WindowSpline(FGPoint points[4], unsigned color)
{
	#define NPTS   64

	FGPoint pts[NPTS];

	calc_spline(points, NPTS, pts);

	for (int i=1; i<NPTS; i++)
	{
		WindowLine(pts[i-1].x, pts[i-1].y, pts[i].x, pts[i].y, color);
		if (state._ppop == _GXOR)
			WindowPixel(pts[i].x, pts[i].y, color);
	}
}

/**
	Draws the text to the FGWindow. The output will
	takes font and rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param s standard C string taht will be printed-out
	@param Ink foreground color
	@param Paper background color
	@see set_ppop()
	@see set_font()
*/
void FGAPI FGWindow::WindowText(int x, int y, const char *s, unsigned Ink, unsigned Paper)
{
	StateLock();
	text(x, y, s, Ink, Paper);
	WindowRepaintUser(x, y, textwidth(s), GetFontH());
	StateUnlock();
}

/**
	Draws the filled box into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param a width
	@param b height
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowBox(int x, int y, int a, int b, unsigned color)
{
	StateLock();
	if (box(x, y, a, b, color))
	{
		WindowRepaintUser(x, y, a, b);
	}
	StateUnlock();
}

/**
	Draws the rectangle into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param a width
	@param b height
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowRect(int x, int y, int a, int b, unsigned color)
{
	WindowLine(x, y, x+a-1, y,color);
	WindowLine(x+a-1, y+1, x+a-1, y+b-1,color);
	WindowLine(x+a-2, y+b-1, x, y+b-1,color);
	WindowLine(x, y+b-2, x, y+1,color);
}

/**
	Draws the patterned rectangle into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param a width
	@param b height
	@param pat the Pattern object
	@see set_ppop()
	@see Pattern
*/
void FGAPI FGWindow::WindowPatternRect(int x, int y, int a, int b, FGPattern *pat)
{
	int tmp = pat->reset;
	pat->reset=1;
	WindowPatternLine(x, y, x+a-1, y, pat);
	pat->reset=0;
	WindowPatternLine(x+a-1, y+1, x+a-1, y+b-1, pat);
	WindowPatternLine(x+a-2, y+b-1, x, y+b-1, pat);
	WindowPatternLine(x, y+b-2, x, y+1, pat);
	pat->reset = tmp;
}

/**
	Draws the line into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x start x position
	@param y start y position
	@param a end x position
	@param b end y position
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowLine(int x, int y, int a, int b, unsigned color)
{
	int x1, x2, y1, y2;
	StateLock();

	if (x < a)
	{
		x1 = x;
		x2 = a;
	}
	else
	{
		x1 = a;
		x2 = x;
	}
	if (y < b)
	{
		y1 = y;
		y2 = b;
	}
	else
	{
		y1 = b;
		y2 = y;
	}

	x += xoff;
	y += yoff;
	a += xoff;
	b += yoff;

	int cl = ClipLine(x,y,a,b,xoff,yoff,xoff+wwrk,yoff+hwrk);

	x -= xoff;
	y -= yoff;
	a -= xoff;
	b -= yoff;

	if (cl)
	{
		__line(x,y,a,b, color);
		WindowRepaintUser(x1, y1, x2 - x1+1, y2 - y1+1);
	}
	StateUnlock();
}

/**
	Draws the pixel into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x start x position
	@param y start y position
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowPixel(int x, int y, unsigned color)
{
	if (y <	0 || x < 0 || y >= hwrk || x >= wwrk)
	{
		return;
	}
	modify_point(image + w * (y+yoff) + x+xoff, color);
	WindowRepaintUser(x, y, 1, 1);
}

/**
	Draws the patterned line into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param a width
	@param b height
	@param p the Pattern object
	@see set_ppop()
	@see Pattern
*/
void FGAPI FGWindow::WindowPatternLine(int x, int y, int a, int b, FGPattern *p)
{
	StateLock();
	int x1, x2, y1, y2;
	x1 = (x < a) ? x : a;
	x2 = (x < a) ? a : x;
	y1 = (y < b) ? y : b;
	y2 = (y < b) ? b : y;
	pline(x, y, a, b, p);
	WindowRepaintUser(x1, y1, x2 - x1+1, y2 - y1+1);
	StateUnlock();
}

/**
	Draws the circle into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param r radius
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowDrawCircle(int x, int y, int r, unsigned color)
{
	StateLock();
	circle(x, y, r, color);
	WindowRepaintUser(x - r, y - r, r + r + 1, r + r + 1);
	StateUnlock();
}

/**
	Draws the filled circle into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param r radius
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowFillCircle(int x, int y, int r, unsigned color)
{
	StateLock();
	fcircle(x, y, r, color);
	WindowRepaintUser(x - r, y - r, r + r + 1, r + r + 1);
	StateUnlock();
}

/**
	Draws the ellipse into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param rx X radius
	@param ry Y radius
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowDrawEllipse(int x, int y, int rx, int ry, unsigned color)
{
	StateLock();
	ellipse(x, y, rx, ry, color);
	WindowRepaintUser(x - rx, y - ry, rx + rx + 1, ry + ry + 1);
	StateUnlock();
}

/**
	Draws the filled ellipse into the FGWindow. The output will
	takes rop from the current FGWindow values.
	@param x x position
	@param y y position
	@param rx X radius
	@param ry Y radius
	@param color foreground color
	@see set_ppop()
*/
void FGAPI FGWindow::WindowFillEllipse(int x, int y, int rx, int ry, unsigned color)
{
	StateLock();
	fellipse(x, y, rx, ry, color);
	WindowRepaintUser(x - rx, y - ry, rx + rx + 1, ry + ry + 1);
	StateUnlock();
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
void FGAPI FGWindow::WindowDrawArc(int x, int y, double ang1, double ang2, int r, unsigned color)
{
	StateLock();
	arc(x, y, ang1, ang2, r, color);
	WindowRepaintUser(x - r, y - r, r + r + 1, r + r + 1);
	StateUnlock();
}

/**
	Scrolls-down the FGWindow about n-pixels.
	@param xx start x position
	@param yy start y position
	@param ww the width of scrolled block
	@param hh the height of scrolled block
	@param about the size to scroll
*/
void FGAPI FGWindow::WindowScrollDown(int xx, int yy, int ww, int hh ,int about)
{
	StateLock();
	bitblit(xx,yy+about,xx,yy,ww,hh,this,BITBLIT_WITH_DECREMENT);
	box(xx, yy, ww, about, paper);
	WindowRepaintUser(xx, yy, ww, hh + about);
	StateUnlock();
}

/**
	Scrolls-up the FGWindow about n-pixels.
	@param xx start x position
	@param yy start y position
	@param ww the width of scrolled block
	@param hh the height of scrolled block
	@param about the size to scroll
*/
void FGAPI FGWindow::WindowScrollUp(int xx, int yy, int ww, int hh ,int about)
{
	StateLock();
	bitblit(xx,yy-about,xx,yy,ww,hh,this);
	box(xx, yy + hh - about, ww, about, paper);
	WindowRepaintUser(xx, yy - about, ww, hh + about);
	StateUnlock();
}

/**
	Scrolls-left the FGWindow about n-pixels.
	@param xx start x position
	@param yy start y position
	@param ww the width of scrolled block
	@param hh the height of scrolled block
	@param about the size to scroll
*/
void FGAPI FGWindow::WindowScrollLeft(int xx, int yy, int ww, int hh ,int about)
{
	StateLock();
	bitblit(xx - about, yy, xx, yy, ww, hh, this);
	box(xx + ww - about, yy, about, hh, paper);
	WindowRepaintUser(xx - about, yy, ww + about, hh);
	StateUnlock();
}

/**
	Scrolls-right the FGWindow about n-pixels.
	@param xx start x position
	@param yy start y position
	@param ww the width of scrolled block
	@param hh the height of scrolled block
	@param about the size to scroll
*/
void FGAPI FGWindow::WindowScrollRight(int xx, int yy, int ww, int hh ,int about)
{
	StateLock();
	bitblit(xx + about, yy, xx, yy, ww, hh, this, BITBLIT_WITH_DECREMENT);
	box(xx, yy, about, hh, paper);
	WindowRepaintUser(xx, yy, ww + about, hh);
	StateUnlock();
}

void FGAPI FGWindow::WindowUnLock(void)
{
	SetStatus(GetStatus() & (~WLOCKED));
	if (GetStatus() & WDIRTY && IsVisible())
	{
		SetStatus(GetStatus() & (~WDIRTY));
		WindowRepaint(0,0,w,h);
	}
}

void FGWindow::FindNextFindow(void)
{
	FGWindow* This = FGApp::GetCurrentWindow();
	int cnt = FGApp::NumberOfWindow();

	if (cnt <= 1 || (This->GetStatus() & WMODAL))
	{
		Snd(1000,80);
		return;
	}

	FGWindowIterator start = FGApp::Windows.begin();
	FGWindowIterator end = FGApp::Windows.end();

	for (cnt *= 2; start != end && cnt; start++)
	{
		cnt--;
		if (*start == This)
		{
			if (++start != end)
			{
				if ((*start)->GetType()==ROOTWINDOW || ((*start)->GetStatus() & WHIDEN))
					continue;

				(*start)->WindowFocus();
				return;
			}
			else
			{
				start = FGApp::Windows.begin();

				for (; start != end && cnt; start++)
				{
					cnt--;
					if (*start == This)
					{
						return;
					}

					if ((*start)->GetType()==ROOTWINDOW || ((*start)->GetStatus() & WHIDEN ))
						continue;
						(*start)->WindowFocus();
					return;
				}
			}
		}
	}
}

int FGAPI FGWindow::GetXM(int a)
{
	if (!(GetStatus() & WMENU))
	{
		::printf("You must use WMENU option for this window\n");
	}
	c_menupos += a;
	return c_menupos - a;
}

int FGAPI FGWindow::GetYM()
{
	return -(status&WNOTEBOOK ? MENUH+NOTEBOOKH : MENUH);
}

/**
	Resizes the FGWindow and/or changes the FGWindow 's position.
	@param sx a new x position
	@param sy a new y position
	@param wx a new width
	@param wy a new height
*/
void FGAPI FGWindow::WindowShape(int sx, int sy, int wx, int wy)
{
	bool changepos = (sx!=x || sy!=y), changesize=(wx!=w || wy!=h);

	// resizing of centred window is moving too.
	if (status&WCENTRED)
	{
		sx += wx/2;
		sy += wy/2;
		changepos = true;
	}

	if (changepos && !changesize) // iba zmena polohy
	{
		WindowMove(sx-x, sy-y);
		return;
	}
	else if (!changepos && changesize) // iba zmena velkosti
	{
		WindowResize(wx-w, wy-h);
		return;
	}

	StateLock();
	c_menupos = 1;
	if (status&WALIGNED)
	{
		sx &= ~3;
		sy &= ~3;
		wx &= ~3;
		wy &= ~3;
	}

	if (sx>X_width-wx) sx = X_width-wx;
	if (sy>Y_width-wy) sy = Y_width-wy;
	if (sx<0) sx = 0;
	if (sy<0) sy = 0;
	if (wx > X_width) wx = X_width;
	if (wx < 32)	  wx = 32;
	if (wy > Y_width) wy = Y_width;
	if (wy < (BASEBORDER*2+TITLEH+MENUH+STATUSBARH+NOTEBOOKH))
		wy = BASEBORDER*2+TITLEH+MENUH+STATUSBARH+NOTEBOOKH;

	int nx=sx-x, ny=sy-y, dx=wx-w, dy=wy-h;

	// zrus obrysy povodneho okna
	WindowUpdate(nx, ny, 1); // redraw on screen

	if (status&WNOTEBOOK)
		RelocateTabPage(nx, ny);
	else
		Relocate(nx, ny);
	Resize(dx, dy);
	xpos = 2;
	ypos = 0;
	// nakresli nove okno
	draw();

	StateUnlock();

	if (CallWindowHook && changepos)  CallWindowHook(x, y, nx, ny, 1); // 1 as move
	if (CallWindowHook && changesize) CallWindowHook(w, h, dx, dy, 2); // 2 as size

	SendEvent(WINDOWRESIZEEVENT, x,y,w,h);
	SendEvent(WINDOWMOVEEVENT, x,y,w,h);
}

/**
	This function will remap the root window to virtually any size and position relative to the top left corner.
	The call:
	FGWindow::WindowVirtualRoot(-1000,-1000, 2000, 2000);
	Creates an virtual root window with position [1000,1000] in a top left corner of screen.
	@param sx x start position, 0 is top left position, you can use both, positive or negative values.
	@param sy y start position, 0 is top left position, you can use both, positive or negative values.
	@param wx the width, the values is from 1 up to some thousands, it is limited by size of VIDEORAM
	@param wy the height, the values is from 1 up to some thousands, it is limited by size of VIDEORAM
*/
void FGAPI FGWindow::WindowVirtualRoot(int sx, int sy, int wx, int wy)
{
	FGWindow *Root = FGApp::GetRootWindow();
	if (!Root) return;
	int nx=sx-Root->x, ny=sy-Root->y, dx=wx-Root->w, dy=wy-Root->h;

	Root->Relocate(nx, ny);
	Root->Resize(dx, dy);
	Root->clear(0);
	ShowAll();
}

/**
	Moves FGWindow at the new position.
	@param nx the delta of (new_x-old_x) positions
	@param ny the delta of (new_y-old_y) positions
*/
void FGAPI FGWindow::WindowMove(int nx, int ny)
{
	if (status&WALIGNED && !cApp->fulldrag)
	{
		nx &= ~3;
		ny &= ~3;
	}
	if (CallWindowHook) CallWindowHook(x, y, nx, ny, 1); // 1 as move
//	XWaitRetrace();
	StateLock();
	WindowUpdate(nx,ny);
	if (status&WNOTEBOOK)
		RelocateTabPage(nx, ny);
	else
		Relocate(nx, ny);
	WindowRepaint(0,0,w,h);
	StateUnlock();
	SendEvent(WINDOWMOVEEVENT, x,y,w,h);
}

/**
	Resizes the FGWindow.
	@param dx the delta of (new_w-old_w) widths
	@param dy the delta of (new_h-old_h) heights
*/
void FGAPI FGWindow::WindowResize(int dx, int dy)
{
	int span = 0;
	// calculate minimal Window vertical size
	if (status & WFRAMED)
		span += BASEBORDER;
	if (status & WTITLED)
		span += (TITLEH+2);
	if (status & WMENU)
		span += (MENUH+1);
	if (status & WNOTEBOOK)
		span += (NOTEBOOKH+1);

	if (status&WALIGNED && !cApp->fulldrag)
	{
		dx &= ~3;
		dy &= ~3;
	}
	if (w + dx > X_width
		|| w + dx < 32
		|| h + dy > Y_width
		|| h + dy < span)
	{
		return;
	}
	if (CallWindowHook) CallWindowHook(w, h, dx, dy, 2); // 2 as resize

//	XWaitRetrace();
	StateLock();
	c_menupos = 1;
	if (dx<0 || dy<0)
		WindowUpdate(dx,dy);
	Resize(dx, dy);
	xpos = 2;
	ypos = 0;
	draw();
	StateUnlock();
	SendEvent(WINDOWRESIZEEVENT, x,y,w,h);
}

/**
	Draws a rectangle in-memory bitmap to the FGWindow. Function clips all coordinates and sizes correctly.
	If the target area is bigger than source area, then image will be tilted.
	@note the ROP and the ALPHA of source bitmap is used.
	@param x target x position
	@param y target y position
	@param xs source x position
	@param ys source y position
	@param w the width of source area
	@param h the height of source area
	@param p the pointer to the source bitmap
	@see FGDrawBuffer::bitblit() set_ppop() SetAlpha()
*/
void FGAPI FGWindow::WindowPutBitmap(int x, int y, int xs, int ys, int w, int h, FGDrawBuffer * p)
{
	StateLock();
	bitblit(x,y,xs,ys,w,h,p);
	WindowRepaintUser(x, y, w, h);
	StateUnlock();
}

/**
	Gets a rectangle area from the FGWindow int an in-memory bitmap.
	@param x source x position
	@param y source y position
	@param xs target x position
	@param ys target y position
	@param w the width of source area
	@param h the height of source area
	@param p the pointer to the source bitmap
*/
void FGAPI FGWindow::WindowGetBitmap(int x, int y, int xs, int ys, int w, int h, FGDrawBuffer * p)
{
	if (p->GetType() == BMP_NONE)
		return;
	StateLock();
	x += xoff;
	y += yoff;
	RamToRam(x, y, GetW(), GetH(), xs, ys, p->GetW(), p->GetH(), w, h, image, p->GetArray(), BITBLIT_WITH_CLIPPING, _GSET);
	StateUnlock();
}

//! internal for print error message
static void IErrorText(const char *s, int flag)
{
	Snd(1000, 100);
	if (flag)
	{
		printf("FATAL ERROR: %s\n", s);
		getchar();
		exit(flag);
	}
	else
	{
		printf("WARNING: %s\n", s);
		getchar();
	}
}

void IError(const char *s, int flag)
{
	int w = (strlen(s) + 8) * 4;
	static FGWindow *errWindow=0;

	if (fgstate.verbose) ::printf("IError:: %s\n", s);
	if (errWindow) return;

	if (X_width)
	{
		w = w < 75 ? 75 : w;
		Snd(1000,80);
		errWindow = new FGWindow(&errWindow, 0, 0, w * 2, 78, "Error", GuiHwnd(0), CWHITE, CREDLIGHT, ((flag&1) ? WMODAL : 0) | WCENTRED | WCENTRED | WESCAPE);

		if (flag&1) errWindow->WindowText(w - 70, 8, "* * * ERROR * * *", CYELLOW);
		else errWindow->WindowText(w - 78, 8, "* * * WARNING * * *", CYELLOW);

		errWindow->WindowText(32, 28, s, CWHITE);
		errWindow->WindowRect(0,0,2*w,78,CBLACK);

		if (flag&1) errWindow->AddPushButton(w - 40, 48, 80, 21, "Abort", CR, FGControl::Quit);
		else errWindow->AddPushButton(w - 40, 48, 80, 21, "Ok", CR, FGControl::Close);
	}
	else IErrorText(s, flag);
//#ifndef FG_THREADED
	if (flag&2) errWindow->ShowModal();
//#endif
}

WindowDatabase::WindowDatabase(char *exename)
{
	int magic;
	FILE *f;

#ifndef __MSDOS__
   sprintf(name, "%s/.windowxy.wdb", FGApp::homedir);
#else
   strcpy(name, "windowxy.wdb");
#endif
	xyCounter = 0;
	sprintf(exe, "%s::", exename);
	f = fopen(name, "rb");
	if (f)
	{
		fread(&magic, sizeof(int), 1,f);
		fread(&xyCounter, sizeof(int), 1,f);
		if (xyCounter<1 || xyCounter>100 || magic!=0x4357)
		{
			remove(name);
			xyCounter = 0;
			endpointer = &first;
			return; // possibly damaged config
		}
		WindowItem **previous = &first;

		for (int i = 0; i < xyCounter; i++)
		{
			*previous = new WindowItem(f);
			previous = &((*previous)->next);
		}
		*previous = 0;
		endpointer = previous;
		fclose(f);
	}
	else
	{
		endpointer = &first;
	}
}

WindowDatabase::~WindowDatabase()
{
	int magic=0x4357;
	FILE *f;

	if (xyCounter <= 0)	return;
	f = fopen(name, "wb");
	if (f)
	{
		WindowItem *p = first, *pp;
		fwrite(&magic, sizeof(int), 1,f);
		fwrite(&xyCounter, sizeof(int), 1,f);

		for (int i = 0; i < xyCounter; i++)
		{
			pp = p->next;
			p->SaveToFile(f);
			delete p;
			p = pp;
		}
		fclose(f);
	}
}

WindowItem* FGAPI WindowDatabase::findxy(char *s)
{
	WindowItem *p = first;
	char ss[128];

	sprintf(ss, "%s%s", exe, s);
	for (int i = xyCounter; i; --i)
	{
		if (!strcmp(p->name, ss))
			return p;
		p = p->next;
	}
	return 0;
}

WindowItem::WindowItem(FILE *f)
{
	char s[80],*p=s;
	int	c;
	fread(&x, sizeof(int),1,f);
	fread(&y, sizeof(int),1,f);
	fread(&w, sizeof(int),1,f);
	fread(&h, sizeof(int),1,f);
	fread(&icon_x, sizeof(int),1,f);
	fread(&icon_y, sizeof(int),1,f);
	fread(&flag, sizeof(int),1,f);
	while ((c=fgetc(f)) != 0)	*p++=(char)c;
	*p=0;
	name = new char[strlen(s)+1];
	strcpy(name,s);
}

WindowItem::WindowItem(int xx, int yy, int ww, int hh, char	*s)
{
	x=xx;
	y=yy;
	w=ww;
	h=hh;
	icon_x = -1;
	icon_y = -1;
	flag = 0;
	name = new char[strlen(s)+1];
	strcpy(name,s);
}

void FGAPI WindowItem::SaveToFile(FILE *f)
{
	fwrite(&x,sizeof(int),1,f);
	fwrite(&y,sizeof(int),1,f);
	fwrite(&w,sizeof(int),1,f);
	fwrite(&h,sizeof(int),1,f);
	fwrite(&icon_x,sizeof(int),1,f);
	fwrite(&icon_y,sizeof(int),1,f);
	fwrite(&flag,sizeof(int),1,f);
	fputs(name,f);
	fputc(0,f);
}

WindowItem::~WindowItem()
{
	delete [] name;
}

void FGAPI WindowDatabase::Add(int &xx, int &yy, int &ww, int &hh, char *s, int fl)
{
	WindowItem *w;
	if (strlen(s) == 0)
		return;

	lock();
	if ((w = findxy(s))==0)
	{
		char ss[128];
		sprintf(ss, "%s%s",exe, s);
		*endpointer = new WindowItem(xx, yy, ww, hh, ss);
		endpointer = &((*endpointer)->next);
		*endpointer = 0;
		xyCounter++;
	}
	else if (fl&WUSELAST)
	{
		xx = w->x;
		yy = w->y;
		if (fl&WSIZEABLE)
		{
			ww = w->w;
			hh = w->h;
		}
	}
	unlock();
}

void FGAPI WindowDatabase::DatabaseResize(int xx, int yy, char *s)
{
	lock();
	WindowItem *w = findxy(s);
	if (w)
	{
		w->w = xx;
		w->h = yy;
	}
	unlock();
}

void FGAPI WindowDatabase::DatabaseSetIcon(int xx, int yy, char *s)
{
	lock();
	WindowItem *w = findxy(s);
	if (w)
	{
		w->icon_x = xx;
		w->icon_y = yy;
	}
	unlock();
}

void FGAPI WindowDatabase::DatabaseGetIcon(int &xx, int &yy, char *s)
{
	lock();
	WindowItem *w = findxy(s);
	if (w)
	{
		xx = w->icon_x;
		yy = w->icon_y;
	}
	unlock();
}

void FGAPI WindowDatabase::DatabaseRelocate(int xx, int yy, char *s)
{
	lock();
	WindowItem *w = findxy(s);
	if (w)
	{
		w->x = xx;
		w->y = yy;
	}
	unlock();
}

// vykresli titulok okna
void FGWindow::WindowDrawTitle(int color)
{
	if (status & WTITLED) // modry/hnedy titulok
	{
		StateLock();
		SAVE_CLIPPING(this)
		save_state();

		set_font(FGFONT_TITLE);
		set_ppop(_GSET);

		int fr = 0, picto=(status&WNOPICTO?0:(status&WMINIMIZE)?PICTOSIZE*2:PICTOSIZE);
		if (GetStatus() & WFRAMED) fr = BASEBORDER-1;
		box(fr, fr, w - (fr + fr)-picto, TITLEH, color);
		box(fr, fr+TITLEH-1, w - (fr + fr)-picto, 1, CWHITE);
		box(w-((status & WFRAMED) ?4:0), fr, 1, TITLEH, CWHITE);
		if (status&WMINIMIZE && !(status&WNOPICTO))
			box(w-PICTOSIZE*2-4, fr, 1, TITLEH, CWHITE);
		if (status&WMINIMIZE || !(status&WNOPICTO))
			box(w-PICTOSIZE-4, fr, 1, TITLEH, CWHITE);
		text(5 + fr, fr + 1, skipstr(name), CWHITE, color);
		WindowPiktograms(w - PICTOSIZE*2-3, fr);
		WindowRepaint(fr, 0, w-fr-fr, BASEBORDER+TITLEH);

		restore_state();
		RESTORE_CLIPPING(this)
		StateUnlock();
	}
}

// nakresli okno
void FGWindow::draw(void)
{
	int ff = GetStatus() & WFRAMED;
	int	tt = GetStatus() & WTITLED;
	int	pp = GetStatus() & WNOPICTO;
	int	mm = GetStatus() & WMENU;
	int	mw = GetStatus() & WMENUWINDOWTYPE;
	int	ss = GetStatus() & WSTATUSBAR;
	int	nn = GetStatus() & WNOTEBOOK;
	int vs = BASEBORDER, vrch;

	SAVE_CLIPPING(this)
	if (tt) vs+=(TITLEH+2);
	if (mm) vs+=MENUH+1;
	if (nn) vs+=NOTEBOOKH+1;
	vrch=vs;
	if (ss) vs+=STATUSBARH;
	if (!ff)  // ak nemas FRAME, potom urob podklad pod titulok
	{
		vs -= 2;
		vrch -= BASEBORDER;
		if (tt)
			FGmemset(image+w * TITLEH, CBLACK, w); // standard
		else if (h > (TITLEH+1))
			FGmemset(image+w * TITLEH, paper, w); // standard
	}
	if (!ff && !pp && tt)
	{
		FGmemset(image, BORD3, w * TITLEH); // standard
	}
	else if (!tt) FGmemset(image, paper, w * TITLEH);
	if (ff)
	{
		// www optimise it for better resize time
		FGmemset(image, BORD3, w*h); // standard
		//
		box(0, 0, w, 1, BORD1);
		box(0, 0, 1, h, BORD1);
		box(1, h-1, w, 1, BORD2);
		box(w-1, 1, 1, h-1, BORD2);
		if (!mw)
		{
			box(BASEBORDER, vrch, w-BASEBORDER*2, h-BASEBORDER-vs, paper);
			rect(BASEBORDER-1, vrch-1, w-BASEBORDER*2+2, h-BASEBORDER-vs+2, CWHITE);
			if (nn)
				box(BASEBORDER-1, vrch-2, w-BASEBORDER*2+2,2, CBLACK);
			else
				box(BASEBORDER-1, vrch-1, w-BASEBORDER*2+2,1, CBLACK);
			box(BASEBORDER-1, vrch-1, 1, h-BASEBORDER-vs+2, CBLACK);
		}
		else
		{
			box(BASEBORDER-2, vrch-2, w-BASEBORDER*2+4, h-BASEBORDER-vs+4, paper);
		}
	}
	else if (h > (TITLEH+1))
		FGmemset(image+w*TITLEH +w, paper, w*(h-TITLEH-1)); // standard

	if (status & WSIZEABLE && ff)
	{
		box(w - 13, h - 3, 12, 2, CYELLOW);
		box(w - 3,  h - 13, 2, 12, CYELLOW);
	}
	if (mm)
	{
		if (ff)
		{
			box(xoff-1, yoff - MENUH-2, w - xoff * 2+2, MENUH+2, CBLACK);
			box(xoff, vrch-MENUH, 1, MENUH+2, CWHITE);
		}
		else
		{
			box(xoff, yoff - MENUH-1-(tt?1:0), w - xoff * 2, MENUH+1+(tt?1:0), CBLACK);
			if (tt)
				box(xoff,MENUH+1, w-xoff*2, MENUH, MENUBG);
			else
				box(xoff,0, w-xoff*2, MENUH, MENUBG);
		}
		box(xoff, yoff - MENUH-1, w - xoff * 2, MENUH, MENUBG);
		box(xoff, yoff - MENUH-1, 1, MENUH, CBLACK);
	}
	if (ss)
	{
		if (ff == 0)
		{
			box(0, h-STATUSBARH, w, STATUSBARH,statuscolor);
		}
	}

	RESTORE_CLIPPING(this)

	if (first_draw)
		SendEvent(REPAINTEVENT);

	SAVE_CLIPPING2(this)

	RedrawControls();

	RESTORE_CLIPPING(this)
	first_draw = 1;
	if (FGApp::GetCurrentWindow() == this)
	{
		WindowStatus(WACTIVE);
	}
	else
	{
		WindowStatus(WDEACTIVE);
	}
	WindowRepaint(0, 0, w, h);
}

static void InitCRC(void)
{
  int I, J;
  unsigned int C;

  for (I=0;I<256;I++)
  {
	for (C=I,J=0;J<8;J++)
	  C=(C & 1) ? (C>>1)^0xEDB88320L : (C>>1);
	CRCTab[I]=C;
  }
}

unsigned int CalculateCRC(unsigned StartCRC, void *Addr, unsigned Size)
{
  unsigned I;
  if (!StartCRC) InitCRC();
  for (I=0; I<Size; I++)
	StartCRC = CRCTab[(unsigned char)StartCRC ^ ((unsigned char*)Addr)[I]] ^ (StartCRC >> 8);
  return(StartCRC);
}

/**
sets the new client space area in the FGWindow, i.e. clipping for drawing.
@param x new top left corner coordinate in x
@param y new top left corner coordinate in y
@param w a new width
@param h a new height
*/
bool FGAPI FGWindow::WindowSetWorkRect(int x,int y,int w,int h)
{
	if (set_clip_rect(x,y,w,h)==0)
		return false; // error!
	xpos = 2;
	ypos = 0;
	return true;
}

void FGAPI FGWindow::SendToWindow(FGEvent *e)
{
	if (image==0)
	{
		IError("Internal Error 1 (SendToWindow)",	1);
		return;
	}

	assert(this);
	e->wnd = this;
	int cc = GetId();

	StateLock();
	switch(e->GetType())
	{
		case REPAINTEVENT:
			OnPaint();
			break;
		case GETFOCUSEVENT:
			OnFocus();
			break;
		case LOSTFOCUSEVENT:
			OnLostFocus();
			break;
//		case INITEVENT:
//			OnInit();
//			break;
		case KEYEVENT:
			OnKeyPress(e->GetKey());
			break;
		case MOVEEVENT:
			OnMouseMove(e->GetX(), e->GetY());
			break;
		case ACCELEVENT:
			OnAccelerator(e->accel);
			break;
		case WINDOWMOVEEVENT:
			OnMove(e->GetX(), e->GetY());
			break;
		case WINDOWRESIZEEVENT:
			OnResize(e->GetW(), e->GetH());
			break;
		case BUTTONHOLDEVENT:
			break;
		case STARTDRAGLEFTEVENT:
			OnStartDrag(1, e->GetX(), e->GetY());
			break;
		case STARTDRAGRIGHTEVENT:
			OnStartDrag(0, e->GetX(), e->GetY());
			break;
		case DRAGLEFTEVENT:
			OnEndDrag(1, e->GetX(), e->GetY(), e->GetW(), e->GetH());
			break;
		case DRAGRIGHTEVENT:
			OnEndDrag(0, e->GetX(), e->GetY(), e->GetW(), e->GetH());
			break;
		case CLICKLEFTEVENT:
			OnClick(e->GetX(), e->GetY());
			break;
		case DBLCLICKLEFTEVENT:
			OnDoubleClick(e->GetX(), e->GetY());
			break;
		case CLICKMIDDLEEVENT:
			OnMiddleButton(e->GetX(), e->GetY());
			break;
		case CLICKRIGHTEVENT:
			OnContextPopup(e->GetX(), e->GetY());
			break;
		case MOUSEWHEELEVENT:
			OnWheel(e->GetX(), e->GetY(), e->GetButtons());
			break;
		case TABSWITCHEVENT:
			OnTabSwitch((const char *)e->GetKey());
			break;
//		case TERMINATEEVENT:
//			OnClose();
//			break;
		case ICONIZEEVENT:
			OnIconize();
			break;
		case NOTIFYEVENT: // not implemented
//			OnCloseQuery(e->GetKey());
			break;
//		default:
//			::printf("Unhandled message %d in WndHandler!", e->GetType());
	}

	FGWindow *ww = FGApp::WindowFind(cc);

	if (ww && handler)
		handler(e);

	// call only if don't removed
	ww = FGApp::WindowFind(cc);
	if (ww)
		StateUnlock();
}

void FGMenuWindow::MenuWindowHandler(FGEvent *p)
{
	FGControl* ctrl;

	if (Proc) Proc(p);

	if (CurrentMenu==0) return;

	switch(p->GetType())
	{
		case MOVEEVENT:
			if (p->GetKey()) // if cursor is over button
			{
				ctrl = FGControl::ButtonFind(p, 5);
//::printf("CTRL = %d\n", ctrl);
				if (ctrl)
				{
//::printf("	name = %s\n", ctrl->name);
					p->wnd->SetDefaultControl(ctrl);
				}
			}
			break;
		case KEYEVENT:
			if (p->GetKey() !=	ESC) break;
			delete CurrentMenu;		// QWERT
			break;
		case ACCELEVENT:
			p->wnd->RunHandler();
			if (!(p->GetObjectType() == LABEL
				|| p->GetObjectType() == PUSHBUTTON
				|| p->GetObjectType() == BASEMENU
				|| p->GetObjectType() == BASEMENUITEM))  break;
		case LOSTFOCUSEVENT:
		case TERMINATEEVENT:
			if (CurrentMenu == p->wnd)
				delete CurrentMenu; // do not delete others QWERT
			break;
	}
}

FGMenuWindow::FGMenuWindow(int w, int h, GuiHwnd proc, void* user_data):
	FGWindow(0,
		fgstate.__hint_x_menu==-1 ? cApp->GetMouseX()+4 : fgstate.__hint_x_menu,
		fgstate.__hint_y_menu==-1 ? cApp->GetMouseY()+4 : fgstate.__hint_y_menu,
		w,
		h,
		"",
		MenuWindowHandler,
		CScheme->menuwindow_fore,
		CScheme->menuwindow_back,
		WFRAMED|WNOPICTO|WMENUWINDOWTYPE|WUSESELECTEDCONTROL|WNODRAWFROMCONSTRUCTOR, user_data)
{
	draw();
	_initmw(proc);
}

void FGMenuWindow::draw(void)
{
	SAVE_CLIPPING(this)
	box(0,0,w,h,CScheme->menuwindow_back);
	rect(0,0,w,h, CScheme->menuwindow_frame);
    WindowRepaint(0,0,w,h);
   	RESTORE_CLIPPING(this)
}

void FGAPI FGWindow::default_clipping(void)
{
	if (status & WFRAMED)
		yoff += BASEBORDER;
	if (status & WTITLED)
		yoff += (TITLEH+2);
	if (status & WMENU)
		yoff += (MENUH+1);
	if (status & WNOTEBOOK)
		yoff += (NOTEBOOKH+1);
	if (status & WFRAMED)
		xoff += BASEBORDER;

	// set clipping window
	wwrk = w - xoff*2;
	hwrk = h - yoff;
	if (status & WFRAMED)
		hwrk -= BASEBORDER;
	if (status & WSTATUSBAR)
		hwrk -= STATUSBARH;
}

// ROOT wnd
FGWindow::FGWindow(FGWindow **wp, FGPixel color):
  FGBaseGui(0, 0, X_width, Y_width, "Root", ROOTWINDOW, CWHITE, CBLACK, WBACK|WUNMOVED), StateMutex(recursiveMutEx)
{
	init();
	assert(! FGApp::GetCurrentWindow() );		// must be first
	Resize(0, 0);
	clear(color);

	if (wp) *wp = this;			// save pointer to itself
	itself = wp;
	FGApp::AddWindowToList(this, status);
	fgstate.DoRootWindow = 1;
	FGEvent e(INITEVENT);
	SendToWindow(&e);
}

/**
	@param wp pointer at the pointer at	the window itself. It is an address of store of pointer to this window.
	When window	is destroyed, to this pointer is assigned NULL. You can basically
	test this pointer to NULL to prevent manipulating with destroyed
	object.
	@param xs X position
	@param ys Y position
	@param ws width of complete window (client space is this size minus the window frame.)
	@param hs height of complete window (client space is this size minus the window frame.)
	@param nm standard C string, which can be with any length. This name will be displayed it the title of the window
	(if title is present).
	@param hnd pointer to the window callback procedure. It is a big switch
	command with many cases. You can set this to 0 by default.
	@param i ink color
	@param p paper color
	@param flag for the window properties (e.g. has frame, has title etc.)
	@param user_data sets user data pointer to this parameter ( see GetUserData() )
*/
FGWindow::FGWindow(FGWindow **wp, int xs, int ys, int ws, int hs, const char *nm, GuiHwnd hnd, int i, int p, int flag, void* user_data):
  FGBaseGui(xs, ys, ws, hs, nm, flag & WMENUWINDOWTYPE ? MENUWINDOW : WINDOW, i, p, flag/*|WEXIST*/), StateMutex(recursiveMutEx)
{
	int ww = 0;
	int hh = 0;

	SetUserData(user_data);
	
	// fix minimal size - 5.feb.2003
	if (w<8) ww = 8;
	if (h<24) hh = 24;

	if (ww && hh)
		FGDrawBuffer::Resize(ww-w,hh-h);
	else if (ww)
		FGDrawBuffer::Resize(ww-w, 0);
	else if (hh)
		FGDrawBuffer::Resize(0, hh-h);

	SetInk(i);
	SetPaper(p);

	init();

	if (entryPoint)
		entryPoint->DatabaseGetIcon(icon_x, icon_y, name);

	if (FGApp::GetCurrentWindow())
		if (FGApp::GetCurrentWindow()->GetStatus()&WMENUWINDOWTYPE && GetCounter()>1)
	{
		FGEvent e(TERMINATEEVENT);
		FGApp::GetCurrentWindow()->SendToWindow(&e);
	}

	Resize(0, 0);

	FGWindow *lst = FGApp::GetCurrentWindow();

	FGApp::AddWindowToList(this, status);

	handler = hnd;
	if (wp) *wp = this;			// save pointer to itself
	itself = wp;

	statuscolor = CScheme->statusbar;

	if (lst)
	{
		FGEvent e(LOSTFOCUSEVENT);
		lst->SendToWindow(&e);
		lst->WindowStatus(WDEACTIVE);
	}
	first_draw = 0;
	default_clipping();

	FGControlContainer cont("", yoff-NOTEBOOKH-2);
	ctrl_list.push_back( cont );
	Buttony = ctrl_list.begin();

	if ((status & WNODRAWFROMCONSTRUCTOR) == false)
		draw();

	FGEvent e(INITEVENT);
	SendToWindow(&e);

	if ((lst && flag & WLASTFOCUS) || (lst && lst->GetStatus() & WMODAL))
		lst->WindowFocus();
}

void FGAPI FGWindow::RedrawControls(void)
{
	// redraw notebooks
	if (status&WNOTEBOOK)
		DrawTabPages();

	FGControlIterator pt = Buttony->begin();

	while ( pt != Buttony->end() )
	{
		(*pt)->draw();
		pt++;
	}
}

bool FGAPI FGWindow::RemoveControl(FGControlBoxIterator iter, FGControl* ctrl)
{
	FGControlIterator butony = iter->begin();

	while ( butony != iter->end() )
	{
		if (ctrl == *butony)
		{
			delete (*butony);
			iter->erase(butony);
			cApp->AutoRepeatEnd();
			return true;
		}
		butony++;
	}
	return false;
}

void FGAPI FGWindow::RemoveControl(FGControl* ctrl)
{
	FGControlBoxIterator iter = ctrl_list.begin();

	while (iter != ctrl_list.end())
	{
		if (RemoveControl(iter, ctrl))
			return;
		iter++;
	}
}

void FGAPI FGWindow::RemoveControls(void)
{
	RemoveControls(Buttony);
}

void FGAPI FGWindow::EnableControls(void)
{
	EnableControls(Buttony);
}

void FGAPI FGWindow::DisableControls(void)
{
	DisableControls(Buttony);
}

void FGAPI FGWindow::RemoveControls(FGControlBoxIterator iter)
{
	FGControlIterator butony = iter->begin();
	FGControlIterator ende = iter->end();

	while ( butony != ende )
	{
		FGControl* ctrl = *butony;
		*butony = 0;
		delete ctrl;
		butony++;
	}
	iter->erase( iter->begin(), iter->end() );

	cApp->AutoRepeatEnd();
}

void FGAPI FGWindow::EnableControls(FGControlBoxIterator iter)
{
	FGControlIterator butony = iter->begin();

	while ( butony != iter->end() )
	{
		(*butony)->Enable();
		butony++;
	}
}

void FGAPI FGWindow::DisableControls(FGControlBoxIterator iter)
{
	FGControlIterator butony = iter->begin();

	while ( butony != iter->end() )
	{
		(*butony)->Disable();
		butony++;
	}
}

FGWindow::~FGWindow()
{
	if (iconized && iconized!=(FGWindow *)-1)
		delete iconized;


	if (type!= ROOTWINDOW)
	{
		WindowFlushInput();
		SendEvent(TERMINATEEVENT);

		StateLock();
		if (itself) *itself = 0;
		DeleteTabPages();
		StateUnlock();

		if (!fgstate.shut_down) WindowUpdate(0,0,1);		// delete all
	}

	status = 0;			// no anythings

	bool was_current = (FGApp::GetCurrentWindow() == this);

	FGApp::RemoveIterator(this);

	if (FGApp::NumberOfWindow() >= 1 && was_current)
	{
		FGWindow* last = FGApp::GetLastWindow();

		if (last->type != ROOTWINDOW)
			FGApp::SetCurrentWindow(last);
		else
			FGApp::SetCurrentWindow(0);

		if (FGApp::GetCurrentWindow())
		{
			if (!fgstate.shut_down)
				FGApp::GetCurrentWindow()->WindowStatus(WACTIVE);
		}
	}
	while(state_locked)
	{
		StateUnlock();
	}
}

/**
	Adds a separator to the menu.
*/
void FGMenuWindow::Separator(void)
{
	int i;
	WindowBox(0, i=GetYM(8)+3, GetWW(), 1, CBLACK);
	WindowBox(0, i+1, GetWW(), 1, CWHITE);
}

/**
	Adds a text form of FGPushButton to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param ws width of the button
	@param hs height of the button, there is recomended values from 21 and above
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGPushButton FGButtonGroup
*/
FGPushButton *FGWindow::AddPushButton(int xs, int ys, int ws, int hs, const char *nm,	int	key, ControlCall f, void* user_data)
{
	return new FGPushButton(xs, ys, ws, hs, nm, key, this,f, user_data);
}

/**
	Adds an icon form of FGPushButton to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param bm pointer to image object - ICON as a FGDrawBuffer object
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGPushButton FGButtonGroup
*/
FGPushButton *FGWindow::AddPushButton(int xs, int ys, int key, FGDrawBuffer *bm, ControlCall f, void* user_data)
{
	if (bm==0 || bm->GetType()==0)
	{
		// fallback if no valid image was passed
		new FGPushButton(xs, ys, 128, 21, "image not found!!!", 0, this, ControlCall(0), user_data );
	}
	return new FGPushButton(xs, ys, key, this,bm, f, user_data);
}

/**
	Adds an icon form of FGPushButton to the FGWindow. This method can set size & name of button directly.
	@param xs x position in a window
	@param ys y position in a window
	@param ws width of the button
	@param hs height of the button, there is recomended values from 21 and above
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param bm pointer to image object - ICON as a FGDrawBuffer object
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGPushButton FGButtonGroup
*/
FGPushButton *FGWindow::AddPushButton(int xs, int ys, int ws, int hs, const char *nm, int key, FGDrawBuffer *bm, ControlCall f, void* user_data)
{
	if (bm==0 || bm->GetType()==0)
	{
		// fallback if no valid image was passed
		new FGPushButton(xs, ys, 128, 21, "image not found!!!", 0, this, ControlCall(0), user_data );
	}
	return new FGPushButton(xs, ys, ws, hs, nm, key, this, bm, f, user_data);
}

/**
	Adds FGTwoStateButton to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param w width of the button
	@param h height of the button, there is recomended values from 21 and above
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGTwoStateButton FGButtonGroup
*/
FGTwoStateButton	*FGWindow::AddTwoStateButton(int	xs,	int	ys,	int w, int h, const char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGTwoStateButton(xs, ys, w, h, nm, key, this, variable, f, user_data);
}

/**
	Adds a FGCheckBox to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGWindow::AddCheckBox(int	xs,	int	ys,	const char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(1,xs, ys, nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a FGCheckBox to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to bool variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGWindow::AddCheckBox(int	xs,	int	ys,	const char *nm, int key, bool * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(xs, ys, nm, key, this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a FGRadioButton to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGRadioButton FGButtonGroup
*/
FGRadioButton	*FGWindow::AddRadioButton(int	xs,	int	ys,	const char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGRadioButton(1,xs, ys, nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a masked version FGCheckBox to the FGWindow. This is like a normal check box
	but it is controlled by one of a 32 bit in a triggered variable (of type uint32).
	This is useful when you want change only one bit of variable.
	@param mask bit mask value, by example 1 for a bit 0 or (1<<20) for twenth bit.
	@param xs x position in a window
	@param ys y position in a window
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGWindow::AddCheckBoxMask(int mask, int	xs,	int	ys,	const char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(mask, xs, ys, nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a masked version FGRadioButton to the FGWindow. This is like a normal check box
	but it is controlled by one of a 32 bit in a triggered variable (of type uint32).
	This is useful when you want change only one bit of variable.
	@param mask bit mask value, by example 1 for a bit 0 or (1<<20) for twenth bit.
	@param xs x position in a window
	@param ys y position in a window
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGRadioButton FGButtonGroup
*/
FGRadioButton	*FGWindow::AddRadioButtonMask(int mask, int	xs,	int	ys,	const char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGRadioButton(mask, xs, ys, nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an integer number without the test to in-a-range-number.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, int *p, ControlCall f, void* user_data)
{
	return new FGEditBox(xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, 0, 0, 0, user_data); // no checking
}

/**
	Adds an FGEditBox for an one-line-edit box for an integer number with the  test to in-a-range-number.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param min minimal correct value from -2147483648 to 2147483647 for a range testing
	@param max maximal correct value from -2147483648 to 2147483647 for a range testing
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, int *p, ControlCall f, int min, int max, void* user_data)
{
	return new FGEditBox(xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, min,	max, 1, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an string. The max length of string is get from the ws2 parameter/8, i.e. you don't type bigger string string that that value.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, char *p, ControlCall f, void* user_data)
{
	return new FGEditBox(0, xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an string. The max length of string is get from the ws2 parameter/8, i.e. you don't type bigger string string that that value.
	@param sz is the maximal size of input string in range from 1 to 127 characters.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int sz, int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, char *p, ControlCall f, void* user_data)
{
	return new FGEditBox(sz, xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an double number with the  test to in-a-range-number.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited double variable
	@param f a callback function of type ControllCall
	@param min minimal correct value for a range testing
	@param max maximal correct value for a range testing
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, double *p, ControlCall f, double min, double max, void* user_data)
{
	return new FGEditBox(xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, min,	max, 1, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an double number without the test to in-a-range-number.
	@param xs x position in a window
	@param ys y position in a window
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited double variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGWindow::AddEditBox(int	xs,	int	ys,	int	ws1, int ws2, const char *nm,	int	key, double *p, ControlCall f, void* user_data)
{
	return new FGEditBox(xs, ys, ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, 0, 0, 0, user_data); // no checking
}

/**
	Adds an line menu item to the FGWindow. This used for pop-up menus. The position and size is calculated automatically.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGBaseMenu
*/
FGBaseMenu *FGWindow::AddBaseMenu(const char *nm, int key, ControlCall f, void* user_data)
{
	return new FGBaseMenu(nm, key, this, f, FGFONT_MENU, user_data);
}

/**
	Adds a text Label.
	@param x x position in a window
	@param y y position in a window
	@param s name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param f a callback function of type ControllCall
	@param i explicit foreground color, default is color of parents
	@param p explicit background color, default is color of parents
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl Label
*/
FGLabel *FGWindow::AddLabel(int x, int y, const char *s, int key, ControlCall f, unsigned i, unsigned p, void* user_data)
{
	return new FGLabel(x,y,s,key,this,f,i==UNDEFINED_COLOR ?this->ink:i, p==UNDEFINED_COLOR ?this->paper:p, user_data);
}

/**
	Adds panel to the FGWindow.
	@param x x position in a window
	@param y y position in a window
	@param w width of the button caption, the text is right alligned
	@param h width of the button input part
	@param s name of the panel
	@param paper explicit background color, default is color of parents
	@param ink explicit foreground color1, default is CWHITED
	@param ink2 explicit foreground color1, default is CDARK
	@note This object is replacement for an older WindowPanel() methods that is absolete
	and will be removed in OpenGUI 4.4.0
	The main reason is compatibility with TabPages - WindowPanel() doesn't work properly with one.
*/
FGPanel* FGWindow::AddPanel(int x, int y, int w, int h, const char *s, unsigned paper, unsigned ink, unsigned ink2)
{
	return new FGPanel(x,y,w,h,s,this,ink,ink2,paper==UNDEFINED_COLOR?this->paper:paper);
}

/**
	Adds dynamic text string to the FGWindow.
	@param x x position in a window
	@param y y position in a window
	@param s name of the panel
	@param ink explicit foreground color1, default is CWHITED
	@param paper explicit background color, default is color of parents
*/
FGText* FGWindow::AddText(int x, int y, const char *s, unsigned ink, unsigned paper)
{
	return new FGText(x,y,s,this,ink==UNDEFINED_COLOR?this->ink:ink,paper==UNDEFINED_COLOR?this->paper:paper);
}

/**
	Adds dynamic image to the FGWindow.
	The attached image is nor released at the end of live.
	@param x x position in a window
	@param y y position in a window
	@param img Image
*/
FGImage* FGWindow::AddImage(int x, int y, FGDrawBuffer* img)
{
	return new FGImage(x, y, img, this);
}

/**
	Adds a horizontal slider to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param min the start value of slider
	@param max the last value of slider
	@param step thes stepping (1 and above)
	@param val a pointer to the controled int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGSlider FGSlideBarH
*/
FGSlideBarH *FGWindow::AddSlideBarH(int xs, int ys, int min, int max, int step, int *val, ControlCall f, void* user_data)
{
	return new FGSlideBarH(xs, ys, min, max, step, val, this,f, user_data);
}

/**
	Adds a vertical slider to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param min the start value of slider
	@param max the last value of slider
	@param step thes stepping (1 and above)
	@param val a pointer to the controled int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGSlider FGSlideBarV
*/
FGSlideBarV *FGWindow::AddSlideBarV(int xs, int ys, int min, int max, int step, int *val, ControlCall f, void* user_data)
{
	return new FGSlideBarV(xs, ys, min, max, step, val, this,f, user_data);
}

/**
	Adds a LISTBOX container to the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param w width of the one item
	@param h height of the one item
	@param dropdown number of lines that list box occupies. This is not a real number items, it is only visual size.
	@param drawone a callback function of type FGListBoxCallBack
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGListBox listbox0
*/
FGListBox * FGWindow::AddListBox(int xs, int ys, int w, int h, int dropdown, FGListBoxCallBack drawone, void* user_data)
{
	return new FGListBox(xs, ys, w, h, dropdown, this, drawone, user_data);
}

/**
	Adds a FGProgressBar into the FGWindow.
	@param xs x position in a window
	@param ys y position in a window
	@param ws width of widget
	@param hs height of widget
	@param size the progressed value
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGProgressBar
*/
FGProgressBar *FGWindow::AddProgressBar(int xs, int ys, int ws, int hs, int size)
{
	return new FGProgressBar(this, xs,ys,ws,hs,size);
}

/**
	Adds an single menu item to the FGMenuWindow. This used for pop-up menus. The position and size is calculated automatically.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGBaseMenuItem
*/
FGBaseMenuItem *FGMenuWindow::AddMenu(const char *nm,	int	key, ControlCall f, void* user_data)
{
	return new FGBaseMenuItem(nm, key, this, f, FGFONT_MENU, user_data);
}

/**
	Adds a Label for an one-line-edit box for an string. The max length of string is get from the ws2 parameter/8, i.e. you don't type bigger string string that that value.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param f a callback function of type ControllCall
	@param i explicit foreground color, default is color of parents
	@param p explicit background color, default is color of parents
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl Label
*/
FGLabel *FGMenuWindow::AddLabel(char *nm, int key, ControlCall f, unsigned i, unsigned p, void* user_data)
{
	return new FGLabel(GetXM(), GetYM(18),nm,key,this,f,i==UNDEFINED_COLOR?this->ink:i, p==UNDEFINED_COLOR?this->paper:p, user_data);
}

/**
	Adds dynamic text string to the FGMenuWindow.
	@param s name of the panel
	@param ink explicit foreground color1, default is CWHITED
	@param paper explicit background color, default is color of parents
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
*/
FGText* FGMenuWindow::AddText(const char *s, unsigned ink, unsigned paper, void* user_data)
{
	return new FGText(GetXM(), GetYM(18), s,this,ink==UNDEFINED_COLOR?this->ink:ink,paper==UNDEFINED_COLOR?this->paper:paper);
}


/**
	Adds a FGCheckBox to the FGMenuWindow.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGMenuWindow::AddCheckBox(char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(1, GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a FGCheckBox to the FGMenuWindow.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to bool variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGMenuWindow::AddCheckBox(char *nm, int key, bool * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f, variable, user_data);
}

/**
	Adds a FGRadioButton to the FGMenuWindow.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGRadioButton	*FGMenuWindow::AddRadioButton(char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGRadioButton(1, GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a FGRadioButton to the FGMenuWindow.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGRadioButton	*FGMenuWindow::AddRadioButton(char *nm, int key, bool * variable, ControlCall f, void* user_data)
{
	return new FGRadioButton(1, GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a masked version FGCheckBox to the FGMenuWindow. This is like a normal check box
	but it is controlled by one of a 32 bit in a triggered variable (of type uint32).
	This is useful when you want change only one bit of variable.
	@param m bit mask value, by example 1 for a bit 0 or (1<<20) for twenth bit.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGCheckBox FGButtonGroup
*/
FGCheckBox	*FGMenuWindow::AddCheckBoxMask(int m, char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGCheckBox(m, GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds a masked version FGRadioButton to the FGMenuWindow. This is like a normal check box
	but it is controlled by one of a 32 bit in a triggered variable (of type uint32).
	This is useful when you want change only one bit of variable.
	@param m bit mask value, by example 1 for a bit 0 or (1<<20) for twenth bit.
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param variable a pointer to int variable. The value of this variable controls the state of button (true/false)
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGRadioButton FGButtonGroup
*/
FGRadioButton	*FGMenuWindow::AddRadioButtonMask(int m, char *nm, int key, int * variable, ControlCall f, void* user_data)
{
	return new FGRadioButton(m, GetXM(),	GetYM(18), nm, key,	this,GetInk(),GetPaper(),f,variable, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an integer number with the  test to in-a-range-number.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@param min minimal correct value from -2147483648 to 2147483647 for a range testing
	@param max maximal correct value from -2147483648 to 2147483647 for a range testing
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int	ws1, int ws2, char *nm,	int	key, int *p, ControlCall f, int min, int max, void* user_data)
{
	return new FGEditBox(GetXM(),	GetYM(24), ws1,	ws2, nm, key, this, p, GetInk(),GetPaper(), f, min, max, 1, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an integer number without the test to in-a-range-number.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int	ws1, int ws2, char *nm,	int	key, int *p, ControlCall f, void* user_data)
{
	return new FGEditBox(GetXM(),	GetYM(24), ws1,	ws2, nm, key, this, p, GetInk(),GetPaper(),f, 0, 0, 0, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an string. The max length of string is get from the ws2 parameter/8, i.e. you don't type bigger string string that that value.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int	ws1, int ws2, char *nm,	int	key, char *p, ControlCall f, void* user_data)
{
	return new FGEditBox(0, GetXM(),	GetYM(24), ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an string. The max length of string is get from the ws2 parameter/8, i.e. you don't type bigger string string that that value.
	@param sz is the maximal size of input string in range from 1 to 127 characters.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited int variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int sz, int	ws1, int ws2, char *nm,	int	key, char *p, ControlCall f, void* user_data)
{
	return new FGEditBox(sz, GetXM(),	GetYM(24), ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an double number with the  test to in-a-range-number.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited double variable
	@param f a callback function of type ControllCall
	@param min minimal correct value for a range testing
	@param max maximal correct value for a range testing
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int	ws1, int ws2, char *nm,	int	key, double	*p, ControlCall f, double min, double max, void* user_data)
{
	return new FGEditBox(GetXM(),	GetYM(23), ws1,	ws2, nm, key, this,	p, GetInk(), GetPaper(),	f, min,	max, 1, user_data);
}

/**
	Adds an FGEditBox for an one-line-edit box for an double number without the test to in-a-range-number.
	@param ws1 width of the button caption, the text is right alligned
	@param ws2 width of the button input part
	@param nm name of the button
	@param key a keycode used as a hot-key for the button (it isn't case sensitive). By example: 'a', 0x20 or ALT_G
	@param p a pointer to the edited double variable
	@param f a callback function of type ControllCall
	@param user_data an optional user data pointer (for better binding between GUI and user's objects).
	@return a pointer to created object, you can save this one if you can call any methods.
	@note You don't need save the pointer if you don't want call other methods ( e.g. Enable() ) or groups with other buttons.
	The object will be release automatically when the FGWindow will be destroyed. Don't destruct the object explicitly!
	@see FGBaseGui FGControl FGEditBox
*/
FGEditBox	*FGMenuWindow::AddEditBox(int	ws1, int ws2, char *nm,	int	key, double	*p,ControlCall f, void* user_data)
{
	return new FGEditBox(GetXM(),	GetYM(23), ws1,	ws2, nm, key, this,	p, GetInk(),GetPaper(),	f, 0, 0, 0, user_data);
}

void FGAPI FGWindow::Relocate(int xx, int yy)
{
	x += xx;
	y += yy;
	if (entryPoint)
		entryPoint->DatabaseRelocate(x,y,name);
}

static void Yes_No(CallBack t)
{
	FGEvent e(NOTIFYEVENT, t->GetLocalId());
	delete _Note_Ptr;
	close_adept->SendToWindow(&e);
	close_adept = 0;
}

/**
	Shows a dialog for save changes if needed (window has set WNOTIFY and has been
	marked as changed (by SetChange()). If the window has set WNOTIFY flag but isn't
	changed then function sends NOTIFYEVENT to the window immediatelly and returns.
	@return true if notifycation has been performed else returns false.
	@see TextEditor SetChange() ResetChange() WindowClose()
*/
bool FGAPI FGWindow::ShowNotify(void)
{
	if (status&WNOTIFY && !close_adept && changed)
	{
		close_adept = this;
		_Note_Ptr = new FGWindow(&_Note_Ptr, 320, 334, 308, 148, "Warning!", 0, CYELLOW, CREDLIGHT, 0xa7);
		_Note_Ptr->AddPushButton(168, 72, 64, 21, "No", 'n', Yes_No);
		_Note_Ptr->AddPushButton(56, 72, 64, 21, "Yes", 'y', Yes_No);
		_Note_Ptr->WindowText(32, 24, "The contents has been changed. Save it?");
		return true;
	}
	else if (status&WNOTIFY && !close_adept)
 // not changed -> close immediatelly
	{
		SendEvent(NOTIFYEVENT);
		return true;
	}
	return false;
}

/**
Closes the FGWindow and release all the controls associated with it.
This is a wrapper for window delete operator. You can still use operator delete
that destroy window immediatelly and unconditionally but you lose a notify mechanism (WNOTIFYEVENT).
WindowClose() test the window for 'changed' flag and WNOTIFY and shows a dialog
"SAVE CHANGES?". For more see at TextEditor implementation where it is used.
@return false if the window has been destroyed or true if still not.
@see ~FGWindow TextEditor
*/
bool FGWindow::WindowClose(void)
{
	if (ShowNotify())
		return true;

	if (status&WNOPICTO)
	{
		IError("Can`t close this window",0);
		return true;
	}

	delete this;
	return false;
}

static void ShowEfect(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	int s=25, xs=(x2-x1)/s, ys=(y2-y1)/s, ws=(w2-w1)/s, hs=(h2-h1)/s;
	for(;s;s--)
	{
		draw_box(x1,y1,w1,h1,0,_GNOT);
		draw_box(x1+1,y1+1,w1,h1,0,_GNOT);
		draw_box(x1+1,y1+1,w1,h1,0,_GNOT);
		draw_box(x1,y1,w1,h1,0,_GNOT);
		x1 += xs;
		y1 += ys;
		w1 += ws;
		h1 += hs;
	}
}

int FGWindow::iconize(int xs, int ys)
{
	int rc;
	if (!(status&WMINIMIZE)) return 0; // bad window

	StateLock();
	if (iconized==0)
	{
		if (cApp->fulldrag && icon) ShowEfect(x,y,w,h,xs,ys,icon->GetW(),icon->GetH());
		WindowStatus(WHIDE);
		if (status&WNOICON) iconized = (FGWindow *)-1;
		else
		{
			iconized = new IconWindow(this, xs, ys, icon);
		}
		icon_x = xs;
		icon_y = ys;
		rc = 1;
	}
	else if (iconized != (FGWindow *)-1)
	{
		icon_x = iconized->x;
		icon_y = iconized->y;
		if (cApp->fulldrag && icon) ShowEfect(icon_x, icon_y, icon->GetW(),icon->GetH(),x,y,w,h);
		delete iconized;
		iconized=0;
		WindowStatus(WVISIBLE);
		WindowFocus();
		rc = 2;
	}
	else // hide and no icon
	{
		if (cApp->fulldrag && icon) ShowEfect(icon_x, icon_y, icon->GetW(),icon->GetH(),x,y,w,h);
		iconized=0;
		WindowStatus(WVISIBLE);
		WindowFocus();
		rc = 2;
	}
	if (entryPoint) entryPoint->DatabaseSetIcon(icon_x, icon_y, name);
	StateUnlock();
	FGEvent e(ICONIZEEVENT, IsIconized());
	SendToWindow(&e);
	return rc;
}

int FGAPI FGWindow::WindowIconize(void)
{
	if (icon_x==-1 && icon_y==-1)
		return iconize(cApp->GetMouseX()-16, cApp->GetMouseY()-16);
	else
		return iconize(icon_x, icon_y);
}

int FGAPI FGWindow::WindowIconize(int xs, int ys)
{
	return iconize(xs, ys);
}

void FGAPI FGWindow::init(void)
{
	c_menupos = 1;
	state_locked = 0;
	ctrl_counter = 0;
	itself = 0;
	handler = 0;
	CallWindowHook = 0;
	changed = 0;
	iconized = 0;
	icon = 0;
	ypos = 0;
	xpos = 1;
	icon_x = -1;
	icon_y = -1;
	status &= ~(WHIDEN|WLOCKED);
	current_tab_page = "";
	ininput = 0;
	call_handler = 0;
	Buttony = FGControlBoxIterator(0);
}

/**
	Draws a panel. This is an ordinary rectangle with caption.
	@see FGWindow::AddPanel()
	@deprecated
*/
void FGAPI FGWindow::WindowPanel(int x, int y, int a, int b, const char *s, unsigned ink1, unsigned ink2)
{
	AddPanel(x,y,a,b,s,ink1,ink2);
}

void FGAPI FGWindow::RepaintBlock(int x, int y, int w, int h)
{
	RamToVideo(image,x, y, this->GetX()+x, this->GetY()+y,w, h, GetW(), GetH(),
		BITBLIT_COPY, _GSET);
}

FGEditBox* FGAPI FGWindow::FindNextControl(void)
{
	FGControlIterator pt = Buttony->begin();
	FGControl* c;
	bool reached = false;
	
	while ( pt != Buttony->end() )
	{
		c = *pt;
		reached |= (c == ininput);

		if (reached)
		{
			pt++;
			if (pt != Buttony->end() )
			{
				c = *pt;
				if (c->GetType()==EDITBOX && (c->GetStatus()&7) != BDISABLED)
					return (FGEditBox *)c;
				else
					continue;
			}
			else
				return 0;	// at the end
		}
		pt++;
	}
	return 0;
}

void FGWindow::ShowAll(void)
{
	FGWindowIterator start = FGApp::Windows.begin();
	FGWindowIterator end = FGApp::Windows.end();

	for( ; start != end; start++ )
		(*start)->WindowUpdateBlock(0,0, (*start)->GetW(), (*start)->GetH());
}

//! Add current controls to the named tabpage and make one current
bool FGAPI FGWindow::AddTabPage(const char *name)
{
	if ((status & WNOTEBOOK) == 0)
	{
		IError("You must specify WNOTEBOOK window flag before AddTabPage()", 0);
		return false;
	}

	FGControlBoxIterator iter = ctrl_list.begin();

	if (iter->GetString()[0] == 0)
	{
		iter->Rename(name);
	}
	else
	{
		while(iter != ctrl_list.end())
		{
			if (strcmp(iter->GetString(), name) == 0)
			{
				// already exists
				RemoveControls(iter);
				return false;
			}
			++iter;
		}
		FGControlContainer tp(name, yoff-NOTEBOOKH-2);
		Buttony = ctrl_list.insert(ctrl_list.end(), tp);
	}

	// redraw the whole window
	SetTabPage(name);
	return true;
}

//
// Set current named tabpage
//
void FGAPI FGWindow::SetTabPage(const char *name)
{
	FGControlBoxIterator iter = ctrl_list.begin();

	while(iter != ctrl_list.end())
	{
		iter->SetFlag(0);
		const char* str = iter->GetString();

		if (strcmp(str, name) == 0)
		{
			// redraw the whole window
			iter->SetFlag(1);
			current_tab_page = iter->GetString();
			cApp->AutoRepeatEnd();
			c_menupos = 1;
			// fix - force end of input line on Tab Switching
			if (IsInInput())
				WindowInputChar(CR);
			Buttony = iter;
		}
		++iter;
	}
	draw();
	SendEvent(TABSWITCHEVENT, (long)current_tab_page);
}

void FGAPI FGWindow::DeleteTabPages(void)
{
	FGControlBoxIterator iter = ctrl_list.begin();

	while(iter != ctrl_list.end())
	{
		RemoveControls(iter);
		++iter;
	}

	// delete all
	ctrl_list.erase(ctrl_list.begin(), ctrl_list.end());

	FGControlContainer cont("", yoff-NOTEBOOKH-2);
	ctrl_list.push_back( cont );
	Buttony = ctrl_list.begin();
}

void FGAPI FGWindow::DeleteTabPage(const char *name)
{
	FGControlBoxIterator iter = ctrl_list.begin();

	while(iter != ctrl_list.end())
	{
		if (strcmp(iter->GetString(), name) == 0)
		{
			RemoveControls(iter);
			ctrl_list.erase(iter);
			SetTabPage(ctrl_list.begin()->GetString());
			return;
		}
		++iter;
	}
}

int	FGWindow::GetNumberOfTabPages(void)
{
	return ctrl_list.size();
}

//! Redraw bookmarks only
void FGAPI FGWindow::DrawTabPages(void)
{
	int _w = 0;
	int f = get_font();
	set_font(FONTSYSLIGHT);

	FGControlBoxIterator iter = ctrl_list.begin();

	while(iter != ctrl_list.end())
	{
		if (*iter->GetString())
		{
			int _y = iter->GetY();
			int _x = _w+4;
			int sirka = iter->GetWidth()-2;

			box(_x+1, _y+1, sirka-2, NOTEBOOKH-1, paper);
			rect(_x, _y+1, sirka, NOTEBOOKH, 0);
			rect(_x+1, _y, sirka-2, NOTEBOOKH+1, 0);
			box(_x, _y+NOTEBOOKH, sirka+2, 2, 0);

			if (iter->GetFlag() == 0)
			{
				rect(_x, _y+1, sirka, NOTEBOOKH+1,0);
				text(_x+NOTEGAP-2, _y+4,iter->GetString(), ink, paper);
			}
			else
			{
				box(_x+2, _y+2, sirka-4, NOTEBOOKH,paper);
				text(_x+NOTEGAP-2, _y+4,iter->GetString(),CScheme->notebook_active, paper);
			}
		}
		_w += iter->GetWidth();
		++iter;
	}
	set_font(f);
}

//! Test bookmarks for click, and make one current if needed
bool FGAPI FGWindow::ClickTabPage(int xx, int yy)
{
	int _w = 0;
	if ((status & WNOTEBOOK) == 0)
	{
		return false;
	}
	xx -= x;
	yy -= y;

	FGControlBoxIterator iter = ctrl_list.begin();

	// here we go for markers, return TRUE if gets one
	while(iter != ctrl_list.end())
	{
		int _y = iter->GetY();
		int _x = _w+4;
		int sirka = iter->GetWidth();

		if (_x<=xx && _x+sirka>xx)
		{
			if (_y<=yy && _y+NOTEBOOKH+2>yy)
			{
				if (iter->GetFlag() == 0)
				{
					SetTabPage(iter->GetString());
					return true;
				}
				else
					return false;
			}
		}
		_w += sirka;
		++iter;
	}
	return false;
}

void FGAPI FGWindow::SetNextTabPage(void)
{
	if ((status & WNOTEBOOK) == 0)
	{
		return ;
	}

	FGControlBoxIterator iter = ctrl_list.begin();
	// here we go for markers, return TRUE if gets one
	while(iter != ctrl_list.end())
	{
		if (iter->GetFlag() == 1)
		{
			++iter;
			if (iter != ctrl_list.end())
				SetTabPage(iter->GetString());
			else
				SetTabPage(ctrl_list.begin()->GetString());
			return;
		}
		++iter;
	}
	if (ctrl_list.size())   // make fisrt as current if none
		SetTabPage(ctrl_list.begin()->GetString());
}

// relokuj vsetky zalozky
void FGAPI FGWindow::RelocateTabPage(int xx, int yy)
{
	x += xx;
	y += yy;
	if (entryPoint)
		entryPoint->DatabaseRelocate(x,y,name);
	if ((status & WNOTEBOOK) == 0)
	{
		return ;
	}
}

//! Run until window close or mrXXX happen
MODAL_RETURN FGAPI FGWindow::ShowModal(void)
{
	int st = status;
	status |= WMODAL;
	MODAL_RETURN rc = cApp->RunModal(this);
	// fix: 25.5.2006 Why reset???
//	status = st;
	return rc;
}

/**
	Draws an ascii text with TTF font described by FontProperty.
	@param x x position
	@param y y position
	@param s an ascii text string by null
	@param f a font face object
	@param color foreground
	@param bk background
	@note this function depends on truetype fonts functionality provided
	by 'freetype2' library (www.freetype.org). You can enable TTF support
	in the OpenGUI by editing 'config.mak' file. See the installation PDF file.
	@see FGFontProperty
*/
void FGAPI FGWindow::WindowText(int x, int y, const char *s, FGFontProperty * f, unsigned color, unsigned bk)
{
#ifdef FG_TTF
	StateLock();
	FGDrawBuffer *text=0;
	switch(f->GetAlgorithm())
	{
		default:
		case FGFontProperty::ttfSolid:
			text = TTF_RenderText_Solid((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#ifndef BPP8
		case FGFontProperty::ttfShaded:
			text = TTF_RenderText_Shaded((TTF_Font *)f->GetFace(), s, color, bk);
			break;
		case FGFontProperty::ttfBlended:
//			text = TTF_RenderText_Blended((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#endif
	}
	if (text)
	{
		bitblit(x,y,0,0,text->GetW(), text->GetH(), text);
		WindowRepaintUser(x, y, text->GetW(), text->GetH());
		delete text;
	}
	StateUnlock();
#endif
}

/**
	Draws an ascii text with TTF font described by FontProperty.
	@param x x position
	@param y y position
	@param s an UTF8 text string by null
	@param f a font face object
	@param color foreground
	@param bk background
	@note this function depends on truetype fonts functionality provided
	by 'freetype2' library (www.freetype.org). You can enable TTF support
	in the OpenGUI by editing 'config.mak' file. See the installation PDF file.
	@see FGFontProperty
*/
void FGAPI FGWindow::WindowTextUTF8(int x, int y, const char *s, FGFontProperty * f, unsigned color, unsigned bk)
{
#ifdef FG_TTF
	StateLock();
	FGDrawBuffer *text=0;
	switch(f->GetAlgorithm())
	{
		default:
		case FGFontProperty::ttfSolid:
			text = TTF_RenderUTF8_Solid((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#ifndef BPP8
		case FGFontProperty::ttfShaded:
			text = TTF_RenderUTF8_Shaded((TTF_Font *)f->GetFace(), s, color, bk);
			break;
		case FGFontProperty::ttfBlended:
//			text = TTF_RenderUTF8_Blended((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#endif
	}
	if (text)
	{
		bitblit(x,y,0,0,text->GetW(), text->GetH(), text);
		WindowRepaintUser(x, y, text->GetW(), text->GetH());
		delete text;
	}
	StateUnlock();
#endif
}

/**
	Draws an ascii text with TTF font described by FontProperty.
	@param x x position
	@param y y position
	@param s 16bit text wide string terminated by null
	@param f a font face object
	@param color foreground
	@param bk background
	@note this function depends on truetype fonts functionality provided
	by 'freetype2' library (www.freetype.org). You can enable TTF support
	in the OpenGUI by editing 'config.mak' file. See the installation PDF file.
	@see FGFontProperty
*/
void FGAPI FGWindow::WindowTextUnicode(int x, int y, const unsigned short *s, FGFontProperty * f, unsigned color, unsigned bk)
{
#ifdef FG_TTF
	StateLock();
	FGDrawBuffer *text=0;
	switch(f->GetAlgorithm())
	{
		default:
		case FGFontProperty::ttfSolid:
			text = TTF_RenderUNICODE_Solid((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#ifndef BPP8
		case FGFontProperty::ttfShaded:
			text = TTF_RenderUNICODE_Shaded((TTF_Font *)f->GetFace(), s, color, bk);
			break;
		case FGFontProperty::ttfBlended:
//			text = TTF_RenderUNICODE_Blended((TTF_Font *)f->GetFace(), s, color, bk);
			break;
#endif
	}
	if (text)
	{
		bitblit(x,y,0,0,text->GetW(), text->GetH(), text);
		WindowRepaintUser(x, y, text->GetW(), text->GetH());
		delete text;
	}
	StateUnlock();
#endif
}

/**
* Parameters are the same as std::printf()
*/
FGDialog::FGDialog(char* format, ...)
{
	char text[256];
	va_list arglist;
	va_start(arglist, format);
#ifndef _MSC_VER
	vsnprintf(text, 255, format, arglist);
#else
	vsprintf(text, format, arglist);
#endif
	va_end(arglist);

	width = FGFontManager::textwidth(FONTSYSLIGHT, text)+40;

	if (width < 280) width = 280;
	wnd = new FGWindow(&wnd, 0,0, width<GetXRes() ? width : GetXRes(), 120, "Error Dialog Box", 0, CBLACK, CREDLIGHT, WSTANDARD|WCENTRED|WNOPICTO|WUSESELECTEDCONTROL);
	wnd->set_font(FONTSYSLIGHT);
	wnd->WindowText(20,16,text);
}

FGDialog::FGDialog(unsigned ink, unsigned paper, const char* title, char* format, ...)
{
	char text[256];
	va_list arglist;
	va_start(arglist, format);
#ifndef _MSC_VER
	vsnprintf(text, 255, format, arglist);
#else
	vsprintf(text, format, arglist);
#endif
	va_end(arglist);

	width = FGFontManager::textwidth(FONTSYSLIGHT, text)+40;

	if (width < 280) width = 280;
	wnd = new FGWindow(&wnd, 0,0, width<GetXRes() ? width : GetXRes(), 120, title, 0, ink, paper, WSTANDARD|WCENTRED|WNOPICTO|WUSESELECTEDCONTROL);
	wnd->set_font(FONTSYSLIGHT);
	wnd->WindowText(20,16,text,ink,paper);
}

/**
* Returns mrOk on user click.
*/
MODAL_RETURN FGDialog::ShowOk(void)
{
	wnd->RemoveControls( wnd->GetCurrentControls() );
	ctrl = wnd->AddPushButton(width/2-40,48,80,23,"Ok",'o');
	ctrl->SetParameter((void *) mrOk);
	wnd->SetDefaultControl(ctrl);
	return wnd->ShowModal();
}

/**
* Returns mrYes or mrNo on user click.
*/
MODAL_RETURN FGDialog::ShowYesNo(const char* str1, int key1, const char* str2, int key2)
{
	wnd->RemoveControls( wnd->GetCurrentControls() );
	ctrl = wnd->AddPushButton(width/2-80,48,72,23,str1, key1);
	ctrl->SetParameter((void *) mrYes );
	wnd->SetDefaultControl(ctrl);
	ctrl = wnd->AddPushButton(width/2+24,48,72,23,str2, key2);
	ctrl->SetParameter((void *)  mrNo );
	return wnd->ShowModal();
}

/**
* Returns mrRetry, mrIgnore or mrCancel on user click.
*/
MODAL_RETURN FGDialog::ShowRetryIgnoreCancel(void)
{
	wnd->RemoveControls( wnd->GetCurrentControls() );
	ctrl = wnd->AddPushButton(width/4*1-32, 48, 64, 23, "Retry",'r');
	ctrl->SetParameter((void *) (mrRetry));
	wnd->SetDefaultControl(ctrl);
	ctrl = wnd->AddPushButton(width/4*2-32, 48, 64, 23, "Ignore",'i');
	ctrl->SetParameter((void *) (mrIgnore));
	ctrl = wnd->AddPushButton(width/4*3-32, 48, 64, 23, "Cancel",'c');
	ctrl->SetParameter((void *) (mrCancel));
	return wnd->ShowModal();
}

void FGWindow::SetActive(FGControl *c)
{
	call_handler = c;
	c->OnActivate();
}

void FGWindow::RunHandler(void)
{
	if (this)	// is object correct?
	{
		if (call_handler)	// is window set to calling?
		{
			FGControl *c = call_handler;
			char* event_name = 0;

			if (c->GetSignalName())
				event_name = strdup(c->GetSignalName());

			ResetActive(); 			// no more call it

			// this code is added from 5.1.9 for the string based
			// event handler naming
			if (event_name)
			{
				CallClosure(event_name, c);
				// release tmpname
				free(event_name);
			}
			else
			{
				if (c->GetHandler())	// get the address of routine
					c->GetHandler()(c); // and call one with self.
			}

			// for convenience
			fgstate.__hint_x_menu = fgstate.__hint_y_menu = -1;
		}
	}
}

// -----------------------------------------------------------------------------

static ClosureList	closures;
static FGMutex closure_mutex(recursiveMutEx);

FGClosure::FGClosure(const char* n, XUIEventHandler func, void* usr_data )
{
	strncpy(event_name, n, sizeof(event_name));
	event_name[sizeof(event_name)-1] = 0;

	function = func;
	user_data = usr_data;
	type = ON_CLICK;
}

FGClosure::FGClosure(const char* n, XUIEnterHandlerText func, void* usr_data )
{
	strncpy(event_name, n, sizeof(event_name));
	event_name[sizeof(event_name)-1] = 0;

	text_handler = func;
	user_data = usr_data;
	type = ON_TEXT_CHANGE;
}

FGClosure::FGClosure(const char* n, XUIEnterHandlerInteger func, void* usr_data )
{
	strncpy(event_name, n, sizeof(event_name));
	event_name[sizeof(event_name)-1] = 0;

	int_handler = func;
	user_data = usr_data;
	type = ON_INTEGER_CHANGE;
}

FGClosure::FGClosure(const char* n, XUIEnterHandlerDouble func, void* usr_data )
{
	strncpy(event_name, n, sizeof(event_name));
	event_name[sizeof(event_name)-1] = 0;

	dbl_handler = func;
	user_data = usr_data;
	type = ON_DOUBLE_CHANGE;
}

FGClosure::FGClosure(const char* n, XUIWindowHandler hwnd, void* usr_data )
{
	strncpy(event_name, n, sizeof(event_name));
	event_name[sizeof(event_name)-1] = 0;

	handler = hwnd;
	user_data = usr_data;
	type = HANDLER;
}

/**
	A special utility function - generates random text strings - ASCIIZ - (with codes from 32 to 127).
	@param buffer pointer to the allocated user memory
	@param size the size of allocated memory. The string will be (size-1) in size.
	@ingroup fgx
*/
void GetTempName(char* buffer, int size)
{
	int i;
	for(i = 0; i < size-1; i++)
	{
		buffer[i] = ' ' + (rand() % 96);
	}
	buffer[i] = 0;
}

/**
	Registers callback for this signal_name within user data pointer.
	@param signal_name the ASCIIZ string
	@param fnc funcion address (XUIEventHandler type callback)
	@param user_data user data pointer
	@ingroup fgx
*/
void RegisterOnClickSignal(const char* signal_name, XUIEventHandler fnc, void* user_data)
{
	closure_mutex.Lock();
	FGClosure closure(signal_name, fnc, user_data);
	closures.push_back(closure);
	closure_mutex.Unlock();
}

/**
	Registers callback (text EditBox type) for this signal_name within user data pointer.
	@param signal_name the ASCIIZ string
	@param fnc funcion address (XUIEnterHandlerText type callback)
	@param user_data user data pointer
	@ingroup fgx
*/
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerText fnc, void* user_data)
{
	closure_mutex.Lock();
	FGClosure closure(signal_name, fnc, user_data);
	closures.push_back(closure);
	closure_mutex.Unlock();
}

/**
	Registers callback (integer EditBox or Slider type) for this signal_name within user data pointer.
	@param signal_name the ASCIIZ string
	@param fnc funcion address (XUIEnterHandlerInteger type callback)
	@param user_data user data pointer
	@ingroup fgx
*/
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerInteger fnc, void* user_data)
{
	closure_mutex.Lock();
	FGClosure closure(signal_name, fnc, user_data);
	closures.push_back(closure);
	closure_mutex.Unlock();
}

/**
	Registers callback (double EditBox type) for this signal_name within user data pointer.
	@param signal_name the ASCIIZ string
	@param fnc funcion address (XUIEnterHandlerDouble type callback)
	@param user_data user data pointer
	@ingroup fgx
*/
void RegisterOnEnterSignal(const char* signal_name, XUIEnterHandlerDouble fnc, void* user_data)
{
	closure_mutex.Lock();
	FGClosure closure(signal_name, fnc, user_data);
	closures.push_back(closure);
	closure_mutex.Unlock();
}

/**
	Registers FGControl* type object with function within user data pointer.
	@param ctrl object to register
	@param fnc funcion address (XUIEnterHandlerText type callback)
	@param user_data user data pointer
	@see FGControl::AttachSignalName()
	@ingroup fgx
*/
void RegisterControl(FGControl* ctrl, XUIEventHandler fnc, void* user_data)
{
	char name[FGClosure::name_size];

	GetTempName(name, sizeof(name)-1);
	RegisterOnClickSignal(name, fnc, user_data);
	ctrl->AttachSignalName(name);
}

/**
	Registers FGWindow handler type object with function within user data pointer.
	@param handler_name the ASCIIZ string
	@param fnc funcion address (XUIWindowHandler type callback)
	@param user_data user data pointer
	@ingroup fgx
*/
void RegisterWindowHandler(const char* handler_name, XUIWindowHandler fnc, void* user_data)
{
	closure_mutex.Lock();
	FGClosure closure(handler_name, fnc, user_data);
	closures.push_back(closure);
	closure_mutex.Unlock();
}

/**
	Changes the user data pointer for already registere named signal (aka callback) + function address.
	@param signal_name the ASCIIZ string
	@param func funcion address
	@param user_data user data pointer
	@return true if signal is found and data pointer changed.
	@ingroup fgx
*/
bool ChangeSignalData(const char* signal_name, void* func, void* user_data)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	while(start != end)
	{
		if (func == (void*)(*start).function && strcmp(signal_name, (*start).event_name) == 0 )
		{
			(*start).user_data = user_data; ;
			closure_mutex.Unlock();
			return true;
		}
		start++;
	}
	closure_mutex.Unlock();
	return false;
}

/**
	Removes the named signal (aka callback) for function address from the registered events.
	@param signal_name the ASCIIZ string
	@param func funcion address
	@return true if signal is found and removed.
	@ingroup fgx
*/
bool DeregisterSignal(const char* signal_name, void* func)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	while(start != end)
	{
		if (func == (void*)(*start).function && strcmp(signal_name, (*start).event_name) == 0 )
		{
			closures.erase(start);
			closure_mutex.Unlock();
			return true;
		}
		start++;
	}
	closure_mutex.Unlock();
	return false;
}

/**
	Removes the named signal (aka callback) for function address and user data pointer combination from the registered events.
	@param signal_name the ASCIIZ string
	@param func funcion address
	@param data user data pointer
	@return true if signal is found and removed.
	@note use this functon if you have got used more than one event+callback for multiple data instances.
	@ingroup fgx
*/
bool DeregisterSignal(const char* signal_name, void* func, void* data)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	while(start != end)
	{
		if (func == (void*)(*start).function && data == (void*)(*start).user_data && strcmp(signal_name, (*start).event_name) == 0 )
		{
			closures.erase(start);
			closure_mutex.Unlock();
			return true;
		}
		start++;
	}
	closure_mutex.Unlock();
	return false;
}

/**
	@internal
*/
void CallClosure(const char* event_name, CallBack cb)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	if (event_name)
		while(start != end)
	{
		FGClosure& closure = *start;

		if (strcmp(event_name, closure.event_name) == 0)
		{
			if (closure.function)
			{
				switch(closure.type)
				{
					case FGClosure::ON_CLICK:
						closure.function(cb, closure.user_data);
						break;
					case FGClosure::ON_INTEGER_CHANGE:
						closure.int_handler(cb, cb->ToInteger(), closure.user_data);
						break;
					case FGClosure::ON_DOUBLE_CHANGE:
						closure.dbl_handler(cb, cb->ToDouble(), closure.user_data);
						break;
					case FGClosure::ON_TEXT_CHANGE:
						closure.text_handler(cb, cb->ToString(), closure.user_data);
						break;
				}
			}
		}
		start++;
	}
	closure_mutex.Unlock();
}

/**
	@internal
*/
void CallClosure(const char* event_name, CallBack cb, int value)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	if (event_name)
		while(start != end)
	{
		if (strcmp(event_name, (*start).event_name) == 0)
		{
			if ((*start).function)
			{
				switch((*start).type)
				{
					case FGClosure::ON_INTEGER_CHANGE:
						(*start).int_handler(cb, value, (*start).user_data);
						break;
					default:
						assert(!"Closure mismatch!");
				}
			}
		}
		start++;
	}
	closure_mutex.Unlock();
}

/**
	@internal
*/
void CallClosure(const char* event_name, FGEvent* event)
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	if (event_name)
		while(start != end)
	{
		if (strcmp(event_name, (*start).event_name) == 0)
		{
			if ((*start).handler)
				(*start).handler( event, (*start).user_data);
		}
		start++;
	}
	closure_mutex.Unlock();
}

/**
	@internal
*/
void CallClosureDebug()
{
	closure_mutex.Lock();
	ClosureIterator start = closures.begin();
	ClosureIterator end = closures.end();

	while(start != end)
	{
		FGClosure& c = (*start);

		switch((*start).type)
		{
			case FGClosure::ON_CLICK:
				printf("'%s' - [%p] : [%p] - OnClick\n", c.event_name, c.function, c.user_data);
				break;
			case FGClosure::ON_INTEGER_CHANGE:
				printf("'%s' - [%p] : [%p] - OnInteger\n", c.event_name, c.function, c.user_data);
				break;
			case FGClosure::ON_DOUBLE_CHANGE:
				printf("'%s' - [%p] : [%p] - OnDouble\n", c.event_name, c.function, c.user_data);
				break;
			case FGClosure::ON_TEXT_CHANGE:
				printf("'%s' - [%p] : [%p] - OnText\n", c.event_name, c.function, c.user_data);
				break;
			case FGClosure::HANDLER:
				printf("'%s' - [%p] : [%p] - OnEvent\n", c.event_name, c.function, c.user_data);
				break;
			default:
				assert(!"Closure mismatch!");
		}
		start++;
	}
	closure_mutex.Unlock();
}

#ifdef FG_NAMESPACE
}
#endif




