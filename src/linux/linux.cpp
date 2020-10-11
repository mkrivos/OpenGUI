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
    linux.cc - Linux support routines
*/

#define __USE_GNU
#include "fgbase.h"
#include "_fastgl.h"

#include "drivers.h"
#include "linkeyb.h"
#include "linux.h"
#include <execinfo.h>

#include "fgevent.h"

#define MY_SIGNAL	SIGUNUSED		// SIGUSR2, SIGUNUSED

#ifdef FG_NAMESPACE
namespace fgl {
#endif

static char *strs[3] = {"/usr/sbin/gpm","-k","-R"};
static char *arguments[8]={strs[0]};
int kbd_fd;
void (*ptr_vga_setpage)(int);
int nofb;
int nodga, nox11;

static char *ttynames[] =
{"/dev/tty0", "/dev/console", NULL};
static char *ttyfmts[] =
{"/dev/tty%d", "/dev/tty%02x", "/dev/tty%x", "/dev/tty%02d", NULL};

#define zero_sa_mask(maskptr) memset(maskptr, 0, sizeof(sigset_t))
static char sig2catch[] =
{SIGHUP, SIGINT, SIGQUIT, SIGILL,
 SIGTRAP, SIGIOT, SIGBUS, SIGFPE,
 SIGSEGV, SIGPIPE, SIGALRM, SIGTERM,
 SIGXCPU, SIGXFSZ, SIGVTALRM,
 SIGPWR};

static struct sigaction old_signal_handler[sizeof(sig2catch)];
extern "C" char *strsignal __P ((int __sig));

static const struct fb_videomode modedb[] = {
    {
    /* 640x400 @ 70 Hz, 31.5 kHz hsync */
    NULL, 70, 640, 400, 39721, 40, 24, 39, 9, 96, 2,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 640x480 @ 60 Hz, 31.5 kHz hsync */
    NULL, 60, 640, 480, 39721, 40, 24, 32, 11, 96, 2,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 800x600 @ 56 Hz, 35.15 kHz hsync */
    NULL, 56, 800, 600, 27777, 128, 24, 22, 1, 72, 2,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 87 Hz interlaced, 35.5 kHz hsync */
    NULL, 87, 1024, 768, 22271, 56, 24, 33, 8, 160, 8,
    0, FB_VMODE_INTERLACED
    }, {
    /* 640x400 @ 85 Hz, 37.86 kHz hsync */
    NULL, 85, 640, 400, 31746, 96, 32, 41, 1, 64, 3,
    FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 640x480 @ 72 Hz, 36.5 kHz hsync */
    NULL, 72, 640, 480, 31746, 144, 40, 30, 8, 40, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 640x480 @ 75 Hz, 37.50 kHz hsync */
    NULL, 75, 640, 480, 31746, 120, 16, 16, 1, 64, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 800x600 @ 60 Hz, 37.8 kHz hsync */
    NULL, 60, 800, 600, 25000, 88, 40, 23, 1, 128, 4,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 640x480 @ 85 Hz, 43.27 kHz hsync */
    NULL, 85, 640, 480, 27777, 80, 56, 25, 1, 56, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 89 Hz interlaced, 44 kHz hsync */
    NULL, 69, 1152, 864, 15384, 96, 16, 110, 1, 216, 10,
    0, FB_VMODE_INTERLACED
    }, {
    /* 800x600 @ 72 Hz, 48.0 kHz hsync */
    NULL, 72, 800, 600, 20000, 64, 56, 23, 37, 120, 6,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 60 Hz, 48.4 kHz hsync */
    NULL, 60, 1024, 768, 15384, 168, 8, 29, 3, 144, 6,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 640x480 @ 100 Hz, 53.01 kHz hsync */
    NULL, 100, 640, 480, 21834, 96, 32, 36, 8, 96, 6,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 60 Hz, 53.5 kHz hsync */
    NULL, 60, 1152, 864, 11123, 208, 64, 16, 4, 256, 8,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 800x600 @ 85 Hz, 55.84 kHz hsync */
    NULL, 85, 800, 600, 16460, 160, 64, 36, 16, 64, 5,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 70 Hz, 56.5 kHz hsync */
    NULL, 70, 1024, 768, 13333, 144, 24, 29, 3, 136, 6,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 87 Hz interlaced, 51 kHz hsync */
    NULL, 87, 1280, 1024, 12500, 56, 16, 128, 1, 216, 12,
    0, FB_VMODE_INTERLACED
    }, {
    /* 800x600 @ 100 Hz, 64.02 kHz hsync */
    NULL, 100, 800, 600, 14357, 160, 64, 30, 4, 64, 6,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 76 Hz, 62.5 kHz hsync */
    NULL, 76, 1024, 768, 11764, 208, 8, 36, 16, 120, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 70 Hz, 62.4 kHz hsync */
    NULL, 70, 1152, 864, 10869, 106, 56, 20, 1, 160, 10,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 61 Hz, 64.2 kHz hsync */
    NULL, 61, 1280, 1024, 9090, 200, 48, 26, 1, 184, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1400x1050 @ 60Hz, 63.9 kHz hsync */
    NULL, 68, 1400, 1050, 9259, 136, 40, 13, 1, 112, 3,
    0, FB_VMODE_NONINTERLACED       
    }, {
    /* 1400x1050 @ 75,107 Hz, 82,392 kHz +hsync +vsync*/
    NULL, 75, 1400, 1050, 9271, 120, 56, 13, 0, 112, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1400x1050 @ 60 Hz, ? kHz +hsync +vsync*/
        NULL, 60, 1400, 1050, 9259, 128, 40, 12, 0, 112, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 85 Hz, 70.24 kHz hsync */
    NULL, 85, 1024, 768, 10111, 192, 32, 34, 14, 160, 6,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 78 Hz, 70.8 kHz hsync */
    NULL, 78, 1152, 864, 9090, 228, 88, 32, 0, 84, 12,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 70 Hz, 74.59 kHz hsync */
    NULL, 70, 1280, 1024, 7905, 224, 32, 28, 8, 160, 8,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1600x1200 @ 60Hz, 75.00 kHz hsync */
    NULL, 60, 1600, 1200, 6411, 256, 32, 52, 10, 160, 8,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 84 Hz, 76.0 kHz hsync */
    NULL, 84, 1152, 864, 7407, 184, 312, 32, 0, 128, 12,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 74 Hz, 78.85 kHz hsync */
    NULL, 74, 1280, 1024, 7407, 256, 32, 34, 3, 144, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1024x768 @ 100Hz, 80.21 kHz hsync */
    NULL, 100, 1024, 768, 8658, 192, 32, 21, 3, 192, 10,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 76 Hz, 81.13 kHz hsync */
    NULL, 76, 1280, 1024, 7407, 248, 32, 34, 3, 104, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1600x1200 @ 70 Hz, 87.50 kHz hsync */
    NULL, 70, 1600, 1200, 5291, 304, 64, 46, 1, 192, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1152x864 @ 100 Hz, 89.62 kHz hsync */
    NULL, 100, 1152, 864, 7264, 224, 32, 17, 2, 128, 19,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 85 Hz, 91.15 kHz hsync */
    NULL, 85, 1280, 1024, 6349, 224, 64, 44, 1, 160, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1600x1200 @ 75 Hz, 93.75 kHz hsync */
    NULL, 75, 1600, 1200, 4938, 304, 64, 46, 1, 192, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1680x1050 @ 60 Hz, 65.191 kHz hsync */
    NULL, 60, 1680, 1050, 6848, 280, 104, 30, 3, 176, 6,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1600x1200 @ 85 Hz, 105.77 kHz hsync */
    NULL, 85, 1600, 1200, 4545, 272, 16, 37, 4, 192, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1280x1024 @ 100 Hz, 107.16 kHz hsync */
    NULL, 100, 1280, 1024, 5502, 256, 32, 26, 7, 128, 15,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 1800x1440 @ 64Hz, 96.15 kHz hsync  */
    NULL, 64, 1800, 1440, 4347, 304, 96, 46, 1, 192, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1800x1440 @ 70Hz, 104.52 kHz hsync  */
    NULL, 70, 1800, 1440, 4000, 304, 96, 46, 1, 192, 3,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 512x384 @ 78 Hz, 31.50 kHz hsync */
    NULL, 78, 512, 384, 49603, 48, 16, 16, 1, 64, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 512x384 @ 85 Hz, 34.38 kHz hsync */
    NULL, 85, 512, 384, 45454, 48, 16, 16, 1, 64, 3,
    0, FB_VMODE_NONINTERLACED
    }, {
    /* 320x200 @ 70 Hz, 31.5 kHz hsync, 8:5 aspect ratio */
    NULL, 70, 320, 200, 79440, 16, 16, 20, 4, 48, 1,
    0, FB_VMODE_DOUBLE
    }, {
    /* 320x240 @ 60 Hz, 31.5 kHz hsync, 4:3 aspect ratio */
    NULL, 60, 320, 240, 79440, 16, 16, 16, 5, 48, 1,
    0, FB_VMODE_DOUBLE
    }, {
    /* 320x240 @ 72 Hz, 36.5 kHz hsync */
    NULL, 72, 320, 240, 63492, 16, 16, 16, 4, 48, 2,
    0, FB_VMODE_DOUBLE
    }, {
    /* 400x300 @ 56 Hz, 35.2 kHz hsync, 4:3 aspect ratio */
    NULL, 56, 400, 300, 55555, 64, 16, 10, 1, 32, 1,
    0, FB_VMODE_DOUBLE
    }, {
    /* 400x300 @ 60 Hz, 37.8 kHz hsync */
    NULL, 60, 400, 300, 50000, 48, 16, 11, 1, 64, 2,
    0, FB_VMODE_DOUBLE
    }, {
    /* 400x300 @ 72 Hz, 48.0 kHz hsync */
    NULL, 72, 400, 300, 40000, 32, 24, 11, 19, 64, 3,
    0, FB_VMODE_DOUBLE
    }, {
    /* 480x300 @ 56 Hz, 35.2 kHz hsync, 8:5 aspect ratio */
    NULL, 56, 480, 300, 46176, 80, 16, 10, 1, 40, 1,
    0, FB_VMODE_DOUBLE
    }, {
    /* 480x300 @ 60 Hz, 37.8 kHz hsync */
    NULL, 60, 480, 300, 41858, 56, 16, 11, 1, 80, 2,
    0, FB_VMODE_DOUBLE
    }, {
    /* 480x300 @ 63 Hz, 39.6 kHz hsync */
    NULL, 63, 480, 300, 40000, 56, 16, 11, 1, 80, 2,
    0, FB_VMODE_DOUBLE
    }, {
    /* 480x300 @ 72 Hz, 48.0 kHz hsync */
    NULL, 72, 480, 300, 33386, 40, 24, 11, 19, 80, 3,
    0, FB_VMODE_DOUBLE
    }, {
    /* 1920x1200 @ 60 Hz, 74.5 Khz hsync */
    NULL, 60, 1920, 1200, 5177, 128, 336, 1, 38, 208, 3,
    FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,FB_VMODE_NONINTERLACED
    }, {
    /* 1152x768, 60 Hz, PowerBook G4 Titanium I and II */
    NULL, 60, 1152, 768, 15386, 158, 26, 29, 3, 136, 6,
    FB_SYNC_HOR_HIGH_ACT|FB_SYNC_VERT_HIGH_ACT, FB_VMODE_NONINTERLACED
    }, {
    /* 1366x768, 60 Hz, 47.403 kHz hsync, WXGA 16:9 aspect ratio */
    NULL, 60, 1366, 768, 13806, 120, 10, 14, 3, 32, 5,
    0, FB_VMODE_NONINTERLACED
    }, { 0, 0 }
};

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

void delay(unsigned cnt)
{
	usleep(cnt*1000);
}

int FGLinuxDriver::recode_key(int k)
{
	int *p=recode+1;
	while(*p!=-1)
	{
		if (*p==k) return *(p-1);
		p += 2;
	}
	return k;
}
#ifndef MAP_FAILED
#define MAP_FAILED ((void *)-1)
#endif

/***************************************************************************/
/* Video Memory Mapping section                                            */
/***************************************************************************/

/* The file desc for /proc/mtrr. Once opened, left opened, and the mtrr
   driver will clean up when we exit. */

#define MTRR_FD_UNOPENED (-1)	/* We have yet to open /proc/mtrr */
#define MTRR_FD_PROBLEM (-2)	/* We tried to open /proc/mtrr, but had
				   a problem. */
static int mtrr_fd = MTRR_FD_UNOPENED;

/* Open /proc/mtrr. FALSE on failure. Will always fail on Linux 2.0,
   and will fail on Linux 2.2 with MTRR support configured out,
   so verbosity should be chosen appropriately. */
static int mtrr_open(void)
{
	/* Only report absence of /proc/mtrr once. */
	char **fn;
	static char *mtrr_files[] = {
		"/dev/cpu/mtrr",	/* Possible future name */
		"/proc/mtrr",		/* Current name */
		NULL
	};

	if (mtrr_fd == MTRR_FD_UNOPENED) {
		/* So open it. */
		for (fn = mtrr_files; mtrr_fd < 0 && *fn; fn++)
			mtrr_fd = open(*fn, O_WRONLY);

		if (mtrr_fd < 0)
			mtrr_fd = MTRR_FD_PROBLEM;
	}

	if (mtrr_fd == MTRR_FD_PROBLEM)
	{
		/* To make sure we only ever warn once, need to check
		   verbosity outside xf86MsgVerb */
//		if (verbose) printf("System lacks support for changing MTRRs\n");
		return 0;
	}
	else
		return 1;
}

static struct mtrr_wc_region * mtrr_cull_wc_region(unsigned long base, unsigned long size)
{
	/* Some BIOS writers thought that setting wc over the mmio
	   region of a graphics devices was a good idea. Try to fix
	   it. */

