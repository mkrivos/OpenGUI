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

#ifndef __FASTGL_H_
#define __FASTGL_H_

#ifdef FG_NAMESPACE
namespace fgl {
#endif

#ifdef __BORLANDC__
#pragma option push -b
#pragma option push -a4
#endif

/**
	@internal
*/
struct WindowItem
{
		int x, y, w, h;
		int icon_x, icon_y, flag;
		char *name;
		WindowItem *next;
	public:
		WindowItem(FILE *f);
		WindowItem(int xx, int yy, int ww, int hh, char *s);
		~WindowItem();
		void FGAPI SaveToFile(FILE *f);
};

class WindowDatabase
{
		friend struct WindowItem;
		int xyCounter;
		WindowItem *first;
		WindowItem **endpointer;
		char name[64], exe[64];
		FGMutex	atomic;
		inline void FGAPI lock(void)
		{
			atomic.Lock();
		}
		inline void FGAPI unlock(void)
		{
			atomic.Unlock();
		}
	public:
		WindowDatabase(char *);
		~WindowDatabase();
		void FGAPI Add(int &, int &, int &, int &, char *, int);
		void FGAPI DatabaseResize(int, int, char *);
		void FGAPI DatabaseRelocate(int, int, char *);
		void FGAPI DatabaseSetIcon(int, int, char *);
		void FGAPI DatabaseGetIcon(int &, int &, char *);
		WindowItem* FGAPI findxy(char *s);
};

#pragma pack(1)
/**
	@internal
*/
struct fonthdr
{
 struct font_t
 {
  int fontchar_w;
  int fontchar_h;
  unsigned int fontchar_fp;
// fix 64 bit
 };

  char font_name[256];
  int font_maxcharw;
  int font_maxcharh;
  int font_spacesize;
  font_t ftable[256];
};
#pragma pack()

/**
	@internal
*/
struct variable_record
{
		unsigned int  off[256];
		unsigned char w[256];
		int	mw,mh;	// max. nasobky X.Y, X je zaokruhlene na 4 bajty.
};

/**
	@internal
*/
struct tga_info
{
  unsigned char idLength;
  unsigned char colorMapType;

  unsigned char imageType;
  /* Known image types. */
#define TGA_TYPE_MAPPED      1
#define TGA_TYPE_COLOR       2
#define TGA_TYPE_GRAY        3

  unsigned char imageCompression;
  /* Only known compression is RLE */
#define TGA_COMP_NONE        0
#define TGA_COMP_RLE         1

  /* Color Map Specification. */
  /* We need to separately specify high and low bytes to avoid endianness
	 and alignment problems. */

  unsigned short colorMapIndex;
  unsigned short colorMapLength;
  unsigned char colorMapSize;

  /* Image Specification. */
  unsigned short xOrigin;
  unsigned short yOrigin;

  unsigned short width;
  unsigned short height;

  unsigned char bpp;
  unsigned char bytes;

  unsigned char alphaBits;
  unsigned char flipHoriz;
  unsigned char flipVert;

  /* Extensions (version 2) */

/* Not all the structures described in the standard are transcribed here
   only those which seem applicable to Gimp */

