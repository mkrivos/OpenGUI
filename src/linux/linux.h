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


#ifndef __LINUX_H_
#define __LINUX_H_
// ---------------------------------------------------------------------------
//
//		Linux base driver
//
// ---------------------------------------------------------------------------

#include <sys/mman.h>
#include <sys/stat.h>
#include <linux/fb.h>

#include "dll.h"
#include "lrmi.h"
#include "vbe.h"

#define GPMDATADEVICE "/dev/gpmdata"
#define DEV_TYPE_NONE		0
#define DEV_TYPE_GPMDATA	1
#define DEV_TYPE_MOUSE		2
#define DEV_TYPE_PENDATA	3
#define DEV_TYPE_PS2		4
#define DEV_TYPE_MICE		5

#include <asm/mtrr.h>

#if defined(DGA_DRIVER) || defined(X11_DRIVER)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/keysym.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

struct fb_videomode
{
	const char *name;	/* optional */
	unsigned int refresh;	/* optional */
	unsigned int xres;
	unsigned int yres;
	unsigned int pixclock;
	unsigned int left_margin;
	unsigned int right_margin;
	unsigned int upper_margin;
	unsigned int lower_margin;
	unsigned int hsync_len;
	unsigned int vsync_len;
	unsigned int sync;
	unsigned int vmode;
};


/*
 * We maintain a list of WC regions for each physical mapping so they can
 * be undone when unmapping.
 */

struct mtrr_wc_region
{
	struct mtrr_sentry sentry;
	int added;					/* added WC or removed it */
	struct mtrr_wc_region *next;
};

mtrr_wc_region *setWC(unsigned long, unsigned long, int, int);
void undoWC(mtrr_wc_region *);

class FGLinuxDriver : public FGDriver
{
		mtrr_wc_region * mtrr;
		fd_set fdset;
		int maxfd;

	protected:
		int		tty_fd;
		int		tty_no;
		bool	vgamouse, gpmmouse, mice_mouse,
				svgalib, switching,
				dgalib, dgamouse,
				x11lib, x11mouse,
				/* Is VT visible? */
				visible;
		int		n_arguments;
		pid_t	old_pid, new_pid;
		int		devtype;
		FGPixel *sbak;		/* addr of backing store */
		int		uid, gid;

		// jump table to svgalib
		int  (*ptr_vga_init)(void);
		int  (*ptr_vga_setmode)(int);
		int  (*ptr_vga_hasmode)(int);
		vga_modeinfo * (*ptr_vga_getmodeinfo)(int);
		unsigned char * (*ptr_vga_getgraphmem)(void);
		int  (*ptr_vga_getmousetype)(void);
		int  (*ptr_vga_setlinearaddressing)(void);
		void (*ptr_vga_setdisplaystart)(int s);
		int  (*ptr_mouse_init)(char *dev, int type, int samplerate);
		void (*ptr_mouse_close)(void);
		void (*ptr_disable)(void);
		void (*ptr_waitretrace)(void);
		int  (*ptr_mouse_update)(void);
		void (*ptr_mouse_setposition)(int x, int y);
		void (*ptr_mouse_setxrange)(int x1, int x2);
		void (*ptr_mouse_setyrange)(int y1, int y2);
		void (*ptr_mouse_setscale)(int s);
		int  (*ptr_mouse_getx)(void);
		int  (*ptr_mouse_gety)(void);
		int  (*ptr_mouse_getbutton)(void);
		int  (*ptr_vga_lastmodenumber)(void);
		int  *ptr_tty_fd;
		int caps_lock, num_lock;

		int  mouseopen(void);
		void mouseclose(void);
		int  checkmouse(int& type, long& key, int& x, int& y, int& buttons);
		int  GetGPM(int& i, char **argv);
		void execute(char **argv, int argc);
		void RestoreGPM(void);
		void RunGPM(int which);
		int recode_key(int k);

		int  ps2_mouse_init(void);
		int  ps2_mouse_get(int& , long& , int& , int& , int&);
		void ps2_mouse_deinit(void);

		int  mice_mouse_init(void);
		int  mice_mouse_get(int& , long& , int& , int& , int&);
		void mice_mouse_deinit(void);

#if defined(DGA_DRIVER) || defined(X11_DRIVER)
		void process_modifiers(int state, bool pressed);
		int TranslateKey(XKeyEvent *xkey, bool pressed);
#endif
	public:
		// return TRUE if user==root && console is virtual && svgalib.so is available
		virtual int	available(void);
		void linux_postinit(void);
		FGLinuxDriver(char *n);
		~FGLinuxDriver();
		virtual void change_mouse(void)
		{
			if (vgamouse)
			{
				ptr_mouse_setxrange(0,(X_width - 1));
				ptr_mouse_setyrange(0,(Y_width-1));
				ptr_mouse_setposition(X_width/2, Y_width/2);
			}
		}
		virtual int set_mouse(void);
		virtual void reset_mouse(void);
		virtual int set_keyboard(void);
		virtual void reset_keyboard(void);
		virtual int get_event(int& type, long& key, int& x, int& y, int& buttons);
};