	struct mtrr_gentry gent;
	char buf[20];
	struct mtrr_wc_region *wcreturn = NULL, *wcr;

	/* Linux 2.0 users should not get a warning without -verbose */
	if (!mtrr_open())
		return NULL;

	for (gent.regnum = 0;
	     ioctl(mtrr_fd, MTRRIOC_GET_ENTRY, &gent) >= 0;
	     gent.regnum++) {
		if (gent.type != MTRR_TYPE_WRCOMB
		    || gent.base + gent.size <= base
		    || base + size <= gent.base)
			continue;

		/* Found an overlapping region. Delete it. */

		wcr = (mtrr_wc_region *)malloc(sizeof(*wcr));
		if (!wcr)
			return NULL;
		wcr->sentry.base = gent.base;
		wcr->sentry.size = gent.size;
		wcr->sentry.type = MTRR_TYPE_WRCOMB;
		wcr->added = 0;

		/* There is now a nicer ioctl-based way to do this,
		   but it isn't in current kernels. */
		snprintf(buf, sizeof(buf), "disable=%u\n", gent.regnum);

		if (write(mtrr_fd, buf, strlen(buf)) >= 0)
		{
//			printf("Removed MMIO write-combining range (0x%lx,0x%lx)\n", gent.base, gent.size);
			wcr->next = wcreturn;
			wcreturn = wcr;
		} else
		{
			free(wcr);
//			printf("Failed to remove MMIO write-combining range (0x%lx,0x%lx)\n",				   gent.base, gent.size);
		}
	}
	return wcreturn;
}


static struct mtrr_wc_region * mtrr_add_wc_region(unsigned long base, unsigned long size, int )
{
	struct mtrr_wc_region *wcr;
	int rc;

	/* Linux 2.0 should not warn, unless the user explicitly asks for WC. */
	if (!mtrr_open())
		return NULL;

	wcr = (mtrr_wc_region *)malloc(sizeof(*wcr));
	if (!wcr)
		return NULL;

	wcr->sentry.base = base;
	wcr->sentry.size = size;
	wcr->sentry.type = MTRR_TYPE_WRCOMB;
	wcr->added = 1;
	wcr->next = NULL;

