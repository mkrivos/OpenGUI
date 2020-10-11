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
    solaris.cc - Linux support routines

*/

#define __USE_GNU
#include "config.h"
#include "fgbase.h"
#include "fgevent.h"
#include "_fastgl.h"

#include "drivers.h"
#include "solaris.h"
#include <sys/wait.h>

#if defined(__sun__) || defined (__sun)

#ifdef FG_NAMESPACE
namespace fgl {
#endif

//void (*ptr_vga_setpage)(int);

static int recode[] = {
PRTRSC,	0xff61,
BACKSP,	0x8,
KUP,	0xff52,
KDOWN,	0xff54,
KRIGHT,	0xff53,
KLEFT,	0xff51,

INSERT,	0xff63,
DEL,	0xffff,
HOME,	0xff50,
END,	0xff57,
PGUP,	0xff55,
PGDOWN,	0xff56,

ALT_A,	0x3061,
ALT_B,	0x3062,
ALT_C,	0x3063,
ALT_D,	0x3064,
ALT_E,	0x3065,
ALT_F,	0x3066,
ALT_G,	0x3067,
ALT_H,	0x3068,
ALT_I,	0x3069,
ALT_J,	0x306a,
ALT_K,	0x306b,
ALT_L,	0x306c,
ALT_M,	0x306d,
ALT_N,	0x306e,
ALT_O,	0x306f,
ALT_P,	0x3070,
ALT_Q,	0x3071,
ALT_R,	0x3072,
ALT_S,	0x3073,
ALT_T,	0x3074,
ALT_U,	0x3075,
ALT_V,	0x3076,
ALT_W,	0x3077,
ALT_X,	0x3078,
ALT_Y,	0x3079,
ALT_Z,	0x307a,

F01,	0xffbe,
F02,	0xffbe +1,
F03,	0xffbe +2,
F04,	0xffbe +3,
F05,	0xffbe +4,
F06,	0xffbe +5,
F07,	0xffbe +6,
F08,	0xffbe +7,
F09,	0xffbe +8,
F10,	0xffbe +9,
F11,	0xffbe +10,
F12,	0xffbe +11,

ALT_F01,0xcfbe,
ALT_F02,0xcfbe +1,
ALT_F03,0xcfbe +2,
ALT_F04,0xcfbe +3,
ALT_F05,0xcfbe +4,
ALT_F06,0xcfbe +5,
ALT_F07,0xcfbe +6,
ALT_F08,0xcfbe +7,
ALT_F09,0xcfbe +8,
ALT_F10,0xcfbe +9,
ALT_F11,0xcfbe +10,
ALT_F12,0xcfbe +11,

CTRL_F01,0xbfbe,
CTRL_F02,0xbfbe +1,
CTRL_F03,0xbfbe +2,
CTRL_F04,0xbfbe +3,
CTRL_F05,0xbfbe +4,
CTRL_F06,0xbfbe +5,
CTRL_F07,0xbfbe +6,
CTRL_F08,0xbfbe +7,
CTRL_F09,0xbfbe +8,
CTRL_F10,0xbfbe +9,
CTRL_F11,0xbfbe +10,
CTRL_F12,0xbfbe +11,

ALT_UP,		0xcf52,
ALT_DOWN,	0xcf54,
ALT_RIGHT, 	0xcf53,
ALT_LEFT,	0xcf51,

ALT_TAB,	0xcf09,

ALT_INSERT,	0xcf63,
ALT_DEL,	0xcfff,
ALT_HOME,	0xcf50,
ALT_END,	0xcf57,
ALT_PGUP,	0xcf55,
ALT_PGDOWN,	0xcf56,

CTRL_UP,	0xbf52,
CTRL_DOWN,	0xbf54,
CTRL_RIGHT, 0xbf53,
CTRL_LEFT,	0xbf51,

CTRL_TAB,	0xbf09,

CTRL_INSERT,0xcf63,
CTRL_DEL,	0xcfff,
CTRL_HOME,	0xcf50,
CTRL_END,	0xcf57,
CTRL_PGUP,	0xcf55,
CTRL_PGDOWN,0xcf56,
-1, -1
};

static int shm_error;
static int (*X_handler)(Display *, XErrorEvent *) = NULL;
static int shm_errhandler(Display *d, XErrorEvent *e)
{
        if ( e->error_code == BadAccess ) {
        	shm_error = 1;
        	return(0);
        } else
		return(X_handler(d,e));
}

void delay(unsigned cnt)
{
	usleep(cnt*1000);
}

void Snd(int a, int b)
{
//	if (kbd_fd>=0) ioctl(kbd_fd, KDMKTONE, (b<<16)+((1193190/a)&0xffff));
}

int FGSolarisDriver::recode_key(int k)
{
	int *p=recode+1;
	while(*p!=-1)
	{
		if (*p==k) return *(p-1);
		p += 2;
	}
	return k;
}

int	FGSolarisDriver::available(void)
{
		const char *display;
		Display *dpy;
		int available=0;

		/* The driver is available if the display is local
		*/
	/* Check to see if we are root and stdin is a virtual console */
	uid = geteuid();
	gid = getegid();
	setuid(0);
	setgid(0);
	root = 0;
	display = NULL;
		stage = 0;
		if ( (strncmp(XDisplayName(display), ":", 1) == 0) ||
		     (strncmp(XDisplayName(display), "unix:", 5) == 0) )
		{
			dpy = XOpenDisplay(display);
			if ( dpy )
			{
				XCloseDisplay(dpy);
				available = 1;
			}
		}
		return available;
}

int FGSolarisDriver::TranslateKey(XKeyEvent *xkey)
{
	KeySym xsym;
	int keysym=0x8000, ctrl=0,
	// 1=shift, 4=ctrl, 8=alt
	offset = (xkey->state&1?0x2000:0)+(xkey->state&8?0x3000:0)+(xkey->state&4?0x4000:0);

	xsym = XLookupKeysym(xkey, 0);
	if (xsym==0) return keysym;

	switch (xsym)
	{
		case XK_Caps_Lock:
			caps_lock = !caps_lock;
			break;
		case XK_Num_Lock:
			num_lock = !num_lock;
			break;
	}

	switch (xsym>>8)
	{
		case 0x00:	/* Latin 1 */
		case 0x01:	/* Latin 2 */
		case 0x02:	/* Latin 3 */
		case 0x03:	/* Latin 4 */
		case 0x04:	/* Katakana */
		case 0x05:	/* Arabic */
		case 0x06:	/* Cyrillic */
		case 0x07:	/* Greek */
		case 0x08:	/* Technical */
		case 0x0A:	/* Publishing */
		case 0x0C:	/* Hebrew */
		case 0x0D:	/* Thai */
			keysym = (xsym&0xFF);
			/* Map capital letter syms to lowercase */
			if ((keysym >= 'a')&&(keysym <= 'z')&&!(xkey->state&0x0C))
			{
				if (caps_lock==1) // if CAPS, complement
					keysym ^= 0x20;
				if (xkey->state&1==1) // if shift, complement
					keysym ^= 0x20; // togle letter case
				break;
			}

			if (xkey->state&1) switch(xsym) //shift + standard key
			{
				case '1':
					ctrl = '!';
					break;
				case '2':
					ctrl = '@';
					break;
				case '3':
					ctrl = '#';
					break;
				case '4':
					ctrl = '$';
					break;
				case '5':
					ctrl = '%';
					break;
				case '6':
					ctrl = '^';
					break;
				case '7':
					ctrl = '&';
					break;
				case '8':
					ctrl = '*';
					break;
				case '9':
					ctrl = '(';
					break;
				case '0':
					ctrl = ')';
					break;
				case '-':
					ctrl = '_';
					break;
				case '=':
					ctrl = '+';
					break;
				case '\\':
					ctrl = '|';
					break;
				case '`':
					ctrl = '~';
					break;

				case '[':
					ctrl = '{';
					break;
				case ']':
					ctrl = '}';
					break;
				case ';':
					ctrl = ':';
					break;
				case '\'':
					ctrl = '\"';
					break;
				case ',':
					ctrl = '<';
					break;
				case '.':
					ctrl = '>';
					break;
				case '/':
					ctrl = '?';
					break;
			}
			if (ctrl==0) keysym += offset; // add state key
			else keysym = ctrl;
			break;
//		case 0xFE:
		case 0xFF:
			if (offset==0) // no state key
			{
				if (((unsigned)xsym>=0xff08U && (unsigned)xsym<=0xff0dU) || xsym==0xff1b)
				{
					keysym = xsym&0xFF; // code 1..1f
					break;
				}
			}
			keysym = (xsym-offset)&0xffff;
			break;
		default:
			return 0x8000;
	}
//sprintf(__gps, "Translating key %x %x <- (%x %x %x):%d\n", xsym, keysym, xkey->type, xkey->keycode, xkey->state,caps_lock);
	return keysym;
}

void FGSolarisDriver::try_mitshm(int ww, int hh)
{
	if(!use_mitshm) return;

	shminfo.shmid = shmget(IPC_PRIVATE, areasize(ww,hh), IPC_CREAT | 0777);

	if ( shminfo.shmid >= 0 )
	{
		shminfo.shmaddr = (char *)shmat(shminfo.shmid, 0, 0);
		shminfo.readOnly = False;
		if ( shminfo.shmaddr != (char *)-1 )
		{
			shm_error = False;
			X_handler = XSetErrorHandler(shm_errhandler);
			XShmAttach(thedisplay, &shminfo);
			XSync(thedisplay, true);
			XSetErrorHandler(X_handler);
			if (shm_error)
				shmdt(shminfo.shmaddr);
		} else
		{
			shm_error = True;
		}
		shmctl(shminfo.shmid, IPC_RMID, NULL);
	}
	else
	{
		shm_error = true;
	}
	if ( shm_error )
		use_mitshm = 0;
}

int	FGSolarisDriver::link(void)
{
	const char *display;

	/* Open the X11 display */
	display = NULL;		/* Get it from DISPLAY environment variable */

	thedisplay = XOpenDisplay(display);

	if ( thedisplay == NULL )
	{
		printf("Couldn't open X11 display\n");
		return 0;
	}

	thescreen = DefaultScreen(thedisplay);
	thecolormap = DefaultColormap(thedisplay, thescreen);
	int depth = DefaultDepth(thedisplay, thescreen);

	if (depth==24)
		depth=32;
	if ( depth != FASTGL_BPP)
	{
		printf("Unsuported color depth %d BPP by X11 server\n", bpp*8);
		return 0;
	}

	/* Determine the current screen depth */
	thevisual = DefaultVisual(thedisplay, thescreen);

	stage = 1;
	subname = "standard X11 mode\n";
	return true;
}

void FGSolarisDriver::get_all_modes(vmode *)
{
	total_modes = 1;		// set only one virtual videomode [window with this size]
}

void FGSolarisDriver::setsize(int ww, int hh)
{
	XSizeHints *hints;

	hints = XAllocSizeHints();
	if ( hints )
	{
		hints->min_width = hints->max_width = ww;
		hints->min_height = hints->max_height = hh;
		hints->flags = PMaxSize | PMinSize;
		XSetWMNormalHints(thedisplay, thewindow, hints);
		XFree(hints);
	}
}

int FGSolarisDriver::set_mode(int ww, int hh)
{
	XEvent anevent;
	XTextProperty textproperty;
	char * windowname="OpenGUI Application", **__name=&windowname;

	synch = 0;
   	mode_num = 0;		// force mode 0
	pitch = bpp;

	islinear = 1;
	fgstate.palette_8=1;

	if (thewindow == 0)
	{
	 /* Create the window */
	 thewindow = XCreateSimpleWindow(thedisplay, FGL_Root, 0, 0, ww, hh, 0, CBLACK, CBLACK);
	 XSelectInput(thedisplay, thewindow, StructureNotifyMask);
	 XMapWindow(thedisplay, thewindow);

	 /* Label the window */
	 XStringListToTextProperty(__name, 1, &textproperty);
	 XSetWMName(thedisplay, thewindow, &textproperty);
	 XSetStandardProperties(thedisplay, thewindow, windowname, windowname, None, NULL, 0, NULL);

	 /* Get the general context */
	 thecontext = XCreateGC(thedisplay, thewindow, 0, NULL);
	 XSetBackground(thedisplay, thecontext, CBLACK);
	 XSetForeground(thedisplay, thecontext, CBLACK);

	 /* Wait for the MapNotify event */
	 for (;;)
	 {
		  XNextEvent(thedisplay, &anevent);
		  if (anevent.type == MapNotify)
				break;
	 }

	 // Set event types
	 XSelectInput(thedisplay, thewindow,
					  ExposureMask |
					  ButtonPressMask |
					  ButtonReleaseMask |
					  PointerMotionMask |
					  KeyPressMask |
//					  StructureNotifyMask |
//					  SubstructureNotifyMask |
					  EnterWindowMask |
					  LeaveWindowMask |
					  FocusChangeMask |
					  PropertyChangeMask);

	 // Erase the display (In the background colour), bring to top
	 XClearWindow(thedisplay, thewindow);
	 XMapRaised(thedisplay, thewindow);

	 XSetLineAttributes(thedisplay, thecontext, 1, LineSolid, CapButt, JoinRound);
	}
	else
	{
		XDestroyImage(theimage);
		if ( use_mitshm )
		{
			XShmDetach(thedisplay, &shminfo);
			XSync(thedisplay, false);
			shmdt(shminfo.shmaddr);
		}
		setsize(ww,hh);
		XResizeWindow(thedisplay, thewindow, ww, hh);
	}
	XWindowAttributes theattr;
	XGetWindowAttributes(thedisplay, thewindow, &theattr);
	virt_w =  req_w = ww = theattr.width;
	virt_h =  req_h = hh = theattr.height;

	try_mitshm(ww,hh);

	if(use_mitshm)
	{
    	// videobase is already set
		linear_base = videobase = (FGPixel *)shminfo.shmaddr;
		theimage = XShmCreateImage(thedisplay, thevisual,
						(bpp == 4) ? 24 : bpp * 8,
					 	ZPixmap,
					    shminfo.shmaddr, &shminfo,
					    ww, hh);
		if(!theimage)
		{
			XShmDetach(thedisplay, &shminfo);
			XSync(thedisplay, false);
			shmdt(shminfo.shmaddr);
        }
	}
	else
	{
		linear_base = videobase = (FGPixel *)malloc(areasize(ww,hh));
		theimage = XCreateImage(thedisplay, thevisual,
			  (bpp == 4) ? 24 : bpp * 8,
			  ZPixmap, 0,
			  (char *)videobase,
			  ww, hh,
			  bpp * 8,
			  0);
		if (theimage == 0)
			return 0;
	}
	/* Create a colormap if necessary */
#ifdef INDEX_COLORS
    int n;

	/* Create new colormap */
	memset(xcolours, 0, 256*sizeof(XColor));
	memset(pixels, 0, 256*sizeof(long));

	xcmap = XCreateColormap(thedisplay, thewindow, DefaultVisual(thedisplay, thescreen), AllocNone);

	/* Get all colours in default colormap */
    for (n = 0; n < 256; n++) xcolours[n].pixel = (long) n;

    /* Allocate colours in new colormap */
    if (!XAllocColorCells(thedisplay, xcmap, true, NULL, 0,  pixels, 256))
        printf("Failed to allocate colour map info\n");

	// install new handler
	vector_palette = palette;
#endif
	accel_name = "XFree86 XAA accelerator";
	stage = 2;
	vector_mouse_cursor = x11_cursor;
	vector_draw_line = x11_line;
	vector_draw_point = x11_point;
	vector_clip_rect = x11_vector_clip_rect;
	ops = 8+2;
	setsize(ww,hh);
	return postinit();
}

int FGSolarisDriver::set_mouse(void)
{
	mousex = oldx = w/2;
	mousey = oldy = h/2;
	mouse_string = "X11_MOUSE";
	return x11mouse = 1;
}

void FGSolarisDriver::reset_mouse(void)
{
	ismouse = x11mouse = 0;
}

int FGSolarisDriver::set_keyboard(void)
{
	return iskeyboard = 1;
}

void FGSolarisDriver::reset_keyboard(void)
{
	iskeyboard = 0;
}

//
// CURSOROUTEVENT ???
//
int FGSolarisDriver::get_event(int& type, int& key, int& x, int& y, int& buttons)
{
//	static int ss=100;
	XEvent xevent;

	// return if no events available
	if (XPending(thedisplay)==0) { return 0;}

	// get event from fifo
	XNextEvent(thedisplay, (XEvent *)&xevent);

	switch (xevent.type)
	{
	    /* Mouse motion? */
		case ButtonRelease:
			but &= ~(1<<(xevent.xbutton.button-1));
			goto mouse;
		case ButtonPress:
			but |= 1<<(xevent.xbutton.button-1);
			goto mouse;
	    case MotionNotify:
			mousex = xevent.xmotion.x;
			if (mousex<0) mousex = 0;
			if (mousex>GetXRes()) mousex = GetXRes();
			mousey = xevent.xmotion.y;
			if (mousey<0) mousey = 0;
			if (mousey>GetYRes()) mousey = GetYRes();

mouse:
			buttons = 0;
			if (but&16) buttons |= FG_BUTTON_WHEEL_DOWN;
			if (but&8)  buttons |= FG_BUTTON_WHEEL_UP;
			if (but&4) buttons |= FG_BUTTON_RIGHT; // right
			if (but&2) buttons |= FG_BUTTON_MIDDLE; // middle
			if (but&1) buttons |= FG_BUTTON_LEFT; // left
			x = mousex;
			y = mousey;
			key  = 0;
			type = MOUSEEVENT;
			return 1;

	    case KeyPress:
			key = recode_key(TranslateKey(&xevent.xkey));
			if (key==0x8000) return false;
			type = KEYEVENT;
			x = y = buttons = 0;
			return 1;
	    case Expose:
			if ( xevent.xexpose.count == 0)
			{
				X11RefreshDisplay();
#ifdef INDEX_COLORS
				XInstallColormap(thedisplay, xcmap);
#endif
            }
		    break;
	    case NoExpose:
			if ( xevent.xexpose.count == 0)
			{
				X11RefreshDisplay();
#ifdef INDEX_COLORS
				XUninstallColormap(thedisplay, xcmap);
#endif
            }
		    break;
#ifdef INDEX_COLORS
//	    case LeaveNotify:
		case FocusOut:
			XUninstallColormap(thedisplay, xcmap);
		    break;
//		case EnterNotify:
	    case FocusIn:
			XInstallColormap(thedisplay, xcmap);
		    break;
#endif
	}
	return false;
}

void FGSolarisDriver::SetCaption(char *new_name)
{
	XTextProperty textproperty;
	char **__name=&new_name;

	/* Label the window */
	XStringListToTextProperty(__name, 1, &textproperty);
	XSetWMName(thedisplay, thewindow, &textproperty);
	XSetStandardProperties(thedisplay, thewindow, new_name, new_name, None, NULL, 0, NULL);
}


#ifdef FG_NAMESPACE
}
#endif
#endif // sun