// ---------------------------------------------------------------------------
//
//		SVGAlib frontend driver
//
// ---------------------------------------------------------------------------

extern "C" unsigned *set_bank_proc, set_vga_banking;

class FGSVGADriver : public FGLinuxDriver
{
		typedef void (*vv)(void);
		typedef int (*iv)(void);
		typedef void (*vi)(int);
		typedef void (*vii)(int,int);
		typedef int (*ii)(int);

		// handle to lib
		Dll *lib;

	public:
		FGSVGADriver() : FGLinuxDriver("SVGAlib"), lib(0)
		{
			svgalib = 1;
		}
		~FGSVGADriver()
		{
			sync();
			lib = 0;
		}
		virtual int	link(void);
		virtual void get_all_modes(vmode *p);
		virtual int set_mode(int ww, int hh);
		virtual int text_mode(void)
		{
			isgraph = 0;
			if (ptr_vga_setmode) ptr_vga_setmode(0);
			return 1;
		}
		virtual int set_page(int page)
		{
			if ((page+1)*pitch*h > (int)linear_size)
				return 0;
			ptr_vga_setdisplaystart(h*pitch*page);
			ptr_waitretrace();
			return 1;
		}
};

// ---------------------------------------------------------------------------
//
//		FBDEV frontend driver
//
// ---------------------------------------------------------------------------

extern "C" int ioperm(unsigned long int __from, unsigned long int __num, int __turn_on);
extern "C" int iopl(int __level);

class FGFBDriver : public FGLinuxDriver
{
		struct	fb_var_screeninfo vinfo, text_vinfo;
		struct	fb_fix_screeninfo finfo;
		char *	fbname;
		int		fbfd;
		int		fixed_mode;
		int		cttyname;
		int		originaltty;
		struct	vt_mode vtm;

		int  fb_find_mode(struct fb_var_screeninfo *var, unsigned int xres, unsigned int yres, unsigned int refresh);
		int  map(void);
		void unmap(void);
		int  vtopen(void);
		int  vtclose(void);

	public:
		FGFBDriver() : FGLinuxDriver("FBDEV")
		{
			memset(&vinfo, 0, sizeof(vinfo));
			memset(&text_vinfo, 0, sizeof(text_vinfo));
			memset(&finfo, 0, sizeof(finfo));
			memset(&vtm, 0, sizeof(vtm));
			fbfd = 0;
			fbname = 0;
			sbak = 0;
			cttyname = 0;
			fixed_mode = 0;
			originaltty = 0;
		}
		~FGFBDriver()
		{
		}

		int		GetDEV(void) { return fbfd; }
		void vtswitch(int s);

		virtual int	link(void);
		virtual void get_all_modes(vmode *p);
		virtual int set_mode(int ww, int hh);
		virtual int text_mode(void)
		{
			isgraph = 0;
			if (fbfd==-1) return 0;
			unmap();
			if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &text_vinfo) == -1)
			{
				/* Don't quit, because the sparc crashes then. */
				perror("Put variable screen settings failed");
			}
			vtclose();
			close(fbfd);
			return 0;
		}
		virtual int set_page(int page)
		{
			vinfo.yoffset = h*page;
			vinfo.xoffset = 0;

			if (ioctl(fbfd, FBIOPAN_DISPLAY, &vinfo) == -1)
			{
				return 0;
			}
			return 1;
		}
};

#ifdef DGA_DRIVER
// ---------------------------------------------------------------------------
//
//		DGADEV frontend driver
//
// ---------------------------------------------------------------------------

#include <X11/extensions/xf86dga.h>

class FGDGADriver : public FGLinuxDriver
{
		// handle to lib
		XDGAMode *modes;
		Colormap DGA_colormap;
		static Display *DGA_Display;
		XDGADevice *mode;
		int DGA_event_base;
		int num_modes;

		static int fillbox(int,int,int,int, FGPixel ink, unsigned ppop);
	public:
		FGDGADriver() : FGLinuxDriver("DGA 2.x") //, lib(0)
		{
			dgalib = 1;
			caps_lock = 0;
			num_lock = 1;
			mode = 0;
			DGA_Display = 0;
			DGA_colormap = 0;
			modes = 0;
			DGA_event_base = 0;
			num_modes = 0;
		}
		~FGDGADriver()
		{
			if (modes) XFree(modes);
			modes = 0;
		}
		virtual int	available(void);
		virtual int	link(void);
		virtual void get_all_modes(vmode *p);
		virtual int set_mode(int ww, int hh);
		virtual int text_mode(void);
		virtual int set_mouse(void);
		virtual void reset_mouse(void);
		virtual int set_keyboard(void);
		virtual void reset_keyboard(void);
		virtual int get_event(int& type, long& key, int& x, int& y, int& buttons);
		virtual int set_page(int page);
};
#endif // DGA_DRIVER

#ifdef X11_DRIVER
// ---------------------------------------------------------------------------
//
//		X11 DEV frontend driver
//
// ---------------------------------------------------------------------------

#define FGL_Root		RootWindow(thedisplay, thescreen)