	if ((rc=ioctl(mtrr_fd, MTRRIOC_ADD_ENTRY, &wcr->sentry)) >= 0) {
		/* Avoid printing on every VT switch */
//		if (verbose) printf("Write-combining range (0x%lx,0x%lx)\n",base, size);
		return wcr;
	}
	else {
		free(wcr);

		/* Don't complain about the VGA region: MTRR fixed
		   regions aren't currently supported, but might be in
		   the future. */
		if ((unsigned long)base >= 0x100000)
		{
//			printf("Failed to set up write-combining range (0x%lx,0x%lx), rc=%d\n", base, size, rc);
		}
		return NULL;
	}
}

static void mtrr_undo_wc_region(struct mtrr_wc_region *wcr)
{
	struct mtrr_wc_region *p, *prev;

	if (mtrr_fd > 0) {
		p = wcr;
		while (p)
		{
			if (p->added)
				ioctl(mtrr_fd, MTRRIOC_DEL_ENTRY, &p->sentry);
			prev = p;
			p = p->next;
			free(prev);
		}
	}
}

mtrr_wc_region *setWC(unsigned long base, unsigned long size, int enable, int verbose)
{
	if (enable)
		return mtrr_add_wc_region(base, size, verbose);
	else
		return mtrr_cull_wc_region(base, size);
}

void undoWC(mtrr_wc_region * regioninfo)
{
	mtrr_undo_wc_region(regioninfo);
}

static void BackTrace(void)
{
	void *array[64];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace (array, 64);
	strings = backtrace_symbols (array, size);

//	printf ("Obtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
	   printf ("%s\n", strings[i]);

	free (strings);
}

static void signal_handler(int v)
{
	unsigned i;
	static int pending=0;

	if (pending) return;

	pending = 1;
	cleanup();
return;	
	BackTrace();
	printf("OpenGUI: Signal %d (%s) received %s.\n", v, strsignal(v),
		   (v == SIGINT) ? " (ctrl-c pressed)" : "");

	for (i = 0; i < sizeof(sig2catch); i++)
		if (sig2catch[i] == v)
		{
			sigaction(v, old_signal_handler + i, NULL);
			raise(v);
			break;
		}
	if (i >= sizeof(sig2catch))
	{
		printf("OpenGUI: Aieeee! Illegal call to signal_handler, raising segfault.\n");
		raise(SIGSEGV);
	}
	if (v != SIGINT) pending = 0;
	else (void) signal(SIGINT, SIG_DFL);

}

static void set_signals(void)
{
	unsigned i;
	struct sigaction siga;

	/* do our own interrupt handling */
	for (i = 0; i < sizeof(sig2catch); i++)
	{
		siga.sa_handler = signal_handler;
		siga.sa_flags = 0;
		zero_sa_mask(&(siga.sa_mask));
		sigaction((int) sig2catch[i], &siga, old_signal_handler + i);
	}
}

static void reset_signals(void)
{
	unsigned i;

	/* do our own interrupt handling */
	for (i = 0; i < sizeof(sig2catch); i++)
	{
		signal((int) sig2catch[i], SIG_DFL);
	}
}

int  FGLinuxDriver::ps2_mouse_init(void)
{
	msefd = open( "/dev/psaux", O_RDONLY );
	if (msefd < 0)
	{
	   printf( "OpenGUI: Error opening `/dev/psaux' !\n" );
	   return -1;
	}
	ps2_mouse = 1;
	mousex = (w-1) / 2;
	mousey = (h-1) / 2;
	maxfd = msefd + 1;
	FD_ZERO(&fdset);
	FD_SET(msefd, &fdset);
	return 0;
}

// type, key, x, y, buttons
int  FGLinuxDriver::ps2_mouse_get(int& type, long& key, int& x, int& y, int& buttons)
{
    static unsigned char packet[3];
    static int pos = 0;
    static int i=0;
    static unsigned char last_buttons = 0;
	static int x_motion=0;
	static int y_motion=0;
    static int readlen=0;
    static unsigned char buf[300];
	unsigned char changed_buttons=0;

	oldy = mousey;
	oldx = mousex;

	struct timeval tv = { 0, 0 };
	fd_set fds = fdset;
	if (select(maxfd, &fds, NULL, NULL, &tv) <= 0)
	{
		return 0;
	}

    if (readlen==i)
	{
		readlen = read(msefd, buf, 300);
		i = 0;
	}

	if (readlen > 0)
	{
          int dx, dy;
          int butt=0;

          for (; i < readlen; i++)
		  {
               if (pos == 0  &&  (buf[i] & 0xc0))
			   {
                    continue;
			   }
			   packet[pos++] = buf[i];

			   if (pos == 3)
			   {
					pos = 0;
					butt = (packet[0] & 0x07);
					dx = (packet[0] & 0x10) ?   packet[1]-256  :  packet[1];
					dy = (packet[0] & 0x20) ? -(packet[2]-256) : -packet[2];

					// button zmena
					if (last_buttons != butt)
					{
						 changed_buttons = last_buttons ^ butt;
						 last_buttons = butt;
						 i++;	// extra byte
						 break;
					}

					static const int accel_limit = 5;
               		static const int accel = 2;

					if ( labs(dx) > accel_limit || labs(dy) > accel_limit )
					{
						dx *= accel;
						dy *= accel;
					}
/*
					if (labs(dx) > 8 || labs(dy) > 8)
					{
						dx *= 2;
						dy *= 2;
					}
					if (labs(dx) > 20 || labs(dy) > 20)
					{
						dx *= 2;
						dy *= 2;
					}
*/
					// update position
				    x_motion += dx;
				    y_motion += dy;
					i++;
					break;
               }
          }
          /* make sure the compressed motion event is dispatched,
             necessary if the last packet was a motion event */

		if (pos == 0) // alligned packet
		{
			if (changed_buttons)
			{
				buttons = butt;
			}
			else
			{
//				buttons = 0;
				if (mousex + x_motion < 0)
					mousex = 0;
				else if (mousex + x_motion > (w-1))
					mousex = w-1;
				else 	mousex += x_motion;

				if (mousey + y_motion < 0)
					mousey = 0;
				else if (mousey + y_motion > (h-1))
					mousey = h-1;
				else 	mousey += y_motion;
				x_motion = y_motion = 0;
			}

			x = mousex;
			y = mousey;
			type = MOUSEEVENT;
			key  = 0;
			return 1;
		}
     }
	 return 0; // no data
}

void FGLinuxDriver::ps2_mouse_deinit(void)
{
     close( msefd );
	 ps2_mouse = 0;
}

int  FGLinuxDriver::mice_mouse_init(void)
{
	msefd = open( "/dev/input/mice", O_RDWR | O_NONBLOCK );
	
	if (msefd < 0)
	{
	   printf( "OpenGUI: Error opening `/dev/input/mice' !\n" );
	   return -1;
	}
	unsigned char mousedev_imex_seq[] = { 0xf3, 200, 0xf3, 200, 0xf3, 80 };
	write (msefd, mousedev_imex_seq, 6);
	
	mice_mouse = 1;
	mousex = (w-1) / 2;
	mousey = (h-1) / 2;
	maxfd = msefd + 1;
	FD_ZERO(&fdset);
	FD_SET(msefd, &fdset);
	return 0;
}

/*
static void mousedev_packet(struct mousedev_list *list, signed char *ps2_data)
{
	struct mousedev_motion *p;
	unsigned long flags;

	p = &list->packets[list->tail];

	ps2_data[0] = 0x08 | ((p->dx < 0) << 4) | ((p->dy < 0) << 5) | (p->buttons & 0x07);
	ps2_data[1] = mousedev_limit_delta(p->dx, 127);
	ps2_data[2] = mousedev_limit_delta(p->dy, 127);
	p->dx -= ps2_data[1];
	p->dy -= ps2_data[2];

	switch (list->mode) {
		case MOUSEDEV_EMUL_EXPS:
			ps2_data[3] = mousedev_limit_delta(p->dz, 7);
			p->dz -= ps2_data[3];
			ps2_data[3] = (ps2_data[3] & 0x0f) | ((p->buttons & 0x18) << 1);
			list->bufsiz = 4;
			break;

		case MOUSEDEV_EMUL_IMPS:
			ps2_data[0] |= ((p->buttons & 0x10) >> 3) | ((p->buttons & 0x08) >> 1);
			ps2_data[3] = mousedev_limit_delta(p->dz, 127);
			p->dz -= ps2_data[3];
			list->bufsiz = 4;
			break;

		case MOUSEDEV_EMUL_PS2:
		default:
			ps2_data[0] |= ((p->buttons & 0x10) >> 3) | ((p->buttons & 0x08) >> 1);
			p->dz = 0;
			list->bufsiz = 3;
			break;
	}

	if (!p->dx && !p->dy && !p->dz) {
		if (list->tail == list->head) {
			list->ready = 0;
			list->last_buttons = p->buttons;
		} else
			list->tail = (list->tail + 1) % PACKET_QUEUE_LEN;
	}
}
*/

// type, key, x, y, buttons
int  FGLinuxDriver::mice_mouse_get(int& type, long& key, int& x, int& y, int& buttons)
{
    char packet[4];
    static unsigned char last_buttons = 0;
	static int x_motion=0;
	static int y_motion=0;
	static int bcount = 0;
    int readlen=0;
	static int next_butt = 0;
	unsigned char changed_buttons=0;

	if (bcount)
	{
		static bool trigger = 0;
		bcount--;
		x = mousex;
		y = mousey;
		type = MOUSEEVENT;
		key  = 0;
		last_buttons = buttons = trigger ? next_butt : 0;
		trigger = !trigger;
		return 1;
	}

	oldy = mousey;
	oldx = mousex;

	readlen = read(msefd, packet, 4);

	if (readlen == 4)
	{
          int dx, dy;
          int butt=0;
//printf("%d %d %d %d\n", packet[0], packet[1], packet[2] , packet[3] );
		butt = (packet[0] & 0x07); // | ((packet[3]>>1) & 0x18);
		dx = packet[1];
		dy = -packet[2];

		if (packet[3]&0x0f) // wheel?
		  {
			if (packet[3]&0x08)
			   {
//				butt |= FG_BUTTON_WHEEL_UP;
				next_butt = FG_BUTTON_WHEEL_UP;
				bcount = 2;
			   }
			else
			   {
//				butt |= FG_BUTTON_WHEEL_DOWN;
				next_butt = FG_BUTTON_WHEEL_DOWN;
				bcount = 2;
			}
		}

					// button zmena
					if (last_buttons != butt)
					{
						 changed_buttons = last_buttons ^ butt;
						 last_buttons = butt;
					}

					static const int accel_limit = 5;
   		static const int accel = 2;

					if ( labs(dx) > accel_limit || labs(dy) > accel_limit )
					{
						dx *= accel;
						dy *= accel;
					}

					// update position
				    x_motion += dx;
				    y_motion += dy;

			if (changed_buttons)
			{
				buttons = butt;
			}
			else
			{
				if (mousex + x_motion < 0)
					mousex = 0;
				else if (mousex + x_motion > (w-1))
					mousex = w-1;
				else 	mousex += x_motion;

				if (mousey + y_motion < 0)
					mousey = 0;
				else if (mousey + y_motion > (h-1))
					mousey = h-1;
				else 	mousey += y_motion;
				x_motion = y_motion = 0;
			}

			x = mousex;
			y = mousey;
			type = MOUSEEVENT;
			key  = 0;
			return 1;
		}
	 return 0; // no data
}

void FGLinuxDriver::mice_mouse_deinit(void)
{
     close( msefd );
	 mice_mouse = 0;
}

int	FGLinuxDriver::available(void)
{
	/* Check to see if we are na and stdin is a virtual console */
	uid = geteuid();
	gid = getegid();
	setuid(0);
	setgid(0);
	root = 0;
//	if (verbose) printf("UID = %d, EUID = %d\n", getuid(), geteuid());
	if (verbose && uid)
	{
		printf("WARNING: You don't have ROOT privilegia!\a\n\n");
	}
	else root=1;
	// set HW input/output privilegia ON
	if (root) iopl(3);
	return true;
}

FGLinuxDriver::FGLinuxDriver(char *n) : FGDriver(n)
{
	maxfd = 0;
	mtrr = 0;
	n_arguments = 1;
	tty_fd = -1;
	devtype = 0;
	but = 0;
	svgalib = 0;
	switching = 0;
	tty_no = -1;
	sbak = 0;
	vgamouse = 0;
	gpmmouse = 0;
	mice_mouse = 0;
	uid = gid = 0;
	dgalib = 0;
	dgamouse = 0;
	x11lib = 0;
	x11mouse =0;
	visible = 0;

	ptr_vga_init = 0;
	ptr_vga_setmode = 0;
	ptr_vga_hasmode = 0;
	ptr_vga_getmodeinfo = 0;
	ptr_vga_getgraphmem = 0;
	ptr_vga_getmousetype = 0;
	ptr_vga_setlinearaddressing = 0;
	ptr_vga_setdisplaystart = 0;
	ptr_mouse_init = 0;
	ptr_mouse_close = 0;
	ptr_disable = 0;
	ptr_waitretrace = 0;
	ptr_mouse_update = 0;
	ptr_mouse_setposition = 0;
	ptr_mouse_setxrange = 0;
	ptr_mouse_setyrange = 0;
	ptr_mouse_setscale = 0;
	ptr_mouse_getx = 0;
	ptr_mouse_gety = 0;
	ptr_mouse_getbutton = 0;
	ptr_vga_lastmodenumber = 0;
	ptr_tty_fd = 0;
	caps_lock = num_lock = 0;
}

FGLinuxDriver::~FGLinuxDriver()
{
	if (mtrr)
	{
		undoWC(mtrr);
	}
	mtrr = 0;

	if (mmio)
	{
		if (munmap((void *)mmio, mmio_size)!=0)
			perror("mmio release!");
	}
	mmio = 0;
	reset_signals();
}

int FGLinuxDriver::GetGPM(int& i, char **argv)
{
	static char arg[80], *p;
	int pid;

	i = 0;
	FILE * f = fopen("/var/run/gpm.pid", "r");
	if (f)
	{
		fscanf(f, "%d", &pid);
		fclose(f);
		sprintf(arg, "/proc/%d/cmdline", pid);
		f = fopen(arg, "r");
		if (f)
		{
			i = 0;
			memset(arg, 0, sizeof(arg));
			fread(arg, 1, sizeof(arg), f);
			p = arg;
			while(strlen(p)!=0 && i<8)
			{
				argv[i] = p;
				p = p + strlen(p)+1;
				i++;
			}
			fclose(f);
			return pid;
		}
		return 0;
	}
	else // gpm isn't running, parse /ets/sysconfig files for arguments
	{
		i=1;
//		if (verbose) printf("GPM not running - trying prepare params from '/etc/config'\n");
		f = fopen("/etc/sysconfig/gpm", "r");
		if (f)
		{
			static char a1[32];
			while (fgets(arg, 79, f) > 0)
			{
				if (sscanf(arg, "DEVICE=\"%s\"", a1) == 1)
				{
					argv[i++]="-m ";
					p = strrchr(a1,'\"');
					if (p) *p=0;
					argv[i++] = a1;
					break;
				}
			}
			fclose(f);
		}
		f = fopen("/etc/sysconfig/mouse", "r");
		if (f)
		{
			static char a2[32];
			while (fgets(arg, 79, f) > 0)
			{
				if (sscanf(arg, "MOUSETYPE=\"%s\"", a2) == 1)
				{
					argv[i++]="-t ";
					p = strrchr(a2,'\"');
					if (p) *p=0;
					argv[i++] = a2;
					break;
				}
			}
			fclose(f);
		}
	}
	return 0;
}

void FGLinuxDriver::execute(char **argv, int argc)
{
	char s[80]="";
	int i=0;

	while(argc--)
	{
		strcat(s, argv[i++]);
		strcat(s, " ");
	}
	if (verbose) printf("Running '%s'\n", s);
	system(s);
}

//
// 1) save current gpm configuration
// 2) set new state of gpm where:
//	which=0 : run standard [svgalib] 'gpm -k'
//	which=1 : run in "SystemxMouse" mode
//
void FGLinuxDriver::RunGPM(int which)
{
	int i;

	// save for restore
	old_pid = GetGPM(n_arguments, arguments);

	*arguments = *strs;
	new_pid = 0;
	if (which == 0) return; // no job

	for (i=0; i<n_arguments; i++)
	{
		// return if SystemxMouse and GPM
		if (!strcmp(arguments[i], "-R") && which)
		{
			old_pid = 0; // do not restart
			return;
		}
	}
	if (which==0)
	{
		old_pid = 0;
		return;
	}
	// kill current gpm = RESTART
	if (old_pid)
	{
		execute(strs, 2);
		delay(200);
	}
	else
	{
		arguments[0]=strs[0];
	}
	// and now run new gpm
	arguments[n_arguments] = strs[2];
	execute(arguments, n_arguments+1);
	delay(200);
	new_pid = 1;
}

void FGLinuxDriver::RestoreGPM(void)
{
	// kill current gpm
	if (new_pid) execute(strs, 2);
	if (old_pid) execute(arguments, n_arguments);
	new_pid = old_pid = 0;
}

int FGLinuxDriver::mouseopen(void)
{
	if (msefd != -1)
	{
		return msefd;
	}
	devtype = DEV_TYPE_GPMDATA;
	if ((msefd = open(GPMDATADEVICE, O_RDWR | O_NDELAY)) < 0)
	{
	    perror("device open: GPMDATA");
		return msefd;
	}

	mousex = (w-1) / 2;
	mousey = (h-1) / 2;
	return msefd;
}

void FGLinuxDriver::mouseclose(void)
{
	if (msefd == -1)
	{
		return;
	}
	close(msefd);
	msefd = -1;
}

int FGLinuxDriver::checkmouse(int& type, long& key, int& x, int& y, int& buttons)
{
	signed char buf[8];
	int dx, dy;

	if (!visible || msefd==-1) return 0;

	oldy = mousey;
	oldx = mousex;

	int size_to_read = (devtype == DEV_TYPE_GPMDATA ? 5 : 3);
	if (read(msefd, &buf, size_to_read) == -1)
	{
		return 0;
	}
	if (buf[0]==0) return 0; // ????
	but = (~buf[0]) & 0x7;

	dx = (int) ((buf[3] * 1));
	dy = (int) (-((buf[4] * 1)));

	static const int accel_limit = 5;
	static const int accel = 2;

	if ( labs(dx) > accel_limit || labs(dy) > accel_limit )
	{
		dx *= accel;
		dy *= accel;
	}
/*
	if (labs(dx) > 4 || labs(dy) > 4)
	{
		dx *= 2;
		dy *= 2;
	}
	if (labs(dx) > 10 || labs(dy) > 10)
	{
		dx *= 2;
		dy *= 2;
	}
*/
	if (mousex + dx < 0)
		mousex = 0;
	else if (mousex + dx > (w-1))
		mousex = w-1;
	else 	mousex += dx;

	if (mousey + dy < 0)
		mousey = 0;
	else if (mousey + dy > (h-1))
		mousey = h-1;
	else 	mousey += dy;

	x = mousex;
	y = mousey;
	if ((but&7) == 4)
	{
		buttons = FG_BUTTON_LEFT;
	}
	else if ((but&7) == 1)
	{
		buttons = FG_BUTTON_RIGHT;
	}
	else  if ((but&7) == 2)
	{
		buttons = FG_BUTTON_MIDDLE;
	}
        else  buttons = 0;

	type = MOUSEEVENT;
	key = 0;
	return 1;
}

void FGLinuxDriver::linux_postinit(void)
{
	if (linear_base)
		mtrr = setWC((unsigned long)linear_base&(0xFFF<<20), (linear_size+0xFFFFF)&(0xFFF<<20), 1, verbose);

	if (mmio_size && mmio_base)
	{
		int mem_fd=open("/dev/mem", O_RDWR);
		if (mem_fd < 0)
		{
		    printf("Cannot open /dev/mem!\n");
		}
		mmio = (unsigned char *)mmap((caddr_t) 0, mmio_size, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd,(__off_t)mmio_base);
		if (mmio == (unsigned char *)-1)
		{
			perror("mmio mapping");
			mmio = 0;
		}
   		close(mem_fd);
	}
	else mmio=0;
	set_signals();
}

int FGLinuxDriver::set_mouse(void)
{
	if (fgstate.force_mice)
		mice_mouse_init();

	if (mice_mouse)
	{
		devtype = DEV_TYPE_MICE;
		mouse_string = "MOUSEDEV";
		return ismouse = true;
	}

	if (fgstate.force_ps2)
		ps2_mouse_init();

	if (ps2_mouse)
	{
		devtype = DEV_TYPE_PS2;
		mouse_string = "PS/2 MOUSE";
		return ismouse = true;
	}

	{
		RunGPM(1); 			// re-run gpm if needed

		if (mouseopen() >= 0)	// try open, if still not opened
		{
			gpmmouse = 1;		// set the result
			mouse_string = "GPM_MOUSE";
		}
		else perror("OpenGUI: could not initialize gpm mouse\n");

		if (gpmmouse==0 && svgalib==1)	// if error and SVGALIB mode, try also svgalib mouse driver
		{
			RunGPM(0); // kill gpm
			vgamouse = ptr_mouse_init( "/dev/mouse", ptr_vga_getmousetype(), MOUSE_DEFAULTSAMPLERATE)==-1?0:1;
			if (vgamouse==0)
			{
				if (verbose)
					printf("OpenGUI: could not initialize svgalib mouse\n");
			}
			else
			{
				ptr_mouse_setxrange(0,(X_width - 1));
				ptr_mouse_setyrange(0,(Y_width-1));
				ptr_mouse_setposition(X_width/2, Y_width/2);
				ptr_mouse_setscale(1);
				mouse_string = "SVGA_MOUSE";
			}
		}
	}

	return ismouse = (vgamouse || gpmmouse || ps2_mouse);
}

void FGLinuxDriver::reset_mouse(void)
{
	if (vgamouse)
	{
		ptr_mouse_close();
		vgamouse = 0;
		RestoreGPM();
	}
	else if (gpmmouse)
	{
		mouseclose();
		gpmmouse = 0;
		RestoreGPM();
	}
	else if (ps2_mouse)
	{
		ps2_mouse_deinit();
	}
	else if (mice_mouse)
	{
		mice_mouse_deinit();
	}
	ismouse = 0;
}

int FGLinuxDriver::set_keyboard(void)
{
	if (svgalib==1)
	{
		if ((kbd_fd = tty_fd = *ptr_tty_fd)==-1)
		{
			printf("Could not get tty\n");
			exit(-1);
		}
	}
	else kbd_fd = tty_fd;

	if ((kbd_fd=GII_keyboard_init(kbd_fd)) < 0)
	{
		printf("Could not initialize keyboard\n");
		ioctl(kbd_fd, KDSKBMODE, K_XLATE);
		exit(-1);
	}
	iskeyboard = 1;
	return true;
}

void FGLinuxDriver::reset_keyboard(void)
{
	if (iskeyboard) GII_close(); // close keyboard
	ioctl(kbd_fd, KDSKBMODE, K_XLATE);
	iskeyboard = 0;
}

int	FGSVGADriver::link(void)
{
		detect();

		subname = "default";
		lib = new Dll("libvga.so.1", RTLD_LAZY | RTLD_GLOBAL);
		if (lib->GetResult()) return false;
		ptr_vga_init = iv(lib->GetAddr("vga_init"));
		ptr_vga_setmode = ii(lib->GetAddr("vga_setmode"));
		ptr_vga_hasmode = ii(lib->GetAddr("vga_hasmode"));
		ptr_vga_setpage = vi(lib->GetAddr("vga_setpage"));
		ptr_vga_setdisplaystart = vi(lib->GetAddr("vga_setdisplaystart"));
		ptr_vga_getmodeinfo = (vga_modeinfo *(*)(int)) lib->GetAddr("vga_getmodeinfo");
		ptr_vga_getgraphmem = (unsigned char * (*)(void))lib->GetAddr("vga_getgraphmem");
		ptr_vga_getmousetype = iv(lib->GetAddr("vga_getmousetype"));
		ptr_vga_setlinearaddressing = iv(lib->GetAddr("vga_setlinearaddressing"));
		ptr_waitretrace = vv(lib->GetAddr("vga_waitretrace"));

		ptr_mouse_init = (int (*)(char *,int,int))lib->GetAddr("mouse_init");
		ptr_mouse_close = vv(lib->GetAddr("mouse_close"));
		ptr_mouse_update = iv(lib->GetAddr("mouse_update"));
		ptr_mouse_setposition = vii(lib->GetAddr("mouse_setposition"));
		ptr_mouse_setxrange = vii(lib->GetAddr("mouse_setxrange"));
		ptr_mouse_setyrange = vii(lib->GetAddr("mouse_setyrange"));
		ptr_mouse_setscale = vi(lib->GetAddr("mouse_setscale"));
		ptr_mouse_getx = iv(lib->GetAddr("mouse_getx"));
		ptr_mouse_gety = iv(lib->GetAddr("mouse_gety"));
		ptr_mouse_getbutton = iv(lib->GetAddr("mouse_getbutton"));
		ptr_disable = (vv)lib->GetAddr("vga_disabledriverreport");
		ptr_vga_lastmodenumber = (iv)lib->GetAddr("vga_lastmodenumber");
		ptr_tty_fd = (int *)lib->GetAddr("__svgalib_tty_fd");

		ptr_disable();
		ptr_vga_init();
		if (mmio_size==0)
		{
			linear_base = (void *)*(long *)lib->GetAddr("__svgalib_linear_mem_base");
			linear_size = *(int *)lib->GetAddr("__svgalib_linear_mem_size");
			mmio_base = (void *)*(long *)lib->GetAddr("__svgalib_mmio_base");
			mmio_size = *(int *)lib->GetAddr("__svgalib_mmio_size");
		}
		linux_postinit();
		if (verbose && ioperm(0x3b0, 0x3df - 0x3b0 + 1, 1))
		{
			printf("Cannot get permissions for I/O operations\n");
			return 0;
		}
		return true;
}

int FGSVGADriver::set_mode(int ww, int hh)
{
	req_w = ww;
	req_h = hh;

	FGDriver::set_mode(w,h);
	mode_num = find(req_w, req_h);

	if (mode_num==-1)
	{
		printf("Mode %dx%d not found!\n", ww, hh);
		return 0;
	}

	int rc = ptr_vga_hasmode(modelist[mode_num].mode);
	if (rc)
	{
		if (verbose) printf("\tmode \"%dx%dx%d\" as %d passed ... ", modelist[mode_num].w, modelist[mode_num].h, bpp*8, modelist[mode_num].mode);
		rc = ptr_vga_setmode(modelist[mode_num].mode);
		if (rc==-1) return 0;
		if (verbose) printf("O.K.\n");
		int sz = ptr_vga_setlinearaddressing();
		if (sz>0 || (ww==320 && hh==200 && bpp==1))
		{
			islinear=1;
		}
		else
		{
			printf("Ooops, this version of the library support only linear framebuffer!\a\n");
			exit(-1);
		}
		videobase = (FGPixel *)ptr_vga_getgraphmem();
		visible = 1;
		synch = 0;
		virt_w = req_w;
		virt_h = req_h;
		return postinit();
	}
	else printf("\tmode \"%dx%dx%d\" as %d not available!\n", modelist[mode_num].w, modelist[mode_num].h, bpp*8, modelist[mode_num].mode);

	return 0;
}

int FGLinuxDriver::get_event(int& type, long& key, int& x, int& y, int& buttons)
{
	int bb;

	if (iskeyboard)
		if ((key = GII_get_key()) != 0)
	{
		type = KEYEVENT;
		x = y = buttons = 0;
		return 1;
	}

	if (ismouse==0) return 0; // no mouse driver

	if (vgamouse)
	{
		if (ptr_mouse_update())
		{
			x = ptr_mouse_getx();
			y = ptr_mouse_gety();
			bb = ptr_mouse_getbutton();
			buttons = 0;
			if (bb&MOUSE_RIGHTBUTTON) buttons |= FG_BUTTON_RIGHT;
			if (bb&MOUSE_LEFTBUTTON) buttons |=	FG_BUTTON_LEFT;
			if (bb&MOUSE_MIDDLEBUTTON) buttons |=	FG_BUTTON_MIDDLE;
			key  = 0;
			type = MOUSEEVENT;
			return 1;
		}
	}
	else if (gpmmouse)
	{
		if (checkmouse(type, key, x, y, buttons))
			return 1;
	}
	else if (ps2_mouse)
	{
		if (ps2_mouse_get(type, key, x, y, buttons))
			return 1;
	}
	else if (mice_mouse)
	{
		if (mice_mouse_get(type, key, x, y, buttons))
			return 1;
	}
	return 0;
}

int	FGFBDriver::link(void)
{
		/* Open the file for reading and writing */
		if (!fbname)
		{
			/* Try to set name from the environment */
			fbname = (char *) getenv("FRAMEBUFFER");
			if (!fbname)
			{
				fbname = "/dev/fb0";
			}
		}
		if (getenv("IOPERM") == NULL)
		{
			if (ioperm(0x3b0, 0x3df - 0x3b0 + 1, 1))
			{
				printf("Cannot get permissions for I/O operations\n");
				return 0;
			}
		}
		fbfd = open(fbname, O_RDWR);
	    if (fbfd<0)
		{
			fbname = "/dev/fb/0";
		    fbfd = open(fbname, O_RDWR);
        }
	    if (fbfd<0)
		{
			detect();
			switch(driver && root)
			{
				case FG_NVIDIA:
					system("modprobe rivafb");
					break;
				case FG_MATROX:
					system("modprobe matroxfb_base");
					break;
				case FG_SIS:
					system("modprobe sisfb");
					break;
				case FG_ATI:
					system("modprobe atyfb");
					break;
				case FG_CIRRUS:
					system("modprobe clgenfb");
					break;
				case FG_BANSHEE:
					system("modprobe tdfxfb");
					break;
				case FG_TRIDENT:
					system("modprobe tridentfb");
					break;
                default:
					return 0;			// not detected
			}
		    fbfd = open(fbname, O_RDWR);
		    if (fbfd<0)
			{
				fbname = "/dev/fb0";
			    fbfd = open(fbname, O_RDWR);
            }
		    if (fbfd<0)
				return 0;			// return if FBDEV is not configured
        }

	    /* Get fixed screen information */
	    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
		{
	        printf("Error reading fixed information\n");
			return 0;
	    }
	    /* Get variable screen information */
	    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo))
		{
	        printf("Error: reading variable information\n");
			return 0;
	    }
		memcpy(&text_vinfo, &vinfo, sizeof(vinfo));
		subname = finfo.id;
		if (!strcmp(finfo.id, "VESA VGA"))
		{
			printf("fixed mode %s - colordepth & resolution change doesn't work anymore!!!\n", finfo.id);
			printf("it is highly recomended to use the native fb device (like radeonfb i.e.)!\n");
			fixed_mode = 1;
        }
		linear_base = (void *)finfo.smem_start;
		linear_size = finfo.smem_len;
		mmio_base = (void *)finfo.mmio_start;
		mmio_size = finfo.mmio_len;
		vtopen();
		map();
		linux_postinit();
		return true;
}

