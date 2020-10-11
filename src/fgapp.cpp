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
	Get & translate user events
*/

/* TODO :
	Zakladna schema je vygenerovat nejaku udalost, tu posla??? oknu,
	pripadne i aplikacii via handler ak nejaky existuje.
	Nova schema by mala dodrzat staru a pridat pretazenie
	urcitych definovanych sprav. Tieto by mali rozne
	parametre. Pretazenie by zaroven odstavilo posielanie
	konkretnej spravy via handler. Podobne by sa mal spravat
	i aplikacny handler.
*/

#include <signal.h>

#if defined( __linux__) || defined( __rtems__) || defined(__sun__) || defined (__sun)
#include <sched.h>
#ifdef __rtems__
#include <rtems/kd.h>
#else
#include <sys/kd.h>
#endif
#include <sys/ioctl.h>
#endif

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
#include "_fastgl.h"
#include "drivers.h"

#ifdef FG_TTF
#include "fgttf.h"
#endif

#ifdef FG_NAMESPACE
namespace fgl {
#endif

#ifdef _WIN32
#ifndef __CYGWIN__
extern void sched_yield(void);
#endif
#endif

#define		REPEAT_DELAY1			22
#define		REPEAT_DELAY2			4
#define		DRAG_TRESHOLD		    8

FGControl *FGApp::idcb = 0;
FGControl *FGApp::idc2 = 0;
int FGApp::fulldrag = 1;
int FGApp::startXdrag =	0;
int FGApp::startYdrag =	0;
int FGApp::endXdrag	= 0;
int FGApp::endYdrag	= 0;
int FGApp::current_mouse_x=0, FGApp::current_mouse_y=0;
int FGApp::__fg_fps=0, FGApp::__fg_fps_total=0;
const FGMouseCursor * FGApp::__fg_cursor = idc_normal;
int FGApp::repeat_status=0, FGApp::repeat_delay1=REPEAT_DELAY1, FGApp::repeat_delay2=REPEAT_DELAY2;
static FGPixel m_save[MSZ*MSZ];
void (*FGApp::DragShape)(int , int ,int	, int ) = 0;
int FGApp::ctrl_break=0;
FGPixel FGApp::__p4[8] = {0, 0, 0, 0, 1, 1, 1, 1};
FGPattern FGApp::__patt(__p4, 8, 1);
FGApp *cApp;
ConfigInterface *cCfg;
FGWindowContainer FGApp::Windows;
FGWindow* FGApp::Current = 0;
FGInputDevice* FGApp::user_input_drv = 0;

void 			(*FGApp::DelayProc) (void) = 0;
void 			(*FGApp::OnEverySecond)(int) = 0;
FGEvent 		FGApp::queque[MAX_EVENT_QUEQUE];
MainHwnd 		FGApp::appHandler;
int 			FGApp::quequeIndex = 0;
char			FGApp::currdir[pathsize];
char*			FGApp::homedir = 0;
FGWindow*		FGApp::Root = 0;
int				FGApp::flags;
int				FGApp::video;
int				FGApp::Argc;
int				FGApp::background;
char**			FGApp::Argv;
char*		   	FGApp::name;
unsigned long 	FGApp::ticks;
int 			FGApp::ttf_support = 0;
bool FGApp::IsShift(void) { return FGDriver::is_shift; }
bool FGApp::IsCtrl(void) { return FGDriver::is_ctrl; }
bool FGApp::IsAlt(void) { return FGDriver::is_alt; }


//! Set caption when application running in windowed mode (X11 or WIN32 GDI).
void FGApp::SetCaption(char *new_name)
{
	__fg_driver->SetCaption(new_name);
}

FGWindowIterator FGApp::GetIterator(FGWindow* Colise)
{
	FGWindowIterator e = Windows.end();
	for (FGWindowIterator i = Windows.begin(); i != e; i++)
		if ( (*i) == Colise)
			return i;
	return Windows.end();
}

FGWindowRIterator FGApp::GetRIterator(FGWindow* Colise)
{
	FGWindowRIterator e = Windows.rend();
	for (FGWindowRIterator i = Windows.rbegin(); i != e; i++)
		if ( (*i) == Colise)
			return i;
	return Windows.rend();
}

FGWindow* FGApp::GetCurrentWindow(void)
{
	if (NumberOfWindow() == 0)
		return 0;
	else if (Current && Current->GetType() == FGDrawBuffer::ROOTWINDOW)
		return 0;

	return Current;
}

/**
	internal
*/
void FGAPI FGApp::DestroyWindow(void)
{
	int nWnd = NumberOfWindow();

	if (nWnd && fgstate.verbose)
		::printf("WARNING: %d window(s) not closed at exit\n", nWnd);
}

int	FGAPI FGApp::_over(int xa1, int ya1, int xa2, int ya2, int xx, int yy, int ww, int hh)
{
	register int xb1 = xx, xb2 = xx + ww, yb1 = yy, yb2 = yy + hh;

	if ((xa1 <= xb1 && xa2 >= xb1) || (xa1 <= xb2 && xa2 >= xb2))	// x sa prekryva
	{
		if (ya1 <= yb1 && ya2 >= yb1)
		{
			return 1;
		}
		if (ya1 <= yb2 && ya2 >= yb2)
		{
			return 2;
		}
		if (yb1 <= ya1 && yb2 >= ya1)
		{
			return 3;
		}
		if (yb1 <= ya2 && yb2 >= ya2)
		{
			return 4;
		}
	}
	else if ((xb1 <= xa1 && xb2 >= xa1) || (xb1 <= xa2 && xb2 >= xa2))
	{
		if ((yb1 <= ya1 && yb2 >= ya1))
		{
			return 5;
		}
		if ((yb1 <= ya2 && yb2 >= ya2))
		{
			return 6;
		}
		if (ya1 <= yb1 && ya2 >= yb1)
		{
			return 7;
		}
		if (ya1 <= yb2 && ya2 >= yb2)
		{
			return 8;
		}
	}
	return 0;
}

//! params are real size & position of cleared window
void FGAPI FGApp::intersect(FGWindow* This, int _x, int _y, int _w, int _h)
{
	FGWindowIterator p = Windows.begin(), end = Windows.end();
	int rc;

	if (!(cApp->flags&APP_ROOTWINDOW))
		fill_box(_x,_y,_w,_h,cApp->background,_GSET);

	// now we trace all windows from down to *this*
	while (p != end)
	{
		FGWindow* ww = (*p);
		if (! (ww->status & WHIDEN) && ww != This)
		{
			rc = _over(ww->x, ww->y, ww->x+ww->w, ww->y+ww->h, _x, _y, _w, _h);
			if (rc)
			{
				ww->WindowUpdateBlock(_x-ww->GetX(), _y-ww->GetY(), _w, _h);
			}
		}
		p++;
	}
}

//! Test overlaping window, called from OdkryteOkno only
FGWindow* FGApp::OverSprite(FGWindowRIterator This, FGWindowRIterator _od, int xx, int yy, int ww, int hh)
{
	register int xa1, xa2, ya1, ya2;
	FGWindow *v = 0;
	FGWindow *od;
	int xb1 = xx, xb2 = xx + ww, yb1 = yy, yb2 = yy + hh;

	while ( _od != This)
	{
		od = (*_od);

		if (!(od->status & WHIDEN))
		{
			xa1 = od->x,
				xa2 = od->x + od->w,
				ya1 = od->y,
				ya2 = od->y + od->h;

			if ((xa1 <= xb1 && xa2 >= xb1) ||
				(xa1 <= xb2 && xa2 >= xb2))	// x sa prekryva
			{
				if ((ya1 <= yb1 && ya2 >= yb1) ||
					(ya1 <= yb2 && ya2 >= yb2))
				{
					v = od;
					break;
				}
				else if (yb1 <= ya1 && yb2 >= ya2)
				{
					v = od;
					break;
				}
			}
			else if ((xb1 <= xa1 && xb2 >= xa1) ||
				(xb1 <= xa2 && xb2 >= xa2))
			{
				if ((yb1 <= ya1 && yb2 >= ya1) ||
					(yb1 <= ya2 && yb2 >= ya2))
				{
					v = od;
					break;
				}
				else if (ya1 <= yb1 && ya2 >= yb2)
				{
					v = od;
					break;
				}
			}
		}
		_od++;
	}
	return v;
}

//! Called from WindowRepaint() for the window overlap testing
FGWindow * FGAPI FGApp::OdkryteOkno(FGWindowRIterator This, FGWindowRIterator pokial, int x, int y, int w, int h)
{
	FGWindow *Colise=0;

	if (! (*This)->IsVisible())
		return (FGWindow *)-1;	// je skryte

	if (NumberOfWindow() < 2)
		return 0;				// je aktivne

	while ( pokial != This)
	{
		if ((Colise = OverSprite(This,pokial,x, y, w, h)) != 0)
		{
			return Colise;
		}
		pokial++;  // fixme:  was --,
	}
	return 0;
}

//! Returns the FGWindow for that the events has been emited.
FGWindow* FGApp::WindowFind(FGEvent * e)
{
	FGWindowRIterator start = FGApp::Windows.rbegin();
	FGWindowRIterator end = FGApp::Windows.rend();

	for( ; start != end; start++ )
	{
		FGWindow* p = *start;
		if ((p->x <= e->x) && ((p->x + p->w) >= e->x))
		{
			if ((p->y <= e->y) && ((p->y + p->h) >= e->y))
				if (p->IsVisible() && (p->type != FGDrawBuffer::ROOTWINDOW) )
					return p;
		}
	}
	return 0;
}

//! Finds the FGWindow by ID.
FGWindow* FGApp::WindowFind(int idw)
{
	FGWindowIterator start = FGApp::Windows.begin();
	FGWindowIterator end = FGApp::Windows.end();

	for (; start != end; start++)
	{
		FGWindow* p= *start;

		if (p->GetId() == idw)
			return p;
	}
	return 0;
}

/**
	Construct the instance of applicaton object. There is allowed only
	one instantion per process only. The following steps are executed at the constructor time:
-	if possible, the window database  is accessed. This contains last sizes and positions for registered windows
-	if possible, the application configuration is loaded from appname.rc file
-	command line parameters are parsed, and arguments from the list of system parameters are extracted (consult  the "reserved parameters" chapter for details)
-	test for the availability of MMX processor and type of graphics card is performed
-	the screen is switched to graphics mode
-	if 8-bit colors are compiled/used, the color palette management is initialized
-	screen is cleared
-	mouse is checked and, if present, it is passed to the system
-	call to SetColorFuzzy() is performed
-	the ROOTWINDOW is created if needed
-	reset the Timer and FGClock() routines
-	release the root UID on LINUX or Solaris
-	global variables cApp and cCfg are set.
-	load the default mouse cursor
-	init TrueType font system (if enabled in compile time)

	AppFlags values - should be OR-ed together:

-	APP_WINDOWDATABASE - enable saving the size and position to the window database
-	APP_CFG	- enable configuration file management
-	APP_MAGNIFIER  - enable mouse drags with rubber-band rectangle selection
-	APP_ROOTWINDOW - enable drawing to the root window - the whole screen
-	APP_ENABLEALTX - enable termination by <ALT+X> key press
-	APP_LOCALDIR - all Config files will be placed in current directory instead in the HOMEDIR

	@param m number of the graphics mode (valid mode number values are described bellow).
	Look for graph_set_mode function.
	@param argv parameter is standard "C" command line parameter
	@param argc parameter is standard "C" command line parameter
	@param bck defines the initial color of the screen background.
	@param appFlags set of a bit flags that defines the behaviour of your
	application. The the meaning of bit flags is described in the table below.

*/
FGApp::FGApp(int m, int &argc, char **&argv, int bck, int appFlags)
{
	volatile int curr = 0, par = 0;
	char *apn=argv[0], *apn2, **cmdl=argv+1;

	current_mouse_x = 0;
	current_mouse_y = 0;
	current_mouse_buttons = 0;
	old_mouse_x = 0;
	old_mouse_y = 0;
	old_mouse_buttons = 0;
	window_drag_in_action = 0;
	button_reached = 0;

	locked = false;
	mouseflag = -1;
	mousefirst = 1;
	video = m;
	window_resize_in_action = 0;
	selection_steps = 0;
	background = bck;
	flags = appFlags;
	hold_event = new FGEvent(BUTTONHOLDEVENT);
	user_input_drv = 0;

	if (cApp) IError("Multiple FGApp class not allowed !", 1);
	cApp = this;
	homedir = 0;
	if ((flags&APP_LOCALDIR) == 0)
		homedir = getenv("HOME");
#ifndef _MSC_VER
	getcwd(currdir, 127);
#else
	_getcwd(currdir, 127);
#endif
	if (homedir==NULL) homedir = ".";
	apn2 = apn;
	apn = apn+strlen(apn)-1;
	if (strcmp(apn-3, ".exe")==0 || strcmp(apn-3, ".EXE")==0)
	{
		apn = apn-4;
		apn[1]=0;
	}
	while ((*apn!='/' && *apn!='\\' && *apn!=':') && apn2<=apn) apn--;
	apn++;
	name = strdup(apn);
#ifdef __linux__
	setenv("EXENAME", name, 1);
#endif

	char nn[128];
#if defined (_WIN32)
	sprintf(nn, "%s.rc", apn);
#else
	if (homedir)
		sprintf(nn, "%s/.%s.rc", homedir, apn);
	else
		sprintf(nn, ".%s.rc", apn);
#endif

	cCfg = new Config(nn, !!(flags&APP_CFG));
	cCfg->ReadInt("default_videomode", video);

	if (argc > 1)
	{
		while (curr<(argc-1))
		{
			curr++;
			if (argv[curr][0] == '-') switch (tolower(argv[curr][1]))
			{
				case 'p':
					if (!memcmp(&argv[curr][1], "ps2", 3))
					{
						fgstate.force_ps2 = 1;
						if (fgstate.verbose) printf("PS2 fallback mouse initialization!\n");
						par ++;
					}
					else *cmdl++ = argv[curr];
					break;
				case 'm':
					if (!memcmp(&argv[curr][1], "mic", 3))
					{
						fgstate.force_mice = 1;
						if (fgstate.verbose) printf("DEVMOUSE fallback mouse initialization!\n");
						par ++;
					}
					else *cmdl++ = argv[curr];
					break;
				case 'r':
					if (!memcmp(&argv[curr][1], "res", 3))
					{
						curr++;
						if (argc)
							video = argv[curr][0] - '0';
						par += 2;
					}
					else *cmdl++ = argv[curr];
					break;
				case 'd':
					if (!memcmp(&argv[curr][1], "dac", 3))
					{
						fgstate.force_dac8 = 1;
						par ++;
					}
					else
						*cmdl++ = argv[curr];
					break;
				case 's':
#ifdef __linux__
					if (!memcmp(&argv[curr][1], "svg", 3))
					{
						fgstate.nofb = 1;	// no detect FRAMEBUFFER
						fgstate.nodga = 1;
						par++;
					}
					else if (!memcmp(&argv[curr][1], "syn", 3))
					{
						curr++;
						if (argc)
							fgstate.synch = atoi(&argv[curr][0]);
						par += 2;
					}
			   else
#endif
					   if (!memcmp(&argv[curr][1], "saf", 3))
					{
						video = G640x480;
	                  	fgstate.verbose = true;
						flags &= ~APP_WINDOWDATABASE;
						if (fgstate.verbose) printf("Safe mode entered ..\a\n");
						par++;
					}
					else *cmdl++ = argv[curr];
					break;
#ifndef __QNX__
				case 'g':
					if (!memcmp(&argv[curr][1], "geo", 3))
					{
						curr++;
						if (argc>1)
						{
							fgstate.__force_x = atoi(&argv[curr++][0]);
							fgstate.__force_y = atoi(&argv[curr][0]);
							video 	  = GCUSTOM;
							fgstate.windowed_mode = true;
							par+=3;
						} else par++;
					}
					else *cmdl++ = argv[curr];
					break;
#endif
#ifdef _WIN32
				case 'w':
					if (!memcmp(&argv[curr][1], "win", 3))
					{
						fgstate.windowed_mode = true;
						par++;
					}
					else *cmdl++ = argv[curr];
					break;
#endif
				case 'n':
					if (!memcmp(&argv[curr][1], "nofu", 4))
					{
						fulldrag = 0;
						if (fgstate.verbose) printf("Full window drag enabled\n");
						par++;
					}
					else if (!memcmp(&argv[curr][1], "noa", 3))
					{
						fgstate.noaccel = 1;
						if (fgstate.verbose) printf("HW acceleration disabled\n");
						par++;
					}
					else
#ifdef __linux__
						if (!memcmp(&argv[curr][1], "nox", 3))
					{
						fgstate.nofb = 1;	// no detect FRAMEBUFFER
						fgstate.nox11 = 1;
						par++;
					}
					else if (!memcmp(&argv[curr][1], "nod", 3))
					{
						fgstate.noaccel = 1;
						if (fgstate.verbose) printf("DGA disabled\n");
						par++;
					}
					else if (!memcmp(&argv[curr][1], "nofb", 4))
					{
						fgstate.nofbset = 1;
						if (fgstate.verbose) printf("FB mode set disabled\n");
						par++;
					}
					else
#endif
						*cmdl++ = argv[curr];
					break;
				case 'h':
					printf("\nOpenGUI %s - (c) Copyright 1996,2008 Marian Krivos (nezmar@atlas.sk)\n\n", FG_VERSION);
#ifdef FG_THREADED
					printf(" threading model: posix threads\n");
#else
					printf(" threading model: single\n");
#endif
					printf(" FGAPI = '%s'\n", APISTR);
					printf(" supported image file formats: BMP GIF PCX TGA");
#ifdef FG_JPEG
					printf(" JPG");
#endif
#ifdef FG_PNG
					printf(" PNG");
#endif
#ifdef FG_TIFF
					printf(" TIFF");
#endif
#ifdef FG_TTF
					printf(" TRUETYPE");
#endif
					printf("\n");
#ifdef __linux__
					printf(" backends: FRAMEBUFFER SVGALIB");
#ifdef X11_DRIVER
					printf(" X11");
#endif
#ifdef DGA_DRIVER
					printf(" DGA2");
#endif
#endif
					printf(" with %d BPP\n", FASTGL_BPP);
#ifdef _WIN32
					printf(" backend: M$_DirectX");
#endif
#ifdef __GNUC__
				    printf(" Compiled with: GNU C++\n");
#endif
#ifdef __ICC
					printf(" Compiled with: Intel C++\n");
#endif
#ifdef __BORLANDC__
					printf(" Compiled with: Borland C++\n");
#endif
					printf("\n   -resolution N\n");
					printf("          :force scr. resolution 'N', where 'N' = \n");
					printf("           1=320x200, 2=640x480, 3=800x600, 4=1024x768, 5=1280x1024\n");
					printf("           6=1600x1200\n");
					printf(" -svga    :don't use FBDEV driver\n");
					printf(" -nodga   :don't use DGA2 driver\n");
					printf(" -nox11   :don't use X11 driver\n");
					printf(" -nofbset :use default fb mode\n");
					printf(" -sync    :monitor refresh\n");
					printf(" -geometry:set custom resolutions x y\n");
					printf(" -noaccel :don`t use HW accelerations\n");
					printf(" -nofull  :use faster window moving\n");
					printf(" -verbose :verbose output\n");
					printf(" -safe    :safe mode\n");
					printf(" -dac8    :force 8bit DAC/palette\n");
					printf(" -ps2     :force PS/2 mouse driver (LINUX)\n");
					printf(" -mice    :force DEVMOUSE mouse driver (LINUX)\n");
					printf(" -window  :run in windowed mode (WIN32)\n");
					getchar();
					par++;
					break;
				case 'v':
					if (!memcmp(&argv[curr][1], "ver", 3))
					{
						fgstate.verbose = 1;
						printf("\nOpenGUI %s - (c) Copyright 1996,2006 Marian Krivos\n", FG_VERSION);
#ifdef _REENTRANT
						printf("		with multi-threading\n");
#endif
						par++;
					}
					else *cmdl++ = argv[curr];
					break;
				default:
					*cmdl++ = argv[curr];
					break;
			}
			else *cmdl++ = argv[curr];
		}
	}
	argc -= par;
	Argv = argv;
	Argc = argc;

	if (flags&APP_WINDOWDATABASE) entryPoint = new WindowDatabase(apn);
	if (video < 1 || video > GCUSTOM)
		video = 1;

	if (!graph_set_mode(video))
	{
		printf("Fatal error: can't set graphics mode %d\n\a", video);
		exit(-2);
	}
	XX = X_width / 2;
	YY = Y_width / 2;
	SetColorFuzzy(3);
	if (background!=255)
		clear_frame_buffer(background);
	if (m>=4)
		idc_normal = &IDC_NORMAL_LARGE;
	else
		idc_normal = &IDC_NORMAL_SMALL;
	CursorLoad(idc_normal);
	SetDelayProc(0);
#ifndef _WIN32
	SetTimerProc(0, 1000);
#endif
	if (flags&APP_ROOTWINDOW) Root = new FGWindow(&Root, background);
#if ( !defined( __MSDOS__ ) && !defined( __rtems__ ) ) && !defined(_WIN32)
	setuid(getuid());
	setgid(getgid());
	seteuid(getuid());
	setegid(getgid());
#endif
#if defined( __linux__ ) || defined( __rtems__ )
	FGTimer::TimerInit();
#endif
#ifdef FG_TTF
	if ( TTF_Init() < 0 )
		fprintf(stderr, "Couldn't initialize TTF\n");
	else
	{
		ttf_support = 1;
		atexit(TTF_Quit);
	}
#endif
}

static void ClrScr(int ako)
{
	switch(ako)
	{
		default:
		case 0:
			{
				for(int i=0; i<(Y_width-1); i++)
				{
					draw_line(i,i,(X_width - 1), i,0,_GSET);
					draw_line(i,i,i, (Y_width-1),0,_GSET);
					if (i%32 ==0) delay(1);
				}
			}
			break;
	}
}

/**
This is called when the application terminates. It performs the following steps:
-	resets the app timer
-	resets color palette
-	closes windows database
-	closes config file
-	resets the windowing system
-	call system's 'sync()' to flus all disk operations
*/
FGApp::~FGApp()
{
#if defined( __linux__ ) || defined( __rtems__ )
	FGTimer::TimerQuit();
#endif
	if (flags&APP_ENABLEEFECTS)
		ClrScr(0);
	else
		clear_frame_buffer(CBLACK);
	SetDelayProc(0);
#ifndef _WIN32
	SetTimerProc(0);
#endif
#ifdef INDEX_COLORS
	_set_default_palette();
#endif
	delete cCfg;
	cCfg = 0;
	if (Root) delete Root;
	Root = 0;
	DestroyWindow();
	if (flags&APP_WINDOWDATABASE) delete entryPoint;
	entryPoint = 0;
	free(name);
	cApp = 0;
#if !defined(__MSDOS__) && !defined(__rtems__) && !defined(_WIN32)
	sync();
#endif
	delete hold_event;
	hold_event = 0;
}

/**
	Peeks the user input, translate to EVENT if any and push this EVENT to the
	event list. You can use this procedure to allowing mouse move when system
	is busy in your code and don't respond to user. By example, when some
	progressbar is running.
*/
void FGAPI FGApp::FGYield(void)
{
	FGEvent ev(0);
	GetUserEvent(ev, 1);	// do while ALT+X no pressed
}

/**
	When you create the instance of the FGApp class and you sets callback for "DelayProc" ,
	it is a right time to call the main application loop. System starts processing
	the user input (keyboard & mouse) until you call AppDone().
	The mouse's movements are automatically convert to move its pointer
	at the screen and you don't care about it. When you press
	left button at the FGControl (by example FGPushButton), system detects it, and sends right event
	to the appropriate window. When you press button on an empty area of
	the window, this one will get event CLICKLEFTEVENT. This events are send to the application procedure,
	when one is defined.
	@param hwnd the pointer at this mysterious procedure.
*/
void FGAPI FGApp::Run(MainHwnd hwnd)
{
	FGEvent *e=queque, event, tmpevent;

	if (hwnd)
		appHandler = hwnd;

	for (;fgstate.shut_down == 0;)
	{
		GetUserEvent(event, -1);	// do while ALT+X no pressed
		TranslateUserEvent(event);
		for (; quequeIndex;)
		{
			quequeIndex--;
			tmpevent = queque[quequeIndex];
			e = &tmpevent;

			// call event handler
			if (appHandler)
				appHandler(e);

			// call event methods
			switch(e->GetType())
			{
				case KEYEVENT:
					OnKeyPress(e->key);
					break;
				case MOVEEVENT:
					OnMouseMove(e->x, e->y);
					break;
				case CLICKLEFTEVENT:
					OnClick(e->x, e->y);
					break;
				case DBLCLICKLEFTEVENT:
					OnDoubleClick(e->x, e->y);
					break;
				case CLICKRIGHTEVENT:
					OnContextPopup(e->x, e->y);
					break;
				case CURSOROUTEVENT:
					OnCursorOut(e->key);
					break;
				case STARTDRAGLEFTEVENT:
					OnStartDrag(1, e->x, e->y);
					break;
				case STARTDRAGRIGHTEVENT:
					OnStartDrag(0, e->x, e->y);
					break;
				case DRAGLEFTEVENT:
					OnEndDrag(1, e->x, e->y, e->w, e->h);
					break;
				case DRAGRIGHTEVENT:
					OnEndDrag(0, e->x, e->y, e->w, e->h);
					break;
				case TABSWITCHEVENT:
					OnTabSwitch((const char *)e->key);
					break;
				case QUITEVENT:
					fgstate.shut_down = 1;
				case CLOSEEVENT:
					return; // force ret if QUITEVENT
			}
			e->type = NOEVENT;
		}
	}
}

/**
	Run modal this FGWindow until closed or any FGControl.
	@param which your FGWindow with the 'OK' and 'Cancel'
	buttons by example.
	@return one of predefined values (mrClose, mrQuit or mrNone)
*/
MODAL_RETURN FGAPI FGApp::RunModal(FGWindow *which)
{
	FGEvent *e=queque, event;

	fgstate.shut_down = 0;

	for ( ; fgstate.shut_down == 0 ; )
	{
		GetUserEvent(event, -1);	// do while ALT+X no pressed

		TranslateUserEvent(event);

		if (quequeIndex)
		{
			quequeIndex = 0;
			e = &event;

			// call event handler
			if (appHandler)
				appHandler(e);

			// call event methods
			switch(e->GetType())
			{
				case ACCELEVENT:
					if (which == e->wnd)
						if (e->accel->GetType()==FGDrawBuffer::PUSHBUTTON && e->accel->GetParameter() != (void *) mrNone)
						{
							long rc = (long)e->accel->GetParameter();
							return (MODAL_RETURN) (rc);
						}
					break;
//					return mrUnknown;
				case QUITEVENT:
					fgstate.shut_down = 1;
				case CLOSEEVENT:
					return mrQuit; // force ret if QUITEVENT
			}
			e->type = NOEVENT;
		}

		// Ooops, our window is gone
		if (GetCurrentWindow() != which)
			return mrClose;

	}
	return mrNone;
}

void FGApp::Timer(void)
{
	static unsigned int last=0,now=0;
	now = FGClock();
	if (last>now)
	{
		last = now;
		return;
	}
	if (now >= (last+ticks) || last>now)
	{
		last = now;
		__fg_fps_total = __fg_fps;
		__fg_fps = 0; // zero FPS counter
		if (OnEverySecond)
		{
			OnEverySecond(now/1000);
		}
		else OnTimer(now/1000);
	}
}

//! add Window to the list, where = { FRONT, BACK }
void FGApp::AddWindowToList(FGWindow* wnd, int where)
{
	FGWindow::WindowListMutex.Lock();

	if (where & WTOP)
		Windows.push_back(wnd);
	else if (where & WBACK)
	{
		FGWindowIterator start = Windows.begin();
		FGWindowIterator end = Windows.end();

		for( ; start != end; start++ )
		{
			FGWindow* p = *start;
			if ((p->status & WBACK) == 0) break;
		}

		if (start == end)
			Windows.push_back(wnd);
		else
			Windows.insert(start, wnd);
	}
	else
	{
		FGWindowIterator start = Windows.begin();
		FGWindowIterator end = Windows.end();

		for( ; start != end; start++ )
		{
			FGWindow* p = *start;
			if (p->GetStatus() & WTOP) break;
		}

		if (start == end)
			Windows.push_back(wnd);
		else
			Windows.insert(start, wnd);
	}
	SetCurrentWindow(wnd);

	FGWindow::WindowListMutex.Unlock();
}

void FGApp::RemoveIterator(FGWindow* wnd)
{
	FGWindow::WindowListMutex.Lock();

	Windows.erase( GetIterator(wnd) );

	FGWindow::WindowListMutex.Unlock();
}

void FGAPI FGApp::SendToApp(FGEvent	*x)
{
	if (quequeIndex<MAX_EVENT_QUEQUE)
	{
		memcpy(queque+quequeIndex++,x,sizeof(FGEvent));
	}
	else IError("Message queque is full!",0);
}

/*
//! try support of MMX for your CPU
int FGApp::test_mmx(void)
{
	// Kylix3 compiler bug!!!
	// (when val is not static)
	fgstate.__fgl_mmx = 0;
#ifndef __LP64__
#ifdef __BORLANDC__
	static int val = 0;
		asm {
				push	eax
				push	ebx
				push	ecx
				push	edx
				pushfd
				pop		eax
				mov		edx,eax
				xor		eax,0x200000
				push	eax
				popfd
				pushfd
				pop		eax
				cmp		eax,edx
				mov		eax,0
				jz		mmx2
				mov		eax,1
				cpuid
				test	edx,0x800000
				mov		eax,0
				jz		mmx2
				inc		eax	// 1
				emms
				emms
mmx2:			mov		[val],eax
				pop		edx
				pop		ecx
				pop		ebx
				pop		eax
		};
		fgstate.__fgl_mmx = val;
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
	__asm
	(
		// Check for CPUID instruction
		"\tpushl %%ebx\n"              	//; save EBX for fpic
		"\tpushf\n"              	//; save EFLAGS
		"\tpop %%eax\n"             	//; store EFLAGS in EAX
		"\tmov %%eax, %%edx\n"        	//; save in EBX for later testing
		"\txor $0x00200000, %%eax\n"  	//; toggle bit 21
		"\tpush %%eax\n"            	//; put to stack
		"\tpopf\n"               	//; save changed EAX to EFLAGS
		"\tpushf\n"              	//; push EFLAGS to TOS
		"\tpop %%eax\n"             	//; store EFLAGS in EAX
		"\tcmp %%edx, %%eax\n"        	//; see if bit 21 has changed
		"\tjz NO_ATHLON\n"        	//; if no change, no CPUID

		// Check for extended functions
		"\tmov $0x80000000, %%eax\n"  	//; query for extended functions
		"\tCPUID\n"               	//; get extended function limit
		"\tcmp $0x80000000, %%eax\n"  	//; is 8000_0001h supported?
		"\tjbe NO_ATHLON\n"       	//; if not, 3DNow! command set not supported

		"\tmov $0x80000001, %%eax\n"  	//; setup extended function 1
		"\tCPUID\n"               	//; call the function
		"\ttest $0x80000000, %%edx\n" 	//; test bit 31
		"\tjz NO_ATHLON\n"        	//; 3DNow! command set not supported
		"\tmov $1, %%eax\n"
		"\tjmp DONE\n"
"NO_ATHLON:\n"
		"\txor %%eax, %%eax\n"
"DONE:\n"
		"\tpopl %%ebx\t\n \tmov %%eax, %0\n"         //; store to bAthlon
		// optimizer hints
		: "=r" (fgstate.__fgl_mmx)          // let compiler choose output register
		:                           // no input selection for compiler
		: "eax", "ecx", "edx"	    // modifies
	);
#endif
#endif // lp64
	return fgstate.__fgl_mmx;
}

void FGApp::set_mmx(void)
{
	if (fgstate.__fgl_mmx & 1)
	{
		fgstate.mmx_state++;
#ifdef __BORLANDC__
		asm emms;
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
		asm("emms");
#endif
	}
}

void FGApp::reset_mmx(void)
{
	if (fgstate.__fgl_mmx & 1)
	{
		fgstate.mmx_state--;
#ifdef __BORLANDC__
		asm emms;
#endif
#if (defined(__linux__) && defined(__GNUC__)) || defined(__ICC)
		asm("emms");
#endif
	}
}
*/

/**
	Save the whole screen to the image file in the current directory,
	name is created as current time in HEX form.
	This function is called by application automagically when
	you press PRINT_SCREEN key.
*/
void FGApp::SaveScreen(void)
{
	FGBitmap *a =	new FGBitmap(X_width, Y_width);
	char s[20];
	time_t num = time(0);
	get_block(0,0,X_width,Y_width,a->GetArray());
	sprintf(s,"%x",(unsigned int)num);
	if (fgstate.verbose) printf("Bitmap '%s' saved\n", s);
	a->BitmapSave(s, true);
	delete a;
}

/**
	Use this for fine calibration for HOLD_EVENT event.
*/
void FGApp::SetRepeatDelay(int c1, int c2)
{
	if (c1>0 &&	c1<2000)
		repeat_delay1 = c1;
	if (c2>0 &&	c2<500)
		repeat_delay2 = c2;
}

/**
	Test keyboard immediately.
	Try peek any KEY from the INPUT and	return 0 if none, or its keycode.
*/
int FGApp::get_key(void)
{
	FGEvent ev(0);

	GetUserEvent(ev, 1);

	if (ev.GetType()==KEYEVENT)
		return ev.GetKey();

	else return 0;
}

// start autorepeat routine
void FGApp::AutoRepeatStart(FGControl *id)
{
	hold_event->key = id->GetId();
	hold_event->accel = id;
	hold_event->guiType = id->GetType(); // od buttonu typ ...
	repeat_status =	1;
	AutoRepeatDo();
	repeat_status =	REPEAT_DELAY1;
}

// process -||-
void FGApp::AutoRepeatDo(void)
{
	if (--repeat_status	<= 0)
	{
		if (GetCurrentWindow())
			GetCurrentWindow()->SendToWindow(hold_event);
		repeat_status =	REPEAT_DELAY2;
	}
}

// end autorepeat routine
void FGApp::AutoRepeatEnd(void)
{
	repeat_status =	0;
}

/**
	Return coordinates of last DRAG&DROP mouse operation.
*/
void FGApp::GetDragVector(int &a, int &b, int &c, int &d)
{
	a =	startXdrag;
	b =	startYdrag;
	c =	endXdrag;
	d =	endYdrag;
}

/**
	Override current shape that is drawed on the screen
	when D&D operation is performed.
*/
void FGApp::SetDragShape(void (*fnc)(int, int, int, int))
{
	DragShape = fnc;
}

void FGApp::GetUserEvent(FGEvent& EVENT, int waits)
{
	int	xx2, yy2, rc = 0;
	xx2	= XX;
	yy2	= YY;

	EVENT.type = 0;

	if (mouseflag == 1)
		vector_mouse_cursor(XX, YY, true, mouseflag);

	for (; waits; waits--)
	{
		if (locked == false)
		{
			rc = __fg_driver->get_event(EVENT.type,EVENT.key,EVENT.x,EVENT.y,EVENT.buttons);
			// try extra input if available
			if (rc == 0)
				if (user_input_drv)
				{
					rc = user_input_drv->GetInputEvent(EVENT.type,EVENT.key,EVENT.x,EVENT.y,EVENT.buttons);
					if (rc)
						user_input_drv->Scale(EVENT.x, EVENT.y);
//printf("(%d) t = %d, x = %d, y = %d, butt = %d\n", rc, EVENT.type, EVENT.x, EVENT.y, EVENT.buttons);
				}
		}

#ifdef FG_MOUSEKEYS
		if (EVENT.type == KEYEVENT)
		{
			static int last=0, akcel=2, cbut=0;
			int doit=0;

			if (last != EVENT.key)	// reset accel
				akcel = 2;

			switch(EVENT.key)
			{
				case FG_MOUSEKEYS_UP:
					EVENT.type = MOUSEEVENT;
					EVENT.y = YY-accel;
					EVENT.x = XX;
					doit = 1;
					break;
				case FG_MOUSEKEYS_DOWN:
					EVENT.type = MOUSEEVENT;
					EVENT.y = YY+accel;
					EVENT.x = XX;
					doit = 1;
					break;
				case FG_MOUSEKEYS_LEFT:
					EVENT.type = MOUSEEVENT;
					EVENT.x = XX-accel;
					EVENT.y = YY;
					doit = 1;
					break;
				case FG_MOUSEKEYS_RIGHT:
					EVENT.type = MOUSEEVENT;
					EVENT.x = xx+accel;
					EVENT.y = YY;
					doit = 1;
					break;
				case FG_MOUSEKEYS_BUTTON1:
					EVENT.type = MOUSEEVENT;
					EVENT.x = XX;
					EVENT.y = YY;
					cbut ^= 1;
					doit = 1;
					break;
				case FG_MOUSEKEYS_BUTTON2:
					type = MOUSEEVENT;
					EVENT.x = XX;
					EVENT.y = YY;
					cbut ^= 2;
					doit = 1;
					break;
			}
			if (doit)
			{
				if (EVENT.akcel<16)		// max accel diff
					EVENT.akcel += 2;
				// round up co-ordinates
				if (EVENT.x<0)
					EVENT.x = 0;
				if (EVENT.x>(X_width - 1))
					EVENT.x = (X_width - 1);
				if (EVENT.y<0)
					EVENT.y = 0;
				if (EVENT.y>Y_max)
					EVENT.y = Y_max;
				EVENT.buttons = cbut;
			}
			last = EVENT.key;
		}
#endif
		if (rc==0)
		{
			sched_yield();
		}
		else
		{
			if (EVENT.type==KEYEVENT
				&& (EVENT.key==PRTRSC || EVENT.key==CTRL_F12))
			{
				SaveScreen();
				continue;
			} // PrtScr
#if defined(__linux__) || defined(__rtems__) || defined(_WIN32)
			else if (EVENT.type==KEYEVENT && EVENT.key==0x0003 && ctrl_break==0)
			{
				raise(SIGINT);
				exit(2);
			}

			else if (EVENT.type==KEYEVENT && EVENT.key==ESC)
			{
//				raise(SIGINT);
//				exit(2);
			}

#endif // LINUX
			XX = EVENT.x;
			YY = EVENT.y;
			break;
		}
		if (waits != 1)
		{
			FGApp::CallDelayProc();			// call delay handler
			FGApp::Timer();                   // call timer handler
			if (repeat_status>0)
				AutoRepeatDo();
		}
	}
//	gprintf(0,-1,0,0,"typ %d, key %x, x %d, y %d b %d\n", EVENT.type,EVENT.key,EVENT.x,EVENT.y,EVENT.buttons);
	if (mouseflag == 1)
		vector_mouse_cursor(xx2, yy2, false, mouseflag);
	if (EVENT.type ==	KEYEVENT)
		mouseflag =	0;
	else
		mouseflag =	1;
}

/**
	Redraw mouse pointer if needed.
*/
void FGApp::UpdateMousePointer(void)
{
	if (mouseflag == 1)
		vector_mouse_cursor(current_mouse_x, current_mouse_y, true, mouseflag);
}

/**
	Hide mouse pointer if needed.
*/
int FGApp::RemoveMousePointer(void)
{
	if (mouseflag == 1)
		vector_mouse_cursor(current_mouse_x, current_mouse_y, false, mouseflag);
	mouseflag =	0;
	return mouseflag;
}

/**
	Redraw mouse pointer unconditionally.
*/
void FGApp::ShowMousePointer(void)
{
	mouseflag =	1;
	vector_mouse_cursor(current_mouse_x, current_mouse_y, true, mouseflag);
}

/**
	Load & set the new mouse pointer.
*/
const FGMouseCursor * FGApp::CursorLoad(const FGMouseCursor *cur)
{
	if (cur == __fg_cursor)
		return cur;
	const FGMouseCursor *old = __fg_cursor;
	int oldf = RemoveMousePointer();
	__fg_cursor = cur;
	if (oldf)
		ShowMousePointer();
	return old;
}

int	__MouseDraw(int x, int y, int a, int& mouseflag)
{
	int	old=mouseflag;
	unsigned int	i;
	FGPixel *maska, *mapa;
	static FGPixel work[sizeof(m_save)], tmp[sizeof(m_save)];

	mouseflag =	a;
	if (a) // draw cursor
	{
		VideoToRam(m_save, x-cApp->__fg_cursor->xoff, y-cApp->__fg_cursor->yoff, 0, 0, cApp->__fg_cursor->w, cApp->__fg_cursor->h, cApp->__fg_cursor->w, cApp->__fg_cursor->h);
		memcpy(work, m_save, sizeof(m_save));
		maska =	cApp->__fg_cursor->mask;
		mapa  = cApp->__fg_cursor->bitmap;
		for	(i = 0;	i <	(cApp->__fg_cursor->w * cApp->__fg_cursor->h); i++)
		{
			work[i]	&= *maska;
			work[i]	|= ((*mapa++) &	(FGPixel)(~*maska++));
		}
		RamToVideo(work, 0, 0, x-cApp->__fg_cursor->xoff, y-cApp->__fg_cursor->yoff, cApp->__fg_cursor->w, cApp->__fg_cursor->h, cApp->__fg_cursor->w, cApp->__fg_cursor->h, BITBLIT_COPY, _GSET);
	}
	else // restore background
	{
		VideoToRam(tmp, x-cApp->__fg_cursor->xoff, y-cApp->__fg_cursor->yoff, 0, 0, cApp->__fg_cursor->w, cApp->__fg_cursor->h, cApp->__fg_cursor->w, cApp->__fg_cursor->h);
		if (memcmp(tmp,	work, sizeof(m_save)) != 0) // yes, changed
		{
			for (i = 0; i < sizeof(m_save)/bpp; i++)
			{
				if (tmp[i] != work[i])
				{
					m_save[i] = tmp[i];
				}
			}
		}
		RamToVideo(m_save, 0, 0, x-cApp->__fg_cursor->xoff, y-cApp->__fg_cursor->yoff, cApp->__fg_cursor->w, cApp->__fg_cursor->h, cApp->__fg_cursor->w, cApp->__fg_cursor->h, BITBLIT_COPY, _GSET);
	}
	return old;
}

// mouse cursor drawer
mouser_t   vector_mouse_cursor= __MouseDraw;

/**
	Enable drawing into the off-screen video RAM.
	This feature is intended for flickerless full screen
	animation. Parameter mode is one of these predefined:

	enum { FG_DOUBLEBUFFER, FG_TRIPLEBUFFER, FG_QUADBUFFER };

	More buffers may help for speed but is X-times memory
	hungry and not the all videocards supports it.
	On exit, return number of true allocated buffers (0 if error).
	You can still use BUFFERING mode when function fail but
	screen will flicker - all drawing goes to the visible screen.
*/
int FGApp::EnableBuffering(int mode)
{
	if (__fg_driver)
		return __fg_driver->EnableBuffering(mode);
	return (ENUM_BUFFERING)false;
}

/**
	Call this routine when all objects of the scene are
	drawn. The buffer will be showed at the next screen
	vertical retrace. Return 0 if fail - no buffering is
	allowed/initialized.
*/
int FGApp::Flip(void)
{
	if (__fg_driver)
	{
		__fg_fps++;
		if (fgstate.verbose)
		{
			gprintf(CWHITE, CBLACK,16,16,"fps %3d", __fg_fps_total);
		}
		UpdateMousePointer();
		return __fg_driver->Flip();
	}
	return 0;
}

/**
	Disable video buffering and release the used video memory.
*/
void FGApp::DisableBuffering(void)
{
	if (__fg_driver)
		__fg_driver->DisableBuffering();
}

/**
	Return current Operating system code.
	Code is one from these OS types.
*/
ENUM_OS FGApp::GetOS(void)
{
#ifdef __QNX__
	return OSTYPE_QNX;
#endif
#ifdef __linux__
	return OSTYPE_LINUX;
#endif
#ifdef __MSDOS__
	return OSTYPE_MSDOS;
#endif
#ifdef _WIN32
	return OSTYPE_WIN32;
#endif
}

void FGApp::blue_rect(int x, int y,	int	w, int h)
{
	draw_pattern_box(x,y, w-1, h-1, &__patt, _GNOT);
}

void FGApp::blue_rect2(int x, int y, int w, int h)
{
	if (!DragShape) blue_rect(x,y,w,h);
	else DragShape(x,y,w,h);
}

int FGApp::isgmkey(int k)
{
	FGWindow *c = GetCurrentWindow();

	switch (k)
	{
		case CTRL_TAB:			// ctrl+TAB
			if (c)
			{
				FGWindow::FindNextFindow();
				return true;
			}
		case ALT_X:			// alt+X
			if (flags & APP_ENABLEALTX)
			{
				AppDone();
				return true;
			}
			else return	0;
		case TAB:
			if (c)
				if (c->GetStatus() & WNOTEBOOK)
				{
					c->SetNextTabPage();
					return true;
				}
			break;
		case KRIGHT:
		case KDOWN:
			if (c)
			{
				FGControl* ctrl = c->GetDefaultControl();
				if (ctrl)
				{
					c->SetNextControl();
					return true;
				}
			}
			break;
		case KLEFT:
		case KUP:
			if (c)
			{
				FGControl* ctrl = c->GetDefaultControl();
				if (ctrl)
				{
					c->SetPreviousControl();
					return true;
				}
			}
			break;
		case ESC:
			if (c && !(c->GetStatus() & WESCAPE))
				break;
		case ALT_F04:			// alt+F4
			if (c)
				c->WindowClose();
			return true;
		case ALT_F12:			// alt+F12
			if (c)
			{
				if (c->FGWindow::WindowIconize())
					return true;
			}
	}
	return false;
}

void FGApp::DoubleClick(int& event)
{
	static int last=0, tmout=0;
	int now = FGClock();
	if (event == last && (now-tmout) <= FGDBLCLICKTIME)
		event += (DBLCLICKLEFTEVENT-CLICKLEFTEVENT);
	else
	{
		last = event;
		tmout = now;
	}
}

void FGApp::ButtonClick(FGEvent& EVENT, FGControl* ctrl)
{
	int x = EVENT.x - ctrl->owner->x;
	int y = EVENT.y - ctrl->owner->y;

	ctrl->ClickDown(x,y);
	EVENT.type = ACCELEVENT;	//bude to sprava
	EVENT.guiType = ctrl->GetType(); // od buttonu typ ...
	EVENT.accel = ctrl;
	EVENT.key = ctrl->GetId();
	delay(100);
	Puk();
	ctrl->ClickUp(true);
}

// event handling core
void FGApp::TranslateUserEvent(FGEvent& EVENT)
{
	FGWindow *n =	0, *curr = GetCurrentWindow(), *ecurr=0, *current_used_window_ptr=0;
	FGControl* current_control_ptr;
	int	magn = flags&APP_MAGNIFIER;
	FGEditBox *ebox=0;
	FGEvent original(EVENT);
	EVENT.guiType = 0;
	FGEvent app(NOEVENT); // events to  application

	if (curr)
		switch (EVENT.type)
		{
			case KEYEVENT:
				if (curr && !curr->IsInInput())	//ai nieje input
				{
					if (isgmkey(EVENT.key))
						return;

					// reload after some possible changes
					curr = GetCurrentWindow();
				    if (curr == 0)
					    return;
					FGControl* ctrl = curr->GetDefaultControl();

					if ( (EVENT.key == CR) && ctrl)
					{
						ButtonClick(EVENT, ctrl);
						curr = GetCurrentWindow();
					}
					else if ( (ctrl = FGControl::ButtonFind(&EVENT, 7)) != 0)
					{
						if (curr->IsVisible() && ctrl)	//tak zisti buttony
						{
							ButtonClick(EVENT, ctrl);
							curr = GetCurrentWindow();
						}
					}
					else // plain KEY - pass to AppProc
					{
						app.type = KEYEVENT;
						app.key  = EVENT.key;
					}
				}
				else
				{
					if (EVENT.key==TAB)
					{
						ebox = curr->FindNextControl();
						ecurr = curr;
						if (curr)
							curr->WindowInputChar(CR);
					}
					else
						if (curr)
							curr->WindowInputChar(EVENT.key);
					EVENT.type = NOEVENT;
				}
				break;
			case MOUSEEVENT:
				if (mousefirst)
				{
					old_mouse_x =	EVENT.x;
					old_mouse_y =	EVENT.y;
					old_mouse_buttons =	EVENT.buttons;
					mousefirst = 0;
				}
				current_mouse_x = EVENT.x;
				current_mouse_y = EVENT.y;
				current_mouse_buttons = EVENT.buttons;

				tmp_event = EVENT.type;
				EVENT.type = NOEVENT;

				// second phase for windowresize
				if ( window_resize_in_action && current_mouse_buttons == FG_BUTTON_LEFT )
				{
					if (fulldrag && !(curr->GetStatus()&WFASTMOVE))
					{
						curr->WindowResize(current_mouse_x - old_mouse_x, current_mouse_y - old_mouse_y);
					}
					else
					{
						blue_rect(curr->GetX(),	curr->GetY(), curr->GetW() + window_resize_width, curr->GetH() + window_resize_height);
						window_resize_width	+= (current_mouse_x - old_mouse_x);
						window_resize_height += (current_mouse_y - old_mouse_y);
						blue_rect(curr->GetX(),	curr->GetY(), curr->GetW() + window_resize_width, curr->GetH() + window_resize_height);
					}
					window_resize_in_action++;
					break;
				}
				// last phase for windowresize
				else if	(window_resize_in_action && current_mouse_buttons == FG_BUTTON_NONE )
				{
					if (!fulldrag || curr->GetStatus() & WFASTMOVE)
					{
						blue_rect(curr->GetX(),	curr->GetY(), curr->GetW() + window_resize_width, curr->GetH() + window_resize_height);
						if (window_resize_width	+ window_resize_height)
						{
							curr->WindowResize(window_resize_width,	window_resize_height);
						}
					}
					window_resize_in_action = 0;
				}
				// second phase for windowmove
				else if	(window_drag_in_action && current_mouse_buttons  == FG_BUTTON_LEFT)
				{
					if (fulldrag && !(curr->GetStatus()&WFASTMOVE))
					{
						curr->WindowMove(current_mouse_x - old_mouse_x,	current_mouse_y - old_mouse_y);
					}
					else
					{
						blue_rect(curr->GetX() + drag_width, curr->GetY() + drag_height, curr->GetW(),	curr->GetH());
						drag_width += (current_mouse_x - old_mouse_x);
						drag_height += (current_mouse_y - old_mouse_y);
						blue_rect(curr->GetX() + drag_width,	curr->GetY() + drag_height, curr->GetW(),	curr->GetH());
					}
					window_drag_in_action++;
				}
				// last phase for windowmove
				else if	(window_drag_in_action && current_mouse_buttons == FG_BUTTON_NONE)
				{
					if (!fulldrag || curr->GetStatus()&WFASTMOVE)
					{
						blue_rect(curr->GetX() + drag_width,	curr->GetY() + drag_height, curr->GetW(),	curr->GetH());
						if (drag_width +	drag_height)
							curr->WindowMove(drag_width,	drag_height);
					}
					window_drag_in_action = 0;
				}
				// release mousebutton on control
				// fix: 8.12.2005 - added test to idcb
				else if	(idcb && button_reached	&& current_mouse_buttons == FG_BUTTON_NONE)
				{
					button_reached = 0;
					idcb->ClickUp(true);
					AutoRepeatEnd();
					curr = GetCurrentWindow();
					if (curr->IsVisible() && ((idc2->ButtonFind(&EVENT, 3)) == idcb))
					{
						EVENT.type = ACCELEVENT;	//bude to sprava
						EVENT.guiType	= idcb->GetType();
						EVENT.accel = idcb;
					}
					else
						EVENT.type = NOEVENT;
					EVENT.key	= idcb->GetId();
					break;
				}
				// push mousebutton on control
				else if	(button_reached	&& current_mouse_buttons  == FG_BUTTON_LEFT)
				{
					if (idcb == 0)
					{
						button_reached = 0;
						AutoRepeatEnd();
					}
					else
					{
						int tt = idcb->GetType();
						if (curr->IsVisible()
						&& ((idc2	= FGControl::ButtonFind(&EVENT,3)) != idcb)
						&& tt!=FGControl::SLIDEBAR)
						{
							button_reached = 0;
							AutoRepeatEnd();
							if (tt!=FGControl::POINTBUTTON && tt!=FGControl::CHECKBUTTON)
								idcb->ClickUp(false);	// nevolat handler !!!!
							break;
						}
						else
						{
							int x = EVENT.x - curr->GetX();
							int y = EVENT.y - curr->GetY();
							idcb->ClickDown(x,y);
						}
					}
				}
				else if	(selection_steps && (current_mouse_buttons == FG_BUTTON_NONE)) // selection_steps end
				{
					if (drag_window) // right click to wnd
					{
						startXdrag = clickx	- curr->GetX() - curr->GetXW();
						startYdrag = clicky	- curr->GetY() - curr->GetYW();
						endXdrag = clickw;
						endYdrag = clickh;
					}
					else
					{
						startXdrag = clickx;
						startYdrag = clicky;
						endXdrag = clickw;
						endYdrag = clickh;
					}
					if (selection_steps > DRAG_TRESHOLD+1)
						if (magn) blue_rect2(clickx, clicky, clickw, clickh);
					if (clickw<0) {	clickx += clickw; clickw = -clickw;	}
					if (clickh<0) {	clicky += clickh; clickh = -clickh;	}
					if (drag_window) // right click to wnd
					{
						if (selection_steps >DRAG_TRESHOLD && old_mouse_buttons == FG_BUTTON_RIGHT)
						{
							EVENT.type = DRAGRIGHTEVENT;
// 28.9.2004 - moved from common part below
						app.type = EVENT.type;
						app.x =	clickx;
						app.y =	clicky;
						app.w =	clickw;
						app.h =	clickh;
						}
						else if	(selection_steps <= DRAG_TRESHOLD	&& old_mouse_buttons == FG_BUTTON_RIGHT)
						{
							DoubleClick(EVENT.type = CLICKRIGHTEVENT);
						}
						else if	(selection_steps > DRAG_TRESHOLD && old_mouse_buttons == FG_BUTTON_LEFT)
						{
							EVENT.type =	DRAGLEFTEVENT;
// 28.9.2004 - moved from common part below
						app.type = EVENT.type;
						app.x =	clickx;
						app.y =	clicky;
						app.w =	clickw;
						app.h =	clickh;
						}
						else if	(selection_steps <= DRAG_TRESHOLD	&& old_mouse_buttons == FG_BUTTON_LEFT)
						{
							DoubleClick(EVENT.type = CLICKLEFTEVENT);
						}
						EVENT.w =	clickw;
						EVENT.h =	clickh;
						curr = drag_window; // redirect it to right window
						EVENT.x =	clickx - curr->GetX() - curr->GetXW();
						EVENT.y =	clicky - curr->GetY() - curr->GetYW();

						// 19.10.2007 - aby sa vyrez posielal aby oknu ak tak je (a nie aj aplikacii)
						if (EVENT.type == DRAGLEFTEVENT || EVENT.type == DRAGRIGHTEVENT)
						{
							// Ak si zacal drag nie nad oknom, tak neposielaj udalost oknu
							if (curr->TitleFind(&app) == 0)
								app.type = 0;
						}
// moved from here !!!
					}
					else  // right click to background
					{
						if (selection_steps > DRAG_TRESHOLD && old_mouse_buttons == FG_BUTTON_RIGHT)
						    app.type = DRAGRIGHTEVENT;
						else if	(selection_steps <= DRAG_TRESHOLD && old_mouse_buttons	== FG_BUTTON_RIGHT)
						{
							DoubleClick(app.type = CLICKRIGHTEVENT);

						}
						else if	(selection_steps > DRAG_TRESHOLD && old_mouse_buttons == FG_BUTTON_LEFT)
						    app.type =	DRAGLEFTEVENT;
						else if	(selection_steps <= DRAG_TRESHOLD && old_mouse_buttons	== FG_BUTTON_LEFT)
						{
							DoubleClick(app.type = CLICKLEFTEVENT);
						}
						app.x =	clickx;
						app.y =	clicky;
						app.w =	clickw;
						app.h =	clickh;
					}
					selection_steps =	0;
					break;
				}
				else if	(selection_steps && (current_mouse_buttons & (FG_BUTTON_LEFT | FG_BUTTON_RIGHT)))	// selection_steps continue
				{
					if (selection_steps == DRAG_TRESHOLD+1)	if (magn) blue_rect2(clickx,	clicky,	clickw,	clickh);
					if (selection_steps > DRAG_TRESHOLD) if (magn) blue_rect2(clickx, clicky, clickw, clickh);
					clickw += (current_mouse_x -	old_mouse_x);
					clickh += (current_mouse_y -	old_mouse_y);
					if (selection_steps > DRAG_TRESHOLD) if (magn) blue_rect2(clickx, clicky, clickw, clickh);
					selection_steps++;
					if (selection_steps == DRAG_TRESHOLD+1)
					{
						int type2 =	(current_mouse_buttons==FG_BUTTON_RIGHT) ? STARTDRAGRIGHTEVENT : STARTDRAGLEFTEVENT;

						if (drag_window)
						{
							EVENT.type = type2;
							EVENT.x =	clickx - curr->GetX() - curr->GetXW();
							EVENT.y =	clicky - curr->GetY() - curr->GetYW();
						}
// 29.7.2004 send to App too!
//						else
						{
							app.type = type2;
							app.x = clickx;
							app.y = clicky;
						}
					}
					else // mouse move only!
					{
						// when ptr is over the window, its receive relative pos
						// app proc receive EACH movement as absolute pos
						app.x = EVENT.x;
						app.y = EVENT.y;
						app.key = 0;

						if (drag_window)
						{
							EVENT.type = MOVEEVENT;
							EVENT.x = EVENT.x - curr->GetX() - curr->GetXW();
							EVENT.y = EVENT.y - curr->GetY() - curr->GetYW();
//printf("%d %d [%d %d %d]\n", EVENT.x, EVENT.y, app.x, curr->GetX(), curr->GetXW());
						}

						app.type = MOVEEVENT;
					}
				}
				// most first push button - what next?
				else if	((!button_reached) && (!window_drag_in_action) && (!selection_steps) && (!window_resize_in_action) && (current_mouse_buttons == FG_BUTTON_LEFT))
				{
					int	t;

					t =	curr->TitleFind(&EVENT);
					if (curr->IsVisible()
					   && ((idcb = FGControl::ButtonFind(&EVENT, 7)) !=	0) && t	!= 4)
					{
						int x = EVENT.x - curr->GetX();
						int y = EVENT.y - curr->GetY();

						button_reached = 1;
						if (curr->GetStatus() & WUSESELECTEDCONTROL)
							curr->SetDefaultControl(idcb);

						idcb->ClickDown(x,y);

						AutoRepeatStart(idcb);
					}
					else if	(t)
					{
						// windowmove start
						if (t == 1 && (!old_mouse_buttons) && !(curr->GetStatus() & WUNMOVED))
						{
							window_drag_in_action = 1;
							drag_width =	drag_height =	0;
							if (!fulldrag || curr->GetStatus()&WFASTMOVE)
							{
								blue_rect(curr->GetX() + drag_width,	curr->GetY() + drag_height, curr->GetW(),	curr->GetH());
							}
						}
						// close window
						else if	(t == 2)
						{
							if (!curr->WindowClose())
							{
								// return if deleted
								return ;
							}
							curr = GetCurrentWindow();
						}
						// hide
						else if	(t==3) curr->WindowIconize();
						// resize
						else if	(t == 4)
						{
							window_resize_in_action = 1;
							window_resize_width	= 0;
							window_resize_height =	0;

							if (!fulldrag || curr->GetStatus()&WFASTMOVE)
							{
								blue_rect(curr->GetX(),	curr->GetY(), curr->GetW() + window_resize_width, curr->GetH() + window_resize_height);
							}
						}
					}
					else		// tuklo sa vedla titulku
					{
						n =	WindowFind(&EVENT);
						// do okna
						// prepni na toto okno ak nieje aktivne
						if ((n != curr)	&& (n))
						{
							if (!(curr->GetStatus()	& WMODAL))
							{
								n->WindowFocus();
								EVENT.type = tmp_event;
								TranslateUserEvent(EVENT);
								curr = GetCurrentWindow();
								if (curr != n || !n) return; // window destroyed !!
							}
							else Snd(100,70);
						}
						else
						{
							selection_steps =	1;
							drag_window =	WindowFind(&EVENT); // 0-back, 1-wnd
							clickx = EVENT.x;
							clicky = EVENT.y;
							clickw = clickh	= 0;
						}
					}
				}
				else if	(old_mouse_buttons == FG_BUTTON_NONE && current_mouse_buttons == FG_BUTTON_RIGHT)	// right click down
				{
					selection_steps =	1;
					drag_window =	WindowFind(&EVENT);  // find window for click
					if ((drag_window != curr) && drag_window)
					{
						if (!(curr->GetStatus()	& WMODAL))
						{
							drag_window->WindowFocus();
							// 2.2.2005 - don't recurse
							// return;

							// 12.2.2007 - reenabled
							// predchadzajuca zmena sposobovala nevyslanie CLICK event po FOCUS
//							EVENT.type = tmp_event;
//							TranslateUserEvent(EVENT); // zavolaj rekurzivne pre vygenerovanie CLICKRIGHTEVENT
							curr = GetCurrentWindow();
							if (curr != drag_window || !drag_window) return; // window destroyed !!

						}
						else Snd(100,70);
					}
					if (curr && curr->IsInInput())  // if input, force ESC
					{
						curr->WindowInputChar(ESC);
					}
					clickx = EVENT.x;
					clicky = EVENT.y;
					clickw = clickh	= 0;
				}
				else if	(current_mouse_buttons == FG_BUTTON_NONE && old_mouse_buttons == FG_BUTTON_NONE)	// only mouse move
				{
					EVENT.type = MOVEEVENT;
					app.type = MOVEEVENT;
					app.x = EVENT.x;
					app.y = EVENT.y;
					app.key = 0;

					current_control_ptr	= FGControl::ButtonFind(&EVENT, 3);

//	absolete?		if (current_control_ptr	&& current_control_ptr->GetOwner() != curr)
//					{
//						break;
//					}

					if (current_control_ptr)
					{
						EVENT.key = current_control_ptr->GetId();
//						if (current_control_ptr->GetTrigger())
//							current_control_ptr = 0;
//printf("xxx\n");
					}
					else
						if	((current_used_window_ptr =	WindowFind(&EVENT)) != 0)
					{
						EVENT.key	= current_used_window_ptr==curr?0:-1;
						EVENT.x =	EVENT.x-current_used_window_ptr->GetX()-current_used_window_ptr->GetXW();
						EVENT.y =	EVENT.y-current_used_window_ptr->GetY()-current_used_window_ptr->GetYW();
						curr = current_used_window_ptr;		// sender is changed !!!
					}
					else
					{
						EVENT.type = NOEVENT;
						app.type = MOVEEVENT;
						app.x = EVENT.x;
						app.y = EVENT.y;
						app.key = 0;
					}
					break;
				}
				else if	(old_mouse_buttons == FG_BUTTON_MIDDLE && current_mouse_buttons == FG_BUTTON_NONE)	// middle click down
				{
					if (WindowFind(&EVENT))
					{
						EVENT.x =	clickx - curr->GetX() - curr->GetXW();
						EVENT.y =	clicky - curr->GetY() - curr->GetYW();
						EVENT.w = EVENT.h = 0;
						DoubleClick(EVENT.type = CLICKMIDDLEEVENT);
					}
					else
					{
						app.x = clickx;
						app.y = clicky;
						app.w = app.h=0;
						DoubleClick(app.type = CLICKMIDDLEEVENT);
					}
				}
				else if	((current_mouse_buttons == FG_BUTTON_WHEEL_UP || current_mouse_buttons == FG_BUTTON_WHEEL_DOWN) && old_mouse_buttons == FG_BUTTON_NONE)	// middle click down
				{
					current_control_ptr	= FGControl::ButtonFind(&original, 7);
					int direction = (current_mouse_buttons == FG_BUTTON_WHEEL_UP)?-1:1;
					EVENT.buttons = app.buttons = direction;
					if (WindowFind(&EVENT))
					{
						EVENT.x =	current_mouse_x - curr->GetX() - curr->GetXW();
						EVENT.y =	current_mouse_y - curr->GetY() - curr->GetYW();
						EVENT.w = EVENT.h = 0;
						EVENT.type = MOUSEWHEELEVENT;
						if (curr && current_control_ptr)
						{
							current_control_ptr->WheelSpin(direction);
						}
					}
					app.x = current_mouse_x;
					app.y = current_mouse_y;
					app.w = app.h=0;
					app.type = MOUSEWHEELEVENT;
				}
		}
	else
	{
		switch(EVENT.type)  // send & mouse keys only
		{
			case KEYEVENT:
				if (isgmkey(EVENT.key))
					break;	// exit when no window are
			case MOVEEVENT:
				FGApp::SendToApp(&EVENT);
		}
		return ;
	}							// no window exists

	if (app.type==MOVEEVENT)
	{
		app.wnd = current_used_window_ptr;
		EVENT.accel = current_control_ptr;
	}

	if (!button_reached && !window_drag_in_action && !selection_steps && !window_resize_in_action && (EVENT.type==MOVEEVENT || app.type==MOVEEVENT))
	{
		int value=0;

		if (old_mouse_x==current_mouse_x)
		{
			if (old_mouse_x==0) value |= 1;
			else if (old_mouse_x>=(X_width - 1)-1) value |= 2;
		}
		if (old_mouse_y==current_mouse_y)
		{
			if (old_mouse_y==0) value |= 4;
			if (old_mouse_y>=(Y_width-1)-1) value |= 8;
		}
		if (value)
		{
			app.type = CURSOROUTEVENT;
			app.key  = value;
		}
	}
	old_mouse_x = current_mouse_x;
	old_mouse_y = current_mouse_y;
	old_mouse_buttons = current_mouse_buttons;

	// there are it's magic place where an events goes into the right place
	if (EVENT.type == ACCELEVENT) app = EVENT;

	int ii = curr ? curr->GetId() : -1;

	if (app.type !=	NOEVENT)
		FGApp::SendToApp(&app); // send to app if needed

	if (EVENT.type != NOEVENT)   // send to FGWindow if needed
	{
		if (EVENT.type == ACCELEVENT && curr->IsActive())
			curr->IsActive()->RefreshGroup();

		if (EVENT.type==MOVEEVENT && EVENT.key==-1)
		{
			EVENT.key = 0;
			if (current_used_window_ptr) current_used_window_ptr->SendToWindow(&EVENT);
		}
		else if ((curr=GetCurrentWindow()) != 0)
		{
			curr->SendToWindow(&EVENT);
		}
	}

	curr = WindowFind(ii);

	if (curr)
		curr->RunHandler(); // call to control handler also if window still exist

	// at this point, call_handler is already NULL

	// switch to the next Editbox
	if (ebox && ecurr && curr==ecurr)
		ebox->ClickUp(1);
}

#ifdef FG_NAMESPACE
}
#endif