class FGX11Driver : public FGLinuxDriver
{
		// handle to lib
		Display *thedisplay;
		Window thewindow;
		int thescreen;
		Visual *thevisual;
		GC thecontext;
		XImage *theimage;
		XImage *theimage2;
		
		/* MIT shared memory extension information */
	    int use_mitshm;
	    XShmSegmentInfo shminfo;
	    XShmSegmentInfo shminfo2;

		Colormap xcmap;
		Colormap thecolormap;
		XColor xcolours[256];
		unsigned long pixels[256];

		int theeventbase;
		int num_modes;
		int stage;
		bool on_the_fly_change;
		
		static int remap_ppop(int ppop)
		{
			static int fnc[9]={GXcopy, GXxor, GXand, GXor, GXcopy,GXcopy,GXinvert,GXcopy, GXcopy};
			return fnc[ppop];
		}

		static int x11_line(int x1, int y1, int x2, int y2, FGPixel ink, unsigned ppop)
		{
			XGCValues val;
			// map FGL to X
			FGX11Driver * drv = (FGX11Driver *)__fg_driver;
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
			FGX11Driver * drv = (FGX11Driver *)__fg_driver;
			// set new PPOP
			val.function = remap_ppop(ppop);
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);

			XSetForeground(drv->thedisplay, drv->thecontext, color);
			XDrawPoint(drv->thedisplay, drv->thewindow, drv->thecontext, x1,y1);
			// return to normal
			val.function = GXcopy;
			XChangeGC(drv->thedisplay, drv->thecontext, GCFunction, &val);
			__draw_point(fgl::videobase,x1,y1,X_virtual,color,ppop);
		}
		static void x11_vector_clip_rect(int x, int y, int w, int h)
		{
			XRectangle rect={x,y,w-x,h-y};
			FGX11Driver * drv = (FGX11Driver *)__fg_driver;
			// set new
			XSetClipRectangles(drv->thedisplay, drv->thecontext, 0,0,&rect,1,Unsorted);
		}
		void X11RefreshDisplay(void)
		{
			UpdateRect(0,0,0,0,w,h);
		}
		void setsize(int ww, int hh);
		static void palette(unsigned n, unsigned rgb)
		{
			FGX11Driver *d = (FGX11Driver *)__fg_driver;
			d->xcolours[n].red   = (rgb>>8) & 0xff00;
			d->xcolours[n].green =   rgb     & 0xff00;
			d->xcolours[n].blue  = (rgb<<8) & 0xff00;
			d->xcolours[n].flags = DoRed | DoGreen | DoBlue;
			XStoreColor(d->thedisplay, d->xcmap, &d->xcolours[n]);
			XInstallColormap(d->thedisplay, d->xcmap);
		}
		void try_mitshm(int ww, int hh);
	public:
		virtual void UpdateRect(int x, int y, int xm, int ym, int w, int h);

		FGX11Driver() : FGLinuxDriver("X Windows Rel. 11") //, lib(0)
		{
			x11lib = 1;
			caps_lock = 0;
			num_lock = 1;
			thedisplay = 0;
			thecolormap = 0;
			use_mitshm = true;
			thewindow=0;
			thevisual=0;
		}
		~FGX11Driver()
		{
			if (stage >= 2)
			{
				XDestroyImage(theimage);
				if ( use_mitshm )
				{
					XShmDetach(thedisplay, &shminfo);
					XSync(thedisplay, false);
					shmdt(shminfo.shmaddr);

					if (on_the_fly_change)
					{
						XDestroyImage(theimage2);

						XShmDetach(thedisplay, &shminfo2);
						XSync(thedisplay, false);
						shmdt(shminfo2.shmaddr);
					}
				}
#ifdef INDEX_COLORS
				XFreeColors(thedisplay, xcmap, pixels, 256, 0);
				XFreeColormap(thedisplay, xcmap);
#endif
				XFreeGC(thedisplay, thecontext);
				videobase = 0;
			}
		}
		virtual int	available(void);
		virtual int	link(void);
		virtual void get_all_modes(vmode *);
		virtual int set_mode(int ww, int hh);
		virtual int set_mouse(void);
		virtual void reset_mouse(void);
		virtual int set_keyboard(void);
		virtual void reset_keyboard(void);
		virtual int get_event(int& type, long& key, int& x, int& y, int& buttons);
		static int x11_cursor(int,int,int a, int&)
		{
			return !a;
		}
		virtual void SetCaption(char *new_name);
};
#endif // X11_DRIVER

#ifdef VESA_DRIVER
class FGVesaDriver : public FGLinuxDriver
{
		static const int VESA_REGS_SIZE = 16384;
		struct
		{
			struct vbe_info_block *info;
			struct vbe_mode_info_block *mode;
		} vesa_data;
		void* LRMI_mem1;
		struct LRMI_regs vesa_r;
		int vesa_memory, vesa_chiptype;
		int vesa_last_mode_set;
		int vesa_init(int force, int par1, int par2);

	public:
		FGVesaDriver();
		~FGVesaDriver();

		virtual int	link(void);
		virtual void get_all_modes(vmode *p);
		virtual int set_mode(int ww, int hh);
		virtual int text_mode(void);
		virtual int set_page(int page);
};
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif /* _linux_H_ */