int FGFBDriver::map(void)
{
	/* Map fb into memory */
	if ((videobase = (FGPixel *) mmap((void *) 0,
		linear_size,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fbfd,
		0)) == (void *) -1)
	{
		perror("map: fb mmap failed");
		return 0;
	}
	visible = 1;
	return mapped=1;
}

void FGFBDriver::unmap(void)
{
	/* Unmap framebuffer from memory */
	if (visible && (munmap((void *) videobase, linear_size) == -1))
	{
		perror("unmap: munmap failed");
	}
	visible = 0;
	mapped=0;
}

static void VTswitch(int s)
{
	((FGFBDriver *)__fg_driver)->vtswitch(s);
}

int FGFBDriver::vtopen(void)
{
	char ttynam[11];
	int i = 0;
	struct vt_stat vts;

	/* Open current console */

	while ((tty_fd = open(ttynames[cttyname], O_WRONLY)) < 0)
	{
		printf("vtopen: open failed on %s\n", ttynames[cttyname]);

		if (ttynames[++cttyname] == NULL)
		{
			perror("vtopen: failed to open a tty");
			return 0;
		}
		else
		{
			printf("vtopen: trying %s\n", ttynames[cttyname]);
		}
	}

	/* Get number of free VT */

	if (ioctl(tty_fd, VT_OPENQRY, &tty_no) == -1)
	{
		perror("vtopen: no free ttys");
		return 0;
	}

	/* Close current console */

	if (close(tty_fd) == -1)
	{
		printf("vtopen: failed to close %s\n",
				ttynames[cttyname]);
	}
#ifdef CLOSE_STDOUT
	close(0);
	close(1);
	close(2);
#endif
	do	{
		(void) sprintf(ttynam, ttyfmts[i++], tty_no);
	}
	while ((ttyfmts[i] != NULL) &&
		   (tty_fd = open(ttynam, O_RDWR)) == -1);

	if (tty_fd == -1)
	{
		printf("vtopen: failed to open %s\n", ttynam);
		return 0;
	}
#ifdef CLOSE_STDOUT
	dup(tty_fd);
	dup(tty_fd);
	setsid();
#endif
	if ((i = open("/dev/tty", O_RDWR)) >= 0)
	{
		ioctl(i, TIOCNOTTY, 0);
		close(i);
	}

	if (ioctl(tty_fd, VT_GETSTATE, &vts) == -1)
	{
		perror("vtopen: couldn't get VT state");
		return 0;
	}
	originaltty = vts.v_active;

	/* Switch to new VT */

	if (ioctl(tty_fd, VT_ACTIVATE, tty_no) == -1)
	{
		printf("vtopen: couldn't switch to VT %d\n", tty_no);
	}

	/* Wait for new VT to become active */

	if (ioctl(tty_fd, VT_WAITACTIVE, tty_no) == -1)
	{
		printf("vtopen: VT %d didn't become active\n", tty_no);
		return 0;
	}
	// ---
//	ioctl( ttyfd, VT_DISALLOCATE, vt->num );

	visible = true;

	/* Get mode of new VT */
	if (ioctl(tty_fd, VT_GETMODE, &vtm) == -1)
	{
		printf("vtopen: Couldn't get mode of VT %d\n", tty_no);
		return 0;
	}

	/* Adjust mode parameters */
	vtm.mode = VT_PROCESS;
	vtm.relsig = MY_SIGNAL;
	vtm.acqsig = MY_SIGNAL;

	/* Set signal handler for VT switches */
	(void) signal(MY_SIGNAL, VTswitch);

	/* Set new mode parameters */
	if (ioctl(tty_fd, VT_SETMODE, &vtm) == -1)
	{
		printf("vtopen: Couldn't set mode of VT %d\n", tty_no);
	}

	/* Disable cursor */
	if (ioctl(tty_fd, KDSETMODE, KD_GRAPHICS) == -1)
	{
		printf("vtopen: Couldn't set keyboard graphics mode on VT %d\n", tty_no);
	}
	return 1;
}

