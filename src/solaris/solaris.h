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
    
    Framebuffer library headers and prototypes 
*/

#if (defined __sun__) || (defined __sun) || (defined sun)

#ifndef __SOLARIS_H_
#define __SOLARIS_H_
// ---------------------------------------------------------------------------
//
//		solaris base driver
//
// ---------------------------------------------------------------------------

#include <sys/mman.h>
#include <sys/stat.h>

#include "dll.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/keysym.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

#ifdef FG_NAMESPACE
namespace fgl {
#endif

// ---------------------------------------------------------------------------
//
//		X11 DEV frontend driver
//
// ---------------------------------------------------------------------------

#define FGL_Root		RootWindow(thedisplay, thescreen)

class FGSolarisDriver : public FGDriver
{
	protected:
		// handle to lib
		Display *thedisplay;
		Window thewindow;
		int thescreen;
		Visual *thevisual;
		Colormap thecolormap;
		GC thecontext;
		XImage *theimage;
        Colormap xcmap;
        XColor xcolours[256];
	    /* MIT shared memory extension information */
	    int use_mitshm;
	    XShmSegmentInfo shminfo;
        unsigned long pixels[256];
		unsigned dgalib:1, dgamouse:1,
				x11lib:1, x11mouse:1;
		int		uid, gid;


		int caps_lock, num_lock;

		int  mouseopen(void);
		void mouseclose(void);
		int  checkmouse(int& type, int& key, int& x, int& y, int& buttons);
		int recode_key(int k);
		int TranslateKey(XKeyEvent *xkey);
		static void palette(unsigned n, unsigned rgb)
		{
			FGSolarisDriver *d = (FGSolarisDriver *)__fg_driver;
            d->xcolours[n].red   = (rgb>>8) & 0xff00;
            d->xcolours[n].green =   rgb     & 0xff00;
            d->xcolours[n].blue  = (rgb<<8) & 0xff00;
            d->xcolours[n].flags = DoRed | DoGreen | DoBlue;
	        XStoreColor(d->thedisplay, d->xcmap, &d->xcolours[n]);
			XInstallColormap(d->thedisplay, d->xcmap);
		}
		int theeventbase;
		int num_modes;
		int stage;

		static int remap_ppop(int ppop)
		{
			static int fnc[9]={GXcopy, GXxor, GXand, GXor, GXcopy,GXcopy,GXinvert,GXcopy, GXcopy};
			return fnc[ppop];
		}

		static int x11_line(int x1, int y1, int x2, int y2, FGPixel ink, unsigned ppop)
		{
			XGCValues val;
			// map FGL to X
			FGSolarisDriver * drv = (FGSolarisDriver *)__fg_driver;
			// set new PPOP
			val.function = remap_ppop(ppop);
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);

			XSetForeground(drv->thedisplay, drv->thecontext, ink);
			XDrawLine(drv->thedisplay, drv->thewindow, drv->thecontext, x1,y1,x2,y2);
			// return to normal
			val.function = GXcopy;
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);
// thread
			drv->vram_line(x1,y1,x2,y2,ink,ppop);
			return 1;
		}
		static void x11_point(FGPixel*, int x1, int y1, int ww, FGPixel color, unsigned ppop)
		{
			XGCValues val;
			// map FGL to X
			FGSolarisDriver * drv = (FGSolarisDriver *)__fg_driver;
			// set new PPOP
			val.function = remap_ppop(ppop);
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);

			XSetForeground(drv->thedisplay, drv->thecontext, color);
			XDrawPoint(drv->thedisplay, drv->thewindow, drv->thecontext, x1,y1);
			// return to normal
			val.function = GXcopy;
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);
			__draw_point(::videobase,x1,y1,X_virtual,color,ppop);
		}
		static void x11_vector_clip_rect(int x, int y, int w, int h)
		{
			FGSolarisDriver * drv = (FGSolarisDriver *)__fg_driver;
			// set new
			XRectangle rect={x,y,w-x,h-y};
// crash under Solaris 8 and OpenWin !!!!
			XSetClipRectangles(drv->thedisplay, drv->thecontext, 0,0,&rect,1,Unsorted);
		}
		void X11RefreshDisplay(void)
		{
			UpdateRect(0,0,0,0,w,h);
		}
		void setsize(int ww, int hh);
		void try_mitshm(int ww, int hh);
	public:
		virtual void UpdateRect(int x, int y, int xm, int ym, int w, int h)
		{
			if (use_mitshm)
				XShmPutImage(thedisplay, thewindow, thecontext, theimage, x,y, xm,ym, w,h, 0);
			else
				XPutImage(thedisplay, thewindow, thecontext, theimage, x,y, xm,ym, w,h );
			XFlush(thedisplay);
		}
		// return TRUE if user==root && console is virtual && svgalib.so is available
		virtual int	available(void);
		virtual int	link(void);
		virtual void get_all_modes(vmode *);
		virtual int set_mode(int ww, int hh);
		virtual int set_mouse(void);
		virtual void reset_mouse(void);
		virtual int set_keyboard(void);
		virtual void reset_keyboard(void);
		virtual int get_event(int& type, int& key, int& x, int& y, int& buttons);
		static int x11_cursor(int,int,int a, int &)
		{
			return !a;
		}
		FGSolarisDriver() : FGDriver("X Windows Release 11")
		{
			but = 0;
			caps_lock = 0;
			num_lock = 1;
			thedisplay = 0;
			thecolormap = 0;
			thewindow = 0;
			thescreen = 0;
			thevisual = 0;
			thecolormap =0;
			thecontext=0;
			theimage=0;
			use_mitshm=1;
		}
		~FGSolarisDriver()
		{
			if (stage >= 2)
			{
				XDestroyImage(theimage);
				if ( use_mitshm )
				{
					XShmDetach(thedisplay, &shminfo);
					XSync(thedisplay, false);
					shmdt(shminfo.shmaddr);
				}
#ifdef INDEX_COLORS
				XFreeColors(thedisplay, xcmap, pixels, 256, 0);
				XFreeColormap(thedisplay, xcmap);
#endif
				videobase = 0;
			}
		}
		virtual void SetCaption(char *new_name);
};

#ifdef FG_NAMESPACE
}
#endif

#endif /* _solaris_H_ */
#endif // sun