  char authorName[41];
  char comment[324];
  unsigned int month, day, year, hour, minute, second;
  char jobName[41];
  char softwareID[41];
  unsigned int pixelWidth, pixelHeight;  /* write dpi? */
  double gamma;
};

// buttony pre SVGA mouse
#define MOUSE_LEFTBUTTON 	4
#define MOUSE_MIDDLEBUTTON 	2
#define MOUSE_RIGHTBUTTON 	1

#define MOUSE_DEFAULTSAMPLERATE 150

/**
	@internal
*/
typedef struct
	{
		int width;
		int height;
		int bytesperpixel;
		int colors;
		int linewidth;			/* scanline width in bytes */
		int maxlogicalwidth;	/* maximum logical scanline width */
		int startaddressrange;	/* changeable bits set */
		int maxpixels;			/* video memory / bytesperpixel */
		int haveblit;			/* mask of blit functions available */
		int flags;				/* other flags */

		/* Extended fields: */

		int chiptype;			/* Chiptype detected */
		int memory;				/* videomemory in KB */
		int linewidth_unit;		/* Use only a multiple of this as parameter for set_logicalwidth and
								   set_displaystart */
		char *linear_aperture;	/* points to mmap secondary mem aperture of card (NULL if unavailable) */
		int aperture_size;		/* size of aperture in KB if size>=videomemory. 0 if unavail */
		void (*set_aperture_page) (int page);
		/* if aperture_size<videomemory select a memory page */
		void *extensions;		/* points to copy of eeprom for mach32 */
		/* depends from actual driver/chiptype.. etc. */
} vga_modeinfo;


#define FGTEXTBUFSZ		16384
#define FGDBLCLICKTIME	300		// timeout in [ms] for mouse doubleclick

#define		MSZ				24	// size of mouse cursor (X&Y)

//void XWaitRetrace(void) HIDDEN;
//void XWaitPeriod(void) HIDDEN;

/**
	@internal
*/
typedef struct
{
	int x, xlast;
	int y, ylast;
	int dx, dy;
	int xmajor;
	int xstep;
	int error;
}	polyedge;

/**
	@internal
*/
typedef enum
{
	inactive,
	active,
	passed
}	edgestat;

/**
	@internal
*/
typedef struct
{
	edgestat status;
	polyedge e;
}	edge;

/**
	@internal
*/
typedef struct _scan
{
	struct _scan *next;
	int x1, x2;
} scan;

/**
	@internal
*/
typedef void (*ScanFillFunc) (int x, int y, int w, FGPixel ink, unsigned ppop);

/**
	@internal
*/
typedef struct
{
	shapper_t	 line;
	ScanFillFunc scan;
	int gc_xoffset;
	int gc_yoffset;
	int gc_ycliplo;
	int gc_xcliplo;
	int gc_ycliphi;
	int gc_xcliphi;
}
PolygonStruct;

extern edge edge_array[128+2] HIDDEN;
extern scan scan_array[128+8] HIDDEN;

/**
	@internal
*/
enum Drivers
{
	FG_VESA, FG_CIRRUS, FG_TRIDENT, FG_S3, FG_S3V2,
	FG_MX, FG_TSENG3, FG_ATI, FG_TSENG4, FG_CHIPS,
	FG_WDIGITAL, FG_BANSHEE, FG_NVIDIA, FG_MATROX, FG_PERMEDIA,
	FG_INTEL740, FG_INTEL810, FG_RENDITION, FG_SIS,
	FG_LAST, FG_KERNEL_ACCEL
};

/**
	@internal
*/
struct PCICardInfo
{
	unsigned short vendor, device;
	void *MMIO_ADDR_DETECTED;
	void *LFB_ADDR_DETECTED;
	unsigned LFB_SIZE, MMIO_SIZE;
};

extern FGPalette ColorsArray[256] HIDDEN;

// show-hide mouse cursor
extern mouser_t   vector_mouse_cursor;

extern struct PCICardInfo CardInfo HIDDEN;

extern int X_virtual HIDDEN, Y_virtual HIDDEN;
extern int X_width;
extern int Y_width;
extern FGPixel *videobase HIDDEN;
void set_palette_map(int cnt, int ind, int *pal) HIDDEN;
extern int clip_x_min HIDDEN, clip_y_min HIDDEN, clip_x_max HIDDEN, clip_y_max HIDDEN;
extern int bpp  HIDDEN;
extern WindowDatabase *entryPoint;

extern int __draw_line(int x, int y, int w, int h, FGPixel ink, unsigned ppop) HIDDEN;
extern int __fill_box(int x, int y, int w, int h, FGPixel ink, unsigned ppop) HIDDEN;
extern void __draw_point(FGPixel *base, int x, int y, int ww, FGPixel color, unsigned ppop) HIDDEN;
extern const unsigned int _default_palette[256] HIDDEN;
extern unsigned int _fgl_palette[256] HIDDEN;
extern unsigned char miro0406[] HIDDEN, miro0808[] HIDDEN, miro0816[] HIDDEN, miro1220[] HIDDEN, miro1625[] HIDDEN,
  miro2034[] HIDDEN;

extern int detect_video(int v) HIDDEN;
extern unsigned int msss16[715*8] HIDDEN, msss16b[814*8] HIDDEN, msss13[407*8] HIDDEN;
extern FGPixel _internal_bitmap0[];

void do_polygon(int n, int pt[][2], PolygonStruct * current, FGPixel ink, unsigned ppop) HIDDEN;
void do_fill_polygon(int n, int pt[][2], PolygonStruct * current, FGPixel ink, unsigned ppop) HIDDEN;
void line(plotter_t draw, int a1, int b1, int a2, int b2, FGPattern * pat, FGPixel ink) HIDDEN;
void FGAPI GetPaletteEntry(unsigned char *rc, unsigned char *gc, unsigned char *bc, int i) HIDDEN;

void L1VideoToRam8(FGPixel *, FGPixel *, int, int, int, int) HIDDEN;
void L1RamToVideo2(FGPixel *, FGPixel *, int, int, int, int, unsigned ppop) HIDDEN;
void L1RamToVideo8(FGPixel *, FGPixel *, int, int, int, int) HIDDEN;
void L1RamToVideoA(FGPixel *, FGPixel *, int, int, int, int, int, unsigned ppop) HIDDEN;
void L1RamToRamA(FGPixel *, FGPixel *, int, int, int, int, int, unsigned ppop) HIDDEN;
void L1RamToRam(FGPixel *, FGPixel *, int, int, int, int) HIDDEN;
void L1RamToRamPpop(FGPixel *, FGPixel *, int, int, int, int, unsigned ppop) HIDDEN;
void L1RamToRamd(FGPixel *, FGPixel *, int, int, int, int) HIDDEN;
void L1Box(FGPixel *, int, int, FGPixel, int ww, unsigned ppop) HIDDEN;
void FGAPI CharOutClip(FGPixel *, FGPixel *, int offset, int w, int h, FGPixel ink, FGPixel paper) HIDDEN;
int  ClipLine(int &x, int &y, int &x1, int& y1, int clip_x_min, int clip_y_min, int clip_x_max, int clip_y_max) HIDDEN;

/**
	@internal
*/
struct FGInternalState
{
	int             mmx_state;
	int             __fgl_mmx;
	int             verbose;
	int             DoRootWindow;
	int             shut_down;
	int             __force_x, __force_y;
	int	            force_dac8;
	int	            force_ps2;
	int				force_mice;
	int             windowed_mode;
	int             __hint_x_menu, __hint_y_menu;
	int             synch;
	int             nofb, noaccel, nodga , nox11;
	int             CRTC;
	int             palette_8;
	int		nofbset;
	//! internal for polygons
    PolygonStruct   ps;
    //! this is a temp buffer for text drawing
    FGPixel         fgimagebuffer[FGTEXTBUFSZ];
	//! draw_pattern_line old pos
    int             xold, yold;
    //! draw_line old pos
    int             _oldx, _oldy;
    //! global transparency
    FGPixel         color_key;

	FGInternalState()
	{
		memset(this, 0, sizeof(*this));
		__hint_x_menu = __hint_y_menu = -1;
        color_key = (FGPixel)DEFAULT_COLOR_KEY;
		CRTC = 0x3d4;
	}
};

extern FGInternalState fgstate;

#define PICTOSIZE		20
#define TITLEH			20
#define BASEBORDER		4
#define MENUH			22
#define STATUSBARH		20

#define BORD1    		CScheme->wnd_bord1
#define BORD2			CScheme->wnd_bord2
#define BORD3			CScheme->wnd_bord3

#define CTITLE   		CScheme->inactive_title		// color of unactive title
#define CTITLEACTIVE 	CScheme->active_title	// color of active title
#define MENUBG	 		CScheme->menu_back

#define		NOTEGAP			8
#define 	NOTEBOOKH		18

#ifdef __BORLANDC__
#pragma option pop /* pop -a switch */
#pragma option pop /* pop -b */
#endif

#ifdef FG_NAMESPACE
}
#endif

#endif