void FGFBDriver::vtswitch(int s)
{
	int	size = X_virtual*Y_width*bpp;
	/* Set this handler again, otherwise signal reverts to default handling */

	(void) signal(MY_SIGNAL, VTswitch);

	/* If the switch has already been acknowledged, and the user tries to */
	/* switch again, ignore until the alarm signal is raised. */

	if (switching && (s == MY_SIGNAL))
	{
		return;
	}

	if (switching == false)
	{
		switching = true;
	}

	/* If drawing, a change in the screen buffer will cause problems: */
	/* delay switch and hope next time it isn't drawing. */
	if (visible)
	{
		/* Switching out, allocate new backing store */
		if (!sbak)
			sbak = (FGPixel *) malloc(size);
		if (sbak == NULL)
		{
			perror("vtswitch: failed to allocate backing store\n");
			exit(-1);
		}
		/* Copy framebuffer to backing store */

		if (sbak && videobase)
			memcpy(sbak, videobase, size);

		/* Unmap current framebuffer */
		unmap();

		/* Make backing store current so it can be written to */
		videobase = sbak;
		fgl::videobase = videobase;
		visible = false;

		/* Release console for switch */

		if (ioctl(tty_fd, VT_RELDISP, 1) == -1)
		{
			printf("vtswitch: switch away from VT %d denied", tty_no);
			exit(-1);
		}
	}
	else
	{
#ifdef INDEX_COLORS
		_set_fgl_palette();
#endif
		if (ioctl(tty_fd, VT_RELDISP, VT_ACKACQ) == -1)
		{
			printf("vtswitch: switch to VT %d denied", tty_no);
		}
		map();
		fgl::videobase = videobase;
		if (sbak && videobase) memcpy(videobase, sbak, size);
		visible = true;
	}
	switching = false;
}

int FGFBDriver::vtclose(void)
{
	/* If backing store allocated, free it */
	if (sbak)
	{
		free(sbak);
	}

	/* Enable cursor */
	ioctl(tty_fd, KDSETMODE, KD_TEXT);

	/* Restore mode parameters */
	vtm.mode = VT_AUTO;

	/* Set new mode parameters */
	if (ioctl(tty_fd, VT_SETMODE, &vtm) == -1)
	{
		printf("vtclose: Couldn't set mode of VT %d\n", tty_no);
	}

	/* Switch back to original VT */

	if (ioctl(tty_fd, VT_ACTIVATE, originaltty) == -1)
	{
		printf("vtclose: couldn't switch to VT %d\n", originaltty);
		return 0;
	}

	/* Close VT */
#ifdef CLOSE_STDOUT
	close(tty_fd+1);
	close(tty_fd+2);
#endif
	if (close(tty_fd) == -1)
	{
		printf("vtclose: failed to close VT\n");
		return 0;
	}
	tty_fd = -1;
	return true;
}

void FGFBDriver::get_all_modes(vmode *p)
{
	for (int m=0; modedb[m].refresh; m++)
	{
		p[total_modes].flag = FG_LINEAR;
		p[total_modes].refresh = modedb[m].refresh;
		p[total_modes].w = modedb[m].xres;
		p[total_modes].h = modedb[m].yres;
		p[total_modes].mode = m;
		total_modes++;
	}
}

int FGFBDriver::set_mode(int ww, int hh)
{
	static struct	fb_var_screeninfo tmp;
	req_w = ww;
	req_h = hh;

	FGDriver::set_mode(w,h);
	// build requested mode ...
	if (fixed_mode==0 && fgstate.nofbset==0)
	{
		tmp.bits_per_pixel = bpp*8;
		tmp.xres_virtual = req_w;
		tmp.yres_virtual = req_h;
		mode_num = fb_find_mode(&tmp, req_w, req_h, synch?synch : (req_w<1024?77:62));

		{
			// set default monitor sync to 60/75 Hz
			if (mode_num<0) return false;
			tmp.activate = FB_ACTIVATE_TEST;

			tmp.yres_virtual = req_h*5; // maxbuffers + 1
			int result = -1;
			// try set mode with height for four buffers
			while(result!=0 && (int)tmp.yres_virtual>req_h)
			{
				tmp.yres_virtual -= req_h;
				if ((result=ioctl(fbfd, FBIOPUT_VSCREENINFO, &tmp)) == 0)
				{
					tmp.activate = FB_ACTIVATE_NOW;
					result = ioctl(fbfd, FBIOPUT_VSCREENINFO, &tmp);
					ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo);
				}
			}
			if (verbose) synch = modedb[mode_num].refresh;
		}
		if (vinfo.bits_per_pixel != FASTGL_BPP)
		{
			printf("Can't change colors depth\n");
			return 0;
		}
		if (mode_num>=0 && verbose) printf("\tmode \"%dx%dx%d\" as %d passed ... OK\n", vinfo.xres, vinfo.yres, bpp*8, mode_num);
	}
	else // VESA FBDEV is FIXED
	{
		mode_num=-1;
		if (vinfo.bits_per_pixel != FASTGL_BPP)
		{
			printf("Can't change colors depth - try compile with switch '-D BPP%d' and link with apropriate version of OpenGUI (fgl, fgl16 or fgl32)\n", vinfo.bits_per_pixel);
			abort();
		}
	}

	islinear = 1;
	req_w = vinfo.xres;
	req_h = vinfo.yres;
	virt_w = vinfo.xres_virtual;
	virt_h = vinfo.yres_virtual;
	pitch  = finfo.line_length;
	if (fixed_mode == 1 && pitch != virt_w)
	{
		printf("Oops, FB driver bug detected - bad virtual width (%d) reported, (%d) forced\n", virt_w, pitch);
		virt_w = pitch;
	}
	if (vinfo.red.length!=6)
	fgstate.palette_8=1;

	return postinit();
}

#define res_matches(v, x, y) ((v).xres == (x) && (v).yres == (y))

int FGFBDriver::fb_find_mode(struct fb_var_screeninfo *var, unsigned int xres, unsigned int yres, unsigned int refresh)
{
	int i, j;
	int dbsize = sizeof(modedb)/sizeof(*modedb)-1;

	for (i = refresh; i >= 40; i--)
	{
		for (j = 0; j < dbsize; j++)
		{
			if (res_matches(modedb[j], xres, yres) && ((unsigned )i==modedb[j].refresh || i==40 || modedb[j].refresh == refresh))
			{
				const fb_videomode *mode = &modedb[j];
				var->xres = mode->xres;
				var->yres = mode->yres;
				var->xres_virtual = mode->xres;
				var->yres_virtual = mode->yres;
				var->pixclock = mode->pixclock;
				var->left_margin = mode->left_margin;
				var->right_margin = mode->right_margin;
				var->upper_margin = mode->upper_margin;
				var->lower_margin = mode->lower_margin;
				var->hsync_len = mode->hsync_len;
				var->vsync_len = mode->vsync_len;
				var->sync = mode->sync;
				var->vmode = mode->vmode;
				if (verbose) printf("trying mode %s %dx%d-%d@%d\n", mode->name ? mode->name : "noname", mode->xres, mode->yres, bpp*8, mode->refresh);
				return j;
			}
		}
	}
	if (verbose) printf("No valid mode found\n");
	return -1;
}

void FGSVGADriver::get_all_modes(vmode *p)
{
	vga_modeinfo *modeinfo;

	for (int m=1; m<=ptr_vga_lastmodenumber(); m++)
	{
		if (ptr_vga_hasmode(m)==0)
			continue;
		modeinfo = ptr_vga_getmodeinfo(m);
		if (modeinfo==0)
			continue;
		if (modeinfo->bytesperpixel != bpp)
			continue; // not at the end, not good mode
#if FASTGL_BPP==15
		if (modeinfo->colors != 32768)
			continue; // not at the end, not good mode
#endif

// fixed 13.1.2005

#if FASTGL_BPP==16
		if (modeinfo->colors != 65536)
			continue; // not at the end, not good mode
#endif
		p->flag = (modeinfo->flags & CAPABLE_LINEAR)?FG_LINEAR:0;

		if (modeinfo->width == 320 && modeinfo->height == 200 &&  modeinfo->bytesperpixel == 1)
			p->flag = FG_LINEAR;    // FIX TO LINEAR/NAKED ISSUES

		p->w = modeinfo->width;
		p->h = modeinfo->height;
		p->mode = m;
		total_modes++;
		p++;
	}
}

void FGAPI Snd(int a, int b)
{
	if ((kbd_fd>=0) && (sound_enabled)) ioctl(kbd_fd, KDMKTONE, (b<<16)+((1193190/a)&0xffff));
}

#if defined(DGA_DRIVER) || defined(X11_DRIVER)

void FGLinuxDriver::process_modifiers(int state, bool pressed)
{
	switch(state)
	{
		case XK_Shift_L:
		case XK_Shift_R:
		    SetShiftState(pressed);
			break;
		case XK_Control_L:
		case XK_Control_R:
			SetControlState(pressed);
			break;
		case XK_Alt_L:
		case XK_Alt_R:
			SetAltState(pressed);
			break;
	}
}

int FGLinuxDriver::TranslateKey(XKeyEvent *xkey, bool pressed)
{
	KeySym xsym;
	int keysym=0x8000, ctrl=0,
	// 1=shift, 4=ctrl, 8=alt
	offset = (xkey->state&1?0x2000:0)+(xkey->state&8?0x3000:0)+(xkey->state&4?0x4000:0);

	xsym = XLookupKeysym(xkey, 0);
	if (xsym==0) return keysym;

	process_modifiers(xsym, pressed);
	if (pressed == false) return keysym;

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
		case 0xFF:
			if (offset==0) // no state key
			{
				if (((unsigned)xsym>=0xff08U && (unsigned)xsym<=0xff0dU) || xsym==0xff1b)
				{
					keysym = xsym&0xFF; // code 1..1f
					break;
				}
			}
			// test na SHIFT, CTRL etc
			if (xsym>= 0xffe1 && xsym <= 0xffea) break;
			keysym = (xsym-offset)&0xffff;
			switch(keysym)
			{
				case 0xffaf:
					keysym = '/';
					break;
				case 0xffaa:
					keysym = '*';
					break;
				case 0xffad:
					keysym = '-';
					break;
				case 0xff95:
					keysym = '7';
					break;
				case 0xff97:
					keysym = '8';
					break;
				case 0xff9a:
					keysym = '9';
					break;
				case 0xffab:
					keysym = '+';
					break;
				case 0xff96:
					keysym = '4';
					break;
				case 0xff9d:
					keysym = '5';
					break;
				case 0xff98:
					keysym = '6';
					break;
				case 0xff9c:
					keysym = '1';
					break;
				case 0xff99:
					keysym = '2';
					break;
				case 0xff9b:
					keysym = '3';
					break;
				case 0xff8d:
					keysym = CR;
					break;
				case 0xff9e:
					keysym = '0';
					break;
				case 0xff9f:
					keysym = '.';
					break;
				case 0xff7f:
					keysym = 0x8000;
					break;
			}
			break;
		default:
			return 0x8000;
	}
//printf("Translating key %x %x <- (%x %x %x):%d\n", xsym, keysym, xkey->type, xkey->keycode, xkey->state,caps_lock);
	return keysym;
}
#endif // common X11 stuff

#ifdef DGA_DRIVER

#define DGA_Screen		DefaultScreen(DGA_Display)
Display *FGDGADriver::DGA_Display;

int	FGDGADriver::available(void)
{
			const char *display;
			Display *dpy;
			int available=0;

			/* The driver is available is available if the display is local
			   and the DGA 2.0+ extension is available, and we can map mem.
			*/
			display = NULL;
			FGLinuxDriver::available();
			if (root == false)
			{
				printf("ROOT is required for DGA!\n");
				return false;
			}
			if ( (strncmp(XDisplayName(display), ":", 1) == 0) ||
				 (strncmp(XDisplayName(display), "unix:", 5) == 0) )
			{
				dpy = XOpenDisplay(display);
			if ( dpy )
			{
				int events, errors, major, minor;

				if ( XF86DGAQueryExtension(dpy, &events, &errors) &&
				     XF86DGAQueryVersion(dpy, &major, &minor) )
					{
						int screen;

						screen = DefaultScreen(dpy);
						if (major >= 2)
							 if ( XDGAOpenFramebuffer(dpy, screen) )
						{
							available = 1;
							XDGACloseFramebuffer(dpy, screen);
						}
						else printf("An old version of DGA extension (2.0 required)\n");
					}
					else printf("extension 'XFree86-DGA' missing\n");
				}
				XCloseDisplay(dpy);
			}
			return(available);
}

int	FGDGADriver::link(void)
{
	const char *display;
	int i, num_formats;
	int event_base, error_base;
	int major_version, minor_version;
	Visual *visual;
	int BitsPerPixel;
	XPixmapFormatValues *pix_format;

	/* Open the X11 display */
	display = NULL;		/* Get it from DISPLAY environment variable */

	DGA_Display = XOpenDisplay(display);

	if ( DGA_Display == NULL )
	{
		perror("Couldn't open X11 display");
		return 0;
	}

	/* Check for the DGA extension */
	if ( ! XDGAQueryExtension(DGA_Display, &event_base, &error_base) ||
	     ! XDGAQueryVersion(DGA_Display, &major_version, &minor_version) )
	{
		perror("DGA extension not available");
		XCloseDisplay(DGA_Display);
		return 0;
	}

	if ( major_version < 2 ) {
		perror("DGA driver requires DGA 2.0 or newer");
		XCloseDisplay(DGA_Display);
		return 0;
	}

	DGA_event_base = event_base;

	/* Determine the current screen depth */
	visual = DefaultVisual(DGA_Display, DGA_Screen);

	BitsPerPixel = DefaultDepth(DGA_Display, DGA_Screen);

	pix_format = XListPixmapFormats(DGA_Display, &num_formats);

	if ( pix_format == NULL )
	{
		perror("Couldn't determine screen formats");
		XCloseDisplay(DGA_Display);
		return 0;
	}

	for ( i=0; i<num_formats; ++i )
	{
		if ( BitsPerPixel == pix_format[i].depth )
			break;
	}

	if ( i != num_formats )
		BitsPerPixel = pix_format[i].bits_per_pixel;

	XFree((char *)pix_format);

	/* Open access to the framebuffer */
	if ( ! XDGAOpenFramebuffer(DGA_Display, DGA_Screen) )
	{
		perror("Unable to map the video memory");
		XCloseDisplay(DGA_Display);
		return 0;
	}

	if (ioperm(0x3b0, 0x3df - 0x3b0 + 1, 1))
	{
    	printf("Cannot get permissions for I/O operations\n");
	    return 0;
	}

	subname = "DGA 2 extension on Xfree86 server\n";
	linux_postinit();
	return true;
}

void PrintMode(XDGAMode *mode)
{
	if (!fgstate.verbose) return;
	printf("Mode: %s (%dx%d) at %d bpp (%f refresh, %d pitch) num: %d\n",
		mode->name,
		mode->viewportWidth, mode->viewportHeight,
		mode->depth == 24 ? mode->bitsPerPixel : mode->depth,
		mode->verticalRefresh, mode->bytesPerScanline, mode->num);
	printf("\tRGB: 0x%8.8lx 0x%8.8lx 0x%8.8lx (%u - %s)\n",
		mode->redMask, mode->greenMask, mode->blueMask,
		mode->visualClass,
		mode->visualClass == TrueColor ? "truecolor" :
		mode->visualClass == DirectColor ? "directcolor" :
		mode->visualClass == PseudoColor ? "pseudocolor" : "unknown");
	printf("\tFlags: ");
	if ( mode->flags & XDGAConcurrentAccess )
		printf(" XDGAConcurrentAccess");
	if ( mode->flags & XDGASolidFillRect )
		printf(" XDGASolidFillRect");
	if ( mode->flags & XDGABlitRect )
		printf(" XDGABlitRect");
	if ( mode->flags & XDGABlitTransRect )
		printf(" XDGABlitTransRect");
	if ( mode->flags & XDGAPixmap )
		printf(" XDGAPixmap");
	if ( mode->flags & XDGAInterlaced )
		printf(" XDGAInterlaced");
	if ( mode->flags & XDGADoublescan )
		printf(" XDGADoublescan");
	if ( mode->viewportFlags & XDGAFlipRetrace )
		printf(" XDGAFlipRetrace");
	if ( mode->viewportFlags & XDGAFlipImmediate )
		printf(" XDGAFlipImmediate");
	printf("\n");
}

void FGDGADriver::get_all_modes(vmode *p)
{
	/* Query for the list of available video modes */
	modes = XDGAQueryModes(DGA_Display, DGA_Screen, &num_modes);

	for (int m=0; m<num_modes; m++ )
	{
		if (verbose) PrintMode(&modes[m]);
		if (modes[m].bitsPerPixel!=FASTGL_BPP) continue;
		p[total_modes].flag = FG_LINEAR;
		p[total_modes].refresh =(int) (modes[m].verticalRefresh);
		p[total_modes].w = modes[m].viewportWidth;
		p[total_modes].h = modes[m].viewportHeight;
		p[total_modes].mode = modes[m].num;
		total_modes++;
	}
}

int FGDGADriver::set_mode(int ww, int hh)
{
   	req_w = ww;
	req_h = hh;
	FGDriver::set_mode(w,h);
	mode_num = find(req_w, req_h);
	if (mode_num==-1)
    {
       	printf("Mode %dx%d not found!\n", ww, hh);
        return 0;
    }

	/* Free any previous colormap */
	if ( DGA_colormap )
	{
		XFreeColormap(DGA_Display, DGA_colormap);
		DGA_colormap = 0;
	}

	/* Set the video mode */
	mode = XDGASetMode(DGA_Display, DGA_Screen, modelist[mode_num].mode);
	if (mode == 0)
	{
		perror("Unable to switch to requested mode");
		return 0;
	}
	videobase = (FGPixel *)mode->data;
	pitch = mode->mode.bytesPerScanline;

	islinear = 1;
	req_w = modelist[mode_num].w;
	req_h = modelist[mode_num].h;
	virt_w = mode->mode.imageWidth;
	virt_h = mode->mode.imageHeight;
	visible = 1;
	synch = modelist[mode_num].refresh;
	fgstate.palette_8=1;
	if ((mode->mode.flags & XDGASolidFillRect) == 0)
	    fgstate.noaccel = 1; //disable accel under X11

	/* Create a colormap if necessary */
#ifdef INDEX_COLORS
	DGA_colormap = XDGACreateColormap(DGA_Display, DGA_Screen, mode, AllocAll);
#else
	DGA_colormap = XDGACreateColormap(DGA_Display, DGA_Screen, mode, AllocNone);
#endif
	XDGAInstallColormap(DGA_Display, DGA_Screen, DGA_colormap);

	XDGASetViewport(DGA_Display, DGA_Screen, 0, 0, XDGAFlipRetrace);

	/* Enable mouse and keyboard support */
	XDGASelectInput(DGA_Display,
	  	DGA_Screen,
		KeyPressMask | KeyReleaseMask |
		ButtonPressMask | ButtonReleaseMask |
		PointerMotionMask );

	XDGASync(DGA_Display, DGA_Screen);

	// enable basic HW acceleration
	if (fgstate.noaccel==0)
	{
		accel_name = "Xfree DGA accelerator";
		ops = 1; // fillrect
		vector_fill_box = fillbox;
	}
	return postinit();
}

int FGDGADriver::text_mode(void)
{
	if ( DGA_Display )
	{
		XDGASync(DGA_Display, DGA_Screen);
		/* Free colormap, if necessary */
		if ( DGA_colormap ) {
			XFreeColormap(DGA_Display, DGA_colormap);
			DGA_colormap = 0;
		}

		/* Unmap memory and reset video mode */
		XDGACloseFramebuffer(DGA_Display, DGA_Screen);
		videobase = 0;
		XDGASetMode(DGA_Display, DGA_Screen, 0);

		/* Close up the display */
		XCloseDisplay(DGA_Display);
	}
	isgraph = 0;
	return 1;
}

int FGDGADriver::set_mouse(void)
{
	mousex = oldx = w/2;
	mousey = oldy = h/2;
	mouse_string = "DGA_MOUSE";
	return dgamouse = 1;
}

void FGDGADriver::reset_mouse(void)
{
    ismouse = dgamouse = 0;
}

int FGDGADriver::set_keyboard(void)
{
	return iskeyboard = 1;
}

void FGDGADriver::reset_keyboard(void)
{
	iskeyboard = 0;
}

int FGDGADriver::get_event(int& type, long& key, int& x, int& y, int& buttons)
{
//	static int ss=100;
	XDGAEvent xevent;

	// return if no events available
	if (XPending(DGA_Display)==0) { return 0;}

	// get event from fifo
	XNextEvent(DGA_Display, (XEvent *)&xevent);

	xevent.type -= DGA_event_base;

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
			mousex += xevent.xmotion.dx;
			if (mousex<0) mousex = 0;
			if (mousex>(X_width - 1)) mousex = (X_width - 1);
			mousey += xevent.xmotion.dy;
			if (mousey<0) mousey = 0;
			if (mousey>(Y_width-1)) mousey = (Y_width-1);

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
		{
			XKeyEvent xkey;
			XDGAKeyEventToXKeyEvent(&xevent.xkey, &xkey);
			key = recode_key(TranslateKey(&xkey, true));
			if (key==0x8000) return false;
//gprintf(0,-1,0,20, "key = %04x %x %x", xkey, key, xevent.xkey);
			type = KEYEVENT;
			x = y = buttons = 0;
			return 1;
	    case KeyRelease:
			{
				XKeyEvent xkey;
				XDGAKeyEventToXKeyEvent(&xevent.xkey, &xkey);
				key = recode_key(TranslateKey(&xkey, false));
				return false;
			}
		}
	}
	return false;
}

int FGDGADriver::set_page(int page)
{
	int yoffset = page*h;
	if (virt_h < h*(page+1))
		return 0; // small VRAM
	XDGASync(DGA_Display, DGA_Screen);
	/* Wait for vertical retrace and then flip display */
	while ( XDGAGetViewportStatus(DGA_Display, DGA_Screen) ) ;
	/* Keep waiting for the hardware ... */ ;
	XDGASetViewport(DGA_Display, DGA_Screen, 0, yoffset, XDGAFlipRetrace);
	return 1;
}

int FGDGADriver::fillbox(int x, int y, int w, int h, FGPixel ink, unsigned ppop)
{
	if (ppop) // fallback
	{
		return __fill_box(x,y,w,h,ink,ppop);
	}
	XDGAFillRectangle(DGA_Display,
		DGA_Screen,
		x,
		__fg_driver->h*__fg_driver->current_buffer+y,
		w,
		h,
		ink);
	XDGASync(DGA_Display, DGA_Screen);
	return 1;
}

#endif

#ifdef X11_DRIVER

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

void FGX11Driver::try_mitshm(int ww, int hh)
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

int	FGX11Driver::available(void)
{
		const char *display;
		Display *dpy;
		int available=0;

		/* The driver is available if the display is local
		*/
		display = NULL;
		FGLinuxDriver::available();
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

int	FGX11Driver::link(void)
{
	const char *display;

	on_the_fly_change = false;

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
		printf("Unsupported color depth %d BPP (X11 server is set to %d BitsPerPixel)\n\n", bpp*8, depth);
		printf("You have the two options:\n");
		printf("    1) Set new color depth for X server (run 'setup')\n");
		printf("    2) Recompile your code with right BPP (BPP16, BPP32 or BPP8)\n\n");

		// 14.2.2006 - 16->32 conversion runs slow but works!!!
		if (depth != 32)
			return false;

		on_the_fly_change = true;
	}

	/* Determine the current screen depth */
	thevisual = DefaultVisual(thedisplay, thescreen);

	if (root && ioperm(0x3b0, 0x3df - 0x3b0 + 1, 1))
	{
		printf("Cannot get permissions for I/O operations\n");
//	    return 0;
	}
	stage = 1;
	subname = "standard Xserver mode\n";
	set_signals();
	return true;
}

void FGX11Driver::get_all_modes(vmode *)
{
	total_modes = 1;		// set only one virtual videomode [window with this size]
}

void FGX11Driver::setsize(int ww, int hh)
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

void FGX11Driver::SetCaption(char *new_name)
{
	XTextProperty textproperty;
	char **__name=&new_name;

	/* Label the window */
	XStringListToTextProperty(__name, 1, &textproperty);
	XSetWMName(thedisplay, thewindow, &textproperty);
	XSetStandardProperties(thedisplay, thewindow, new_name, new_name, None, NULL, 0, NULL);
}

static unsigned rgb16to32[65536];

int FGX11Driver::set_mode(int ww, int hh)
{
	XEvent anevent;
	XTextProperty textproperty;
	char * windowname=getenv("EXENAME")/*"OpenGUI Application"*/, **__name=&windowname;

	synch = 0;
	mode_num = 0;		// force mode 0
	pitch = bpp;

	islinear = 1;
	visible = 1;
	fgstate.palette_8=1;

	if (thewindow == 0)
	{
		 /* Create the window */
		 thewindow = XCreateSimpleWindow(
			thedisplay,
			FGL_Root,
			0,
			0,
			ww,
			hh,
			0,
			CBLACK,
			CBLACK);

		 XSelectInput(
			thedisplay,
			thewindow,
			StructureNotifyMask);

		 XMapWindow(
			thedisplay,
			thewindow);

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
					  KeyReleaseMask |
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

	// aligning to 32bit fixes crashing with mit_shm extension !!!
	// I don't know why ...
	virt_w =  req_w = ww = theattr.width&0xfffc;
	virt_h =  req_h = hh = theattr.height&0xfffc;

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

		if (on_the_fly_change)
		{
			// create additional 24bit image
			shminfo2.shmid = shmget(IPC_PRIVATE, ww*hh*4, IPC_CREAT | 0777);
			shminfo2.shmaddr = (char *)shmat(shminfo2.shmid, 0, 0);
			shminfo2.readOnly = False;
			XShmAttach(thedisplay, &shminfo2);
			XSync(thedisplay, true);
			shmctl(shminfo2.shmid, IPC_RMID, NULL);

			theimage2 = XShmCreateImage(thedisplay, thevisual,
						24,
						ZPixmap,
						shminfo2.shmaddr, &shminfo2,
						ww, hh);

			for(int i=0; i<0x10000; i++)
			{
				#define conv16to32(rgb) \
					(((rgb<<8)&0xF80000)\
					 | ((rgb<<5)&0xFc00)\
					 | ((rgb<<3)&0xF8))
				rgb16to32[i] = conv16to32(i);
			}
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
#else
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

void FGX11Driver::UpdateRect(int x, int y, int xm, int ym, int w, int h)
{
	if (use_mitshm)
	{
		if (on_the_fly_change == false)
		{
			XShmPutImage(thedisplay, thewindow, thecontext, theimage, x,y, xm,ym, w,h, 0);
		}
		else
		{
			unsigned* dst = (unsigned *)shminfo2.shmaddr, *d;
			unsigned short* src = (unsigned short *)shminfo.shmaddr, *s;

			for(int i=0; i<h; i++)
			{
            	d = dst+(FGDriver::w*(y+i))+x;
            	s = src+(FGDriver::w*(y+i))+x;

				for(int j=0; j<w; j++)
				{
					*d++ = rgb16to32[*s++];
                }
			}
			XShmPutImage(thedisplay, thewindow, thecontext, theimage2, x,y, xm,ym, w,h, 0);
		}
	}
	else
	{
		XPutImage(thedisplay, thewindow, thecontext, theimage, x,y, xm,ym, w,h );
	}
	XFlush(thedisplay);
}

int FGX11Driver::set_mouse(void)
{
	mousex = oldx = w/2;
	mousey = oldy = h/2;
	mouse_string = "X11_MOUSE";
	return x11mouse = 1;
}

void FGX11Driver::reset_mouse(void)
{
    ismouse = x11mouse = 0;
}

int FGX11Driver::set_keyboard(void)
{
	return iskeyboard = 1;
}

void FGX11Driver::reset_keyboard(void)
{
	iskeyboard = 0;
}

//
// CURSOROUTEVENT ???
//
int FGX11Driver::get_event(int& type, long& key, int& x, int& y, int& buttons)
{
//	static int ss=100;
	XEvent xevent;

	// return if no events available
	if (XPending(thedisplay)==0)
		return 0;

	// get event from fifo
	XNextEvent(thedisplay, (XEvent *)&xevent);

//printf("event = %x\n", xevent.type);
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
			if (mousex>(X_width - 1)) mousex = (X_width - 1);
			mousey = xevent.xmotion.y;
			if (mousey<0) mousey = 0;
			if (mousey>(Y_width-1)) mousey = (Y_width-1);

mouse:
			buttons = 0;
			if (but&16) buttons |= FG_BUTTON_WHEEL_DOWN;
			if (but&8)  buttons |= FG_BUTTON_WHEEL_UP;
			if (but&4)  buttons |= FG_BUTTON_RIGHT; // right
			if (but&2)  buttons |= FG_BUTTON_MIDDLE; // middle
			if (but&1)  buttons |= FG_BUTTON_LEFT; // left
//printf("  key = %04x %x\n", but, buttons);
			x = mousex;
			y = mousey;
			key  = 0;
			type = MOUSEEVENT;
			return 1;

		case KeyPress:
			key = recode_key(TranslateKey(&xevent.xkey, true));
			if (key==0x8000) return false;
//printf("  key = %04x\n", key);
			type = KEYEVENT;
			x = y = buttons = 0;
			return 1;
		case KeyRelease:
			key = recode_key(TranslateKey(&xevent.xkey, false));
			return 0;
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
		case FocusOut:
			XUninstallColormap(thedisplay, xcmap);
		    break;
	    case FocusIn:
			XInstallColormap(thedisplay, xcmap);
			break;
#endif
	}
	return 0;
}

#endif

// ----------------------------------------------------------------------

#ifdef VESA_DRIVER

FGVesaDriver::FGVesaDriver()
: FGLinuxDriver("VESA VBE")
{
	LRMI_mem1 = 0;
	vesa_data.info = 0;
	vesa_data.mode = 0;
	vesa_memory = vesa_chiptype = 0;
	vesa_last_mode_set = -1;
}

FGVesaDriver::~FGVesaDriver()
{
	if (vesa_data.info)
	{
		LRMI_free_real(vesa_data.info);
		vesa_data.info = 0;
	}
	if (vesa_data.mode)
	{
		LRMI_free_real(vesa_data.mode);
		vesa_data.mode = 0;
	}
	if (LRMI_mem1)
	{
		LRMI_free_real(LRMI_mem1);
		LRMI_mem1 = 0;
	}
}

int	FGVesaDriver::link(void)
{
	if (LRMI_init() == false)
		return false;

	LRMI_mem1 = LRMI_alloc_real(VESA_REGS_SIZE);
	vesa_data.info = (struct vbe_info_block *) LRMI_alloc_real(sizeof (struct vbe_info_block) );
	vesa_data.mode = (struct vbe_mode_info_block *) LRMI_alloc_real(sizeof (struct vbe_mode_info_block));

	vesa_r.eax = 0x4f00;
	vesa_r.es = (unsigned int) vesa_data.info >> 4;
	vesa_r.edi = 0;

	LRMI_int(0x10, &vesa_r);
	if (vesa_r.eax != 0x4f)
		return false;

	return !vesa_init(0, 0, 0);
}

void FGVesaDriver::get_all_modes(vmode *p)
{
	for (int m=0x100; m<0x180; m++)
	{
		memset(&vesa_r, 0, sizeof (vesa_r));

		vesa_r.eax = 0x4f01;
		vesa_r.ecx = m;
		vesa_r.es = (unsigned int) vesa_data.mode >> 4;
		vesa_r.edi = (unsigned int) vesa_data.mode & 0xf;

		if (!LRMI_int(0x10, &vesa_r))
		{
			fprintf(stderr, "Can't get mode info (vm86 failure)\n");
			return ;
		}

		if (vesa_r.eax != 0x004F)
			continue;				// not valid mode
		// must be: linear[80] | graphic[10] | color[08] | supported[01]
		if ( (vesa_data.mode->mode_attributes &0x99) != 0x99 )
			continue;
		if (vesa_data.mode->bits_per_pixel != FASTGL_BPP)
			continue;

printf("mode %x = attr %x = [%d x %d]\n", m, vesa_data.mode->mode_attributes, vesa_data.mode->x_resolution, vesa_data.mode->y_resolution);
		p[total_modes].flag = true;
		p[total_modes].refresh = 0;
		p[total_modes].w = vesa_data.mode->x_resolution;
		p[total_modes].h = vesa_data.mode->y_resolution;
		p[total_modes].mode = m;
		total_modes++;
	}
}

int FGVesaDriver::set_mode(int ww, int hh)
{
	req_w = ww;
	req_h = hh;

	FGDriver::set_mode(w,h);
	mode_num = find(req_w, req_h);

	if (mode_num==-1)
	{
		printf("Mode %dx%d not found!\n", ww, hh);
		return 0;
	}

	vesa_r.eax = 0x4f01;
	vesa_r.ecx = mode_num;
	vesa_r.es = (unsigned int) vesa_data.mode >> 4;
	vesa_r.edi = (unsigned int) vesa_data.mode & 0xf;
	LRMI_int(0x10, &vesa_r);

	vesa_r.eax = 0x4f02;
	vesa_r.ebx = mode_num | 0x8000 | 0x4000;
	vesa_last_mode_set = vesa_r.ebx;
	LRMI_int(0x10, &vesa_r);

	islinear = 1;
	req_w = virt_w = vesa_data.mode->x_resolution;
	req_h = virt_h = vesa_data.mode->y_resolution;
	pitch = vesa_data.mode->bytes_per_scanline;
	linear_base = (void *)vesa_data.mode->phys_base_ptr;

	if (vesa_data.info->capabilities & 1) // CAPABLE
	{
		vesa_r.eax = 0x4f08; // set DAC width to 8 bit
		vesa_r.ebx = 0x0800;
		LRMI_int(0x10, &vesa_r);
		if (vesa_r.eax == 0x004F)
			fgstate.palette_8 = true;
	}

	return postinit();
}

int FGVesaDriver::text_mode(void)
{
	vesa_r.eax = 0x4f02;
	vesa_r.ebx = 3;
	vesa_last_mode_set = vesa_r.ebx;
	LRMI_int(0x10, &vesa_r);
	return true;
}

int FGVesaDriver::set_page(int page)
{
}

int FGVesaDriver::vesa_init(int force, int par1, int par2)
{
//	__svgalib_textprog |= 1;

	/* Get I/O priviledge */

	if (force)
	{
		vesa_memory = par1;
		vesa_chiptype = par2;
	}
	else
	{
		vesa_memory = 4096;
	};
/*
	__svgalib_LRMI_init();
	for (i = 0; i < __GLASTMODE; i++)
		SVGALIB_VESA[i] = IS_IN_STANDARD_VGA_DRIVER(i);
*/
	vesa_r.eax = 0x4f00;
	vesa_r.es = (unsigned int) vesa_data.info >> 4;
	vesa_r.edi = 0;

	memcpy(vesa_data.info->vbe_signature, "VBE2", 4);

	LRMI_int(0x10, &vesa_r);

	if ((vesa_r.eax & 0xffff) != 0x4f || strncmp(vesa_data.info->vbe_signature, "VESA", 4) != 0)
	{
		fprintf(stderr, "No VESA bios detected!\n");
		fprintf(stderr, "Try running vga_reset.\n");
		return 1;
	}

	if (vesa_data.info->vbe_version >= 0x0200)
		vesa_chiptype = 1;
	else
		return false;

	if (vesa_data.info->vbe_version >= 0x0300)
		vesa_chiptype = 2;

	vesa_memory = vesa_data.info->total_memory * 64;

#if 0
	while (*mode_list != -1)
	{
		memset(&vesa_r, 0, sizeof (vesa_r));

		vesa_r.eax = 0x4f01;
		vesa_r.ecx = *mode_list;
		vesa_r.es = (unsigned int) vesa_data.mode >> 4;
		vesa_r.edi = (unsigned int) vesa_data.mode & 0xf;

		if ((vesa_chiptype >= 1) && (vesa_data.mode->mode_attributes & 0x80))
			linear_base = (void *)vesa_data.mode->phys_base_ptr;
		if (!LRMI_int(0x10, &vesa_r))
		{
			fprintf(stderr, "Can't get mode info (vm86 failure)\n");
			return 1;
		}
		for (i = 0; i <= __GLASTMODE; i++)
			if ((infotable[i].xdim == vesa_data.mode->x_resolution) &&
				(infotable[i].ydim == vesa_data.mode->y_resolution) &&
				(((vesa_data.mode->rsvd_mask_size == 8) && (infotable[i].bytesperpixel == 4)) ||
				 ((vesa_data.mode->bits_per_pixel == 32) && (infotable[i].bytesperpixel == 4)) ||
				 ((vesa_data.mode->bits_per_pixel == 24) && (infotable[i].bytesperpixel == 3)) ||
				 ((vesa_data.mode->green_mask_size == 5) && (infotable[i].colors == 32768)) ||
				 ((vesa_data.mode->green_mask_size == 6) && (infotable[i].colors == 65536)) ||
				 ((vesa_data.mode->memory_model == VBE_MODEL_PLANAR) && (infotable[i].colors == 16)) ||
				 ((vesa_data.mode->memory_model == VBE_MODEL_256) && (infotable[i].colors == 256)) ||
				 ((vesa_data.mode->memory_model == VBE_MODEL_PACKED) && (infotable[i].colors == 256) && (vesa_data.mode->bits_per_pixel == 8))))
			{
				SVGALIB_VESA[i] = *mode_list;
				i = __GLASTMODE + 1;
			};

		mode_list++;
	};
#endif
/*
	vesa_r.eax = 0x4f04;
	vesa_r.edx = 0;
	vesa_r.ecx = __svgalib_VESA_savebitmap;
	vesa_r.ebx = 0;
	LRMI_int(0x10, &vesa_r);
	vesa_regs_size = vesa_r.ebx * 64;
	LRMI_free_real(vesa_data.info);
*/
//	SVGALIB_VESA[TEXT] = 3;
/*
	cardspecs = malloc(sizeof (CardSpecs));
	cardspecs->videoMemory = vesa_memory;
	cardspecs->maxPixelClock4bpp = 300000;
	cardspecs->maxPixelClock8bpp = 300000;
	cardspecs->maxPixelClock16bpp = 300000;
	cardspecs->maxPixelClock24bpp = 300000;
	cardspecs->maxPixelClock32bpp = 300000;
	cardspecs->flags = CLOCK_PROGRAMMABLE;
	cardspecs->maxHorizontalCrtc = 4088;
	cardspecs->nClocks = 1;
	cardspecs->clocks = NULL;
	cardspecs->mapClock = vesa_map_clock;
	cardspecs->mapHorizontalCrtc = vesa_map_horizontal_crtc;
	cardspecs->matchProgrammableClock = vesa_match_programmable_clock;
*/
//	LRMI_mem1 = LRMI_alloc_real(vesa_regs_size);
//	LRMI_mem2 = LRMI_alloc_real(sizeof (struct vbe_info_block) + sizeof (struct vbe_mode_info_block));

	islinear = true;
	linear_size = vesa_memory * 0x400;
	const char* vesaname = (vesa_chiptype == 2) ? "VBE3" : (vesa_chiptype ? "VBE2.0" : "VBE1.2");
	printf("Using VESA driver, %d KB. %s\n", vesa_memory, vesaname);
	return 0;
}
#endif

#ifdef FG_NAMESPACE
}
#endif


